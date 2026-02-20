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
#include "stats.hpp"
#include <thread>
#include <chrono>
#include <ctime>

#include <unistd.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

host_t::host_t(system_t *s)
{
    system = s;

	video_viewer_framebuffer = new uint32_t[VDC_XRES * VDC_YRES];

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);

	// each call to SDL_PollEvent invokes SDL_PumpEvents() that updates this array
	int no_of_keys;
    sdl_keyboard_state = SDL_GetKeyboardState(&no_of_keys);
	if (!sdl_keyboard_state) {
		printf("[SDL] Error getting keyboard state: %s\n", SDL_GetError());
	}

	for (int i=0; i<128; i++) {
		keyboard_state[i] = 0;
	}

	printf("[SDL] Version %i.%i.%i\n", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);

	const char *base_path = SDL_GetBasePath();
	printf("[SDL] Base path is: %s\n", base_path);
	//SDL_free(base_path);

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
	delete osd_notify;
	delete osd_stats;
	delete [] video_viewer_framebuffer;

	video_stop();
	audio_cleanup();
    SDL_Quit();
}

enum events_output_state host_t::events_process_events()
{
	enum events_output_state return_value = NO_EVENT;

	SDL_Event event;

	bool alt_pressed = sdl_keyboard_state[SDL_SCANCODE_LALT] || sdl_keyboard_state[SDL_SCANCODE_RALT];

