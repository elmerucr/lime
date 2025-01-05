#include "vdc.hpp"
#include "common.hpp"
#include <cstdio>
#include <cassert>

vdc_t::vdc_t()
{
    ram = new uint8_t[RAM_SIZE];

    buffer = new uint8_t[VIDEO_WIDTH * VIDEO_HEIGHT];

    // fill with pattern
    for (int i = 0; i < RAM_SIZE; i++) {
        ram[i] = (i & 0x40) ? 0xff : 0x00;
    }

    // tileset 1
    for (int i = 0; i < 0x1000; i++) {
        ram[TILESET_1 + i] = 0;
    }

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

    for (int i=0; i<0x400; i++) {
        ram[BACKGROUND_0 + i] = i & 0xff;
    }
    ram[BACKGROUND_0 + 36] = 1;
    ram[BACKGROUND_0 + 37] = 2;
    ram[BACKGROUND_0 + 68] = 3;
    ram[BACKGROUND_0 + 69] = 4;
}

vdc_t::~vdc_t()
{
    delete [] buffer;
    delete [] ram;
}

void vdc_t::run()
{
    uint8_t tx, ty;
    uint8_t px, py; // pixel in x...
    uint8_t tile_index;

    for (uint16_t y = 0; y < VIDEO_HEIGHT; y++){
            ty = (bg0_y + y) & 0xff;
            py = ty % 8;
        for (uint16_t x = 0; x < VIDEO_WIDTH; x++) {
            tx = (bg0_x + x) & 0xff;
            px = tx % 4;
            assert(px < 4);

            tile_index = ram[BACKGROUND_0 + (32 * (ty >> 3)) + (tx >> 3)];

            buffer[(VIDEO_WIDTH * y) + x] =
            (ram[TILESET_1 + (16 * tile_index) + (py << 1) + ((tx & 0x4) ? 1 : 0)] & (0b11 << (2 * (3 - px)))) >> (2 * (3 - px));

        }
    }
}
