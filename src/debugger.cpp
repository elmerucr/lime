// ---------------------------------------------------------------------
// debugger.cpp
// lime
//
// Copyright © 2025 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include "debugger.hpp"
#include "core.hpp"
#include "keyboard.hpp"
#include <cstdint>

// ---------------------------------------------------------------------
// hex2int
// take a hex string and convert it to a 32bit number (max 8 hex digits)
// from https://stackoverflow.com/questions/10156409/convert-hex-string-char-to-int
//
// This function is slightly adopted to check for true values. It returns false
// when there's wrong input.
// ---------------------------------------------------------------------
bool debugger_t::hex_string_to_int(const char *temp_string, uint32_t *return_value)
{
	uint32_t val = 0;
	while (*temp_string) {
		/* Get current character then increment */
		uint8_t byte = *temp_string++;
		/* Transform hex character to the 4bit equivalent number */
		if (byte >= '0' && byte <= '9') {
			byte = byte - '0';
		} else if (byte >= 'a' && byte <='f') {
			byte = byte - 'a' + 10;
		} else if (byte >= 'A' && byte <='F') {
			byte = byte - 'A' + 10;
		} else {
			/* Problem, return false and do not write the return value */
			return false;
		}
		/* Shift 4 to make space for new digit, and add the 4 bits of the new digit */
		val = (val << 4) | (byte & 0xf);
	}
	*return_value = val;
	return true;
}

bool debugger_t::binary_string_to_int(const char *temp_string, uint32_t *return_value)
{
	uint32_t val = 0;
	while (*temp_string) {
		/* Get current character then increment */
		uint8_t c = *temp_string++;

		if (c == '1') {
			c = 0b1;
		} else if (c == '.') {
			c = 0b0;
		} else {
			/* Problem, return false and do not write the return value */
			return false;
		}
		/* Shift 1 to make space for new digit, and add the 4 bits of the new digit */
		val = (val << 1) | c;
	}
	*return_value = val;
	return true;
}

debugger_t::debugger_t(system_t *s)
{
	system = s;

	terminal = new terminal_t(system, 60, 14, PUNCH_LIGHTBLUE, PUNCH_BLUE);
	terminal->clear();
	print_version();
	terminal->activate_cursor();

	status1 = new terminal_t(system, 60, 24, PICOTRON_V5_1A, LIME_COLOR_00);
	exception_status = new terminal_t(system, 20, 5, PICOTRON_V5_1A, 0xff000000);
	vdc_status = new terminal_t(system, 16, 6, PICOTRON_V5_1A, 0xff000000);
}

debugger_t::~debugger_t()
{
	delete vdc_status;
	delete exception_status;
	delete status1;
	delete terminal;
}

