; ----------------------------------------------------------------------
; definitions.i
; lime
;
; Copyright © 2025 elmerucr. All rights reserved.
; ----------------------------------------------------------------------

VECTOR_ILLOP_INDIRECT	equ	$0200
VECTOR_SWI3_INDIRECT	equ	$0202
VECTOR_SWI2_INDIRECT	equ	$0204
VECTOR_FIRQ_INDIRECT	equ	$0206
VECTOR_IRQ_INDIRECT	equ	$0208
VECTOR_SWI_INDIRECT	equ	$020a
VECTOR_NMI_INDIRECT	equ	$020c
; $020e unused and reserved
VECTOR_TIMER0_INDIRECT	equ	$0210
VECTOR_TIMER1_INDIRECT	equ	$0212
VECTOR_TIMER2_INDIRECT	equ	$0214
VECTOR_TIMER3_INDIRECT	equ	$0216
VECTOR_TIMER4_INDIRECT	equ	$0218
VECTOR_TIMER5_INDIRECT	equ	$021a
VECTOR_TIMER6_INDIRECT	equ	$021c
VECTOR_TIMER7_INDIRECT	equ	$021e
VECTOR_VDC_INDIRECT	equ	$0220

; vdc (video display controller)
VDC_SR			equ	$0400
VDC_BG_COLOR		equ	$0404
VDC_LAYER_CURRENT	equ	$0406
VDC_SPRITE_CURRENT	equ	$0407
VDC_SPRITE_X		equ	$0418
VDC_SPRITE_Y		equ	$0419

; core
CORE_INPUT_0		equ	$0480

; sound devices
SID0_F			equ	$0500
SID0_PW			equ	$0502
SID0_V			equ	$051b
SID1_V			equ	$053b
MIX_SID0_LEFT		equ	$0580
MIX_SID0_RIGHT		equ	$0581
MIX_SID1_LEFT		equ	$0582
MIX_SID1_RIGHT		equ	$0583
MIX_ANA0_LEFT		equ	$0584
MIX_ANA0_RIGHT		equ	$0585
MIX_ANA1_LEFT		equ	$0586
MIX_ANA1_RIGHT		equ	$0587
