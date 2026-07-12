// ---------------------------------------------------------------------
// keyboard.cpp
// lime
//
// Copyright © 2023-2025 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include "common.hpp"
#include "keyboard.hpp"
#include <iostream>

#define SHIFT_PRESSED   0b00000001
#define CTRL_PRESSED    0b00000010
//#define ALT_PRESSED     0b00000100

inline char event_to_ascii(uint8_t scancode, uint8_t modifiers)
{
	char result;

	switch (scancode) {
		case SCANCODE_ESCAPE:       result = ASCII_ESCAPE;              break;
		case SCANCODE_F1:           result = ASCII_F1;              break;
		case SCANCODE_F2:           result = ASCII_F2;              break;
		case SCANCODE_F3:           result = ASCII_F3;              break;
		case SCANCODE_F4:           result = ASCII_F4;              break;
		case SCANCODE_F5:           result = ASCII_F5;              break;
		case SCANCODE_F6:           result = ASCII_F6;              break;
		case SCANCODE_F7:           result = ASCII_F7;              break;
		case SCANCODE_F8:           result = ASCII_F8;              break;
		case SCANCODE_GRAVE:        result = (modifiers & SHIFT_PRESSED) ? ASCII_TILDE : ASCII_GRAVE;              break;
		case SCANCODE_1:            result = (modifiers & SHIFT_PRESSED) ? ASCII_EXCL_MARK : ASCII_1;              break;
		case SCANCODE_2:            result = (modifiers & SHIFT_PRESSED) ? ASCII_AT : ASCII_2;              break;
		case SCANCODE_3:            result = (modifiers & SHIFT_PRESSED) ? ASCII_NUMBER : ASCII_3;              break;
		case SCANCODE_4:            result = (modifiers & SHIFT_PRESSED) ? ASCII_DOLLAR : ASCII_4;              break;
		case SCANCODE_5:            result = (modifiers & SHIFT_PRESSED) ? ASCII_PERCENT : ASCII_5;              break;
		case SCANCODE_6:            result = (modifiers & SHIFT_PRESSED) ? ASCII_CARET : ASCII_6;              break;
		case SCANCODE_7:            result = (modifiers & SHIFT_PRESSED) ? ASCII_AMPERSAND : ASCII_7;              break;
		case SCANCODE_8:            result = (modifiers & SHIFT_PRESSED) ? ASCII_ASTERISK : ASCII_8;              break;
		case SCANCODE_9:            result = (modifiers & SHIFT_PRESSED) ? ASCII_OPEN_PAR : ((modifiers & CTRL_PRESSED) ? ASCII_REVERSE_ON : ASCII_9);              break;
		case SCANCODE_0:            result = (modifiers & SHIFT_PRESSED) ? ASCII_CLOSE_PAR : ((modifiers & CTRL_PRESSED) ? ASCII_REVERSE_OFF : ASCII_0);              break;
		case SCANCODE_MINUS:        result = (modifiers & SHIFT_PRESSED) ? ASCII_UNDERSCORE : ASCII_HYPHEN;              break;
		case SCANCODE_EQUALS:       result = (modifiers & SHIFT_PRESSED) ? ASCII_PLUS : ASCII_EQUALS;              break;
		case SCANCODE_BACKSPACE:    result = ASCII_BACKSPACE;              break;
		case SCANCODE_TAB:          result = ASCII_SPACE;              break;	// TODO: temp hack to make things work
		case SCANCODE_Q:            result = (modifiers & SHIFT_PRESSED) ? ASCII_Q : ASCII_q;              break;
		case SCANCODE_W:            result = (modifiers & SHIFT_PRESSED) ? ASCII_W : ASCII_w;              break;
		case SCANCODE_E:            result = (modifiers & SHIFT_PRESSED) ? ASCII_E : ASCII_e;              break;
		case SCANCODE_R:            result = (modifiers & SHIFT_PRESSED) ? ASCII_R : ASCII_r;              break;
		case SCANCODE_T:            result = (modifiers & SHIFT_PRESSED) ? ASCII_T : ASCII_t;              break;
		case SCANCODE_Y:            result = (modifiers & SHIFT_PRESSED) ? ASCII_Y : ASCII_y;              break;
		case SCANCODE_U:            result = (modifiers & SHIFT_PRESSED) ? ASCII_U : ASCII_u;              break;
		case SCANCODE_I:            result = (modifiers & SHIFT_PRESSED) ? ASCII_I : ASCII_i;              break;
		case SCANCODE_O:            result = (modifiers & SHIFT_PRESSED) ? ASCII_O : ASCII_o;              break;
		case SCANCODE_P:            result = (modifiers & SHIFT_PRESSED) ? ASCII_P : ASCII_p;              break;
		case SCANCODE_LEFTBRACKET:  result = (modifiers & SHIFT_PRESSED) ? ASCII_OPEN_BRACE : ASCII_OPEN_BRACK;              break;
		case SCANCODE_RIGHTBRACKET: result = (modifiers & SHIFT_PRESSED) ? ASCII_CLOSE_BRACE : ASCII_CLOSE_BRACK;              break;
		case SCANCODE_RETURN:       result = ASCII_LF;              break;
		case SCANCODE_A:            result = (modifiers & SHIFT_PRESSED) ? ASCII_A : ASCII_a;              break;
		case SCANCODE_S:            result = (modifiers & SHIFT_PRESSED) ? ASCII_S : ASCII_s;              break;
		case SCANCODE_D:            result = (modifiers & SHIFT_PRESSED) ? ASCII_D : ASCII_d;              break;
		case SCANCODE_F:            result = (modifiers & SHIFT_PRESSED) ? ASCII_F : ASCII_f;              break;
		case SCANCODE_G:            result = (modifiers & SHIFT_PRESSED) ? ASCII_G : ASCII_g;              break;
		case SCANCODE_H:            result = (modifiers & SHIFT_PRESSED) ? ASCII_H : ASCII_h;              break;
		case SCANCODE_J:            result = (modifiers & SHIFT_PRESSED) ? ASCII_J : ASCII_j;              break;
		case SCANCODE_K:            result = (modifiers & SHIFT_PRESSED) ? ASCII_K : ASCII_k;              break;
		case SCANCODE_L:            result = (modifiers & SHIFT_PRESSED) ? ASCII_L : ASCII_l;              break;
		case SCANCODE_SEMICOLON:    result = (modifiers & SHIFT_PRESSED) ? ASCII_COLON : ASCII_SEMI_COLON;              break;
		case SCANCODE_APOSTROPHE:   result = (modifiers & SHIFT_PRESSED) ? ASCII_DOUBLE_QUOTES : ASCII_SINGLE_QUOTE;              break;
		case SCANCODE_BACKSLASH:    result = (modifiers & SHIFT_PRESSED) ? ASCII_VERT_BAR : ASCII_BACKSLASH;              break;
		case SCANCODE_Z:            result = (modifiers & SHIFT_PRESSED) ? ASCII_Z : ASCII_z;              break;
		case SCANCODE_X:            result = (modifiers & SHIFT_PRESSED) ? ASCII_X : ASCII_x;              break;
		case SCANCODE_C:            result = (modifiers & SHIFT_PRESSED) ? ASCII_C : ASCII_c;              break;
		case SCANCODE_V:            result = (modifiers & SHIFT_PRESSED) ? ASCII_V : ASCII_v;              break;
		case SCANCODE_B:            result = (modifiers & SHIFT_PRESSED) ? ASCII_B : ASCII_b;              break;
		case SCANCODE_N:            result = (modifiers & SHIFT_PRESSED) ? ASCII_N : ASCII_n;              break;
		case SCANCODE_M:            result = (modifiers & SHIFT_PRESSED) ? ASCII_M : ASCII_m;              break;
		case SCANCODE_COMMA:        result = (modifiers & SHIFT_PRESSED) ? ASCII_LESS : ASCII_COMMA;       break;
		case SCANCODE_PERIOD:       result = (modifiers & SHIFT_PRESSED) ? ASCII_GREATER : ASCII_PERIOD;   break;
		case SCANCODE_SLASH:        result = (modifiers & SHIFT_PRESSED) ? ASCII_QUESTION_M : ASCII_SLASH; break;
		case SCANCODE_SPACE:        result = ASCII_SPACE;              break;
		case SCANCODE_LEFT:         result = ASCII_CURSOR_LEFT;              break;
		case SCANCODE_UP:           result = ASCII_CURSOR_UP;              break;
		case SCANCODE_DOWN:         result = ASCII_CURSOR_DOWN;              break;
		case SCANCODE_RIGHT:        result = ASCII_CURSOR_RIGHT;              break;
		default:                    result = ASCII_NULL;              break;
	}

	if (modifiers & CTRL_PRESSED) result &= 0x1f;	// the old way of making a control character

	return result;
}

