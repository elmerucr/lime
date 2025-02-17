; ----------------------------------------------------------------------
; rom.s (compiles with asm6809)
; lime
;
; Copyright (C)2025 elmerucr
; ----------------------------------------------------------------------

	setdp	$00		; assembler now assumes dp = $00 and
				; uses dp addressing when appropriate

	org	$ff00

	fcn	"lime rom v0.2 20250205"
reset
	lds	#$0200		; sets system stackpointer + enables nmi
	ldu	#$ff00		; sets user stackpointer

	lda	$00		; make font visible to cpu
	ora	#%00000010
	sta	$00

	ldx	#$1000		; copy font from rom to ram
1	lda	,x
	sta	,x+
	cmpx	#$2000
	bne	1b

	lda	$00		; only rom remains visible to cpu
	anda	#%11111101
	sta	$00

; ----------------------------------------------------------------------
; place logo
; ----------------------------------------------------------------------
	clrb			; holds current sprite
	ldx	#logo_data	; x points to start of data
1	stb	$0407		; set active sprite
	ldy	#$0418		; y points to sprite registers
2	lda	,x+		; copy data
	sta	,y+
	cmpy	#$041c		; did we copy 4 values?
	bne	2b		; not yet, continue at 2
	incb			; we did, set next active sprite
	cmpx	#logo_data+32	; did we reach end of data?
	bne	1b		; no, continue at 1

; ---------------
; do jump vectors
; ---------------
	ldx	#exc_irq
	stx	$0200

3	jsr	test
	bra	3b		; endless loop

test
	lda	#$34
	tfr	a,b
	addb	#$01
	tfr	b,a
	rts

logo_data
	fcb	112,64,%111,$1c	; icon top left
	fcb	120,64,%111,$1d	; icon top right
	fcb	112,72,%111,$1e	; icon bottom left
	fcb	120,72,%111,$1f	; icon bottom right

	fcb	107,80,%111,$6c	; l
	fcb	112,80,%111,$69	; i
	fcb	118,80,%111,$6d	; m
	fcb	126,80,%111,$65	; e

exc_irq
	rti

1	jmp	[$0200]

	org	$fff0
vectors
	fdb	$0000
	fdb	$0000
	fdb	$0000
	fdb	$0000
	fdb	1b		; irq vector
	fdb	$0000
	fdb	$0000
	fdb	reset		; reset vector
