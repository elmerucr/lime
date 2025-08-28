; ----------------------------------------------------------------------
; rom.s (assembles with vasmm68k_mot)
; lime
;
; Copyright © 2025 elmerucr. All rights reserved.
; ----------------------------------------------------------------------

	org	$00010000	; rom based at $10000

	dc.l	$01000000	; initial ssp at end of ram
	dc.l	_start		; reset vector

	dc.b	"rom m68000 0.1 20250828"

_start	move.l	#exc_addr_error,$000c.w		; exception for address error
	move.l	#$00010000,A0			; set usp
	move.l	A0,USP
	move.w	#$0200,SR			; cpu state
	move.l	#exc_addr_error,-(SP)		; test usp

.1	move.b	$404.w,D0			; change background colour loop
	addq.b	#$1,D0
	move.b	D0,$404.w
	jmp	.1

exc_addr_error
	bra	exc_addr_error
