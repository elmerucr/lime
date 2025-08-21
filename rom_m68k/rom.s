; ----------------------------------------------------------------------
; rom.s (assembles with vasmm68k_mot)
; lime
;
; Copyright © 2025 elmerucr. All rights reserved.
; ----------------------------------------------------------------------

	org	$00010000

	dc.l	$0000c000	; initial ssp
	dc.l	_start		; reset vector

	dc.b	"rom 0.1 20250821"

_start	addq.b	#$1,$400
	jmp	_start
