#ifndef HOST_HPP
#define HOST_HPP

#include <SDL2/SDL.h>
#include "rca.hpp"

class host_t {
private:
    const uint8_t *sdl_keyboard_state;
    int window_scaling{1};
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *screen;
    uint32_t *framebuffer;
    bool running{true};
    rca_t rca;
    bool video_fullscreen{false};

    uint32_t blend(uint32_t c0, uint32_t c1);

    const uint32_t palette[4] = {
        0xff081820,
        0xff346856,
        0xff88c070,
        0xffe0f8d0
    };

    const uint8_t scanline_alpha = 176;

    bool scanlines{true};

public:
    host_t();
    ~host_t();

    void process_events();
    void events_wait_until_key_released(SDL_KeyCode key);
    void video_toggle_fullscreen();
    void run();
};

#endif
