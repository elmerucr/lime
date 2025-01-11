#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include <cstdint>
#include "common.hpp"

class debugger_t {
private:
public:
	debugger_t();
	~debugger_t();

	uint32_t *buffer;
};

#endif
