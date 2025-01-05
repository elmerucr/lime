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

    // fill with pattern
    for (int i = 0; i < RAM_SIZE; i++) {
        ram[i] = (i & 0x40) ? 0xff : 0x00;
    }

	// clear tileset memory
    for (int i = 0; i < 0x1800; i++) {
        ram[TILESET_0 + i] = 0;
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

    // tileset 0
    // tile 0 = empty

    // tile 1
    ram[TILESET_1 + 16] = 0b00'00'00'00; ram[TILESET_1 + 17] = 0b00'00'00'00;
    ram[TILESET_1 + 18] = 0b00'00'00'01; ram[TILESET_1 + 19] = 0b00'00'00'00;
    ram[TILESET_1 + 20] = 0b00'00'01'11; ram[TILESET_1 + 21] = 0b10'00'00'00;
    ram[TILESET_1 + 22] = 0b00'00'01'11; ram[TILESET_1 + 23] = 0b10'10'00'00;
    ram[TILESET_1 + 24] = 0b00'01'11'10; ram[TILESET_1 + 25] = 0b11'11'10'00;
    ram[TILESET_1 + 26] = 0b00'01'11'10; ram[TILESET_1 + 27] = 0b10'10'11'11;
    ram[TILESET_1 + 28] = 0b00'01'11'10; ram[TILESET_1 + 29] = 0b10'10'10'10;
    ram[TILESET_1 + 30] = 0b00'01'11'10; ram[TILESET_1 + 31] = 0b10'10'11'11;

    // tile 2
    ram[TILESET_1 + 32] = 0b00'00'00'00; ram[TILESET_1 + 33] = 0b00'00'00'00;
    ram[TILESET_1 + 34] = 0b00'00'00'00; ram[TILESET_1 + 35] = 0b00'00'00'00;
    ram[TILESET_1 + 36] = 0b00'00'00'00; ram[TILESET_1 + 37] = 0b00'00'00'00;
    ram[TILESET_1 + 38] = 0b00'00'00'00; ram[TILESET_1 + 39] = 0b00'00'00'00;
    ram[TILESET_1 + 40] = 0b00'00'00'00; ram[TILESET_1 + 41] = 0b00'00'00'00;
    ram[TILESET_1 + 42] = 0b00'00'00'00; ram[TILESET_1 + 43] = 0b00'00'00'00;
    ram[TILESET_1 + 44] = 0b11'00'00'00; ram[TILESET_1 + 45] = 0b00'00'00'00;
    ram[TILESET_1 + 46] = 0b10'11'00'00; ram[TILESET_1 + 47] = 0b00'00'00'00;

    // tile 3
    ram[TILESET_1 + 48] = 0b00'01'11'10; ram[TILESET_1 + 49] = 0b11'11'10'10;
    ram[TILESET_1 + 50] = 0b00'00'01'11; ram[TILESET_1 + 51] = 0b10'10'10'10;
    ram[TILESET_1 + 52] = 0b00'00'01'11; ram[TILESET_1 + 53] = 0b10'10'10'11;
    ram[TILESET_1 + 54] = 0b00'00'00'01; ram[TILESET_1 + 55] = 0b11'10'10'11;
    ram[TILESET_1 + 56] = 0b00'00'00'00; ram[TILESET_1 + 57] = 0b01'11'11'10;
    ram[TILESET_1 + 58] = 0b00'00'00'00; ram[TILESET_1 + 59] = 0b00'01'01'11;
    ram[TILESET_1 + 60] = 0b00'00'00'00; ram[TILESET_1 + 61] = 0b00'00'00'01;
    ram[TILESET_1 + 62] = 0b00'00'00'00; ram[TILESET_1 + 63] = 0b00'00'00'00;

    // tile 4
    ram[TILESET_1 + 64] = 0b11'10'11'00; ram[TILESET_1 + 65] = 0b00'00'00'00;
    ram[TILESET_1 + 66] = 0b11'10'11'10; ram[TILESET_1 + 67] = 0b00'00'00'00;
    ram[TILESET_1 + 68] = 0b10'10'10'11; ram[TILESET_1 + 69] = 0b10'00'00'00;
    ram[TILESET_1 + 70] = 0b10'10'10'11; ram[TILESET_1 + 71] = 0b10'10'00'00;
    ram[TILESET_1 + 72] = 0b10'10'10'10; ram[TILESET_1 + 73] = 0b11'11'01'00;
    ram[TILESET_1 + 74] = 0b11'11'11'11; ram[TILESET_1 + 75] = 0b01'01'00'00;
    ram[TILESET_1 + 76] = 0b01'01'01'01; ram[TILESET_1 + 77] = 0b00'00'00'00;
    ram[TILESET_1 + 78] = 0b00'00'00'00; ram[TILESET_1 + 79] = 0b00'00'00'00;

    // have something to display
    for (int i=0; i<0x400; i++) {
        ram[LAYER_0 + i] = i & 0xff;
    }

    ram[LAYER_0 + 36] = 1;
    ram[LAYER_0 + 37] = 2;
    ram[LAYER_0 + 68] = 3;
    ram[LAYER_0 + 69] = 4;
}

vdc_t::~vdc_t()
{
    delete [] buffer;
    delete [] ram;
}

void vdc_t::update()
{
    for (uint16_t scr_y = 0; scr_y < VIDEO_HEIGHT; scr_y++) {
        uint8_t y = (bg0_y + scr_y) & 0xff;
        uint8_t y_in_tile = y % 8;

        for (uint16_t scr_x = 0; scr_x < VIDEO_WIDTH; scr_x++) {
            uint8_t x = (bg0_x + scr_x) & 0xff;
            uint8_t px = x % 4;
            uint8_t tile_index = ram[LAYER_0 + ((y >> 3) << 5) + (x >> 3)];

            buffer[(VIDEO_WIDTH * scr_y) + scr_x] =
            	(ram[TILESET_1 + (tile_index << 4) + (y_in_tile << 1) + ((x & 0x4) ? 1 : 0)] &
				(0b11 << (2 * (3 - px)))) >> (2 * (3 - px));
        }
    }
}
