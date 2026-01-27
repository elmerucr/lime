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
	printf("[vdc] Connecting to exceptions getting dev %i for mc6809\n", dev_number_exceptions);

	dev_number_sn74ls148 = sn74ls148->connect_device(6, "vdc");
	printf("[vdc] Connecting to sn74ls148 at IPL 6 getting dev %i for mc68000\n", dev_number_exceptions);

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
		sprite[i].flags0_bit0_visible        = false;
		sprite[i].flags0_bit2_transparent    = false;
		sprite[i].flags0_bit4_xpos_rel_layer = false;
		sprite[i].flags0_bit5_ypos_rel_layer = false;
		sprite[i].flags0_bit67_transparency = 0;
		sprite[i].flags1_bit0_flip_h   = false;
		sprite[i].flags1_bit1_flip_v   = false;
		sprite[i].flags1_bit2_flip_xy  = false;
		sprite[i].flags1_bit45_width  = 0;
		sprite[i].flags1_bit67_height = 0;
		sprite[i].index = 0;
		sprite[i].colors[0] = 0b00;
		sprite[i].colors[1] = 0b01;
		sprite[i].colors[2] = 0b10;
		sprite[i].colors[3] = 0b11;
		sprite[i].tileset_address = VDC_TILESET_ADDRESS;
	}

	for (int i=0; i<4; i++) {
		layer[i].x = 0;
		layer[i].y = 0;
		layer[i].flags0_bit0_visible      = false;
		layer[i].flags0_bit1_bitmapped    = false;
		layer[i].flags0_bit2_transparent  = false;
		layer[i].flags0_bit3_color_memory = false;
        layer[i].flags1_bit45_width  = 0;
		layer[i].flags1_bit67_height = 0;
		layer[i].colors[0] = 0b00;
		layer[i].colors[1] = 0b01;
		layer[i].colors[2] = 0b10;
		layer[i].colors[3] = 0b11;
		layer[i].tileset_address = VDC_TILESET_ADDRESS;
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

	calculate_crt_palette();
}

void vdc_t::draw_scanline(uint16_t scanline)
{
	if (scanline < VDC_YRES) {

		for (int x=0; x<VDC_XRES; x++) {
			buffer[(VDC_XRES * scanline) + x] = crt_palette[bg_color];
		}

		for (int l=3; l>=0; l--) {
			if (layer[l].flags0_bit0_visible) {
				draw_scanline_layer(&layer[l], scanline);
			}

			for (uint8_t i=0; i<64; i++) {
				draw_scanline_sprite(&sprite[(64*l) + (63-i)], scanline, &layer[l]);
			}
		}
	}
}

void vdc_t::draw_scanline_layer(layer_t *l, uint16_t sl)
{
	uint16_t y = (l->y + sl) & ((0x100 << l->flags1_bit67_height) - 1);

	//if (l->flags1_bit6_double_h) y >>= 1;
	y >>= l->flags1_bit67_height;

	uint8_t y_in_tile = y % 8;

	for (uint16_t scr_x = 0; scr_x < VDC_XRES; scr_x++)
	{
		uint16_t x = (l->x + scr_x) & ((0x200 << l->flags1_bit45_width) - 1);

		//if (l->flags1_bit4_double_w) x >>= 1;
		x >>= l->flags1_bit45_width;

		uint8_t  px = x % 4;

		uint16_t tile_index = (l->tiles_address  + ((y >> 3) << 6) + (x >> 3)) & 0xffff;
		if (!l->flags0_bit1_bitmapped) tile_index = ram[tile_index];

		uint8_t  color_index = ram[(l->colors_address + ((y >> 3) << 6) + (x >> 3)) & 0xffff];

		// result has value 0b00, 0b01, 0b10 or 0b11
		uint8_t result = (ram[(l->tileset_address + (tile_index << 4) + (y_in_tile << 1) + ((x & 0x4) ? 1 : 0)) & 0xffff] >> (2 * (3 - px))) & 0b11;

		// if NOT (transparent AND 0b00) then pixel must be drawn
		if (!(l->flags0_bit2_transparent && !result)) {
			buffer[(VDC_XRES * sl) + scr_x] = (l->flags0_bit3_color_memory && (result == 0b11)) ? crt_palette[color_index] : crt_palette[l->colors[result]];
		}
	}
}

