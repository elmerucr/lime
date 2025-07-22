// ---------------------------------------------------------------------
// ttl74ls148.cpp
// lime
//
// Copyright © 2019-2025 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include "ttl74ls148.hpp"
#include "common.hpp"
#include "core.hpp"

ttl74ls148_t::ttl74ls148_t(system_t *s)
{
	system = s;

	number_of_devices = 0;

	for (int i=0; i<256; i++) {
		devices[i] = { true, 0, empty_name };
	}

	level = 0;
}

uint8_t ttl74ls148_t::connect_device(int level, std::string n)
{
	devices[number_of_devices].level = level;
	devices[number_of_devices].dev_name = n;
	number_of_devices++;
	return (number_of_devices - 1);
}

void ttl74ls148_t::pull_line(uint8_t handler)
{
	devices[handler].state = false;
	update_interrupt_level();
}

void ttl74ls148_t::release_line(uint8_t handler)
{
	devices[handler].state = true;
	update_interrupt_level();
}

void ttl74ls148_t::update_interrupt_level()
{
	level = 0;

	for (int i=0; i<number_of_devices; i++) {
		if ((devices[i].state == false) && (devices[i].level > level))
			level = devices[i].level;
	}

	system->core->cpu_m68k->setIPL(level);
}

void ttl74ls148_t::status(char *b, int buffer_length)
{
	b += snprintf(b, buffer_length, "----ttl74ls148-------");
	b += snprintf(b, buffer_length, "\n dev ipl lin name");
	for (int i=0; i<number_of_devices; i++) {
		b += snprintf(b, buffer_length, "\n  %1i   %1i   %c  \"%s\"", i, devices[i].level, devices[i].state ? '1' : '0', devices[i].dev_name.c_str());
	}
	b += snprintf(b, buffer_length, "\n\n     output:%i", level);
}

// void TTL74LS148_t::status(char *b, int buffer_length, uint8_t device)
// {
// 	if (device < next_available_device) {
// 		snprintf(b, buffer_length, "%1i  %c \"%s\"", device, irq_input_pins[device] ? '1' : '0', dev_name[device]);
// 	} else {
// 		b[0] = '\0';
// 	}
// }
