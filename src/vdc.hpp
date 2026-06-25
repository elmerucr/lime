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
	//     | | +-+--------- horizontal stretch (0b00 = 1x, 0b01 = 2x, 0b10 = 4x, 0b11 = 8x)
	//     +-+------------- vertical stretch (0b00 = 1x, 0b01 = 2x, 0b10 = 4x, 0b11 = 8x)
	//
	// -----------------------------------------------------------------
	uint8_t flags1_bit45_hstretch;
	uint8_t flags1_bit67_vstretch;

	// -----------------------------------------------------------------
	// flags2
	//
	// bit 7 6 5 4 3 2 1 0
	//         | |     | |
	//         | |     +-+- hor size (0b00 = 4, 0b01 = 8, 0b10 = 16, 0b11 = 32)
	//         +-+--------- ver size (0b00 = 4, 0b01 = 8, 0b10 = 16, 0b11 = 32)
	// -----------------------------------------------------------------
	uint8_t flags2_bit01_hsize;
	uint8_t flags2_bit45_vsize;

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
	//     +-+------------- transparency of sprite (0b00 = no transp., 0b01/0b01/0b11 various degrees)
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
	//     | | +-+--------- horizontal stretch (0b00 = 1x, 0b01 = 2x, 0b10 = 4x, 0b11 = 8x)
	//     +-+------------- vertical stretch (0b00 = 1x, 0b01 = 2x, 0b10 = 4x, 0b11 = 8x)
	//
	// -----------------------------------------------------------------
	bool flags1_bit0_flip_h;
	bool flags1_bit1_flip_v;
	bool flags1_bit2_flip_xy;
	uint8_t flags1_bit45_hstretch;
	uint8_t flags1_bit67_vstretch;

	uint8_t index;

	uint8_t colors[4];

	uint16_t tileset_address;
};

class vdc_t {
private:
	font_cbm_8x8_t font;

    uint8_t border_color;
    uint8_t border_size;

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

