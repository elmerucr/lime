; ----------------------------------------------------------------------
; rom.s (compiles with asm6809)
; lime
;
; Copyright (C)2025 elmerucr
; ----------------------------------------------------------------------

	setdp	$00		; assembler now assumes dp = $00 and will
				; use dp addressing when appropriate

	org	$ff00

	fcn	"lime rom v0.1 20250202"
start
	lds	#$0200		; init system stackpointer, enables nmi as well
	ldu	#$ff00		; init user stackpointer

	lda	$00
	ora	#%00000010
	sta	$00		; make font + sys_rom visible to cpu

	ldx	#$1000
1	lda	,x
	sta	,x+
	cmpx	#$2000
	bne	1b

	lda	$00
	anda	#%11111101
	sta	$00	; only sys_rom visible now

2	jmp	2b


	org	$fff0
vectors
	fdb	$0000	;
	fdb	$0000	;
	fdb	$0000	;
	fdb	$0000	;
	fdb	$0000	;
	fdb	$0000	;
	fdb	$0000	;
	fdb	start	; reset vector
