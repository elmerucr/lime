; ----------------------------------------------------------------------
; rom.s (assembles with vasmm68k_mot)
; lime
;
; Copyright © 2025-2026 elmerucr. All rights reserved.
; ----------------------------------------------------------------------

; ----------------------------------------------------------------------
; - Calling convention: D0-D1/A0-A1 are scratch registers, and need to
;   be caller saved when to be kept
; - tab size: 8
;
; ----------------------------------------------------------------------

	include	"definitions.i"

logo_animation	equ	$6000	; 1 byte
binary_ready	equ	$6001	; 1 byte
cursor_pos	equ	$6002	; 1 word
cursor_color	equ	$6004	; 1 byte
; available slot
terminal_chars	equ	$6006	; 1 word
terminal_colors	equ	$6008	; 1 word

TERMINAL_HPITCH	equ	$40	; 64
TERMINAL_VPITCH	equ	$20	; 32
TERMINAL_WIDTH	equ	$28	; 40 columns
TERMINAL_HEIGHT	equ	$16	; 22 rows
TERMINAL_SIZE	equ	(TERMINAL_HPITCH*TERMINAL_HEIGHT)


	org	$00010000	; rom based at $10000

	dc.l	$01000000	; initial ssp at end of ram
	dc.l	_start		; reset vector
	dc.b	"rom mc68000 0.7.20260104"

	align	2

_start
	move.l	#$00010000,A0			; set usp
	move.l	A0,USP

	move.l	#$8000,D0			; small delay at reset
.1	subq.l	#1,D0
	bne	.1

	jsr	init_vector_table
	jsr	vdc_init_layer0
	jsr	vdc_copy_rom_font
	jsr	vdc_copy_logo_tiles
	jsr	vdc_init_logo
	jsr	sound_reset
	jsr	terminal_init
	jsr	terminal_clear

	move.b	#$68,logo_animation.w		; init variable for letter wobble
	move.b	#$b3,VDC_IRQ_SCANLINE_LSB	; set rasterline 179
	move.b	#%00000001,VDC_CR		; enable irq's for vdc

	move.w	#$0000,SR			; set status register (User Mode, ipl = 0)

	clr.b	binary_ready.w

loop	tst.b	binary_ready.w
	beq	loop				; loop forever, wait for events

_jump


exc_addr_error
	bra	exc_addr_error			; TODO: bsod when this happens?


exc_lvl1_irq_auto
	rte


exc_lvl2_irq_auto
	movem.l	D0-D1,-(SP)
	move.b	CORE_SR,D0				; did core cause an irq?
	beq	.2					; no
	move.b	D0,CORE_SR				; yes, acknowledge

	move.b	CORE_FILE_DATA.w,D0			; get first byte
	cmp	#1,D0
	bne	.1					; it's not a valid file

	pea	file_loading
	jsr	terminal_putstring
	lea	(4,SP),SP

	clr.l	D0
	move.b	CORE_FILE_DATA.w,D0
	lsl.l	#8,D0
	move.b	CORE_FILE_DATA.w,D0
	lsl.l	#8,D0
	move.b	CORE_FILE_DATA.w,D0

	movem.l	D0-D1,-(SP)
	move.b	#'$',-(SP)
	jsr	terminal_putchar
	lea	(2,SP),SP
	movem.l	(SP)+,D0-D1

	move.l	D0,-(SP)
	move.b	#6,-(SP)
	jsr	terminal_put_hex_byte
	lea	(6,SP),SP
	bra	.2

.1	pea	file_error
	jsr	terminal_putstring
	lea	(4,SP),SP
.2	movem.l	(SP)+,D0-D1
	rte


exc_lvl4_irq_auto					; coupled to timer
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

.3	rte


