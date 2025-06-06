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

	terminal = new terminal_t(system, 60, 17, PUNCH_LIGHTBLUE, PUNCH_BLUE);
	terminal->clear();
	print_version();
	terminal->activate_cursor();

	status1 = new terminal_t(system, 58, 20, LIME_COLOR_02, LIME_COLOR_00);
	status2 = new terminal_t(system, 17, 16, LIME_COLOR_02, LIME_COLOR_00);
}

debugger_t::~debugger_t()
{
	delete status2;
	delete status1;
	delete terminal;
}

void debugger_t::redraw()
{
	// clear buffer
	for (int y=0; y<SCREEN_HEIGHT; y++) {
		for (int x=0; x<SCREEN_WIDTH; x++) {
			system->host->video_framebuffer[(y * SCREEN_WIDTH) + x] = PUNCH_BLACK;
		}
	}

	// draw terminal tiles
	for (int y = 0; y < (terminal->height << 3); y++) {
		uint8_t y_in_char = y % 8;
		for (int x = 0; x < (terminal->width << 3); x++) {
			uint8_t symbol = terminal->tiles[((y>>3) * terminal->width) + (x >> 3)];
	 		uint8_t x_in_char = x % 8;
			system->host->video_framebuffer[((y + 176) * SCREEN_WIDTH) + x + 0] =
				(debugger_cbm_font.original_data[(symbol << 3) + y_in_char] & (0b1 << (7 - x_in_char))) ?
				terminal->fg_colors[((y>>3) * terminal->width) + (x >> 3)] :
				terminal->bg_colors[((y>>3) * terminal->width) + (x >> 3)] ;
		}
	}

	// update status text
	status1->clear();
	system->core->cpu->status(text_buffer, 1024);
	status1->printf("__cpu_____________________________________________________%s", text_buffer);
	status1->printf("\n\n__disassembly____________________________\n");
	uint16_t pc = system->core->cpu->get_pc();
	for (int i=0; i<5; i++) {
		pc += disassemble_instruction_status1(pc);
		status1->putchar('\n');
	}

	status1->printf("\n__timer_____s___bpm___cycles__");
	for (int i=0; i<8; i++) {
		status1->printf("\n    %u   %s %s %5u %10u",
			i,
			system->core->timer->io_read_byte(0x01) & (1 << i) ? " on" : "off",
			system->core->timer->io_read_byte(0x00) & (1 << i) ? "*" : "-",
			system->core->timer->get_timer_bpm(i),
			system->core->timer->get_timer_clock_interval(i) - system->core->timer->get_timer_counter(i)
		);
	}

	// draw status1 tiles
	for (int y = 0; y < (status1->height << 3); y++) {
		uint8_t y_in_char = y % 8;
		for (int x = 0; x < (status1->width << 2); x++) {
			uint8_t symbol = status1->tiles[((y>>3) * status1->width) + (x >> 2)];
	 		uint8_t x_in_char = x % 4;
			system->host->video_framebuffer[((y + 8) * SCREEN_WIDTH) + x + 0] =
				(debugger_font.data[(symbol << 3) + y_in_char] & (0b1 << (3 - x_in_char))) ?
				status1->fg_colors[((y>>3) * status1->width) + (x >> 2)] :
				status1->bg_colors[((y>>3) * status1->width) + (x >> 2)] ;
		}
	}

	status2->clear();
	status2->printf(" __usp___ __ssp__");

	for (int i=0; i<4; i++) {
		uint16_t usp = (system->core->cpu->get_us() + i) & 0xffff;
		uint8_t usp_b = system->core->read8(usp);
		uint16_t ssp = (system->core->cpu->get_sp() + i) & 0xffff;
		uint8_t ssp_b = system->core->read8(ssp);

		status2->printf(" %04x %02x  %04x %02x",
			usp,
			usp_b,
			ssp,
			ssp_b
		);
	}
	system->core->exceptions->status(text_buffer, 2048);
	status2->printf("\n%s", text_buffer);

	status2->printf("\n __vdc___________");
	status2->printf(" cycle %i/%i", system->core->vdc->get_cycles_run(), CPU_CYCLES_PER_SCANLINE);
	status2->printf("\n scanl %i/%i", system->core->vdc->get_current_scanline(), VDC_SCANLINES - 1);
	if (system->core->vdc->get_generate_interrupts()) {
		status2->printf("\n   irq %i", system->core->vdc->get_irq_scanline());
	}

	// draw status2 tiles
	for (int y = 0; y < (status2->height << 3); y++) {
		uint8_t y_in_char = y % 8;
		for (int x = 0; x < (status2->width << 2); x++) {
			uint8_t symbol = status2->tiles[((y>>3) * status2->width) + (x >> 2)];
	 		uint8_t x_in_char = x % 4;
			system->host->video_framebuffer[((y + 40) * SCREEN_WIDTH) + x + 164] =
				(debugger_font.data[(symbol << 3) + y_in_char] & (0b1 << (3 - x_in_char))) ?
				status2->fg_colors[((y>>3) * status2->width) + (x >> 2)] :
				status2->bg_colors[((y>>3) * status2->width) + (x >> 2)] ;
		}
	}

	// copy vdc buffer contents into video framebuffer (no need for scanline stuff)
	for (int y = 0; y < VDC_YRES; y++) {
		for (int x = 0; x < VDC_XRES; x++) {
			system->host->video_framebuffer[((y + 8) * SCREEN_WIDTH) + x + 240] =
				system->core->vdc->buffer[(y * VDC_XRES) + x];
		}
	}

	const int16_t arrows[9][2] = {
		{1,-2},
		{1,-1},{2,-1},
		{1, 0},{2, 0},{3,0},
		{1, 1},{2, 1},
		{1, 2}
	};

	if (system->core->vdc->get_current_scanline() < VDC_YRES) {
		for (int i=0; i<9; i++) {
			system->host->video_framebuffer[((8+system->core->vdc->get_current_scanline()+arrows[i][1])*SCREEN_WIDTH) + 239 + arrows[i][0]] = LIME_COLOR_02;
			system->host->video_framebuffer[((8+system->core->vdc->get_current_scanline()+arrows[i][1])*SCREEN_WIDTH) + 0 + (SCREEN_WIDTH-arrows[i][0])] = LIME_COLOR_02;
		}
		system->host->video_framebuffer[
			((system->core->vdc->get_cycles_run()*VDC_XRES/CPU_CYCLES_PER_SCANLINE)+240) +
			((8+system->core->vdc->get_current_scanline()) * SCREEN_WIDTH)
		] = LIME_COLOR_02;
	}

	// progress bar for cycles done for scanline
	for (int x=240; x<(240+VDC_XRES); x++) {
		if (x < ((system->core->vdc->get_cycles_run()*VDC_XRES)/CPU_CYCLES_PER_SCANLINE)+240) {
			system->host->video_framebuffer[(3*SCREEN_WIDTH) + x] = LIME_COLOR_02;
			system->host->video_framebuffer[(4*SCREEN_WIDTH) + x] = LIME_COLOR_02;
		} else {
			system->host->video_framebuffer[(3*SCREEN_WIDTH) + x] = LIME_COLOR_00;
			system->host->video_framebuffer[(4*SCREEN_WIDTH) + x] = LIME_COLOR_00;
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
//			case ASCII_F3:
//				terminal->deactivate_cursor();
//				terminal->printf("run");
//				system->switch_to_run_mode();
//				system->host->events_wait_until_key_released(SDLK_F3);
//				break;
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
	// } else if (token0[0] == '\'') {
	// 	have_prompt = false;
	// 	enter_vram_binary_line(c);
	} else if (token0[0] == ',') {
	 	have_prompt = false;
	 	enter_assembly_line(c);
	} else if (strcmp(token0, "b") == 0) {
		bool breakpoints_present = false;
		token1 = strtok(NULL, " ");
		if (token1 == NULL) {
			terminal->printf("\nbreakpoints:");
			for (int i=0; i<65536; i++) {
				if (system->core->cpu->breakpoint_array[i]) {
					breakpoints_present = true;
					terminal->printf("\n$%04x", i);
				}
			}
			if (!breakpoints_present) terminal->printf("\nnone");
		} else {
			uint32_t address;
			if (!hex_string_to_int(token1, &address)) {
				terminal->printf("\nerror: '%s' is not a hex number", token1);
			} else {
				address &= 0xffff;
				system->core->cpu->toggle_breakpoint(address);
				if (system->core->cpu->breakpoint_array[address]) {
					terminal->printf("\nbreakpoint set at $%04x", address);
				} else {
					terminal->printf("\nbreakpoint removed at $%04x", address);
				}
			}
		}
	} else if (strcmp(token0, "cls") == 0) {
		terminal->clear();
	} else if (strcmp(token0, "d") == 0) {
	 	have_prompt = false;
	 	token1 = strtok(NULL, " ");

	 	uint8_t lines_remaining = terminal->lines_remaining();
	 	if (lines_remaining == 0) lines_remaining = 1;

	 	uint32_t temp_pc = system->core->cpu->get_pc();

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
	} else if (strcmp(token0, "x") == 0) {
		terminal->printf("\nexit punch (y/n)");
		redraw();
		system->host->update_screen();
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

		uint32_t temp_pc = system->core->cpu->get_pc();

		if (token1 == NULL) {
			for (int i=0; i<lines_remaining; i++) {
				terminal->putchar('\n');
				memory_dump(temp_pc);
				temp_pc = (temp_pc + 8) & 0xffff;
			}
		} else {
			if (!hex_string_to_int(token1, &temp_pc)) {
				terminal->printf("\nerror: '%s' is not a hex number", token1);
				have_prompt = true;
			} else {
				for (int i=0; i<lines_remaining; i++) {
					terminal->putchar('\n');
					memory_dump(temp_pc & 0xffff);
					temp_pc = (temp_pc + 8) & 0xffff;
				}
			}
		}
	} else if (strcmp(token0, "g") == 0) {
		have_prompt = false;
		token1 = strtok(NULL, " ");

		uint8_t lines_remaining = terminal->lines_remaining();
		if (lines_remaining == 0) lines_remaining = 1;

		uint32_t temp_pc = system->core->cpu->get_pc();

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
		system->host->update_screen();
		if (system->host->events_yes_no()) {
			system->core->reset();
			system->host->events_wait_until_key_released(SDLK_y);
		} else {
			system->host->events_wait_until_key_released(SDLK_n);
		}
	} else if (strcmp(token0, "run") == 0) {
		have_prompt = false;
		system->switch_to_run_mode();
		system->host->events_wait_until_key_released(SDLK_RETURN);
	} else if (strcmp(token0, "timer") == 0) {
		for (int i=0; i<8; i++) {
			system->core->timer->status(text_buffer, i);
			terminal->printf("%s", text_buffer);
		}
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

void debugger_t::memory_dump(uint16_t address)
{
	address = address & 0xffff;

	uint8_t data[8];

	terminal->printf("\r.:%04x ", address);
	for (int i=0; i<8; i++) {
		data[i] = system->core->read8(address);
		terminal->printf("%02x ", data[i]);
		address++;
		address &= 0xffff;
	}

	terminal->bg_color = LIME_COLOR_00;
	terminal->fg_color = LIME_COLOR_02;

	for (int i=0; i<8; i++) {
		terminal->putsymbol(data[i]);
	}

	terminal->bg_color = PUNCH_BLUE;
	terminal->fg_color = PUNCH_LIGHTBLUE;

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

	buffer[5]  = '\0';
	buffer[8]  = '\0';
	buffer[11] = '\0';
	buffer[14] = '\0';
	buffer[17] = '\0';
	buffer[20] = '\0';
	buffer[23] = '\0';
	buffer[26] = '\0';
	buffer[29] = '\0';

	if (!hex_string_to_int(&buffer[1], &address)) {
		terminal->putchar('\r');
		terminal->cursor_right();
		terminal->cursor_right();
		terminal->puts("????");
	} else if (!hex_string_to_int(&buffer[6], &arg0)) {
		terminal->putchar('\r');
		for (int i=0; i<7; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[9], &arg1)) {
		terminal->putchar('\r');
		for (int i=0; i<10; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[12], &arg2)) {
		terminal->putchar('\r');
		for (int i=0; i<13; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[15], &arg3)) {
		terminal->putchar('\r');
		for (int i=0; i<16; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[18], &arg4)) {
		terminal->putchar('\r');
		for (int i=0; i<19; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[21], &arg5)) {
		terminal->putchar('\r');
		for (int i=0; i<22; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[24], &arg6)) {
		terminal->putchar('\r');
		for (int i=0; i<25; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[27], &arg7)) {
		terminal->putchar('\r');
		for (int i=0; i<28; i++) terminal->cursor_right();
		terminal->puts("??");
	} else {
		uint16_t original_address = address;

		arg0 &= 0xff;
		arg1 &= 0xff;
		arg2 &= 0xff;
		arg3 &= 0xff;
		arg4 &= 0xff;
		arg5 &= 0xff;
		arg6 &= 0xff;
		arg7 &= 0xff;

		system->core->write8(address, (uint8_t)arg0); address +=1; address &= 0xffff;
		system->core->write8(address, (uint8_t)arg1); address +=1; address &= 0xffff;
		system->core->write8(address, (uint8_t)arg2); address +=1; address &= 0xffff;
		system->core->write8(address, (uint8_t)arg3); address +=1; address &= 0xffff;
		system->core->write8(address, (uint8_t)arg4); address +=1; address &= 0xffff;
		system->core->write8(address, (uint8_t)arg5); address +=1; address &= 0xffff;
		system->core->write8(address, (uint8_t)arg6); address +=1; address &= 0xffff;
		system->core->write8(address, (uint8_t)arg7); address +=1; address &= 0xffff;

		terminal->putchar('\r');

		memory_dump(original_address);

		original_address += 8;
		original_address &= 0xffff;
		terminal->printf("\n.:%04x ", original_address);
		have_prompt = false;
	}
}

void debugger_t::memory_binary_dump(uint16_t address)
{
	address = address & 0xffff;
	uint32_t temp_address = address;
	terminal->printf("\r.;%04x ", temp_address);
	uint8_t res[8];
	for (int i=0; i<8; i++) {
		uint16_t temp_address = (address + ((i & 0b100) ? 1 : 0));
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

	buffer[5]  = '\0';
	buffer[8]  = '\0';
	buffer[11] = '\0';
	buffer[14] = '\0';
	buffer[17] = '\0';
	buffer[20] = '\0';
	buffer[23] = '\0';
	buffer[26] = '\0';
	buffer[29] = '\0';

	if (!hex_string_to_int(&buffer[1], &address)) {
		terminal->putchar('\r');
		terminal->cursor_right();
		terminal->cursor_right();
		terminal->puts("????");
	} else if (!binary_string_to_int(&buffer[6], &arg0)) {
		terminal->putchar('\r');
		for (int i=0; i<7; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!binary_string_to_int(&buffer[9], &arg1)) {
		terminal->putchar('\r');
		for (int i=0; i<10; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!binary_string_to_int(&buffer[12], &arg2)) {
		terminal->putchar('\r');
		for (int i=0; i<13; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!binary_string_to_int(&buffer[15], &arg3)) {
		terminal->putchar('\r');
		for (int i=0; i<16; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!binary_string_to_int(&buffer[18], &arg4)) {
		terminal->putchar('\r');
		for (int i=0; i<19; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!binary_string_to_int(&buffer[21], &arg5)) {
		terminal->putchar('\r');
		for (int i=0; i<22; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!binary_string_to_int(&buffer[24], &arg6)) {
		terminal->putchar('\r');
		for (int i=0; i<25; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!binary_string_to_int(&buffer[27], &arg7)) {
		terminal->putchar('\r');
		for (int i=0; i<28; i++) terminal->cursor_right();
		terminal->puts("??");
	} else {
		uint16_t original_address = address;

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
		address &= 0xffff;

		system->core->write8(address, (arg4 << 6) | (arg5 << 4) | (arg6 << 2) | arg7);

		terminal->putchar('\r');

		memory_binary_dump(original_address);

		original_address += 2;
		original_address &= 0xffff;
		terminal->printf("\n.;%04x ", original_address);
		have_prompt = false;
	}
}

uint32_t debugger_t::disassemble_instruction_status1(uint16_t address)
{
	uint32_t cycles;
	if (system->core->cpu->breakpoint_array[address]) {
		status1->fg_color = 0xffe04040;	// orange
	}
	cycles = system->core->cpu->disassemble_instruction(text_buffer, 1024, address) & 0xffff;
	status1->printf("%s", text_buffer);
	status1->fg_color = LIME_COLOR_02;

	return cycles;
}

uint32_t debugger_t::disassemble_instruction_terminal(uint16_t address)
{
	uint32_t cycles;
	cycles = system->core->cpu->disassemble_instruction(text_buffer, 1024, address) & 0xffff;
	terminal->printf(",%s", text_buffer);

	terminal->putchar('\r');
	for (int i=0; i<7; i++) terminal->cursor_right();
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

void debugger_t::enter_assembly_line(char *buffer)
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
