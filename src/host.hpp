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

enum events_output_state {
	QUIT_EVENT = -1,
	NO_EVENT = 0,
	KEYPRESS_EVENT = 1
};

class host_t {
private:
    system_t *system;

    const uint8_t *sdl_keyboard_state;

    // video related
    int video_scaling{1};
    SDL_Window *video_window;
    SDL_Renderer *video_renderer;
	bool vsync;
    bool video_scanlines{true};
    const uint8_t video_scanline_alpha = 176;
    bool video_fullscreen{false};

    SDL_Texture *core_texture;
    uint32_t *core_framebuffer;

	SDL_Texture *debugger_texture;

	int video_window_width;
	int video_window_height;

	const SDL_Rect viewer = {
		116, 4, PIXELS_PER_SCANLINE/2, SCANLINES/2
	};

    uint32_t video_blend(uint32_t c0, uint32_t c1);

	char *home;
public:
    host_t(system_t *s);
    ~host_t();

	char *sdl_preference_path;

	void video_init();
	void video_stop();
    void video_toggle_fullscreen();
    void update_screen();

	inline bool vsync_enabled() { return vsync; }
	inline bool vsync_disabled() { return !vsync; }

    enum events_output_state events_process_events();
	uint8_t keyboard_state[128];
    void events_wait_until_key_released(SDL_KeyCode key);
};

#endif
