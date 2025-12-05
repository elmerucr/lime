// ---------------------------------------------------------------------
// osd.hpp
// lime
//
// Copyright © 2025 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#ifndef OSD_HPP
#define OSD_HPP

#include <cstdint>
#include "system.hpp"
#include "font_cbm_8x8.hpp"
#include "terminal.hpp"
#include "common.hpp"

class osd_t {
private:
	system_t *system;

	const uint32_t fg = LIME_COLOR_03;
	const uint32_t bg = (LIME_COLOR_01 & 0x00ffffff) | 0xe0000000;

	font_cbm_8x8_t font;
public:
	osd_t(system_t *s, uint8_t w, uint8_t h);
	~osd_t();

	uint8_t width;	// in characters
	uint8_t height;	// in characters

	uint32_t *buffer;

	terminal_t *terminal;

	void redraw();
};

#endif