void debugger_t::redraw()
{
	// clear buffer
	for (int y=0; y<SCREEN_HEIGHT; y++) {
		for (int x=0; x<SCREEN_WIDTH; x++) {
			system->host->video_framebuffer[(y * SCREEN_WIDTH) + x] = PUNCH_LIGHTBLUE;
		}
	}

	// draw terminal tiles
	for (int y = 0; y < (terminal->height << 3); y++) {
		uint8_t y_in_char = y % 8;
		for (int x = 0; x < (terminal->width << 3); x++) {
			uint8_t symbol = terminal->tiles[((y>>3) * terminal->width) + (x >> 3)];
	 		uint8_t x_in_char = x % 8;
			system->host->video_framebuffer[((y + 200) * SCREEN_WIDTH) + x + 0] =
				(debugger_cbm_font.original_data[(symbol << 3) + y_in_char] & (0b1 << (7 - x_in_char))) ?
				terminal->fg_colors[((y>>3) * terminal->width) + (x >> 3)] :
				terminal->bg_colors[((y>>3) * terminal->width) + (x >> 3)] ;
		}
	}

	// update status text
	status1->clear();
	if (system->core->mc68000_active) {
		system->core->mc68000->disassembleSR(text_buffer);
		uint32_t isp = system->core->mc68000->getISP();
		uint32_t usp = system->core->mc68000->getUSP();
		status1->printf(
			"-----------------------Motorola 68000-----------------------"
			"   D0:%08x   D4:%08x    A0:%08x   A4:%08x\n"
			"   D1:%08x   D5:%08x    A1:%08x   A5:%08x\n"
			"   D2:%08x   D6:%08x    A2:%08x   A6:%08x\n"
			"   D3:%08x   D7:%08x    A3:%08x   A7:%08x\n\n"
			"     PC:%08x    SR:%04x (%s)    IPL:%i\n\n"
			"    SSP:%08x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x\n"
			"    USP:%08x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x\n",

			system->core->mc68000->getD(0),
			system->core->mc68000->getD(4),
			system->core->mc68000->getA(0),
			system->core->mc68000->getA(4),

			system->core->mc68000->getD(1),
			system->core->mc68000->getD(5),
			system->core->mc68000->getA(1),
			system->core->mc68000->getA(5),

			system->core->mc68000->getD(2),
			system->core->mc68000->getD(6),
			system->core->mc68000->getA(2),
			system->core->mc68000->getA(6),

			system->core->mc68000->getD(3),
			system->core->mc68000->getD(7),
			system->core->mc68000->getA(3),
			system->core->mc68000->getA(7),

			system->core->mc68000->getPC(),
			system->core->mc68000->getSR(),
			text_buffer,
			system->core->mc68000->getIPL(),

			isp,
			system->core->read8(isp+0), system->core->read8(isp+1),
			system->core->read8(isp+2), system->core->read8(isp+3),
			system->core->read8(isp+4), system->core->read8(isp+5),
			system->core->read8(isp+6), system->core->read8(isp+7),
			system->core->read8(isp+8), system->core->read8(isp+9),
			system->core->read8(isp+10), system->core->read8(isp+11),
			system->core->read8(isp+12), system->core->read8(isp+13),
			system->core->read8(isp+14), system->core->read8(isp+15),

			usp,
			system->core->read8(usp+0), system->core->read8(usp+1),
			system->core->read8(usp+2), system->core->read8(usp+3),
			system->core->read8(usp+4), system->core->read8(usp+5),
			system->core->read8(usp+6), system->core->read8(usp+7),
			system->core->read8(usp+8), system->core->read8(usp+9),
			system->core->read8(usp+10), system->core->read8(usp+11),
			system->core->read8(usp+12), system->core->read8(usp+13),
			system->core->read8(usp+14), system->core->read8(usp+15)
		);

		status1->printf(
			"\n------------------------disassembler------------------------"
		);
		uint32_t pc = system->core->mc68000->getPC();
		uint32_t new_pc;
		for (int i=0; i<5; i++) {
			new_pc = pc + system->core->mc68000->disassemble(text_buffer, pc);
			if (system->core->mc68000->debugger.breakpoints.isSetAt(pc)) {
				status1->fg_color = 0xffe04040;	// orange
			}
			if (m68k_disassembly) {
				status1->printf(",%06x  %s\n", pc, text_buffer);
			} else {
				status1->printf(",%06x  ", pc);
				for (int i=pc; i < new_pc; i++) {
					status1->printf("%02x", system->core->read8(i));
				}
				status1->printf("\n");
			}
			status1->fg_color = PICOTRON_V5_1A;
			pc = new_pc;
		}
	} else {
		uint16_t ssp = system->core->mc6809->get_sp() & 0xffff;
		uint16_t usp = system->core->mc6809->get_us() & 0xffff;

		system->core->mc6809->status(text_buffer, 1024);
		status1->printf("-----------------------Motorola 6809------------------------%s", text_buffer);
		status1->printf(
			"\n\n      system stack: %04x %02x %02x %02x %02x %02x %02x %02x %02x",
			ssp, system->core->read8(ssp), system->core->read8(ssp+1), system->core->read8(ssp+2),
			system->core->read8(ssp+3), system->core->read8(ssp+4), system->core->read8(ssp+5),
			system->core->read8(ssp+6), system->core->read8(ssp+7)
		);
		status1->printf(
			"\n        user stack: %04x %02x %02x %02x %02x %02x %02x %02x %02x",
			usp, system->core->read8(usp), system->core->read8(usp+1), system->core->read8(usp+2),
			system->core->read8(usp+3), system->core->read8(usp+4), system->core->read8(usp+5),
			system->core->read8(usp+6), system->core->read8(usp+7)
		);
		status1->printf("\n\n------------------------disassembler------------------------");
		uint16_t pc = system->core->mc6809->get_pc();
		for (int i=0; i<9; i++) {
			pc += disassemble_instruction_status1(pc);
			status1->putchar('\n');
		}
	}

		status1->printf("\n--------timer---------\nt     s  bpm   cycles  ");
		for (int i=0; i<4; i++) {
			int j = i + (timers_4_7 ? 4 : 0);
			status1->printf("\n%1x %s %s %05u %08x",
				j,
				system->core->timer->io_read_byte(0x01) & (1 << j) ? " on" : "off",
				system->core->timer->io_read_byte(0x00) & (1 << j) ? "*" : "-",
				system->core->timer->get_timer_bpm(j),
				system->core->timer->get_timer_clock_interval(j) - system->core->timer->get_timer_counter(j)
			);
		}

	exception_status->clear();

	if (system->core->mc68000_active) {
		system->core->sn74ls148->status(text_buffer, 2048);
	} else {
		system->core->exceptions->status(text_buffer, 2048);
	}
	exception_status->printf("%s", text_buffer);

	// copy exception_status into status1
	for (int y = 0; y < exception_status->height; y++) {
		for (int x = 0; x < exception_status->width; x++) {
			status1->tiles[((18 + y) * status1->width) + 23 + x] =
				exception_status->tiles[(y * exception_status->width) + x];
		}
	}

	vdc_status->clear();
	vdc_status->printf("----vdc/cpu-----");
	vdc_status->printf("scanline %3i/%3i  cycles %3i/%3i",
		system->core->vdc->get_current_scanline(),
		VDC_SCANLINES - 1,
		(system->core->vdc->get_cycles_run() << system->core->cpu_multiplier) + system->core->cpu_to_core_clock->get_mod(),
		CORE_CYCLES_PER_SCANLINE << system->core->cpu_multiplier
	);
	vdc_status->printf("\n rasterline %3i\n        irq %s",
		system->core->vdc->get_irq_scanline(),
		system->core->vdc->get_generate_interrupts() ? "yes" : " no"
	);

	// copy vdc_status into status1
	for (int y = 0; y < vdc_status->height; y++) {
		for (int x = 0; x < vdc_status->width; x++) {
			status1->tiles[((18 + y) * status1->width) + 43 + x] =
				vdc_status->tiles[(y * vdc_status->width) + x];
		}
	}

	// draw status1 (including chars of exception_status)
	for (int y = 0; y < (status1->height << 3); y++) {
		uint8_t y_in_char = y % 8;
		for (int x = 0; x < (status1->width << 3); x++) {
			uint8_t symbol = status1->tiles[((y>>3) * status1->width) + (x >> 3)];
	 		uint8_t x_in_char = x % 8;
			system->host->video_framebuffer[((y + 0) * SCREEN_WIDTH) + x + 0] =
				(debugger_cbm_font.original_data[(symbol << 3) + y_in_char] & (0b1 << (7 - x_in_char))) ?
				status1->fg_colors[((y>>3) * status1->width) + (x >> 3)] :
				status1->bg_colors[((y>>3) * status1->width) + (x >> 3)] ;
		}
	}

	for (int i=0; i<(VDC_XRES*VDC_YRES); i++) {
		system->host->video_viewer_framebuffer[i] = system->core->vdc->buffer[i];
	}
	// highlight scanline being drawn + dot for current pixel
	int y = system->core->vdc->get_current_scanline();
	if (y < VDC_YRES) {
		for (int x=0; x<VDC_XRES; x++) {
			system->host->video_viewer_framebuffer[(y * VDC_XRES) + x] = LIME_COLOR_01;
		}
		system->host->video_viewer_framebuffer[
			(system->core->vdc->get_cycles_run()*VDC_XRES/CORE_CYCLES_PER_SCANLINE) + (y * VDC_XRES)
		] = LIME_COLOR_03;
		if (y > 0) {
			system->host->video_viewer_framebuffer[
				(system->core->vdc->get_cycles_run()*VDC_XRES/CORE_CYCLES_PER_SCANLINE) + ((y - 1) * VDC_XRES)
			] = LIME_COLOR_03;
		}
		if (y < (VDC_YRES - 1)) {
			system->host->video_viewer_framebuffer[
				(system->core->vdc->get_cycles_run()*VDC_XRES/CORE_CYCLES_PER_SCANLINE) + ((y + 1) * VDC_XRES)
			] = LIME_COLOR_03;
		}
	}
}

