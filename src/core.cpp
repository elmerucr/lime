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

	rom_m68k = new rom_m68k_t();
	rom_mc6809 = new rom_mc6809_t();

	exceptions = new exceptions_ic();
	sn74ls148 = new sn74ls148_t(system);

	vdc = new vdc_t(exceptions, sn74ls148);

	cpu_mc6809 = new cpu_mc6809_t(system);

	cpu_mc6809->assign_nmi_line(&exceptions->nmi_output_pin);
	cpu_mc6809->assign_irq_line(&exceptions->irq_output_pin);

	cpu_m68k = new cpu_m68k_t(system);
	cpu_m68k->setModel(moira::Model::M68000 , moira::Model::M68000);
	cpu_m68k->setDasmSyntax(moira::Syntax::MOIRA);
	cpu_m68k->setDasmIndentation(8);

	timer = new timer_ic(exceptions, sn74ls148);

	sound = new sound_ic(system);

	cpu2sid = new clocks(CPU_CLOCK_SPEED/FPS, SID_CLOCK_SPEED/FPS);

	font = new font_cbm_8x8_t();

	// register core as an interrupt device
	dev_number_exceptions = exceptions->connect_device("core");
	printf("[core] Connecting to exceptions getting dev %i\n", dev_number_exceptions);

	dev_number_sn74ls148 = sn74ls148->connect_device(2, "core");
	printf("[core] Connecting to sn74ls148 at ipl 2 getting dev %i\n", dev_number_sn74ls148);

	m68k_active = false;
}

core_t::~core_t()
{
	delete font;
	delete cpu2sid;
	delete sound;
	delete timer;
	delete cpu_m68k;
	delete sn74ls148;
	delete cpu_mc6809;
	delete vdc;
	delete exceptions;
	delete rom_mc6809;
	delete rom_m68k;
}

enum output_states core_t::run(bool debug)
{
	enum output_states output_state = NORMAL;
	bool frame_done;

	do {

		uint16_t cpu_cycles = cpu_mc6809->execute();
		frame_done = vdc->run(cpu_cycles);
		timer->run(cpu_cycles);
		uint16_t sound_cycles = cpu2sid->clock(cpu_cycles);
		sound->run(sound_cycles);
		sound_cycle_saldo += sound_cycles;

	} while ((!cpu_mc6809->breakpoint()) && (!frame_done) && (!debug));

	if (cpu_mc6809->breakpoint()) output_state = BREAKPOINT;

	return output_state;
}

