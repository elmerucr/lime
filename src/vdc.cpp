// ---------------------------------------------------------------------
// vdc.cpp
// lime
//
// video display controller
// ---------------------------------------------------------------------

#include "vdc.hpp"
#include "common.hpp"
#include <cstdio>
#include "rca.hpp"

vdc_t::vdc_t()
{
    ram = new uint8_t[VRAM_SIZE];
    buffer = new uint32_t[VIDEO_XRES * VIDEO_YRES];

	layer[0].address = VDC_LAYERS_ADDRESS + 0x0000;
	layer[1].address = VDC_LAYERS_ADDRESS + 0x0400;
	layer[2].address = VDC_LAYERS_ADDRESS + 0x0800;
	layer[3].address = VDC_LAYERS_ADDRESS + 0x0c00;
}

vdc_t::~vdc_t()
{
    delete [] buffer;
    delete [] ram;
}

void vdc_t::reset()
{
	// fill video buffer with something
	rca_t rca;
	int8_t c = 0;
	for (int i=0; i<(VIDEO_XRES*VIDEO_YRES); i++) {
		//if (rca.byte() < 10) buffer[i] = palette[rca.byte() & 0b11];
		if (c < 10) {
			buffer[i] = palette[rca.byte() & 0b11];
		} else {
			buffer[i] = palette[0b00];
		}
		// buffer[i] = c & 0b1000 ? palette[0b1] : palette[0b0];
		if (c++ == 37) c = 0;
	}

    // fill memory with alternating pattern
    for (int i = 0; i < VRAM_SIZE; i++) {
        ram[i] = (i & 0x40) ? 0xff : 0x00;
    }

	// clear full tileset memory (6kb)
    for (int i = 0; i < 0x1800; i++) {
		// takes care of both TILESET_0 and TILESET_1
        ram[VDC_TILESET_0_ADDRESS + i] = 0;
    }

    // tileset 0
	const uint8_t lime_icon[64] = {
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

	for (int i = 0; i < 64; i++) {
		// skip tile 0, is empty
		ram[VDC_TILESET_0_ADDRESS + (1 * 16) + i] = lime_icon[i];
	}

    // tileset 1 contains all CBM characters
    for (int i = 0; i < (8 * font.size); i++) {
        uint16_t index = i >> 3;
        int index_pos = 7 - (i & 0b111);

        uint16_t address = VDC_TILESET_1_ADDRESS + (i >> 2);
        uint8_t address_pos = 6 - ((i & 0b11) << 1);

        if (font.data[index] & (0b1 << index_pos)) {
            ram[address] |= (0b11 << address_pos);
        }
    }

	// icon
	sprite[0] = { 112, 64, 0b00000101, 0x01 };
	sprite[1] = { 120, 64, 0b00000101, 0x02 };
	sprite[2] = { 112, 72, 0b00000101, 0x03 };
	sprite[3] = { 120, 72, 0b00000101, 0x04 };

	// text
	sprite[4] = { 107, 80, 0b00000111, 0x6c };	// l
	sprite[5] = { 112, 80, 0b00000111, 0x69 };	// i
	sprite[6] = { 118, 80, 0b00000111, 0x6d };	// m
	sprite[7] = { 126, 80, 0b00000111, 0x65 };	// e

	cycles_run = 0;
	current_scanline = 0;
}

void vdc_t::draw_layer(layer_t *l, uint8_t sl)
{
	if (l->flags & 0b1) {
		// Determine tileset
		uint16_t tileset = (l->flags & 0b10) ? VDC_TILESET_1_ADDRESS : VDC_TILESET_0_ADDRESS;

		if (sl < VIDEO_YRES) {
			uint8_t y = (l->y + sl) & 0xff;
			uint8_t y_in_tile = y % 8;

			for (uint16_t scr_x = 0; scr_x < VIDEO_XRES; scr_x++) {
				uint8_t x = (l->x + scr_x) & 0xff;
				uint8_t px = x % 4;
				uint8_t tile_index = ram[l->address + ((y >> 3) << 5) + (x >> 3)];

				uint8_t res =
					(ram[tileset + (tile_index << 4) + (y_in_tile << 1) + ((x & 0x4) ? 1 : 0)] &
					(0b11 << (2 * (3 - px)))) >> (2 * (3 - px));
				// if NOT (transparent AND 0b00) then pixel must be written
				if (!((l->flags & 0b100) && !res)) {
					buffer[(VIDEO_XRES * sl) + scr_x] = palette[l->palette[res]];
				}
			}
		}
	}
}

void vdc_t::draw_sprite(sprite_t *s, uint8_t sl, layer_t *l)
{
	if (s->flags & 0b1) {
		// Determine tileset
		uint16_t tileset = (s->flags & 0b10) ? VDC_TILESET_1_ADDRESS : VDC_TILESET_0_ADDRESS;

		// Subtract y position from scanline, remainder is y position in sprite
		uint8_t y = sl - s->y;

		// if y < 8, it's in range of the sprite
		if (y < 8) {
			// relative position
			uint8_t adj_x = s->x - ((s->flags & 0b1000) ? l->x : 0);

			uint8_t start_x{0}, end_x{0};

			if (s->x < VIDEO_XRES) {
				start_x = adj_x;
				end_x = ((adj_x + 8) > VIDEO_XRES) ? VIDEO_XRES : (adj_x + 8);
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
					buffer[(VIDEO_XRES * sl) + scr_x] = palette[s->palette[res]];
				}

				// restore y, if xy flip was done before
				if (s->flags & 0b01000000) y = x;
			}
		}
	}
}

void vdc_t::draw_scanline(uint16_t scanline)
{
	if (scanline < VIDEO_YRES) {
		for (int x=0; x<VIDEO_XRES; x++) {
			buffer[(VIDEO_XRES * scanline) + x] = palette[bg_color & 0b11];
		}

		for (int l=3; l>=0; l--) {
			draw_layer(&layer[l], scanline);

			for (uint8_t i=0; i<64; i++) {
				draw_sprite(&sprite[(64*l) + (63-i)], scanline, &layer[l]);
			}
		}
	}
}

uint8_t vdc_t::io_read8(uint16_t address)
{
	switch (address & 0xff) {
		case 0x02:
			return current_scanline >> 8;
		case 0x03:
			return current_scanline & 0xff;
		case 0x04:
			return bg_color & 0b11;
		default:
			return 0;
	}
}

void vdc_t::io_write8(uint16_t address, uint8_t value)
{
	switch (address & 0xff) {
	case 0x04:
		bg_color = value & 0b11;
		break;
	default:
		break;
	}
}

bool vdc_t::run(uint32_t number_of_cycles)
{
	bool frame_done = false;

	cycles_run += number_of_cycles;

	if (cycles_run >= CPU_CYCLES_PER_SCANLINE) {
		draw_scanline(current_scanline);
		cycles_run -= CPU_CYCLES_PER_SCANLINE;
		current_scanline++;

		if (current_scanline == VIDEO_SCANLINES) {
			current_scanline = 0;
			frame_done = true;
		}
	}

	return frame_done;
}
