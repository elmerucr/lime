;-----------------------------------------------------------------------
; definitions.i
; lime
;
; Copyright © 2025 elmerucr. All rights reserved.
;-----------------------------------------------------------------------

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
VDC_CR			equ	$0401
VDC_CURRENT_SCANLINE	equ	$0402
VDC_IRQ_SCANLINE	equ	$0403
VDC_BG_COLOR		equ	$0404
VDC_CURRENT_PALETTE	equ	$0405
VDC_CURRENT_LAYER	equ	$0406
VDC_CURRENT_SPRITE	equ	$0407

VDC_PALETTE_ALPHA	equ	$0408
VDC_PALETTE_RED		equ	$0409
VDC_PALETTE_GREEN	equ	$040a
VDC_PALETTE_BLUE	equ	$040b

VDC_LAYER_X		equ	$0410
VDC_LAYER_Y		equ	$0411
VDC_LAYER_FLAGS0	equ	$0412
VDC_LAYER_FLAGS1	equ	$0413
VDC_LAYER_COLOR0	equ	$0418
VDC_LAYER_COLOR1	equ	$0419
VDC_LAYER_COLOR2	equ	$041a
VDC_LAYER_COLOR3	equ	$041b

VDC_SPRITE_X		equ	$0420
VDC_SPRITE_Y		equ	$0421
VDC_SPRITE_FLAGS0	equ	$0422
VDC_SPRITE_FLAGS1	equ	$0423
VDC_SPRITE_INDEX	equ	$0424
VDC_SPRITE_COLOR0	equ	$0428
VDC_SPRITE_COLOR1	equ	$0429
VDC_SPRITE_COLOR2	equ	$042a
VDC_SPRITE_COLOR3	equ	$042b

VDC_TILESET0		equ	$0800
VDC_TILESET1		equ	$1000
VDC_LAYER0		equ	$2000
VDC_LAYER1		equ	$2400
VDC_LAYER2		equ	$2800
VDC_LAYER3		equ	$2c00

; timer
TIMER_SR		equ	$0440
TIMER_CR		equ	$0441
TIMER0_BPM		equ	$0450
TIMER1_BPM		equ	$0452
TIMER2_BPM		equ	$0454
TIMER3_BPM		equ	$0456
TIMER4_BPM		equ	$0458
TIMER5_BPM		equ	$045a
TIMER6_BPM		equ	$045c
TIMER7_BPM		equ	$045e

; core
CORE_SR			equ	$0480
CORE_CR			equ	$0481
CORE_ROMS		equ	$0482
CORE_FILE_DATA		equ	$0484
CORE_INPUT0		equ	$0488

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

; general
RAM_START		equ	$3000
ROM_START		equ	$fe00
