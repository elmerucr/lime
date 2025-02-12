// ---------------------------------------------------------------------
// osd.cpp
// lime
//
// Copyright © 2025 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include "osd.hpp"
#include "stats.hpp"

osd_t::osd_t(system_t *s)
{
	system = s;

	terminal = new terminal_t(system, width, height, fg, bg);
	terminal->clear();

	buffer = new uint32_t[width*8*height*8];
	for (int i = 0; i < width * 8 * height * 8; i++) {
		buffer[i] = i & 0b100 ? fg : bg;
	}
}

osd_t::~osd_t()
{
	delete [] buffer;
	delete terminal;
}

void osd_t::redraw()
{
	terminal->printf("%s", system->stats->summary());
	// update buffer
	for (int y=0; y<height*8; y++) {
		uint8_t y_in_char = y % 8;
		for (int x=0; x<width*8; x++) {
			uint8_t symbol = terminal->tiles[((y>>3) * ((width*8) >> 3)) + (x >> 3)];
			uint8_t x_in_char = x % 8;
			buffer[(y * width*8) + x] =
				(font.original_data[(symbol << 3) + y_in_char] & (0b1 << (7 - x_in_char))) ? fg : bg;
		}
	}
}
