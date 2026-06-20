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

	// using gtia colors of https://lospec.com/palette-list/atari-8-bit-series-gtia
	uint32_t palette[256] = {
		0xff000000, 0xff111111, 0xff222222, 0xff333333, 0xff444444, 0xff555555, 0xff666666, 0xff777777,
		0xff888888, 0xff999999, 0xffaaaaaa, 0xffbbbbbb, 0xffcccccc, 0xffdddddd, 0xffeeeeee, 0xffffffff,
		0xff091900, 0xff192806, 0xff29370d, 0xff3a4714, 0xff4a561b, 0xff5a6522, 0xff6b7529, 0xff7b8430,
		0xff8c9336, 0xff9ca33d, 0xffacb244, 0xffbdc14b, 0xffcdd152, 0xffdee059, 0xffeeef60, 0xffffff67,
		0xff300000, 0xff3d1108, 0xff4b2211, 0xff593319, 0xff674422, 0xff75552a, 0xff826633, 0xff90773b,
		0xff9e8844, 0xffac994c, 0xffbaaa55, 0xffc7bb5d, 0xffd5cc66, 0xffe3dd6e, 0xfff1ee77, 0xffffff80,
		0xff4b0000, 0xff570f0c, 0xff631e18, 0xff6f2e24, 0xff7a3d30, 0xff874d3c, 0xff935c49, 0xff9f6b55,
		0xffab7b61, 0xffb68a6d, 0xffc39a79, 0xffcfa986, 0xffdbb892, 0xffe6c89e, 0xfff3d7aa, 0xffffe7b7,
		0xff550000, 0xff600e10, 0xff6b1c21, 0xff772a32, 0xff823843, 0xff8d4654, 0xff995465, 0xffa46276,
		0xffaf7187, 0xffbb7f98, 0xffc68da9, 0xffd19bba, 0xffdda9cb, 0xffe8b7dc, 0xfff3c5ed, 0xffffd4fe,
		0xff4c0047, 0xff570d53, 0xff631b5f, 0xff6f286b, 0xff7b3678, 0xff874384, 0xff935190, 0xff9f5e9c,
		0xffab6ca9, 0xffb779b5, 0xffc387c1, 0xffcf94cd, 0xffdba2da, 0xffe7afe6, 0xfff3bdf2, 0xffffcbff,
		0xff30007e, 0xff3b0b85, 0xff49198d, 0xff572796, 0xff65349f, 0xff7242a7, 0xff8050b0, 0xff8e5db8,
		0xff9c6bc1, 0xffa979c9, 0xffb786d2, 0xffc594db, 0xffd3a2e3, 0xffe0afec, 0xffeebdf4, 0xfffccbfd,
		0xff0a0097, 0xff1a0e9d, 0xff2a1da4, 0xff3b2cab, 0xff4b3ab2, 0xff5b49b9, 0xff6c58c0, 0xff7c67c7,
		0xff8c75ce, 0xff9c84d5, 0xffad93dc, 0xffbda2e3, 0xffceb0ea, 0xffdebff1, 0xffeecef8, 0xffffddff,
		0xff00008e, 0xff0c0d94, 0xff1b1e9c, 0xff2a2ea3, 0xff393eab, 0xff484eb2, 0xff575eba, 0xff666ec1,
		0xff747ec9, 0xff838fd0, 0xff929fd8, 0xffa1afdf, 0xffb0bfe6, 0xffbfcfee, 0xffcedff5, 0xffddeffd,
		0xff000e64, 0xff0c1e6e, 0xff192e78, 0xff263e83, 0xff324e8d, 0xff3f5e97, 0xff4c6ea2, 0xff587eac,
		0xff658eb6, 0xff729ec1, 0xff7eaecb, 0xff8bbed5, 0xff98cee0, 0xffa4deea, 0xffb1eef4, 0xffbeffff,
		0xff002422, 0xff09302e, 0xff153f3d, 0xff204d4c, 0xff2c5c5a, 0xff376a69, 0xff427978, 0xff4e8786,
		0xff599695, 0xff65a4a4, 0xff70b3b2, 0xff7cc1c1, 0xff87d0d0, 0xff92dfde, 0xff9eeded, 0xffa9fcfc,
		0xff003200, 0xff0b3f0e, 0xff164d1c, 0xff225b2b, 0xff2d6839, 0xff397648, 0xff448456, 0xff509164,
		0xff5b9f73, 0xff67ad81, 0xff72ba90, 0xff7ec89e, 0xff89d6ac, 0xff95e3bb, 0xffa0f1c9, 0xffacffd8,
		0xff003400, 0xff0c410a, 0xff194f14, 0xff265c1e, 0xff336a28, 0xff407732, 0xff4c853c, 0xff599246,
		0xff66a050, 0xff73ad5a, 0xff80bb64, 0xff8cc86e, 0xff99d678, 0xffa6e382, 0xffb3f18c, 0xffc0ff97,
		0xff002a00, 0xff0f3807, 0xff1e460e, 0xff2d5416, 0xff3c621d, 0xff4b7124, 0xff5a7f2c, 0xff698d33,
		0xff799b3b, 0xff88a942, 0xff97b849, 0xffa6c651, 0xffb5d458, 0xffc4e260, 0xffd3f067, 0xffe3ff6f,
		0xff0d1700, 0xff1d2606, 0xff2d350d, 0xff3d4514, 0xff4d541b, 0xff5d6422, 0xff6d7329, 0xff7d8330,
		0xff8e9237, 0xff9ea23e, 0xffaeb145, 0xffbec14c, 0xffced053, 0xffdee05a, 0xffeeef61, 0xffffff68,
		0xff330000, 0xff401008, 0xff4e2111, 0xff5b321a, 0xff694323, 0xff77542c, 0xff846535, 0xff92763e,
		0xff9f8646, 0xffad974f, 0xffbba858, 0xffc8b961, 0xffd6ca6a, 0xffe3db73, 0xfff1ec7c, 0xfffffd85
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
