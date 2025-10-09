#include <cstdio>
#include <cstdarg>
#include <cstring>
#include "terminal.hpp"
#include "debugger.hpp"
#include "core.hpp"

terminal_t::terminal_t(system_t *s, uint8_t w, uint8_t h, uint32_t fg, uint32_t bg)
{
	system = s;

	width = w;
	height = h;

	total_tiles = width * height;

	fg_color = fg;
	cursor_original_color = fg_color;
	bg_color = bg;
	cursor_original_background_color = bg_color;

	tiles = new uint8_t[total_tiles];
	fg_colors = new uint32_t[total_tiles];
	bg_colors = new uint32_t[total_tiles];
}

terminal_t::~terminal_t()
{
	delete [] bg_colors;
	delete [] fg_colors;
	delete [] tiles;
}

void terminal_t::clear()
{
	deactivate_cursor();
	for (int i=0; i < total_tiles; i++) {
		tiles[i] = ' ';
		fg_colors[i] = fg_color;
		bg_colors[i] = bg_color;
	}
	cursor_position = 0;
	activate_cursor();
}

void terminal_t::putsymbol_at_cursor(char symbol)
{
	tiles[cursor_position] = symbol;
	fg_colors[cursor_position] = fg_color;
	bg_colors[cursor_position] = bg_color;
}

void terminal_t::putsymbol(char symbol)
{
	putsymbol_at_cursor(symbol);
	cursor_position++;
	if (cursor_position >= total_tiles) {
		add_bottom_row();
		cursor_position -= width;
	}
}

void terminal_t::add_bottom_row()
{
	uint16_t no_of_tiles_to_move = total_tiles - width;

	for (int i=0; i < no_of_tiles_to_move; i++) {
		tiles[i] = tiles[width + i];
		fg_colors[i] = fg_colors[width + i];
		bg_colors[i] = bg_colors[width + i];
	}
	for (int i=no_of_tiles_to_move; i < total_tiles; i++) {
		tiles[i] = ' ';
		fg_colors[i] = fg_color;
		bg_colors[i] = bg_color;
	}
}

int terminal_t::putchar(int character)
{
	uint8_t result = (uint8_t)character;
	switch (result) {
		case '\r':
			cursor_position -= cursor_position % width;
			break;
		case '\n':
			cursor_position -= cursor_position % width;
			if ((cursor_position / width) == (height - 1)) {
				add_bottom_row();
			} else {
				cursor_position += width;
			}
			break;
		case '\t':
			while ((cursor_position % width) & 0b11) {
				putsymbol(' ');
			}
			break;
		default:
			putsymbol(result);
			break;
	}
	return result;
}

int terminal_t::puts(const char *text)
{
	int char_count = 0;
	if (text) {
		while (*text) {
			putchar(*text);
			char_count++;
			text++;
		}
	}
	return char_count;
}

int terminal_t::printf(const char *format, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, format);
	int number = vsnprintf(buffer, 1024, format, args);
	va_end(args);
	puts(buffer);
	return number;
}

void terminal_t::activate_cursor()
{
	cursor_original_char = tiles[cursor_position];
	cursor_original_color = fg_colors[cursor_position];
	cursor_original_background_color = bg_colors[cursor_position];
	cursor_blinking = true;
	cursor_countdown = 0;
}

void terminal_t::deactivate_cursor()
{
	cursor_blinking = false;
	tiles[cursor_position] = cursor_original_char;
	fg_colors[cursor_position] = cursor_original_color;
	bg_colors[cursor_position] = cursor_original_background_color;
}

void terminal_t::process_cursor_state()
{
	if (cursor_blinking == true) {
		if (cursor_countdown == 0) {
			tiles[cursor_position] ^= 0x80;
			if (((tiles[cursor_position]) & 0x80) != (cursor_original_char & 0x80)) {
				fg_colors[cursor_position] = fg_color;
			} else {
				fg_colors[cursor_position] = cursor_original_color;
			}
			cursor_countdown += cursor_interval;
		}
		cursor_countdown--;
	}
}

void terminal_t::cursor_left()
{
	if (cursor_position > 0) cursor_position--;
}

void terminal_t::cursor_right()
{
	cursor_position++;
	if (cursor_position > total_tiles - 1) {
		add_bottom_row();
		cursor_position -= width;
	}
}

