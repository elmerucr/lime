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
    // video_framebuffer = new uint32_t[SCREEN_WIDTH * (SCREEN_HEIGHT + 1)];
	// for (int i=0; i<SCREEN_WIDTH * (SCREEN_HEIGHT + 1); i++) video_framebuffer[i] = 0;

	video_viewer_framebuffer = new uint32_t[VDC_XRES * VDC_YRES];

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
	home = getenv("HOME");
//#   error "Unknown compiler"
#endif

	audio_init();
	video_init();
}

host_t::~host_t()
{
	delete osd;
	delete [] video_viewer_framebuffer;
    // delete [] video_framebuffer;

	video_stop();
	audio_stop();
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
				} else if ((event.key.keysym.sym == SDLK_r) && alt_pressed) {
					events_wait_until_key_released(SDLK_r);
					system->core->reset();
                } else if ((event.key.keysym.sym == SDLK_q) && alt_pressed) {
                    events_wait_until_key_released(SDLK_q);
					return_value = QUIT_EVENT;
                } else if (event.key.keysym.sym == SDLK_F9) {
                    events_wait_until_key_released(SDLK_F9);
                    system->switch_mode();
                } else if (event.key.keysym.sym == SDLK_F10) {
					osd_visible = !osd_visible;
				}
                break;
			case SDL_DROPFILE:
				{
				char *path = event.drop.file;
				system->core->attach_bin(path);
				}
				break;
            case SDL_QUIT:
				return_value = QUIT_EVENT;
                break;
            default:
                break;
        }
    }

	if (!alt_pressed) {
		(keyboard_state[SCANCODE_ESCAPE      ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_ESCAPE      ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_F1          ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_F1          ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_F2          ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_F2          ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_F3          ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_F3          ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_F4          ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_F4          ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_F5          ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_F5          ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_F6          ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_F6          ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_F7          ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_F7          ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_F8          ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_F8          ] ? 0b1 : 0b0;
		// TODO: what was different here in old E64?
		(keyboard_state[SCANCODE_GRAVE       ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_GRAVE       ] ? 0b1 : 0b0;
		//
		(keyboard_state[SCANCODE_1           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_1           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_2           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_2           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_3           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_3           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_4           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_4           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_5           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_5           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_6           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_6           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_7           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_7           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_8           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_8           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_9           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_9           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_0           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_0           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_MINUS       ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_MINUS       ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_EQUALS      ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_EQUALS      ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_BACKSPACE   ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_BACKSPACE   ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_TAB         ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_TAB         ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_Q           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_Q           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_W           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_W           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_E           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_E           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_R           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_R           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_T           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_T           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_Y           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_Y           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_U           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_U           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_I           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_I           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_O           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_O           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_P           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_P           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_LEFTBRACKET ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_LEFTBRACKET ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_RIGHTBRACKET] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_RIGHTBRACKET] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_RETURN      ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_RETURN      ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_A           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_A           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_S           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_S           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_D           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_D           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_F           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_F           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_G           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_G           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_H           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_H           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_J           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_J           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_K           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_K           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_L           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_L           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_SEMICOLON   ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_SEMICOLON   ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_APOSTROPHE  ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_APOSTROPHE  ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_BACKSLASH   ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_BACKSLASH   ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_LSHIFT      ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_LSHIFT      ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_Z           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_Z           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_X           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_X           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_C           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_C           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_V           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_V           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_B           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_B           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_N           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_N           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_M           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_M           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_COMMA       ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_COMMA       ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_PERIOD      ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_PERIOD      ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_SLASH       ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_SLASH       ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_RSHIFT      ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_RSHIFT      ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_LCTRL       ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_LCTRL       ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_SPACE       ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_SPACE       ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_RCTRL       ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_RCTRL       ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_LEFT        ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_LEFT        ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_UP          ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_UP          ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_DOWN        ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_DOWN        ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_RIGHT       ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_RIGHT       ] ? 0b1 : 0b0;
	};

	if (return_value == QUIT_EVENT) printf("[SDL] detected quit event\n");
	return return_value;
}

