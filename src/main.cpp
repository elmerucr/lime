#include "host.hpp"
#include "vdc.hpp"

int main()
{
    host_t *host = new host_t();
    host->run();
    delete host;

    vdc_t vdc;

    return 0;
}
