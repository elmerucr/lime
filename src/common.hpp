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
#define LIME_MAJOR_VERSION		0
#define LIME_MINOR_VERSION		7
#define LIME_BUILD				20250523
#define LIME_YEAR				2025

// ---------------------------------------------------------------------
// vdc (video display controller)
// ---------------------------------------------------------------------
#define VDC_RAM					0x10000
#define VDC_XRES				240
#define VDC_YRES				160
#define VDC_SCANLINES			256
#define VDC_TILESET_0_ADDRESS	0x0800
#define VDC_TILESET_1_ADDRESS	0x1000
#define VDC_LAYERS_ADDRESS		0x2000

// ---------------------------------------------------------------------
// system / host
// ---------------------------------------------------------------------
#define	FPS						60
#define SCREEN_WIDTH			(2*VDC_XRES)
#define SCREEN_HEIGHT			(2*VDC_YRES)

// ---------------------------------------------------------------------
// cpu
// ---------------------------------------------------------------------
#define CPU_CYCLES_PER_PIXEL	1
#define CPU_CYCLES_PER_SCANLINE	(CPU_CYCLES_PER_PIXEL*VDC_XRES)
#define CPU_CYCLES_PER_FRAME	(CPU_CYCLES_PER_SCANLINE*VDC_SCANLINES)
#define CPU_CLOCK_SPEED			(CPU_CYCLES_PER_FRAME*FPS)

// ---------------------------------------------------------------------
// audio
// ---------------------------------------------------------------------
#define SAMPLE_RATE				48000
#define AUDIO_BUFFER_SIZE		8000.0
#define SID_CLOCK_SPEED			985248
#define SID_CYCLES_PER_FRAME	(SID_CLOCK_SPEED/FPS)

// ---------------------------------------------------------------------
// color palette
// ---------------------------------------------------------------------
#define	LIME_COLOR_00	0xff'08'18'20	// gameboy green palette:
#define LIME_COLOR_01	0xff'34'68'56	// https://www.color-hex.com/color-palette/1041681
#define LIME_COLOR_02	0xff'88'c0'70	// https://lospec.com/palette-list/nintendo-gameboy-bgb
#define LIME_COLOR_03	0xff'e0'f8'd0
#define LIME_COLOR_04	0xff'25'03'03	// reds inspired on HSL + gameboy
#define LIME_COLOR_05	0xff'90'0c'0c
#define LIME_COLOR_06	0xff'ef'41'41
#define LIME_COLOR_07	0xff'fb'cd'cd
#define LIME_COLOR_08	0xff'03'0e'25	// blue inspired on HSL + gameboy
#define LIME_COLOR_09	0xff'0c'37'90
#define LIME_COLOR_0A	0xff'41'79'ef
#define LIME_COLOR_0B	0xff'cd'dc'fb
#define LIME_COLOR_0C	0xff'14'14'14	// grey scale
#define LIME_COLOR_0D	0xff'4f'4f'4f
#define LIME_COLOR_0E	0xff'99'99'99
#define LIME_COLOR_0F	0xff'e3'e3'e3

#define LIME_COLOR_10	0xff'1f'24'4b	// PAPER 8 by Frosty Rabbid 🐰
#define LIME_COLOR_11	0xff'65'40'53	// https://lospec.com/palette-list/paper-8
#define LIME_COLOR_12	0xff'a8'60'5d
#define LIME_COLOR_13	0xff'd1'a6'7e
#define LIME_COLOR_14	0xff'f6'e7'9c
#define LIME_COLOR_15	0xff'b6'cf'8e
#define LIME_COLOR_16	0xff'60'ae'7b
#define LIME_COLOR_17	0xff'3c'6b'64

