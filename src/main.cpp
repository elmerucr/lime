#include "host.hpp"

int main()
{
    host_t *host = new host_t();
    host->run();
    delete host;
    return 0;
}
