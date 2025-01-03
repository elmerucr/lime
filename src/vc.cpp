#include "vc.hpp"

vc_t::vc_t()
{
    ram = new uint8_t[0x10000];
}

vc_t::~vc_t()
{
    delete [] ram;
}