#define LIME_COLOR_18	0xff'93'3a'63	// FLUFFY8 PALETTE by vee
#define LIME_COLOR_19	0xff'ff'95'b1	// https://lospec.com/palette-list/fluffy8
#define LIME_COLOR_1A	0xff'ff'c4'cf
#define LIME_COLOR_1B	0xff'ff'ec'f0
#define LIME_COLOR_1C	0xff'd4'd5'f6
#define LIME_COLOR_1D	0xff'96'b3'fd
#define LIME_COLOR_1E	0xff'76'79'db
#define LIME_COLOR_1F	0xff'44'38'7a

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
// Punch colors (resembling c64)
// ---------------------------------------------------------------------
#define PUNCH_BLACK       0xff'00'00'00
#define PUNCH_WHITE       0xff'ff'ff'ff
#define PUNCH_RED         0xff'cc'00'44
#define PUNCH_CYAN        0xff'44'cc'cc
#define PUNCH_PURPLE      0xff'88'00'cc
#define PUNCH_GREEN       0xff'44'cc'00
#define PUNCH_BLUE        0xff'22'22'88
#define PUNCH_YELLOW      0xff'ff'ff'55
#define PUNCH_ORANGE      0xff'cc'88'00
#define PUNCH_BROWN       0xff'55'22'00
#define PUNCH_LIGHTRED    0xff'cc'88'88
#define PUNCH_DARKGREY    0xff'55'55'55
#define PUNCH_GREY        0xff'aa'aa'aa
#define PUNCH_LIGHTGREEN  0xff'aa'ff'aa
#define PUNCH_LIGHTBLUE   0xff'88'88'cc
#define PUNCH_LIGHTGREY   0xff'cc'cc'cc

