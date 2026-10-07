;-----------------------------------------------------------------------
; rom.s (assembles with vasmm68k_mot), see Makefile
; lime
;
; Copyright © 2025-2026 elmerucr. All rights reserved.
;-----------------------------------------------------------------------
; Calling conventions:
; - d0-d1/a0-a1 are scratch registers, and need to be caller saved
;   when to be kept
; - All other registers must be callee saved (if used by callee)
; - Calling a trap is an exception, but otherwise works the same as
;   a conventional function / routine (same scratch registers)
; - Other (real) exceptions will save and restore all registers
;
; Other:
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
T_HPITCH	equ	$80	; 128 tiles
T_VPITCH	equ	$20	; 32 tiles
T_WIDTH		equ	$50	; 80 columns visible
T_HEIGHT	equ	$14	; 20 rows
T_BG_COL	equ	$93
T_FG_COL	equ	$99
T_TILES		equ	$2000
T_COLORS	equ	$3000

;-----------------------------------------------------------------------
		rsset	$6000

logo		rs.b	64
logo_cntdwn	rs.l	1
logo_animation	rs.b	1
logo_status	rs.b	1

cursor_pos	rs.w	1
cursor_color	rs.b	1
cursor_active	rs.b	1
cursor_interval	rs.b	1
cursor_cntdwn	rs.b	1
cursor_ori_chr	rs.b	1
cursor_ori_col	rs.b	1

t_chars		rs.l	1
t_colors	rs.l	1
t_buf_1		rs.b	(2*T_WIDTH)
t_buf_2		rs.b	(2*T_WIDTH)
t_link_table	rs.b	T_HEIGHT

chunk_length	rs.l	1
chunk_address	rs.l	1
exec_address	rs.l	1

prnga		rs.b	1
prngb		rs.b	1
prngc		rs.b	1
prngx		rs.b	1

;-----------------------------------------------------------------------

	section	code

	org	$00010000	; rom based at $10000

	dc.l	$01000000	; initial ssp at end of ram
	dc.l	start		; reset vector
version	dc.b	"rom 0.10.20261006",0


start
	move.l	#$00010000,a0			; set usp
	move.l	a0,usp

	jsr	init_vector_table
	jsr	copy_fonts_from_rom
	jsr	copy_logo_tile
	jsr	init_logo
	jsr	sound_reset

	move.l	#T_TILES,t_chars		; default location
	move.l	#T_COLORS,t_colors	; default location

	move.b	#$01,VDC_BORDER_COLOR.w		; dark grey / black
	move.b	#$0a,VDC_BORDER_SIZE.w		; 10 pixels hborder
	clr.b	VDC_CURRENT_LAYER.w		; make layer 0 current
	move.l	#$800,VDC_LAYER_TILESET_ADDR.w
	move.l	#T_TILES,VDC_LAYER_TILES_ADDR.w
	move.l	#T_COLORS,VDC_LAYER_COLORS_ADDR.w
	move.b	#%1100,VDC_LAYER_FLAGS0.w	;
	move.w	#$000a,VDC_LAYER_Y_MSB.w	; y location

	clr.b	cursor_active
	move.b	#$b7,cursor_color		; greenish
	move.b	#$01,VDC_BG_COLOR		; black / dark grey

	bsr	t_clear
	lea	logo_boot_msg,a0		; print boot message
	bsr	t_putstring

	move.b	#$68,logo_animation.w		; init variable for letter wobble
	move.l	#$77777,logo_cntdwn		; counter init value before displaying message
	move.b	#$b3,VDC_IRQ_SCANLINE_LSB	; set rasterline 179
	move.b	#%00000001,VDC_CR		; enable irq's for vdc

	andi.w	#$00ff,sr			; jump to user mode, IPL reg = 0b000

	clr.b	logo_status.w
	clr.l	prnga				; init random generator, clears all: rnda, rndb, rndc, rndx


logo_screen
	subq.l	#1,logo_cntdwn
	bne.s	.ls1				; didn't reach 0
	move.b	#%1101,VDC_LAYER_FLAGS0.w	; display layer 0

.ls1
	move.b	(KEYBOARD_STATE+1).w,d0		; check status of esc key
	beq.s	.ls2				; not pressed
	btst	#0,d0				; check bit0
	bne.s	.ls2
	or.b	#%00000010,logo_status
