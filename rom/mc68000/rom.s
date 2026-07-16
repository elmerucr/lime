;-----------------------------------------------------------------------
; rom.s (assembles with vasmm68k_mot)
; lime
;
; Copyright © 2025-2026 elmerucr. All rights reserved.
;-----------------------------------------------------------------------
; Calling convention:
; - D0-D1/A0-A1 are scratch registers, and need to be caller saved
;   when to be kept
; - Calling a trap is an exception, but otherwise works the same as
;   a conventional function / routine (scratch registers)
; - Other (real) exceptions will save and restore all registers
; - Tab size: 8
;-----------------------------------------------------------------------
; rom v0.10
; adjusted (again) for 320x180 resolution
;
; rom v0.9
; adjusted for 320x176 screen resolution
;-----------------------------------------------------------------------

	include	"definitions.inc"

;-----------------------------------------------------------------------
; constants
TERMINAL_HPITCH	equ	$80	; 128 tiles
TERMINAL_VPITCH	equ	$20	; 32 tiles
TERMINAL_WIDTH	equ	$50	; 80 columns visible
TERMINAL_HEIGHT	equ	$14	; 20 rows
TERMINAL_BG_COL	equ	$93
TERMINAL_FG_COL	equ	$99

;-----------------------------------------------------------------------
		rsset	$6000

logo_cntdwn	rs.l	1
logo_animation	rs.b	1
logo_status	rs.b	1
cursor_pos	rs.w	1
cursor_color	rs.b	1
cursor_active	rs.b	1
terminal_chars	rs.l	1
terminal_colors	rs.l	1
terminal_buf_1	rs.b	128
terminal_buf_2	rs.b	128

chunk_length	rs.l	1
chunk_address	rs.l	1
exec_address	rs.l	1

rnda		rs.b	1
rndb		rs.b	1
rndc		rs.b	1
rndx		rs.b	1

;-----------------------------------------------------------------------

	section	code

	org	$00010000	; rom based at $10000

	dc.l	$01000000	; initial ssp at end of ram
	dc.l	start		; reset vector
version	dc.b	"rom mc68000 0.10.20260715",0


start
	move.l	#$00010000,A0			; set usp
	move.l	A0,USP

	move.l	#$f000,D0			; small delay at reset
.1	subq.l	#1,D0
	bne	.1

	jsr	init_vector_table
	jsr	copy_fonts_from_rom
	jsr	copy_logo_tiles
	jsr	init_logo
	jsr	sound_reset

	move.l	#VDC_LAYER_TILES,terminal_chars		; default location
	move.l	#VDC_LAYER_COLORS,terminal_colors	; default location

	move.b	#$01,VDC_BORDER_COLOR.w		; dark grey / black
	move.b	#$0a,VDC_BORDER_SIZE.w
	clr.b	VDC_CURRENT_LAYER.w		; make layer 0 current
	move.b	#%1100,VDC_LAYER_FLAGS0.w	;
	move.w	#$fff6,VDC_LAYER_Y_MSB.w	; y location

	clr.b	cursor_active
	move.b	#$b7,cursor_color		; greenish
	move.b	#$01,VDC_BG_COLOR		; black / dark grey

	bsr	terminal_clear
	lea	logo_boot_msg,A0		; print boot message
	bsr	terminal_putstring

	move.b	#$68,logo_animation.w		; init variable for letter wobble
	move.l	#$77777,logo_cntdwn		; counter init value before displaying message
	move.b	#$b3,VDC_IRQ_SCANLINE_LSB	; set rasterline 179
	move.b	#%00000001,VDC_CR		; enable irq's for vdc

	andi.w	#$00ff,SR			; jump to user mode, IPL reg = 0b000

	clr.b	logo_status.w
	clr.l	rnda				; init random generator, clears all: rnda, rndb, rndc, rndx


logo_screen
	subq.l	#1,logo_cntdwn
	bne.s	.ls1				; didn't reach 0
	move.b	#%1101,$414.w			; display layer 0

