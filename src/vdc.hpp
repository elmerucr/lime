// ---------------------------------------------------------------------
// vdc.hpp
// lime
//
// video display controller
// ---------------------------------------------------------------------

// ---------------------------------------------------------------------
// sprites 0-63      front
// layer_0             .
// sprites 64-127      .
// layer 1             .
// sprites 128-191     .
// layer 2             .
// sprites 192-255     .
// layer 3            back
// ---------------------------------------------------------------------

// ---------------------------------------------------------------------
// Registers in vdc
// ---------------------------------------------------------------------
// 0x00: Status register
//
// bit 7 6 5 4 3 2 1 0
//                   |
//                   +- read:  0 = no irq / 1 = irq
//                      write: 0 = do nothing / 1 = acknowledge irq
// ---------------------------------------------------------------------
// 0x01: Control register
//
// bit 7 6 5 4 3 2 1 0
//                   |
//                   +- read/write:	0 = vdc no irq's / 1 = irq's
// ---------------------------------------------------------------------
// 0x02: reserved
// 0x03: reserved
// 0x04: bg color
// 0x05: current palette index
// 0x06: Current Layer (0-3)
// 0x07: Current Sprite (0-255)
// 0x0c: Current scanline MSB
// 0x0d: Current scanline LSB
// 0x0e: IRQ scanline MSB
// 0x0f: IRQ scanline LSB
//
// 0x20: x_pos current sprite MSB
// 0x21: x_pos current sprite LSB
// 0x22: y_pos current sprite MSB
// 0x23: y_pos current sprite LSB
// 0x24: flags0 current sprite
// 0x25: flags1 current sprite
// ---------------------------------------------------------------------

#ifndef VDC_HPP
#define VDC_HPP

#include <cstdint>
#include "common.hpp"
#include "exceptions.hpp"
#include "sn74ls148.hpp"
#include "font_cbm_8x8.hpp"

struct layer_t {
	uint16_t x;
	uint16_t y;

	// -----------------------------------------------------------------
	// flags0
	//
	// bit 7 6 5 4 3 2 1 0
	//             | | | |
	//             | | | +- hidden (0) / visible (1)
	//             | | +--- tilemode (0) / bitmapped (1)
	//             | +----- 0b00 patterns code to opaque (0) or transparent (1)
	//             +------- 0b11 assumes color 3 (0) or color from memory (1) (color per tile)
	//
	// -----------------------------------------------------------------
	bool flags0_bit0_visible;
	bool flags0_bit1_bitmapped;
	bool flags0_bit2_transparent;
	bool flags0_bit3_color_memory;

	// -----------------------------------------------------------------
	// flags1
	//
	// bit 7 6 5 4 3 2 1 0
	//     | | | |
	//     | | +-+--------- bits coding for horizontal size (0b00 = 1x, 0b01 = 2x, 0b10 = 4x, 0b11 = 8x)
	//     +-+------------- bits coding for vertical size (0b00 = 1x, 0b01 = 2x, 0b10 = 4x, 0b11 = 8x)
	//
	// -----------------------------------------------------------------
	uint8_t flags1_bit45_width;
	uint8_t flags1_bit67_height;

	uint8_t colors[4];

	uint16_t tileset_address;
	uint16_t tiles_address;
	uint16_t colors_address;
};

struct sprite_t {
	uint16_t x;
	uint16_t y;

	// -----------------------------------------------------------------
	// flags0
	//
	// bit 7 6 5 4 3 2 1 0
	//     | | | |   |   |
	//     | | | |   |   +- hidden (0) / visible (1)
	//     | | | |   +----- 0b00 patterns code to opaque (0) or transparent (1)
	//     | | | |
	//     | | | +--------- x pos relative to screen (0) or associated layer (1)
	//     | | +----------- y pos relative to screen (0) or associated layer (1)
	//     +-+------------- transparency of sprite (0b00 = no transp., 0b01/0b01/0b11 variious degrees)
	//
	// -----------------------------------------------------------------
	bool flags0_bit0_visible;
	bool flags0_bit2_transparent;
	bool flags0_bit4_xpos_rel_layer;
	bool flags0_bit5_ypos_rel_layer;
	uint8_t flags0_bit67_transparency;

	// -----------------------------------------------------------------
	// flags1
	//
	// bit 7 6 5 4 3 2 1 0
	//     | | | |   | | |
	//     | | | |   | | +- flip h  (1)
	//     | | | |   | +--- flip v  (1)
	//     | | | |   +----- flip xy (1)
	//     | | +-+--------- bits coding for horizontal size (0b00 = 1x, 0b01 = 2x, 0b10 = 4x, 0b11 = 8x)
	//     +-+------------- bits coding for vertical size (0b00 = 1x, 0b01 = 2x, 0b10 = 4x, 0b11 = 8x)
	//
	// -----------------------------------------------------------------
	bool flags1_bit0_flip_h;
	bool flags1_bit1_flip_v;
	bool flags1_bit2_flip_xy;
	uint8_t flags1_bit45_width;
	uint8_t flags1_bit67_height;

