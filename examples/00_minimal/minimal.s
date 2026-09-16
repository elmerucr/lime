; preamble (7 bytes)
	dc.b	$01		; it's a decb for mc68000, and first chunk
	dc.l	$00000006	; this chunk contains 6 bytes
	dc.l	$00020000	; this chunk loads at $20000

; chunk data (6 bytes) contains two instructions that change the background color
	dc.b	$52,$38,$04,$04	; .1      addq.b  #1,$404.w
	dc.b	$60,$fa		;         bra     .1

; preamble (7 bytes)
	dc.b	$01		; second chunk
	dc.l	$00000004	; this chunk contains 4 bytes
	dc.l	$00020010	; this loads at $20010

; chunk data (4 bytes)
	dc.b	$de,$ad,$be,$ef	; deadbeef :-)

; postamble (7 bytes)
	dc.b	$fe		; start
	dc.l	$00000000	; always the same
	dc.l	$00020000	; start code at $20000