.ls1	move.b	(KEYBOARD_STATE+1).w,D0		; check status of esc key
	beq.s	.ls2				; not pressed
	btst	#0,D0				; check bit0
	bne.s	.ls2
	or.b	#%00000010,logo_status
.ls2	tst.b	logo_status
	beq.s	logo_screen			; nothing happened, go back

; either [esc] or boot happened
	clr.b	CORE_CR				; stop irq's when new bin inserted or basic mode starts
	clr.b	VDC_CR				; stop VDC interrupts

	clr.b	D0
.ls3	move.b	D0,VDC_CURRENT_SPRITE		; make sprite 0-7 inactive
	clr.b	VDC_SPRITE_FLAGS0
	addq.b	#1,D0
	cmp.b	#8,D0
	bne	.ls3

	clr.b	VDC_CURRENT_LAYER.w		; make layer 0 current and visible
	or.b	#%00000001,VDC_LAYER_FLAGS0.w

	move.b	#TERMINAL_FG_COL,cursor_color
	move.b	#TERMINAL_BG_COL,VDC_BG_COLOR.w	; Atari Basic BG
	move.b	#%10000000,KEYBOARD_CR.w	; purge keyboard events
	bsr	terminal_clear
	bsr	terminal_welcome

	btst	#0,logo_status
	bne.s	boot_binary


screen_editor
	;bsr	START
	;jmp	WSTART
	bsr	basic_cold_start
	bsr	basic_warm_start
	move.b	#%1,cursor_active
.se1	bsr	terminal_flip_cursor		; make visible

.se2	move.b	KEYBOARD_EVENTS.w,D1		; load potential key event into D1
	beq.s	.se2				; no key event (D1 == 0)
	bsr	terminal_flip_cursor		; hide
	cmp.b	#$0a,D1				; is it a newline (return)?
	bne.s	.se3				; no

	bsr.s	screen_copy_to_line_buffer	; yes, copy to line buffer
	move.l	D1,-(SP)
	bsr	basic_process_buffer
	move.l	(SP)+,D1

.se3	move.b	#1,D0				; char out routine
	trap	#15				;
	bra.s	.se1


;-----------------------------------------------------------------------
; Subroutine: screen_copy_to_line_buffer
; Inputs:     -
; Outputs:    -
; Destroyed:  D0, A0, A1
;-----------------------------------------------------------------------
screen_copy_to_line_buffer
	move.w	cursor_pos,D0		; get current cursor position
	andi.w	#$ff80,D0		; cursor to start of line
	movea.l	terminal_chars,A0	; point to beginning of chars
	lea	(A0,D0),A0		; point to start of current line
	lea	terminal_buf_1,A1	; point to buffer
	move.l	#(80-1),D0		; counter = 80 chars
.1	move.b	(A0)+,(A1)+
	dbra	D0,.1
	move.b	#$00,(A1)		; end of line marker
	rts


boot_binary
	movem.l	D0-D1,-(SP)
	move.b	CORE_FILE_DATA.w,D0		; get first byte
	cmp.b	#1,D0
	bne	.bb3				; it's not a valid file

	lea	file_loading1,A0
	bsr	terminal_putstring

.bb1	lea	file_loading2,A0
	bsr	terminal_putstring

	clr.l	D0					; getting chunk length
	move.b	CORE_FILE_DATA.w,D0
	bne	.bb3					; should be zero this first byte
	move.b	CORE_FILE_DATA.w,D0
	lsl.l	#8,D0
	move.b	CORE_FILE_DATA.w,D0
	lsl.l	#8,D0
	move.b	CORE_FILE_DATA.w,D0
	move.l	D0,chunk_length

	move.b	#6,D1
	bsr	terminal_put_hex_number

	lea	file_loading3,A0
	bsr	terminal_putstring

	clr.l	D0			; getting the start address of
	move.b	CORE_FILE_DATA.w,D0	; chunk into D0
	bne	.bb3
	move.b	CORE_FILE_DATA.w,D0
	lsl.l	#8,D0
	move.b	CORE_FILE_DATA.w,D0
	lsl.l	#8,D0
	move.b	CORE_FILE_DATA.w,D0
	move.l	D0,chunk_address

	move.b	#6,D1
	bsr	terminal_put_hex_number

	move.l	chunk_length,D0
	movea.l	chunk_address,A0

