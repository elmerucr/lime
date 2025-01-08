// ---------------------------------------------------------------------
// vdc.cpp
//
// video display controller
// ---------------------------------------------------------------------

#include "vdc.hpp"
#include "common.hpp"
#include <cstdio>

vdc_t::vdc_t()
{
    ram = new uint8_t[RAM_SIZE];

    buffer = new uint8_t[VIDEO_WIDTH * VIDEO_HEIGHT];
}

vdc_t::~vdc_t()
{
    delete [] buffer;
    delete [] ram;
}

void vdc_t::draw_layer(layer_t *l)
{
	//
}

void vdc_t::draw_sprite(sprite_t *s, uint8_t sl)
{
	if (s->flags & 0b1) {
		// Determine tileset
		uint16_t tileset = (s->flags & 0b10) ? TILESET_1 : TILESET_0;

		// Subtract y position from scanline, remainder is y position in sprite
		uint8_t y = sl - s->y;

		// if y < 8, it's inside sprite
		if (y < 8) {
			uint8_t start_x{0}, end_x{0};

			if (s->x < VIDEO_WIDTH) {
				start_x = s->x;
				end_x = ((s->x + 8) > VIDEO_WIDTH) ? VIDEO_WIDTH : (s->x + 8);
			} else if (s->x >= 248) {
				end_x = s->x + 8;
			}

			// checking this flag before getting in for loop with scr_x
			// otherwise this is executed each new pixel
			if (s->flags & 0b00100000) y = 7 - y;

			for (uint8_t scr_x=start_x; scr_x<end_x; scr_x++) {
				uint8_t x = scr_x - s->x;

				if (s->flags & 0b00010000) x = 7 - x;


				uint8_t res =
					(ram[tileset + (s->index << 4) + (y << 1) + ((x & 0x4) ? 1 : 0)] &
					(0b11 << (2 * (3 - (x%4))))) >> (2 * (3 - (x%4)));
				// if NOT (transparency AND 0b00) then write pixel
				if (!((s->flags & 0b100) && !res)) {
					buffer[(VIDEO_WIDTH * sl) + scr_x] = s->palette[res];
				}
			}
		}
	}
}

// void vdc_t::_draw_sprite(sprite_t *s, uint8_t sl)
// {
// 	if (s->flags & 0b1) {
// 		// determine tileset
// 		uint16_t tileset = (s->flags & 0b10) ? TILESET_1 : TILESET_0;

// 		// subtract y position from scanline
// 		// remainder is y position in sprite
// 		uint8_t y = sl - s->y;

// 		// if y < 8, it's inside sprite
// 		if (y < 8) {
// 			uint8_t scr_y = (s->y + y)  & 0xff;
// 			bool ver_flip = s->flags & 0b100000;
// 			y = ver_flip ? 7 - y : y;
// 			if (scr_y < VIDEO_HEIGHT) {
// 				bool hor_flip = s->flags & 0b010000;

// 				for (int x=0; x<8; x++) {
// 					uint8_t scr_x = (s->x + (hor_flip ? 7 - x : x)) & 0xff;

// 					if (scr_x < VIDEO_WIDTH) {
// 						// find correct value for color
// 						uint8_t res =
// 							(ram[tileset + (s->index << 4) + (y << 1) + ((x & 0x4) ? 1 : 0)] &
// 							(0b11 << (2 * (3 - (x%4))))) >> (2 * (3 - (x%4)));
// 						// if NOT (transparency AND 0b00) then write pixel
// 						if (!((s->flags & 0b100) && !res)) {
// 							buffer[(VIDEO_WIDTH * scr_y) + scr_x] = res;
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}
// }

