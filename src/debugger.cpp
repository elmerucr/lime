


#include "debugger.hpp"

debugger_t::debugger_t()
{
	buffer = new uint32_t[4 * PIXELS_PER_SCANLINE * SCANLINES];
}

debugger_t::~debugger_t()
{
	delete [] buffer;
}
