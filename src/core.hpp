// ---------------------------------------------------------------------
// core.hpp
// lime
//
// (c)2025 elmerucr
// ---------------------------------------------------------------------

// ---------------------------------------------------------------------
// 0x0000-0x00ff base page
// 0x0100-0x01ff system stack
// ??0x0200-0x03ff io
// ??0x0400-0x07ff sprite table (128 x 8 bytes), xpos, ypos, flags, pointer, palette
// 0x0800-0x1fff tilesets (6kb), 0x2000 start of set 0, 0x2800 start of set 1
// ---------------------------------------------------------------------

#ifndef CORE_HPP
#define CORE_HPP

#include "system.hpp"
#include "vdc.hpp"
#include "cpu.hpp"
#include "exceptions.hpp"
#include "rom.hpp"
#include "font_cbm_8x8.hpp"
#include "timer.hpp"
#include "sound.hpp"
#include "clocks.hpp"

#include <cstdio>

#define CBM_FONT_PAGE		0x10
#define COMBINED_PAGE		0x04
#define		VDC_SUB_PAGE	0x00
#define		TIMER_SUB_PAGE	0x40
#define		CORE_SUB_PAGE	0x80
#define	SOUND_PAGE			0x05
#define SYSTEM_ROM_PAGE		0xfe

enum output_states {
	NORMAL,
	BREAKPOINT,
};

class core_t {
private:
	uint32_t sound_cycle_saldo{0};

	//bool irq_line_frame_done{true};

	system_t *system;
	rom_t *rom;

	// memory configuration address $00
	bool system_rom_visible;	// bit 0
	bool character_rom_visible;	// bit 1

	// irq related
	uint8_t irq_number;		// unique number assigned by exception unit
	bool irq_line;
	bool generate_interrupts;
	bool bin_attached;

	FILE *f{NULL};
	uint8_t file_data[65536];
	uint16_t file_pointer;
public:
	core_t(system_t *s);
	~core_t();

	vdc_t *vdc;
	exceptions_ic *exceptions;
	cpu_t *cpu;
	clocks *cpu2sid;
	font_cbm_8x8_t *font;
	timer_ic *timer;
	sound_ic *sound;

	uint32_t get_sound_cycle_saldo() {
		uint32_t result = sound_cycle_saldo;
		sound_cycle_saldo = 0;
		return result;
	}

	void reset();

	uint8_t read8(uint16_t address);
	void write8(uint16_t address, uint8_t value);

	enum output_states run(bool debug);

	void attach_bin(char *path);
};

#endif