uint8_t core_t::read8(uint32_t address)
{
	address &= VDC_RAM_MASK;

	if (m68k_active & !(address & 0xfffff8)) {
		// make sure m68k vectors are read, if needed
		return rom_m68k->data[address];
	} else if ((address & 0xffff00) == 0x000400) {
		// combined io page
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
							(mc6809_rom_visible    ? 0b00000001 : 0b00000000) |
							(m68k_rom_visible      ? 0b00000010 : 0b00000000) |
							(character_rom_visible ? 0b00000100 : 0b00000000) ;
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
	} else if ((address & 0xffff00) == 0x000500) {
		// sound io page
		return sound->io_read_byte(address);
	} else if ((address & 0xfff000) == 0x001000) {
		// character rom
		if (character_rom_visible) {
			return font->io_read8(address);
		} else {
			return vdc->ram[address];
		}
	} else if ((address & 0xfffc00) == 0x00fc00) {
		// mc6809 rom
		if (mc6809_rom_visible) {
			return rom_mc6809->data[address & 0x3ff];
		} else {
			return vdc->ram[address];
		}
	} else if ((address & 0xff0000) == 0x010000) {
		// m68k rom
		if (m68k_rom_visible) {
			return rom_m68k->data[address & 0xffff];
		} else {
			return vdc->ram[address];
		}
	} else {
		// it's vdc ram
		return vdc->ram[address];
	}


	// switch (address >> 8) {
	// 	case CBM_FONT_PAGE + 0x0:
	// 	case CBM_FONT_PAGE + 0x1:
	// 	case CBM_FONT_PAGE + 0x2:
	// 	case CBM_FONT_PAGE + 0x3:
	// 	case CBM_FONT_PAGE + 0x4:
	// 	case CBM_FONT_PAGE + 0x5:
	// 	case CBM_FONT_PAGE + 0x6:
	// 	case CBM_FONT_PAGE + 0x7:
	// 	case CBM_FONT_PAGE + 0x8:
	// 	case CBM_FONT_PAGE + 0x9:
	// 	case CBM_FONT_PAGE + 0xa:
	// 	case CBM_FONT_PAGE + 0xb:
	// 	case CBM_FONT_PAGE + 0xc:
	// 	case CBM_FONT_PAGE + 0xd:
	// 	case CBM_FONT_PAGE + 0xe:
	// 	case CBM_FONT_PAGE + 0xf:
	// 		if (character_rom_visible) {
	// 			return font->io_read8(address);
	// 		} else {
	// 			return vdc->ram[address];
	// 		}
	// 	case COMBINED_PAGE:
	// 		switch (address & 0x00c0) {
	// 			case VDC_SUB_PAGE:
	// 				return vdc->io_read8(address);
	// 			case TIMER_SUB_PAGE:
	// 				return timer->io_read_byte(address);
	// 			case CORE_SUB_PAGE:
	// 				switch (address & 0x3f) {
	// 					case 0x00:
	// 						// status register
	// 						return irq_line ? 0b0 : 0b1;
	// 					case 0x01:
	// 						return
	// 							(generate_interrupts   ? 0b00000001 : 0b00000000) ;
	// 					case 0x02:
	// 						// core roms
	// 						return
	// 							(mc6809_rom_visible    ? 0b00000001 : 0b00000000) |
	// 							(m68k_rom_visible      ? 0b00000010 : 0b00000000) |
	// 							(character_rom_visible ? 0b00000100 : 0b00000000) ;
	// 					case 0x04:
	// 						return file_data[file_pointer++];
	// 					case 0x08:
	// 						// Controller is NES style
	// 						return
	// 							((system->host->keyboard_state[SCANCODE_UP]     & 0b1) ? 0b00000001 : 0) |	// up
	// 							((system->host->keyboard_state[SCANCODE_DOWN]   & 0b1) ? 0b00000010 : 0) |	// down
	// 							((system->host->keyboard_state[SCANCODE_LEFT]   & 0b1) ? 0b00000100 : 0) |	// left
	// 							((system->host->keyboard_state[SCANCODE_RIGHT]  & 0b1) ? 0b00001000 : 0) |	// right
	// 							((system->host->keyboard_state[SCANCODE_Z]      & 0b1) ? 0b00010000 : 0) |	// A
	// 							((system->host->keyboard_state[SCANCODE_X]      & 0b1) ? 0b00100000 : 0) |	// B
	// 							((system->host->keyboard_state[SCANCODE_RSHIFT] & 0b1) ? 0b01000000 : 0) |	// Select
	// 							((system->host->keyboard_state[SCANCODE_RETURN] & 0b1) ? 0b10000000 : 0) ;	// Start
	// 					default:
	// 						return 0x00;
	// 				}
	// 			default:
	// 				return 0x00;
	// 		}
	// 	case SOUND_PAGE:
	// 		return sound->io_read_byte(address);
	// 	case MC6809_ROM_PAGE:
	// 	case MC6809_ROM_PAGE+1:
	// 	case MC6809_ROM_PAGE+2:
	// 	case MC6809_ROM_PAGE+3:
	// 		if (mc6809_rom_visible) {
	// 			return rom_mc6809->data[address & 0x3ff];
	// 		} else {
	// 			return vdc->ram[address];
	// 		}
	// 	default:
	// 		return vdc->ram[address];
	// }
}

void core_t::write8(uint32_t address, uint8_t value)
{
	address &= VDC_RAM_MASK;
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
								exceptions->release(dev_number_exceptions);
								irq_line = true;
							}
							break;
						case 0x01:
							if (value & 0b00000001) {
								generate_interrupts = true;
								if (bin_attached == true) {
									bin_attached = false;
									exceptions->pull(dev_number_exceptions);
									irq_line = false;
								}
							} else {
								generate_interrupts = false;
							}
							break;
						case 0x02:
							mc6809_rom_visible    = (value & 0b00000001) ? true : false;
							m68k_rom_visible      = (value & 0b00000010) ? true : false;
							character_rom_visible = (value & 0b00000100) ? true : false;
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
	bin_attached = false;

	mc6809_rom_visible = true;
	m68k_rom_visible = true;
	character_rom_visible = false;

	sound->reset();
	timer->reset();
	vdc->reset();	// vdc before cpu, as vdc also inits ram
	cpu_mc6809->reset();

	// temp hack for 68000
	write8(0, 0x00);
	write8(1, 0x00);
	write8(2, 0xc0);
	write8(3, 0x00);
	write8(4, 0x00);
	write8(5, 0x00);
	write8(6, 0xd0);
	write8(7, 0x00);
	write8(0xd000, 0x52);
	write8(0xd001, 0x39);
	write8(0xd002, 0x00);
	write8(0xd003, 0x00);
	write8(0xd004, 0x04);
	write8(0xd005, 0x04);
	write8(0xd006, 0x4e);
	write8(0xd007, 0xf9);
	write8(0xd008, 0x00);
	write8(0xd009, 0x00);
	write8(0xd00a, 0xd0);
	write8(0xd00b, 0x00);

	cpu_m68k->reset();
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
			size_t bytes_read = fread(file_data, pos, 1, f);
			fclose(f);
			for (int i=pos; i<65536; i++) {
				file_data[i] = 0x00;
			}
			file_pointer = 0;
			printf("[core] Attaching file\n");
			bin_attached = true;
			if (generate_interrupts) {
				exceptions->pull(dev_number_exceptions);
				irq_line = false;
				bin_attached = false;
			}
		}
	} else {
		printf("[core] Can't load: %s is a directory\n", path);
	}
}
