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
		if ((i % VDC_XRES) & 0b100) {
			buffer[i] = crt_palette[0b01];
		} else {
			buffer[i] = crt_palette[0b00];
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

	layer[0].tiles_address = VDC_LAYER0_TILES_ADDRESS & 0xfffe;
	layer[1].tiles_address = VDC_LAYER1_TILES_ADDRESS & 0xfffe;
	layer[2].tiles_address = VDC_LAYER2_TILES_ADDRESS & 0xfffe;
	layer[3].tiles_address = VDC_LAYER3_TILES_ADDRESS & 0xfffe;

	layer[0].colors_address = VDC_LAYER0_COLORS_ADDRESS & 0xfffe;
	layer[1].colors_address = VDC_LAYER1_COLORS_ADDRESS & 0xfffe;
	layer[2].colors_address = VDC_LAYER2_COLORS_ADDRESS & 0xfffe;
	layer[3].colors_address = VDC_LAYER3_COLORS_ADDRESS & 0xfffe;

	current_layer = 0;
	current_sprite = 0;
	current_palette_index = 0;

	cycles_run = 0;
	current_scanline = 0;
	irq_scanline = 0;
	new_scanline = true;

	bg_color = 0x00;

	irq_line = true;
	generate_interrupts = false;

	init_crt_palette();
}

void vdc_t::draw_layer(layer_t *l, uint16_t sl)
{
	if (l->flags0 & 0b1) {
		// Determine tileset address
		uint16_t tileset = (l->flags0 & 0b10) ? VDC_TILESET1_ADDRESS : VDC_TILESET0_ADDRESS;

		// Is scanline visible
		if (sl < VDC_YRES) {
			uint8_t y = (l->y + sl) & 0xff;

			// Double size vertical?
			if (l->flags1 & 0b01000000) {
				y >>= 1;
			}

			uint8_t y_in_tile = y % 8;

			bool tiles_colors = l->flags0 & 0b00001000;

			for (uint16_t scr_x = 0; scr_x < VDC_XRES; scr_x++) {
				uint16_t x = (l->x + scr_x) & 0x1ff;

				// Double size horizontal?
				if (l->flags1 & 0b00010000) {
					x >>= 1;
				}

				uint8_t px = x % 4;
				uint8_t tile_index = ram[(l->tiles_address + ((y >> 3) << 6) + (x >> 3)) & 0xffff];
				uint8_t color_index = ram[(l->colors_address + ((y >> 3) << 6) + (x >> 3)) & 0xffff];

				uint8_t result =
					(ram[tileset + (tile_index << 4) + (y_in_tile << 1) + ((x & 0x4) ? 1 : 0)] &
					(0b11 << (2 * (3 - px)))) >> (2 * (3 - px));
				// if NOT (transparent AND 0b00) then pixel must be written
				if (!((l->flags0 & 0b100) && !result)) {
					buffer[(VDC_XRES * sl) + scr_x] = tiles_colors ? crt_palette[color_index] : crt_palette[l->colors[result]];
				}
			}
		}
	}
}

