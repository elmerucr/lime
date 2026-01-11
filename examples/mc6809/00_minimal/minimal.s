; preamble (5 bytes)
	fcb	$00		; it's a decb for mc6809, and first chunk
	fcb	$00,$05		; this chunk contains 6 bytes
	fcb	$60,$00		; this chunk loads at $6000

; chunk data (5 bytes) contains two instructions that change the background color
	fcb	$7c,$04,$04	;	1	inc	$404
	fcb	$20,$fb		;		bra	1b

; preamble (5 bytes)
	fcb	$00		; second chunk
	fcb	$00,$04		; this chunk contains 4 bytes
	fcb	$60,$10		; this chunk loads at $6010

; chunk data (4 bytes)
	fcb	$de,$ad,$be,$ef	; deadbeef :-)

; postamble (5 bytes)
	fcb	$ff		;
	fcb	$00,$00		; always the same
	fcb	$60,$00		; start code at $6000