inline uint32_t blend(uint8_t transparency, uint32_t source, uint32_t target)
{
	// transparency must be element of { 0, 3 }
	return
		((((4 - transparency) * (source & 0x00ff00ff) + (transparency * (target & 0x00ff00ff))) >> 2) & 0x00ff00ff) |
		((((4 - transparency) * (source & 0x0000ff00) + (transparency * (target & 0x0000ff00))) >> 2) & 0x0000ff00) |
		0xff000000;
}

inline void vdc_t::draw_scanline_sprite(sprite_t *s, uint16_t sl, layer_t *l)
{
	if (s->flags0_bit0_visible)
	{
		// determine real y position of sprite and correct if relative to layer
		uint16_t y = s->y - ((s->flags0_bit5_ypos_rel_layer) ? l->y : 0);

		// Subtract sprite y position from scanline, remainder is y position
		// in sprite. It wraps around 256 (height of 32 tiles of height 8 each)
		uint16_t y_in_sprite = (sl - y) & ((0x100 << l->flags1_bit67_height) - 1);

        y_in_sprite >>= s->flags1_bit67_height;

		if (y_in_sprite < 8) {
			// find real x postion of sprite correct for relative to layer or not
			uint16_t real_x = s->x - ((s->flags0_bit4_xpos_rel_layer) ? l->x : 0);

			real_x &= ((0x200 << l->flags1_bit45_width) - 1);

			uint16_t start_x, end_x;

			uint16_t width = (8 << s->flags1_bit45_width);

			if (real_x < VDC_XRES) {
				// sprite is on the left of the right edge
				start_x = real_x;
				end_x = ((real_x + width) > VDC_XRES) ? VDC_XRES : (real_x + width);
			} else if (real_x >= ((0x200 << l->flags1_bit45_width) - width)) {
				//
				start_x = 0;
				end_x = (real_x + width) & ((0x200 << l->flags1_bit45_width) - 1);
			} else {
				// not visible
				start_x = end_x = 0;
			}

			if (s->flags1_bit1_flip_v) y_in_sprite = 7 - y_in_sprite;

			for (uint16_t scr_x = start_x; scr_x < end_x; scr_x++) {
				uint16_t x = scr_x - real_x;

				//if (s->flags1_bit4_double_w) x >>= 1;
                x >>= s->flags1_bit45_width;
				if (s->flags1_bit0_flip_h) x = 7 - x;
				if (s->flags1_bit2_flip_xy) { uint8_t t = x; x = y_in_sprite; y_in_sprite = t; }

				// look up color value { 0b00, 0b01, 0b10, 0b11 } (result) from tileset
				uint8_t result = (ram[(s->tileset_address + (s->index << 4) + (y_in_sprite << 1) + ((x & 0x4) ? 1 : 0)) & 0xffff] >> (2 * (3 - (x%4)))) & 0b11;

				// if NOT (transparent AND 0b00) then pixel must be drawn
				if (!((s->flags0_bit2_transparent) && !result)) {
					uint32_t target = buffer[(VDC_XRES * sl) + scr_x];
					buffer[(VDC_XRES * sl) + scr_x] = blend(s->flags0_bit67_transparency, crt_palette[s->colors[result]], target);
				}

				// restore y, if xy flip was done before
				if (s->flags1_bit2_flip_xy) y_in_sprite = x;
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
			return
				(layer[current_layer].flags0_bit0_visible      ? 0b00000001 : 0) |
				(layer[current_layer].flags0_bit1_bitmapped    ? 0b00000010 : 0) |
				(layer[current_layer].flags0_bit2_transparent  ? 0b00000100 : 0) |
				(layer[current_layer].flags0_bit3_color_memory ? 0b00001000 : 0) ;
		case 0x15:
			return
				(layer[current_layer].flags1_bit45_width  << 4) |
				(layer[current_layer].flags1_bit67_height << 6) ;
		case 0x16:
			return (layer[current_layer].colors_address & 0xff00) >> 8;
		case 0x17:
			return layer[current_layer].colors_address & 0xff;
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
			return (layer[current_layer].tileset_address & 0xff00) >> 8;
		case 0x1f:
			return layer[current_layer].tileset_address & 0xff;

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
			return
				(sprite[current_sprite].flags0_bit0_visible        ? 0b00000001 : 0) |
				(sprite[current_sprite].flags0_bit2_transparent    ? 0b00000100 : 0) |
				(sprite[current_sprite].flags0_bit4_xpos_rel_layer ? 0b00010000 : 0) |
				(sprite[current_sprite].flags0_bit5_ypos_rel_layer ? 0b00100000 : 0) |
				(sprite[current_sprite].flags0_bit67_transparency << 6             ) ;
		case 0x25:
			return
				(sprite[current_sprite].flags1_bit0_flip_h   ? 0b00000001 : 0) |
				(sprite[current_sprite].flags1_bit1_flip_v   ? 0b00000010 : 0) |
				(sprite[current_sprite].flags1_bit2_flip_xy	 ? 0b00000100 : 0) |
				(sprite[current_sprite].flags1_bit45_width << 4              ) |
				(sprite[current_sprite].flags1_bit67_height << 6             ) ;
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
		case 0x2e:
			return (sprite[current_sprite].tileset_address & 0xff00) >> 8;
		case 0x2f:
			return sprite[current_sprite].tileset_address & 0xff;

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
				calculate_crt_palette_entry(current_palette_index);
			}
			break;
		case 0x0a:
			if (current_palette_index & 0x80) {
				palette[current_palette_index] = (palette[current_palette_index] & 0xffff00ff) | (value << 8);
				calculate_crt_palette_entry(current_palette_index);
			}
			break;
		case 0x0b:
			if (current_palette_index & 0x80) {
				palette[current_palette_index] = (palette[current_palette_index] & 0xffffff00) | value;
				calculate_crt_palette_entry(current_palette_index);
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
			layer[current_layer].flags0_bit0_visible      = value & 0b00000001 ? true : false;
			layer[current_layer].flags0_bit1_bitmapped    = value & 0b00000010 ? true : false;
			layer[current_layer].flags0_bit2_transparent  = value & 0b00000100 ? true : false;
			layer[current_layer].flags0_bit3_color_memory = value & 0b00001000 ? true : false;
			break;
		case 0x15:
			layer[current_layer].flags1_bit45_width  = (value & 0b00110000) >> 4;
			layer[current_layer].flags1_bit67_height = (value & 0b11000000) >> 6;
			break;
		case 0x16:
			layer[current_layer].colors_address = (layer[current_layer].colors_address & 0x00ff) | (value << 8);
			break;
		case 0x17:
			layer[current_layer].colors_address = (layer[current_layer].colors_address & 0xff00) | (value & 0xfe);
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
			layer[current_layer].tileset_address = (layer[current_layer].tileset_address & 0x00ff) | (value << 8);
			break;
		case 0x1f:
			layer[current_layer].tileset_address = (layer[current_layer].tileset_address & 0xff00) | (value & 0xfe);
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
			sprite[current_sprite].flags0_bit0_visible        = value & 0b00000001 ? true : false;
			sprite[current_sprite].flags0_bit2_transparent    = value & 0b00000100 ? true : false;
			sprite[current_sprite].flags0_bit4_xpos_rel_layer = value & 0b00010000 ? true : false;
			sprite[current_sprite].flags0_bit5_ypos_rel_layer = value & 0b00100000 ? true : false;
			sprite[current_sprite].flags0_bit67_transparency = (value & 0b11000000) >> 6;

			break;
		case 0x25:
			sprite[current_sprite].flags1_bit0_flip_h   = value & 0b00000001 ? true : false;
			sprite[current_sprite].flags1_bit1_flip_v   = value & 0b00000010 ? true : false;
			sprite[current_sprite].flags1_bit2_flip_xy  = value & 0b00000100 ? true : false;
			sprite[current_sprite].flags1_bit45_width  = (value & 0b00110000) >> 4;
			sprite[current_sprite].flags1_bit67_height = (value & 0b11000000) >> 6;
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
		case 0x2e:
			sprite[current_sprite].tileset_address = (sprite[current_sprite].tileset_address & 0x00ff) | (value << 8);
			break;
		case 0x2f:
			sprite[current_sprite].tileset_address = (sprite[current_sprite].tileset_address & 0xff00) | (value & 0xfe);
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

void vdc_t::calculate_crt_palette_entry(uint8_t c)
{
	uint8_t high = 255 - ((255 - crt_contrast) / 4);
	uint8_t low  = (255 - crt_contrast) / 4;

	uint8_t a = (palette[c] & 0xff000000) >> 24;
	uint8_t r = (palette[c] & 0xff0000) >> 16;
	uint8_t g = (palette[c] & 0xff00) >> 8;
	uint8_t b =  palette[c] & 0xff;

	r = (((high - low) * r) / high) + low;
	g = (((high - low) * g) / high) + low;
	b = (((high - low) * b) / high) + low;

	crt_palette[c] = (a << 24) | (r << 16) | (g << 8) | b;
}

void vdc_t::calculate_crt_palette()
{
	for (int i=0; i<256; i++) {
		calculate_crt_palette_entry(i);
	}
}