.bb2	move.b	CORE_FILE_DATA.w,(A0)+	; get data and store in memory
	subq	#1,D0
	bne	.bb2

	move.l	A0,-(SP)
	lea	file_loading3,A0
	bsr	terminal_putstring
	movea.l	(SP)+,A0

	subq.l	#1,A0			; now A0 contains the last address in which a byte was loaded
	move.l	A0,D0
	move.b	#6,D1
	bsr	terminal_put_hex_number

	move.b	CORE_FILE_DATA.w,D0	; look for next chunk
	cmp.b	#1,D0
	beq	.bb1			; yes, another data chunk
	cmp.b	#$fe,D0			; no, is this a postamble?
	bne	.bb3			; no = error

	move.b	CORE_FILE_DATA.w,D0	; yes it's the postamble
	bne	.bb3			; should be zero
	move.b	CORE_FILE_DATA.w,D0
	bne	.bb3			; should be zero
	move.b	CORE_FILE_DATA.w,D0
	bne	.bb3			; should be zero
	move.b	CORE_FILE_DATA.w,D0
	bne	.bb3			; should be zero

	clr.l	D0
	move.b	CORE_FILE_DATA.w,D0
	bne	.bb3			; should be zero
	move.b	CORE_FILE_DATA.w,D0
	lsl.l	#8,D0
	move.b	CORE_FILE_DATA.w,D0
	lsl.l	#8,D0
	move.b	CORE_FILE_DATA.w,D0	; D0 contains starting address
	move.l	D0,exec_address

	or.b	#%00000001,logo_status
	bra	.bb4

.bb3	lea	file_error,A0
	bsr	terminal_putstring
.bb4	movem.l	(SP)+,D0-D1

	lea	file_loading4,A0
	bsr	terminal_putstring

	move.l	exec_address,D0
	move.b	#8,D1
	bsr	terminal_put_hex_number

	move.l	#$000c0000,D0			; wait loop
.bb5	subq.l	#1,D0
	bne	.bb5

	bsr	terminal_clear

	movea.l	exec_address,A0
	jmp	(A0)

;----------------------------------
; destroys D0, A0
;----------------------------------
terminal_flip_cursor
	tst.b	cursor_active
	beq.s	.1
	move.w	cursor_pos,D0
	movea.l	#VDC_LAYER_TILES,A0
	eori.b	#$80,(A0,D0)
.1	rts


exc_addr_error
	move.b	#$01,VDC_BG_COLOR.w	; black
.1	bra	.1


exc_illegal_instr
	move.b	#$41,VDC_BG_COLOR.w	; red
.1	bra.s	.1


exc_privilege_violation
	move.b	#$b4,VDC_BG_COLOR.w	; green
.1	bra.s	.1


exc_spurious_interrupt
	rte


exc_lvl1_irq_auto
	rte


exc_lvl2_irq_auto
	movem.l	D0-D1,-(SP)
	move.b	CORE_SR.w,D0			; did core cause an irq?
	beq	.el1				; no
	move.b	D0,CORE_SR.w			; yes, acknowledge
	or.b	#%1,logo_status
.el1	movem.l	(SP)+,D0-D1
	rte


exc_lvl4_irq_auto		; coupled to timer
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
	move.b	VDC_CURRENT_SPRITE,-(SP)
	movem.l	D0-D1,-(SP)

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
	move.b	#92,VDC_SPRITE_Y_LSB		; base position for each letter

	move.b	VDC_SPRITE_X_LSB,D0		; store x for current sprite in D0
	sub.b	logo_animation,D0		; subtract logo_an x value from D0

	cmp.b	#8,D0
	bcc	.3				; if more than 8, jump to .3

	subq.b	#1,VDC_SPRITE_Y_LSB		; move letter up 1 pixel