.ls2
	tst.b	logo_status
	beq.s	logo_screen			; nothing happened, go back

; either [esc] or boot happened
	clr.b	CORE_CR				; stop irq's when new bin inserted or basic mode starts
	clr.b	VDC_CR				; stop VDC interrupts

	clr.b	d0
.ls3	move.b	d0,VDC_CURRENT_SPRITE		; make sprite 0-7 inactive
	clr.b	VDC_SPRITE_FLAGS0
	addq.b	#1,d0
	cmp.b	#8,d0
	bne	.ls3

	clr.b	VDC_CURRENT_LAYER.w		; make layer 0 current and visible
	or.b	#%00000001,VDC_LAYER_FLAGS0.w

	move.b	#T_FG_COL,cursor_color
	move.b	#T_BG_COL,VDC_BG_COLOR.w	; Atari Basic BG
	move.b	#%10000000,KEYBOARD_CR.w	; purge keyboard events
	bsr	t_clear
	bsr	t_welcome

	btst	#0,logo_status
	bne.s	boot_binary


screen_editor
	bsr	b_cold_start
	bsr	b_warm_start
	move.b	#20,cursor_interval		; 20/50 = 0.4s if at 50Hz
	;move.b	#%1,cursor_active
	move.w	#$bb8,TIMER0_BPM.w		; 3000bpm = 50Hz
	move.b	#%00000001,TIMER_CR.w

.se1	bsr	t_cursor_activate		; make cursor visible

.se2	move.b	KEYBOARD_EVENTS.w,d0		; load potential key event into d0
	beq.s	.se2				; no key event (d0 == 0)
	move.l	d0,-(sp)
	bsr	t_cursor_deactivate		; hide
	move.l	(sp)+,d0
	cmp.b	#$0a,d0				; is it a newline (return)?
	bne.s	.se3				; no

	move.l	d0,-(sp)
	bsr.s	screen_copy_to_line_buffer	; yes, copy to line buffer
	bsr	b_process_buffer
	move.l	(sp)+,d0

.se3	move.b	#1,d1				; char out routine
	trap	#15				;
	bra.s	.se1


;-----------------------------------------------------------------------
; Subroutine: screen_copy_to_line_buffer
; Inputs:     -
; Outputs:    -
; Destroyed:  d0, a0, a1
;-----------------------------------------------------------------------
screen_copy_to_line_buffer
	move.w	cursor_pos,d0		; get current cursor position
	andi.w	#$ff80,d0		; cursor to start of line
	movea.l	t_chars,a0		; point to beginning of chars
	lea	(a0,d0),a0		; point to start of current line
	lea	t_buf_1,a1	; point to buffer
	move.l	#(80-1),d0		; counter = 80 chars
.1	move.b	(a0)+,(a1)+
	dbra	d0,.1
	move.b	#$00,(a1)		; end of line marker
	rts


boot_binary
	movem.l	d0-d1,-(sp)
	move.b	CORE_FILE_DATA.w,d0		; get first byte
	cmp.b	#1,d0
	bne	.bb3				; it's not a valid file

	lea	file_loading1,a0
	bsr	t_putstring

.bb1	lea	file_loading2,a0
	bsr	t_putstring

	clr.l	d0					; getting chunk length
	move.b	CORE_FILE_DATA.w,d0
	bne	.bb3					; should be zero this first byte
	move.b	CORE_FILE_DATA.w,d0
	lsl.l	#8,d0
	move.b	CORE_FILE_DATA.w,d0
	lsl.l	#8,d0
	move.b	CORE_FILE_DATA.w,d0
	move.l	d0,chunk_length

	move.b	#6,d1
	bsr	t_put_hex_number

	lea	file_loading3,a0
	bsr	t_putstring

	clr.l	d0			; getting the start address of
	move.b	CORE_FILE_DATA.w,d0	; chunk into d0
	bne	.bb3
	move.b	CORE_FILE_DATA.w,d0
	lsl.l	#8,d0
	move.b	CORE_FILE_DATA.w,d0
	lsl.l	#8,d0
	move.b	CORE_FILE_DATA.w,d0
	move.l	d0,chunk_address

	move.b	#6,d1
	bsr	t_put_hex_number

	move.l	chunk_length,d0
	movea.l	chunk_address,a0

