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

	//const uint32_t fg = 0xffff8010;
	//const uint32_t bg = 0x80804008;
	const uint32_t fg = LIME_COLOR_03;
	//const uint32_t bg = 0xff'1a'34'2b;
	const uint32_t bg = (LIME_COLOR_01 & 0x00ffffff) | 0x80000000;

	font_cbm_8x8_t font;
	terminal_t *terminal;
public:
	osd_t(system_t *s);
	~osd_t();

	const uint8_t width{47};	// in characters
	const uint8_t height{3};	// in characters

	uint32_t *buffer;

	void redraw();
};

#endif
