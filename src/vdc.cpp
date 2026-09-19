// ---------------------------------------------------------------------
// vdc.cpp
// lime
//
// video display controller
//
// Copyright © 2025-2026 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include "vdc.hpp"
#include "common.hpp"
#include <cstdint>
#include <cstdio>

vdc_t::vdc_t(sn74ls148_t *t)
{
	sn74ls148 = t;

	dev_number_sn74ls148 = sn74ls148->connect_device(6, "vdc");
	printf("[vdc] Connecting to sn74ls148 at IPL 6 getting dev %i for mc68000\n", dev_number_exceptions);

    ram = new uint8_t[VDC_RAM];
    buffer = new uint32_t[VDC_XRES * VDC_YRES];

	calculate_crt_palette();
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
		sprite[i].flags0_bit67_transparency = 0;
		sprite[i].flags1_bit0_flip_h   = false;
		sprite[i].flags1_bit1_flip_v   = false;
		sprite[i].flags1_bit45_hstretch  = 0;
		sprite[i].flags1_bit67_vstretch = 0;
		sprite[i].flags2_bit01_hsize = 0b01;	// defaults to 8 pixels
		sprite[i].flags2_bit45_vsize = 0b01;	// defaults to 8 pixels
		sprite[i].index = 0;
		sprite[i].colors[0] = 0x01;
		sprite[i].colors[1] = 0xc2;
		sprite[i].colors[2] = 0xc7;
		sprite[i].colors[3] = 0xce;
		sprite[i].tileset_address = 0;
	}

	for (int i=0; i<4; i++) {
		layer[i].x = 0;
		layer[i].y = 0;
		layer[i].flags0_bit0_visible      = false;
		layer[i].flags0_bit2_transparent  = false;
		layer[i].flags0_bit3_color_memory = false;
		layer[i].flags1_bit0_flip_h       = false;
		layer[i].flags1_bit1_flip_v       = false;
        layer[i].flags1_bit45_hstretch = 0;
		layer[i].flags1_bit67_vstretch = 0;
		layer[i].flags2_bit01_hsize = 0b00;
		layer[i].flags2_bit45_vsize = 0b01;
		layer[i].tiles_address = 0;
		layer[i].colors_address = 0;
		layer[i].colors[0] = 0x01;
		layer[i].colors[1] = 0xc2;
		layer[i].colors[2] = 0xc7;
		layer[i].colors[3] = 0xce;
		layer[i].tileset_address = 0;
	}

	current_layer = 0;
	current_sprite = 0;

	cycles_run = 0;
	current_scanline = 0;
	irq_scanline = 0;
	new_scanline = true;

	border_color = 0x00;
	border_size = 0x00;
	bg_color = 0x00;

	irq_line = true;
	generate_interrupts = false;
}

void vdc_t::draw_scanline(uint16_t scanline)
{
	if (scanline < VDC_YRES) {

        if ((scanline < border_size) || (scanline >= (VDC_YRES - border_size))) {
            for (int x=0; x<VDC_XRES; x++) {
                buffer[(VDC_XRES * scanline) + x] = crt_palette[border_color];
            }
        } else {
            for (int x=0; x<VDC_XRES; x++) {
                buffer[(VDC_XRES * scanline) + x] = crt_palette[bg_color];
            }

            for (int l=3; l>=0; l--) {
                if (layer[l].flags0_bit0_visible) {
                    draw_scanline_layer(&layer[l], scanline);
                }

                for (uint8_t i=0; i<64; i++) {
                    uint8_t s = (64 * l) + (63 - i);
                    if (sprite[s].flags0_bit0_visible) {
                        draw_scanline_sprite(&sprite[s], scanline);
                    }
                }
            }
        }
	}
}

