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
    ram = new uint8_t[VRAM_SIZE];
    buffer = new uint8_t[VIDEO_WIDTH * VIDEO_HEIGHT];
}

vdc_t::~vdc_t()
{
    delete [] buffer;
    delete [] ram;
}

void vdc_t::reset()
{
    // fill memory with alternating pattern
    for (int i = 0; i < VRAM_SIZE; i++) {
        ram[i] = (i & 0x40) ? 0xff : 0x00;
    }

	// clear full tileset memory (6kb)
    for (int i = 0; i < 0x1800; i++) {
        ram[TILESET_0 + i] = 0;
    }

    // tileset 0
	const uint8_t icon[64] = {
		0b00'00'00'00, 0b00'00'00'00, 0b00'00'00'01, 0b00'00'00'00,	// tile 1 (icon upper left)
		0b00'00'01'11, 0b10'00'00'00, 0b00'00'01'11, 0b10'10'00'00,
		0b00'01'11'10, 0b11'11'10'00, 0b00'01'11'10, 0b10'10'11'11,
		0b00'01'11'10, 0b10'10'10'10, 0b00'01'11'10, 0b10'10'11'11,
		0b00'00'00'00, 0b00'00'00'00, 0b00'00'00'00, 0b00'00'00'00,	// tile 2 (icon upper right)
		0b00'00'00'00, 0b00'00'00'00, 0b00'00'00'00, 0b00'00'00'00,
		0b00'00'00'00, 0b00'00'00'00, 0b00'00'00'00, 0b00'00'00'00,
		0b11'00'00'00, 0b00'00'00'00, 0b10'11'00'00, 0b00'00'00'00,
		0b00'01'11'10, 0b11'11'10'10, 0b00'00'01'11, 0b10'10'10'10,	// tile 3 (icon bottom left)
		0b00'00'01'11, 0b10'10'10'11, 0b00'00'00'01, 0b11'10'10'11,
		0b00'00'00'00, 0b01'11'11'10, 0b00'00'00'00, 0b00'01'01'11,
		0b00'00'00'00, 0b00'00'00'01, 0b00'00'00'00, 0b00'00'00'00,
		0b11'10'11'00, 0b00'00'00'00, 0b11'10'11'10, 0b00'00'00'00,	// tile 4 (icon bottom right)
		0b10'10'10'11, 0b10'00'00'00, 0b10'10'10'11, 0b10'10'00'00,
		0b10'10'10'10, 0b11'11'01'00, 0b11'11'11'11, 0b01'01'00'00,
		0b01'01'01'01, 0b00'00'00'00, 0b00'00'00'00, 0b00'00'00'00
	};

	for (int i = 0; i < 80; i++) {
		// skip tile 0, is empty
		ram[TILESET_0 + (1 * 16) + i] = icon[i];
	}

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

    // have something to display
    for (int i=0; i<0x400; i++) {
		ram[LAYER_0 + i] = i & 0xff;
		//ram[LAYER_0 + i] = 0;
    }
}

void vdc_t::draw_layer(layer_t *l, uint8_t sl)
{
	if (l->flags & 0b1) {
		// Determine tileset
		uint16_t tileset = (l->flags & 0b10) ? TILESET_1 : TILESET_0;

		if (sl < VIDEO_HEIGHT) {
			uint8_t y = (l->y + sl) & 0xff;
			uint8_t y_in_tile = y % 8;

			for (uint16_t scr_x = 0; scr_x < VIDEO_WIDTH; scr_x++) {
				uint8_t x = (l->x + scr_x) & 0xff;
				uint8_t px = x % 4;
				uint8_t tile_index = ram[LAYER_0 + ((y >> 3) << 5) + (x >> 3)];

				buffer[(VIDEO_WIDTH * sl) + scr_x] =
					(ram[tileset + (tile_index << 4) + (y_in_tile << 1) + ((x & 0x4) ? 1 : 0)] &
					(0b11 << (2 * (3 - px)))) >> (2 * (3 - px));
			}
		}
	}
}

void vdc_t::draw_sprite(sprite_t *s, uint8_t sl, layer_t *l)
{
	if (s->flags & 0b1) {
		// Determine tileset
		uint16_t tileset = (s->flags & 0b10) ? TILESET_1 : TILESET_0;

		// Subtract y position from scanline, remainder is y position in sprite
		uint8_t y = sl - s->y;

		// if y < 8, it's in range of the sprite
		if (y < 8) {
			// relative position
			uint8_t adj_x = s->x - ((s->flags & 0b1000) ? l->x : 0);

			uint8_t start_x{0}, end_x{0};

			if (s->x < VIDEO_WIDTH) {
				start_x = adj_x;
				end_x = ((adj_x + 8) > VIDEO_WIDTH) ? VIDEO_WIDTH : (adj_x + 8);
			} else if (adj_x >= 248) {
				end_x = adj_x + 8;
			}

			// test flip vertical
			if (s->flags & 0b00100000) y = 7 - y;

			for (uint8_t scr_x=start_x; scr_x<end_x; scr_x++) {
				uint8_t x = scr_x - adj_x;

				// test flip horizontal flag
				if (s->flags & 0b00010000) x = 7 - x;

				// test flip xy flag, if 1, swap x and y
				if (s->flags & 0b01000000) { uint8_t t = x; x = y; y = t; }

				uint8_t res = (ram[tileset + (s->index << 4) + (y << 1) + ((x & 0x4) ? 1 : 0)] &
					(0b11 << (2 * (3 - (x%4))))) >> (2 * (3 - (x%4)));
				// if NOT (transparent AND 0b00) then pixel must be written
				if (!((s->flags & 0b100) && !res)) {
					buffer[(VIDEO_WIDTH * sl) + scr_x] = s->palette[res];
				}

				// restore y, if xy flip was done before
				if (s->flags & 0b01000000) y = x;
			}
		}
	}
}

void vdc_t::update_scanline(uint8_t s)
{
	// draw layer 3
	// draw sprites 255-192 (in that order)
	// draw layer 2
	// draw sprites 191-128 (in that order)
	// draw layer 1
	// draw sprites 127-164 (in that order)

	// draw layer 0
	draw_layer(&layer[0], s);

	// draw sprites 63-0 (in that order)
	for (int i=63; i >= 0; i--) {
		draw_sprite(&sprite[i], s, &layer[0]);
	}
}