	// Using Atari GTIA colors of https://lospec.com/palette-list/atari-8-bit-family-gtia
uint32_t palette[256] = {
	0xff000000, 0xff111111, 0xff222222, 0xff333333, 0xff444444, 0xff555555, 0xff666666, 0xff777777,
	0xff888888, 0xff999999, 0xffaaaaaa, 0xffbbbbbb, 0xffcccccc, 0xffdddddd, 0xffeeeeee, 0xffffffff,
	0xff190700, 0xff2a1800, 0xff3b2900, 0xff4c3a00, 0xff5d4b00, 0xff6e5c00, 0xff7f6d00, 0xff907e09,
	0xffa18f1a, 0xffb3a02b, 0xffc3b13c, 0xffd4c24d, 0xffe5d35e, 0xfff7e46f, 0xfffff582, 0xffffff96,
	0xff310000, 0xff3f0000, 0xff531700, 0xff642800, 0xff753900, 0xff864a00, 0xff975b0a, 0xffa86c1b,
	0xffb97d2c, 0xffca8e3d, 0xffdb9f4e, 0xffecb05f, 0xfffdc170, 0xffffd285, 0xffffe39c, 0xfffff4b2,
	0xff420404, 0xff4f0000, 0xff600800, 0xff711900, 0xff822a0d, 0xff933b1e, 0xffa44c2f, 0xffb55d40,
	0xffc66e51, 0xffd77f62, 0xffe89073, 0xfff9a183, 0xffffb298, 0xffffc3ae, 0xffffd4c4, 0xffffe5da,
	0xff410103, 0xff50000f, 0xff61001b, 0xff720f2b, 0xff83203c, 0xff94314d, 0xffa5425e, 0xffb6536f,
	0xffc76480, 0xffd87591, 0xffe986a2, 0xfffa97b3, 0xffffa8c8, 0xffffb9de, 0xffffcaef, 0xfffbdcf6,
	0xff330035, 0xff440041, 0xff55004c, 0xff660c5c, 0xff771d6d, 0xff882e7e, 0xff993f8f, 0xffaa50a0,
	0xffbb61b1, 0xffcc72c2, 0xffdd83d3, 0xffee94e4, 0xffffa5e4, 0xffffb6e9, 0xffffc7ee, 0xffffd8f3,
	0xff1d005c, 0xff2e0068, 0xff400074, 0xff511084, 0xff622195, 0xff7332a6, 0xff8443b7, 0xff9554c8,
	0xffa665d9, 0xffb776ea, 0xffc887eb, 0xffd998eb, 0xffe9a9ec, 0xfffbbaeb, 0xffffcbef, 0xffffdff9,
	0xff020071, 0xff13007d, 0xff240b8c, 0xff351c9d, 0xff462dae, 0xff573ebf, 0xff684fd0, 0xff7960e1,
	0xff8a71f2, 0xff9b82f7, 0xffac93f7, 0xffbda4f7, 0xffceb5f7, 0xffdfc6f7, 0xfff0d7f7, 0xffffe8f8,
	0xff000068, 0xff000a7c, 0xff081b90, 0xff192ca1, 0xff2a3db2, 0xff3b4ec3, 0xff4c5fd4, 0xff5d70e5,
	0xff6e81f6, 0xff7f92ff, 0xff90a3ff, 0xffa1b4ff, 0xffb2c5ff, 0xffc3d6ff, 0xffd4e7ff, 0xffe5f8ff,
	0xff000a4d, 0xff001b63, 0xff002c79, 0xff023d8f, 0xff134ea0, 0xff245fb1, 0xff3570c2, 0xff4681d3,
	0xff5792e4, 0xff68a3f5, 0xff79b4ff, 0xff8ac5ff, 0xff9bd6ff, 0xfface7ff, 0xffbdf8ff, 0xffceffff,
	0xff001a26, 0xff002b3c, 0xff003c52, 0xff004d68, 0xff065e7c, 0xff176f8d, 0xff28809e, 0xff3991af,
	0xff4aa2c0, 0xff5bb3d1, 0xff6cc4e2, 0xff7dd5f3, 0xff8ee6ff, 0xff9ff7ff, 0xffb0ffff, 0xffc1ffff,
	0xff01250a, 0xff023610, 0xff004622, 0xff005738, 0xff05684d, 0xff16795e, 0xff278a6f, 0xff389b80,
	0xff49ac91, 0xff5abda2, 0xff6bceb3, 0xff7cdfc4, 0xff8df0d5, 0xff9effe5, 0xffaffff1, 0xffc0fffd,
	0xff04260d, 0xff043811, 0xff054713, 0xff005a1b, 0xff106b1b, 0xff217c2c, 0xff328d3d, 0xff439e4e,
	0xff54af5f, 0xff65c070, 0xff76d181, 0xff87e292, 0xff98f3a3, 0xffa9ffb3, 0xffbaffbf, 0xffcbffcb,
	0xff00230a, 0xff003510, 0xff044613, 0xff155613, 0xff266713, 0xff377813, 0xff488914, 0xff599a25,
	0xff6aab36, 0xff7bbc47, 0xff8ccd58, 0xff9dde69, 0xffaeef7a, 0xffbfff8b, 0xffd0ff97, 0xffe1ffa3,
	0xff001707, 0xff0e2808, 0xff1f3908, 0xff304a08, 0xff415b08, 0xff526c08, 0xff637d08, 0xff748e0d,
	0xff859f1e, 0xff96b02f, 0xffa7c140, 0xffb8d251, 0xffc9e362, 0xffdaf473, 0xffebff82, 0xfffcff8e,
	0xff1b0701, 0xff2c1801, 0xff3c2900, 0xff4d3b00, 0xff5f4c00, 0xff705e00, 0xff816f00, 0xff938009,
	0xffa4921a, 0xffb2a02b, 0xffc7b43d, 0xffd8c64e, 0xffead760, 0xfff6e46f, 0xfffffa84, 0xffffff99
};

	uint8_t crt_contrast{0xee};
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