void vdc_t::draw_sprite(sprite_t *s, uint16_t sl, layer_t *l)
{
	if (s->flags0 & 0b1) {
		// Determine tileset and assign appropriate address
		uint16_t tileset = (s->flags0 & 0b10) ? VDC_TILESET1_ADDRESS : VDC_TILESET0_ADDRESS;

		// if sprite y position relative to layer, adjust
		uint16_t adj_y = s->y - ((s->flags0 & 0b100000) ? l->y : 0);

		// Subtract sprite y position from scanline, remainder is y position
		// in sprite. It wraps around 256 (height of 32 tiles of 8x8)
		uint16_t y_in_sprite = (sl - adj_y) & 0xff;

		// Determine vertical size, and adjust y accordingly
		if (s->flags1 & 0b01000000) {
			y_in_sprite >>= 1;
		}

		// if y_in_sprite < 8, it's in range of the sprite (values 0 to 7 are valid)
		if (y_in_sprite < 8) {
			// if position is relative to layer, adjust x
			uint16_t adj_x = s->x - ((s->flags0 & 0b10000) ? l->x : 0);

			adj_x &= 0x1ff;

			uint16_t start_x, end_x;

			uint16_t width = 8;

			if (s->flags1 & 0b00010000) {
				width <<= 1;
			}

			if (adj_x < VDC_XRES) {
				// sprite is on the left of the right edge
				start_x = adj_x;
				end_x = ((adj_x + width) > VDC_XRES) ? VDC_XRES : (adj_x + width);
			} else if (adj_x >= (0x200 - width)) {
				//
				start_x = 0;
				end_x = (adj_x + width) & 0x1ff;
			} else {
				// not visible
				start_x = end_x = 0;
			}

			// test flip vertical
			if (s->flags1 & 0b00000010) y_in_sprite = 7 - y_in_sprite;

			for (uint16_t scr_x = start_x; scr_x < end_x; scr_x++) {
				uint16_t x = scr_x - adj_x;

				// Check for double width
				if (s->flags1 & 0b00010000) {
					x >>= 1;
				}

				// test flip horizontal flag
				if (s->flags1 & 0b00000001) x = 7 - x;

				// test flip xy flag, if 1, swap x and y
				if (s->flags1 & 0b00000100) { uint8_t t = x; x = y_in_sprite; y_in_sprite = t; }

				// look up color value (result) from tile
				uint8_t result = (ram[tileset + (s->index << 4) + (y_in_sprite << 1) + ((x & 0x4) ? 1 : 0)] &
					(0b11 << (2 * (3 - (x%4))))) >> (2 * (3 - (x%4)));

				// if NOT (transparent AND 0b00) then pixel must be written
				if (!((s->flags0 & 0b100) && !result)) {
					buffer[(VDC_XRES * sl) + scr_x] = crt_palette[s->colors[result]];
				}

				// restore y, if xy flip was done before
				if (s->flags1 & 0b00000100) y_in_sprite = x;
			}
		}
	}
}