.bb2	move.b	CORE_FILE_DATA.w,(a0)+	; get data and store in memory
	subq	#1,d0
	bne	.bb2

	move.l	a0,-(sp)
	lea	file_loading3,a0
	bsr	t_putstring
	movea.l	(sp)+,a0

	subq.l	#1,a0			; now a0 contains the last address in which a byte was loaded
	move.l	a0,d0
	move.b	#6,d1
	bsr	t_put_hex_number

	move.b	CORE_FILE_DATA.w,d0	; look for next chunk
	cmp.b	#1,d0
	beq	.bb1			; yes, another data chunk
	cmp.b	#$fe,d0			; no, is this a postamble?
	bne	.bb3			; no = error

	move.b	CORE_FILE_DATA.w,d0	; yes it's the postamble
	bne	.bb3			; should be zero
	move.b	CORE_FILE_DATA.w,d0
	bne	.bb3			; should be zero
	move.b	CORE_FILE_DATA.w,d0
	bne	.bb3			; should be zero
	move.b	CORE_FILE_DATA.w,d0
	bne	.bb3			; should be zero

	clr.l	d0
	move.b	CORE_FILE_DATA.w,d0
	bne	.bb3			; should be zero
	move.b	CORE_FILE_DATA.w,d0
	lsl.l	#8,d0
	move.b	CORE_FILE_DATA.w,d0
	lsl.l	#8,d0
	move.b	CORE_FILE_DATA.w,d0	; d0 contains starting address
	move.l	d0,exec_address

	or.b	#%00000001,logo_status
	bra	.bb4

.bb3	lea	file_error,a0
	bsr	t_putstring
.bb4	movem.l	(sp)+,d0-d1

	lea	file_loading4,a0
	bsr	t_putstring

	move.l	exec_address,d0
	move.b	#8,d1
	bsr	t_put_hex_number

	move.l	#$000c0000,d0			; wait loop
.bb5	subq.l	#1,d0
	bne	.bb5

	bsr	t_clear

	movea.l	exec_address,a0
	jmp	(a0)


;-----------------------------------------------------------------------
; Subroutine: t_cursor_activate
;-----------------------------------------------------------------------
t_cursor_activate
	movea.l	t_chars,a0
	move.w	cursor_pos,d0
	move.b	(a0,d0),cursor_ori_chr
	movea.l	t_colors,a0
	move.b	(a0,d0),cursor_ori_col
	clr.b	cursor_cntdwn			; set counter on 0
	move.b	#%1,cursor_active
	rts


;-----------------------------------------------------------------------
; Subroutine: t_cursor_deactivate
;-----------------------------------------------------------------------
t_cursor_deactivate
	clr.b	cursor_active
	movea.l	t_chars,a0
	move.w	cursor_pos,d0
	move.b	cursor_ori_chr,(a0,d0)
	movea.l	t_colors,a0
	move.b	cursor_ori_col,(a0,d0)
	rts


;-----------------------------------------------------------------------
; Subroutine: t_cursor_process
;-----------------------------------------------------------------------
t_cursor_process
	movem.l	d2-d3,-(sp)		; routine called by exc handler that already restores other regs
	tst.b	cursor_active
	beq	.end

	tst.b	cursor_cntdwn
	bne	.cntdwn

	movea.l	t_chars,a0
	move.w	cursor_pos,d0
	eori.b	#$80,(a0,d0)		; invert char

	move.b	(a0,d0),d1
	andi.b	#$80,d1
	move.b	cursor_ori_chr,d2
	andi.b	#$80,d2
	movea.l	t_colors,a0
	move.b	cursor_interval,d3

	cmp.b	d1,d2
	beq.s	.equal
.uneq	move.b	cursor_color,(a0,d0)	; apply cursor color when cursor is visible
	add.b	d3,cursor_cntdwn
	bra.s	.cntdwn
.equal	move.b	cursor_ori_col,(a0,d0)	; apply orig color when not visible
	add.b	d3,cursor_cntdwn
.cntdwn	subi.b	#1,cursor_cntdwn
.end	movem.l	(sp)+,d2-d3
	rts


; -----------
;
; ------
exc_addr_error
	move.b	#$01,VDC_BG_COLOR.w	; black
.1	bra	.1


; -----------
;
; ------
exc_illegal_instr
	move.b	#$41,VDC_BG_COLOR.w	; red
.1	bra.s	.1