void debugger_t::prompt()
{
	terminal->printf("\n.");
}

void debugger_t::run()
{
	uint8_t symbol = 0;

	terminal->process_cursor_state();

	while (system->keyboard->events_waiting()) {
		terminal->deactivate_cursor();
		symbol = system->keyboard->pop_event();
		switch (symbol) {
			case ASCII_F1:
				system->core->run(true);
				break;
			case ASCII_F2:
				do {
					system->core->run(true);
				} while (!system->core->vdc->started_new_scanline());
				break;
			case ASCII_F3:
				timers_4_7 = !timers_4_7;
				break;
			case ASCII_F4:
				m68k_disassembly = !m68k_disassembly;
				break;
			case ASCII_CURSOR_LEFT:
				terminal->cursor_left();
				break;
			case ASCII_CURSOR_RIGHT:
				terminal->cursor_right();
				break;
			case ASCII_CURSOR_UP:
				terminal->cursor_up();
				break;
			case ASCII_CURSOR_DOWN:
				terminal->cursor_down();
				break;
			case ASCII_BACKSPACE:
				terminal->backspace();
				break;
			case ASCII_LF:
				// TODO: supply textbuffer here to isolate command?
				char command_buffer[256];
				terminal->get_command(command_buffer, 256);
				process_command(command_buffer);
				break;
			default:
				terminal->putchar(symbol);
				break;
		}
		terminal->activate_cursor();
	}
}

