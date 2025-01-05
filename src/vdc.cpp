#include "vdc.hpp"
#include "common.hpp"
#include <cstdio>

vdc_t::vdc_t()
{
    ram = new uint8_t[RAM_SIZE];

    // fill with pattern
    for (int i = 0; i < RAM_SIZE; i++) {
        ram[i] = (i & 0x40) ? 0xff : 0x00;
    }

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

    // tile 0 = empty

    // tile 1
    ram[TILESET_0 + 16] = 0b00'00'00'00; ram[TILESET_0 + 17] = 0b00'00'00'00;
    ram[TILESET_0 + 18] = 0b00'00'00'01; ram[TILESET_0 + 19] = 0b00'00'00'00;
    ram[TILESET_0 + 20] = 0b00'00'01'11; ram[TILESET_0 + 21] = 0b10'00'00'00;
    ram[TILESET_0 + 22] = 0b00'00'01'11; ram[TILESET_0 + 23] = 0b10'10'00'00;
    ram[TILESET_0 + 24] = 0b00'01'11'10; ram[TILESET_0 + 25] = 0b11'00'00'00;
    ram[TILESET_0 + 26] = 0b00'01'11'10; ram[TILESET_0 + 27] = 0b10'00'00'00;
    ram[TILESET_0 + 28] = 0b00'01'11'10; ram[TILESET_0 + 29] = 0b10'00'00'00;
    ram[TILESET_0 + 30] = 0b00'01'11'10; ram[TILESET_0 + 31] = 0b10'00'00'00;

    // tile 2

    // tile 3

    // tile 4

    for (int i=0; i<0x400; i++) {
        ram[BACKGROUND_0 + i] = i & 0xff;
    }
}

vdc_t::~vdc_t()
{
    delete [] ram;
}

void vdc_t::run()
{
    uint8_t tx, ty;
    uint8_t index;

    for (uint16_t y = 0; y < VIDEO_HEIGHT; y++){
            ty = (bg0_y + y) & 0xff;
        for (uint16_t x = 0; x < VIDEO_WIDTH; x++) {
            tx = (bg0_x + x) & 0xff;

            index = ram[BACKGROUND_0 + (32 *(ty >> 3)) + (tx >> 3)];
        }
    }
}
