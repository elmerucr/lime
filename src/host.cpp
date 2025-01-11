// ---------------------------------------------------------------------
// host.cpp
// lime
//
// Copyright © 2025 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include "host.hpp"
#include "common.hpp"
#include "vdc.hpp"
#include <thread>
#include <chrono>

host_t::host_t(system_t *s)
{
    // one extra line, needed for proper scanline effect
    core_framebuffer = new uint32_t[PIXELS_PER_SCANLINE * ((2 * SCANLINES) + 1)];

    for (int i=0; i<PIXELS_PER_SCANLINE * ((2 * SCANLINES) + 1); i++) {
        core_framebuffer[i] = 0;
    }

    system = s;

    SDL_Init(SDL_INIT_EVERYTHING);

    sdl_keyboard_state = SDL_GetKeyboardState(NULL);

    SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);
    printf("desktop screen height: %i\n", dm.h);
    video_scaling = (4 * dm.h) / (5 * SCANLINES);
	if (video_scaling & 0b1) video_scaling--;
    printf("window scaling: %i\n", video_scaling);

    video_window = SDL_CreateWindow(
        "lime",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        video_scaling * PIXELS_PER_SCANLINE,
        video_scaling * SCANLINES,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
    );

    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    video_renderer = SDL_CreateRenderer(video_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	SDL_RenderSetLogicalSize(video_renderer, PIXELS_PER_SCANLINE, SCANLINES);
    SDL_ShowCursor(SDL_DISABLE);

    // black = clear color
    SDL_SetRenderDrawColor(video_renderer, 0, 0, 0, 255);

    core_texture = SDL_CreateTexture(video_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, PIXELS_PER_SCANLINE, 2 * SCANLINES);
    SDL_SetTextureBlendMode(core_texture, SDL_BLENDMODE_BLEND);
}

host_t::~host_t()
{
    printf("quitting\n");
    delete [] core_framebuffer;
    SDL_DestroyTexture(core_texture);
    SDL_DestroyRenderer(video_renderer);
    SDL_DestroyWindow(video_window);
    SDL_Quit();
}

bool host_t::process_events()
{
    bool return_value{true};

	SDL_Event event;

	//bool shift_pressed = sdl2_keyboard_state[SDL_SCANCODE_LSHIFT] | sdl2_keyboard_state[SDL_SCANCODE_RSHIFT];
	bool alt_pressed = sdl_keyboard_state[SDL_SCANCODE_LALT] | sdl_keyboard_state[SDL_SCANCODE_RALT];
	//bool gui_pressed   = sdl2_keyboard_state[SDL_SCANCODE_LGUI] | sdl2_keyboard_state[SDL_SCANCODE_RGUI];

	while (SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_KEYDOWN:
			    if ((event.key.keysym.sym == SDLK_f) && alt_pressed) {
					events_wait_until_key_released(SDLK_f);
					video_toggle_fullscreen();
                } else if ((event.key.keysym.sym == SDLK_s) && alt_pressed) {
                    video_scanlines = !video_scanlines;
                    //if (++video_scanlines > 1) video_scanlines = 0;
                } else if ((event.key.keysym.sym == SDLK_q) && alt_pressed) {
                    events_wait_until_key_released(SDLK_q);
                    return_value = false;
                } else if (event.key.keysym.sym == SDLK_F9) {
                    events_wait_until_key_released(SDLK_F9);
                    system->switch_mode();
                }
                break;
            case SDL_QUIT:
                return_value = false;
                break;
            default:
                break;
        }
    }

    return return_value;
}

uint32_t host_t::blend(uint32_t c0, uint32_t c1)
{
    return
    ((((c0 & 0x00ff00ff) + (c1 & 0x00ff00ff)) >> 1) & 0x00ff00ff) |
    ((((c0 & 0x0000ff00) + (c1 & 0x0000ff00)) >> 1) & 0x0000ff00) |
    (video_scanline_alpha << 24);
}

void host_t::update_screen()
{
    SDL_RenderClear(video_renderer);

	switch (system->current_mode) {
		case RUN_MODE:
			for (int y=0; y < SCANLINES; y++) {
				for (int x=0; x < PIXELS_PER_SCANLINE; x ++) {
					core_framebuffer[((y << 1) * PIXELS_PER_SCANLINE) + x] = system->vdc->buffer[(PIXELS_PER_SCANLINE * y) + x];
				}
			}
			if (video_scanlines) {
				for (int y=0; y < SCANLINES; y++) {
					for (int x=0; x < PIXELS_PER_SCANLINE; x++) {
						core_framebuffer[(((y << 1) + 1) * PIXELS_PER_SCANLINE) + x] = blend(
							core_framebuffer[(((y << 1) + 0) * PIXELS_PER_SCANLINE) + x],
							core_framebuffer[(((y << 1) + 2) * PIXELS_PER_SCANLINE) + x]
						);
					}
				}
			} else {
				for (int y=0; y < SCANLINES; y++) {
					for (int x=0; x < PIXELS_PER_SCANLINE; x++) {
						core_framebuffer[(((y << 1) + 1) * PIXELS_PER_SCANLINE) + x] =
							core_framebuffer[(((y << 1) + 0) * PIXELS_PER_SCANLINE) + x];
					}
				}
			}
			SDL_UpdateTexture(core_texture, nullptr, (void *)core_framebuffer, PIXELS_PER_SCANLINE*sizeof(uint32_t));
			SDL_RenderCopy(video_renderer, core_texture, nullptr, nullptr);
			break;
		case DEBUG_MODE:
			SDL_RenderCopy(video_renderer, core_texture, nullptr, &viewer);

			break;
	}

    SDL_RenderPresent(video_renderer);
}

void host_t::events_wait_until_key_released(SDL_KeyCode key)
{
	SDL_Event event;
	bool wait = true;
	while (wait) {
	    SDL_PollEvent(&event);
	    if ((event.type == SDL_KEYUP) && (event.key.keysym.sym == key)) wait = false;
	    std::this_thread::sleep_for(std::chrono::microseconds(40000));
	}
}

void host_t::video_toggle_fullscreen()
{
	video_fullscreen = !video_fullscreen;
	if (video_fullscreen) {
		SDL_SetWindowFullscreen(video_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		//SDL_GetWindowSize(video_window, &window_width, &window_height);
		//SDL_RenderSetLogicalSize(video_renderer, window_width, window_height);
		//printf("[SDL] Fullscreen size: %i x %i\n", window_width, window_height);
	} else {
		SDL_SetWindowFullscreen(video_window, SDL_WINDOW_RESIZABLE);
		//SDL_GetWindowSize(video_window, &window_width, &window_height);
		//SDL_RenderSetLogicalSize(video_renderer, window_width, window_height);
		//printf("[SDL] Window size: %i x %i\n", window_width, window_height);
	}
}

void host_t::video_init()
{
	//
}

void host_t::video_stop()
{
	//
}