void debugger_t::process_command(char *c)
{
	int cnt = 0;
	while ((*c == ' ') || (*c == '.')) {
		c++;
		cnt++;
	}

	have_prompt = true;

	char *token0, *token1, *token2;
	token0 = strtok(c, " ");

	if (token0 == NULL) {
		//have_prompt = false;
	} else if (token0[0] == ':') {
		have_prompt = false;
		enter_memory_line(c);
	} else if (token0[0] == ';') {
		have_prompt = false;
		enter_memory_binary_line(c);
	} else if (token0[0] == ',') {
		if (!system->core->mc68000_active) {
			// mc6809 mode
			have_prompt = false;
			enter_mc6809_assembly_line(c);
		}
	} else if (strcmp(token0, "b") == 0) {
		bool breakpoints_present = false;
		token1 = strtok(NULL, " ");
		if (token1 == NULL) {
			terminal->printf("\nbreakpoints:");
			if (system->core->mc68000_active) {
				for (uint32_t i=0x000000; i<0x1000000; i++) {
					if (system->core->mc68000->debugger.breakpoints.isSetAt(i)) {
						breakpoints_present = true;
						terminal->printf("\n$%06x", i);
					}
				}
			} else {
				for (int i=0; i<65536; i++) {
					if (system->core->mc6809->breakpoint_array[i]) {
						breakpoints_present = true;
						terminal->printf("\n$%04x", i);
					}
				}
			}
			if (!breakpoints_present) {
				terminal->printf("\nnone");
			}
		} else {
			uint32_t address;
			if (!hex_string_to_int(token1, &address)) {
				terminal->printf("\nerror: '%s' is not a hex number", token1);
			} else {
				if (system->core->mc68000_active) {
					address &= 0xffffff;
					if (system->core->mc68000->debugger.breakpoints.isSetAt(address)) {
						system->core->mc68000->debugger.breakpoints.removeAt(address);
						terminal->printf("\nbreakpoint removed at $%06x", address);
					} else {
						system->core->mc68000->debugger.breakpoints.setAt(address);
						terminal->printf("\nbreakpoint set at $%06x", address);
					}
				} else {
					address &= 0xffff;
					system->core->mc6809->toggle_breakpoint(address);
					if (system->core->mc6809->breakpoint_array[address]) {
						terminal->printf("\nbreakpoint set at $%04x", address);
					} else {
						terminal->printf("\nbreakpoint removed at $%04x", address);
					}
			}
			}
		}
	} else if (strcmp(token0, "cls") == 0) {
		terminal->clear();
	} else if (strcmp(token0, "cpu") == 0) {
		terminal->printf("\nchange cpu and reset lime (y/n)");
		redraw();
		system->host->video_update_screen();
		if (system->host->events_yes_no()) {
			system->core->mc68000_active = !system->core->mc68000_active;
			if (system->core->mc68000_active) {
				terminal->printf("\nmc68000 mode");
			} else {
				terminal->printf("\nmc6809 mode");
			}
			system->core->reset();
			system->host->events_wait_until_key_released(SDLK_y);
		} else {
			system->host->events_wait_until_key_released(SDLK_n);
		}
	} else if (strcmp(token0, "d") == 0) {
	 	have_prompt = false;
	 	token1 = strtok(NULL, " ");

	 	uint8_t lines_remaining = terminal->lines_remaining();
	 	if (lines_remaining == 0) lines_remaining = 1;

	 	uint32_t temp_pc;

		if (system->core->mc68000_active) {
			temp_pc = system->core->mc68000->getPC();
		} else {
			temp_pc = system->core->mc6809->get_pc();
		}

	 	if (token1 == NULL) {
	 		for (int i=0; i<lines_remaining; i++) {
	 			terminal->printf("\n.");
	 			temp_pc += disassemble_instruction_terminal(temp_pc);
	 		}
	 	} else {
	 		if (!hex_string_to_int(token1, &temp_pc)) {
	 			terminal->printf("\nerror: '%s' is not a hex number", token1);
	 			have_prompt = true;
	 		} else {
	 			for (int i=0; i<lines_remaining; i++) {
	 				terminal->printf("\n.");
	 				temp_pc += disassemble_instruction_terminal(temp_pc);
	 			}
	 		}
	 	}
	} else if (strcmp(token0, "v") == 0) {
		system->host->viewer_visible = !system->host->viewer_visible;
	} else if (strcmp(token0, "x") == 0) {
		terminal->printf("\nexit lime (y/n)");
		redraw();
		system->host->video_update_screen();
		if (system->host->events_yes_no()) {
			system->running = false;
			system->host->events_wait_until_key_released(SDLK_y);
			have_prompt = false;
		} else {
			system->host->events_wait_until_key_released(SDLK_n);
		}
	} else if (strcmp(token0, "m") == 0) {
		have_prompt = false;
		token1 = strtok(NULL, " ");

		uint8_t lines_remaining = terminal->lines_remaining();
		if (lines_remaining == 0) lines_remaining = 1;

		uint32_t temp_pc;

		if (system->core->mc68000_active) {
			temp_pc = system->core->mc68000->getPC();
		} else {
			temp_pc = system->core->mc6809->get_pc();
		}

		if (token1 == NULL) {
			for (int i=0; i<lines_remaining; i++) {
				terminal->putchar('\n');
				memory_dump(temp_pc);
				temp_pc = (temp_pc + 8) & 0xfffffe;
			}
		} else {
			if (!hex_string_to_int(token1, &temp_pc)) {
				terminal->printf("\nerror: '%s' is not a hex number", token1);
				have_prompt = true;
			} else {
				for (int i=0; i<lines_remaining; i++) {
					terminal->putchar('\n');
					memory_dump(temp_pc & 0xfffffe);
					temp_pc = (temp_pc + 8) & 0xfffffe;
				}
			}
		}
	} else if (strcmp(token0, "g") == 0) {
		have_prompt = false;
		token1 = strtok(NULL, " ");

		uint8_t lines_remaining = terminal->lines_remaining();
		if (lines_remaining == 0) lines_remaining = 1;

		uint32_t temp_pc;

		if (system->core->mc68000_active) {
			temp_pc = system->core->mc68000->getPC();
		} else {
			temp_pc = system->core->mc6809->get_pc();
		}

		if (token1 == NULL) {
			for (int i=0; i<lines_remaining; i++) {
				terminal->putchar('\n');
				memory_binary_dump(temp_pc);
				temp_pc = (temp_pc + 2) & 0xffff;
			}
		} else {
			if (!hex_string_to_int(token1, &temp_pc)) {
				terminal->printf("\nerror: '%s' is not a hex number", token1);
				have_prompt = true;
			} else {
				for (int i=0; i<lines_remaining; i++) {
					terminal->putchar('\n');
					memory_binary_dump(temp_pc & 0xffff);
					temp_pc = (temp_pc + 2) & 0xffff;
				}
			}
		}
	} else if (strcmp(token0, "dgc") == 0) {
		have_prompt = false;
		enter_dgc_line(c);
	} else if (strcmp(token0, "n") == 0) {
		system->core->run(true);
	} else if (strcmp(token0, "pal") == 0) {
		token1 = strtok(NULL, " ");

		uint32_t temp_no;

		if (token1 == NULL) {
			temp_no = 0x00;
		} else {
			if (!hex_string_to_int(token1, &temp_no)) {
				temp_no = 0x00;
			} else {
				temp_no &= 0xf8;
			}
	}

		for (int i=temp_no; i<(temp_no + 0x40); i++) {
			if ((i & 0b111) == 0) terminal->printf("\n %02x ", i & 0xff);
			terminal->bg_color = system->core->vdc->palette[i & 0xff];
			terminal->printf("  ");
			terminal->bg_color = PUNCH_BLUE;
		}
		terminal->printf(
			"\n\ndgc  %02x  %02x  %02x  %02x\n    ",
			terminal_graphics_colors[0],
			terminal_graphics_colors[1],
			terminal_graphics_colors[2],
			terminal_graphics_colors[3]
		);
		for (int i=0; i<4; i++) {
			terminal->bg_color = system->core->vdc->palette[terminal_graphics_colors[i]];
			terminal->printf("    ");
		}
		terminal->bg_color = PUNCH_BLUE;
	} else if (strcmp(token0, "reset") == 0) {
		terminal->printf("\nreset lime (y/n)");
		redraw();
		system->host->video_update_screen();
		if (system->host->events_yes_no()) {
			system->core->reset();
			system->host->events_wait_until_key_released(SDLK_y);
		} else {
			system->host->events_wait_until_key_released(SDLK_n);
		}
	} else if (strcmp(token0, "run") == 0) {
		have_prompt = false;
		system->host->events_wait_until_key_released(SDLK_RETURN);
		system->switch_to_run_mode();
	} else if (strcmp(token0, "ver") == 0) {
		print_version();
	} else {
		terminal->printf("\r");
		for (int i=0; i<strlen(token0) + cnt; i++) terminal->cursor_right();
		terminal->printf("?");
	}

	if (have_prompt) prompt();
}

