;-----------------------------------------------------------------------
; rom.s (assembles with asm6809)
; lime
;
; Copyright © 2025 elmerucr. All rights reserved.
;-----------------------------------------------------------------------

; rom 0.9 20250603
; adding rnd routine

; rom 0.8 20250420
;
;

		include	"definitions.i"

		setdp	$00

logo_animation	equ	$00
execution_addr	equ	$01
binary_ready	equ	$03
rnda		equ	$fc
rndb		equ	$fd
rndc		equ	$fe
rndx		equ	$ff

		setdp	$00		; assembler now assumes dp = $00 and
					; uses dp addressing when appropriate

		org	$fc00

		fcn	"rom_mc6809 0.10 20250816"
reset		lds	#$0200		; sets system stackpointer + enables nmi
		ldu	#$fe00		; sets user stackpointer

		lda	CORE_ROMS	; make font visible to cpu
		ora	#%00000100
		sta	CORE_ROMS

		ldx	#VDC_TILESET1	; copy font from rom to ram
1		lda	,x
		sta	,x+
		cmpx	#VDC_TILESET1+$1000
		bne	1b

		lda	CORE_ROMS		; turn off font rom
		anda	#%11111011
		sta	CORE_ROMS

		ldx	#logo_chars
		ldy	#$11c0
2		lda	,x+
		sta	,y+
		cmpx	#logo_chars+64
		bne	2b

; init logo
		ldx	#logo_data		; x points to start of logo data
		clrb				; b holds current sprite
3		stb	VDC_CURRENT_SPRITE	; set active sprite
		ldy	#VDC_SPRITE_X		; y points to start of sprite registers
4		lda	,x+			; copy data
		sta	,y+
		cmpy	#VDC_SPRITE_X+5		; did we copy 5 values?
		bne	4b			; not yet, continue at 2
		incb				; we did, set next active sprite
		cmpx	#logo_data+40		; did we reach end of data?
		bne	3b			; no, continue at 1

; set variable for letter wobble
		lda	#$40
		sta	logo_animation

; set jump vectors
		ldx	#exc_irq
		stx	VECTOR_IRQ_INDIRECT
		ldx	#timer_interrupt
		ldy	#VECTOR_TIMER0_INDIRECT
1		stx	,y++
		cmpy	#VECTOR_TIMER0_INDIRECT+16
		bne	1b
		ldx	#vdc_interrupt
		stx	VECTOR_VDC_INDIRECT

; set raster irq on scanline 159
		lda	#$9f
		sta	VDC_IRQ_SCANLINE
		lda	#%00000001
		sta	VDC_CR		; enable irq's for vdc

; cleary binary ready flag
		clr	binary_ready

; sound
		bsr	sound_reset

; enable irq's
		andcc	#%11101111

; clear rnd variables
		clr	rnda
		clr	rndb
		clr	rndc
		clr	rndx

; "main" loop
loop		sync
		lda	binary_ready
		beq	loop

; disable logo and jump to start of binary
_jump		orcc	#%00010000		; disable irq's
		clr	CORE_CR			; no irq when new bin inserted
		clr	VDC_CR
		clrb
1		stb	VDC_CURRENT_SPRITE
		lda	VDC_SPRITE_FLAGS0
		anda	#%11111110
		sta	VDC_SPRITE_FLAGS0
		incb
		cmpb	#$08
		bne	1b
		jmp	[execution_addr]

sound_reset	pshu	y,b,a
		ldb	#$40
		ldy	#SID0_F			; start of sound (sid 0)
1		clr	,y+
		decb
		bne	1b
		lda	#$7f			; mixer at half volume
		ldb	#$08			; 8 mixing registers in total
		ldy	#MIX_SID0_LEFT		; start of io mixer
2		sta	,y+
		decb
		bne	2b
		lda	#$0f			; set sid volumes to max
		sta	SID0_V			; sid 0 volume
		sta	SID1_V			; sid 1 volume
		pulu	y,b,a
		rts

logo_data	fcb	112,64,%111,0,$1c		; icon top left
		fcb	120,64,%111,0,$1d		; icon top right
		fcb	112,72,%111,0,$1e		; icon bottom left
		fcb	120,72,%111,0,$1f		; icon bottom right
		fcb	107,80,%111,0,$6c		; l
		fcb	112,80,%111,0,$69		; i
		fcb	118,80,%111,0,$6d		; m
		fcb	126,80,%111,0,$65		; e

exc_irq		lda	TIMER_SR		; load timer status register
		beq	exc_vdc
		ldx	#VECTOR_TIMER0_INDIRECT	; it is one of the timers, load x with 1st vector indirect
		lda	#%00000001
exc_test_tim	bita	TIMER_SR
		beq	exc_next_tim
		sta	TIMER_SR		; acknowledge interrupt
		jmp	[,x]
