// ---------------------------------------------------------------------
// vdc.hpp
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
// ---------------------------------------------------------------------

#ifndef VC_HPP
#define VC_HPP

#include <cstdint>
#include "font_cbm_8x8.hpp"

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
	//
	// -----------------------------------------------------------------
	uint8_t flags{0b00000000};
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
    uint8_t *ram;
    font_cbm_8x8_t font;
	void draw_layer(layer_t *l, uint8_t sl);
	void draw_sprite(sprite_t *s, uint8_t sl, layer_t *t);
public:
    vdc_t();
    ~vdc_t();

	layer_t layer[4];
	sprite_t sprite[256];

    uint8_t *buffer;

	void reset();

    void update_scanline(uint8_t s);
};

#endif