void vdc_t::reset()
{
    // fill with pattern
    for (int i = 0; i < RAM_SIZE; i++) {
        ram[i] = (i & 0x40) ? 0xff : 0x00;
    }

	// clear tileset memory
    for (int i = 0; i < 0x1800; i++) {
        ram[TILESET_0 + i] = 0;
    }

    // tileset 0
    // tile 0 = empty

    // tile 1
    ram[TILESET_0 + 16] = 0b00'00'00'00; ram[TILESET_0 + 17] = 0b00'00'00'00;
    ram[TILESET_0 + 18] = 0b00'00'00'01; ram[TILESET_0 + 19] = 0b00'00'00'00;
    ram[TILESET_0 + 20] = 0b00'00'01'11; ram[TILESET_0 + 21] = 0b10'00'00'00;
    ram[TILESET_0 + 22] = 0b00'00'01'11; ram[TILESET_0 + 23] = 0b10'10'00'00;
    ram[TILESET_0 + 24] = 0b00'01'11'10; ram[TILESET_0 + 25] = 0b11'11'10'00;
    ram[TILESET_0 + 26] = 0b00'01'11'10; ram[TILESET_0 + 27] = 0b10'10'11'11;
    ram[TILESET_0 + 28] = 0b00'01'11'10; ram[TILESET_0 + 29] = 0b10'10'10'10;
    ram[TILESET_0 + 30] = 0b00'01'11'10; ram[TILESET_0 + 31] = 0b10'10'11'11;

    // tile 2
    ram[TILESET_0 + 32] = 0b00'00'00'00; ram[TILESET_0 + 33] = 0b00'00'00'00;
    ram[TILESET_0 + 34] = 0b00'00'00'00; ram[TILESET_0 + 35] = 0b00'00'00'00;
    ram[TILESET_0 + 36] = 0b00'00'00'00; ram[TILESET_0 + 37] = 0b00'00'00'00;
    ram[TILESET_0 + 38] = 0b00'00'00'00; ram[TILESET_0 + 39] = 0b00'00'00'00;
    ram[TILESET_0 + 40] = 0b00'00'00'00; ram[TILESET_0 + 41] = 0b00'00'00'00;
    ram[TILESET_0 + 42] = 0b00'00'00'00; ram[TILESET_0 + 43] = 0b00'00'00'00;
    ram[TILESET_0 + 44] = 0b11'00'00'00; ram[TILESET_0 + 45] = 0b00'00'00'00;
    ram[TILESET_0 + 46] = 0b10'11'00'00; ram[TILESET_0 + 47] = 0b00'00'00'00;

    // tile 3
    ram[TILESET_0 + 48] = 0b00'01'11'10; ram[TILESET_0 + 49] = 0b11'11'10'10;
    ram[TILESET_0 + 50] = 0b00'00'01'11; ram[TILESET_0 + 51] = 0b10'10'10'10;
    ram[TILESET_0 + 52] = 0b00'00'01'11; ram[TILESET_0 + 53] = 0b10'10'10'11;
    ram[TILESET_0 + 54] = 0b00'00'00'01; ram[TILESET_0 + 55] = 0b11'10'10'11;
    ram[TILESET_0 + 56] = 0b00'00'00'00; ram[TILESET_0 + 57] = 0b01'11'11'10;
    ram[TILESET_0 + 58] = 0b00'00'00'00; ram[TILESET_0 + 59] = 0b00'01'01'11;
    ram[TILESET_0 + 60] = 0b00'00'00'00; ram[TILESET_0 + 61] = 0b00'00'00'01;
    ram[TILESET_0 + 62] = 0b00'00'00'00; ram[TILESET_0 + 63] = 0b00'00'00'00;

    // tile 4
    ram[TILESET_0 + 64] = 0b11'10'11'00; ram[TILESET_0 + 65] = 0b00'00'00'00;
    ram[TILESET_0 + 66] = 0b11'10'11'10; ram[TILESET_0 + 67] = 0b00'00'00'00;
    ram[TILESET_0 + 68] = 0b10'10'10'11; ram[TILESET_0 + 69] = 0b10'00'00'00;
    ram[TILESET_0 + 70] = 0b10'10'10'11; ram[TILESET_0 + 71] = 0b10'10'00'00;
    ram[TILESET_0 + 72] = 0b10'10'10'10; ram[TILESET_0 + 73] = 0b11'11'01'00;
    ram[TILESET_0 + 74] = 0b11'11'11'11; ram[TILESET_0 + 75] = 0b01'01'00'00;
    ram[TILESET_0 + 76] = 0b01'01'01'01; ram[TILESET_0 + 77] = 0b00'00'00'00;
    ram[TILESET_0 + 78] = 0b00'00'00'00; ram[TILESET_0 + 79] = 0b00'00'00'00;

    // tileset 1
    for (int i = 0; i < (8 * font.size); i++) {
        uint16_t index = i >> 3;
        int index_pos = 7 - (i & 0b111);

        uint16_t address = TILESET_1 + (i >> 2);
        uint8_t address_pos = 6 - ((i & 0b11) << 1);

        if (font.data[index] & (0b1 << index_pos)) {
            ram[address] |= (0b11 << address_pos);
        }
    }

    // // have something to display
    // for (int i=0; i<0x400; i++) {
    //     ram[LAYER_0 + i] = i & 0xff;
    // }
}

void vdc_t::update_scanline(uint8_t s)
{
	// draw layer 3
	// draw sprites 255-192
	// draw layer 2
	// draw sprites 191-128
	// draw layer 1
	// draw sprites 127-164

	// draw layer 0
	if (s < VIDEO_HEIGHT) {
        uint8_t y = (bg0_y + s) & 0xff;
        uint8_t y_in_tile = y % 8;

        for (uint16_t scr_x = 0; scr_x < VIDEO_WIDTH; scr_x++) {
            uint8_t x = (bg0_x + scr_x) & 0xff;
            uint8_t px = x % 4;
            uint8_t tile_index = ram[LAYER_0 + ((y >> 3) << 5) + (x >> 3)];

            buffer[(VIDEO_WIDTH * s) + scr_x] =
            	(ram[TILESET_1 + (tile_index << 4) + (y_in_tile << 1) + ((x & 0x4) ? 1 : 0)] &
				(0b11 << (2 * (3 - px)))) >> (2 * (3 - px));
		}
	}

	// draw sprites 63-0 (in that order)
	for (int i=63; i >= 0; i--) {
		draw_sprite(&sprite[i], s);
	}
}
