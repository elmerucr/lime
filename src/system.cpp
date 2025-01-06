#include "system.hpp"
#include "host.hpp"
#include "vdc.hpp"
#include <cmath>

system_t::system_t()
{
    host = new host_t(this);
    vdc = new vdc_t();
}

system_t::~system_t()
{
    delete vdc;
    delete host;
}

void system_t::run()
{
    running = true;

	vdc->sprite[0].x = 88;
	vdc->sprite[0].y = 52;
	vdc->sprite[0].index = 1;
	vdc->sprite[0].flags = 0b00000101;
	vdc->sprite[1].x = 96;
	vdc->sprite[1].y = 52;
	vdc->sprite[1].index = 2;
	vdc->sprite[1].flags = 0b00000101;
	vdc->sprite[2].x = 88;
	vdc->sprite[2].y = 60;
	vdc->sprite[2].index = 3;
	vdc->sprite[2].flags = 0b00000101;
	vdc->sprite[3].x = 96;
	vdc->sprite[3].y = 60;
	vdc->sprite[3].index = 4;
	vdc->sprite[3].flags = 0b00000101;

    while (running) {
        running = host->process_events();

		static uint8_t t = 0;

		vdc->bg0_x = (8 * cos(8.0*M_PI*((float)t)/255.0));
		vdc->bg0_y = (8 * sin(8.0*M_PI*((float)t)/255.0));
		t++;

        vdc->update();
        host->update_screen();
    }
}