; -----------
;
; ------
exc_privilege_violation
	move.b	#$b4,VDC_BG_COLOR.w	; green
.1	bra.s	.1


; -----------
;
; ------
exc_spurious_interrupt
	rte


; -----------
;
; ------
exc_lvl1_irq_auto
	rte


; -----------
;
; ------
exc_lvl2_irq_auto
	movem.l	d0-d1,-(sp)
	move.b	CORE_SR.w,d0			; did core cause an irq?
	beq	.el1				; no
	move.b	d0,CORE_SR.w			; yes, acknowledge
	or.b	#%1,logo_status
.el1	movem.l	(sp)+,d0-d1
	rte


; -----------
;
; ------
exc_lvl4_irq_auto		; coupled to timer
	movem.l	d0-d1/a0,-(sp)
	movea.l	#VEC_TIMER0,a0
	move.b	#%00000001,d0	; d0 contains the bit to be tested

.1	move.b	d0,d1		; copy d0 to d1
	and.b	TIMER_SR.w,d1
	bne	.2		; it was this timer
	addq	#4,a0
	asl.b	d0
	beq	.3
	bra.s	.1

	; code for dealing with this timer
.2	move.b	d0,TIMER_SR.w	; confirm this irq
	movea.l	(a0),a0
	jsr	(a0)

.3	movem.l	(sp)+,d0-d1/a0
	rte

; ----------------------------------------------------------------------
; Subroutine: exception vdc / letter wobble
; -----------------
exc_lvl6_irq_auto				; coupled to vdc
	move.b	VDC_CURRENT_SPRITE,-(SP)
	movem.l	d0-d1,-(sp)

	move.b	VDC_SR.w,d0
	beq	.end
	move.b	d0,VDC_SR.w			; acknowledge irq

	move.b	logo_animation,d0
	addq.b	#$1,d0
	cmp.b	#$b8,d0				; did we reach x position $b8?
	bne	.1				; no jump to .1
	move.b	#%00000001,CORE_CR		; yes, activate irq's for binary insert (each time we reach $b8)
						; this makes sure letters wobble at least 1 time before binary
						; load process starts
	move.b	#$48,d0				; reset x position to $48

.1	move.b	d0,logo_animation

	move.b	#1,d1				; start with sprite 1 (letter 'l')
.2	move.b	d1,VDC_CURRENT_SPRITE
	move.b	#92,VDC_SPRITE_Y_LSB		; base position for each letter

	move.b	VDC_SPRITE_X_LSB,d0		; store x for current sprite in d0
	sub.b	logo_animation,d0		; subtract logo_an x value from d0

	cmp.b	#8,d0
	bcc	.3				; if more than 8, jump to .3

	subq.b	#1,VDC_SPRITE_Y_LSB		; move letter up 1 pixel

.3	addq	#1,d1				; move to next sprite
	cmp.b	#5,d1				; did we reach sprite 5?
	bne	.2				; not yet, jump to .2

.end	movem.l	(sp)+,d0-d1
	move.b	(sp)+,VDC_CURRENT_SPRITE
	rte

exc_trap14_handler
	cmp.b	#0,d1
	bne	.1
	bsr	prng
.1	rte

; ----------------------------------------------------------------------
;
;
;
;
; ----------------------------------------------------------------------
exc_trap15_handler
	cmp.b	#1,d1			; simple, no jump table needed yet
	bne	.1
	;move.b	d1,d0
	bsr	t_putchar
	rte

.1	cmp.b	#2,d1
	bne	.2
	bsr	t_putstring

.2	rte

timer_default_handler
	move.b	#$12,VDC_BG_COLOR.w
	rts


; ----------------------------------------------------------------------
; Subroutine: sound_reset (what to do with analog?)
; ----------------------------------------------------------------------
sound_reset
	movea.l	#SID0_BASE,a0		; clear sids
	moveq	#64-1,d0
.1	clr.b	(a0)+
	dbra	d0,.1
	move.b	#$7f,d0			; set mixer values
	movea.l	#MIX_SID0_LEFT,a0
	moveq	#8-1,d1
