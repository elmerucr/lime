// ---------------------------------------------------------------------
// timer.cpp
// lime
//
// Copyright © 2019-2025 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include "timer.hpp"
#include "common.hpp"
#include <cstdio>

timer_ic::timer_ic(exceptions_ic *e, sn74ls148_t *t)
{
	exceptions = e;
	dev_number_exceptions = exceptions->connect_device("timer");
	printf("[timer] Connecting to exceptions getting dev %i\n", dev_number_exceptions);

	sn74ls148 = t;
	dev_number_sn74ls148 = sn74ls148->connect_device(4, "timer");
	printf("[timer] Connecting to sn74ls148 at ipl 4 getting dev %i\n", dev_number_sn74ls148);
}

void timer_ic::reset()
{
	// No pending irq's
	status_register = 0x00;

	// All timers turned off
	control_register = 0x00;

	for (int i=0; i<8; i++) {
		timer[i].bpm = 0x0001; // load with 1, may never be zero
		timer[i].clock_interval = bpm_to_clock_interval(timer[i].bpm);
		timer[i].counter = 0;
	}

	exceptions->release(dev_number_exceptions);
	sn74ls148->release_line(dev_number_sn74ls148);
}

void timer_ic::run(uint32_t number_of_cycles)
{
	for (int i=0; i<8; i++) {
		timer[i].counter += number_of_cycles;
		if ((timer[i].counter >= timer[i].clock_interval)) {
			while (timer[i].counter >= timer[i].clock_interval)
				timer[i].counter -= timer[i].clock_interval;
			if (control_register & (0b1 << i)) {
				exceptions->pull(dev_number_exceptions);
				sn74ls148->pull_line(dev_number_sn74ls148);
				status_register |= (0b1 << i);
			}
		}
	}
}

uint32_t timer_ic::bpm_to_clock_interval(uint16_t bpm)
{
	return (60.0 / bpm) * CORE_CLOCK_SPEED;
}

uint8_t timer_ic::io_read_byte(uint8_t address)
{
	switch (address & 0x1f) {
		case 0x00:
			return status_register;
		case 0x01:
			return control_register;
		case 0x10:
			return (timer[0].bpm & 0xff00) >> 8;
		case 0x11:
			return timer[0].bpm & 0xff;
		case 0x12:
			return (timer[1].bpm & 0xff00) >> 8;
		case 0x13:
			return timer[1].bpm & 0xff;
		case 0x14:
			return (timer[2].bpm & 0xff00) >> 8;
		case 0x15:
			return timer[2].bpm & 0xff;
		case 0x16:
			return (timer[3].bpm & 0xff00) >> 8;
		case 0x17:
			return timer[3].bpm & 0xff;
		case 0x18:
			return (timer[4].bpm & 0xff00) >> 8;
		case 0x19:
			return timer[4].bpm & 0xff;
		case 0x1a:
			return (timer[5].bpm & 0xff00) >> 8;
		case 0x1b:
			return timer[5].bpm & 0xff;
		case 0x1c:
			return (timer[6].bpm & 0xff00) >> 8;
		case 0x1d:
			return timer[6].bpm & 0xff;
		case 0x1e:
			return (timer[7].bpm & 0xff00) >> 8;
		case 0x1f:
			return timer[7].bpm & 0xff;
		default:
			return 0;
	}
}

