#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include <cstdint>
#include "common.hpp"
#include "font_cbm_8x8.hpp"
#include "rca.hpp"

class debugger_t {
private:
	font_cbm_8x8_t font;
	uint8_t *chars;
	uint32_t *fg_color;
	uint32_t *bg_color;

	rca_t rca;
public:
	debugger_t();
	~debugger_t();

	uint32_t *buffer;

	void redraw();
};

#endif