exc_lvl6_irq_auto				; coupled to vdc
	move.b	VDC_CURRENT_SPRITE,-(SP)

	move.b	VDC_SR.w,D0
	beq	.end
	move.b	D0,VDC_SR.w			; acknowledge irq

	move.b	logo_animation,D0
	addq.b	#$1,D0
	cmp.b	#$b8,D0				; did we reach x position $b8?
	bne	.1				; no jump to .1
	move.b	#%00000001,CORE_CR		; yes, activate irq's for binary insert (each time we reach $b8)
						; this makes sure letters wobble at least 1 time before binary
						; load process starts
	move.b	#$48,D0				; reset x position to $48

.1	move.b	D0,logo_animation

	move.b	#4,D1				; start with sprite 4 (letter 'l')
.2	move.b	D1,VDC_CURRENT_SPRITE
	move.b	#90,VDC_SPRITE_Y_LSB		; base position for each letter

	move.b	VDC_SPRITE_X_LSB,D0		; store x for current sprite in D0
	sub.b	logo_animation,D0		; subtract logo_an x value from D0

	cmp.b	#8,D0
	bcc	.3				; if more than 8, jump to .3

	subq.b	#1,VDC_SPRITE_Y_LSB		; move letter up 1 pixel

.3	addq	#1,D1				; move to next sprite
	cmp.b	#8,D1				; did we reach sprite 8?
	bne	.2				; not yet, jump to .2

.end	move.b	CORE_INPUT0.w,VDC_BG_COLOR.w

	move.b	(SP)+,VDC_CURRENT_SPRITE
	rte


timer_default_handler
	move.b	#$12,VDC_BG_COLOR.w
	rts


sound_reset
	movem.l	D0/A0,-(SP)

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

	movem.l	(SP)+,D0/A0
	rts


init_vector_table
	; can this be improved?
	move.l	#exc_addr_error,VEC_ADDR_ERROR.w
	move.l	#exc_lvl1_irq_auto,VEC_LVL1_IRQ_AUTO.w
	move.l	#exc_lvl2_irq_auto,VEC_LVL2_IRQ_AUTO.w
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
	rts


vdc_init_layer0
	move.b	VDC_CURRENT_LAYER.w,-(SP)
	clr.b	VDC_CURRENT_LAYER.w		; make layer 0 current
	move.b	#%1101,VDC_LAYER_FLAGS0.w	;
	move.b	(SP)+,VDC_CURRENT_LAYER.w
	rts


vdc_copy_rom_font
	move.b	CORE_ROMS.w,-(SP)

	or.b	#%00000010,CORE_ROMS.w			; make rom font visible to cpu
	movea.l	#VDC_TILESET_ADDRESS,A0
	movea.l	#VDC_TILESET_ADDRESS+$1000,A1

.1	move.l	(A0),(A0)+				; copy rom font to underlying ram
	cmpa	A1,A0
	bne	.1

	move.b	(SP)+,CORE_ROMS.w
	rts


vdc_copy_logo_tiles
	movea.l	#logo_tiles,A0
	movea.w	#$11c0,A1		; start at tile $1c

.1	move.b	(A0)+,(A1)+
	cmpa.l	#logo_tiles+64,A0	; 64 bytes = 4 tiles, 16 bytes/tile
	bne	.1

	rts


; setup sprites 0 - 7 (position, flags, index)
vdc_init_logo
	movea.l	#logo_data,A0
	clr.b	D0

.1	move.b	D0,VDC_CURRENT_SPRITE
	movea.l	#VDC_SPRITE_X_MSB,A1

.2	move.b	(A0)+,(A1)+
	cmpa.l	#VDC_SPRITE_X_MSB+7,A1
	bne	.2
	addq	#1,D0
	cmpa.l	#logo_data+56,A0	; 8 * 7 bytes = 56 bytes
	bne	.1

	rts


vdc_disable_logo
	; stop interrupt routine
	; make sprites 0-7 hidden, but leave other data as it is
	; <OR> restore original chars?
	rts


terminal_init
	move.w	#VDC_LAYER0_TILES,terminal_chars
	move.w	#VDC_LAYER0_COLORS,terminal_colors
	move.b	#$02,cursor_color
	rts


