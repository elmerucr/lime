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
#include "rca.hpp"
#include "rom.hpp"

#define ROM_PAGE	0xff

enum output_states {
	NORMAL,
	BREAKPOINT,
};

class core_t {
private:
	system_t *system;
	rom_t *rom;
	rca_t rca;
public:
	core_t(system_t *s);
	~core_t();

	vdc_t *vdc;
	cpu_t *cpu;

	void reset();

	uint8_t read8(uint16_t address);
	void write8(uint16_t address, uint8_t value);

	enum output_states run(bool debug);
};

#endif
