; ----------------------------------------------------------------------
; rom.s (assembles with vasmm68k_mot)
; lime
;
; Copyright © 2025 elmerucr. All rights reserved.
; ----------------------------------------------------------------------

	org	$00010000

	dc.l	$0000c000	; initial ssp
	dc.l	_start		; reset vector

	dc.b	"rom m68000 0.1 20250824"

_start	addq.b	#$1,$404.w
	jmp	_start