// 01 0xff'00'00'00
// 02 0xff'00'00'00
// 03 0xff'00'00'00
// 04 0xff'00'00'11
// 05 0xff'00'00'22
// 06 0xff'00'00'44
// 07 0xff'00'00'55
// 08 0xff'00'00'33
// 09 0xff'00'00'55
// 0a 0xff'00'00'88
// 0b 0xff'00'00'aa
// 0c 0xff'00'00'55
// 0d 0xff'00'00'88
// 0e 0xff'00'00'cc
// 0f 0xff'00'00'ff
// 10 0xff'00'11'00
// 11 0xff'00'22'00
// 12 0xff'00'44'00
// 13 0xff'00'55'00
// 14 0xff'00'11'11
// 15 0xff'00'22'22
// 16 0xff'00'44'44
// 17 0xff'00'55'55
// 18 0xff'00'11'33
// 19 0xff'00'22'55
// 1a 0xff'00'44'88
// 1b 0xff'00'55'aa
// 1c 0xff'00'11'55
// 1d 0xff'00'22'88
// 1e 0xff'00'44'cc
// 1f 0xff'00'55'ff
// 20 0xff'00'33'00
// 21 0xff'00'55'00
// 22 0xff'00'88'00
// 23 0xff'00'aa'00
// 24 0xff'00'33'11
// 25 0xff'00'55'22
// 26 0xff'00'88'44
// 27 0xff'00'aa'55
// 28 0xff'00'33'33
// 29 0xff'00'55'55
// 2a 0xff'00'88'88
// 2b 0xff'00'aa'aa
// 2c 0xff'00'33'55
// 2d 0xff'00'55'88
// 2e 0xff'00'88'cc
// 2f 0xff'00'aa'ff
// 30 0xff'00'55'00
// 31 0xff'00'88'00
// 32 0xff'00'cc'00
// 33 0xff'00'ff'00
// 34 0xff'00'55'11
// 35 0xff'00'88'22
// 36 0xff'00'cc'44
// 37 0xff'00'ff'55
// 38 0xff'00'55'33
// 39 0xff'00'88'55
// 3a 0xff'00'cc'88
// 3b 0xff'00'ff'aa
// 3c 0xff'00'55'55
// 3d 0xff'00'88'88
// 3e 0xff'00'cc'cc
// 3f 0xff'00'ff'ff
// 40 0xff'11'00'00
// 41 0xff'22'00'00
// 42 0xff'44'00'00
// 43 0xff'55'00'00
// 44 0xff'11'00'11
// 45 0xff'22'00'22
// 46 0xff'44'00'44
// 47 0xff'55'00'55
// 48 0xff'11'00'33
// 49 0xff'22'00'55
// 4a 0xff'44'00'88
// 4b 0xff'55'00'aa
// 4c 0xff'11'00'55
// 4d 0xff'22'00'88
// 4e 0xff'44'00'cc
// 4f 0xff'55'00'ff
// 50 0xff'11'11'00
// 51 0xff'22'22'00
// 52 0xff'44'44'00
// 53 0xff'55'55'00
// 54 0xff'11'11'11
// 55 0xff'22'22'22
// 56 0xff'44'44'44
// 57 0xff'55'55'55
// 58 0xff'11'11'33
// 59 0xff'22'22'55
// 5a 0xff'44'44'88
// 5b 0xff'55'55'aa
// 5c 0xff'11'11'55
// 5e 0xff'44'44'cc
// 5f 0xff'55'55'ff
// 60 0xff'11'33'00
// 61 0xff'22'55'00
// 62 0xff'44'88'00
// 63 0xff'55'aa'00
// 64 0xff'11'33'11
// 65 0xff'22'55'22
// 66 0xff'44'88'44
// 67 0xff'55'aa'55
// 68 0xff'11'33'33
// 69 0xff'22'55'55
// 6a 0xff'44'88'88
// 6b 0xff'55'aa'aa
// 6c 0xff'11'33'55
// 6d 0xff'22'55'88
// 6e 0xff'44'88'cc
// 6f 0xff'55'aa'ff
// 70 0xff'11'55'00
// 71 0xff'22'88'00
// 73 0xff'55'ff'00
// 74 0xff'11'55'11
// 75 0xff'22'88'22
// 76 0xff'44'cc'44
// 77 0xff'55'ff'55
// 78 0xff'11'55'33
// 79 0xff'22'88'55
// 7a 0xff'44'cc'88
// 7b 0xff'55'ff'aa
// 7c 0xff'11'55'55
// 7d 0xff'22'88'88
// 7f 0xff'55'ff'ff
// 80 0xff'33'00'00
// 81 0xff'55'00'00
// 82 0xff'88'00'00
// 83 0xff'aa'00'00
// 84 0xff'33'00'11
// 85 0xff'55'00'22
// 86 0xff'88'00'44
// 87 0xff'aa'00'55
// 88 0xff'33'00'33
// 89 0xff'55'00'55
// 8a 0xff'88'00'88
// 8b 0xff'aa'00'aa
// 8c 0xff'33'00'55
// 8d 0xff'55'00'88
// 8f 0xff'aa'00'ff
// 90 0xff'33'11'00
// 92 0xff'88'44'00
// 93 0xff'aa'55'00
// 94 0xff'33'11'11
// 95 0xff'55'22'22
// 96 0xff'88'44'44
// 97 0xff'aa'55'55
// 98 0xff'33'11'33
// 99 0xff'55'22'55
// 9a 0xff'88'44'88
// 9b 0xff'aa'55'aa
// 9c 0xff'33'11'55
// 9d 0xff'55'22'88
// 9e 0xff'88'44'cc
// 9f 0xff'aa'55'ff
// a0 0xff'33'33'00
// a1 0xff'55'55'00
// a2 0xff'88'88'00
// a3 0xff'aa'aa'00
// a4 0xff'33'33'11
// a5 0xff'55'55'22
// a6 0xff'88'88'44
// a7 0xff'aa'aa'55
// a8 0xff'33'33'33
// aa 0xff'88'88'88
// ac 0xff'33'33'55
// ad 0xff'55'55'88
// af 0xff'aa'aa'ff
// b0 0xff'33'55'00
// b1 0xff'55'88'00
// b2 0xff'88'cc'00
// b3 0xff'aa'ff'00
// b4 0xff'33'55'11
// b5 0xff'55'88'22
// b6 0xff'88'cc'44
// b7 0xff'aa'ff'55
// b8 0xff'33'55'33
// b9 0xff'55'88'55
// ba 0xff'88'cc'88
// bc 0xff'33'55'55
// bd 0xff'55'88'88
// be 0xff'88'cc'cc
// bf 0xff'aa'ff'ff
// c0 0xff'55'00'00
// c1 0xff'88'00'00
// c2 0xff'cc'00'00
// c3 0xff'ff'00'00
// c4 0xff'55'00'11
// c5 0xff'88'00'22
// c7 0xff'ff'00'55
// c8 0xff'55'00'33
// c9 0xff'88'00'55
// ca 0xff'cc'00'88
// cb 0xff'ff'00'aa
// cc 0xff'55'00'55
// cd 0xff'88'00'88
// ce 0xff'cc'00'cc
// cf 0xff'ff'00'ff
// d0 0xff'55'11'00
// d1 0xff'88'22'00
// d2 0xff'cc'44'00
// d3 0xff'ff'55'00
// d4 0xff'55'11'11
// d5 0xff'88'22'22
// d6 0xff'cc'44'44
// d7 0xff'ff'55'55
// d8 0xff'55'11'33
// d9 0xff'88'22'55
// da 0xff'cc'44'88
// db 0xff'ff'55'aa
// dc 0xff'55'11'55
// dd 0xff'88'22'88
// de 0xff'cc'44'cc
// df 0xff'ff'55'ff
// e0 0xff'55'33'00
// e1 0xff'88'55'00
// e3 0xff'ff'aa'00
// e4 0xff'55'33'11
// e5 0xff'88'55'22
// e6 0xff'cc'88'44
// e7 0xff'ff'aa'55
// e8 0xff'55'33'33
// e9 0xff'88'55'55
// eb 0xff'ff'aa'aa
// ec 0xff'55'33'55
// ed 0xff'88'55'88
// ee 0xff'cc'88'cc
// ef 0xff'ff'aa'ff
// f0 0xff'55'55'00
// f1 0xff'88'88'00
// f2 0xff'cc'cc'00
// f3 0xff'ff'ff'00
// f4 0xff'55'55'11
// f5 0xff'88'88'22
// f6 0xff'cc'cc'44
// f8 0xff'55'55'33
// f9 0xff'88'88'55
// fa 0xff'cc'cc'88
// fb 0xff'ff'ff'aa
// fc 0xff'55'55'55
// fd 0xff'88'88'88

