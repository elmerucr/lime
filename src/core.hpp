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
#include "rca.hpp"
#include "rom.hpp"
#include "font_cbm_8x8.hpp"

#define CBM_FONT_PAGE	0x10
#define VDC_PAGE		0x02
#define ROM_PAGE		0xff

enum output_states {
	NORMAL,
	BREAKPOINT,
};

class core_t {
private:
	int32_t cpu_cycle_saldo{0};
	bool irq_line_frame_done{true};

	system_t *system;
	rom_t *rom;
	rca_t rca;

	bool cbm_font_visible;
public:
	core_t(system_t *s);
	~core_t();

	vdc_t *vdc;
	exceptions_ic *exceptions;
	cpu_t *cpu;
	font_cbm_8x8_t *font;

	void reset();
	int32_t get_cpu_cycle_saldo() { return cpu_cycle_saldo; }

	uint8_t read8(uint16_t address);
	void write8(uint16_t address, uint8_t value);

	enum output_states run(bool debug);
};

#endif
