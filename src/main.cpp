#include "host.hpp"
#include "vdc.hpp"

int main()
{
    vdc_t vdc;
    vdc.run();

    host_t *host = new host_t(vdc.buffer);
    host->run();
    delete host;

    return 0;
}
