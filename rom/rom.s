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

logo	clrb			; holds current sprite
	ldx	#data		; x points to start of data
1	stb	$0207		; set active sprite
	ldy	#$0218		; y points to sprite registers
2	lda	,x+		; copy data
	sta	,y+
	cmpy	#$021c		; did we copy 4 values?
	bne	2b		; not yet, continue at 2
	incb			; we did, set next active sprite
	cmpx	#data+32	; did we reach end of data?
	bne	1b		; no, continue at 1

3	bra	3b		; endless loop

data
	fcb	112,64,%111,$1c	; icon top left
	fcb	120,64,%111,$1d	; icon top right
	fcb	112,72,%111,$1e	; icon bottom left
	fcb	120,72,%111,$1f	; icon bottom right

	fcb	107,80,%111,$6c	; l
	fcb	112,80,%111,$69	; i
	fcb	118,80,%111,$6d	; m
	fcb	126,80,%111,$65	; e


	org	$fff0
vectors
	fdb	$0000
	fdb	$0000
	fdb	$0000
	fdb	$0000
	fdb	$0000
	fdb	$0000
	fdb	$0000
	fdb	start		; reset vector
