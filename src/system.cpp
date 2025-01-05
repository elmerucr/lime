#include "system.hpp"
#include "host.hpp"
#include "vdc.hpp"

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

    while (running) {
        running = host->process_events();

        vdc->bg0_x++;
        vdc->bg0_y++;

        vdc->update();
        host->update_screen();
    }
}
