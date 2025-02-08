// ---------------------------------------------------------------------
// vdc.cpp
// lime
//
// video display controller
// ---------------------------------------------------------------------

#include "vdc.hpp"
#include "common.hpp"
#include <cstdio>

vdc_t::vdc_t()
{
    ram = new uint8_t[RAM_SIZE];
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
	// initial video buffer status, buffer invisible to system
	for (int i=0; i<(VIDEO_YRES*VIDEO_XRES); i++) {
		if ((i%VIDEO_YRES) & 0b100) {
			buffer[i] = colors[0b01];
		} else {
			buffer[i] = colors[0b00];
		}
	}

    // fill memory with alternating pattern
    for (int i = 0; i < RAM_SIZE; i++) {
        ram[i] = (i & 0x40) ? 0xff : 0x00;
    }

	current_layer = 0;
	current_sprite = 0;

	cycles_run = 0;
	next_scanline = 160;
	new_scanline = true;
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
					buffer[(VIDEO_XRES * sl) + scr_x] = colors[l->palette[res]];
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
					buffer[(VIDEO_XRES * sl) + scr_x] = colors[s->palette[res]];
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
			buffer[(VIDEO_XRES * scanline) + x] = colors[bg_color];
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
		// general
		case 0x02:
			return next_scanline >> 8;
		case 0x03:
			return next_scanline & 0xff;
		case 0x04:
			return bg_color;
		case 0x06:
			return current_layer;
		case 0x07:
			return current_sprite;

		// layers
		case 0x10:
			return layer[current_layer].x;
		case 0x11:
			return layer[current_layer].y;
		case 0x12:
			return layer[current_layer].flags;
		//case 0x13:
		//	return 0;
		case 0x14:
			return layer[current_layer].palette[0];
		case 0x15:
			return layer[current_layer].palette[1];
		case 0x16:
			return layer[current_layer].palette[2];
		case 0x17:
			return layer[current_layer].palette[3];

		// sprites
		case 0x18:
			return sprite[current_sprite].x;
		case 0x19:
			return sprite[current_sprite].y;
		case 0x1a:
			return sprite[current_sprite].flags;
		case 0x1b:
			return sprite[current_sprite].index;
		case 0x1c:
			return sprite[current_sprite].palette[0];
		case 0x1d:
			return sprite[current_sprite].palette[1];
		case 0x1e:
			return sprite[current_sprite].palette[2];
		case 0x1f:
			return sprite[current_sprite].palette[3];

		default:
			return 0;
	}
}

void vdc_t::io_write8(uint16_t address, uint8_t value)
{
	switch (address & 0xff) {
		// general
		case 0x04:
			bg_color = value;
			break;
		case 0x06:
			current_layer = value &0b11;
			break;
		case 0x07:
			current_sprite = value;
			break;

		// layers
		case 0x10:
			layer[current_layer].x = value;
			break;
		case 0x11:
			layer[current_layer].y = value;
			break;
		case 0x12:
			layer[current_layer].flags = value & 0b00000111;
			break;
		//case 0x13:
		//	break;
		case 0x14:
			layer[current_layer].palette[0] = value;
			break;
		case 0x15:
			layer[current_layer].palette[1] = value;
			break;
		case 0x16:
			layer[current_layer].palette[2] = value;
			break;
		case 0x17:
			layer[current_layer].palette[3] = value;
			break;

		// sprites
		case 0x18:
			sprite[current_sprite].x = value;
			break;
		case 0x19:
			sprite[current_sprite].y = value;
			break;
		case 0x1a:
			sprite[current_sprite].flags = value & 0b01111111;
			break;
		case 0x1b:
			sprite[current_sprite].index = value;
			break;
		case 0x1c:
			sprite[current_sprite].palette[0] = value;
			break;
		case 0x1d:
			sprite[current_sprite].palette[1] = value;
			break;
		case 0x1e:
			sprite[current_sprite].palette[2] = value;
			break;
		case 0x1f:
			sprite[current_sprite].palette[3] = value;
			break;

		default:
			break;
	}
}

bool vdc_t::run(uint32_t number_of_cycles)
{
	new_scanline = false;
	bool frame_done = false;

	cycles_run += number_of_cycles;

	if (cycles_run >= CPU_CYCLES_PER_SCANLINE) {
		draw_scanline(next_scanline);
		cycles_run -= CPU_CYCLES_PER_SCANLINE;
		next_scanline++;
		new_scanline = true;

		if (next_scanline == VIDEO_SCANLINES) {
			next_scanline = 0;
			frame_done = true;
		}
	}

	return frame_done;
}
