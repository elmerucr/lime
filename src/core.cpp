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
#include <unistd.h>

core_t::core_t(system_t *s)
{
	system = s;

	rom = new rom_t();

	exceptions = new exceptions_ic();

	vdc = new vdc_t(exceptions);

	cpu = new cpu_t(system);

	cpu->assign_nmi_line(&exceptions->nmi_output_pin);
	cpu->assign_irq_line(&exceptions->irq_output_pin);

	timer = new timer_ic(exceptions);

	sound = new sound_ic(system);

	cpu2sid = new clocks(CPU_CLOCK_SPEED/FPS, SID_CLOCK_SPEED/FPS);

	font = new font_cbm_8x8_t();

	// register core as an interrupt device
	irq_number = exceptions->connect_device("core");
	printf("[core] Connecting to exceptions getting irq %i\n", irq_number);
}

core_t::~core_t()
{
	delete font;
	delete cpu2sid;
	delete sound;
	delete timer;
	delete exceptions;
	delete cpu;
	delete vdc;
	delete rom;
}

enum output_states core_t::run(bool debug)
{
	enum output_states output_state = NORMAL;
	bool frame_done;

	do {

		uint16_t cpu_cycles = cpu->execute();
		frame_done = vdc->run(cpu_cycles);
		timer->run(cpu_cycles);
		uint16_t sound_cycles = cpu2sid->clock(cpu_cycles);
		sound->run(sound_cycles);
		sound_cycle_saldo += sound_cycles;

	} while ((!cpu->breakpoint()) && (!frame_done) && (!debug));

	if (cpu->breakpoint()) output_state = BREAKPOINT;

	return output_state;
}

uint8_t core_t::read8(uint16_t address)
{
	uint8_t r;
	switch (address >> 8) {
		case CBM_FONT_PAGE + 0x0:
		case CBM_FONT_PAGE + 0x1:
		case CBM_FONT_PAGE + 0x2:
		case CBM_FONT_PAGE + 0x3:
		case CBM_FONT_PAGE + 0x4:
		case CBM_FONT_PAGE + 0x5:
		case CBM_FONT_PAGE + 0x6:
		case CBM_FONT_PAGE + 0x7:
		case CBM_FONT_PAGE + 0x8:
		case CBM_FONT_PAGE + 0x9:
		case CBM_FONT_PAGE + 0xa:
		case CBM_FONT_PAGE + 0xb:
		case CBM_FONT_PAGE + 0xc:
		case CBM_FONT_PAGE + 0xd:
		case CBM_FONT_PAGE + 0xe:
		case CBM_FONT_PAGE + 0xf:
			if (character_rom_visible) {
				return font->io_read8(address);
			} else {
				return vdc->ram[address];
			}
		case COMBINED_PAGE:
			switch (address & 0x00c0) {
				case VDC_SUB_PAGE:
					return vdc->io_read8(address);
				case TIMER_SUB_PAGE:
					return timer->io_read_byte(address);
				case CORE_SUB_PAGE:
					switch (address & 0x3f) {
						case 0x00:
							// status register
							return irq_line ? 0b0 : 0b1;
						case 0x01:
							return
								(generate_interrupts   ? 0b00000001 : 0b00000000) ;
						case 0x02:
							// core roms
							return
								(system_rom_visible    ? 0b00000001 : 0b00000000) |
								(character_rom_visible ? 0b00000010 : 0b00000000) ;
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
							return 0x00;
					}
				default:
					return 0x00;
			}
		case SOUND_PAGE:
			return sound->io_read_byte(address);
		case SYSTEM_ROM_PAGE:
		case SYSTEM_ROM_PAGE+1:
			if (system_rom_visible) {
				return rom->data[address & 0x1ff];
			} else {
				return vdc->ram[address];
			}
		default:
			return vdc->ram[address];
	}
}

void core_t::write8(uint16_t address, uint8_t value)
{
	switch (address >> 8) {
		case COMBINED_PAGE:
			switch (address & 0x00c0) {
				case VDC_SUB_PAGE:
					vdc->io_write8(address, value);
					break;
				case TIMER_SUB_PAGE:
					timer->io_write_byte(address, value);
					break;
				case CORE_SUB_PAGE:
					switch (address & 0x3f) {
						case 0x00:
							if ((value & 0b1) && !irq_line) {
								exceptions->release(irq_number);
								irq_line = true;
							}
							break;
						case 0x01:
							generate_interrupts   = (value & 0b00000001) ? true : false;
							break;
						case 0x02:
							system_rom_visible    = (value & 0b00000001) ? true : false;
							character_rom_visible = (value & 0b00000010) ? true : false;
							break;
						default:
							//
							break;
					}
					break;
				default:
					//
					break;
			}
			break;
		case SOUND_PAGE:
			sound->io_write_byte(address, value);
			break;
		default:
			vdc->ram[address] = value;
			break;
	}
}

void core_t::reset()
{
	irq_line = true;
	generate_interrupts = false;

	system_rom_visible = true;
	character_rom_visible = false;

	sound->reset();
	timer->reset();
	vdc->reset();	// vdc before cpu, as vdc also inits ram
	cpu->reset();
}

void core_t::attach_bin(char *path)
{
	if (chdir(path)) {
		f = fopen(path, "r");
		fseek(f, 0L, SEEK_END);
		long pos = ftell(f);
		printf("[core] %s - %lu bytes\n", path, pos);
		if (pos > 65535) {
			printf("[core] Can't load: file too large\n");
			fclose(f);
		} else {
			// go back to beginning of file, read data
			rewind(f);
			fread(file_data, pos, 1, f);
			fclose(f);
			for (int i=pos; i<65536; i++) {
				file_data[i] = 0x00;
			}
			file_pointer = 0;
			printf("[core] Attaching file\n");
			if (generate_interrupts) {
				exceptions->pull(irq_number);
				irq_line = false;
			}
		}
	} else {
		printf("[core] Can't load: %s is a directory\n", path);
	}
}