// ---------------------------------------------------------------------
// Picotron Palette WIP v 5 (only black is changed)
// ---------------------------------------------------------------------
#define PICOTRON_V5_00	0xff141414
#define PICOTRON_V5_01	0xff742f29
#define PICOTRON_V5_02	0xffab5236
#define PICOTRON_V5_03	0xffff856d
#define PICOTRON_V5_04	0xffffccaa
#define PICOTRON_V5_05	0xffff004d
#define PICOTRON_V5_06	0xffc3004c
#define PICOTRON_V5_07	0xff7e2553
#define PICOTRON_V5_08	0xff492d38
#define PICOTRON_V5_09	0xff5f574f
#define PICOTRON_V5_0A	0xffa28879
#define PICOTRON_V5_0B	0xffc2c3c7
#define PICOTRON_V5_0C	0xfffff1e8
#define PICOTRON_V5_0D	0xffffacc5
#define PICOTRON_V5_0E	0xffff77a8
#define PICOTRON_V5_0F	0xffe40dab
#define PICOTRON_V5_10	0xff1d2b53
#define PICOTRON_V5_11	0xff1c5eac
#define PICOTRON_V5_12	0xff00a5a1
#define PICOTRON_V5_13	0xff29adff
#define PICOTRON_V5_14	0xff64dff6
#define PICOTRON_V5_15	0xffbd9adf
#define PICOTRON_V5_16	0xff83769c
#define PICOTRON_V5_17	0xff754e97
#define PICOTRON_V5_18	0xff125359
#define PICOTRON_V5_19	0xff008751
#define PICOTRON_V5_1A	0xff00b251
#define PICOTRON_V5_1B	0xff00e436
#define PICOTRON_V5_1C	0xff90ec42
#define PICOTRON_V5_1D	0xffffec27
#define PICOTRON_V5_1E	0xffffa300
#define PICOTRON_V5_1F	0xffeb6b00

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
