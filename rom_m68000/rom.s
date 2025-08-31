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

	align	2

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
	bra	exc_addr_error			; TODO: screendump when this happens?

logo_data
	dc.b	112,64,%111,0,$1c		; icon top left
	dc.b	120,64,%111,0,$1d		; icon top right
	dc.b	112,72,%111,0,$1e		; icon bottom left
	dc.b	120,72,%111,0,$1f		; icon bottom right
	dc.b	107,80,%111,0,$6c		; l
	dc.b	112,80,%111,0,$69		; i
	dc.b	118,80,%111,0,$6d		; m
	dc.b	126,80,%111,0,$65		; e
