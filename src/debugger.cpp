#include "debugger.hpp"
#include <cstdint>

debugger_t::debugger_t()
{
	buffer = new uint32_t[4 * PIXELS_PER_SCANLINE * SCANLINES];

	chars = new uint8_t[(PIXELS_PER_SCANLINE >> 2) * (SCANLINES >> 2)];
	fg_color = new uint32_t[(PIXELS_PER_SCANLINE >> 2) * (SCANLINES >> 2)];
	bg_color = new uint32_t[(PIXELS_PER_SCANLINE >> 2) * (SCANLINES >> 2)];

	for (int i=0; i<(PIXELS_PER_SCANLINE >> 2) * (SCANLINES >> 2); i++) {
		chars[i] = rca.byte();
		fg_color[i] =
			0xff000000 |
			(rca.byte() << 16) |
			//(rca.byte() << 8) |
			rca.byte();
		bg_color[i] =
			0xff000000 |
			//(rca.byte() << 16) |
			//(rca.byte() << 8) |
			rca.byte();
	}
}

debugger_t::~debugger_t()
{
	delete [] bg_color;
	delete [] fg_color;
	delete [] chars;
	delete [] buffer;
}

void debugger_t::redraw()
{
	//

	//

	// update buffer
	for (int y=0; y<(2*SCANLINES); y++) {
		uint8_t y_in_char = y % 8;
		for (int x=0; x<(2*PIXELS_PER_SCANLINE); x++) {
			uint8_t symbol = chars[((y>>3) * (PIXELS_PER_SCANLINE >> 2)) + (x >> 3)];
			uint8_t x_in_char = x % 8;
			buffer[(y * 2 * PIXELS_PER_SCANLINE) + x] =
				(font.data[(symbol << 3) + y_in_char] & (0b1 << (7 - x_in_char))) ?
				fg_color[((y>>3) * (PIXELS_PER_SCANLINE >> 2)) + (x >> 3)] :
				bg_color[((y>>3) * (PIXELS_PER_SCANLINE >> 2)) + (x >> 3)] ;
		}
	}
}