.2	move.b	d0,(a0)+
	dbra	d1,.2
	move.b	#$f,SID0_V		; set sid volumes
	move.b	#$f,SID1_V
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
	move.l	#t_cursor_process,VEC_TIMER0.w
	move.l	#timer_default_handler,VEC_TIMER1.w
	move.l	#timer_default_handler,VEC_TIMER2.w
	move.l	#timer_default_handler,VEC_TIMER3.w
	move.l	#timer_default_handler,VEC_TIMER4.w
	move.l	#timer_default_handler,VEC_TIMER5.w
	move.l	#timer_default_handler,VEC_TIMER6.w
	move.l	#timer_default_handler,VEC_TIMER7.w
	rts

; ----------------------------------------------------------------------
; Subroutine: copy_fonts_from_rom (to underlying ram)
; ----------------------------------------------------------------------
copy_fonts_from_rom
	move.b	CORE_ROMS.w,-(sp)
	or.b	#%00000110,CORE_ROMS.w		; make rom font visible to cpu
	movea.l	#$800,a0
	move.l	#$1800-1,d0
.start	move.b	(a0),(a0)+
	dbra	d0,.start
	move.b	(sp)+,CORE_ROMS.w		; restore rom settings
	rts


copy_logo_tile
	movea.l	#logo_tile,a0
	movea.l	#logo,a1		; start at tile $1c
	moveq	#64-1,d0		; 64 bytes = 1 16x16 tile
.1	move.b	(a0)+,(a1)+
	dbra	d0,.1
	rts


; ----------------------------------------------------------------------
; Routine: init_logo (setup sprites 0 - 4 (position, flags, index))
; ----------------------------------------------------------------------
init_logo
	movea.l	#logo_data,a0
	moveq	#0,d0
.1	move.b	d0,VDC_CURRENT_SPRITE
	movea.l	#VDC_SPRITE_X_MSB,a1
.2	move.b	(a0)+,(a1)+
	cmpa.l	#VDC_SPRITE_X_MSB+12,a1
	bne	.2
	addq	#1,d0
	cmpa.l	#logo_data+60,a0	; 5 sprites x 8 = 40
	bne	.1
	rts

; ----------------------------------------------------------------------
; Subroutine: t_clear
; ----------------------------------------------------------------------
t_clear
	movem.l	d2-d4,-(sp)
	move.w	#(T_HPITCH*T_VPITCH)-1,d0
	movea.l	t_chars,a0
	movea.l	t_colors,a1
.1	move.b	#' ',(a0)+
	move.b	cursor_color.w,(a1)+
	dbra	d0,.1

	move.l	#T_HEIGHT-1,d0
	lea	t_link_table,a0
.2	move.b	#$80,(a0)+
	dbra	d0,.2

	clr.w	cursor_pos

	movem.l	(sp)+,d2-d4
	rts


; ----------------------------------------------------------------------
; Subroutine: t_putchar
; Inputs:     d0 contains char to be printed
; Outputs:    -
; Destroyed:  d0,d1,a0,a1
; ----------------------------------------------------------------------
t_putchar
	movea.l	t_chars,a0
	movea.l	t_colors,a1
	move.w	cursor_pos,d1

	cmp.b	#$0a,d0			; check for linefeed
	beq	.lf
	cmp.b	#$0d,d0			; check for carriage return
	beq	.cr
	cmp.b	#$1d,d0			; cursor right
	beq	.right
	cmp.b	#$11,d0			; cursor down
	beq	.down
	cmp.b	#$91,d0			; cursor up
	beq	.up
	cmp.b	#$9d,d0			; cursor left
	beq	.left
	cmp.b	#$08,d0			; backspace
	beq	.bs

	move.b	d0,(a0,d1.w)		; print char
	move.b	cursor_color,(a1,d1.w)	; set color

.right	addq.w	#1,d1			; move cursor one step to the right
	move.w	d1,d0
	andi.w	#%1111111,d0
	cmp.w	#T_WIDTH,d0	; are we at pos 80 or higher?
	blo	.2			; no

.lf	addi.w	#T_HPITCH,d1	; yes, move cursor one line down, followed by carriage return
.cr	andi.w	#%1111111110000000,d1	; cursor to beginning of line (carriage return)

.1	cmp.w	#(T_HPITCH*T_HEIGHT),d1	; check for cursor out of screen
	blo	.2			; no

	subi.w	#T_HPITCH,d1	; move cursor one line up
	move.w	d1,cursor_pos
	bsr	t_add_bottom_row
	rts

.2	move.w	d1,cursor_pos
	rts

