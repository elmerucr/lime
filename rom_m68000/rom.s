; ----------------------------------------------------------------------
; rom.s (assembles with vasmm68k_mot)
; lime
;
; Copyright © 2025 elmerucr. All rights reserved.
; ----------------------------------------------------------------------

	org	$00010000	; rom based at $10000

	dc.l	$01000000	; initial ssp
	dc.l	_start		; reset vector

	dc.b	"rom m68000 0.1 20250826"

_start	move.l	#$deadbeef,A0
	move.l	A0,USP
.1	move.b	$404.w,D0
	addq.b	#$1,D0
	move.b	D0,$404.w
	jmp	.1
