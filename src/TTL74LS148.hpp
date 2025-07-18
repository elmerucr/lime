// ---------------------------------------------------------------------
// TTL74LS148.hpp
// lime
//
// Copyright © 2019-2025 elmerucr. All rights reserved.
//
// exception collector and
// priority encoder
// 8 input lines, 3 bit output
// ---------------------------------------------------------------------

#ifndef TTL74LS148_HPP
#define TTL74LS148_HPP

#include "system.hpp"

#include <cstdint>

class TTL74LS148_t {
private:
    system_t *system;
	struct device {
		bool state;
		int level;
	};
	struct device devices[256];
	uint8_t number_of_devices;
	int output_level;
public:
	TTL74LS148_t(system_t *s);

	void pull_line(uint8_t handler);
	void release_line(uint8_t handler);

	/* Recalculates interrupt level based on individual connections */
	void update_interrupt_level();

	/*
	 * When connecting a device, both a pointer to a pin and an interrupt
	 * level (1-6) must be supplied. Returns a unique interrupt_device_no
	 */
	uint8_t connect_device(int level);
};

#endif
