// ---------------------------------------------------------------------
// common.hpp
// lime
//
// Copyright © 2025 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#ifndef COMMON_HPP
#define COMMON_HPP

// ---------------------------------------------------------------------
// Version information
// ---------------------------------------------------------------------
#define LIME_MAJOR_VERSION	0
#define LIME_MINOR_VERSION	1
#define LIME_BUILD			20250112
#define LIME_YEAR			2025

// ---------------------------------------------------------------------
// Video
// ---------------------------------------------------------------------
#define VRAM_SIZE			0x10000
#define PIXELS_PER_SCANLINE	240
#define SCANLINES			160

// vdc
#define VDC_TILESET_0	0x0800
#define VDC_TILESET_1	0x1000

#define VDC_LAYERS		0x2000

// gameboy colors
#define	GB_COLOR_0	0xff081820
#define GB_COLOR_1	0xff346856
#define GB_COLOR_2	0xff88c070
#define GB_COLOR_3	0xffe0f8d0

#endif
