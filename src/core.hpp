// ---------------------------------------------------------------------
// core.hpp
// lime
//
// (c)2025 elmerucr
// ---------------------------------------------------------------------

// ---------------------------------------------------------------------
// Register 0 - Status Register (SR)
//
// (READ)
// 7 6 5 4 3 2 1 0
// | | | | | | | |
// | | | | | | | +--
//
// Register 1 - Control Register (SR)
//
// (READ)
// 7 6 5 4 3 2 1 0
// | | | | | | | |
// | | | | | | | +--
//
// (WRITE)
// 7 6 5 4 3 2 1 0
// | | | | | | | |
// | | | | | | | +--
// | | | | | | +----
// | | | | | +------
// | | | | +--------
// | | | +----------
// | | +------------
// | +-------------- 1 reset into mc6809 mode (exclusive or with bit 7)
// +---------------- 1 reset into mc68000 mode (exclusive or with bit 6)
// ---------------------------------------------------------------------

#ifndef CORE_HPP
#define CORE_HPP

#include "system.hpp"
#include "vdc.hpp"
#include "cpu_mc6809.hpp"
#include "exceptions.hpp"
#include "cpu_mc68000.hpp"
#include "sn74ls148.hpp"
#include "rom_mc68000.hpp"
#include "rom_mc6809.hpp"
#include "font_cbm_8x8.hpp"
#include "timer.hpp"
#include "sound.hpp"
#include "clocks.hpp"

#include <cstdio>

#define COMBINED_IO_PAGE    0x000400
#define     VDC_SUB_PAGE      0x00
#define     TIMER_SUB_PAGE    0x40
#define     CORE_SUB_PAGE     0x80
#define	SOUND_IO_PAGE       0x000500
#define CBM_FONT_PAGE       0x001000
#define MC6809_ROM_ADDRESS  0x00fc00
#define MC68000_ROM_ADDRESS 0x010000

enum output_states {
	NORMAL,
	BREAKPOINT
};

class core_t {
private:
	uint32_t sound_cycle_saldo{0};

	system_t *system;
	rom_mc68000_t *rom_mc68000;
	rom_mc6809_t *rom_mc6809;

	// memory configuration address $02 in core bank
	bool system_rom_visible;	// bit 0
	bool character_rom_visible;	// bit 1

	// irq related
	uint8_t dev_number_exceptions;		// unique number assigned by exception unit
	uint8_t dev_number_sn74ls148;
	bool irq_line;
	bool generate_interrupts;
	bool bin_attached;

	FILE *f{NULL};
	uint8_t file_data[65536];
	uint16_t file_pointer;
public:
	core_t(system_t *s);
	~core_t();

	bool mc68000_active;

	uint8_t cpu_multiplier{0};

	vdc_t *vdc;

	exceptions_ic *exceptions;	// for mc6809
	sn74ls148_t *sn74ls148;		// for m68k

	cpu_mc6809_t *mc6809;
	cpu_mc68000_t *mc68000;

	clocks *cpu_to_core_clock;
	clocks *core_to_sid_clock;

	font_cbm_8x8_t *font;
	timer_ic *timer;
	sound_ic *sound;

	uint32_t get_sound_cycle_saldo() {
		uint32_t result = sound_cycle_saldo;
		sound_cycle_saldo = 0;
		return result;
	}

	void reset();

	uint8_t read8(uint32_t address);
	void write8(uint32_t address, uint8_t value);

	enum output_states run(bool debug);

	void attach_bin(char *path);
};

#endif
