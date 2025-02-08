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

	buffer = new uint32_t[DEBUGGER_WIDTH * DEBUGGER_HEIGHT];

	tiles = new uint8_t[(DEBUGGER_WIDTH >> 2) * (DEBUGGER_HEIGHT >> 3)];
	fg_colors = new uint32_t[(DEBUGGER_WIDTH >> 2) * (DEBUGGER_HEIGHT >> 3)];
	bg_colors = new uint32_t[(DEBUGGER_WIDTH >> 2) * (DEBUGGER_HEIGHT >> 3)];

	for (int i=0; i<(DEBUGGER_WIDTH >> 2) * (DEBUGGER_HEIGHT >> 3); i++) {
		tiles[i] = ' ';
		fg_colors[i] = C64_BLUE;
		bg_colors[i] = C64_LIGHTBLUE;
	}

	terminal = new terminal_t(system, 54, 20, C64_LIGHTBLUE, C64_BLUE);
	terminal->clear();
	print_version();
	terminal->activate_cursor();

	status = new terminal_t(system, 116, 17, LIME_COLOR_2, LIME_COLOR_0);
}

debugger_t::~debugger_t()
{
	delete status;
	delete terminal;
	delete [] bg_colors;
	delete [] fg_colors;
	delete [] tiles;
	delete [] buffer;
}

