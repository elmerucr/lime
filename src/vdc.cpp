// ---------------------------------------------------------------------
// vdc.cpp
// lime
//
// video display controller
// ---------------------------------------------------------------------

#include "vdc.hpp"
#include "common.hpp"
#include <cstdio>

vdc_t::vdc_t(exceptions_ic *e, sn74ls148_t *t)
{
	exceptions = e;
	sn74ls148 = t;

	dev_number_exceptions = exceptions->connect_device("vdc");
	printf("[vdc] Connecting to exceptions getting dev %i for MC6809\n", dev_number_exceptions);

	dev_number_sn74ls148 = sn74ls148->connect_device(6, "vdc");
	printf("[vdc] Connecting to sn74ls148 at IPL 6 getting dev %i for M68K\n", dev_number_exceptions);

    ram = new uint8_t[VDC_RAM];
    buffer = new uint32_t[VDC_XRES * VDC_YRES];

	layer[0].address = VDC_LAYER0_ADDRESS;
	layer[1].address = VDC_LAYER1_ADDRESS;
	layer[2].address = VDC_LAYER2_ADDRESS;
	layer[3].address = VDC_LAYER3_ADDRESS;
}

vdc_t::~vdc_t()
{
    delete [] buffer;
    delete [] ram;
}

void vdc_t::reset()
{
	// initial video buffer status, buffer invisible to system
	for (int i = 0; i < (VDC_YRES * VDC_XRES); i++) {
		if ((i % VDC_YRES) & 0b100) {
			buffer[i] = palette[0b01];
		} else {
			buffer[i] = palette[0b00];
		}
	}

    // fill memory with alternating pattern
    for (int i = 0; i < VDC_RAM; i++) {
        ram[i] = (i & 0x40) ? 0xff : 0x00;
    }

	for (int i=0; i<256; i++) {
		sprite[i].x = 0;
		sprite[i].y = 0;
		sprite[i].flags0 = 0;
		sprite[i].flags1 = 0;
		sprite[i].index = 0;
		sprite[i].colors[0] = 0b00;
		sprite[i].colors[1] = 0b01;
		sprite[i].colors[2] = 0b10;
		sprite[i].colors[3] = 0b11;
	}

	for (int i=0; i<4; i++) {
		layer[i].x = 0;
		layer[i].y = 0;
		layer[i].flags0 = 0x00;
		layer[i].flags1 = 0x00;
		layer[i].colors[0] = 0b00;
		layer[i].colors[1] = 0b01;
		layer[i].colors[2] = 0b10;
		layer[i].colors[3] = 0b11;
	}

	current_layer = 0;
	current_sprite = 0;
	current_palette = 0;

	cycles_run = 0;
	current_scanline = 0;
	irq_scanline = 0;
	new_scanline = true;

	bg_color = 0x00;

	irq_line = true;
	generate_interrupts = false;
}

void vdc_t::draw_layer(layer_t *l, uint8_t sl)
{
	if (l->flags0 & 0b1) {
		// Determine tileset
		uint16_t tileset = (l->flags0 & 0b10) ? VDC_TILESET1_ADDRESS : VDC_TILESET0_ADDRESS;

		if (sl < VDC_YRES) {
			uint8_t y = (l->y + sl) & 0xff;

			if (l->flags1 & 0b01000000) {
				y >>= 1;
			}

			uint8_t y_in_tile = y % 8;

			for (uint16_t scr_x = 0; scr_x < VDC_XRES; scr_x++) {
				uint8_t x = (l->x + scr_x) & 0xff;

				if (l->flags1 & 0b00010000) {
					x >>= 1;
				}

				uint8_t px = x % 4;
				uint8_t tile_index = ram[l->address + ((y >> 3) << 5) + (x >> 3)];

				uint8_t result =
					(ram[tileset + (tile_index << 4) + (y_in_tile << 1) + ((x & 0x4) ? 1 : 0)] &
					(0b11 << (2 * (3 - px)))) >> (2 * (3 - px));
				// if NOT (transparent AND 0b00) then pixel must be written
				if (!((l->flags0 & 0b100) && !result)) {
					buffer[(VDC_XRES * sl) + scr_x] = palette[l->colors[result]];
				}
			}
		}
	}
}

