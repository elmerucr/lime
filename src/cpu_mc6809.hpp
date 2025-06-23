/*
 * cpu_mc6809.hpp
 * lime
 *
 * Copyright © 2023-2025 elmerucr. All rights reserved.
 */

#ifndef CPU_MC6809_HPP
#define CPU_MC6809_HPP

#include "mc6809.hpp"
#include "system.hpp"

class cpu_mc6809_t : public mc6809 {
public:
	cpu_mc6809_t(system_t *s) {
		system = s;
	}

	uint8_t read8(uint16_t address) const;
	void write8(uint16_t address, uint8_t value) const;

private:
	system_t *system;
};

#endif
