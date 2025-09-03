// ---------------------------------------------------------------------
// clocks.hpp
// lime
//
// Copyright © 2019-2025 elmerucr. All rights reserved.
//
// Algorithm based on bresenham line algorithm.
// ---------------------------------------------------------------------

#ifndef CLOCKS_HPP
#define CLOCKS_HPP

#include <cstdint>

class clocks {
private:
	uint64_t base_clock_freq;
	uint64_t target_clock_freq;

	uint64_t mult;
	uint64_t mod;
public:
	clocks(uint32_t base_clock_f, uint32_t target_clock_f) {
		base_clock_freq = base_clock_f;
		target_clock_freq = target_clock_f;
		mod = 0;
	}

	inline uint32_t clock(uint32_t delta_base_clock) {
		mult = (delta_base_clock * target_clock_freq) + mod;
		mod  = mult % base_clock_freq;
		return (uint32_t) mult / base_clock_freq;
	}

	inline void adjust_base_clock(uint32_t base_clock_f) {
		base_clock_freq = base_clock_f;
	}

	inline void adjust_target_clock(uint32_t target_clock_f) {
		target_clock_freq = target_clock_f;
	}
};

#endif
