#include "host.hpp"
#include "common.hpp"
#include <thread>
#include <chrono>

host_t::host_t()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    sdl_keyboard_state = SDL_GetKeyboardState(NULL);

    SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);
    printf("desktop screen height: %i\n", dm.h);
    window_scaling = ((3 * dm.h) / (8 * VIDEO_HEIGHT)) << 1;
    printf("window scaling: %i\n", window_scaling);

    window = SDL_CreateWindow(
        "lemon",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_scaling * VIDEO_WIDTH,
        window_scaling * VIDEO_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
    );

    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	SDL_RenderSetLogicalSize(renderer, VIDEO_WIDTH, VIDEO_HEIGHT);
    SDL_ShowCursor(SDL_DISABLE);

    // black clearcolor
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, VIDEO_WIDTH, 2 * VIDEO_HEIGHT);
    SDL_SetTextureBlendMode(screen, SDL_BLENDMODE_BLEND);

    // one extra line, needed for scanline effect
    framebuffer = new uint32_t[VIDEO_WIDTH * ((2 * VIDEO_HEIGHT) + 1)];

    for (int i=0; i<VIDEO_WIDTH * ((2 * VIDEO_HEIGHT) + 1); i++) {
        framebuffer[i] = 0;
    }
}

host_t::~host_t()
{
    printf("quitting\n");
    delete [] framebuffer;
    SDL_DestroyTexture(screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void host_t::process_events()
{
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
                    if (++scanline_mode > 2) scanline_mode = 0;
                } else if ((event.key.keysym.sym == SDLK_q) && alt_pressed) {
                    events_wait_until_key_released(SDLK_q);
                    running = false;
                }
                break;
            case SDL_QUIT:
                running = false;
                break;
            default:
                break;
        }
    }
}

uint32_t host_t::blend(uint32_t c0, uint32_t c1)
{
    return
    ((((c0 & 0x00ff00ff) + (c1 & 0x00ff00ff)) >> 1) & 0x00ff00ff) |
    ((((c0 & 0x0000ff00) + (c1 & 0x0000ff00)) >> 1) & 0x0000ff00) |
    (scanline_alpha << 24);
}

void host_t::run()
{
    while (running) {
        process_events();
        SDL_RenderClear(renderer);

        for (int y=0; y < VIDEO_HEIGHT; y++) {
            for (int x=0; x < VIDEO_WIDTH; x ++) {
                framebuffer[((y << 1) * VIDEO_WIDTH) + x] = palette[rca.byte() & 0b11];
            }
        }

        switch (scanline_mode) {
            case 1:
                for (int y=0; y < VIDEO_HEIGHT; y++) {
                    for (int x=0; x < VIDEO_WIDTH; x++) {
                        framebuffer[(((y << 1) + 1) * VIDEO_WIDTH) + x] = blend(
                            framebuffer[(((y << 1) + 0) * VIDEO_WIDTH) + x],
                            framebuffer[(((y << 1) + 2) * VIDEO_WIDTH) + x]
                        );
                    }
                }
                break;
            case 2:
                for (int y=0; y < VIDEO_HEIGHT; y++) {
                    for (int x=0; x < VIDEO_WIDTH; x++) {
                        framebuffer[(((y << 1) + 1) * VIDEO_WIDTH) + x] =
                            (framebuffer[(((y << 1) + 0) * VIDEO_WIDTH) + x] & 0x00ffffff) |
                            (scanline_alpha << 24);
                    }
                }
                break;
            default:
                for (int y=0; y < VIDEO_HEIGHT; y++) {
                    for (int x=0; x < VIDEO_WIDTH; x++) {
                        framebuffer[(((y << 1) + 1) * VIDEO_WIDTH) + x] =
                            framebuffer[(((y << 1) + 0) * VIDEO_WIDTH) + x];
                    }
                }
                break;
        }

        SDL_UpdateTexture(screen, nullptr, (void *)framebuffer, VIDEO_WIDTH*sizeof(uint32_t));
        SDL_RenderCopy(renderer, screen, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
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
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		//SDL_GetWindowSize(window, &window_width, &window_height);
		//SDL_RenderSetLogicalSize(video_renderer, window_width, window_height);
		//printf("[SDL] Fullscreen size: %i x %i\n", window_width, window_height);
	} else {
		SDL_SetWindowFullscreen(window, SDL_WINDOW_RESIZABLE);
		//SDL_GetWindowSize(video_window, &window_width, &window_height);
		//SDL_RenderSetLogicalSize(video_renderer, window_width, window_height);
		//printf("[SDL] Window size: %i x %i\n", window_width, window_height);
	}
}
