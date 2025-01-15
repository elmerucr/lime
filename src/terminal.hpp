#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <cstdint>

#define TEXT_BUFFER_SIZE	256

enum output_type {
	NOTHING,
	MEMORY,
	MEMORY_BINARY,
	DISASSEMBLY
};

class terminal_t {
public:
	terminal_t(uint8_t w, uint8_t h, uint32_t fg, uint32_t bg);
	~terminal_t();

	void clear();
	void putsymbol_at_cursor(char symbol);
	void putsymbol(char symbol);
	int putchar(int character);
	int puts(const char *text);
	int printf(const char *format, ...);

	void cursor_left();
	void cursor_right();
	void cursor_up();
	void cursor_down();
	void backspace();

	void activate_cursor();
	void deactivate_cursor();
	void process_cursor_state();

	void add_bottom_row();
	void add_top_row();

	void get_command(char *c, int length);

	inline int lines_remaining() {
		return height - (cursor_position / width) - 1;
	}

	uint8_t width;
	uint8_t height;

	uint32_t fg_color;
	uint32_t bg_color;

	uint8_t *tiles;
	uint32_t *fg_colors;
	uint32_t *bg_colors;

private:
	uint16_t total_tiles;

	uint16_t cursor_position{0};
	uint8_t  cursor_interval{20};
	uint8_t  cursor_countdown{0};
	char     cursor_original_char{0x20};
	uint8_t  cursor_original_color;
	uint8_t  cursor_original_background_color;
	bool     cursor_blinking{false};

	enum output_type check_output(bool top_down, uint32_t *address, uint32_t *w);

	char	text_buffer[TEXT_BUFFER_SIZE];
};

#endif
