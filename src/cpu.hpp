/*
 * cpu.hpp
 * lime
 *
 * Copyright © 2025-2026 elmerucr. All rights reserved.
 */

#ifndef CPU_HPP
#define CPU_HPP

#include "Moira.h"
#include "system.hpp"

using namespace moira;

class cpu_t : public Moira {
private:
	u8   read8 (u32 addr) const override;
	u16	 read16(u32 addr) const override;
	u32  read32(u32 addr) const override;
	void write8 (u32 addr, u8  val) const override;
	void write16(u32 addr, u16 val) const override;
	void write32(u32 addr, u32 val) const override;
	void didReachBreakpoint(u32 addr) override;
public:
	cpu_t(system_t *s);
	~cpu_t();
	void reset();
	system_t *system;

	i64 old_clock;
	bool breakpoint_reached = false;
};

#endif
