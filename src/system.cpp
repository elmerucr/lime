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
#include <cmath>

system_t::system_t()
{
	system_start_time = std::chrono::steady_clock::now();

	printf("lime v%i.%i.%i (C)%i elmerucr\n",
	       LIME_MAJOR_VERSION,
	       LIME_MINOR_VERSION,
	       LIME_BUILD, LIME_YEAR);

    host = new host_t(this);

	core = new core_t(this);
	debugger = new debugger_t();

	switch_to_run_mode();
}

system_t::~system_t()
{
	delete debugger;
	delete core;
    delete host;
}

void system_t::run()
{
    running = true;

    while (running) {
        running = host->process_events();

		switch (current_mode) {
			case RUN_MODE:
				core->run();
				break;
			case DEBUG_MODE:
				//
				break;
		};

        host->update_screen();
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

void system_t::switch_mode()
{
	if (current_mode == RUN_MODE) {
		switch_to_debug_mode();
	} else {
		switch_to_run_mode();
	}
}
