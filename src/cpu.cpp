/*
 * cpu.cpp
 * lime
 *
 * Copyright © 2025-2026 elmerucr. All rights reserved.
 */

#include "cpu.hpp"
#include "core.hpp"

cpu_t::cpu_t(system_t *s)
{
	printf("[cpu] mc680000 Moira %s\n", build().c_str());
	system = s;
}

cpu_t::~cpu_t()
{
	printf("[cpu] cleaning up\n");
}

void cpu_t::reset()
{
	printf("[cpu] resetting\n");
	Moira::reset();
}

u8 cpu_t::read8(u32 addr) const
{
	return system->core->read8(addr);
}

u16 cpu_t::read16(u32 addr) const
{
	return (system->core->read8(addr) << 8) | system->core->read8(addr + 1);
}

u32 cpu_t::read32(u32 addr) const
{
	return
		(system->core->read8(addr) << 24)		|
		(system->core->read8(addr + 1) << 16)	|
		(system->core->read8(addr + 2) <<  8)	|
		 system->core->read8(addr + 3)			;
}

void cpu_t::write8 (u32 addr, u8 val) const
{
	system->core->write8(addr, val);
}

void cpu_t::write16(u32 addr, u16 val) const
{
	system->core->write8(addr, (val & 0xff00) >> 8);
	system->core->write8(addr + 1, val & 0x00ff);
}

void cpu_t::write32(u32 addr, u32 val) const
{
	system->core->write8(addr + 0, (val & 0xff000000) >> 24);
	system->core->write8(addr + 1, (val & 0x00ff0000) >> 16);
	system->core->write8(addr + 2, (val & 0x0000ff00) >>  8);
	system->core->write8(addr + 3, val & 0x000000ff);
}

void cpu_t::didReachBreakpoint(u32 addr)
{
	breakpoint_reached = true;
}
