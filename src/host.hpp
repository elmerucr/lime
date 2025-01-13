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

class host_t {
private:
    system_t *system;

    const uint8_t *sdl_keyboard_state;

    // video related
    int video_scaling{1};
    SDL_Window *video_window;
    SDL_Renderer *video_renderer;
    bool video_scanlines{true};
    const uint8_t video_scanline_alpha = 176;
    bool video_fullscreen{false};

    SDL_Texture *core_texture;
    uint32_t *core_framebuffer;

	SDL_Texture *debugger_texture;

	const SDL_Rect viewer = {
		PIXELS_PER_SCANLINE/2, 0, PIXELS_PER_SCANLINE/2, SCANLINES/2
	};

    uint32_t blend(uint32_t c0, uint32_t c1);

public:
    host_t(system_t *s);

    ~host_t();

	void video_init();
	void video_stop();

    bool process_events();
    void events_wait_until_key_released(SDL_KeyCode key);
    void video_toggle_fullscreen();
    void update_screen();
};

#endif
