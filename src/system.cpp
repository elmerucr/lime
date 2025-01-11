// ---------------------------------------------------------------------
// system.cpp
// lime
//
// Copyright © 2025 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include "system.hpp"
#include "host.hpp"
#include "core.hpp"
#include "debugger.hpp"
#include "vdc.hpp"
#include <cmath>
#include "rca.hpp"

system_t::system_t()
{
	system_start_time = std::chrono::steady_clock::now();

	printf("lime v%i.%i.%i (C)%i elmerucr\n",
	       LIME_MAJOR_VERSION,
	       LIME_MINOR_VERSION,
	       LIME_BUILD, LIME_YEAR);

    host = new host_t(this);

	core = new core_t();
	debugger = new debugger_t();

	switch_to_run_mode();

    vdc = new vdc_t();
	vdc->reset();
}

system_t::~system_t()
{
    delete vdc;
	delete debugger;
	delete core;
    delete host;
}

void system_t::run()
{
    running = true;

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

	rca_t rca;

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

    while (running) {
        running = host->process_events();

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

        host->update_screen();
    }
}

void system_t::switch_mode()
{
	if (current_mode == RUN_MODE) {
		switch_to_debug_mode();
	} else {
		switch_to_run_mode();
	}
}

void system_t::switch_to_run_mode()
{
	current_mode = RUN_MODE;
	printf("[system] switching to run mode\n");
}

void system_t::switch_to_debug_mode()
{
	current_mode = DEBUG_MODE;
	printf("[system] switching to debug mode\n");
}
