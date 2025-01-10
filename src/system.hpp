// ---------------------------------------------------------------------
// 0x0000-0x00ff base page
// 0x0100-0x01ff system stack
// 0x0200-0x03ff io
// 0x0400-0x07ff sprite table (128 x 8 bytes), xpos, ypos, flags, pointer, palette
// 0x2000-0x37ff tilesets (6kb), 0x2000 start of set 0, 0x2800 start of set 1
// ---------------------------------------------------------------------

#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <chrono>
#include <thread>

enum mode {
	DEBUG_MODE = 0,
	RUN_MODE = 1
};

class host_t;
class core_t;
class vdc_t;

class system_t {
private:
	std::chrono::time_point<std::chrono::steady_clock> system_start_time;
	std::chrono::time_point<std::chrono::steady_clock> end_of_frame_time;
public:
    system_t();
    ~system_t();

    host_t *host;
	core_t *core;
    vdc_t *vdc;

	enum mode current_mode;
	void switch_mode();
	void switch_to_debug_mode();
	void switch_to_run_mode();

    void run();

    bool running;
};

#endif