.down	addi.w	#T_HPITCH,d1	; yes, move cursor one line down
	bra	.1
	rts

.up	subi.w	#T_HPITCH,d1	; move cursor one line up
	bpl.s	.2
	addi.w	#T_HPITCH,d1	; move cursor one line down
	bra.s	.2

.left	tst.w	d1
	bne.s	.l0
	rts
.l0	move.w	d1,d0
	andi.w	#$7f,d0
	tst.w	d0
	bne	.l1
	addi.w	#(T_WIDTH-1),d1
	bra	.up
.l1	subq.w	#1,d1
	bra.s	.2

.bs	move.w	d1,d0
	beq	.2		; do nothing if we're at position 0 (left top)
	andi.b	#$7f,d0		; the byte in d1 now contains the current column
	bne	.bs0		; it's column 0
	subi.w	#(T_HPITCH-(T_WIDTH-1)),d1
	move.b	#' ',(a0,d1)
	move.b	cursor_color,(a1,d1)
	bra	.2

.bs0	move.l	d2,-(sp)

	move.w	d1,d2
.bs1	move.b	(a0,d2),-1(a0,d2)
	move.b	(a1,d2),-1(a1,d2)
	addq.w	#1,d2
	addq.b	#1,d0
	cmp.b	#T_WIDTH,d0
	bne	.bs1

	subq.w	#1,d2
	move.b	#' ',(a0,d2)
	move.b	cursor_color,(a1,d2)

	move.l	(sp)+,d2

	subq.w	#1,d1

	bra	.2


; ----------------------------------------------------------------------
; Routine: t_putstring (zero terminated)
; Input:   a0 points to first character
; Output:  -
; ----------------------------------------------------------------------
t_putstring
	move.b	(a0)+,d0
	beq	.end
	move.l	a0,-(sp)
	bsr	t_putchar
	movea.l	(sp)+,a0
	bra	t_putstring
.end	rts


; ----------------------------------------------------------------------
; Routine:   t_put_hex_number
; Inputs:    d0 contains de number to print, d1 no of digits to print
; Outputs:   -
; Destroyed: d0,d1,a0,a1
; ----------------------------------------------------------------------
t_put_hex_number
	tst.b	d1		; d1 contains no of digits to print
	beq	.2		; if this is 0, end this function
	subq.b	#1,d1		; reduce number of digits to print by 1
	beq	.1		; if this is 0 (now), only one digit to print
	move.l	d0,-(sp)
	lsr.l	#4,d0
	move.b	d1,-(sp)
	jsr	t_put_hex_number
	move.b	(sp)+,d1
	move.l	(sp)+,d0
.1	move.l	d0,d1
	andi.l	#$f,d1
	lea	hex_values,a0
	move.b	(a0,d1),d0
	jsr	t_putchar
.2	rts


; ----------------------------------------------------------------------
; Routine:   terminal_put_bcd_number
; Inputs:    d0/d1 combined (contain max 10 bcd numbers, 2 in d0, 8 in d1)
;
; Destroyed: d0/d1
; ----------------------------------------------------------------------
terminal_put_bcd_number
	movem.l	d2-d4,-(sp)
	moveq	#0,d4		; flag for first non zero, then print all zeroes
	moveq	#10-1,d3	; max 10 digits
.start	move.b	d0,d2
	lsr.b	#4,d2		; d2.b now holds a number
	bne	.print		; it's a 1 or higher
	tst.b	d4		; it's a 0, but check if it must printed
	beq.s	.cont		; no, go to .cont
.print	moveq	#1,d4
	addi.b	#$30,d2
	movem.l	d0-d1,-(sp)
	move.b	d2,d0
	bsr	t_putchar
	movem.l	(sp)+,d0-d1
.cont	asl.l	d1
	roxl.l	d0
	asl.l	d1
	roxl.l	d0
	asl.l	d1
	roxl.l	d0
	asl.l	d1
	roxl.l	d0
	dbra	d3,.start
	tst.b	d4		; if d4.b is still 0, then nothing has been printed
	bne	.end		; something was printed already
	move.b	#'0',d0		; nothing printed yet, so print 0
	bsr	t_putchar
.end	movem.l	(sp)+,d2-d4
	rts