exc_next_tim	asla
		beq	exc_irq_end
		leax	2,x			; load x with address of next vector
		bra	exc_test_tim
exc_vdc		lda	VDC_SR
		beq	exc_core
		sta	VDC_SR
		ldx	#VECTOR_VDC_INDIRECT
		jmp	[,x]
exc_core	lda	CORE_SR			; triggered when binary inserted
		beq	exc_irq_end
		sta	CORE_SR
		jmp	core_interrupt
exc_irq_end	rti

vdc_interrupt	lda	VDC_CURRENT_SPRITE
		pshs	a
		lda	logo_animation
		inca
		cmpa	#$90
		bne	1f			; didn't reach #$90
		lda	#%00000001		; we did reach #$90
		sta	CORE_CR			; activate irq's for binary insert
						; this makes sure letters wobble at least 1 time
		clra

1		sta	logo_animation

		ldb	#$04			; set current sprite to 4
2		stb	VDC_CURRENT_SPRITE

		lda	#80			; set default y value
		sta	VDC_SPRITE_Y

		lda	VDC_SPRITE_X		; load its x register
		suba	logo_animation		; subtract the ani var
		suba	#$08
		bhi	3f			; if difference larger than 8 keep its y value
		dec	VDC_SPRITE_Y
3		incb
		cmpb	#$08
		bne	2b

		lda	CORE_INPUT0		; use controller input to change screen background color
		sta	VDC_BG_COLOR
		puls	a
		sta	VDC_CURRENT_SPRITE
		rti

timer_interrupt	rti

; handle loading of binary and jump to code if successful
core_interrupt	lda	CORE_FILE_DATA		; first value $00?
		bne	core_int_end		; no
1		lda	CORE_FILE_DATA		; yes, start chunk
		ldb	CORE_FILE_DATA
		tfr	d,y			; y holds size
		lda	CORE_FILE_DATA
		ldb	CORE_FILE_DATA
		tfr	d,x			; x holds memory location
2		lda	CORE_FILE_DATA		; get a byte
		sta	,x+
		leay	-1,y
		bne	2b			; finished?
		lda	CORE_FILE_DATA
		beq	1b			; is $00 start another chunk
		cmpa	#$ff
		bne	core_int_end		; not equal
		lda	CORE_FILE_DATA
		bne	core_int_end		; not equal to zero
		lda	CORE_FILE_DATA
		bne	core_int_end		; not equal to zero
		lda	CORE_FILE_DATA
		ldb	CORE_FILE_DATA		; d now contains execution address
		std	execution_addr		; store it
		inc	binary_ready		; %00000001
core_int_end	rti

; prng / rnd routine, value contained in ac
; see:
; c version:
;
;
;
;
;
rnd_impl	inc	rndx
		lda	rnda
		eora	rndc
		eora	rndx
		sta	rnda
		adda	rndb
		sta	rndb
		lsra
		adda	rndc
		eora	rnda
		sta	rndc
		rts

1		jmp	[VECTOR_IRQ_INDIRECT]

logo_chars	fcb	%00000000,%00000000	; tile 1 (icon upper left)
		fcb	%00000001,%00000000
		fcb	%00000111,%10000000
		fcb	%00000111,%10100000
		fcb	%00011110,%11111000
		fcb	%00011110,%10101111
		fcb	%00011110,%10101010
		fcb	%00011110,%10101111

		fcb	%00000000,%00000000	; tile 2 (icon upper right)
		fcb	%00000000,%00000000
		fcb	%00000000,%00000000
		fcb	%00000000,%00000000
		fcb	%00000000,%00000000
		fcb	%00000000,%00000000
		fcb	%11000000,%00000000
		fcb	%10110000,%00000000

		fcb	%00011110,%11111010	; tile 3 (icon bottom left)
		fcb	%00000111,%10101010
		fcb	%00000111,%10101011
		fcb	%00000001,%11101011
		fcb	%00000000,%01111110
		fcb	%00000000,%00010111
		fcb	%00000000,%00000001
		fcb	%00000000,%00000000

		fcb	%11101100,%00000000	; tile 4 (icon bottom right)
		fcb	%11101110,%00000000
		fcb	%10101011,%10000000
		fcb	%10101011,%10100000
		fcb	%10101010,%11110100
		fcb	%11111111,%01010000
		fcb	%01010101,%00000000
		fcb	%00000000,%00000000

		org	$fff0 - (vectors - rnd)

rnd		jmp	rnd_impl

vectors		fdb	$0000
		fdb	$0000
		fdb	$0000
		fdb	$0000
		fdb	1b		; irq vector
		fdb	$0000
		fdb	$0000
		fdb	reset		; reset vector
