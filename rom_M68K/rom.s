	org	$00010000

	dc.l	$0000c000	; initial ssp
	dc.l	_start		; reset vector

_start	move.b	#$12,-(A7)
	jmp	_start
