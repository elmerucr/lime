; ----------------------------------------------------------------------
; definitions.i
; lime
;
; Copyright © 2025 elmerucr. All rights reserved.
; ----------------------------------------------------------------------

; vectors
VEC_ADDR_ERROR			equ	$000c
VEC_LVL1_IRQ_AUTO		equ	$0064
VEC_LVL4_IRQ_AUTO		equ	$0070	; entry point for timer
VEC_LVL6_IRQ_AUTO		equ	$0078	; entry point for vdc
VEC_TIMER0			equ	$0100
VEC_TIMER1			equ	$0104
VEC_TIMER2			equ	$0108
VEC_TIMER3			equ	$010c
VEC_TIMER4			equ	$0110
VEC_TIMER5			equ	$0114
VEC_TIMER6			equ	$0118
VEC_TIMER7			equ	$011c

; vdc (video display controller)
VDC_SR				equ	$0400
VDC_CR				equ	$0401
VDC_BG_COLOR			equ	$0404
VDC_CURRENT_SPRITE		equ	$0407
VDC_CURRENT_SCANLINE_MSB	equ	$040c
VDC_CURRENT_SCANLINE_LSB	equ	$040d
VDC_IRQ_SCANLINE_MSB		equ	$040e
VDC_IRQ_SCANLINE_LSB		equ	$040f

VDC_LAYER_BASE_MSB		equ	$0416
VDC_LAYER_BASE_LSB		equ	$0417

VDC_SPRITE_X_MSB		equ	$0420
VDC_SPRITE_X_LSB		equ	$0421
VDC_SPRITE_Y_MSB		equ	$0422
VDC_SPRITE_Y_LSB		equ	$0423

VDC_TILESET0			equ	$0800
VDC_TILESET1			equ	$1000

; sound devices
SID0_F				equ	$0500
SID0_PW				equ	$0502
SID0_V				equ	$051b
SID1_V				equ	$053b
MIX_SID0_LEFT			equ	$0580
MIX_SID0_RIGHT			equ	$0581
MIX_SID1_LEFT			equ	$0582
MIX_SID1_RIGHT			equ	$0583
MIX_ANA0_LEFT			equ	$0584
MIX_ANA0_RIGHT			equ	$0585
MIX_ANA1_LEFT			equ	$0586
MIX_ANA1_RIGHT			equ	$0587

; timer
TIMER_SR			equ	$0440
TIMER_CR			equ	$0441
TIMER0_BPM			equ	$0450
TIMER1_BPM			equ	$0452
TIMER2_BPM			equ	$0454
TIMER3_BPM			equ	$0456
TIMER4_BPM			equ	$0458
TIMER5_BPM			equ	$045a
TIMER6_BPM			equ	$045c
TIMER7_BPM			equ	$045e

; core
CORE_ROMS			equ	$0482
CORE_INPUT0			equ	$0488
