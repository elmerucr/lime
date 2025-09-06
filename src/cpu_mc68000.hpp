/*
 * cpu_mc68000.hpp
 * lime
 *
 * Copyright © 2025 elmerucr. All rights reserved.
 */

#ifndef CPU_MC68000_HPP
#define CPU_MC68000_HPP

#include "Moira.h"
#include "system.hpp"

using namespace moira;

class cpu_mc68000_t : public Moira {
private:
	u8   read8 (u32 addr) const override;
	u16  read16(u32 addr) const override;
	void write8 (u32 addr, u8  val) const override;
	void write16(u32 addr, u16 val) const override;
	void didReachBreakpoint(u32 addr) override;
public:
	cpu_mc68000_t(system_t *s);
	system_t *system;

	i64 old_clock;
	bool breakpoint_reached = false;
};

#endif