bool scancode_not_modifier[] =
{
    true,                       // 0x00
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,                       // 0x08
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,                       // 0x10
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,                       // 0x18
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,                       // 0x20
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,                       // 0x28
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,                       // 0x30
    true,
    false,                      // left shift
    true,
    true,
    true,
    true,
    true,
    true,                       // 0x38
    true,
    true,
    true,
    true,
    false,                      // right shift
    false,                      // left control
    //false,                      // left alt
    true,
    //false,                      // right alt
    false,                      // right control
    true,
    true,
    true,
    true
};

keyboard_t::keyboard_t(system_t *s)
{
	system = s;
	reset();
}

void keyboard_t::process()
{
    uint8_t modifier_keys_status =
		((system->host->keyboard_state[SCANCODE_LSHIFT] & 0b1) ? SHIFT_PRESSED : 0) |
		((system->host->keyboard_state[SCANCODE_RSHIFT] & 0b1) ? SHIFT_PRESSED : 0) |
		((system->host->keyboard_state[SCANCODE_LCTRL ] & 0b1) ? CTRL_PRESSED  : 0) |
		((system->host->keyboard_state[SCANCODE_RCTRL ] & 0b1) ? CTRL_PRESSED  : 0) ;

	for (int i=0; i<128; i++) {
		switch (system->host->keyboard_state[i] & 0b11) {
			case 0b01:
				// event key down
				if (generate_events && scancode_not_modifier[i]) {
					key_down = true;
					last_char = i;
					time_to_next = repeat_delay_ms * 1000;
					push_event(event_to_ascii(last_char, modifier_keys_status)); // immediate
				}
				break;
			case 0b10:
				// event key up
				if (generate_events) {
					if (i == last_char) {
						key_down = false;
					}
				}
			default:
				// do nothing
				break;
		}
	}

	microseconds_remaining = microseconds_per_frame;

	if (key_down) {
		while (microseconds_remaining) {
			if (time_to_next < microseconds_remaining) {
				microseconds_remaining -= time_to_next;
				time_to_next = repeat_speed_ms * 1000;
				// TODO: see old E64 here... avoid two call to push_event!
				push_event(event_to_ascii(last_char, modifier_keys_status));
			} else {
				time_to_next -= microseconds_remaining;
				microseconds_remaining = 0;
			}
		}
	}
}

