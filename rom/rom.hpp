// ---------------------------------------------------------------------
// rom.hpp
// lime
//
// Copyright (C)2025 elmerucr. All rights reserved.
// Sun Mar  9 12:04:46 2025
// ---------------------------------------------------------------------

#ifndef ROM_HPP
#define ROM_HPP

#include <cstdint>

class rom_t {
public:
	const uint8_t data[512] = {
		0x72,0x6f,0x6d,0x20,0x30,0x2e,0x36,0x20,
		0x32,0x30,0x32,0x35,0x30,0x33,0x30,0x34,
		0x00,0x10,0xce,0x02,0x00,0xce,0xfe,0x00,
		0xb6,0x04,0x82,0x8a,0x02,0xb7,0x04,0x82,
		0x8e,0x10,0x00,0xa6,0x84,0xa7,0x80,0x8c,
		0x20,0x00,0x26,0xf7,0xb6,0x04,0x82,0x84,
		0xfd,0xb7,0x04,0x82,0x8e,0xfe,0xa6,0x5f,
		0xf7,0x04,0x07,0x10,0x8e,0x04,0x18,0xa6,
		0x80,0xa7,0xa0,0x10,0x8c,0x04,0x1c,0x26,
		0xf6,0x5c,0x8c,0xfe,0xc6,0x26,0xe9,0x0f,
		0x00,0x8e,0xfe,0xc6,0xbf,0x02,0x08,0x8e,
		0xff,0x2b,0x10,0x8e,0x02,0x10,0xaf,0xa1,
		0x10,0x8c,0x02,0x20,0x26,0xf8,0x8e,0xfe,
		0xfa,0xbf,0x02,0x20,0x86,0x9f,0xb7,0x04,
		0x03,0x86,0x01,0xb7,0x04,0x01,0x8d,0x05,
		0x1c,0xef,0x13,0x20,0xfd,0x36,0x32,0x8e,
		0x00,0x40,0x10,0x8e,0x05,0x00,0x6f,0xa0,
		0x30,0x1f,0x26,0xfa,0x86,0x7f,0x8e,0x00,
		0x08,0x10,0x8e,0x05,0x80,0xa7,0xa0,0x30,
		0x1f,0x26,0xfa,0x86,0x0f,0xb7,0x05,0x1b,
		0xb7,0x05,0x3b,0x37,0x32,0x39,0x70,0x40,
		0x07,0x1c,0x78,0x40,0x07,0x1d,0x70,0x48,
		0x07,0x1e,0x78,0x48,0x07,0x1f,0x6b,0x50,
		0x07,0x6c,0x70,0x50,0x07,0x69,0x76,0x50,
		0x07,0x6d,0x7e,0x50,0x07,0x65,0xb6,0x04,
		0x40,0x27,0x16,0x8e,0x02,0x10,0x86,0x01,
		0xb5,0x04,0x40,0x27,0x05,0xb7,0x04,0x40,
		0x6e,0x94,0x48,0x27,0x1c,0x30,0x02,0x20,
		0xef,0xb6,0x04,0x00,0x27,0x08,0xb7,0x04,
		0x00,0x8e,0x02,0x20,0x6e,0x94,0xb6,0x04,
		0x80,0x27,0x06,0xb7,0x04,0x80,0x7e,0xff,
		0x2c,0x3b,0x96,0x00,0x4c,0x81,0x80,0x26,
		0x06,0x86,0x01,0xb7,0x04,0x81,0x4f,0x97,
		0x00,0xc6,0x04,0xf7,0x04,0x07,0x86,0x50,
		0xb7,0x04,0x19,0xb6,0x04,0x18,0x90,0x00,
		0x80,0x08,0x22,0x03,0x7a,0x04,0x19,0x5c,
		0xc1,0x08,0x26,0xe7,0xb6,0x04,0x88,0xb7,
		0x04,0x04,0x3b,0x3b,0xb6,0x04,0x84,0x26,
		0x32,0xb6,0x04,0x84,0xf6,0x04,0x84,0x1f,
		0x02,0xb6,0x04,0x84,0xf6,0x04,0x84,0x1f,
		0x01,0xb6,0x04,0x84,0xa7,0x80,0x31,0x3f,
		0x26,0xf7,0xb6,0x04,0x84,0x81,0xff,0x26,
		0x12,0xb6,0x04,0x84,0x26,0x0d,0xb6,0x04,
		0x84,0x26,0x08,0xb6,0x04,0x84,0xf6,0x04,
		0x84,0xdd,0x01,0x3b,0x6e,0x9f,0x02,0x08,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0xff,0x64,0x00,0x00,0x00,0x00,0xfe,0x11
	};
};

#endif