void debugger_t::print_version()
{
	terminal->printf("\nlime %i.%i.%i (C)%i elmerucr",
	       LIME_MAJOR_VERSION,
	       LIME_MINOR_VERSION,
	       LIME_BUILD, LIME_YEAR);
}

void debugger_t::memory_dump(uint32_t address)
{
	address = address & 0xfffffe;

	uint8_t data[8];

	terminal->printf("\r.:%06x ", address);
	for (int i=0; i<8; i++) {
		data[i] = system->core->read8(address);
		terminal->printf("%02x ", data[i]);
		address++;
		address &= 0xffffff;
	}

	//terminal->bg_color = LIME_COLOR_00;
	//terminal->fg_color = PICOTRON_V5_1A;

	for (int i=0; i<8; i++) {
		terminal->putsymbol(data[i]);
	}

	//terminal->bg_color = PUNCH_BLUE;
	//terminal->fg_color = PUNCH_LIGHTBLUE;

	for (int i=0; i<32; i++) {
		terminal->cursor_left();
	}
}

void debugger_t::enter_memory_line(char *buffer)
{
	have_prompt = true;

	uint32_t address;
	uint32_t arg0, arg1, arg2, arg3;
	uint32_t arg4, arg5, arg6, arg7;

	buffer[7]  = '\0';
	buffer[10]  = '\0';
	buffer[13] = '\0';
	buffer[16] = '\0';
	buffer[19] = '\0';
	buffer[22] = '\0';
	buffer[25] = '\0';
	buffer[28] = '\0';
	buffer[31] = '\0';

	if (!hex_string_to_int(&buffer[1], &address)) {
		terminal->putchar('\r');
		terminal->cursor_right();
		terminal->cursor_right();
		terminal->puts("??????");
	} else if (!hex_string_to_int(&buffer[8], &arg0)) {
		terminal->putchar('\r');
		for (int i=0; i<9; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[11], &arg1)) {
		terminal->putchar('\r');
		for (int i=0; i<12; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[14], &arg2)) {
		terminal->putchar('\r');
		for (int i=0; i<15; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[17], &arg3)) {
		terminal->putchar('\r');
		for (int i=0; i<18; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[20], &arg4)) {
		terminal->putchar('\r');
		for (int i=0; i<21; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[23], &arg5)) {
		terminal->putchar('\r');
		for (int i=0; i<24; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[26], &arg6)) {
		terminal->putchar('\r');
		for (int i=0; i<27; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[29], &arg7)) {
		terminal->putchar('\r');
		for (int i=0; i<30; i++) terminal->cursor_right();
		terminal->puts("??");
	} else {
		address &= 0xfffffe;
		uint32_t original_address = address;

		arg0 &= 0xff;
		arg1 &= 0xff;
		arg2 &= 0xff;
		arg3 &= 0xff;
		arg4 &= 0xff;
		arg5 &= 0xff;
		arg6 &= 0xff;
		arg7 &= 0xff;

		system->core->write8(address, (uint8_t)arg0); address +=1; address &= 0xffffff;
		system->core->write8(address, (uint8_t)arg1); address +=1; address &= 0xffffff;
		system->core->write8(address, (uint8_t)arg2); address +=1; address &= 0xffffff;
		system->core->write8(address, (uint8_t)arg3); address +=1; address &= 0xffffff;
		system->core->write8(address, (uint8_t)arg4); address +=1; address &= 0xffffff;
		system->core->write8(address, (uint8_t)arg5); address +=1; address &= 0xffffff;
		system->core->write8(address, (uint8_t)arg6); address +=1; address &= 0xffffff;
		system->core->write8(address, (uint8_t)arg7); address +=1; address &= 0xffffff;

		terminal->putchar('\r');

		memory_dump(original_address);

		original_address += 8;
		original_address &= 0xffffff;
		terminal->printf("\n.:%06x ", original_address);
		have_prompt = false;
	}
}