	while (SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_EVENT_KEY_DOWN:
				return_value = KEYPRESS_EVENT;
			    if ((event.key.scancode == SDL_SCANCODE_F) && alt_pressed) {
					events_wait_until_key_released(SDL_SCANCODE_F);
					video_toggle_fullscreen();
				} else if ((event.key.scancode == SDL_SCANCODE_P) && alt_pressed) {
					save_screenshot();
				} else if ((event.key.scancode == SDL_SCANCODE_R) && alt_pressed) {
					events_wait_until_key_released(SDL_SCANCODE_R);
					osd_notify_frames_remaining = 90;
					osd_notify->terminal->clear();
					for (int i=0; i<((45-12) / 2); i++) osd_notify->terminal->cursor_right();
					osd_notify->terminal->printf("system reset");
					system->core->reset();
				} else if ((event.key.scancode == SDL_SCANCODE_S) && alt_pressed) {
					video_toggle_scanlines();
					osd_notify_frames_remaining = 90;
					osd_notify->terminal->clear();
					for (int i=0; i<((45-13) / 2); i++) osd_notify->terminal->cursor_right();
					osd_notify->terminal->printf("scanlines %s", video_scanlines ? "on" : "off");
				} else if ((event.key.scancode == SDL_SCANCODE_D) && alt_pressed) {
					if (video_scanlines) {
						if (video_scanlines_alpha > 0xf0) {
							video_scanlines_alpha = 0x00;
						} else {
							video_scanlines_alpha += 0x11;
						}
						osd_notify_frames_remaining = 90;
						osd_notify->terminal->clear();
						for (int i=0; i<((45-19) / 2); i++) osd_notify->terminal->cursor_right();
						osd_notify->terminal->printf("brightness %2i of 15", video_scanlines_alpha / 17);
					}
				} else if ((event.key.scancode == SDL_SCANCODE_A) && alt_pressed) {
					system->core->vdc->change_crt_contrast();
					osd_notify_frames_remaining = 90;
					osd_notify->terminal->clear();
					for (int i=0; i<((45-17) / 2); i++) osd_notify->terminal->cursor_right();
					osd_notify->terminal->printf("contrast %2i of 15", system->core->vdc->get_crt_contrast() / 17);
                } else if ((event.key.scancode == SDL_SCANCODE_Q) && alt_pressed) {
                    events_wait_until_key_released(SDL_SCANCODE_Q);
					return_value = QUIT_EVENT;
                } else if ((event.key.scancode == SDL_SCANCODE_C) && alt_pressed) {
					if (system->current_mode == DEBUG_MODE) {
						system->debugger->terminal->clear();
						system->debugger->prompt();
					}
                } else if (event.key.scancode == SDL_SCANCODE_F9) {
                    events_wait_until_key_released(SDL_SCANCODE_F9);
                    system->switch_mode();
                } else if (event.key.scancode == SDL_SCANCODE_F10) {
					osd_stats_visible = !osd_stats_visible;
				}
                break;
			case SDL_EVENT_DROP_FILE:
				{
					const char *path = event.drop.data;
					system->core->attach_bin(path);
				}
				break;
            case SDL_EVENT_QUIT:
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

void host_t::save_screenshot()
{
	time_t rawtime = time(NULL);
	struct tm *timeinfo = localtime(&rawtime); // or gmtime for UTC
	char buffer[35]; // 34 characters + 1 for null terminator
	strftime(buffer, sizeof(buffer), "lime_screenshot_%Y%m%d%H%M%S.png", timeinfo);

	osd_notify_frames_remaining = 90;
	osd_notify->terminal->clear();
	osd_notify->terminal->printf("  saving %s", buffer);

	// create buffer that supports double y resolution
	uint32_t b[2 * VDC_XRES * 2 * VDC_YRES];

	// redo scanline blending
	uint32_t *cur_pix_src = system->core->vdc->buffer;
	uint32_t *nxt_pix_src = cur_pix_src + VDC_XRES;
	uint32_t *cur_pix_dst = b;
	uint32_t *nxt_pix_dst = cur_pix_dst + (2 * VDC_XRES);
	for (int y = 0; y < VDC_YRES; y++) {
		if (y == (VDC_YRES - 1)) nxt_pix_src = cur_pix_src;
		for (int x = 0; x < VDC_XRES; x++) {
			*cur_pix_dst++ = *cur_pix_src;
			*cur_pix_dst++ = *cur_pix_src;
			*nxt_pix_dst++ = video_scanlines ? video_blend(*cur_pix_src, *nxt_pix_src) : *cur_pix_src;
			*nxt_pix_dst++ = video_scanlines ? video_blend(*cur_pix_src, *nxt_pix_src) : *cur_pix_src;
			cur_pix_src++;
			nxt_pix_src++;
		}
		cur_pix_dst += (2 * VDC_XRES);
		nxt_pix_dst += (2 * VDC_XRES);
	}

	// reorganize byte ordering fot png format (rgba, in that order)
	uint8_t c[4 * 2 * VDC_XRES * 2 * VDC_YRES];
	for (int i=0; i<(2 *VDC_XRES*2*VDC_YRES); i++) {
		uint8_t alpha = (b[i] & 0xff000000) >> 24;
		c[(i << 2) + 0] = ((b[i] & 0xff0000) >> 16) * alpha / 255;
		c[(i << 2) + 1] = ((b[i] & 0x00ff00) >>  8) * alpha / 255;
		c[(i << 2) + 2] = ((b[i] & 0x0000ff) >>  0) * alpha / 255;
		c[(i << 2) + 3] = 0xff;
	}

	// write png
	chdir(home);
	stbi_write_png(buffer, 2 * VDC_XRES, 2 * VDC_YRES, 4, (void *)c, 8 * VDC_XRES);
}

void host_t::video_update_screen()
{
	void *pixels;
	int pitch;

	SDL_RenderClear(video_renderer);

	//SDL_UpdateTexture(vdc_texture, nullptr, (void *)system->core->vdc->buffer, VDC_XRES*sizeof(uint32_t));
	SDL_LockTexture(vdc_texture, nullptr, &pixels, &pitch);
	pitch /= vdc_texture_bytes_per_pixel;
	// for (int y = 0; y < VDC_YRES; y++) {
	// 	int i = (y == (VDC_YRES - 1) ? 0 : 1);
	// 	for (int x = 0; x < VDC_XRES; x++) {
	// 		uint32_t cur_pix = system->core->vdc->buffer[((y + 0) * VDC_XRES) + x];
	// 		uint32_t nxt_pix = system->core->vdc->buffer[((y + i) * VDC_XRES) + x];
	// 		((uint32_t *)pixels)[(((2 * y) + 0) * pitch) + x] = cur_pix;
	// 		((uint32_t *)pixels)[(((2 * y) + 1) * pitch) + x] = video_scanlines ? video_blend(cur_pix, nxt_pix) : cur_pix;
	// 	}
	// }
	uint32_t *cur_pix_src = system->core->vdc->buffer;
	uint32_t *nxt_pix_src = cur_pix_src + VDC_XRES;
	uint32_t *cur_pix_dst = (uint32_t *)pixels;
	uint32_t *nxt_pix_dst = cur_pix_dst + pitch;
	for (int y = 0; y < VDC_YRES; y++) {
		if (y == (VDC_YRES - 1)) nxt_pix_src = cur_pix_src;
		for (int x = 0; x < VDC_XRES; x++) {
			*cur_pix_dst++ = *cur_pix_src;
			*nxt_pix_dst++ = video_scanlines ? video_blend(*cur_pix_src, *nxt_pix_src) : *cur_pix_src;
			cur_pix_src++;
			nxt_pix_src++;
		}
		cur_pix_dst += pitch;
		nxt_pix_dst += pitch;
	}
	SDL_UnlockTexture(vdc_texture);

	SDL_RenderTexture(video_renderer, vdc_texture, nullptr, nullptr);

	if (system->current_mode == DEBUG_MODE) {
		system->debugger->redraw();
		SDL_UpdateTexture(debugger_texture, nullptr, (void *)system->debugger->buffer, DEBUGGER_XRES*sizeof(uint32_t));
		SDL_RenderTexture(video_renderer, debugger_texture, nullptr, nullptr);
		if (viewer_visible) {
			SDL_UpdateTexture(viewer_texture, nullptr, (void *)video_viewer_framebuffer, VDC_XRES*sizeof(uint32_t));
			SDL_RenderTexture(video_renderer, viewer_texture, nullptr, &viewer_texture_placement);
		}
	}

	if (osd_stats_visible) {
		osd_stats->terminal->printf("%s", system->stats->summary());
		osd_stats->redraw();
		SDL_UpdateTexture(osd_stats_texture, nullptr, (void *)osd_stats->buffer, osd_stats->width*8*sizeof(uint32_t));
		SDL_RenderTexture(video_renderer, osd_stats_texture, nullptr, &osd_stats_placement);
	}

	if(osd_notify_frames_remaining > 0) {
		osd_notify->redraw();
		SDL_UpdateTexture(osd_notify_texture, nullptr, (void *)osd_notify->buffer, osd_notify->width*8*sizeof(uint32_t));
		SDL_RenderTexture(video_renderer, osd_notify_texture, nullptr, &osd_notify_placement);
		osd_notify_frames_remaining--;
	}

    SDL_RenderPresent(video_renderer);
}

void host_t::events_wait_until_key_released(SDL_Scancode key)
{
	SDL_Event event;
	bool wait = true;
	while (wait) {
	    SDL_PollEvent(&event);
	    if ((event.type == SDL_EVENT_KEY_UP) && (event.key.scancode == key)) wait = false;
	    std::this_thread::sleep_for(std::chrono::microseconds(40000));
	}
}

void host_t::video_toggle_fullscreen()
{
	bool video_fullscreen = (SDL_GetWindowFlags(video_window) & SDL_WINDOW_FULLSCREEN);
	SDL_SetWindowFullscreen(video_window, !video_fullscreen);
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

	int num_displays = 0;
	SDL_DisplayID *displays = SDL_GetDisplays(&num_displays);
	printf("[SDL] Number of displays: %i\n", num_displays);
	const SDL_DisplayMode *mode = SDL_GetDesktopDisplayMode(*displays);
	printf("[SDL] Desktop display mode: %ix%i\n", mode->w, mode->h);

	if ((VDC_YRES * mode->w / mode->h) > VDC_XRES) {
		// display is wider than aspect ratio
		video_scaling = mode->h / VDC_YRES;
		if ((mode->h % VDC_YRES) == 0) {
			video_scaling--;
		}
	} else {
		video_scaling = mode->w / VDC_XRES;
		if ((mode->w % VDC_XRES) == 0) {
			video_scaling--;
		}
	}
	SDL_free(displays);

	if (video_scaling < 1) {
		video_scaling = 1;
	}

    printf("[SDL] Video scaling will be %i time%s\n", video_scaling, (video_scaling == 1) ? "" : "s");

	video_window = SDL_CreateWindow(nullptr, video_scaling * VDC_XRES, video_scaling * VDC_YRES, SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE);
	SDL_SetWindowAspectRatio(video_window, (float)VDC_XRES/VDC_YRES, (float)VDC_XRES/VDC_YRES);
	SDL_SetWindowPosition(video_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    SDL_Surface *icon = SDL_CreateSurface(64, 64, SDL_PIXELFORMAT_ARGB8888);
    if (icon->pixels) {
        const uint8_t pixels[] = {
            "                "
            "   *            "
            "  *./           "
            "  *.//          "
            " *./../         "
            " *.///..        "
            " *./////.       "
            " *.///../.      "
            " *./..//./.     "
            "  *.////././    "
            "  *.///.///./   "
            "   *.//.///.//  "
            "    *../////..* "
            "     **.....**  "
            "       *****    "
            "                "
        };
        for (unsigned int y = 0; y < 64; y++) {
            for (unsigned int x = 0; x < 64; x++) {
                switch (pixels[(16 * (y >> 2)) + (x >> 2)]) {
                    case '*': ((uint32_t *)icon->pixels)[(y * 64) + x] = 0xff'34'68'56; break;
                    case '/': ((uint32_t *)icon->pixels)[(y * 64) + x] = 0xff'88'c0'70; break;
                    case '.': ((uint32_t *)icon->pixels)[(y * 64) + x] = 0xff'e0'f8'd0; break;
                    default : ((uint32_t *)icon->pixels)[(y * 64) + x] = 0x00'00'00'00; break;
                }
            }
        }
        SDL_SetWindowIcon(video_window, icon);
        SDL_DestroySurface(icon);
    }

	SDL_GetWindowSize(video_window, &video_window_width, &video_window_height);
	printf("[SDL] Display window dimension: %u x %u pixels\n", video_window_width, video_window_height);

	osd_stats = new osd_t(system, 47, 3);
	osd_notify = new osd_t(system, 45, 1);
	osd_notify->terminal->clear();

	// create renderer and link it to window
	video_renderer = SDL_CreateRenderer(video_window, nullptr);
	if (video_renderer) {
		printf("[SDL] Created a \"%s\" renderer\n", SDL_GetRendererName(video_renderer));
	}

	int w, h;
	SDL_GetRenderOutputSize(video_renderer, &w, &h);
	printf("[SDL] Renderer outputs at %ix%i\n", w, h);

	printf("[SDL] Display refresh rate of current display is %.2fHz\n", mode->refresh_rate);
	if ((mode->refresh_rate > ((float)FPS - .6)) && (mode->refresh_rate < ((float)FPS + .6))) {
		printf("[SDL] This is equal to the fps of lime, trying for vsync\n");
		SDL_SetRenderVSync(video_renderer, 1);
	} else {
		printf("[SDL] This differs from the FPS of lime, going for software FPS\n");
		SDL_SetRenderVSync(video_renderer, 0);
	}

    int i;
    SDL_GetRenderVSync(video_renderer, &i);
	if (i) {
        printf("[SDL] vsync every %i frame(s)\n", i);
    } else {
        printf("[SDL] vsync is off\n");
    }
	vsync = (i == 1) ? true : false;

    vdc_texture = SDL_CreateTexture(video_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, VDC_XRES, 2 * VDC_YRES);
    SDL_SetTextureScaleMode(vdc_texture, SDL_SCALEMODE_PIXELART);
	SDL_SetTextureBlendMode(vdc_texture, SDL_BLENDMODE_BLEND);
	vdc_texture_bytes_per_pixel = (SDL_BITSPERPIXEL(vdc_texture->format) >> 3);

	debugger_texture = SDL_CreateTexture(video_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, DEBUGGER_XRES, DEBUGGER_YRES);
	SDL_SetTextureScaleMode(debugger_texture, SDL_SCALEMODE_PIXELART);
	SDL_SetTextureBlendMode(debugger_texture, SDL_BLENDMODE_BLEND);

	osd_stats_texture = SDL_CreateTexture(video_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, osd_stats->width*8, osd_stats->height*8);
	SDL_SetTextureScaleMode(osd_stats_texture, SDL_SCALEMODE_PIXELART);
	SDL_SetTextureBlendMode(osd_stats_texture, SDL_BLENDMODE_BLEND);
	osd_stats_placement = {
 		.x = (float)(DEBUGGER_XRES - (osd_stats->width * 8)) / 2,
 		.y = (float)DEBUGGER_YRES - (osd_stats->height * 8) - 4,
 		.w = (float)osd_stats->width * 8,
 		.h = (float)osd_stats->height * 8
	};

	osd_notify_texture = SDL_CreateTexture(video_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, osd_notify->width*8, osd_notify->height*8);
	SDL_SetTextureScaleMode(osd_notify_texture, SDL_SCALEMODE_PIXELART);
	SDL_SetTextureBlendMode(osd_notify_texture, SDL_BLENDMODE_BLEND);
	osd_notify_placement = {
 		.x = (float)(DEBUGGER_XRES - (osd_notify->width * 8)) / 2,
 		.y = 4.0,
 		.w = (float)osd_notify->width * 8,
 		.h = (float)osd_notify->height * 8
	};

	viewer_texture = SDL_CreateTexture(video_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, VDC_XRES, VDC_YRES);
    SDL_SetTextureScaleMode(viewer_texture, SDL_SCALEMODE_PIXELART);
	SDL_SetTextureBlendMode(viewer_texture, SDL_BLENDMODE_BLEND);
	viewer_texture_placement = {
		.x = (float)DEBUGGER_XRES - (((15 * 8 * VDC_XRES) / VDC_YRES) + 4),
		.y = (float)DEBUGGER_YRES - ((15 * 8) + 12),
		.w = (float)(15 * 8 * VDC_XRES) / VDC_YRES,
		.h = (float)15 * 8
	};

    SDL_SetRenderDrawColor(video_renderer, 0, 0, 0, 255);	// set clear color black

	SDL_SetRenderLogicalPresentation(video_renderer, DEBUGGER_XRES, DEBUGGER_YRES, SDL_LOGICAL_PRESENTATION_LETTERBOX);
	SDL_HideCursor();				// make sure cursor isn't visible
}

void host_t::video_stop()
{
	SDL_DestroyTexture(viewer_texture);
	SDL_DestroyTexture(osd_stats_texture);
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
		if (event.type == SDL_EVENT_KEY_DOWN) {
			if (event.key.scancode == SDL_SCANCODE_Y) {
				checking = false;
			} else if (event.key.scancode == SDL_SCANCODE_N) {
				return_value = false;
				checking = false;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	return return_value;
}

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
	SDL_zero(audio_spec);

	/*
	 * Define audio specification
	 */
	audio_spec.freq = SAMPLE_RATE;
	audio_spec.format = SDL_AUDIO_F32;
	audio_spec.channels = 2;

	/*
	 * Open audio device, allowing any changes to the specification
	 */
    audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio_spec, nullptr, nullptr);
    if (!audio_stream) {
        SDL_Log("[SDL] Couldn't create audio stream: %s", SDL_GetError());
		// this is not enough and even wrong...
		// consider a system without audio support?
        return SDL_Quit();
    }

	printf("[SDL] audio now using backend '%s'\n", SDL_GetCurrentAudioDriver());
	printf("[SDL] audio information / format:\n");
	printf("[SDL]  frequency  : %d\n", audio_spec.freq);
	printf("[SDL]  float      : %s\n", SDL_AUDIO_ISFLOAT(audio_spec.format) ? "yes" : "no");
	printf("[SDL]  signed     : %s\n", SDL_AUDIO_ISSIGNED(audio_spec.format) ? "yes" : "no");
	printf("[SDL]  big endian : %s\n", SDL_AUDIO_ISBIGENDIAN(audio_spec.format) ? "yes" : "no");
	printf("[SDL]  bitsize    : %d\n", SDL_AUDIO_BITSIZE(audio_spec.format));
	printf("[SDL]  channels   : %d\n", audio_spec.channels);

	audio_bytes_per_sample = SDL_AUDIO_BITSIZE(audio_spec.format) / 8;
	printf("[SDL] audio is using %d bytes per sample per channel\n", audio_bytes_per_sample);

	audio_bytes_per_ms = (double)SAMPLE_RATE * audio_spec.channels * audio_bytes_per_sample / 1000;
	printf("[SDL] audio is using %f bytes per ms\n", audio_bytes_per_ms);

	printf("[SDL] start audio\n");
    SDL_ResumeAudioStreamDevice(audio_stream);
}

void host_t::audio_cleanup()
{
	printf("[SDL] cleanup audio\n");
	SDL_PauseAudioStreamDevice(audio_stream);
}
