#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include <cstdint>
#include "common.hpp"
#include "font_4x8.hpp"
#include "rca.hpp"
#include "terminal.hpp"

class debugger_t {
private:
	font_4x8_t debugger_font;
	uint8_t *tiles;
	uint32_t *fg_colors;
	uint32_t *bg_colors;
	rca_t rca;
	terminal_t *terminal;
	terminal_t *status;
public:
	debugger_t();
	~debugger_t();
	uint32_t *buffer;
	void redraw();
};

#endif
