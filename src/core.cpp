// ---------------------------------------------------------------------
// core.cpp
// lime
//
// (c)2025 elmerucr
// ---------------------------------------------------------------------

#include <cmath>
#include "core.hpp"
#include "common.hpp"
#include "host.hpp"
#include "keyboard.hpp"
#include <unistd.h>

core_t::core_t(system_t *s)
{
	system = s;

	rom = new rom_t();

	sn74ls148 = new sn74ls148_t(system);

	vdc = new vdc_t(sn74ls148);

	cpu = new cpu_t(system);
	cpu->setModel(moira::Model::M68000 , moira::Model::M68000);
	cpu->setDasmSyntax(moira::Syntax::MOIRA);
	cpu->setDasmIndentation(8);

	timer = new timer_ic(sn74ls148);

	sound = new sound_ic(system);

	cpu_to_core_clock = new clocks(cpu_multiplier, 1);

	core_to_sid_clock = new clocks(CORE_CYCLES_PER_FRAME, SID_CYCLES_PER_FRAME);

	font_cbm = new font_cbm_8x8_t();
	font_4x8 = new font_4x8_t();

	// register core as an interrupt device

	dev_number_sn74ls148 = sn74ls148->connect_device(2, "core");
	printf("[core] Connecting to sn74ls148 at ipl 2 getting dev %i\n", dev_number_sn74ls148);

	// 16mb should fit anything for mc68000
	file_data = new uint8_t[0x1000000];
}

core_t::~core_t()
{
	delete [] file_data;
	delete font_4x8;
	delete font_cbm;
	delete core_to_sid_clock;
	delete cpu_to_core_clock;
	delete sound;
	delete timer;
	delete cpu;
	delete sn74ls148;
	delete vdc;
	delete rom;
}

enum output_states core_t::run(bool debug)
{
	enum output_states output_state = NORMAL;
	uint16_t cpu_cycles;
	uint16_t core_cycles;
	bool frame_done;
	uint16_t sound_cycles;

	cpu_to_core_clock->adjust_base_clock(0b1 << cpu_multiplier);

	do {

		cpu->execute();
		cpu_cycles = cpu->getClock() - cpu->old_clock;
		cpu->old_clock += cpu_cycles;

		core_cycles = cpu_to_core_clock->clock(cpu_cycles);

		frame_done = vdc->run(core_cycles);
		timer->run(core_cycles);
		sound_cycles = core_to_sid_clock->clock(core_cycles);
		sound->run(sound_cycles);
		sound_cycle_saldo += sound_cycles;

	} while((!cpu->breakpoint_reached) && (!frame_done) && (!debug));

	if (cpu->breakpoint_reached) {
		cpu->breakpoint_reached = false;
		output_state = BREAKPOINT;
	}

	return output_state;
}

uint8_t core_t::io_read8(uint32_t address)
{
	switch (address & 0x3f) {
		case 0x00:
			// status register
			return irq_line ? 0b0 : 0b1;
		case 0x01:
			// control register
			return
				(generate_interrupts   ? 0b00000001 : 0b00000000) ;
		case 0x02:
			// core roms
			return
				(system_rom_visible        ? 0b00000001 : 0b00000000) |
				(character_cbm_rom_visible ? 0b00000010 : 0b00000000) |
				(character_4x8_rom_visible ? 0b00000100 : 0b00000000) ;
		case 0x03:
			return cpu_multiplier;
		case 0x04:
			return file_data[file_pointer++];
		case 0x08:
			// Controller is NES style
			return
				((system->host->keyboard_state[SCANCODE_UP]     & 0b1) ? 0b00000001 : 0) |	// up
				((system->host->keyboard_state[SCANCODE_DOWN]   & 0b1) ? 0b00000010 : 0) |	// down
				((system->host->keyboard_state[SCANCODE_LEFT]   & 0b1) ? 0b00000100 : 0) |	// left
				((system->host->keyboard_state[SCANCODE_RIGHT]  & 0b1) ? 0b00001000 : 0) |	// right
				((system->host->keyboard_state[SCANCODE_Z]      & 0b1) ? 0b00010000 : 0) |	// A
				((system->host->keyboard_state[SCANCODE_X]      & 0b1) ? 0b00100000 : 0) |	// B
				((system->host->keyboard_state[SCANCODE_RSHIFT] & 0b1) ? 0b01000000 : 0) |	// Select
				((system->host->keyboard_state[SCANCODE_RETURN] & 0b1) ? 0b10000000 : 0) ;	// Start
		default:
			return 0xff;
	}
}

