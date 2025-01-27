// ---------------------------------------------------------------------
// vdc.hpp
// lime
//
// video display controller
// ---------------------------------------------------------------------

// ---------------------------------------------------------------------
//
// sprites 0-63     front
// layer_0            .
// sprites 64-127     .
// layer 1            .
// sprites 128-191    .
// layer 2            .
// sprites 192-255    .
// layer 3          back
//
// 0x00: SR
// 0x01: CR
// 0x02: read
// 0x03:
// 0x04: bg color
//
// ---------------------------------------------------------------------

#ifndef VC_HPP
#define VC_HPP

#include <cstdint>
#include "common.hpp"
#include "font_cbm_8x8.hpp"

// gameboy green palette
// https://lospec.com/palette-list/nintendo-gameboy-bgb
const uint32_t palette[4] = {
	GB_COLOR_0,
	GB_COLOR_1,
	GB_COLOR_2,
	GB_COLOR_3
};

struct layer_t {
	uint8_t x{0};
	uint8_t y{0};

	// -----------------------------------------------------------------
	// flags
	//
	// bit 7 6 5 4 3 2 1 0
	//       | | | | | | |
	//       | | | | | | +- unactive (0b0) / active (1)
	//       | | | | | +--- tileset 0 (0) / tileset 1 (1)
	//       | | | | +----- 0b00 is opaque (0) or transparent (1)
	//
	// -----------------------------------------------------------------
	uint8_t flags{0b00000000};

	// palette values can never be >0b11
	uint8_t palette[4] = { 0b00, 0b01, 0b10, 0b11 };

	// not to be changed
	uint16_t address;
};

struct sprite_t {
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
	//       | | | | +----- 0b00 is opaque (0) or transparent (1)
	//       | | | +------- x pos relative to screen (0) or associated layer (1)
	//       | | +--------- flip h  (1)
	//       | +----------- flip v  (1)
	//       +------------- flip xy (1)
	//
	// -----------------------------------------------------------------
	uint8_t flags{0b00000000};

	uint8_t index{0};

	// palette values can never be >0b11
	uint8_t palette[4] = { 0b00, 0b01, 0b10, 0b11 };
};

class vdc_t {
private:
    font_cbm_8x8_t font;
	void draw_layer(layer_t *l, uint8_t sl);
	void draw_sprite(sprite_t *s, uint8_t sl, layer_t *t);

	uint8_t bg_color{0b00};

	uint16_t current_scanline;
	int32_t cycles_run;

	bool new_scanline;

public:
    vdc_t();
    ~vdc_t();

    uint8_t *ram;

	layer_t layer[4];
	sprite_t sprite[256];

    uint32_t *buffer;

	uint8_t io_read8(uint16_t address);
	void io_write8(uint16_t address, uint8_t value);

	inline int32_t get_cycles_run() { return cycles_run ; }
	inline uint16_t get_current_scanline() { return current_scanline; }

	void reset();

	// returns true if frame done
	bool run(uint32_t number_of_cycles);

    void draw_scanline(uint16_t scanline);

	bool started_new_scanline() { return new_scanline; }
};

#endif