.3	addq	#1,D1				; move to next sprite
	cmp.b	#8,D1				; did we reach sprite 8?
	bne	.2				; not yet, jump to .2

.end	movem.l	(SP)+,D0-D1
	move.b	(SP)+,VDC_CURRENT_SPRITE
	rte

exc_trap14_handler
	cmp.b	#0,D0
	bne	.1
	bsr	rnd_impl
.1	rte

exc_trap15_handler
	cmp.b	#1,D0			; simple, no jump table needed yet
	bne	.1
	move.b	D1,D0
	bsr	terminal_putchar
	rte

.1	cmp.b	#2,D0
	bne	.2
	bsr	terminal_putstring

.2	rte

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


;-----------------------------------------------------------------------
; Doesn't affect any registers
;-----------------------------------------------------------------------
init_vector_table
	move.l	#exc_addr_error,VEC_ADDR_ERROR.w
	move.l	#exc_illegal_instr,VEC_ILLEGAL_INSTR.w
	move.l	#exc_privilege_violation,VEC_PRIVILEGE_VIOLATION.w
	move.l	#exc_spurious_interrupt,VEC_SPURIOUS_INTERRUPT.w
	move.l	#exc_lvl1_irq_auto,VEC_LVL1_IRQ_AUTO.w
	move.l	#exc_lvl2_irq_auto,VEC_LVL2_IRQ_AUTO.w
	move.l	#exc_lvl4_irq_auto,VEC_LVL4_IRQ_AUTO.w
	move.l	#exc_lvl6_irq_auto,VEC_LVL6_IRQ_AUTO.w
	move.l	#exc_trap14_handler,VEC_TRAP14.w
	move.l	#exc_trap15_handler,VEC_TRAP15.w
	move.l	#timer_default_handler,VEC_TIMER0.w
	move.l	#timer_default_handler,VEC_TIMER1.w
	move.l	#timer_default_handler,VEC_TIMER2.w
	move.l	#timer_default_handler,VEC_TIMER3.w
	move.l	#timer_default_handler,VEC_TIMER4.w
	move.l	#timer_default_handler,VEC_TIMER5.w
	move.l	#timer_default_handler,VEC_TIMER6.w
	move.l	#timer_default_handler,VEC_TIMER7.w
	rts


copy_fonts_from_rom
	move.b	CORE_ROMS.w,-(SP)

	or.b	#%00000110,CORE_ROMS.w			; make rom font visible to cpu
	movea.l	#$800,A0
	movea.l	#$2000,A1

.1	move.l	(A0),(A0)+				; copy rom font to underlying ram
	cmpa	A1,A0
	bne	.1

	move.b	(SP)+,CORE_ROMS.w
	rts


copy_logo_tiles
	movea.l	#logo_tiles,A0
	movea.w	#$11c0,A1		; start at tile $1c

.1	move.b	(A0)+,(A1)+
	cmpa.l	#logo_tiles+64,A0	; 64 bytes = 4 tiles, 16 bytes/tile
	bne	.1

	rts


; setup sprites 0 - 7 (position, flags, index)
init_logo
	movea.l	#logo_data,A0
	clr.b	D0

.1	move.b	D0,VDC_CURRENT_SPRITE
	movea.l	#VDC_SPRITE_X_MSB,A1

.2	move.b	(A0)+,(A1)+
	cmpa.l	#VDC_SPRITE_X_MSB+8,A1
	bne	.2
	addq	#1,D0
	cmpa.l	#logo_data+64,A0	; 8 * 8 bytes = 64 bytes
	bne	.1

	rts


terminal_clear
	move.w	#(TERMINAL_HPITCH*TERMINAL_VPITCH),D0
	move.b	cursor_color.w,D1
	movea.l	terminal_chars,A0
	movea.l	terminal_colors,A1

.1	move.b	#' ',(A0)+
	move.b	D1,(A1)+
	subq.w	#1,D0
	bne	.1

	clr.w	cursor_pos
	rts

