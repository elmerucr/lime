// ---------------------------------------------------------------------
// debugger.hpp
// lime
//
// Copyright © 2025 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include <cstdint>
#include "common.hpp"
#include "font_cbm_8x8.hpp"
#include "terminal.hpp"
#include "system.hpp"

class debugger_t {
private:
	font_cbm_8x8_t debugger_cbm_font;
	terminal_t *status1;
	terminal_t *exception_status;
	terminal_t *vdc_status;
	system_t *system;

	bool have_prompt;
	char text_buffer[1024];

	uint8_t terminal_graphics_colors[4] {
		0b00, 0b01, 0b10, 0b11
	};

	bool timers_4_7 = false;
	bool m68k_disassembly = true;

public:
	debugger_t(system_t *s);
	~debugger_t();

	terminal_t *terminal;

	uint32_t *buffer;

	void process_command(char *c);
	void prompt();
	void print_version();
	void memory_dump(uint32_t address);
	void enter_memory_line(char *buffer);
	void memory_binary_dump(uint32_t address);
	void enter_memory_binary_line(char *buffer);
	void enter_dgc_line(char *buffer);
	void enter_mc6809_assembly_line(char *buffer);
	uint32_t disassemble_instruction_status1(uint16_t address);
	uint32_t disassemble_instruction_terminal(uint32_t address);

	bool hex_string_to_int(const char *temp_string, uint32_t *return_value);
	bool binary_string_to_int(const char *temp_string, uint32_t *return_value);

	void redraw();

	void run();
};

#endif
