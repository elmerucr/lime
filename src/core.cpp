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
}

void core_t::run()
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
}

uint8_t core_t::read8(uint16_t address)
{
	return vdc->ram[address];
}

void core_t::write8(uint16_t address, uint8_t value)
{
	vdc->ram[address] = value;
}

void core_t::reset()
{
	cpu->reset();
	vdc->reset();
}
