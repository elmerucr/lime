#ifndef ROM_HPP
#define ROM_HPP

#include <cstdint>

class rom_t {
public:
	const uint8_t data[256] = {
		0x10, 0xce, 0x02, 0x00,	// ff00	lds		$0200		; this enables NMI's on the 6809
		0xce, 0xff, 0x00,		// ff04	ldu		$ff00
		0x86, 0x02,				// ff07	lda		#%00000010
		0x97, 0x00,				// ff09	sta		$00			; make font visible in vram
		0x8e, 0x10, 0x00,		// ff0b	ldx		#$1000
		0xa6, 0x84,				// ff0e lda		,x
		0xa7, 0x80,				// ff10 sta		,x+
		0x8c, 0x20, 0x00,		// ff12 cmpx	#$2000
		0x26, 0xf7,				// ff15 bne		$ff0e
		0x0f, 0x00,				// ff17 clr		$00			; make font not visible
		0x7e, 0xff, 0x19,		// ff19 jmp		$ff19

		0,0,0,0,

		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	// $ff20
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	// $ff30
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	// $ff40
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	// $ff50
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	// $ff60
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	// $ff70
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	// $ff80
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	// $ff90
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	// $ffa0
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	// $ffb0
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	// $ffc0
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	// $ffd0
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	// $ffe0

		0x00, 0x00,							// $fff0 VECTOR_ILL_OPC	= 0x0000
		0x00, 0x00,							// $fff2 VECTOR_SWI3	= 0x0000
		0x00, 0x00,							// $fff4 VECTOR_SWI2	= 0x0000
		0x00, 0x00,							// $fff6 VECTOR_FIRQ	= 0x0000
		0x00, 0x00,							// $fff8 VECTOR_IRQ		= 0x0000
		0x00, 0x00,							// $fffa VECTOR_SWI		= 0x0000
		0x00, 0x00,							// $fffc VECTOR_NMI		= 0x0000
		0xff, 0x00							// $fffe VECTOR_RESET	= 0xff00
	};
};

#endif
