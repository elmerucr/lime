; ----------------------------------------------------------------------
; rom.s (compiles with asm6809)
; lime
;
; Copyright (C)2025 elmerucr
; ----------------------------------------------------------------------

	setdp	$00		; assembler now assumes dp = $00 and
				; uses dp addressing when appropriate

	org	$ff00

	fcn	"lime rom v0.2 20250202"
start
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

2	jmp	2b		; endless loop

data
	fcb	112,64,%00000111,$1c	; icon top left
	fcb	120,64,%00000111,$1d	; icon top right
	fcb	112,72,%00000111,$1e	; icon bottom left
	fcb	120,72,%00000111,$1f	; icon bottom right

	fcb	107,80,%00000111,$6c	; l
	fcb	112,80,%00000111,$69	; i
	fcb	118,80,%00000111,$6d	; m
	fcb	126,80,%00000111,$65	; e

	org	$fff0		; vectors
vectors
	fdb	$0000
	fdb	$0000
	fdb	$0000
	fdb	$0000
	fdb	$0000
	fdb	$0000
	fdb	$0000
	fdb	start		; reset vector