void vdc_t::draw_scanline(uint16_t scanline)
{
	if (scanline < VDC_YRES) {
		for (int x=0; x<VDC_XRES; x++) {
			buffer[(VDC_XRES * scanline) + x] = crt_palette[bg_color];
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
		case 0x02:
		case 0x03:
			// reserved
			return 0;
		case 0x04:
			return bg_color;
		case 0x05:
			return current_palette_index;
		case 0x06:
			return current_layer;
		case 0x07:
			return current_sprite;
		case 0x08:
			return (palette[current_palette_index] & 0xff000000) >> 24;
		case 0x09:
			return (palette[current_palette_index] & 0x00ff0000) >> 16;
		case 0x0a:
			return (palette[current_palette_index] & 0x0000ff00) >>  8;
		case 0x0b:
			return (palette[current_palette_index] & 0x000000ff) >>  0;
		case 0x0c:
			return (current_scanline & 0xff00) >> 8;
		case 0x0d:
			return current_scanline & 0xff;
		case 0x0e:
			return (irq_scanline & 0xff00) >> 8;
		case 0x0f:
			return irq_scanline & 0xff;

		// layers
		case 0x10:
			return (layer[current_layer].x & 0xff00) >> 8;
		case 0x11:
			return layer[current_layer].x & 0xff;
		case 0x12:
			return (layer[current_layer].y & 0xff00) >> 8;
		case 0x13:
			return layer[current_layer].y & 0xff;
		case 0x14:
			return layer[current_layer].flags0;
		case 0x15:
			return layer[current_layer].flags1;
		case 0x18:
			return layer[current_layer].colors[0];
		case 0x19:
			return layer[current_layer].colors[1];
		case 0x1a:
			return layer[current_layer].colors[2];
		case 0x1b:
			return layer[current_layer].colors[3];
		case 0x1c:
			return (layer[current_layer].tiles_address & 0xff00) >> 8;
		case 0x1d:
			return layer[current_layer].tiles_address & 0xff;
		case 0x1e:
			return (layer[current_layer].colors_address & 0xff00) >> 8;
		case 0x1f:
			return layer[current_layer].colors_address & 0xff;

		// sprites
		case 0x20:
			return (sprite[current_sprite].x & 0xff00) >> 8;
		case 0x21:
			return sprite[current_sprite].x & 0xff;
		case 0x22:
			return (sprite[current_sprite].y & 0xff00) >> 8;
		case 0x23:
			return sprite[current_sprite].y & 0xff;
		case 0x24:
			return sprite[current_sprite].flags0;
		case 0x25:
			return sprite[current_sprite].flags1;
		case 0x26:
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
		case 0x02:
		case 0x03:
			// reserved
			break;
		case 0x04:
			bg_color = value;
			break;
		case 0x05:
			current_palette_index = value;
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
			if (current_palette_index & 0x80) {
				palette[current_palette_index] = (palette[current_palette_index] & 0xff00ffff) | (value << 16);
				crt_palette_color(current_palette_index);
			}
			break;
		case 0x0a:
			if (current_palette_index & 0x80) {
				palette[current_palette_index] = (palette[current_palette_index] & 0xffff00ff) | (value << 8);
				crt_palette_color(current_palette_index);
			}
			break;
		case 0x0b:
			if (current_palette_index & 0x80) {
				palette[current_palette_index] = (palette[current_palette_index] & 0xffffff00) | value;
				crt_palette_color(current_palette_index);
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
			layer[current_layer].x = (layer[current_layer].x & 0x00ff) | (value << 8);
			break;
		case 0x11:
			layer[current_layer].x = (layer[current_layer].x & 0xff00) | value;
			break;
		case 0x12:
			layer[current_layer].y = (layer[current_layer].y & 0x00ff) | (value << 8);
			break;
		case 0x13:
			layer[current_layer].y = (layer[current_layer].y & 0xff00) | value;
			break;
		case 0x14:
			layer[current_layer].flags0 = value & 0b00001111;
			break;
		case 0x15:
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
		case 0x1c:
			layer[current_layer].tiles_address = (layer[current_layer].tiles_address & 0x00ff) | (value << 8);
			break;
		case 0x1d:
			layer[current_layer].tiles_address = (layer[current_layer].tiles_address & 0xff00) | (value & 0xfe);
			break;
		case 0x1e:
			layer[current_layer].colors_address = (layer[current_layer].colors_address & 0x00ff) | (value << 8);
			break;
		case 0x1f:
			layer[current_layer].colors_address = (layer[current_layer].colors_address & 0xff00) | (value & 0xfe);
			break;

		// sprites
		case 0x20:
			sprite[current_sprite].x = (sprite[current_sprite].x & 0x00ff) | (value << 8);
			break;
		case 0x21:
			sprite[current_sprite].x = (sprite[current_sprite].x & 0xff00) | value;
			break;
		case 0x22:
			sprite[current_sprite].y = (sprite[current_sprite].y & 0x00ff) | (value << 8);
			break;
		case 0x23:
			sprite[current_sprite].y = (sprite[current_sprite].y & 0xff00) |  value;
			break;
		case 0x24:
			sprite[current_sprite].flags0 = value & 0b00110111;
			break;
		case 0x25:
			sprite[current_sprite].flags1 = value & 0b01010111;
			break;
		case 0x26:
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

	if (cycles_run >= CORE_CYCLES_PER_SCANLINE) {
		new_scanline = true;
		cycles_run -= CORE_CYCLES_PER_SCANLINE;
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

void vdc_t::crt_palette_color(uint8_t c)
{
	uint8_t high = 255 - ((255 - crt_contrast) / 4);
	uint8_t low = (255 - crt_contrast) / 4;

	uint8_t a = (palette[c] & 0xff000000) >> 24;
	uint8_t r = (palette[c] & 0xff0000) >> 16;
	uint8_t g = (palette[c] & 0xff00) >> 8;
	uint8_t b = palette[c] & 0xff;
	r = (((high - low) * r) / high) + low;
	g = (((high - low) * g) / high) + low;
	b = (((high - low) * b) / high) + low;
	crt_palette[c] = (a << 24) | (r << 16) | (g << 8) | b;
}

void vdc_t::init_crt_palette()
{
	for (int i=0; i<256; i++) {
		crt_palette_color(i);
	}
}