	uint8_t index;

	uint8_t colors[4];

	uint16_t tileset_address;
};

class vdc_t {
private:
	font_cbm_8x8_t font;

	uint8_t bg_color;

	uint16_t current_scanline;
	int32_t cycles_run;

	bool new_scanline;

	uint8_t current_layer;
	uint8_t current_sprite;
	uint8_t current_palette_index;

	void draw_scanline(uint16_t scanline);
	void draw_scanline_layer(layer_t *l, uint16_t sl);
	void draw_scanline_sprite(sprite_t *s, uint16_t sl, layer_t *t);

	exceptions_ic *exceptions;
	sn74ls148_t *sn74ls148;

	bool irq_line;
	bool generate_interrupts;
	uint16_t irq_scanline;

public:
	vdc_t(exceptions_ic *e, sn74ls148_t *t);
	~vdc_t();

	uint8_t dev_number_exceptions;
	uint8_t dev_number_sn74ls148;

	uint8_t *ram;

	layer_t  layer[4];
	sprite_t sprite[256];

	uint32_t *buffer;

	uint8_t io_read8(uint16_t address);
	void    io_write8(uint16_t address, uint8_t value);

	inline int32_t  get_cycles_run() { return cycles_run; }
	inline uint16_t get_current_scanline() { return current_scanline; }
	inline uint16_t get_irq_scanline() { return irq_scanline; }
	inline bool     get_generate_interrupts() { return generate_interrupts; }

	void reset();

	// returns true if frame done
	bool run(uint32_t number_of_cycles);

	bool started_new_scanline() { return new_scanline; }

	uint32_t palette[256] = {
		LIME_COLOR_00,  LIME_COLOR_01,  LIME_COLOR_02,  LIME_COLOR_03,		// 0x00 - 0x0f
		LIME_COLOR_04,  LIME_COLOR_05,  LIME_COLOR_06,  LIME_COLOR_07,
		LIME_COLOR_08,  LIME_COLOR_09,  LIME_COLOR_0A,  LIME_COLOR_0B,
		LIME_COLOR_0C,  LIME_COLOR_0D,  LIME_COLOR_0E,  LIME_COLOR_0F,

		PUNCH_BLACK,  PUNCH_WHITE,      PUNCH_RED,       PUNCH_CYAN,		// 0x10 - 0x1f (C64 like)
		PUNCH_PURPLE, PUNCH_GREEN,      PUNCH_BLUE,      PUNCH_YELLOW,
		PUNCH_ORANGE, PUNCH_BROWN,      PUNCH_LIGHTRED,  PUNCH_DARKGREY,
		PUNCH_GREY,   PUNCH_LIGHTGREEN, PUNCH_LIGHTBLUE, PUNCH_LIGHTGREY,

		PICOTRON_V5_00, PICOTRON_V5_01, PICOTRON_V5_02, PICOTRON_V5_03,		// 0x20 - 0x3f
		PICOTRON_V5_04, PICOTRON_V5_05, PICOTRON_V5_06, PICOTRON_V5_07,
		PICOTRON_V5_08, PICOTRON_V5_09, PICOTRON_V5_0A, PICOTRON_V5_0B,
		PICOTRON_V5_0C, PICOTRON_V5_0D, PICOTRON_V5_0E, PICOTRON_V5_0F,
		PICOTRON_V5_10, PICOTRON_V5_11, PICOTRON_V5_12, PICOTRON_V5_13,
		PICOTRON_V5_14, PICOTRON_V5_15, PICOTRON_V5_16, PICOTRON_V5_17,
		PICOTRON_V5_18, PICOTRON_V5_19, PICOTRON_V5_1A, PICOTRON_V5_1B,
		PICOTRON_V5_1C, PICOTRON_V5_1D, PICOTRON_V5_1E, PICOTRON_V5_1F,

		// 0x40 - 0x7f
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,

		// 0x80 - 0xff
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00,
		LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00, LIME_COLOR_00
	};

	uint8_t crt_contrast{0xcc};
	uint32_t crt_palette[256];
	void calculate_crt_palette_entry(uint8_t c);
	void calculate_crt_palette();

	uint8_t get_crt_contrast() {
		return crt_contrast;
	}

	void change_crt_contrast() {
		if (crt_contrast > 0xf0) {
			crt_contrast = 0x00;
		} else {
			crt_contrast += 0x11;
		}
		calculate_crt_palette();
	}
};

#endif
