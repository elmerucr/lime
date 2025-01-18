#include "debugger.hpp"
#include "core.hpp"
#include <cstdint>

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

	terminal = new terminal_t(54, 38, C64_LIGHTBLUE, C64_BLUE);
	terminal->clear();
	terminal->printf("lime (c)2025 elmerucr");
	terminal->activate_cursor();

	status = new terminal_t(60, 17, GB_COLOR_2, GB_COLOR_0);
	status->clear();

	char buffer[1024];
	system->core->cpu->status(buffer, 1024);

	status->printf("__cpu_________________________________________________\n%s", buffer);
	status->printf("\n\n__disassembly_________________________________________");
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
	terminal->process_cursor_state();
	for (int y = 0; y<terminal->height; y++) {
		for (int x = 0; x<terminal->width; x++) {
			tiles[(y+1)*120 + x + 2] = terminal->tiles[(y*terminal->width)+x];
			fg_colors[(y+1)*120 + x + 2] = terminal->fg_colors[(y*terminal->width)+x];
			bg_colors[(y+1)*120 + x + 2] = terminal->bg_colors[(y*terminal->width)+x];
		}
	}

	for (int y = 0; y<status->height; y++) {
		for (int x = 0; x<status->width; x++) {
			tiles[(y+22)*120 + x + 58] = status->tiles[(y*status->width)+x];
			fg_colors[(y+22)*120 + x + 58] = status->fg_colors[(y*status->width)+x];
			bg_colors[(y+22)*120 + x + 58] = status->bg_colors[(y*status->width)+x];
		}
	}

	//

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
}

void debugger_t::prompt()
{
	terminal->printf("\n.");
}