; ----------------------------------------------------------------------
; Subroutine: terminal_putchar
; Inputs:     D0 contains char to be printed
; Outputs:    -
; Destroyed:  D0,D1,A0,A1
; ----------------------------------------------------------------------
terminal_putchar
	movea.l	terminal_chars,A0
	movea.l	terminal_colors,A1
	move.w	cursor_pos,D1

	cmp.b	#$0a,D0			; check for linefeed
	beq	.lf
	cmp.b	#$0d,D0			; check for carriage return
	beq	.cr
	cmp.b	#$1d,D0			; cursor right
	beq	.right
	cmp.b	#$11,D0			; cursor down
	beq	.down
	cmp.b	#$91,D0			; cursor up
	beq	.up
	cmp.b	#$9d,D0			; cursor left
	beq	.left
	cmp.b	#$08,D0			; backspace
	beq	.bs

	move.b	D0,(A0,D1.w)		; print char
	move.b	cursor_color,(A1,D1.w)	; set color

.right	addq.w	#1,D1			; move cursor one step to the right
	move.w	D1,D0
	andi.w	#%1111111,D0
	cmp.w	#TERMINAL_WIDTH,D0	; are we at pos 80 or higher?
	blo	.2			; no

.lf	addi.w	#TERMINAL_HPITCH,D1	; yes, move cursor one line down, followed by carriage return
.cr	andi.w	#%1111111110000000,D1	; cursor to beginning of line (carriage return)

.1	cmp.w	#(TERMINAL_HPITCH*TERMINAL_HEIGHT),D1	; check for cursor out of screen
	blo	.2			; no

	subi.w	#TERMINAL_HPITCH,D1	; move cursor one line up
	move.w	D1,cursor_pos
	bsr	terminal_add_bottom_row
	rts

.2	move.w	D1,cursor_pos
	rts

.down	addi.w	#TERMINAL_HPITCH,D1	; yes, move cursor one line down
	bra	.1
	rts

.up	subi.w	#TERMINAL_HPITCH,D1	; move cursor one line up
	bpl.s	.2
	addi.w	#TERMINAL_HPITCH,D1	; move cursor one line down
	bra.s	.2

.left	tst.w	D1
	bne.s	.l0
	rts
.l0	move.w	D1,D0
	andi.w	#$7f,D0
	tst.w	D0
	bne	.l1
	addi.w	#(TERMINAL_WIDTH-1),D1
	bra	.up
.l1	subq.w	#1,D1
	bra.s	.2

.bs	move.w	D1,D0
	beq	.2		; do nothing if we're at position 0 (left top)
	andi.b	#$7f,D0		; the byte in D1 now contains the current column
	bne	.bs0		; it's column 0
	subi.w	#(TERMINAL_HPITCH-(TERMINAL_WIDTH-1)),D1
	move.b	#' ',(A0,D1)
	move.b	cursor_color,(A1,D1)
	bra	.2

.bs0	move.l	D2,-(SP)

	move.w	D1,D2
.bs1	move.b	(A0,D2),-1(A0,D2)
	move.b	(A1,D2),-1(A1,D2)
	addq.w	#1,D2
	addq.b	#1,D0
	cmp.b	#TERMINAL_WIDTH,D0
	bne	.bs1

	subq.w	#1,D2
	move.b	#' ',(A0,D2)
	move.b	cursor_color,(A1,D2)

	move.l	(SP)+,D2

	subq.w	#1,D1

	bra	.2


; ----------------------------------------------------------------------
;
;
;
;
; ----------------------------------------------------------------------
terminal_putstring
	move.b	(A0)+,D0
	beq	.1
	move.l	A0,-(SP)
	bsr	terminal_putchar
	movea.l	(SP)+,A0
	bra	terminal_putstring
.1	rts