void debugger_t::redraw()
{
	// copy terminal tiles into tiles buffer
	for (int y = 0; y<terminal->height; y++) {
		for (int x = 0; x<terminal->width; x++) {
			tiles[(y+1)*(DEBUGGER_WIDTH>>2) + x + 2] = terminal->tiles[(y*terminal->width)+x];
			fg_colors[(y+1)*(DEBUGGER_WIDTH>>2) + x + 2] = terminal->fg_colors[(y*terminal->width)+x];
			bg_colors[(y+1)*(DEBUGGER_WIDTH>>2) + x + 2] = terminal->bg_colors[(y*terminal->width)+x];
		}
	}

	// redraw status
	status->clear();
	system->core->cpu->status(text_buffer, 1024);
	status->printf("__cpu_______________________________________________________\n%s", text_buffer);
	status->printf("\n\n__disassembly_______________________________________________");
	uint16_t pc = system->core->cpu->get_pc();
	for (int i=0; i<5; i++) {
		status->putchar('\n');
		pc += disassemble_instruction(pc);
	}
	status->printf("\n\n__vdc_______________________________________________________");
	status->printf("\ndrawing scanline: %3i", system->core->vdc->get_current_scanline());
	status->printf("\n   next scanline: %3i in ", system->core->vdc->get_next_scanline());
	status->printf("%3i cpu cycles", CPU_CYCLES_PER_SCANLINE - system->core->vdc->get_cycles_run());

	// copy status tiles into tiles buffer
	for (int y = 0; y<status->height; y++) {
		for (int x = 0; x<status->width; x++) {
			tiles[(y+22)*(DEBUGGER_WIDTH>>2) + x + 2] = status->tiles[(y*status->width)+x];
			fg_colors[(y+22)*(DEBUGGER_WIDTH>>2) + x + 2] = status->fg_colors[(y*status->width)+x];
			bg_colors[(y+22)*(DEBUGGER_WIDTH>>2) + x + 2] = status->bg_colors[(y*status->width)+x];
		}
	}

	// update buffer
	for (int y=0; y<DEBUGGER_HEIGHT; y++) {
		uint8_t y_in_char = y % 8;
		for (int x=0; x<DEBUGGER_WIDTH; x++) {
			uint8_t symbol = tiles[((y>>3) * (DEBUGGER_WIDTH >> 2)) + (x >> 2)];
			uint8_t x_in_char = x % 4;
			buffer[(y * DEBUGGER_WIDTH) + x] =
				(debugger_font.data[(symbol << 3) + y_in_char] & (0b1 << (3 - x_in_char))) ?
				fg_colors[((y>>3) * (DEBUGGER_WIDTH >> 2)) + (x >> 2)] :
				bg_colors[((y>>3) * (DEBUGGER_WIDTH >> 2)) + (x >> 2)] ;
		}
	}

	const int16_t arrows[16][2] = {
		{0,-3},
		{0,-2},{1,-2},
		{0,-1},{1,-1},{2,-1},
		{0, 0},{1, 0},{2, 0},{3,0},
		{0, 1},{1, 1},{2, 1},
		{0, 2},{1, 2},
		{0, 3}
	};

	if (system->core->vdc->get_current_scanline() < VIDEO_YRES) {
		for (int i=0; i<16; i++) {
			buffer[((8+system->core->vdc->get_current_scanline()+arrows[i][1])*DEBUGGER_WIDTH) + 227 + arrows[i][0]] = LIME_COLOR_2;
			buffer[((8+system->core->vdc->get_current_scanline()+arrows[i][1])*DEBUGGER_WIDTH) + -4 + (DEBUGGER_WIDTH-arrows[i][0])] = LIME_COLOR_2;
		}
	}

	// progress bar for cycles done for scanline
	for (int x=232; x<(232+VIDEO_XRES); x++) {
		if (x < ((system->core->vdc->get_cycles_run()*VIDEO_XRES)/CPU_CYCLES_PER_SCANLINE)+232) {
			buffer[(3*DEBUGGER_WIDTH) + x] = LIME_COLOR_2;
			buffer[(4*DEBUGGER_WIDTH) + x] = LIME_COLOR_2;
		} else {
			buffer[(3*DEBUGGER_WIDTH) + x] = LIME_COLOR_0;
			buffer[(4*DEBUGGER_WIDTH) + x] = LIME_COLOR_0;
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
	// } else if (token0[0] == ',') {
	// 	have_prompt = false;
	// 	enter_assembly_line(c);
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
	// } else if (strcmp(token0, "d") == 0) {
	// 	have_prompt = false;
	// 	token1 = strtok(NULL, " ");

	// 	uint8_t lines_remaining = terminal->lines_remaining();
	// 	if (lines_remaining == 0) lines_remaining = 1;

	// 	uint32_t temp_pc = system->core->cpu->get_pc();

	// 	if (token1 == NULL) {
	// 		for (int i=0; i<lines_remaining; i++) {
	// 			terminal->printf("\n.");
	// 			temp_pc += disassemble_instruction(temp_pc);
	// 		}
	// 	} else {
	// 		if (!hex_string_to_int(token1, &temp_pc)) {
	// 			terminal->printf("\nerror: '%s' is not a hex number", token1);
	// 			have_prompt = true;
	// 		} else {
	// 			for (int i=0; i<lines_remaining; i++) {
	// 				terminal->printf("\n.");
	// 				temp_pc += disassemble_instruction(temp_pc);
	// 			}
	// 		}
	// 	}
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
	// } else if (strcmp(token0, "n") == 0) {
	// 	system->core->run(true);
	// 	status();
	} else if (strcmp(token0, "col") == 0) {
		terminal->printf("\n     00  01  10  11");
		for (int i=0; i<16; i++) {
			if ((i & 0b11) == 0) {
				terminal->printf("  \n %c%c ", i & 0b1000 ? '1' : '0', i & 0b0100 ? '1' : '0');
			}
			terminal->bg_color = colors[i];
			terminal->printf("    ");
			terminal->bg_color = C64_BLUE;;
		}
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
	// } else if (strcmp(token0, "timer") == 0) {
	// 	for (int i=0; i<8; i++) {
	// 		system->core->timer->status(text_buffer, i);
	// 		terminal->printf("%s", text_buffer);
	// 	}
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
	terminal->printf("\nlime v%i.%i.%i (C)%i elmerucr",
	       LIME_MAJOR_VERSION,
	       LIME_MINOR_VERSION,
	       LIME_BUILD, LIME_YEAR);
}

void debugger_t::memory_dump(uint16_t address)
{
	address = address & 0xffff;

	uint32_t temp_address = address;
	terminal->printf("\r.:%04x ", temp_address);
	for (int i=0; i<8; i++) {
		terminal->printf("%02x ", system->core->read8(temp_address));
		temp_address++;
		temp_address &= 0xffff;
	}
	temp_address = address;

	terminal->bg_color = LIME_COLOR_0;
	terminal->fg_color = LIME_COLOR_2;

	for (int i=0; i<8; i++) {
		uint8_t temp_byte = system->core->read8(temp_address);
		terminal->putsymbol(temp_byte);
		temp_address++;
		temp_address &= 0xffff;
	}

	terminal->bg_color = C64_BLUE;
	terminal->fg_color = C64_LIGHTBLUE;

	//address += 8;
	//address &= 0xffff;

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
		switch (res[i]) {
			case 0b00: terminal->bg_color = LIME_COLOR_0; break;
			case 0b01: terminal->bg_color = LIME_COLOR_1; break;
			case 0b10: terminal->bg_color = LIME_COLOR_2; break;
			case 0b11: terminal->bg_color = LIME_COLOR_3; break;
		}
		terminal->printf("  ");
	}
	terminal->bg_color = C64_BLUE;

	for (int i=0; i<40; i++) {
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

uint32_t debugger_t::disassemble_instruction(uint16_t address)
{
	uint32_t cycles;
	if (system->core->cpu->breakpoint_array[address]) {
		status->fg_color = 0xffe04040;	// orange
		//terminal->bg_color = bg_acc;
	}
	cycles = system->core->cpu->disassemble_instruction(text_buffer, 1024, address) & 0xffff;
	status->printf("%s", text_buffer);
	status->fg_color = LIME_COLOR_2;
	//terminal->bg_color = bg;

	status->putchar('\r');
	for (int i=0; i<7; i++) status->cursor_right();
	return cycles;
}
