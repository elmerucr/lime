#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include <cstdint>
#include "common.hpp"
#include "font_4x8.hpp"
#include "rca.hpp"
#include "terminal.hpp"
#include "system.hpp"

class debugger_t {
private:
	font_4x8_t debugger_font;
	uint8_t *tiles;
	uint32_t *fg_colors;
	uint32_t *bg_colors;
	rca_t rca;
	terminal_t *status;
	system_t *system;
public:
	debugger_t(system_t *s);
	~debugger_t();

	terminal_t *terminal;
	void prompt();

	uint32_t *buffer;
	void redraw();

	void run();
};

#endif