void keyboard_t::push_event(uint8_t event)
{
	event_list[head] = event;

	head++;

	if (head == tail) {
		tail++;
	}
}

uint8_t keyboard_t::pop_event()
{
	uint8_t result;
	if( head == tail ) {
		// no events in queue / stack return empty scancode
		result = SCANCODE_EMPTY;
	} else {
		result = event_list[tail];
		tail++;
	}
	return result;
}

void keyboard_t::reset()
{
	repeat_delay_ms = 500;
	repeat_speed_ms = 50;
	head = tail = 0;
}

uint8_t keyboard_t::io_read8(uint16_t address)
{
	if (address & 0x80) {
		return system->host->keyboard_state[address & 0x7f];
	}

	switch (address & 0xff) {
		case 0x00:
			// status register
			return events_waiting() ? 0b1 : 0b0;
		case 0x02:
			return (repeat_delay_ms & 0xff00) >> 8;
		case 0x03:
			return repeat_delay_ms & 0xff;
		case 0x04:
			return (repeat_speed_ms & 0xff00) >> 8;
		case 0x05:
			return repeat_speed_ms & 0xff;
		case 0x06:
			return pop_event();
		default:
			return 0x00;
	}
}

void keyboard_t::io_write8(uint16_t address, uint8_t value)
{
	switch (address & 0xff) {
		case 0x01:
			// control register
			if (value & 0b10000000) purge();
			break;
		case 0x02:
			repeat_delay_ms = (repeat_delay_ms & 0x00ff) | (value << 8);
			break;
		case 0x03:
			repeat_delay_ms = (repeat_delay_ms & 0xff00) | value;
			//if (repeat_speed_ms == 0) repeat_speed_ms = 1;
			break;
		case 0x04:
			repeat_speed_ms = (repeat_speed_ms & 0x00ff) | (value << 8);
			break;
		case 0x05:
			repeat_speed_ms = (repeat_speed_ms & 0xff00) | value;
			if (repeat_speed_ms == 0) repeat_speed_ms = 1;
			break;
		default:
			break;
	}
}
