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

// ---------------------------------------------------------------------
// debugger
// ---------------------------------------------------------------------
#define DEBUGGER_WIDTH	480
#define DEBUGGER_HEIGHT	320

// ---------------------------------------------------------------------
// gameboy colors
// ---------------------------------------------------------------------
#define	GB_COLOR_0	0xff081820
#define GB_COLOR_1	0xff346856
#define GB_COLOR_2	0xff88c070
#define GB_COLOR_3	0xffe0f8d0

// ---------------------------------------------------------------------
// C64 colors (VirtualC64)
// ---------------------------------------------------------------------
#define C64_BLACK       0xff'00'00'00	//   0,   0,   0
#define C64_WHITE       0xff'ff'ff'ff	// 255, 255, 255
#define C64_RED         0xff'78'38'39	// 120,  56,  57
#define C64_CYAN        0xff'89'cb'c7	// 137, 203, 199
#define C64_PURPLE      0xff'83'40'93	// 131,  64, 147
#define C64_GREEN       0xff'6b'aa'59	// 107, 170,  89
#define C64_BLUE        0xff'2d'2b'95	//  45,  43, 149
#define C64_YELLOW      0xff'ed'f0'82	// 237, 240, 130
#define C64_ORANGE      0xff'85'53'31	// 133,  83,  49
#define C64_BROWN       0xff'50'39'0e	//  80,  57,  14
#define C64_LIGHTRED    0xff'b7'70'72	// 183, 112, 114
#define C64_DARKGREY    0xff'49'49'49	//  73,  73,  73
#define C64_GREY        0xff'7b'7b'7b	// 123, 123, 123
#define C64_LIGHTGREEN  0xff'bb'fd'a9	// 187, 253, 169
#define C64_LIGHTBLUE   0xff'6f'6c'e3	// 111, 108, 227
#define C64_LIGHTGREY   0xff'b1'b1'b1	// 177, 177, 177

#endif
