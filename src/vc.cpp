#include "vc.hpp"

vc_t::vc_t()
{
    ram = new uint8_t[0x10000];

    for (uint16_t address=0x2800; address<0x3800; address++) {
        
    }
}

vc_t::~vc_t()
{
    delete [] ram;
}
