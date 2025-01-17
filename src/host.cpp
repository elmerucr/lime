// ---------------------------------------------------------------------
// host.cpp
// lime
//
// Copyright © 2025 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include "host.hpp"
#include "common.hpp"
#include "core.hpp"
#include "vdc.hpp"
#include "debugger.hpp"
#include <thread>
#include <chrono>

host_t::host_t(system_t *s)
{
    // one extra line, needed for proper scanline effect
    core_framebuffer = new uint32_t[PIXELS_PER_SCANLINE * ((2 * SCANLINES) + 1)];
	for (int i=0; i<PIXELS_PER_SCANLINE * ((2 * SCANLINES) + 1); i++) core_framebuffer[i] = 0;

    system = s;

    SDL_Init(SDL_INIT_EVERYTHING);

	// each call to SDL_PollEvent invokes SDL_PumpEvents() that updates this array
    sdl_keyboard_state = SDL_GetKeyboardState(NULL);
	for (int i=0; i<128; i++) keyboard_state[i] = 0;

	SDL_version compiled;
	SDL_VERSION(&compiled);
	printf("[SDL] Compiled against SDL version %d.%d.%d\n", compiled.major, compiled.minor, compiled.patch);

	SDL_version linked;
	SDL_GetVersion(&linked);
	printf("[SDL] Linked against SDL version %d.%d.%d\n", linked.major, linked.minor, linked.patch);

	char *base_path = SDL_GetBasePath();
	printf("[SDL] Base path is: %s\n", base_path);
	SDL_free(base_path);

	sdl_preference_path = SDL_GetPrefPath("elmerucr", "lime");
	printf("[SDL] Preference path is: %s\n", sdl_preference_path);

#if defined(__APPLE__)
	home = getenv("HOME");
	printf("[host] User homedir is: %s\n", home);
#elif defined(__linux)
	home = getenv("HOME");
	printf("[host] User homedir is: %s\n", home);
#else
#   error "Unknown compiler"
#endif

	//audio_init();
	video_init();
}

host_t::~host_t()
{
    delete [] core_framebuffer;

	video_stop();
	//audio_stop();
    SDL_Quit();
}

enum events_output_state host_t::events_process_events()
{
	enum events_output_state return_value = NO_EVENT;

	SDL_Event event;

	//bool shift_pressed = sdl2_keyboard_state[SDL_SCANCODE_LSHIFT] | sdl2_keyboard_state[SDL_SCANCODE_RSHIFT];
	bool alt_pressed = sdl_keyboard_state[SDL_SCANCODE_LALT] | sdl_keyboard_state[SDL_SCANCODE_RALT];
	//bool gui_pressed   = sdl2_keyboard_state[SDL_SCANCODE_LGUI] | sdl2_keyboard_state[SDL_SCANCODE_RGUI];

	while (SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_KEYDOWN:
				return_value = KEYPRESS_EVENT;
			    if ((event.key.keysym.sym == SDLK_f) && alt_pressed) {
					events_wait_until_key_released(SDLK_f);
					video_toggle_fullscreen();
                } else if ((event.key.keysym.sym == SDLK_s) && alt_pressed) {
					if (system->current_mode == RUN_MODE) {
                    	video_scanlines = !video_scanlines;
					}
                } else if ((event.key.keysym.sym == SDLK_q) && alt_pressed) {
                    events_wait_until_key_released(SDLK_q);
					return_value = QUIT_EVENT;
                } else if (event.key.keysym.sym == SDLK_F9) {
                    events_wait_until_key_released(SDLK_F9);
                    system->switch_mode();
                }
                break;
            case SDL_QUIT:
				return_value = QUIT_EVENT;
                break;
            default:
                break;
        }
    }

    return return_value;
}

uint32_t host_t::video_blend(uint32_t c0, uint32_t c1)
{
    return
    ((((c0 & 0x00ff00ff) + (c1 & 0x00ff00ff)) >> 1) & 0x00ff00ff) |
    ((((c0 & 0x0000ff00) + (c1 & 0x0000ff00)) >> 1) & 0x0000ff00) |
    (video_scanline_alpha << 24);
}