void vdc_t::draw_sprite(sprite_t *s, uint8_t sl, layer_t *l)
{
	if (s->flags0 & 0b1) {
		// Determine tileset
		uint16_t tileset = (s->flags0 & 0b10) ? VDC_TILESET1_ADDRESS : VDC_TILESET0_ADDRESS;

		// Subtract sprite y position from scanline, remainder is y position in sprite
		uint8_t y = sl - s->y;

		// Determine vertical size
		if (s->flags1 & 0b01000000) {
			y >>= 1;
		}

		// if y < 8, it's in range of the sprite
		if (y < 8) {
			// test if position is relative to layer
			uint8_t adj_x = s->x - ((s->flags0 & 0b1000) ? l->x : 0);

			uint8_t start_x, end_x;

			uint8_t width = 8;

			if (s->flags1 & 0b00010000) {
				width <<= 1;
			}

			if (s->x < VDC_XRES) {
				start_x = adj_x;
				end_x = ((adj_x + width) > VDC_XRES) ? VDC_XRES : (adj_x + width);
			} else if (adj_x >= (256 - width)) {
				start_x = 0;
				end_x = adj_x + width;
			}

			// test flip vertical
			if (s->flags1 & 0b00000010) y = 7 - y;

			for (uint8_t scr_x=start_x; scr_x<end_x; scr_x++) {
				uint8_t x = scr_x - adj_x;

				// Check for double width
				if (s->flags1 & 0b00010000) {
					x >>= 1;
				}

				// test flip horizontal flag
				if (s->flags1 & 0b00000001) x = 7 - x;

				// test flip xy flag, if 1, swap x and y
				if (s->flags1 & 0b00000100) { uint8_t t = x; x = y; y = t; }

				// look up color value (result) from tile
				uint8_t result = (ram[tileset + (s->index << 4) + (y << 1) + ((x & 0x4) ? 1 : 0)] &
					(0b11 << (2 * (3 - (x%4))))) >> (2 * (3 - (x%4)));

				// if NOT (transparent AND 0b00) then pixel must be written
				if (!((s->flags0 & 0b100) && !result)) {
					buffer[(VDC_XRES * sl) + scr_x] = palette[s->colors[result]];
				}

				// restore y, if xy flip was done before
				if (s->flags1 & 0b00000100) y = x;
			}
		}
	}
}

