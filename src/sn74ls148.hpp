// ---------------------------------------------------------------------
// sn74ls148.hpp
// lime
//
// Copyright © 2019-2025 elmerucr. All rights reserved.
//
// exception collector and
// priority encoder
// 8 input lines, 3 bit output
// ---------------------------------------------------------------------

#ifndef SN74LS148_HPP
#define SN74LS148_HPP

#include "system.hpp"

#include <cstdint>
#include <string>

class sn74ls148_t {
private:
    system_t *system;

	std::string empty_name = "unassigned";

	struct device {
		bool state;
		int level;
		std::string dev_name;
	};
	struct device devices[256];
	uint8_t number_of_devices;
	unsigned int level;
public:
	sn74ls148_t(system_t *s);

	void pull_line(uint8_t handler);
	void release_line(uint8_t handler);

	/* Recalculates interrupt level based on individual connections */
	void update_interrupt_level();

	/*
	 * When connecting a device, both a pointer to a pin and an interrupt
	 * level (1-6) must be supplied. Returns a unique interrupt_device_no
	 */
	uint8_t connect_device(int level, std::string n);

	/*
	 * Last calculated level
	 */
	unsigned int get_ipl_level() { return level; }

	void status(char *b, int buffer_length);
};

#endif
