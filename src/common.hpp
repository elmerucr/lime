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
#define LIME_MINOR_VERSION	2
#define LIME_BUILD			20250202
#define LIME_YEAR			2025

// ---------------------------------------------------------------------
// video
// ---------------------------------------------------------------------
#define VRAM_SIZE		0x10000
#define VIDEO_XRES		240
#define VIDEO_YRES		160
#define VIDEO_SCANLINES	480
#define	FPS				60

// ---------------------------------------------------------------------
// vdc (video display controller)
// ---------------------------------------------------------------------
#define VDC_TILESET_0_ADDRESS	0x0800
#define VDC_TILESET_1_ADDRESS	0x1000
#define VDC_LAYERS_ADDRESS		0x2000

// ---------------------------------------------------------------------
// audio
// ---------------------------------------------------------------------
#define SAMPLE_RATE				48000
#define AUDIO_BUFFER_SIZE		8000.0
#define SID_CLOCK_SPEED			985248
#define SID_CYCLES_PER_FRAME	(SID_CLOCK_SPEED/FPS)

// ---------------------------------------------------------------------
// cpu
// ---------------------------------------------------------------------
#define CPU_CYCLES_PER_SCANLINE	(1*VIDEO_XRES)
#define CPU_CYCLES_PER_FRAME	(CPU_CYCLES_PER_SCANLINE*VIDEO_SCANLINES)
#define CPU_CLOCK_SPEED			(CPU_CYCLES_PER_FRAME*FPS)

// ---------------------------------------------------------------------
// debugger
// ---------------------------------------------------------------------
#define DEBUGGER_WIDTH	600
#define DEBUGGER_HEIGHT	400

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

// ---------------------------------------------------------------------
// Ascii values (some of them are petscii)
// ---------------------------------------------------------------------
#define ASCII_NULL          0x00    // null
#define ASCII_BACKSPACE     0x08
#define ASCII_HOR_TAB       0x09
#define ASCII_CR            0x0d    // carriage return
#define ASCII_LF            0x0a    // linefeed
#define ASCII_CURSOR_DOWN   0x11    // petscii
#define ASCII_REVERSE_ON    0x12    // petscii
#define ASCII_ESCAPE        0x1b
#define ASCII_CURSOR_RIGHT  0x1d    // petscii
#define ASCII_SPACE         0x20    // space
#define ASCII_EXCL_MARK     0x21    // !
#define ASCII_DOUBLE_QUOTES 0x22    // "
#define ASCII_NUMBER        0x23    // #
#define ASCII_DOLLAR        0x24    // $
#define ASCII_PERCENT       0x25    // %
#define ASCII_AMPERSAND     0x26    // &
#define ASCII_SINGLE_QUOTE  0x27    // '
#define ASCII_OPEN_PAR      0x28    // (
#define ASCII_CLOSE_PAR     0x29    // )
#define ASCII_ASTERISK      0x2a    // *
#define ASCII_PLUS          0x2b    // +
#define ASCII_COMMA         0x2c    // ,
#define ASCII_HYPHEN        0x2d    // -
#define ASCII_PERIOD        0x2e    // .
#define ASCII_SLASH         0x2f    // /
#define ASCII_0             0x30    // 0
#define ASCII_1             0x31    // 1
#define ASCII_2             0x32    // 2
#define ASCII_3             0x33    // 3
#define ASCII_4             0x34    // 4
#define ASCII_5             0x35    // 5
#define ASCII_6             0x36    // 6
#define ASCII_7             0x37    // 7
#define ASCII_8             0x38    // 8
#define ASCII_9             0x39    // 9
#define ASCII_COLON         0x3a    // :
#define ASCII_SEMI_COLON    0x3b    // ;
#define ASCII_LESS          0x3c    // <
#define ASCII_EQUALS        0x3d    // =
#define ASCII_GREATER       0x3e    // >
#define ASCII_QUESTION_M    0x3f    // ?
#define ASCII_AT            0x40    // @
#define ASCII_A             0x41
#define ASCII_B             0x42
#define ASCII_C             0x43
#define ASCII_D             0x44
#define ASCII_E             0x45
#define ASCII_F             0x46
#define ASCII_G             0x47
#define ASCII_H             0x48
#define ASCII_I             0x49
#define ASCII_J             0x4a
#define ASCII_K             0x4b
#define ASCII_L             0x4c
#define ASCII_M             0x4d
#define ASCII_N             0x4e
#define ASCII_O             0x4f
#define ASCII_P             0x50
#define ASCII_Q             0x51
#define ASCII_R             0x52
#define ASCII_S             0x53
#define ASCII_T             0x54
#define ASCII_U             0x55
#define ASCII_V             0x56
#define ASCII_W             0x57
#define ASCII_X             0x58
#define ASCII_Y             0x59
#define ASCII_Z             0x5a
#define ASCII_OPEN_BRACK    0x5b    // [
#define ASCII_BACKSLASH     0x5c    // \    patched
#define ASCII_CLOSE_BRACK   0x5d    // ]
#define ASCII_CARET         0x5e    // ^    patched
#define ASCII_UNDERSCORE    0x5f    // _
#define ASCII_GRAVE         0x60    // `
#define ASCII_a             0x61
#define ASCII_b             0x62
#define ASCII_c             0x63
#define ASCII_d             0x64
#define ASCII_e             0x65
#define ASCII_f             0x66
#define ASCII_g             0x67
#define ASCII_h             0x68
#define ASCII_i             0x69
#define ASCII_j             0x6a
#define ASCII_k             0x6b
#define ASCII_l             0x6c
#define ASCII_m             0x6d
#define ASCII_n             0x6e
#define ASCII_o             0x6f
#define ASCII_p             0x70
#define ASCII_q             0x71
#define ASCII_r             0x72
#define ASCII_s             0x73
#define ASCII_t             0x74
#define ASCII_u             0x75
#define ASCII_v             0x76
#define ASCII_w             0x77
#define ASCII_x             0x78
#define ASCII_y             0x79
#define ASCII_z             0x7a
#define ASCII_OPEN_BRACE    0x7b    // {
#define ASCII_VERT_BAR      0x7c    // |
#define ASCII_CLOSE_BRACE   0x7d    // }
#define ASCII_TILDE         0x7e    // ~
#define ASCII_DELETE        0x7f

#define ASCII_F1            0x85    // taken from cbm petscii
#define ASCII_F2            0x86    // taken from cbm petscii
#define ASCII_F3            0x87    // taken from cbm petscii
#define ASCII_F4            0x88    // taken from cbm petscii
#define ASCII_F5            0x89    // taken from cbm petscii
#define ASCII_F6            0x8a    // taken from cbm petscii
#define ASCII_F7            0x8b    // taken from cbm petscii
#define ASCII_F8            0x8c    // taken from cbm petscii

#define ASCII_CURSOR_UP     0x91    // petscii cursor up
#define ASCII_REVERSE_OFF   0x92    // petscii
#define ASCII_CURSOR_LEFT   0x9d    // petscii cursor left

#endif
