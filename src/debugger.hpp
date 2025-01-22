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

	bool have_prompt;
	char text_buffer[1024];
public:
	debugger_t(system_t *s);
	~debugger_t();

	terminal_t *terminal;

	void process_command(char *c);
	void prompt();
	void print_version();
	void memory_dump(uint16_t address);
	void enter_memory_line(char *buffer);
	void memory_binary_dump(uint16_t address);
	void enter_memory_binary_line(char *buffer);
	uint32_t disassemble_instruction(uint16_t address);

	bool hex_string_to_int(const char *temp_string, uint32_t *return_value);
	bool binary_string_to_int(const char *temp_string, uint32_t *return_value);

	uint32_t *buffer;
	void redraw();

	void run();
};

#endif