void debugger_t::memory_binary_dump(uint32_t address)
{
	address = address & 0xfffffe;
	uint32_t temp_address = address;
	terminal->printf("\r.;%06x ", temp_address);
	uint8_t res[8];
	for (int i=0; i<8; i++) {
		uint32_t temp_address = (address + ((i & 0b100) ? 1 : 0));
		uint8_t v = system->core->read8(temp_address);
		v >>= ((3 - (i & 0b11)) << 1);
		v &= 0b11;
		res[i] = v;
	}
	for (int i=0; i<8; i++) {
		terminal->printf("%c%c ", res[i] & 0b10 ? '1' : '.', res[i] & 0b1 ? '1' : '.');
	}
	for (int i=0; i<8; i++) {
		terminal->bg_color = system->core->vdc->palette[terminal_graphics_colors[res[i]]];
		terminal->printf(" ");
	}
	terminal->bg_color = PUNCH_BLUE;

	for (int i=0; i<32; i++) {
		terminal->cursor_left();
	}
}

void debugger_t::enter_memory_binary_line(char *buffer)
{
	have_prompt = true;

	uint32_t address;
	uint32_t arg0, arg1, arg2, arg3;
	uint32_t arg4, arg5, arg6, arg7;

	buffer[7]  = '\0';
	buffer[10]  = '\0';
	buffer[13] = '\0';
	buffer[16] = '\0';
	buffer[19] = '\0';
	buffer[22] = '\0';
	buffer[25] = '\0';
	buffer[28] = '\0';
	buffer[31] = '\0';

	if (!hex_string_to_int(&buffer[1], &address)) {
		terminal->putchar('\r');
		terminal->cursor_right();
		terminal->cursor_right();
		terminal->puts("??????");
	} else if (!binary_string_to_int(&buffer[8], &arg0)) {
		terminal->putchar('\r');
		for (int i=0; i<9; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!binary_string_to_int(&buffer[11], &arg1)) {
		terminal->putchar('\r');
		for (int i=0; i<12; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!binary_string_to_int(&buffer[14], &arg2)) {
		terminal->putchar('\r');
		for (int i=0; i<15; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!binary_string_to_int(&buffer[17], &arg3)) {
		terminal->putchar('\r');
		for (int i=0; i<18; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!binary_string_to_int(&buffer[20], &arg4)) {
		terminal->putchar('\r');
		for (int i=0; i<21; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!binary_string_to_int(&buffer[23], &arg5)) {
		terminal->putchar('\r');
		for (int i=0; i<24; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!binary_string_to_int(&buffer[26], &arg6)) {
		terminal->putchar('\r');
		for (int i=0; i<27; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!binary_string_to_int(&buffer[29], &arg7)) {
		terminal->putchar('\r');
		for (int i=0; i<30; i++) terminal->cursor_right();
		terminal->puts("??");
	} else {
		uint32_t original_address = address;

		arg0 &= 0b11;
		arg1 &= 0b11;
		arg2 &= 0b11;
		arg3 &= 0b11;
		arg4 &= 0b11;
		arg5 &= 0b11;
		arg6 &= 0b11;
		arg7 &= 0b11;

		system->core->write8(address, (arg0 << 6) | (arg1 << 4) | (arg2 << 2) | arg3);
		address +=1;
		address &= 0xffffff;

		system->core->write8(address, (arg4 << 6) | (arg5 << 4) | (arg6 << 2) | arg7);

		terminal->putchar('\r');

		memory_binary_dump(original_address);

		original_address += 2;
		original_address &= 0xfffffe;
		terminal->printf("\n.;%06x ", original_address);
		have_prompt = false;
	}
}

uint32_t debugger_t::disassemble_instruction_status1(uint16_t address)
{
	uint32_t cycles;
	if (system->core->mc6809->breakpoint_array[address]) {
		status1->fg_color = 0xffe04040;	// orange
	}
	cycles = system->core->mc6809->disassemble_instruction(text_buffer, 1024, address) & 0xffff;
	status1->printf(",%s", text_buffer);
	status1->fg_color = PICOTRON_V5_1A;

	return cycles;
}

uint32_t debugger_t::disassemble_instruction_terminal(uint32_t address)
{
	uint32_t cycles;

	if (system->core->mc68000_active) {
		cycles = system->core->mc68000->disassemble(text_buffer, address);
		terminal->printf(",%06x  %s", address, text_buffer);

		terminal->putchar('\r');
		for (int i=0; i<10; i++) terminal->cursor_right();
	} else {
		cycles = system->core->mc6809->disassemble_instruction(text_buffer, 1024, address & 0xffff);
		terminal->printf(",%s", text_buffer);

		terminal->putchar('\r');
		for (int i=0; i<7; i++) terminal->cursor_right();
	}

	return cycles;
}

void debugger_t::enter_dgc_line(char *buffer)
{
	have_prompt = true;

	uint32_t arg0, arg1, arg2, arg3;

	buffer[7] 	= '\0';
	buffer[11]	= '\0';
	buffer[15]	= '\0';
	buffer[19]	= '\0';

	if (!hex_string_to_int(&buffer[5], &arg0)) {
		terminal->putchar('\r');
		for (int i=0; i<5; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[9], &arg1)) {
		terminal->putchar('\r');
		for (int i=0; i<9; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[13], &arg2)) {
		terminal->putchar('\r');
		for (int i=0; i<13; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[17], &arg3)) {
		terminal->putchar('\r');
		for (int i=0; i<17; i++) terminal->cursor_right();
		terminal->puts("??");
	} else {
		arg0 &= 0xff;
		arg1 &= 0xff;
		arg2 &= 0xff;
		arg3 &= 0xff;

		terminal_graphics_colors[0] = arg0;
		terminal_graphics_colors[1] = arg1;
		terminal_graphics_colors[2] = arg2;
		terminal_graphics_colors[3] = arg3;

		terminal->printf("\n    ");
		//for (int i=0; i<16; i++) terminal->cursor_right();
		for (int i=0; i<4; i++) {
			terminal->bg_color = system->core->vdc->palette[terminal_graphics_colors[i]];
			terminal->printf("    ");
		}
		terminal->bg_color = PUNCH_BLUE;
	}
}

void debugger_t::enter_mc6809_assembly_line(char *buffer)
{
	uint32_t word;
	uint32_t address;

	buffer[5] = '\0';

	if (!hex_string_to_int(&buffer[1], &word)) {
		// not a valid address
		terminal->putchar('\r');
		terminal->cursor_right();
		terminal->cursor_right();
		terminal->puts("????");
		have_prompt = true;
	} else {
		// valid address
		address = word;

		// prepare for reading arguments (1 to 5)
		uint8_t count{0};
		char old_char;

		for (int i=0; i<5; i++) {
			old_char = buffer[8 + (2 * i)];
			buffer[8 + (2 * i)] = '\0';
			if (hex_string_to_int(&buffer[6 + (2 * i)], &word)) {
				system->core->write8((address + i) & 0xffff, word & 0xff);
				count++;
				buffer[8 + (2 * i)] = old_char;
			} else break;
		}

		if (count) {
			terminal->printf("\r.");
			uint8_t no = disassemble_instruction_terminal(address);
			terminal->printf("\n.,%04x ", (address + no) & 0xffff);
		} else {
			terminal->printf("\n.");
		}
	}
}
