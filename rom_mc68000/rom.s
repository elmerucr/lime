; ----------------------------------------------------------------------
; rom.s (assembles with vasmm68k_mot)
; lime
;
; Copyright © 2025 elmerucr. All rights reserved.
; ----------------------------------------------------------------------

	include	"definitions.i"

LOGO_ANIMATION	equ	$3000


	org	$00010000	; rom based at $10000

	dc.l	$01000000	; initial ssp at end of ram
	dc.l	_start		; reset vector

	dc.b	"rom mc68000 0.2 20250910"

	align	2

_start
	; fill vector table
	move.l	#exc_addr_error,VEC_ADDR_ERROR.w
	move.l	#exc_lvl1_irq_auto,VEC_LVL1_IRQ_AUTO.w
	move.l	#exc_lvl4_irq_auto,VEC_LVL4_IRQ_AUTO.w
	move.l	#exc_lvl6_irq_auto,VEC_LVL6_IRQ_AUTO.w

	; set usp
	move.l	#$00010000,A0
	move.l	A0,USP

	or.b	#%00000010,CORE_ROMS.w			; make rom font visible to cpu
	movea	#VDC_TILESET1,A0
.1	move.l	(A0),(A0)+				; copy rom font to underlying ram
	cmpa	#VDC_TILESET1+$1000,A0
	bne	.1
	and.b	#%11111101,CORE_ROMS.w			; turn off rom font

; copy logo tiles
	movea.l	#logo_tiles,A0
	movea.w	#$11c0,A1
.2	move.b	(A0)+,(A1)+
	cmpa.l	#logo_tiles+64,A0
	bne	.2

; init logo
	movea.l	#logo_data,A0
	clr.b	D0
.3	move.b	D0,VDC_CURRENT_SPRITE
	movea.l	#VDC_SPRITE_X,A1
.4	move.b	(A0)+,(A1)+
	cmpa.l	#VDC_SPRITE_X+5,A1
	bne	.4
	addq	#1,D0
	cmpa.l	#logo_data+40,A0
	bne	.3

; set variable for letter wobble
	move.b	#$40,LOGO_ANIMATION.w

; set raster irq on scanline 159
	move.b	#$9f,VDC_IRQ_SCANLINE_LSB
	move.b	#%00000001,VDC_CR			; enable irq's for vdc


	move.w	#$0200,SR				; set status register (User Mode, ipl = 0)
	move.l	#exc_addr_error,-(SP)			; test usp

.5	bra	.5					; loop forever, wait for events

exc_addr_error
	bra	exc_addr_error				; TODO: bsod when this happens?

exc_lvl1_irq_auto
	rte

exc_lvl4_irq_auto					; coupled to timer
	move.l	D0,-(SP)
	move.b	TIMER_SR.w,D0
	move.l	(SP)+,D0
	rte

exc_lvl6_irq_auto					; coupled to vdc
	move.l	D0,-(SP)

	move.b	VDC_SR.w,D0
	beq	.1
	move.b	D0,VDC_SR.w

	move.b	CORE_INPUT0.w,VDC_BG_COLOR.w

	move.l	(SP)+,D0
.1	rte

logo_data
	dc.b	112,64,%111,0,$1c			; icon top left
	dc.b	120,64,%111,0,$1d			; icon top right
	dc.b	112,72,%111,0,$1e			; icon bottom left
	dc.b	120,72,%111,0,$1f			; icon bottom right
	dc.b	107,80,%111,0,$6c			; l
	dc.b	112,80,%111,0,$69			; i
	dc.b	118,80,%111,0,$6d			; m
	dc.b	126,80,%111,0,$65			; e

logo_tiles
	dc.b	%00000000,%00000000	; tile 1 (icon upper left)
	dc.b	%00000001,%00000000
	dc.b	%00000111,%10000000
	dc.b	%00000111,%10100000
	dc.b	%00011110,%11111000
	dc.b	%00011110,%10101111
	dc.b	%00011110,%10101010
	dc.b	%00011110,%10101111

	dc.b	%00000000,%00000000	; tile 2 (icon upper right)
	dc.b	%00000000,%00000000
	dc.b	%00000000,%00000000
	dc.b	%00000000,%00000000
	dc.b	%00000000,%00000000
	dc.b	%00000000,%00000000
	dc.b	%11000000,%00000000
	dc.b	%10110000,%00000000

	dc.b	%00011110,%11111010	; tile 3 (icon bottom left)
	dc.b	%00000111,%10101010
	dc.b	%00000111,%10101011
	dc.b	%00000001,%11101011
	dc.b	%00000000,%01111110
	dc.b	%00000000,%00010111
	dc.b	%00000000,%00000001
	dc.b	%00000000,%00000000

	dc.b	%11101100,%00000000	; tile 4 (icon bottom right)
	dc.b	%11101110,%00000000
	dc.b	%10101011,%10000000
	dc.b	%10101011,%10100000
	dc.b	%10101010,%11110100
	dc.b	%11111111,%01010000
	dc.b	%01010101,%00000000
	dc.b	%00000000,%00000000
