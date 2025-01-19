// ---------------------------------------------------------------------
// system.cpp
// lime
//
// Copyright © 2025 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include "system.hpp"
#include "host.hpp"
#include "core.hpp"
#include "keyboard.hpp"
#include "debugger.hpp"
#include "stats.hpp"
#include <cmath>

system_t::system_t()
{
	system_start_time = std::chrono::steady_clock::now();

	printf("[lime] v%i.%i.%i (C)%i elmerucr\n",
	       LIME_MAJOR_VERSION,
	       LIME_MINOR_VERSION,
	       LIME_BUILD, LIME_YEAR);

    host = new host_t(this);

	core = new core_t(this);

	keyboard = new keyboard_t(this);
	keyboard->reset();
	keyboard->enable_events();

	debugger = new debugger_t(this);

	stats = new stats_t(this);

	// default start mode
	switch_to_run_mode();
	//switch_to_debug_mode();

	core->reset();
}

system_t::~system_t()
{
	delete stats;
	delete debugger;
	delete keyboard;
	delete core;
    delete host;

	printf("[lime] %.2f seconds running time\n", (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - system_start_time).count() / 1000);
}

void system_t::run()
{
    running = true;

	stats->reset();

	end_of_frame_time = std::chrono::steady_clock::now();

    while (running) {

        if (host->events_process_events() == QUIT_EVENT) running = false;

		keyboard->process();

		switch (current_mode) {
			case RUN_MODE:
				core->run();
				break;
			case DEBUG_MODE:
				debugger->run();
				//
				break;
		};

		// Time measurement
		stats->start_idle_time();

		/*
		 * If vsync is enabled, the update screen function takes more
		 * time, i.e. it will return after a few milliseconds, exactly
		 * when vertical refresh is done. This will avoid tearing.
		 * There's no need then to let the system sleep with a
		 * calculated value. But we will still have to do a time
		 * measurement for estimation of idle time.
		 *
		 * When there's no vsync, sleep time is done manually.
		 */
		if (host->vsync_disabled() || (stats->current_smoothed_framerate() > (FPS + 1))) {
			end_of_frame_time += std::chrono::microseconds(1000000/FPS);
			/*
			 * If the next update is in the past, calculate a
			 * new update moment.
			 */
			if (end_of_frame_time > std::chrono::steady_clock::now()) {
				std::this_thread::sleep_until(end_of_frame_time);
			} else {
				end_of_frame_time = std::chrono::steady_clock::now();
			}
		}

        host->update_screen();

		stats->start_core_time();

		stats->process_parameters();
    }
}

void system_t::switch_to_debug_mode()
{
	debugger->prompt();
	debugger->terminal->activate_cursor();
	current_mode = DEBUG_MODE;
}

void system_t::switch_to_run_mode()
{
	debugger->terminal->deactivate_cursor();
	current_mode = RUN_MODE;
}

void system_t::switch_mode()
{
	if (current_mode == RUN_MODE) {
		switch_to_debug_mode();
	} else {
		switch_to_run_mode();
	}
}
