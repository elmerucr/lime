// ---------------------------------------------------------------------
// font_4x8.hpp
// punch
//
// Copyright © 2024 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

// ---------------------------------------------------------------------
// Inspired by:
// https://hackaday.io/project/6309-vga-graphics-over-spi-and-serial-vgatonic/log/20759-a-tiny-4x6-pixel-font-that-will-fit-on-almost-any-microcontroller-license-mit
// ---------------------------------------------------------------------

#include <cstdint>

#ifndef FONT_4X8_HPP
#define FONT_4X8_HPP

class font_4x8_t {
public:
	uint8_t data[2048]; // slightly larger then 768 bytes to make masking possible

	font_4x8_t() {
		for (int i=0; i<256; i++) {
			data[i*8 + 0] = (i & 0x80) ? 0xff : 0x00;
			data[i*8 + 1] = tiny_font_raw[i*6 + 0];
			data[i*8 + 2] = tiny_font_raw[i*6 + 1];
			data[i*8 + 3] = tiny_font_raw[i*6 + 2];
			data[i*8 + 4] = tiny_font_raw[i*6 + 3];
			data[i*8 + 5] = tiny_font_raw[i*6 + 4];
			data[i*8 + 6] = tiny_font_raw[i*6 + 5];
			data[i*8 + 7] = (i & 0x80) ? 0xff : 0x00;
		}
	}

private:
	const uint8_t tiny_font_raw[6 * 256] = {
		0b0000,	// $00
		0b0000,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b1100,	// $01
		0b1100,
		0b1100,
		0b0000,
		0b0000,
		0b0000,

		0b0011,	// $02
		0b0011,
		0b0011,
		0b0000,
		0b0000,
		0b0000,

		0b1111,	// $03
		0b1111,
		0b1111,
		0b0000,
		0b0000,
		0b0000,

		0b0000,	// $04
		0b0000,
		0b0000,
		0b1100,
		0b1100,
		0b1100,

		0b1100,	// $05
		0b1100,
		0b1100,
		0b1100,
		0b1100,
		0b1100,

		0b0011,	// $06
		0b0011,
		0b0011,
		0b1100,
		0b1100,
		0b1100,

		0b1111,	// $07
		0b1111,
		0b1111,
		0b1100,
		0b1100,
		0b1100,

		0b0000,	// $08
		0b0000,
		0b0000,
		0b0011,
		0b0011,
		0b0011,

		0b1100,	// $09
		0b1100,
		0b1100,
		0b0011,
		0b0011,
		0b0011,

		0b0011,	// $0a
		0b0011,
		0b0011,
		0b0011,
		0b0011,
		0b0011,

		0b1111,	// $0b
		0b1111,
		0b1111,
		0b0011,
		0b0011,
		0b0011,

		0b0000,	// $0c
		0b0000,
		0b0000,
		0b1111,
		0b1111,
		0b1111,

		0b1100,	// $0d
		0b1100,
		0b1100,
		0b1111,
		0b1111,
		0b1111,

		0b0011,	// $0e
		0b0011,
		0b0011,
		0b1111,
		0b1111,
		0b1111,

		0b1111,	// $0f
		0b1111,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b0110,	// $10
		0b0110,
		0b1110,
		0b1110,
		0b0000,
		0b0000,

		0b0110,	// $11
		0b0110,
		0b0111,
		0b0111,
		0b0000,
		0b0000,

		0b0000,	// $12
		0b0000,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b0000,	// $13
		0b0000,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b0000,	// $14
		0b0000,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b0000,	// $15
		0b0000,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b0000,	// $16
		0b0000,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b0000,	// $17
		0b0000,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b0000,	// $18
		0b0000,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b0000,	// $19
		0b0000,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b0000,	// $1a
		0b0000,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b0000,	// $1b
		0b0000,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b0000,	// $1c
		0b0000,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b0000,	// $1d
		0b0000,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b0000,	// $1e
		0b0000,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b0000,	// $1f
		0b0000,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b0000,	// $20
		0b0000,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b0100,	// $21
		0b0100,
		0b0100,
		0b0000,
		0b0100,
		0b0000,

		0b1010,	// $22
		0b1010,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b1010,	// $23
		0b1110,
		0b1010,
		0b1110,
		0b1010,
		0b0000,

		0b0110,	// $24
		0b1100,
		0b1110,
		0b0110,
		0b1100,
		0b0000,

		0b1010,	// $25
		0b0010,
		0b0100,
		0b1000,
		0b1010,
		0b0000,

		0b0100,	// $26
		0b1010,
		0b0100,
		0b1010,
		0b1100,
		0b0000,

		0b0100,	// $27
		0b0100,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b0010,	// $28
		0b0100,
		0b0100,
		0b0100,
		0b0010,
		0b0000,

		0b0100,	// $29
		0b0010,
		0b0010,
		0b0010,
		0b0100,
		0b0000,

		0b0000,	// $2a
		0b1010,
		0b0100,
		0b1010,
		0b0000,
		0b0000,

		0b0000,	// $2b
		0b0100,
		0b1110,
		0b0100,
		0b0000,
		0b0000,

		0b0000,	// $2c
		0b0000,
		0b0000,
		0b0000,
		0b0100,
		0b1000,

		0b0000,	// $2d
		0b0000,
		0b1110,
		0b0000,
		0b0000,
		0b0000,

		0b0000,	// $2e
		0b0000,
		0b0000,
		0b0000,
		0b0100,
		0b0000,

		0b0010,	// $2f
		0b0010,
		0b0100,
		0b1000,
		0b1000,
		0b0000,

		0b0110,	// $30
		0b1010,
		0b1010,
		0b1010,
		0b1100,
		0b0000,

		0b0100,	// $31
		0b1100,
		0b0100,
		0b0100,
		0b1110,
		0b0000,

		0b1100,	// $32
		0b0010,
		0b0110,
		0b1000,
		0b1110,
		0b0000,

		0b1100,	// $33
		0b0010,
		0b0100,
		0b0010,
		0b1100,
		0b0000,

		0b1000,	// $34
		0b1000,
		0b1010,
		0b1110,
		0b0010,
		0b0000,

		0b1110,	// $35
		0b1000,
		0b1110,
		0b0010,
		0b1100,
		0b0000,

		0b0110,	// $36
		0b1000,
		0b1110,
		0b1010,
		0b1100,
		0b0000,

		0b1110,	// $37
		0b0010,
		0b0100,
		0b1000,
		0b1000,
		0b0000,

		0b0110,	// $38
		0b1010,
		0b1110,
		0b1010,
		0b1100,
		0b0000,

		0b0110,	// $39
		0b1010,
		0b1110,
		0b0010,
		0b1100,
		0b0000,

		0b0000,	// $3a
		0b0100,
		0b0000,
		0b0100,
		0b0000,
		0b0000,

		0b0000,	// $3b
		0b0100,
		0b0000,
		0b0100,
		0b1000,
		0b0000,

		0b0010,	// $3c
		0b0100,
		0b1000,
		0b0100,
		0b0010,
		0b0000,

		0b0000,	// $3d
		0b1110,
		0b0000,
		0b1110,
		0b0000,
		0b0000,

		0b1000,	// $3e
		0b0100,
		0b0010,
		0b0100,
		0b1000,
		0b0000,

		0b1110,	// $3f
		0b0010,
		0b0100,
		0b0000,
		0b0100,
		0b0000,

		0b0100,	// $40
		0b1010,
		0b1010,
		0b1000,
		0b0110,
		0b0000,

		0b0110,	// $41
		0b1010,
		0b1110,
		0b1010,
		0b1010,
		0b0000,

		0b0110,	// $42
		0b1010,
		0b1100,
		0b1010,
		0b1100,
		0b0000,

		0b0110,	// $43
		0b1000,
		0b1000,
		0b1000,
		0b0110,
		0b0000,

		0b1100,	// $44
		0b1010,
		0b1010,
		0b1010,
		0b1100,
		0b0000,

		0b0110,	// $45
		0b1000,
		0b1110,
		0b1000,
		0b1110,
		0b0000,

		0b0110,	// $46
		0b1000,
		0b1110,
		0b1000,
		0b1000,
		0b0000,

		0b0110,	// $47
		0b1000,
		0b1010,
		0b1010,
		0b0110,
		0b0000,

		0b1010,	// $48
		0b1010,
		0b1110,
		0b1010,
		0b1010,
		0b0000,

		0b1110,	// $49
		0b0100,
		0b0100,
		0b0100,
		0b1110,
		0b0000,

		0b0110,	// $4a
		0b0010,
		0b0010,
		0b1010,
		0b0100,
		0b0000,

		0b1010,	// $4b
		0b1010,
		0b1100,
		0b1010,
		0b1010,
		0b0000,

		0b1000,	// $4c
		0b1000,
		0b1000,
		0b1000,
		0b1110,
		0b0000,

		0b1010,	// $4d
		0b1110,
		0b1010,
		0b1010,
		0b1010,
		0b0000,

		0b1100,	// $4e
		0b1010,
		0b1010,
		0b1010,
		0b1010,
		0b0000,

		0b0100,	// $4f
		0b1010,
		0b1010,
		0b1010,
		0b0100,
		0b0000,

		0b1100,	// $50
		0b1010,
		0b1110,
		0b1000,
		0b1000,
		0b0000,

		0b0110,	// $51
		0b1010,
		0b1010,
		0b1110,
		0b0110,
		0b0000,

		0b0110,	// $52
		0b1010,
		0b1100,
		0b1010,
		0b1010,
		0b0000,

		0b0110,	// $53
		0b1000,
		0b0100,
		0b0010,
		0b1100,
		0b0000,

		0b1110,	// $54
		0b0100,
		0b0100,
		0b0100,
		0b0100,
		0b0000,

		0b1010,	// $55
		0b1010,
		0b1010,
		0b1010,
		0b0110,
		0b0000,

		0b1010,	// $56
		0b1010,
		0b1010,
		0b1010,
		0b0100,
		0b0000,

		0b1010,	// $57
		0b1010,
		0b1010,
		0b1110,
		0b1010,
		0b0000,

		0b1010,	// $58
		0b1010,
		0b0100,
		0b1010,
		0b1010,
		0b0000,

		0b1010,	// $59
		0b1010,
		0b0100,
		0b0100,
		0b0100,
		0b0000,

		0b1110,	// $5a
		0b0010,
		0b0100,
		0b1000,
		0b1110,
		0b0000,

		0b0110,	// $5b
		0b0100,
		0b0100,
		0b0100,
		0b0110,
		0b0000,

		0b1000,	// $5c
		0b1000,
		0b0100,
		0b0010,
		0b0010,
		0b0000,

		0b0110,	// $5d
		0b0010,
		0b0010,
		0b0010,
		0b0110,
		0b0000,

		0b0100,	// $5e
		0b1010,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b0000,	// $5f
		0b0000,
		0b0000,
		0b0000,
		0b1110,
		0b0000,

		0b0100,	// $60
		0b0010,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b0000,	// $61
		0b0110,
		0b1010,
		0b1010,
		0b0110,
		0b0000,

		0b1000,	// $62
		0b1100,
		0b1010,
		0b1010,
		0b1100,
		0b0000,

		0b0000,	// $63
		0b0110,
		0b1000,
		0b1000,
		0b0110,
		0b0000,

		0b0010,	// $64
		0b0110,
		0b1010,
		0b1010,
		0b0110,
		0b0000,

		0b0000,	// $65
		0b0110,
		0b1010,
		0b1100,
		0b0110,
		0b0000,

		0b0100,	// $66
		0b1010,
		0b1000,
		0b1100,
		0b1000,
		0b0000,

		0b0000,	// $67
		0b0110,
		0b1010,
		0b0110,
		0b0010,
		0b1100,

		0b1000,	// $68
		0b1100,
		0b1010,
		0b1010,
		0b1010,
		0b0000,

		0b0100,	// $69
		0b0000,
		0b0100,
		0b0100,
		0b0100,
		0b0000,

		0b0100,	// $6a
		0b0000,
		0b0100,
		0b0100,
		0b0100,
		0b1000,

		0b1000,	// $6b
		0b1010,
		0b1100,
		0b1010,
		0b1010,
		0b0000,

		0b0100,	// $6c
		0b0100,
		0b0100,
		0b0100,
		0b0010,
		0b0000,

		0b0000,	// $6d
		0b1010,
		0b1110,
		0b1010,
		0b1010,
		0b0000,

		0b0000,	// $6e
		0b1100,
		0b1010,
		0b1010,
		0b1010,
		0b0000,

		0b0000,	// $6f
		0b0100,
		0b1010,
		0b1010,
		0b0100,
		0b0000,

		0b0000,	// $70
		0b1100,
		0b1010,
		0b1010,
		0b1100,
		0b1000,

		0b0000,	// $71
		0b0110,
		0b1010,
		0b1010,
		0b0110,
		0b0010,

		0b0000,	// $72
		0b1010,
		0b1100,
		0b1000,
		0b1000,
		0b0000,

		0b0000,	// $73
		0b0110,
		0b1100,
		0b0010,
		0b1100,
		0b0000,

		0b1000,	// $74
		0b1100,
		0b1000,
		0b1000,
		0b0110,
		0b0000,

		0b0000,	// $75
		0b1010,
		0b1010,
		0b1010,
		0b0110,
		0b0000,

		0b0000,	// $76
		0b1010,
		0b1010,
		0b1010,
		0b1100,
		0b0000,

		0b0000,	// $77
		0b1010,
		0b1010,
		0b1110,
		0b1010,
		0b0000,

		0b0000,	// $78
		0b1010,
		0b0100,
		0b1010,
		0b1010,
		0b0000,

		0b0000,	// $79
		0b1010,
		0b1010,
		0b0110,
		0b0010,
		0b0100,

		0b0000,	// $7a
		0b1110,
		0b0010,
		0b0100,
		0b1110,
		0b0000,

		0b0110,	// $7b
		0b0100,
		0b1100,
		0b0100,
		0b0110,
		0b0000,

		0b0100,	// $7c
		0b0100,
		0b0100,
		0b0100,
		0b0100,
		0b0000,

		0b1100,	// $7d
		0b0100,
		0b0110,
		0b0100,
		0b1100,
		0b0000,

		0b0100,	// $7e
		0b1010,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b0100,	// $7f
		0b1010,
		0b1010,
		0b1110,
		0b0000,
		0b0000,

		0b1111,	// $80
		0b1111,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b0011,	// $81
		0b0011,
		0b0011,
		0b1111,
		0b1111,
		0b1111,

		0b1100,	// $82
		0b1100,
		0b1100,
		0b1111,
		0b1111,
		0b1111,

		0b0000,	// $83
		0b0000,
		0b0000,
		0b1111,
		0b1111,
		0b1111,

		0b1111,	// $84
		0b1111,
		0b1111,
		0b0011,
		0b0011,
		0b0011,

		0b0011,	// $85
		0b0011,
		0b0011,
		0b0011,
		0b0011,
		0b0011,

		0b1100,	// $86
		0b1100,
		0b1100,
		0b0011,
		0b0011,
		0b0011,

		0b0000,	// $87
		0b0000,
		0b0000,
		0b0011,
		0b0011,
		0b0011,

		0b1111,	// $88
		0b1111,
		0b1111,
		0b1100,
		0b1100,
		0b1100,

		0b0011,	// $89
		0b0011,
		0b0011,
		0b1100,
		0b1100,
		0b1100,

		0b1100,	// $8a
		0b1100,
		0b1100,
		0b1100,
		0b1100,
		0b1100,

		0b0000,	// $8b
		0b0000,
		0b0000,
		0b1100,
		0b1100,
		0b1100,

		0b1111,	// $8c
		0b1111,
		0b1111,
		0b0000,
		0b0000,
		0b0000,

		0b0011,	// $8d
		0b0011,
		0b0011,
		0b0000,
		0b0000,
		0b0000,

		0b1100,	// $8e
		0b1100,
		0b1100,
		0b0000,
		0b0000,
		0b0000,

		0b0000,	// $8f
		0b0000,
		0b0000,
		0b0000,
		0b0000,
		0b0000,

		0b1001,	// $90
		0b1001,
		0b0001,
		0b0001,
		0b1111,
		0b1111,

		0b1001,	// $91
		0b1001,
		0b1000,
		0b1000,
		0b1111,
		0b1111,

		0b1111,	// $92
		0b1111,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b1111,	// $93
		0b1111,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b1111,	// $94
		0b1111,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b1111,	// $95
		0b1111,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b1111,	// $96
		0b1111,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b1111,	// $97
		0b1111,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b1111,	// $98
		0b1111,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b1111,	// $99
		0b1111,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b1111,	// $9a
		0b1111,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b1111,	// $9b
		0b1111,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b1111,	// $9c
		0b1111,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b1111,	// $9d
		0b1111,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b1111,	// $9e
		0b1111,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b1111,	// $9f
		0b1111,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b1111,	// $a0
		0b1111,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b1011,	// $a1
		0b1011,
		0b1011,
		0b1111,
		0b1011,
		0b1111,

		0b0101,	// $a2
		0b0101,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b0101,	// $a3
		0b0001,
		0b0101,
		0b0001,
		0b0101,
		0b1111,

		0b1001,	// $a4
		0b0011,
		0b0001,
		0b1001,
		0b0011,
		0b1111,

		0b0101,	// $a5
		0b1101,
		0b1011,
		0b0111,
		0b0101,
		0b1111,

		0b1011,	// $a6
		0b0101,
		0b1011,
		0b0101,
		0b0011,
		0b1111,

		0b1011,	// $a7
		0b1011,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b1101,	// $a8
		0b1011,
		0b1011,
		0b1011,
		0b1101,
		0b1111,

		0b1011,	// $a9
		0b1101,
		0b1101,
		0b1101,
		0b1011,
		0b1111,

		0b1111,	// $aa
		0b0101,
		0b1011,
		0b0101,
		0b1111,
		0b1111,

		0b1111,	// $ab
		0b1011,
		0b0001,
		0b1011,
		0b1111,
		0b1111,

		0b1111,	// $ac
		0b1111,
		0b1111,
		0b1111,
		0b1011,
		0b0111,

		0b1111,	// $ad
		0b1111,
		0b0001,
		0b1111,
		0b1111,
		0b1111,

		0b1111,	// $ae
		0b1111,
		0b1111,
		0b1111,
		0b1011,
		0b1111,

		0b1101,	// $af
		0b1101,
		0b1011,
		0b0111,
		0b0111,
		0b1111,

		0b1001,	// $b0
		0b0101,
		0b0101,
		0b0101,
		0b0011,
		0b1111,

		0b1011,	// $b1
		0b0011,
		0b1011,
		0b1011,
		0b0001,
		0b1111,

		0b0011,	// $b2
		0b1101,
		0b1001,
		0b0111,
		0b0001,
		0b1111,

		0b0011,	// $b3
		0b1101,
		0b1011,
		0b1101,
		0b0011,
		0b1111,

		0b0111,	// $b4
		0b0111,
		0b0101,
		0b0001,
		0b1101,
		0b1111,

		0b0001,	// $b5
		0b0111,
		0b0001,
		0b1101,
		0b0011,
		0b1111,

		0b1001,	// $b6
		0b0111,
		0b0001,
		0b0101,
		0b0011,
		0b1111,

		0b0001,	// $b7
		0b1101,
		0b1011,
		0b0111,
		0b0111,
		0b1111,

		0b1001,	// $b8
		0b0101,
		0b0001,
		0b0101,
		0b0011,
		0b1111,

		0b1001,	// $b9
		0b0101,
		0b0001,
		0b1101,
		0b0011,
		0b1111,

		0b1111,	// $ba
		0b1011,
		0b1111,
		0b1011,
		0b1111,
		0b1111,

		0b1111,	// $bb
		0b1011,
		0b1111,
		0b1011,
		0b0111,
		0b1111,

		0b1101,	// $bc
		0b1011,
		0b0111,
		0b1011,
		0b1101,
		0b1111,

		0b1111,	// $bd
		0b0001,
		0b1111,
		0b0001,
		0b1111,
		0b1111,

		0b0111,	// $be
		0b1011,
		0b1101,
		0b1011,
		0b0111,
		0b1111,

		0b0001,	// $bf
		0b1101,
		0b1011,
		0b1111,
		0b1011,
		0b1111,

		0b1011,	// $c0
		0b0101,
		0b0101,
		0b0111,
		0b1001,
		0b1111,

		0b1001,	// $c1
		0b0101,
		0b0001,
		0b0101,
		0b0101,
		0b1111,

		0b1001,	// $c2
		0b0101,
		0b0011,
		0b0101,
		0b0011,
		0b1111,

		0b1001,	// $c3
		0b0111,
		0b0111,
		0b0111,
		0b1001,
		0b1111,

		0b0011,	// $c4
		0b0101,
		0b0101,
		0b0101,
		0b0011,
		0b1111,

		0b1001,	// $c5
		0b0111,
		0b0001,
		0b0111,
		0b0001,
		0b1111,

		0b1001,	// $c6
		0b0111,
		0b0001,
		0b0111,
		0b0111,
		0b1111,

		0b1001,	// $c7
		0b0111,
		0b0101,
		0b0101,
		0b1001,
		0b1111,

		0b0101,	// $c8
		0b0101,
		0b0001,
		0b0101,
		0b0101,
		0b1111,

		0b0001,	// $c9
		0b1011,
		0b1011,
		0b1011,
		0b0001,
		0b1111,

		0b1001,	// $ca
		0b1101,
		0b1101,
		0b0101,
		0b1011,
		0b1111,

		0b0101,	// $cb
		0b0101,
		0b0011,
		0b0101,
		0b0101,
		0b1111,

		0b0111,	// $cc
		0b0111,
		0b0111,
		0b0111,
		0b0001,
		0b1111,

		0b0101,	// $cd
		0b0001,
		0b0101,
		0b0101,
		0b0101,
		0b1111,

		0b0011,	// $ce
		0b0101,
		0b0101,
		0b0101,
		0b0101,
		0b1111,

		0b1011,	// $cf
		0b0101,
		0b0101,
		0b0101,
		0b1011,
		0b1111,

		0b0011,	// $d0
		0b0101,
		0b0001,
		0b0111,
		0b0111,
		0b1111,

		0b1001,	// $d1
		0b0101,
		0b0101,
		0b0001,
		0b1001,
		0b1111,

		0b1001,	// $d2
		0b0101,
		0b0011,
		0b0101,
		0b0101,
		0b1111,

		0b1001,	// $d3
		0b0111,
		0b1011,
		0b1101,
		0b0011,
		0b1111,

		0b0001,	// $d4
		0b1011,
		0b1011,
		0b1011,
		0b1011,
		0b1111,

		0b0101,	// $d5
		0b0101,
		0b0101,
		0b0101,
		0b1001,
		0b1111,

		0b0101,	// $d6
		0b0101,
		0b0101,
		0b0101,
		0b1011,
		0b1111,

		0b0101,	// $d7
		0b0101,
		0b0101,
		0b0001,
		0b0101,
		0b1111,

		0b0101,	// $d8
		0b0101,
		0b1011,
		0b0101,
		0b0101,
		0b1111,

		0b0101,	// $d9
		0b0101,
		0b1011,
		0b1011,
		0b1011,
		0b1111,

		0b0001,	// $da
		0b1101,
		0b1011,
		0b0111,
		0b0001,
		0b1111,

		0b1001,	// $db
		0b1011,
		0b1011,
		0b1011,
		0b1001,
		0b1111,

		0b0111,	// $dc
		0b0111,
		0b1011,
		0b1101,
		0b1101,
		0b1111,

		0b1001,	// $dd
		0b1101,
		0b1101,
		0b1101,
		0b1001,
		0b1111,

		0b1011,	// $de
		0b0101,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b1111,	// $df
		0b1111,
		0b1111,
		0b1111,
		0b0001,
		0b1111,

		0b1011,	// $e0
		0b1101,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b1111,	// $e1
		0b1001,
		0b0101,
		0b0101,
		0b1001,
		0b1111,

		0b0111,	// $e2
		0b0011,
		0b0101,
		0b0101,
		0b0011,
		0b1111,

		0b1111,	// $e3
		0b1001,
		0b0111,
		0b0111,
		0b1001,
		0b1111,

		0b1101,	// $e4
		0b1001,
		0b0101,
		0b0101,
		0b1001,
		0b1111,

		0b1111,	// $e5
		0b1001,
		0b0101,
		0b0011,
		0b1001,
		0b1111,

		0b1011,	// $e6
		0b0101,
		0b0111,
		0b0011,
		0b0111,
		0b1111,

		0b1111,	// $e7
		0b1001,
		0b0101,
		0b1001,
		0b1101,
		0b0011,

		0b0111,	// $e8
		0b0011,
		0b0101,
		0b0101,
		0b0101,
		0b1111,

		0b1011,	// $e9
		0b1111,
		0b1011,
		0b1011,
		0b1011,
		0b1111,

		0b1011,	// $ea
		0b1111,
		0b1011,
		0b1011,
		0b1011,
		0b0111,

		0b0111,	// $eb
		0b0101,
		0b0011,
		0b0101,
		0b0101,
		0b1111,

		0b1011,	// $ec
		0b1011,
		0b1011,
		0b1011,
		0b1101,
		0b1111,

		0b1111,	// $ed
		0b0101,
		0b0001,
		0b0101,
		0b0101,
		0b1111,

		0b1111,	// $ee
		0b0011,
		0b0101,
		0b0101,
		0b0101,
		0b1111,

		0b1111,	// $ef
		0b1011,
		0b0101,
		0b0101,
		0b1011,
		0b1111,

		0b1111,	// $f0
		0b0011,
		0b0101,
		0b0101,
		0b0011,
		0b0111,

		0b1111,	// $f1
		0b1001,
		0b0101,
		0b0101,
		0b1001,
		0b1101,

		0b1111,	// $f2
		0b0101,
		0b0011,
		0b0111,
		0b0111,
		0b1111,

		0b1111,	// $f3
		0b1001,
		0b0011,
		0b1101,
		0b0011,
		0b1111,

		0b0111,	// $f4
		0b0011,
		0b0111,
		0b0111,
		0b1001,
		0b1111,

		0b1111,	// $f5
		0b0101,
		0b0101,
		0b0101,
		0b1001,
		0b1111,

		0b1111,	// $f6
		0b0101,
		0b0101,
		0b0101,
		0b0011,
		0b1111,

		0b1111,	// $f7
		0b0101,
		0b0101,
		0b0001,
		0b0101,
		0b1111,

		0b1111,	// $f8
		0b0101,
		0b1011,
		0b0101,
		0b0101,
		0b1111,

		0b1111,	// $f9
		0b0101,
		0b0101,
		0b1001,
		0b1101,
		0b1011,

		0b1111,	// $fa
		0b0001,
		0b1101,
		0b1011,
		0b0001,
		0b1111,

		0b1001,	// $fb
		0b1011,
		0b0011,
		0b1011,
		0b1001,
		0b1111,

		0b1011,	// $fc
		0b1011,
		0b1011,
		0b1011,
		0b1011,
		0b1111,

		0b0011,	// $fd
		0b1011,
		0b1001,
		0b1011,
		0b0011,
		0b1111,

		0b1011,	// $fe
		0b0101,
		0b1111,
		0b1111,
		0b1111,
		0b1111,

		0b1011,	// $ff
		0b0101,
		0b0101,
		0b0001,
		0b1111,
		0b1111
	};
};

#endif
