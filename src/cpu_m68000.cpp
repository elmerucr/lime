/*
 * cpu_m68000.cpp
 * lime
 *
 * Copyright © 2025 elmerucr. All rights reserved.
 */

#include "cpu_m68000.hpp"
#include "core.hpp"

cpu_m68000_t::cpu_m68000_t(system_t *s)
{
	system = s;
}

u8 cpu_m68000_t::read8(u32 addr) const
{
	return system->core->read8(addr);
}

u16 cpu_m68000_t::read16(u32 addr) const
{
	return (system->core->read8(addr) << 8) | system->core->read8(addr + 1);
}

void cpu_m68000_t::write8 (u32 addr, u8 val) const
{
	system->core->write8(addr, val);
}

void cpu_m68000_t::write16(u32 addr, u16 val) const
{
	system->core->write8(addr, (val & 0xff00) >> 8);
	system->core->write8(addr + 1, val & 0x00ff);
}
