// ---------------------------------------------------------------------
// host.hpp
// lime
//
// Copyright © 2025 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#ifndef HOST_HPP
#define HOST_HPP

#include <SDL2/SDL.h>
#include "common.hpp"
#include "system.hpp"
#include "osd.hpp"

enum events_output_state {
	QUIT_EVENT = -1,
	NO_EVENT = 0,
	KEYPRESS_EVENT = 1
};

enum scancodes {
	SCANCODE_EMPTY = 0x00,     // 0x00
	SCANCODE_ESCAPE,
	SCANCODE_F1,
	SCANCODE_F2,
	SCANCODE_F3,
	SCANCODE_F4,
	SCANCODE_F5,
	SCANCODE_F6,
	SCANCODE_F7,               // 0x08
	SCANCODE_F8,
	SCANCODE_GRAVE,
	SCANCODE_1,
	SCANCODE_2,
	SCANCODE_3,
	SCANCODE_4,
	SCANCODE_5,
	SCANCODE_6,                // 0x10
	SCANCODE_7,
	SCANCODE_8,
	SCANCODE_9,
	SCANCODE_0,
	SCANCODE_MINUS,
	SCANCODE_EQUALS,
	SCANCODE_BACKSPACE,
	SCANCODE_TAB,              // 0x18
	SCANCODE_Q,
	SCANCODE_W,
	SCANCODE_E,
	SCANCODE_R,
	SCANCODE_T,
	SCANCODE_Y,
	SCANCODE_U,
	SCANCODE_I,               // 0x20
	SCANCODE_O,
	SCANCODE_P,
	SCANCODE_LEFTBRACKET,
	SCANCODE_RIGHTBRACKET,
	SCANCODE_RETURN,
	SCANCODE_A,
	SCANCODE_S,
	SCANCODE_D,                // 0x28
	SCANCODE_F,
	SCANCODE_G,
	SCANCODE_H,
	SCANCODE_J,
	SCANCODE_K,
	SCANCODE_L,
	SCANCODE_SEMICOLON,
	SCANCODE_APOSTROPHE,       // 0x30
	SCANCODE_BACKSLASH,
	SCANCODE_LSHIFT,
	SCANCODE_Z,
	SCANCODE_X,
	SCANCODE_C,
	SCANCODE_V,
	SCANCODE_B,
	SCANCODE_N,                // 0x38
	SCANCODE_M,
	SCANCODE_COMMA,
	SCANCODE_PERIOD,
	SCANCODE_SLASH,
	SCANCODE_RSHIFT,
	SCANCODE_LCTRL,
	//SCANCODE_LALT,
	SCANCODE_SPACE,
	//SCANCODE_RALT,
	SCANCODE_RCTRL,              // 0x40
	SCANCODE_LEFT,
	SCANCODE_UP,
	SCANCODE_DOWN,
	SCANCODE_RIGHT
};

class host_t {
private:
    system_t		*system;
    const uint8_t	*sdl_keyboard_state;

    // video related
	SDL_DisplayMode	video_displaymode;
    int				video_scaling{1};
    SDL_Window		*video_window;
	int				video_window_width;
	int				video_window_height;
    SDL_Renderer	*video_renderer;
	bool			vsync;
    bool			video_scanlines{true};
    const uint8_t	video_scanline_alpha = 176;
    bool			video_fullscreen{false};

    SDL_Texture		*video_texture;

	SDL_Texture		*osd_texture;
	bool			osd_visible{false};
	osd_t			*osd;
	SDL_Rect		osd_windowed;

    inline uint32_t video_blend(uint32_t c0, uint32_t c1) {
		    return
			((((c0 & 0x00ff00ff) + (c1 & 0x00ff00ff)) >> 1) & 0x00ff00ff) |
			((((c0 & 0x0000ff00) + (c1 & 0x0000ff00)) >> 1) & 0x0000ff00) |
			(video_scanline_alpha << 24);
	}

	char *home;
public:
    host_t(system_t *s);
    ~host_t();

	char *sdl_preference_path;

	void video_init();
	void video_stop();
    void video_toggle_fullscreen();
    void update_screen();

    uint32_t *video_framebuffer;	// used for scanline effect

	inline bool vsync_enabled() { return vsync; }
	inline bool vsync_disabled() { return !vsync; }

    enum events_output_state events_process_events();
	uint8_t keyboard_state[128];
    void events_wait_until_key_released(SDL_KeyCode key);
	bool events_yes_no();	// return true on 'y' and false on 'n'
};

#endif