void host_t::video_update_screen()
{
	SDL_RenderClear(video_renderer);

	SDL_UpdateTexture(vdc_texture, nullptr, (void *)system->core->vdc->buffer, VDC_XRES*sizeof(uint32_t));
	SDL_RenderCopy(video_renderer, vdc_texture, nullptr, nullptr);

	if (system->current_mode == DEBUG_MODE) {
		system->debugger->redraw();
		SDL_UpdateTexture(debugger_texture, nullptr, (void *)system->debugger->buffer, DEBUGGER_XRES*sizeof(uint32_t));
		//SDL_SetTextureScaleMode(debugger_texture, SDL_ScaleModeLinear);
		SDL_RenderCopy(video_renderer, debugger_texture, nullptr, nullptr);
		if (viewer_visible) {
			SDL_UpdateTexture(viewer_texture, nullptr, (void *)video_viewer_framebuffer, VDC_XRES*sizeof(uint32_t));
			SDL_RenderCopy(video_renderer, viewer_texture, nullptr, &viewer_texture_placement);
		}
	}

	if (osd_visible) {
		osd->redraw();
		SDL_UpdateTexture(osd_texture, nullptr, (void *)osd->buffer, osd->width*8*sizeof(uint32_t));
		SDL_RenderCopy(video_renderer, osd_texture, nullptr, &osd_placement);
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
	// video_set_dimensions();
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


	SDL_GetCurrentDisplayMode(0, &video_displaymode);
	printf("[SDL] Display current desktop dimension: %i x %i\n", video_displaymode.w, video_displaymode.h);

	if ((VDC_YRES * video_displaymode.w / video_displaymode.h) > VDC_XRES) {
		// wider than 3:2 aspect ration
		video_scaling = video_displaymode.h / VDC_YRES;
		if ((video_displaymode.h % VDC_YRES) == 0) {
			video_scaling--;
		}
	} else {
		video_scaling = video_displaymode.w / VDC_XRES;
		if ((video_displaymode.w % VDC_XRES) == 0) {
			video_scaling--;
		}
	}

	if (video_scaling < 1) video_scaling = 1;

    printf("[SDL] Video scaling will be %i time%s\n", video_scaling, (video_scaling == 1) ? "" : "s");

    video_window = SDL_CreateWindow(
        nullptr,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        video_scaling * VDC_XRES,
        video_scaling * VDC_YRES,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
    );

	SDL_GetWindowSize(video_window, &video_window_width, &video_window_height);
	printf("[SDL] Display window dimension: %u x %u pixels\n", video_window_width, video_window_height);

	osd = new osd_t(system);

	// create renderer and link it to window
	printf("[SDL] Display refresh rate of current display is %iHz\n", video_displaymode.refresh_rate);
	if (video_displaymode.refresh_rate == FPS) {
		printf("[SDL] This is equal to the FPS of lime, trying for vsync\n");
		SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
		video_renderer = SDL_CreateRenderer(video_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	} else {
		printf("[SDL] This differs from the FPS of lime, going for software FPS\n");
		video_renderer = SDL_CreateRenderer(video_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	}

	int w, h;
	SDL_GetRendererOutputSize(video_renderer, &w, &h);
	printf("[SDL] Renderer Output Size: %i x %i\n", w, h);

	if ((w % DEBUGGER_XRES) == 0) {
		printf("[SDL] Width of renderer fits debugger xres of %i\n", DEBUGGER_XRES);
	} else {
		printf("[SDL] Width of renderer doesn't fit debugger xres of %i\n", DEBUGGER_XRES);
		video_scaling--;
		if (video_scaling == 0 ) video_scaling = 1;
		SDL_SetWindowSize(video_window, video_scaling * VDC_XRES, video_scaling * VDC_YRES);
		SDL_GetWindowSize(video_window, &video_window_width, &video_window_height);
		printf("[SDL] Display new window dimension: %u x %u pixels\n", video_window_width, video_window_height);
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

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");	// nearest pixel
    vdc_texture = SDL_CreateTexture(video_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, VDC_XRES, VDC_YRES);
	SDL_SetTextureBlendMode(vdc_texture, SDL_BLENDMODE_BLEND);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");	// bilinear filtering hint
	debugger_texture = SDL_CreateTexture(video_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, DEBUGGER_XRES, DEBUGGER_YRES);
	SDL_SetTextureBlendMode(debugger_texture, SDL_BLENDMODE_BLEND);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");	// nearest pixel
	osd_texture = SDL_CreateTexture(video_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, osd->width*8, osd->height*8);
    SDL_SetTextureBlendMode(osd_texture, SDL_BLENDMODE_BLEND);
	osd_placement = {
 		.x = (DEBUGGER_XRES / 2) - ((osd->width / 2) * 8),
 		.y = DEBUGGER_YRES - ((osd->height) * 8),
 		.w = osd->width * 8,
 		.h = osd->height * 8
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");	// bilinear filtering hint
	viewer_texture = SDL_CreateTexture(video_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, VDC_XRES, VDC_YRES);
    SDL_SetTextureBlendMode(viewer_texture, SDL_BLENDMODE_BLEND);
	viewer_texture_placement = {
		.x = DEBUGGER_XRES - (((16 * 8 * VDC_XRES) / VDC_YRES) + 8),
		.y = DEBUGGER_YRES - ((15 * 8) + 16),
		.w = (16 * 8 * VDC_XRES) / VDC_YRES,
		.h = 16 * 8
	};

	SDL_RenderSetLogicalSize(video_renderer, DEBUGGER_XRES, DEBUGGER_YRES);
    SDL_ShowCursor(SDL_DISABLE);	// make sure cursor isn't visible
}

void host_t::video_stop()
{
	SDL_DestroyTexture(viewer_texture);
	SDL_DestroyTexture(osd_texture);
	SDL_DestroyTexture(debugger_texture);
    SDL_DestroyTexture(vdc_texture);
    SDL_DestroyRenderer(video_renderer);
    SDL_DestroyWindow(video_window);
}

void host_t::video_set_window_title(const char *t)
{
	SDL_SetWindowTitle(video_window, t);
}

bool host_t::events_yes_no()
{
	SDL_Event event;
	bool checking = true;
	bool return_value = true;
	while (checking) {
		SDL_PollEvent(&event);
		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_y) {
				checking = false;
			} else if (event.key.keysym.sym == SDLK_n) {
				return_value = false;
				checking = false;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	return return_value;
}

// void host_t::video_toggle_fullscreen_stretched()
// {
// 	if (video_fullscreen) {
// 		video_fullscreen_stretched = !video_fullscreen_stretched;
// 		video_set_dimensions();
// 	}
// }

void host_t::audio_init()
{
	/*
	 * Print the list of audio backends
	 */
	int numAudioDrivers = SDL_GetNumAudioDrivers();
	printf("[SDL] audio backend(s): %d compiled into SDL: ", numAudioDrivers);
	for (int i=0; i<numAudioDrivers; i++) {
		printf(" \'%s\' ", SDL_GetAudioDriver(i));
	}
	printf("\n");

	// What's this all about???
	SDL_zero(audio_spec_want);

	/*
	 * Define audio specification
	 */
	audio_spec_want.freq = SAMPLE_RATE;
	audio_spec_want.format = AUDIO_F32SYS;
	audio_spec_want.channels = 2;
	audio_spec_want.samples = 512;
	audio_spec_want.callback = nullptr;

	/*
	 * Open audio device, allowing any changes to the specification
	 */
	audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec_want, &audio_spec_have,
						 SDL_AUDIO_ALLOW_ANY_CHANGE);
	if(!audio_device) {
		printf("[SDL] failed to open audio device: %s\n", SDL_GetError());
		// this is not enough and even wrong...
		// consider a system without audio support?
		SDL_Quit();
	}

	printf("[SDL] audio now using backend '%s'\n", SDL_GetCurrentAudioDriver());
	printf("[SDL] audio information:        want\thave\n");
	printf("[SDL]         frequency         %d\t%d\n", audio_spec_want.freq, audio_spec_have.freq);
	printf("[SDL]         format\n"
	       "[SDL]          float            %s\t%s\n",
	       SDL_AUDIO_ISFLOAT(audio_spec_want.format) ? "yes" : "no",
	       SDL_AUDIO_ISFLOAT(audio_spec_have.format) ? "yes" : "no");
	printf("[SDL]          signed           %s\t%s\n",
	       SDL_AUDIO_ISSIGNED(audio_spec_want.format) ? "yes" : "no",
	       SDL_AUDIO_ISSIGNED(audio_spec_have.format) ? "yes" : "no");
	printf("[SDL]          big endian       %s\t%s\n",
	       SDL_AUDIO_ISBIGENDIAN(audio_spec_want.format) ? "yes" : "no",
	       SDL_AUDIO_ISBIGENDIAN(audio_spec_have.format) ? "yes" : "no");
	printf("[SDL]          bitsize          %d\t%d\n",
	       SDL_AUDIO_BITSIZE(audio_spec_want.format),
	       SDL_AUDIO_BITSIZE(audio_spec_have.format));
	printf("[SDL]          channels         %d\t%d\n", audio_spec_want.channels, audio_spec_have.channels);
	printf("[SDL]          samples          %d\t%d\n", audio_spec_want.samples, audio_spec_have.samples);

	audio_bytes_per_sample = SDL_AUDIO_BITSIZE(audio_spec_have.format) / 8;
	printf("[SDL] audio is using %d bytes per sample per channel\n", audio_bytes_per_sample);

	audio_bytes_per_ms = (double)SAMPLE_RATE * audio_spec_have.channels * audio_bytes_per_sample / 1000;
	printf("[SDL] audio is using %f bytes per ms\n", audio_bytes_per_ms);

	audio_running = false;

	audio_start();
}

void host_t::audio_start()
{
	if (!audio_running) {
		printf("[SDL] start audio\n");
		// Unpause audiodevice, and process audiostream
		SDL_PauseAudioDevice(audio_device, 0);
		audio_running = true;
	}
}

void host_t::audio_stop()
{
	if (audio_running) {
		printf("[SDL] stop audio\n");
		// Pause audiodevice
		SDL_PauseAudioDevice(audio_device, 1);
		audio_running = false;
	}
}


// void host_t::video_set_dimensions()
// {
// 	if (video_fullscreen) {
// 		SDL_SetWindowFullscreen(video_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
// 		SDL_GetWindowSize(video_window, &video_window_width, &video_window_height);
// 		SDL_RenderSetLogicalSize(video_renderer, video_window_width, video_window_height);
// 		if (video_fullscreen_stretched) {
// 			video_placement = {
// 				.x = (video_window_width - (3 * video_window_height) / 2) / 2,
// 				.y = 0,
// 				.w = (3 * video_window_height) / 2,
// 				.h = video_window_height
// 			};
// 			viewer_placement = {
// 				.x = ((8 * 43 * video_window_height) / SCREEN_HEIGHT) + (video_window_width - (3 * video_window_height) / 2) / 2,
// 				.y = (8 * 27 * video_window_height) / SCREEN_HEIGHT,
// 				.w = 15 * 8 * (video_window_height) / SCREEN_HEIGHT,
// 				.h = 10 * 8 * video_window_height / SCREEN_HEIGHT
// 			};
// 		} else {
// 			// not stretched
// 			video_placement = {
// 				.x = (video_window_width - (video_scaling * SCREEN_WIDTH)) / 2,
// 				.y = (video_window_height - (video_scaling * SCREEN_HEIGHT)) / 2,
// 				.w = video_scaling * SCREEN_WIDTH,
// 				.h = video_scaling * SCREEN_HEIGHT
// 			};
// 			viewer_placement = {
// 				.x = ((video_window_width - (video_scaling * SCREEN_WIDTH)) / 2) + (8 * 43 * video_scaling),
// 				.y = ((video_window_height - (video_scaling * SCREEN_HEIGHT)) / 2) + (8 * 27 * video_scaling),
// 				.w = 15 * 8 * video_scaling,
// 				.h = 10 * 8 * video_scaling
// 			};
// 		}
// 		printf("[SDL] Fullscreen size: %i x %i\n", video_window_width, video_window_height);
// 	} else {
// 		SDL_SetWindowFullscreen(video_window, SDL_WINDOW_RESIZABLE);
// 		SDL_GetWindowSize(video_window, &video_window_width, &video_window_height);
// 		SDL_RenderSetLogicalSize(video_renderer, video_window_width, video_window_height);
// 		video_placement = {
// 			.x = 0,
// 			.y = 0,
// 			.w = video_window_width,
// 			.h = video_window_height
// 		};
// 		viewer_placement = {
// 			.x = 43 * 8 * video_scaling,
// 			.y = 27 * 8 * video_scaling,
// 			.w = 15 * 8 * video_scaling,
// 			.h = 10 * 8 * video_scaling
// 		};
// 		printf("[SDL] Window size: %i x %i\n", video_window_width, video_window_height);
// 	}

// 	osd_placement = {
// 		.x = (video_window_width - (video_scaling * osd->width * 8)) / 2,
// 		.y = (video_window_height - (video_scaling * osd->height * 8)),
// 		.w = video_scaling * osd->width * 8,
// 		.h = video_scaling * osd->height * 8
// 	};
// }
