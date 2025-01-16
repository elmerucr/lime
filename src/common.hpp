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
#define LIME_BUILD			20250116
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

// ------
// debugger
//
#define DEBUGGER_WIDTH	480
#define DEBUGGER_HEIGHT	320

// gameboy colors
#define	GB_COLOR_0	0xff081820
#define GB_COLOR_1	0xff346856
#define GB_COLOR_2	0xff88c070
#define GB_COLOR_3	0xffe0f8d0

// ---------------------------------------------------------------------
// C64 colors (VirtualC64) argb4444 format
// ---------------------------------------------------------------------
#define C64_BLACK       0xff000000
#define C64_WHITE       0xffffffff
#define C64_RED         0xff773333
#define C64_CYAN        0xff88cccc
#define C64_PURPLE      0xff884499
#define C64_GREEN       0xff66aa55

#define C64_BLUE        0xff2d2b95	// 45 43 149

#define C64_YELLOW      0xffeeee88
#define C64_ORANGE      0xff885533
#define C64_BROWN       0xff553311
#define C64_LIGHTRED    0xffbb7777
#define C64_DARKGREY    0xff444444
#define C64_GREY        0xff777777

#define C64_LIGHTGREEN  0xff9ae29b
#define C64_LIGHTBLUE   0xff6f6ce3	// 111 108 227
#define C64_LIGHTGREY   0xffadadad

#endif