void vdc_t::draw_scanline(uint16_t scanline)
{
	if (scanline < VDC_YRES) {
		for (int x=0; x<VDC_XRES; x++) {
			buffer[(VDC_XRES * scanline) + x] = palette[bg_color];
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
	switch (address & 0x3f) {
		case 0x00:
			// status register
			return irq_line ? 0b0 : 0b1;
		case 0x01:
			// control register
			return generate_interrupts ? 0b1 : 0b0;
		case 0x04:
			return bg_color;
		case 0x05:
			return current_palette;
		case 0x06:
			return current_layer;
		case 0x07:
			return current_sprite;
		case 0x08:
			return (palette[current_palette] & 0xff000000) >> 24;
		case 0x09:
			return (palette[current_palette] & 0x00ff0000) >> 16;
		case 0x0a:
			return (palette[current_palette] & 0x0000ff00) >>  8;
		case 0x0b:
			return (palette[current_palette] & 0x000000ff) >>  0;
		case 0x0c:
			return (current_scanline & 0xff00) >> 8;
		case 0x0d:
			return current_scanline & 0xff;
		case 0x0e:
			return (irq_scanline & 0xff00) >> 8;;
		case 0x0f:
			return irq_scanline & 0xff;

		// layers
		case 0x10:
			return layer[current_layer].x;
		case 0x11:
			return layer[current_layer].y;
		case 0x12:
			return layer[current_layer].flags0;
		case 0x13:
			return layer[current_layer].flags1;
		case 0x18:
			return layer[current_layer].colors[0];
		case 0x19:
			return layer[current_layer].colors[1];
		case 0x1a:
			return layer[current_layer].colors[2];
		case 0x1b:
			return layer[current_layer].colors[3];

		// sprites
		case 0x20:
			return sprite[current_sprite].x;
		case 0x21:
			return sprite[current_sprite].y;
		case 0x22:
			return sprite[current_sprite].flags0;
		case 0x23:
			return sprite[current_sprite].flags1;
		case 0x24:
			return sprite[current_sprite].index;
		case 0x28:
			return sprite[current_sprite].colors[0];
		case 0x29:
			return sprite[current_sprite].colors[1];
		case 0x2a:
			return sprite[current_sprite].colors[2];
		case 0x2b:
			return sprite[current_sprite].colors[3];

		default:
			return 0;
	}
}

void vdc_t::io_write8(uint16_t address, uint8_t value)
{
	switch (address & 0x3f) {
		case 0x00:
			if ((value & 0b1) && !irq_line) {
				exceptions->release(dev_number_exceptions);
				sn74ls148->release_line(dev_number_sn74ls148);
				irq_line = true;
			}
			break;
		case 0x01:
			generate_interrupts = (value & 0b1) ? true : false;
			break;
		case 0x04:
			bg_color = value;
			break;
		case 0x05:
			current_palette = value;
			break;
		case 0x06:
			current_layer = value &0b11;
			break;
		case 0x07:
			current_sprite = value;
			break;
		case 0x08:
			// don't change, is always 0xff
			break;
		case 0x09:
			if (current_palette & 0x80) {
				palette[current_palette] = (palette[current_palette] & 0xff00ffff) | (value << 16);
			}
			break;
		case 0x0a:
			if (current_palette & 0x80) {
				palette[current_palette] = (palette[current_palette] & 0xffff00ff) | (value << 8);
			}
			break;
		case 0x0b:
			if (current_palette & 0x80) {
				palette[current_palette] = (palette[current_palette] & 0xffffff00) | value;
			}
			break;
		case 0x0e:
			irq_scanline = (irq_scanline & 0x00ff) | (value << 8);
			if (irq_scanline >= VDC_SCANLINES) {
				irq_scanline = VDC_SCANLINES - 1;
			}
			break;
		case 0x0f:
			irq_scanline = (irq_scanline & 0xff00) | value;
			if (irq_scanline >= VDC_SCANLINES) {
				irq_scanline = VDC_SCANLINES - 1;
			}
			break;

		// layers
		case 0x10:
			layer[current_layer].x = value;
			break;
		case 0x11:
			layer[current_layer].y = value;
			break;
		case 0x12:
			layer[current_layer].flags0 = value & 0b00000111;
			break;
		case 0x13:
			layer[current_layer].flags1 = value & 0b01010000;
			break;
		case 0x18:
			layer[current_layer].colors[0] = value;
			break;
		case 0x19:
			layer[current_layer].colors[1] = value;
			break;
		case 0x1a:
			layer[current_layer].colors[2] = value;
			break;
		case 0x1b:
			layer[current_layer].colors[3] = value;
			break;

		// sprites
		case 0x20:
			sprite[current_sprite].x = value;
			break;
		case 0x21:
			sprite[current_sprite].y = value;
			break;
		case 0x22:
			sprite[current_sprite].flags0 = value & 0b00001111;
			break;
		case 0x23:
			sprite[current_sprite].flags1 = value & 0b01010111;
			break;
		case 0x24:
			sprite[current_sprite].index = value;
			break;
		case 0x28:
			sprite[current_sprite].colors[0] = value;
			break;
		case 0x29:
			sprite[current_sprite].colors[1] = value;
			break;
		case 0x2a:
			sprite[current_sprite].colors[2] = value;
			break;
		case 0x2b:
			sprite[current_sprite].colors[3] = value;
			break;

		default:
			break;
	}
}

bool vdc_t::run(uint32_t number_of_cycles)
{
	if (new_scanline) {
		draw_scanline(current_scanline);
		new_scanline = false;
	}

	bool frame_done = false;
	cycles_run += number_of_cycles;

	if (cycles_run >= MC6809_CYCLES_PER_SCANLINE) {
		new_scanline = true;
		cycles_run -= MC6809_CYCLES_PER_SCANLINE;
		current_scanline++;
		if (current_scanline == VDC_SCANLINES) {
			frame_done = true;
			current_scanline = 0;
		}
		if ((current_scanline == irq_scanline) && generate_interrupts) {
			exceptions->pull(dev_number_exceptions);
			sn74ls148->pull_line(dev_number_sn74ls148);
			irq_line = false;
		}
	}

	return frame_done;
}
