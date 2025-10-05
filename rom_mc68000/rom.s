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

	dc.b	"rom mc68000 0.3 20250929"

	align	2

_start
	; fill vector table
	move.l	#exc_addr_error,VEC_ADDR_ERROR.w
	move.l	#exc_lvl1_irq_auto,VEC_LVL1_IRQ_AUTO.w
	move.l	#exc_lvl4_irq_auto,VEC_LVL4_IRQ_AUTO.w
	move.l	#exc_lvl6_irq_auto,VEC_LVL6_IRQ_AUTO.w
	move.l	#timer_default_handler,VEC_TIMER0.w
	move.l	#timer_default_handler,VEC_TIMER1.w
	move.l	#timer_default_handler,VEC_TIMER2.w
	move.l	#timer_default_handler,VEC_TIMER3.w
	move.l	#timer_default_handler,VEC_TIMER4.w
	move.l	#timer_default_handler,VEC_TIMER5.w
	move.l	#timer_default_handler,VEC_TIMER6.w
	move.l	#timer_default_handler,VEC_TIMER7.w

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
	movea.l	#VDC_SPRITE_X_MSB,A1
.4	move.b	(A0)+,(A1)+
	cmpa.l	#VDC_SPRITE_X_MSB+7,A1
	bne	.4
	addq	#1,D0
	cmpa.l	#logo_data+56,A0
	bne	.3

; set variable for letter wobble
	move.b	#$40,LOGO_ANIMATION.w

; set raster irq on scanline 159
	move.b	#$9f,VDC_IRQ_SCANLINE_LSB		; rasterline 159
	move.b	#%00000001,VDC_CR			; enable irq's for vdc


	move.w	#$0200,SR				; set status register (User Mode, ipl = 0)

	jsr	sound_reset

	move.b	#0,D0
loop	cmp.b	#4,D0
	addq.b	#1,D0

	bra	loop					; loop forever, wait for events

exc_addr_error
	bra	exc_addr_error				; TODO: bsod when this happens?

exc_lvl1_irq_auto
	rte

exc_lvl4_irq_auto					; coupled to timer
	movem.l	D0-D1/A0,-(SP)

	movea.l	#VEC_TIMER0,A0
	move.b	#%00000001,D0	; D0 contains the bit to be tested

.1	move.b	D0,D1		; copy D0 to D1
	and.b	TIMER_SR.w,D1
	bne	.2		; it was this timer
	addq	#4,A0
	asl.b	D0
	beq	.3
	bra.s	.1

	; code for dealing with this timer
.2	move.b	D0,TIMER_SR.w	; confirm this irq
	movea.l	(A0),A0
	jsr	(A0)

.3	movem.l	(SP)+,D0-D1/A0
	rte

exc_lvl6_irq_auto				; coupled to vdc
	movem.l	D0-D1,-(SP)
	move.b	VDC_CURRENT_SPRITE,-(SP)

	move.b	VDC_SR.w,D0
	beq	.end
	move.b	D0,VDC_SR.w			; acknowledge irq

	move.b	LOGO_ANIMATION,D0
	addq.b	#$1,D0
	cmp.b	#$90,D0
	bne	.1
	move.b	#%00000001,CORE_CR		; activate irq's for binary insert
						; this makes sure letters wobble at least 1 time
	clr.b	D0

.1	move.b	D0,LOGO_ANIMATION

	move.b	#4,D1				; start with sprite 4 (letter 'l')
.2	move.b	D1,VDC_CURRENT_SPRITE
	move.b	#90,VDC_SPRITE_Y_LSB		; base position for each letter

	move.b	VDC_SPRITE_X_LSB,D0		; store x for current sprite in D0
	sub.b	LOGO_ANIMATION,D0		; subtract logo_an x value from D0

	cmp.b	#8,D0
	bcc	.3				; if more than 8, jump to .3

	subq.b	#1,VDC_SPRITE_Y_LSB		; move letter up 1 pixel

.3	addq	#1,D1				; move to next sprite
	cmp.b	#8,D1				; did we reach sprite 8?
	bne	.2				; not yet, jump to .2

.end	move.b	CORE_INPUT0.w,VDC_BG_COLOR.w
	move.b	(SP)+,VDC_CURRENT_SPRITE
	movem.l	(SP)+,D0-D1
	rte

timer_default_handler
	move.b	#$12,VDC_BG_COLOR.w
	rts

sound_reset
	movea.l	#SID0_F,A0
.1	clr.b	(A0)+
	cmpa.l	#SID0_F+$40,A0
	bne	.1
	move.b	#$7f,D0
	movea.l	#MIX_SID0_LEFT,A0
.2	move.b	D0,(A0)+
	cmpa.l	#MIX_SID0_LEFT+$8,A0
	bne	.2
	move.b	#$f,SID0_V
	move.b	#$f,SID1_V
	rts

logo_data
	dc.b	0,112,0,74,%111,0,$1c	; icon top left
	dc.b	0,120,0,74,%111,0,$1d	; icon top right
	dc.b	0,112,0,82,%111,0,$1e	; icon bottom left
	dc.b	0,120,0,82,%111,0,$1f	; icon bottom right
	dc.b	0,107,0,90,%111,0,$6c	; l
	dc.b	0,112,0,90,%111,0,$69	; i
	dc.b	0,118,0,90,%111,0,$6d	; m
	dc.b	0,126,0,90,%111,0,$65	; e

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
