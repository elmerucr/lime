// ---------------------------------------------------------------------
// sn74ls148.cpp
// lime
//
// Copyright © 2019-2025 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include "sn74ls148.hpp"
#include "common.hpp"
#include "core.hpp"

sn74ls148_t::sn74ls148_t(system_t *s)
{
	system = s;

	number_of_devices = 0;

	for (int i=0; i<256; i++) {
		devices[i] = { true, 0, empty_name };
	}

	level = 0;
}

uint8_t sn74ls148_t::connect_device(int level, std::string n)
{
	devices[number_of_devices].level = level;
	devices[number_of_devices].dev_name = n;
	number_of_devices++;
	return (number_of_devices - 1);
}

void sn74ls148_t::pull_line(uint8_t handler)
{
	devices[handler].state = false;
	update_interrupt_level();
}

void sn74ls148_t::release_line(uint8_t handler)
{
	devices[handler].state = true;
	update_interrupt_level();
}

void sn74ls148_t::update_interrupt_level()
{
	level = 0;

	for (int i=0; i<number_of_devices; i++) {
		if ((devices[i].state == false) && (devices[i].level > level))
			level = devices[i].level;
	}

	system->core->mc68000->setIPL(level);
}

void sn74ls148_t::status(char *b, int buffer_length)
{
	b += snprintf(b, buffer_length, "-----sn74ls148----- ");
	b += snprintf(b, buffer_length, "dev ipl lin devname");
	for (int i=0; i<number_of_devices; i++) {
		b += snprintf(b, buffer_length, "\n %1i   %1i   %c  \"%s\"", i, devices[i].level, devices[i].state ? '1' : '0', devices[i].dev_name.c_str());
	}
}