void vdc_t::draw_scanline_layer(layer_t *l, uint16_t sl)
{
	uint16_t y_in_layer = ((uint16_t)(sl - l->y)) % ((32 * (4 << l->flags2_bit45_vsize)) << l->flags1_bit67_vstretch);
	y_in_layer >>= l->flags1_bit67_vstretch;
	if (l->flags1_bit1_flip_v) y_in_layer = 255 - y_in_layer;

	uint8_t y_in_tile = y_in_layer % (4 << l->flags2_bit45_vsize);

	for (uint16_t scr_x = 0; scr_x < VDC_XRES; scr_x++)
	{
		uint16_t x_in_layer = ((uint16_t)(scr_x - l->x)) % (((128 * (4 << l->flags2_bit01_hsize)) << l->flags1_bit45_hstretch));
		x_in_layer >>= l->flags1_bit45_hstretch;
		if (l->flags1_bit0_flip_h) x_in_layer = (128 * (4 << l->flags2_bit01_hsize)) - 1 - x_in_layer;

		uint8_t px_in_byte = x_in_layer % 4;

		// << 7 heeft te maken met aantal tiles per lijn = 128 (een deel is niet zichtbaar)
		uint16_t index = (((y_in_layer / (4 << l->flags2_bit45_vsize)) << 7) + (x_in_layer / (4 << l->flags2_bit01_hsize))) & 0xffff;
		uint16_t tile_index = ram[(l->tiles_address + index) & 0xffffff];

		uint8_t color = ram[(l->colors_address + index) & 0xffff];

		// result has value 0b00, 0b01, 0b10 or 0b11
		uint8_t result = (ram[(
			l->tileset_address +
			(tile_index * (4 << l->flags2_bit45_vsize) * (1 << l->flags2_bit01_hsize)) +
			(y_in_tile * (1 << l->flags2_bit01_hsize)) +
			((x_in_layer >> 2) % (1 << l->flags2_bit01_hsize))
		) & 0xffffff] >> (2 * (3 - px_in_byte))) & 0b11;

		// if NOT (transparent AND 0b00) then pixel must be drawn
		if (!(l->flags0_bit2_transparent && !result)) {
			//buffer[(VDC_XRES * sl) + (l->flags1_bit0_flip_h ? (VDC_XRES - 1) - scr_x : scr_x)] = (l->flags0_bit3_color_memory && (result == 0b11)) ? crt_palette[color] : crt_palette[l->colors[result]];
			buffer[(VDC_XRES * sl) + scr_x] = (l->flags0_bit3_color_memory && (result == 0b11)) ? crt_palette[color] : crt_palette[l->colors[result]];
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

inline void vdc_t::draw_scanline_sprite(sprite_t *s, uint16_t sl)
{
	// Subtract sprite y position from scanline, remainder is y position in sprite.
	uint16_t y_in_sprite = (sl - s->y);

	y_in_sprite >>= s->flags1_bit67_vstretch;

	if (y_in_sprite < (4 << s->flags2_bit45_vsize)) {
		uint16_t width = ((4 << s->flags2_bit01_hsize) << s->flags1_bit45_hstretch);

		uint16_t start_x = 0;	// sprite by default not visible
		uint16_t end_x = 0;		// sprite by default not visible

		if (s->x < VDC_XRES) {	// sprite is on the left of the right edge
			start_x = s->x;
			end_x = ((s->x + width) > VDC_XRES) ? VDC_XRES : (s->x + width);
		} else if (s->x >= (0x10000 - width)) {
			end_x = s->x + width;
		}

		if (s->flags1_bit1_flip_v) y_in_sprite = ((4 << s->flags2_bit45_vsize) - 1) - y_in_sprite;

		for (uint16_t scr_x = start_x; scr_x < end_x; scr_x++) {
			uint16_t x = scr_x - s->x;

			x >>= s->flags1_bit45_hstretch;
			if (s->flags1_bit0_flip_h) x = ((4 << s->flags2_bit01_hsize) - 1) - x;

			// look up color value { 0b00, 0b01, 0b10, 0b11 } (result) from tileset
			uint8_t result =
				(
					ram
						[
							(
								s->tileset_address +
								(s->index * (4 << (s->flags2_bit01_hsize + s->flags2_bit45_vsize))) +
								(y_in_sprite << s->flags2_bit01_hsize) +
								//((x & 0x4) ? 1 : 0)) & 0xffff]
								(x >> 2)
							) & 0xffffff
						] >> (2 * (3 - (x%4)))
				) & 0b11;

			// if NOT (transparent AND 0b00) then pixel must be drawn
			if (!((s->flags0_bit2_transparent) && !result)) {
				uint32_t target = buffer[(VDC_XRES * sl) + scr_x];
				buffer[(VDC_XRES * sl) + scr_x] = blend(s->flags0_bit67_transparency, crt_palette[s->colors[result]], target);
			}
		}
	}
}

uint8_t vdc_t::io_read8(uint16_t address)
{
	switch (address & 0x7f) {
		case 0x00:
			// status register
			return irq_line ? 0b0 : 0b1;
		case 0x01:
			// control register
			return generate_interrupts ? 0b1 : 0b0;
		case 0x02:
            return border_color;
		case 0x03:
			return border_size;
		case 0x04:
			return bg_color;
		// case 0x05:
		// 	return current_palette_index;
		case 0x06:
			return current_layer;
		case 0x07:
			return current_sprite;
		// case 0x08:
		// 	return (palette[current_palette_index] & 0xff000000) >> 24;
		// case 0x09:
		// 	return (palette[current_palette_index] & 0x00ff0000) >> 16;
		// case 0x0a:
		// 	return (palette[current_palette_index] & 0x0000ff00) >>  8;
		// case 0x0b:
		// 	return (palette[current_palette_index] & 0x000000ff) >>  0;
		case 0x0c:
			return (current_scanline & 0xff00) >> 8;
		case 0x0d:
			return current_scanline & 0xff;
		case 0x0e:
			return (irq_scanline & 0xff00) >> 8;
		case 0x0f:
			return irq_scanline & 0xff;

		// layers
		case 0x40:
			return (layer[current_layer].x & 0xff00) >> 8;
		case 0x41:
			return layer[current_layer].x & 0xff;
		case 0x42:
			return (layer[current_layer].y & 0xff00) >> 8;
		case 0x43:
			return layer[current_layer].y & 0xff;
		case 0x44:
			return
				(layer[current_layer].flags0_bit0_visible      ? 0b00000001 : 0) |
				(layer[current_layer].flags0_bit2_transparent  ? 0b00000100 : 0) |
				(layer[current_layer].flags0_bit3_color_memory ? 0b00001000 : 0) ;
		case 0x45:
			return
				(layer[current_layer].flags1_bit0_flip_h    ? 0b00000001 : 0) |
				(layer[current_layer].flags1_bit1_flip_v    ? 0b00000010 : 0) |
				(layer[current_layer].flags1_bit45_hstretch             << 4) |
				(layer[current_layer].flags1_bit67_vstretch             << 6) ;
		case 0x46:
			return
				(layer[current_layer].flags2_bit01_hsize << 0) |
				(layer[current_layer].flags2_bit45_vsize << 4) ;
		case 0x48:
			return 0x00;
		case 0x49:
			return (layer[current_layer].tileset_address & 0xff0000) >> 16;
		case 0x4a:
			return (layer[current_layer].tileset_address & 0xff00) >> 8;
		case 0x4b:
			return layer[current_layer].tileset_address & 0xff;
		// case 0x4c:
		// 	return (layer[current_layer].tiles_address & 0xff00) >> 8;
		// case 0x4d:
		// 	return (layer[current_layer].colors_address & 0xff00) >> 8;
		case 0x50:
			return 0x00;
		case 0x51:
			return (layer[current_layer].tiles_address & 0xff0000) >> 16;
		case 0x52:
			return (layer[current_layer].tiles_address & 0xff00) >> 8;
		case 0x53:
			return layer[current_layer].tiles_address & 0xff;
		case 0x54:
			return 0x00;
		case 0x55:
			return (layer[current_layer].colors_address & 0xff0000) >> 16;
		case 0x56:
			return (layer[current_layer].colors_address & 0xff00) >> 8;
		case 0x57:
			return layer[current_layer].colors_address & 0xff;
		case 0x58:
			return layer[current_layer].colors[0];
		case 0x59:
			return layer[current_layer].colors[1];
		case 0x5a:
			return layer[current_layer].colors[2];
		case 0x5b:
			return layer[current_layer].colors[3];

		// sprites
		case 0x60:
			return (sprite[current_sprite].x & 0xff00) >> 8;
		case 0x61:
			return sprite[current_sprite].x & 0xff;
		case 0x62:
			return (sprite[current_sprite].y & 0xff00) >> 8;
		case 0x63:
			return sprite[current_sprite].y & 0xff;
		case 0x64:
			return
				(sprite[current_sprite].flags0_bit0_visible        ? 0b00000001 : 0) |
				(sprite[current_sprite].flags0_bit2_transparent    ? 0b00000100 : 0) |
				(sprite[current_sprite].flags0_bit67_transparency << 6             ) ;
		case 0x65:
			return
				(sprite[current_sprite].flags1_bit0_flip_h   ? 0b00000001 : 0) |
				(sprite[current_sprite].flags1_bit1_flip_v   ? 0b00000010 : 0) |
				(sprite[current_sprite].flags1_bit45_hstretch << 4           ) |
				(sprite[current_sprite].flags1_bit67_vstretch << 6           ) ;
		case 0x66:
			return
				(sprite[current_sprite].flags2_bit01_hsize << 0) |
				(sprite[current_sprite].flags2_bit45_vsize << 4) ;
		case 0x67:
			return sprite[current_sprite].index;
		case 0x68:
			return 0x00;
		case 0x69:
			return (sprite[current_sprite].tileset_address & 0xff0000) >> 16;
		case 0x6a:
			return (sprite[current_sprite].tileset_address & 0xff00) >> 8;
		case 0x6b:
			return sprite[current_sprite].tileset_address & 0xff;
		case 0x78:
			return sprite[current_sprite].colors[0];
		case 0x79:
			return sprite[current_sprite].colors[1];
		case 0x7a:
			return sprite[current_sprite].colors[2];
		case 0x7b:
			return sprite[current_sprite].colors[3];

		default:
			return 0xff;
	}
}

void vdc_t::io_write8(uint16_t address, uint8_t value)
{
	switch (address & 0x7f) {
		case 0x00:
			if ((value & 0b1) && !irq_line) {
				sn74ls148->release_line(dev_number_sn74ls148);
				irq_line = true;
			}
			break;
		case 0x01:
			generate_interrupts = (value & 0b1) ? true : false;
			break;
		case 0x02:
            border_color = value;
            break;
		case 0x03:
			border_size = value;
			break;
		case 0x04:
			bg_color = value;
			break;
		// case 0x05:
		// 	current_palette_index = value;
		// 	break;
		case 0x06:
			current_layer = value &0b11;
			break;
		case 0x07:
			current_sprite = value;
			break;
		// case 0x08:
		// 	// don't change, is always 0xff
		// 	break;
		// case 0x09:
		// 	palette[current_palette_index] = (palette[current_palette_index] & 0xff00ffff) | (value << 16);
		// 	calculate_crt_palette_entry(current_palette_index);
		// 	break;
		// case 0x0a:
		// 	palette[current_palette_index] = (palette[current_palette_index] & 0xffff00ff) | (value << 8);
		// 	calculate_crt_palette_entry(current_palette_index);
		// 	break;
		// case 0x0b:
		// 	palette[current_palette_index] = (palette[current_palette_index] & 0xffffff00) | value;
		// 	calculate_crt_palette_entry(current_palette_index);
		// 	break;
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
		case 0x40:
			layer[current_layer].x = (layer[current_layer].x & 0x00ff) | (value << 8);
			break;
		case 0x41:
			layer[current_layer].x = (layer[current_layer].x & 0xff00) | value;
			break;
		case 0x42:
			layer[current_layer].y = (layer[current_layer].y & 0x00ff) | (value << 8);
			break;
		case 0x43:
			layer[current_layer].y = (layer[current_layer].y & 0xff00) | value;
			break;
		case 0x44:
			layer[current_layer].flags0_bit0_visible      = value & 0b00000001 ? true : false;
			layer[current_layer].flags0_bit2_transparent  = value & 0b00000100 ? true : false;
			layer[current_layer].flags0_bit3_color_memory = value & 0b00001000 ? true : false;
			break;
		case 0x45:
			layer[current_layer].flags1_bit0_flip_h    = (value & 0b00000001) ? true : false;
			layer[current_layer].flags1_bit1_flip_v    = (value & 0b00000010) ? true : false;
			layer[current_layer].flags1_bit45_hstretch = (value & 0b00110000) >> 4;
			layer[current_layer].flags1_bit67_vstretch = (value & 0b11000000) >> 6;
			break;
		case 0x46:
			layer[current_layer].flags2_bit01_hsize = value & 0b11;
			layer[current_layer].flags2_bit45_vsize = (value & 0b00110000) >> 4;
			break;
		case 0x48:
			break;
		case 0x49:
			layer[current_layer].tileset_address = (layer[current_layer].tileset_address & 0xffff) | (value << 16);
			break;
		case 0x4a:
			layer[current_layer].tileset_address = (layer[current_layer].tileset_address & 0xff00ff) | (value << 8);
			break;
		case 0x4b:
			layer[current_layer].tileset_address = (layer[current_layer].tileset_address & 0xffff00) | value;
			break;
		// case 0x4c:
		// 	layer[current_layer].tiles_address = value << 8;
		// 	break;
		// case 0x4d:
		// 	layer[current_layer].colors_address = value << 8;
		// 	break;
		case 0x50:
			break;
		case 0x51:
			layer[current_layer].tiles_address = (layer[current_layer].tiles_address & 0xffff) | (value << 16);
			break;
		case 0x52:
			layer[current_layer].tiles_address = (layer[current_layer].tiles_address & 0xff00ff) | (value << 8);
			break;
		case 0x53:
			layer[current_layer].tiles_address = (layer[current_layer].tiles_address & 0xffff00) | value;
			break;
		case 0x54:
			break;
		case 0x55:
			layer[current_layer].colors_address = (layer[current_layer].colors_address & 0xffff) | (value << 16);
			break;
		case 0x56:
			layer[current_layer].colors_address = (layer[current_layer].colors_address & 0xff00ff) | (value << 8);
			break;
		case 0x57:
			layer[current_layer].colors_address = (layer[current_layer].colors_address & 0xffff00) | value;
			break;
		case 0x58:
			layer[current_layer].colors[0] = value;
			break;
		case 0x59:
			layer[current_layer].colors[1] = value;
			break;
		case 0x5a:
			layer[current_layer].colors[2] = value;
			break;
		case 0x5b:
			layer[current_layer].colors[3] = value;
			break;

		// sprites
		case 0x60:
			sprite[current_sprite].x = (sprite[current_sprite].x & 0x00ff) | (value << 8);
			break;
		case 0x61:
			sprite[current_sprite].x = (sprite[current_sprite].x & 0xff00) | value;
			break;
		case 0x62:
			sprite[current_sprite].y = (sprite[current_sprite].y & 0x00ff) | (value << 8);
			break;
		case 0x63:
			sprite[current_sprite].y = (sprite[current_sprite].y & 0xff00) |  value;
			break;
		case 0x64:
			sprite[current_sprite].flags0_bit0_visible        = value & 0b00000001 ? true : false;
			sprite[current_sprite].flags0_bit2_transparent    = value & 0b00000100 ? true : false;
			sprite[current_sprite].flags0_bit67_transparency = (value & 0b11000000) >> 6;
			break;
		case 0x65:
			sprite[current_sprite].flags1_bit0_flip_h   = value & 0b00000001 ? true : false;
			sprite[current_sprite].flags1_bit1_flip_v   = value & 0b00000010 ? true : false;
			sprite[current_sprite].flags1_bit45_hstretch  = (value & 0b00110000) >> 4;
			sprite[current_sprite].flags1_bit67_vstretch = (value & 0b11000000) >> 6;
			break;
		case 0x66:
			sprite[current_sprite].flags2_bit01_hsize = value & 0b11;
			sprite[current_sprite].flags2_bit45_vsize = (value & 0b00110000) >> 4;
			break;
		case 0x67:
			sprite[current_sprite].index = value;
			break;
		case 0x68:
			break;
		case 0x69:
			sprite[current_sprite].tileset_address = (sprite[current_sprite].tileset_address & 0xffff) | (value << 16);
			break;
		case 0x6a:
			sprite[current_sprite].tileset_address = (sprite[current_sprite].tileset_address & 0xff00ff) | (value << 8);
			break;
		case 0x6b:
			sprite[current_sprite].tileset_address = (sprite[current_sprite].tileset_address & 0xffff00) | value;
			break;
		case 0x78:
			sprite[current_sprite].colors[0] = value;
			break;
		case 0x79:
			sprite[current_sprite].colors[1] = value;
			break;
		case 0x7a:
			sprite[current_sprite].colors[2] = value;
			break;
		case 0x7b:
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
