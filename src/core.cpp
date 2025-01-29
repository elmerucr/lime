// ---------------------------------------------------------------------
// core.cpp
// lime
//
// (c)2025 elmerucr
// ---------------------------------------------------------------------

#include <cmath>
#include "core.hpp"
#include "common.hpp"

core_t::core_t(system_t *s)
{
	system = s;

	rom = new rom_t();

	vdc = new vdc_t();

	cpu = new cpu_t(system);

	exceptions = new exceptions_ic();
	cpu->assign_nmi_line(&exceptions->nmi_output_pin);
	cpu->assign_irq_line(&exceptions->irq_output_pin);

	font = new font_cbm_8x8_t();
}

core_t::~core_t()
{
	delete font;
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
		//timer->run(cpu_cycles);
		//uint16_t sound_cycles = cpu2sid->clock(cpu_cycles);
		//sound->run(sound_cycles);
		cpu_cycle_saldo += cpu_cycles;
		//sound_cycle_saldo += sound_cycles;

	} while ((!cpu->breakpoint()) && (!frame_done) && (!debug));

	if (cpu->breakpoint()) output_state = BREAKPOINT;

	if (cpu_cycle_saldo >= CPU_CYCLES_PER_FRAME) {
		cpu_cycle_saldo -= CPU_CYCLES_PER_FRAME;
		// if (generate_interrupts_frame_done) {
		// 	exceptions->pull(irq_number);
		// 	irq_line_frame_done = false;
		// }
	}

	return output_state;
}

uint8_t core_t::read8(uint16_t address)
{
	if (address) {
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
				if (cbm_font_visible) {
					return font->read8(address);
				} else {
					return vdc->ram[address];
				}
			case VDC_PAGE:
				return vdc->io_read8(address);
			case ROM_PAGE:
				return rom->data[address & 0xff];
			default:
				return vdc->ram[address];
		}
	} else {
		// address = 0
		return cbm_font_visible ? 0b10 : 0b00;
	}
}

void core_t::write8(uint16_t address, uint8_t value)
{
	if (address) {
		switch (address >> 8) {
			case VDC_PAGE:
				vdc->io_write8(address, value);
				break;
			default:
				vdc->ram[address] = value;
				break;
		}
	} else {
		cbm_font_visible = value & 0b10 ? true : false;
	}
}

void core_t::reset()
{
	cpu_cycle_saldo = 0;
	irq_line_frame_done = true;

	cbm_font_visible = false;

	vdc->reset();	// vdc before cpu, as vdc also inits ram
	cpu->reset();
}
