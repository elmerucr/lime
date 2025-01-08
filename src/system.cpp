#include "system.hpp"
#include "host.hpp"
#include "vdc.hpp"
#include <cmath>
#include "rca.hpp"

system_t::system_t()
{
    host = new host_t(this);
    vdc = new vdc_t();

	vdc->reset();
}

system_t::~system_t()
{
    delete vdc;
    delete host;
}

void system_t::run()
{
    running = true;

	// icon
	vdc->sprite[0] = { 88, 47, 0b00000101, 0x01 };
	vdc->sprite[1] = { 96, 47, 0b00000101, 0x02 };
	vdc->sprite[2] = { 88, 55, 0b00000101, 0x03 };
	vdc->sprite[3] = { 96, 55, 0b00000101, 0x04 };

	// text
	vdc->sprite[4] = {  83, 62, 0b00000111, 0x6c };	// l
	vdc->sprite[5] = {  88, 62, 0b00000111, 0x69 };	// i
	vdc->sprite[6] = {  94, 62, 0b00000111, 0x6d };	// m
	vdc->sprite[7] = { 102, 62, 0b00000111, 0x65 };	// e

	rca_t rca;

	for (int i=9; i<64; i++) {
		vdc->sprite[i].x = rca.byte();
		vdc->sprite[i].y = rca.byte();
		vdc->sprite[i].flags = 0b111;
		vdc->sprite[i].index = rca.byte();
		vdc->sprite[i].palette[0] = rca.byte() & 0b11;
		vdc->sprite[i].palette[1] = rca.byte() & 0b11;
		vdc->sprite[i].palette[2] = rca.byte() & 0b11;
		vdc->sprite[i].palette[3] = rca.byte() & 0b11;
	}

    while (running) {
        running = host->process_events();

		static uint8_t t = 0;

		vdc->bg0_x = (8 * cos(8.0*M_PI*((float)t)/255.0));
		vdc->bg0_y = (8 * sin(8.0*M_PI*((float)t)/255.0));
		t++;

		for (uint8_t s=0; s < VIDEO_HEIGHT; s++) {
			vdc->bg0_x += (abs(60 - (s >> 5)) % 14);
        	vdc->update_scanline(s);
		}

		for (int i=9; i<64; i++) {
			vdc->sprite[i].x--;
			vdc->sprite[i].y++;
			if (vdc->sprite[i].y == 240) vdc->sprite[i].x = rca.byte();
		}

        host->update_screen();
    }
}