void host_t::update_screen()
{
	for (int y=0; y < SCANLINES; y++) {
		for (int x=0; x < PIXELS_PER_SCANLINE; x ++) {
			core_framebuffer[((y << 1) * PIXELS_PER_SCANLINE) + x] = system->core->vdc->buffer[(PIXELS_PER_SCANLINE * y) + x];
		}
	}

	if (video_scanlines && (system->current_mode == RUN_MODE)) {
		for (int y=0; y < SCANLINES; y++) {
			for (int x=0; x < PIXELS_PER_SCANLINE; x++) {
				core_framebuffer[(((y << 1) + 1) * PIXELS_PER_SCANLINE) + x] = video_blend(
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
    SDL_RenderClear(video_renderer);

	switch (system->current_mode) {
		case RUN_MODE:
			SDL_RenderCopy(video_renderer, core_texture, nullptr, nullptr);
			break;
		case DEBUG_MODE:
			system->debugger->redraw();
			SDL_UpdateTexture(debugger_texture, nullptr, (void *)system->debugger->buffer, DEBUGGER_WIDTH*sizeof(uint32_t));
			SDL_RenderCopy(video_renderer, debugger_texture, nullptr, nullptr);

			// draw black rectangle
			SDL_RenderFillRect(video_renderer, &viewer);
			// render core texture
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
	} else {
		SDL_SetWindowFullscreen(video_window, SDL_WINDOW_RESIZABLE);
	}
}

void host_t::video_init()
{
	// print the list of video backends
	int num_video_drivers = SDL_GetNumVideoDrivers();
	printf("[SDL] Display %d video backend(s) compiled into SDL: ", num_video_drivers);
	for (int i=0; i<num_video_drivers; i++) {
		printf(" \'%s\' ", SDL_GetVideoDriver(i));
	}
	printf("\n[SDL] Display now using backend '%s'\n", SDL_GetCurrentVideoDriver());

    SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);
	printf("[SDL] Display current desktop dimension: %i x %i\n", dm.w, dm.h);

    video_scaling = dm.h / SCANLINES;
	while ((10 * video_scaling * SCANLINES) > (9 * dm.h)) video_scaling--;
	if (video_scaling & 0b1) video_scaling--;
    printf("[SDL] Video scaling will be %i times\n", video_scaling);

	char title[64];
	snprintf(title, 64, "lime  v%i.%i  %i", LIME_MAJOR_VERSION, LIME_MINOR_VERSION, LIME_BUILD);

    video_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        video_scaling * PIXELS_PER_SCANLINE,
        video_scaling * SCANLINES,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
    );

	SDL_GetWindowSize(video_window, &video_window_width, &video_window_height);
	printf("[SDL] Display window dimension: %u x %u pixels\n", video_window_width, video_window_height);

	// create renderer and link it to window
	printf("[SDL] Display refresh rate of current display is %iHz\n", dm.refresh_rate);
	if (dm.refresh_rate == FPS) {
		printf("[SDL] This is equal to the FPS of lime, trying for vsync\n");
		SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
		video_renderer = SDL_CreateRenderer(video_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	} else {
		printf("[SDL] This differs from the FPS of lime, going for software FPS\n");
		video_renderer = SDL_CreateRenderer(video_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	}

    // set clear color black
    SDL_SetRenderDrawColor(video_renderer, 0, 0, 0, 255);

	SDL_RendererInfo current_renderer;
	SDL_GetRendererInfo(video_renderer, &current_renderer);
	vsync = (current_renderer.flags & SDL_RENDERER_PRESENTVSYNC) ? true : false;

	printf("[SDL] Renderer Name: %s\n", current_renderer.name);
	printf("[SDL] Renderer %saccelerated\n",
	       (current_renderer.flags & SDL_RENDERER_ACCELERATED) ? "" : "not ");
	printf("[SDL] Renderer vsync is %s\n", vsync ? "enabled" : "disabled");
	printf("[SDL] Renderer does%s support rendering to target texture\n", current_renderer.flags & SDL_RENDERER_TARGETTEXTURE ? "" : "n't");

    core_texture = SDL_CreateTexture(video_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, PIXELS_PER_SCANLINE, 2 * SCANLINES);
    SDL_SetTextureBlendMode(core_texture, SDL_BLENDMODE_BLEND);

    debugger_texture = SDL_CreateTexture(video_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, DEBUGGER_WIDTH, DEBUGGER_HEIGHT);
    SDL_SetTextureBlendMode(debugger_texture, SDL_BLENDMODE_BLEND);

	SDL_RenderSetLogicalSize(video_renderer, PIXELS_PER_SCANLINE, SCANLINES);
    SDL_ShowCursor(SDL_DISABLE);	// make sure cursor isn't visible
}

void host_t::video_stop()
{
	SDL_DestroyTexture(debugger_texture);
    SDL_DestroyTexture(core_texture);
    SDL_DestroyRenderer(video_renderer);
    SDL_DestroyWindow(video_window);
}
