; ----------------------------------------------------------------------
; definitions.i
; lime
;
; Copyright © 2025 elmerucr. All rights reserved.
; ----------------------------------------------------------------------

; vectors
VEC_ADDR_ERROR		equ	$000c
VEC_LVL1_IRQ_AUTO	equ	$0064
VEC_LVL6_IRQ_AUTO	equ	$0078

; vdc (video display controller)
VDC_SR				equ	$0400
VDC_CR				equ	$0401
VDC_BG_COLOR			equ	$0404
VDC_CURRENT_SPRITE		equ	$0407
VDC_CURRENT_SCANLINE_MSB	equ	$040c
VDC_CURRENT_SCANLINE_LSB	equ	$040d
VDC_IRQ_SCANLINE_MSB		equ	$040e
VDC_IRQ_SCANLINE_LSB		equ	$040f

VDC_SPRITE_X			equ	$0420

VDC_TILESET0			equ	$0800
VDC_TILESET1			equ	$1000

; core
CORE_ROMS			equ	$0482
CORE_INPUT0			equ	$0488