terminal_clear
	move.w	#TERMINAL_SIZE,D0
	move.b	cursor_color.w,D1
	movea.w	terminal_chars,A0
	movea.w	terminal_colors,A1

.1	move.b	#' ',(A0)+
	move.b	D1,(A1)+
	subq.w	#1,D0
	bne	.1

	clr.w	cursor_pos
	rts

terminal_putchar
	movea.w	terminal_chars,A0
	movea.w	terminal_colors,A1
	move.w	cursor_pos,D0

	cmp.b	#$0a,(4,SP)		; check for newline
	beq	.1
	cmp.b	#$0d,(4,SP)		; check for carriage return
	beq	.2

	move.b	(4,SP),(A0,D0)		; print char
	move.b	cursor_color,(A1,D0)	; set color

	addq.w	#1,D0			; move cursor 1 step
	move.w	D0,D1
	andi.w	#%111111,D1
	cmp.w	#TERMINAL_WIDTH,D1	; are we at pos 40 or higher?
	blo	.3			; no

.1	addi.w	#TERMINAL_HPITCH,D0	; yes, move cursor one line down
.2	andi.w	#%1111111111000000,D0	; cursor to beginning of line (carriage return)

	cmp.w	#TERMINAL_SIZE,D0	; check for cursor lower than TERMINAL_HEIGHT
	blo	.3			; no

	subi.w	#TERMINAL_HPITCH,D0	; move cursor one line up
	move.w	D0,cursor_pos
	jsr	terminal_add_bottom_row
	rts

.3	move.w	D0,cursor_pos
	rts


terminal_putstring
	movea.l	(4,SP),A0
.1	move.b	(A0)+,D0
	beq	.2
	move.l	A0,-(SP)
	move.b	D0,-(SP)
	jsr	terminal_putchar
	lea	(2,SP),SP
	movea.l	(SP)+,A0
	bra	.1
.2	rts

terminal_put_hex_byte
	move.b	(4,SP),D0	; D0 contains no of digits to print
	beq	.2		; if this is 0, end this function

	subq.b	#1,D0		; reduce number of digits to print by 1
	beq	.1		; if this is 0 (now), only one digit to print

	move.l	(6,SP),D1	; D1 contains the number to be printed

	lsr.l	#4,D1
	move.l	D1,-(SP)
	move.b	D0,-(SP)
	jsr	terminal_put_hex_byte
	lea	(6,SP),SP

.1	move.l	(6,SP),D0
	andi.l	#$f,D0
	lea	hex_values,A0
	move.b	(A0,D0),-(SP)
	jsr	terminal_putchar
	lea	(2,SP),SP

.2	rts

terminal_add_bottom_row
	movem.l	A2-A3,-(SP)

	movea.w	terminal_chars,A0
	lea	(TERMINAL_HPITCH,A0),A1
	movea.w	terminal_colors,A2
	lea	(TERMINAL_HPITCH,A2),A3

	move.w	#TERMINAL_SIZE,D0	; use terminal size, then lowest row is filled properly
	lsr.w	#2,D0			; divide by 4
.1	move.l	(A1)+,(A0)+		; do 4 bytes at once
	move.l	(A3)+,(A2)+		; do 4 bytes at once
	subq.w	#1,D0
	bne	.1

	movem.l	(SP)+,A2-A3
	rts


file_error
	dc.b	$0a,"Error: not a valid binary",0


file_loading
	dc.b	$0a,"Loading binary",$0a,0


	align	2
logo_data
	dc.b	0,152,0,74,%111,0,$1c	; icon top left
	dc.b	0,160,0,74,%111,0,$1d	; icon top right
	dc.b	0,152,0,82,%111,0,$1e	; icon bottom left
	dc.b	0,160,0,82,%111,0,$1f	; icon bottom right
	dc.b	0,147,0,90,%111,0,$6c	; l
	dc.b	0,152,0,90,%111,0,$69	; i
	dc.b	0,158,0,90,%111,0,$6d	; m
	dc.b	0,166,0,90,%111,0,$65	; e


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

hex_values
	dc.b	"0123456789abcdef"