; ----------------------------------------------------------------------
;
; Inputs:    D0 contains de number to print, D1 no of digits to print
; Outputs:   -
; Destroyed: D0,D1,A0,A1
; ----------------------------------------------------------------------
terminal_put_hex_number
	tst.b	D1		; D1 contains no of digits to print
	beq	.2		; if this is 0, end this function
	subq.b	#1,D1		; reduce number of digits to print by 1
	beq	.1		; if this is 0 (now), only one digit to print
	move.l	D0,-(SP)
	lsr.l	#4,D0
	move.b	D1,-(SP)
	jsr	terminal_put_hex_number
	move.b	(SP)+,D1
	move.l	(SP)+,D0
.1	move.l	D0,D1
	andi.l	#$f,D1
	lea	hex_values,A0
	move.b	(A0,D1),D0
	jsr	terminal_putchar
.2	rts


; ----------------------------------------------------------------------
; Routine:   terminal_add_bottom_row
; Inputs:    -
; Outputs:   -
; Destroyed: D0,D1,A0,A1
; ----------------------------------------------------------------------
terminal_add_bottom_row
	movem.l	D2/A2-A3,-(SP)

	movea.l	terminal_chars,A0
	lea	TERMINAL_HPITCH(A0),A1
	movea.l	terminal_colors,A2
	lea	TERMINAL_HPITCH(A2),A3

	move.w	#(TERMINAL_HPITCH*(TERMINAL_HEIGHT-1)),D0	; use terminal size minus lowest row
	lsr.w	#2,D0			; divide by 4
.1	move.l	(A1)+,(A0)+		; do 4 bytes at once
	move.l	(A3)+,(A2)+		; do 4 bytes at once
	subq.w	#1,D0
	bne	.1

	move.b	#TERMINAL_HPITCH,D0	; do last row
	move.b	#' ',D1
	move.b	cursor_color,D2
.2	move.b	D1,(A0)+
	move.b	D2,(A2)+
	subq.b	#1,D0
	bne.s	.2

	movem.l	(SP)+,D2/A2-A3
	rts

terminal_welcome
	lea	welcome,A0
	jsr	terminal_putstring
	lea	version,A0
	jsr	terminal_putstring
	rts

; see: https://www.stix.id.au/wiki/Fast_8-bit_pseudorandom_number_generator
rnd_impl
	addq.b	#1,rndx.w
	move.b	rnda.w,D0	; D0 = a
	move.b	rndc.w,D1	; D1 = c
	eor.b	D1,D0		; (a ^ c), in D0
	move.b	rndx.w,D1	; D1 = x
	eor.b	D1,D0		; (a ^ c) ^ x, in D0
	move.b	D0,rnda.w	; store result in a

	move.b	rndb.w,D1	; D1 = b
	add.b	D0,D1		; b = b + a
	move.b	D1,rndb.w
	ror.b	#1,D1
	add.b	rndc.w,D1
	eor.b	D1,D0
	move.b	D0,rndc.w
	rts

logo_boot_msg
		dc.b	$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a
		dc.b	"             drop a binary file to boot or hit [esc] to start basic",0
welcome		dc.b	"lime virtual computer system",$0a,0
file_error	dc.b	$0a,$0a,"error: not a valid binary",0
file_loading1	dc.b	$0a,$0a,"  size    from    to",0
file_loading2	dc.b	$0a,"$",0
file_loading3	dc.b	" $",0
file_loading4	dc.b	$0a,$0a," jumping to $",0


logo_data
	dc.b	0,152,0,76,%111,0,0,$1c	; icon top left
	dc.b	0,160,0,76,%111,0,0,$1d	; icon top right
	dc.b	0,152,0,84,%111,0,0,$1e	; icon bottom left
	dc.b	0,160,0,84,%111,0,0,$1f	; icon bottom right
	dc.b	0,147,0,92,%111,0,0,$6c	; l
	dc.b	0,152,0,92,%111,0,0,$69	; i
	dc.b	0,158,0,92,%111,0,0,$6d	; m
	dc.b	0,166,0,92,%111,0,0,$65	; e


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

	include "basic.s"

	include	"TBI68K.ASM"

end_of_rom
