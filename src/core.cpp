// ---------------------------------------------------------------------
// core.cpp
// lime
//
// (c)2025 elmerucr
// ---------------------------------------------------------------------

#include "core.hpp"
#include "common.hpp"

core_t::core_t(system_t *s)
{
	system = s;

	rom = new rom_t();

	vdc = new vdc_t();

	cpu = new cpu_t(system);

	vdc->layer[0].flags = 0b101;
	vdc->layer[1].palette[0b11] = 0b01;
	vdc->layer[1].flags = 0b001;

	// icon
	vdc->sprite[0] = { 112, 64, 0b00000101, 0x01 };
	vdc->sprite[1] = { 120, 64, 0b00000101, 0x02 };
	vdc->sprite[2] = { 112, 72, 0b00000101, 0x03 };
	vdc->sprite[3] = { 120, 72, 0b00000101, 0x04 };

	// text
	vdc->sprite[4] = { 107, 80, 0b00000111, 0x6c };	// l
	vdc->sprite[5] = { 112, 80, 0b00000111, 0x69 };	// i
	vdc->sprite[6] = { 118, 80, 0b00000111, 0x6d };	// m
	vdc->sprite[7] = { 126, 80, 0b00000111, 0x65 };	// e

	for (int i=8; i<256; i++) {
		vdc->sprite[i].x = rca.byte();
		vdc->sprite[i].y = rca.byte();
		vdc->sprite[i].flags = 0b111 | (rca.byte() & 0b1000);
		vdc->sprite[i].index = rca.byte();
		vdc->sprite[i].palette[0] = rca.byte() & 0b11;
		vdc->sprite[i].palette[1] = rca.byte() & 0b11;
		vdc->sprite[i].palette[2] = rca.byte() & 0b11;
		vdc->sprite[i].palette[3] = rca.byte() & 0b11;
	}
}

core_t::~core_t()
{
	delete cpu;
	delete vdc;
	delete rom;
}

enum output_states core_t::run(bool debug)
{
	static uint8_t t = 0;
	vdc->layer[0].x = int(32 * cos(4*M_PI*(t/255.0)));
	vdc->layer[0].y = int(32 * sin(4*M_PI*(t/255.0)));
	t++;
	vdc->layer[1].y++;

	for (uint8_t s=0; s < SCANLINES; s++) {
		//vdc->layer[0].x += (abs(60 - (s >> 5)) % 14);
		vdc->update_scanline(s);
	}

	for (int i=8; i<255; i++) {
		vdc->sprite[i].x--;
		vdc->sprite[i].y++;
		if (vdc->sprite[i].y == 240) vdc->sprite[i].x = rca.byte();
	}

	return BREAKPOINT;
}

uint8_t core_t::read8(uint16_t address)
{
	switch (address >> 8) {
		case ROM_PAGE:
			return rom->data[address & 0xff];
		default:
			return vdc->ram[address];
	}
}

void core_t::write8(uint16_t address, uint8_t value)
{
	vdc->ram[address] = value;
}

void core_t::reset()
{
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
