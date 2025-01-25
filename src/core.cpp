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
}

core_t::~core_t()
{
	delete exceptions;
	delete cpu;
	delete vdc;
	delete rom;
}

enum output_states core_t::run(bool debug)
{
	for (uint8_t s=0; s < VIDEO_YRES; s++) {
		vdc->draw_scanline(s);
	}
	return BREAKPOINT;
}

uint8_t core_t::read8(uint16_t address)
{
	switch (address >> 8) {
		case VDC_PAGE:
			return vdc->io_read8(address);
		case ROM_PAGE:
			return rom->data[address & 0xff];
		default:
			return vdc->ram[address];
	}
}

void core_t::write8(uint16_t address, uint8_t value)
{
	switch (address >> 8) {
		case VDC_PAGE:
			vdc->io_write8(address, value);
			break;
		default:
			vdc->ram[address] = value;
			break;
	}
}

void core_t::reset()
{
	cpu_cycle_saldo = 0;
	irq_line_frame_done = true;

	vdc->reset();	// vdc before cpu, as vdc also inits ram
	cpu->reset();
}

// enum output_states core_t::run(bool debug)
// {
// 	enum output_states output_state = NORMAL;

// 	do {

// 		uint16_t cpu_cycles = cpu->execute();
// 		timer->run(cpu_cycles);
// 		uint16_t sound_cycles = cpu2sid->clock(cpu_cycles);
// 		sound->run(sound_cycles);
// 		cpu_cycle_saldo += cpu_cycles;
// 		sound_cycle_saldo += sound_cycles;

// 	} while ((!cpu->breakpoint()) && (cpu_cycle_saldo < CPU_CYCLES_PER_FRAME) && (!debug));

// 	if (cpu->breakpoint()) output_state = BREAKPOINT;

// 	if (cpu_cycle_saldo >= CPU_CYCLES_PER_FRAME) {
// 		cpu_cycle_saldo -= CPU_CYCLES_PER_FRAME;
// 		if (generate_interrupts_frame_done) {
// 			exceptions->pull(irq_number);
// 			irq_line_frame_done = false;
// 		}
// 		blitter->set_pixel_saldo(MAX_PIXELS_PER_FRAME);
// 	}

// 	return output_state;
// }
