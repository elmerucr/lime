#include "debugger.hpp"
#include <cstdint>

debugger_t::debugger_t()
{
	buffer = new uint32_t[4 * PIXELS_PER_SCANLINE * SCANLINES];

	tiles = new uint8_t[(PIXELS_PER_SCANLINE >> 2) * (SCANLINES >> 2)];
	fg_colors = new uint32_t[(PIXELS_PER_SCANLINE >> 2) * (SCANLINES >> 2)];
	bg_colors = new uint32_t[(PIXELS_PER_SCANLINE >> 2) * (SCANLINES >> 2)];

	for (int i=0; i<(PIXELS_PER_SCANLINE >> 2) * (SCANLINES >> 2); i++) {
		tiles[i] = rca.byte();
		fg_colors[i] =
			0xff000000 |
			(rca.byte() << 16) |
			//(rca.byte() << 8) |
			rca.byte();
		bg_colors[i] =
			0xff000000 |
			//(rca.byte() << 16) |
			//(rca.byte() << 8) |
			rca.byte();
	}

	terminal = new terminal_t(56, 16, GB_COLOR_2, GB_COLOR_0);
	terminal->clear();
	terminal->activate_cursor();
}

debugger_t::~debugger_t()
{
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
			tiles[(22+y)*60 + (2+x)] = terminal->tiles[(y*terminal->width)+x];
			fg_colors[(22+y)*60 + (2+x)] = terminal->fg_colors[(y*terminal->width)+x];
			bg_colors[(22+y)*60 + (2+x)] = terminal->bg_colors[(y*terminal->width)+x];
		}
	}

	//

	// update buffer
	for (int y=0; y<(2*SCANLINES); y++) {
		uint8_t y_in_char = y % 8;
		for (int x=0; x<(2*PIXELS_PER_SCANLINE); x++) {
			uint8_t symbol = tiles[((y>>3) * (PIXELS_PER_SCANLINE >> 2)) + (x >> 3)];
			uint8_t x_in_char = x % 8;
			buffer[(y * 2 * PIXELS_PER_SCANLINE) + x] =
				(font.data[(symbol << 3) + y_in_char] & (0b1 << (7 - x_in_char))) ?
				fg_colors[((y>>3) * (PIXELS_PER_SCANLINE >> 2)) + (x >> 3)] :
				bg_colors[((y>>3) * (PIXELS_PER_SCANLINE >> 2)) + (x >> 3)] ;
		}
	}
}
