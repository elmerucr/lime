// ---------------------------------------------------------------------
// vdc.hpp
// lime
//
// video display controller
// ---------------------------------------------------------------------

// ---------------------------------------------------------------------
//
// sprites 0-63      front
// layer_0             .
// sprites 64-127      .
// layer 1             .
// sprites 128-191     .
// layer 2             .
// sprites 192-255     .
// layer 3           back
//
// 0x00: SR
// 0x01: CR
// 0x02: read
// 0x03:
// 0x04: bg color
//
// 0x06: current layer (0-3)
// 0x07: current sprite (0-255)
//
// 0x18: x_pos current sprite
// 0x19: y_pos current sprite
//
// ---------------------------------------------------------------------

#ifndef VC_HPP
#define VC_HPP

#include <cstdint>
#include "common.hpp"
#include "exceptions.hpp"
#include "font_cbm_8x8.hpp"

const uint32_t palette[256] = {
	LIME_COLOR_00,
	LIME_COLOR_01,
	LIME_COLOR_02,
	LIME_COLOR_03,
	LIME_COLOR_04,
	LIME_COLOR_05,
	LIME_COLOR_06,
	LIME_COLOR_07,
	LIME_COLOR_08,
	LIME_COLOR_09,
	LIME_COLOR_0A,
	LIME_COLOR_0B,
	LIME_COLOR_0C,
	LIME_COLOR_0D,
	LIME_COLOR_0E,
	LIME_COLOR_0F,
	0xff141414,		// need scanlines!
	C64_WHITE,
	C64_RED,
	C64_CYAN,
	C64_PURPLE,
	C64_GREEN,
	C64_BLUE,
	C64_YELLOW,
	C64_ORANGE,
	C64_BROWN,
	C64_LIGHTRED,
	C64_DARKGREY,
	C64_GREY,
	C64_LIGHTGREEN,
	C64_LIGHTBLUE,
	C64_LIGHTGREY,

	0xff141414,	// Picotron Palette WIP v 5 (only black is changed)
	0xff742f29,
	0xffab5236,
	0xffff856d,
	0xffffccaa,
	0xffff004d,
	0xffc3004c,
	0xff7e2553,

	0xff492d38,
	0xff5f574f,
	0xffa28879,
	0xffc2c3c7,
	0xfffff1e8,
	0xffffacc5,
	0xffff77a8,
	0xffe40dab,

	0xff1d2b53,
	0xff1c5eac,
	0xff00a5a1,
	0xff29adff,
	0xff64dff6,
	0xffbd9adf,
	0xff83769c,
	0xff754e97,

	0xff125359,
	0xff008751,
	0xff00b251,
	0xff00e436,
	0xff90ec42,
	0xffffec27,
	0xffffa300,
	0xffeb6b00,

	0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505,
	0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505,
	0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505,
	0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505,
	0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505,
	0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505,
	0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505,
	0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505,
	0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505,
	0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505,
	0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505,
	0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505, 0xff050505
};

struct layer_t
{
	uint8_t x{0};
	uint8_t y{0};

	// -----------------------------------------------------------------
	// flags
	//
	// bit 7 6 5 4 3 2 1 0
	//               | | |
	//               | | +- unactive (0) / active (1)
	//               | +--- tileset 0 (0) / tileset 1 (1)
	//               +----- 0b00 patterns code to opaque (0) or transparent (1)
	//
	// -----------------------------------------------------------------
	uint8_t flags{0b00000000};

	uint8_t colors[4] = {0b00, 0b01, 0b10, 0b11};

	// not to be changed
	uint16_t address;
};

struct sprite_t
{
	// x and y positions
	uint8_t x{0};
	uint8_t y{0};

	// -----------------------------------------------------------------
	// flags
	//
	// bit 7 6 5 4 3 2 1 0
	//       | | | | | | |
	//       | | | | | | +- unactive (0b0) / active (1)
	//       | | | | | +--- tileset 0 (0) / tileset 1 (1)
	//       | | | | +----- 0b00 patterns code to opaque (0) or transparent (1)
	//       | | | +------- x pos relative to screen (0) or associated layer (1)
	//       | | +--------- flip h  (1)
	//       | +----------- flip v  (1)
	//       +------------- flip xy (1)
	//
	// -----------------------------------------------------------------
	uint8_t flags{0b00000000};

	uint8_t index{0};

	uint8_t colors[4] = {0b00, 0b01, 0b10, 0b11};
};

class vdc_t
{
private:
	font_cbm_8x8_t font;

	uint8_t bg_color;

	uint16_t current_scanline;
	int32_t cycles_run;

	bool new_scanline;

	uint8_t current_layer;
	uint8_t current_sprite;

	void draw_scanline(uint16_t scanline);
	void draw_layer(layer_t *l, uint8_t sl);
	void draw_sprite(sprite_t *s, uint8_t sl, layer_t *t);

	exceptions_ic *exceptions;

	bool irq_line;
	bool generate_interrupts;
	uint16_t irq_scanline;

public:
	vdc_t(exceptions_ic *e);
	~vdc_t();

	uint8_t irq_number;

	uint8_t *ram;

	layer_t layer[4];
	sprite_t sprite[256];

	uint32_t *buffer;

	uint8_t io_read8(uint16_t address);
	void io_write8(uint16_t address, uint8_t value);

	inline int32_t get_cycles_run() { return cycles_run; }
	inline uint16_t get_current_scanline() { return current_scanline; }
	inline uint16_t get_irq_scanline() { return irq_scanline; }
	inline bool get_generate_interrupts() { return generate_interrupts; }

	void reset();

	// returns true if frame done
	bool run(uint32_t number_of_cycles);

	bool started_new_scanline() { return new_scanline; }
};

#endif