void core_t::io_write8(uint32_t address, uint8_t value)
{
	switch (address & 0x3f) {
	case 0x00:
		// status register
		if ((value & 0b1) && !irq_line) {
			sn74ls148->release_line(dev_number_sn74ls148);
			irq_line = true;
		}
		break;
	case 0x01:
		// control register
		if (value & 0b00000001) {
			generate_interrupts = true;
			if (bin_attached == true) {
				bin_attached = false;
				sn74ls148->pull_line(dev_number_sn74ls148);
				irq_line = false;
			}
		} else {
			generate_interrupts = false;
		}
		break;
	case 0x02:
		system_rom_visible        = (value & 0b00000001) ? true : false;
		character_cbm_rom_visible = (value & 0b00000010) ? true : false;
		character_4x8_rom_visible = (value & 0b00000100) ? true : false;
		break;
	case 0x03:
		cpu_multiplier = value & 0b11;
		break;
	default:
		//
		break;
	}
}

uint8_t core_t::read8(uint32_t address)
{
	address &= VDC_RAM_MASK;

	if (!(address & 0xfffff8)) {
		// make sure mc68000 vectors are read, if needed
		return rom->data[address];
	} else if ((address & 0xffff00) == COMBINED_IO_PAGE) {
		switch (address & 0x00c0) {
			case VDC_SUB_PAGE_1:
			case VDC_SUB_PAGE_2:
				return vdc->io_read8(address);
			case CORE_SUB_PAGE:
				return io_read8(address);
			case TIMER_SUB_PAGE:
				return timer->io_read_byte(address);
			default:
				return 0x00;
		}
	} else if ((address & 0xffff00) == SOUND_IO_PAGE) {
		return sound->io_read_byte(address);
	} else if ((address & 0xffff00) == KEYBOARD_IO_PAGE) {
		return system->keyboard->io_read8(address);
	} else if ((address & 0xfff800) == FONT_4X8_PAGE) {
		if (character_4x8_rom_visible) {
			return font_4x8->io_read8(address);
		} else {
			return vdc->ram[address];
		}
	} else if ((address & 0xfff000) == FONT_CBM_PAGE) {
		if (character_cbm_rom_visible) {
			return font_cbm->io_read8(address);
		} else {
			return vdc->ram[address];
		}
	} else if ((address & 0xff0000) == ROM_ADDRESS) {
		if (system_rom_visible) {
			return rom->data[address & 0xffff];
		} else {
			return vdc->ram[address];
		}
	} else {
		// it's vdc ram
		return vdc->ram[address];
	}
}

void core_t::write8(uint32_t address, uint8_t value)
{
	address &= VDC_RAM_MASK;

	if ((address & 0xffff00) == COMBINED_IO_PAGE) {
		// combined page
		switch (address & 0xc0) {
			case VDC_SUB_PAGE_1:
			case VDC_SUB_PAGE_2:
				vdc->io_write8(address, value);
				break;
			case TIMER_SUB_PAGE:
				timer->io_write_byte(address, value);
				break;
			case CORE_SUB_PAGE:
				io_write8(address, value);
				break;
			default:
				//
				break;
		}
	} else if ((address & 0xffff00) == SOUND_IO_PAGE) {
		sound->io_write_byte(address, value);
	} else if ((address & 0xffff00) == KEYBOARD_IO_PAGE) {
		system->keyboard->io_write8(address, value);
	} else {
		vdc->ram[address] = value;
	}
}

void core_t::reset()
{
	irq_line = true;
	generate_interrupts = false;
	bin_attached = false;

	system_rom_visible = true;
	character_cbm_rom_visible = false;
	character_4x8_rom_visible = false;

	sound->reset();
	timer->reset();
	vdc->reset();	// vdc before cpu, as vdc also inits ram

	cpu->reset();
	cpu->old_clock = 0;
	cpu->setClock(0);

	cpu_to_core_clock->reset();

	cpu_multiplier = 0b11;
}

void core_t::attach_bin(const char *path)
{
	if (chdir(path)) {
		f = fopen(path, "r");
		fseek(f, 0L, SEEK_END);
		long pos = ftell(f);
		printf("[core] %s - %lu bytes\n", path, pos);
		if (pos > 0x1000000) {
			printf("[core] Can't load: file larger than 16mb\n");
			fclose(f);
		} else {
			// go back to beginning of file, read data
			rewind(f);
			size_t bytes_read = fread(file_data, pos, 1, f);
			fclose(f);

			// fill remaining buffer space with 0x00
			for (int i=pos; i < 0x1000000; i++) {
				file_data[i] = 0x00;
			}

			file_pointer = 0;
			printf("[core] Attaching file\n");
			bin_attached = true;
			if (generate_interrupts) {
				sn74ls148->pull_line(dev_number_sn74ls148);
				irq_line = false;
				bin_attached = false;
			}
		}
	} else {
		printf("[core] Can't load: %s is a directory\n", path);
	}
}
