/*
 * cpu_mc6809.cpp
 * lime
 *
 * Copyright © 2023-2025 elmerucr. All rights reserved.
 */

#include "cpu_mc6809.hpp"
#include "core.hpp"

uint8_t cpu_mc6809_t::read8(uint16_t address) const {
	return system->core->read8(address);
}

void cpu_mc6809_t::write8(uint16_t address, uint8_t value) const {
	system->core->write8(address, value);
}