; ----------------------------------------------------------------------
; Routine:   t_add_bottom_row
; Inputs:    -
; Outputs:   -
; Destroyed: d0,d1,a0,a1
; ----------------------------------------------------------------------
t_add_bottom_row
	movem.l	d2/a2-a3,-(sp)

	movea.l	t_chars,a0
	lea	T_HPITCH(a0),a1
	movea.l	t_colors,a2
	lea	T_HPITCH(a2),a3

	move.w	#(T_HPITCH*(T_HEIGHT-1)),d0	; use terminal size minus lowest row
	lsr.w	#2,d0			; divide by 4
.1	move.l	(a1)+,(a0)+		; do 4 bytes at once
	move.l	(a3)+,(a2)+		; do 4 bytes at once
	subq.w	#1,d0
	bne	.1

; TODO: How about >79????
	move.b	#T_HPITCH,d0	; do last row
	move.b	#' ',d1
	move.b	cursor_color,d2
.2	move.b	d1,(a0)+
	move.b	d2,(a2)+
	subq.b	#1,d0
	bne.s	.2

	movem.l	(sp)+,d2/a2-a3
	rts


t_welcome
	lea	welcome,a0
	jsr	t_putstring
	lea	version,a0
	jsr	t_putstring
	rts


; ----------------------------------------------------------------------
; Subroutine: prng
; see:        https://www.stix.id.au/wiki/Fast_8-bit_pseudorandom_number_generator
; Inputs:     -
; Outputs:    d0 contains random number between 0 and 255
; Destroyed:  d1
; ----------------------------------------------------------------------
prng
	addq.b	#1,prngx.w
	move.b	prnga.w,d0	; d0 = a
	move.b	prngc.w,d1	; d1 = c
	eor.b	d1,d0		; (a ^ c), in d0
	move.b	prngx.w,d1	; d1 = x
	eor.b	d1,d0		; (a ^ c) ^ x, in d0
	move.b	d0,prnga.w	; store result in a

	move.b	prngb.w,d1	; d1 = b
	add.b	d0,d1		; b = b + a
	move.b	d1,prngb.w
	ror.b	#1,d1
	add.b	prngc.w,d1
	eor.b	d1,d0
	move.b	d0,prngc.w
	rts


logo_boot_msg	dc.b	$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a,$0a
		dc.b	"             drop a binary file to boot or hit [esc] to start basic",0
welcome		dc.b	"lime computer system",$0a,0
file_error	dc.b	$0a,$0a,"error: not a valid binary",0
file_loading1	dc.b	$0a,$0a,"  size    from    to",0
file_loading2	dc.b	$0a,"$",0
file_loading3	dc.b	" $",0
file_loading4	dc.b	$0a,$0a," jumping to $",0


logo_data
	dc.b	0,152,0,76,%00000111,0,%00100010,$00,$00,$00,$60,$00 ; icon
	dc.b	0,147,0,92,%00000111,0,%00010001,'l',$00,$00,$10,$00
	dc.b	0,152,0,92,%00000111,0,%00010001,'i',$00,$00,$10,$00
	dc.b	0,158,0,92,%00000111,0,%00010001,'m',$00,$00,$10,$00
	dc.b	0,166,0,92,%00000111,0,%00010001,'e',$00,$00,$10,$00


logo_tile
	dc.b	%00000000,%00000000,%00000000,%00000000
	dc.b	%00000001,%00000000,%00000000,%00000000
	dc.b	%00000111,%10000000,%00000000,%00000000
	dc.b	%00000111,%10100000,%00000000,%00000000
	dc.b	%00011110,%11111000,%00000000,%00000000
	dc.b	%00011110,%10101111,%00000000,%00000000
	dc.b	%00011110,%10101010,%11000000,%00000000
	dc.b	%00011110,%10101111,%10110000,%00000000
	dc.b	%00011110,%11111010,%11101100,%00000000
	dc.b	%00000111,%10101010,%11101110,%00000000
	dc.b	%00000111,%10101011,%10101011,%10000000
	dc.b	%00000001,%11101011,%10101011,%10100000
	dc.b	%00000000,%01111110,%10101010,%11110100
	dc.b	%00000000,%00010111,%11111111,%01010000
	dc.b	%00000000,%00000001,%01010101,%00000000
	dc.b	%00000000,%00000000,%00000000,%00000000


hex_values
	dc.b	"0123456789abcdef"

	cnop	0,2		; alignment
	include "basic.s"


end_of_rom
