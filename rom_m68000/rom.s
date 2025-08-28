; ----------------------------------------------------------------------
; rom.s (assembles with vasmm68k_mot)
; lime
;
; Copyright © 2025 elmerucr. All rights reserved.
; ----------------------------------------------------------------------

	org	$00010000	; rom based at $10000

	dc.l	$01000000	; initial ssp
	dc.l	_start		; reset vector

	dc.b	"rom m68000 0.1 20250828"

_start	lea	exc_addr_error,A0
	move.l	A0,$000c.w		; exception for address error
	move.l	#$00010000,A0
	move.l	A0,USP
	move.w	#$0200,SR
	move.l	#exc_addr_error,-(A7)
.1	move.b	$404.w,D0
	addq.b	#$1,D0
	move.b	D0,$404.w
	jmp	.1

exc_addr_error
	bra	exc_addr_error