void terminal_t::cursor_up()
{
	cursor_position -= width;

	if (cursor_position >= total_tiles) {
		cursor_position += width;
		uint32_t address;
		uint32_t w;

		switch (check_output(true, &address, &w)) {
			case NOTHING:
				break;
			case MEMORY:
				add_top_row();
				system->debugger->memory_dump((address - 8) & 0xfffffe);
				break;
			case MEMORY_BINARY:
				add_top_row();
				printf("\r");
				system->debugger->memory_binary_dump((address - 2) & 0xfffffe);
				break;
			case DISASSEMBLY:
				break;
		}
	}
}

void terminal_t::cursor_down()
{
	cursor_position += width;

	// cursor out of current screen?
	if (cursor_position >= total_tiles) {
		//add_bottom_row();
		cursor_position -= width;

		uint32_t address{0};
		uint32_t w{0};

		switch (check_output(false, &address, &w)) {
			case NOTHING:
				add_bottom_row();
				break;
			case MEMORY:
				add_bottom_row();
				system->debugger->memory_dump((address + 8) & 0xfffffe);
				break;
			case MEMORY_BINARY:
				printf("\n");
				system->debugger->memory_binary_dump((address + 2) & 0xfffffe);
				break;
			case DISASSEMBLY:
				if (system->core->mc68000_active) {
					address += system->core->mc68000->disassemble(text_buffer, address);
				} else {
					address += system->core->mc6809->disassemble_instruction(text_buffer, TEXT_BUFFER_SIZE, address);
				}
				add_bottom_row();
				printf("\r.");
				system->debugger->disassemble_instruction_terminal(address);
				break;
		}
	}
}

void terminal_t::backspace()
{
	uint16_t pos = cursor_position;
	uint16_t min_pos = 0;

	if (pos > min_pos) {
		cursor_position--;
		while (pos % width) {
			tiles[pos - 1] = tiles[pos];
			fg_colors[pos - 1] = fg_colors[pos];
			bg_colors[pos - 1] = bg_colors[pos];
			pos++;
		}
		tiles[pos - 1] = ' ';
		fg_colors[pos - 1] = fg_color;
		bg_colors[pos - 1] = bg_color;
	}
}

///*
// * https://gist.github.com/kenkam/790090
// */
//inline char *trim_space(char *str)
//{
//	char *end;
//	/* skip leading whitespace */
//	while (*str == ASCII_SPACE) {
//		str = str + 1;
//	}
//	/* remove trailing whitespace */
//	end = str + strlen(str) - 1;
//	while (end > str && (*end == ASCII_SPACE)) {
//		end = end - 1;
//	}
//	/* write null character */
//	*(end+1) = '\0';
//	return str;
//}

void terminal_t::get_command(char *c, int l)
{
	int length = width < (l-1) ? width : (l-1);

	uint16_t pos = cursor_position - (cursor_position % width);

	for (int i=0; i<length; i++) {
		c[i] = tiles[pos + i];
	}
	c[length] = 0;
}

enum output_type terminal_t::check_output(bool top_down, uint32_t *address, uint32_t *w)
{
	enum output_type output = NOTHING;

	char potential_address[7];

	for (int i = 0; i < total_tiles; i += width) {
		if (tiles[i + 1] == ':') {
			output = MEMORY;
			for (int j=0; j<6; j++) {
				potential_address[j] = tiles[i + 2 + j];
			}
			potential_address[6] = 0;
			system->debugger->hex_string_to_int(potential_address, address);
			if (top_down) break;
		} else if (tiles[i + 1] == ';') {
			output = MEMORY_BINARY;
			for (int j=0; j<6; j++) {
				potential_address[j] = tiles[i + 2 + j];
			}
			potential_address[6] = 0;
			system->debugger->hex_string_to_int(potential_address, address);
			if (top_down) break;
		} else if (tiles[i + 1] == ',') {
			output = DISASSEMBLY;
			int digits = system->core->mc68000_active ? 6 : 4;
			for (int j=0; j<digits; j++) {
				potential_address[j] = tiles[i + 2 + j];
			}
			potential_address[digits] = 0;
			// TODO: !!!
			system->debugger->hex_string_to_int(potential_address, address);
			if (top_down) break;
		}
	}
	return output;
}

void terminal_t::add_top_row()
{
	uint16_t no_of_tiles_to_move = total_tiles - width;

	uint16_t last_tile = total_tiles - 1;

	for (int i=0; i < no_of_tiles_to_move; i++) {
		tiles[last_tile - i] = tiles[last_tile - width - i];
		fg_colors[last_tile - i] = fg_colors[last_tile - width - i];
		bg_colors[last_tile - i] = bg_colors[last_tile - width - i];
	}
	for (int i=0; i < width; i++) {
		tiles[i] = ' ';
		fg_colors[i] = fg_color;
		bg_colors[i] = bg_color;
	}
}