void timer_ic::io_write_byte(uint8_t address, uint8_t byte)
{
	switch (address & 0x1f) {
		case 0x00:
			// ---------------------------------------------------------
			// b s   r
			// 0 0 = 0
			// 0 1 = 1
			// 1 0 = 0
			// 1 1 = 0
			//
			// b = bit that's written
			// s = status (on if an interrupt was caused)
			// r = boolean result (acknowledge an interrupt (s=1) if b=1
			// r = (~b) & s
			// ---------------------------------------------------------
			status_register = (~byte) & status_register;
			if ((status_register & 0xff) == 0) {
				// no timers left causing interrupts
				exceptions->release(dev_number_exceptions);
				sn74ls148->release_line(dev_number_sn74ls148);
			}
			break;
		case 0x01:
		{
			uint8_t turned_on = byte & (~control_register);
			for (int i=0; i<8; i++) {
				if (turned_on & (0b1 << i)) {
					timer[i].counter = 0;
				}
			}
			control_register = byte;
			break;
		}
		case 0x10:
			timer[0].bpm = (timer[0].bpm & 0x00ff) | (byte << 8);
			if (timer[0].bpm == 0) timer[0].bpm = 1;
			timer[0].clock_interval = bpm_to_clock_interval(timer[0].bpm);
			break;
		case 0x11:
			timer[0].bpm = (timer[0].bpm & 0xff00) | byte;
			if (timer[0].bpm == 0) timer[0].bpm = 1;
			timer[0].clock_interval = bpm_to_clock_interval(timer[0].bpm);
			break;
		case 0x12:
			timer[1].bpm = (timer[1].bpm & 0x00ff) | (byte << 8);
			if (timer[1].bpm == 0) timer[0].bpm = 1;
			timer[1].clock_interval = bpm_to_clock_interval(timer[1].bpm);
			break;
		case 0x13:
			timer[1].bpm = (timer[1].bpm & 0xff00) | byte;
			if (timer[1].bpm == 0) timer[0].bpm = 1;
			timer[1].clock_interval = bpm_to_clock_interval(timer[1].bpm);
			break;
		case 0x14:
			timer[2].bpm = (timer[2].bpm & 0x00ff) | (byte << 8);
			if (timer[2].bpm == 0) timer[0].bpm = 1;
			timer[2].clock_interval = bpm_to_clock_interval(timer[2].bpm);
			break;
		case 0x15:
			timer[2].bpm = (timer[2].bpm & 0xff00) | byte;
			if (timer[2].bpm == 0) timer[0].bpm = 1;
			timer[2].clock_interval = bpm_to_clock_interval(timer[2].bpm);
			break;
		case 0x16:
			timer[3].bpm = (timer[3].bpm & 0x00ff) | (byte << 8);
			if (timer[3].bpm == 0) timer[0].bpm = 1;
			timer[3].clock_interval = bpm_to_clock_interval(timer[3].bpm);
			break;
		case 0x17:
			timer[3].bpm = (timer[3].bpm & 0xff00) | byte;
			if (timer[3].bpm == 0) timer[0].bpm = 1;
			timer[3].clock_interval = bpm_to_clock_interval(timer[3].bpm);
			break;
		case 0x18:
			timer[4].bpm = (timer[4].bpm & 0x00ff) | (byte << 8);
			if (timer[4].bpm == 0) timer[0].bpm = 1;
			timer[4].clock_interval = bpm_to_clock_interval(timer[4].bpm);
			break;
		case 0x19:
			timer[4].bpm = (timer[4].bpm & 0xff00) | byte;
			if (timer[4].bpm == 0) timer[0].bpm = 1;
			timer[4].clock_interval = bpm_to_clock_interval(timer[4].bpm);
			break;
		case 0x1a:
			timer[5].bpm = (timer[5].bpm & 0x00ff) | (byte << 8);
			if (timer[5].bpm == 0) timer[0].bpm = 1;
			timer[5].clock_interval = bpm_to_clock_interval(timer[5].bpm);
			break;
		case 0x1b:
			timer[5].bpm = (timer[5].bpm & 0xff00) | byte;
			if (timer[5].bpm == 0) timer[0].bpm = 1;
			timer[5].clock_interval = bpm_to_clock_interval(timer[5].bpm);
			break;
		case 0x1c:
			timer[6].bpm = (timer[6].bpm & 0x00ff) | (byte << 8);
			if (timer[6].bpm == 0) timer[0].bpm = 1;
			timer[6].clock_interval = bpm_to_clock_interval(timer[6].bpm);
			break;
		case 0x1d:
			timer[6].bpm = (timer[6].bpm & 0xff00) | byte;
			if (timer[6].bpm == 0) timer[0].bpm = 1;
			timer[6].clock_interval = bpm_to_clock_interval(timer[6].bpm);
			break;
		case 0x1e:
			timer[7].bpm = (timer[7].bpm & 0x00ff) | (byte << 8);
			if (timer[7].bpm == 0) timer[0].bpm = 1;
			timer[7].clock_interval = bpm_to_clock_interval(timer[7].bpm);
			break;
		case 0x1f:
			timer[7].bpm = (timer[7].bpm & 0xff00) | byte;
			if (timer[7].bpm == 0) timer[0].bpm = 1;
			timer[7].clock_interval = bpm_to_clock_interval(timer[7].bpm);
			break;
		default:
			// do nothing
			break;
	}
}

void timer_ic::set(uint8_t timer_no, uint16_t bpm)
{
	timer_no &= 0b111; // limit to max 7

	if (bpm == 0) bpm = 1;
	timer[timer_no].bpm = bpm;
	timer[timer_no].clock_interval = bpm_to_clock_interval(bpm);

	uint8_t byte = io_read_byte(0x01);
	io_write_byte(0x01, (0b1 << timer_no) | byte);
}

void timer_ic::status(char *buffer, uint8_t timer_no)
{
	timer_no &= 0b00000111;

	snprintf(buffer, 64, "\n%2u:%s/%5u/%10u/%10u",
		 timer_no,
		 control_register & (0b1 << timer_no) ? " on" : "off",
		 timer[timer_no].bpm,
		 timer[timer_no].counter,
		 timer[timer_no].clock_interval);
}
