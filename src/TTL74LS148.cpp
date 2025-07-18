// ---------------------------------------------------------------------
// TTL74LS148.cpp
// lime
//
// Copyright © 2019-2025 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include "TTL74LS148.hpp"
#include "common.hpp"
#include "core.hpp"

TTL74LS148_t::TTL74LS148_t(system_t *s)
{
	system = s;

	number_of_devices = 0;
	for (int i=0; i<256; i++)
		devices[i] = { true, 0 };
}

uint8_t TTL74LS148_t::connect_device(int level)
{
	devices[number_of_devices].level = level;
	number_of_devices++;
	return (number_of_devices - 1);
}

void TTL74LS148_t::pull_line(uint8_t handler)
{
	devices[handler].state = false;
	update_interrupt_level();
}

void TTL74LS148_t::release_line(uint8_t handler)
{
	devices[handler].state = true;
	update_interrupt_level();
}

void TTL74LS148_t::update_interrupt_level()
{
	unsigned int level = 0;
	for (int i=0; i<number_of_devices; i++) {
		if ((devices[i].state == false) && (devices[i].level > level))
			level = devices[i].level;
	}
	system->core->cpu_m68k->setIPL(level);
}
