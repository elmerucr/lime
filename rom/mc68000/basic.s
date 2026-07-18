;-----------------------------------------------------------------------
b_start	rs.l	1	;rsset	$....	no need, rs continues
b_end	rs.l	1
;-----------------------------------------------------------------------


b_cold_start
	move.l	$20000,b_start	; start of basic memory
	move.l	$30000,b_end
	rts


b_warm_start
	lea	b_prompt,A0
	bsr	terminal_putstring
	rts


b_process_buffer
	lea	terminal_buf_1,A0
	bsr.s	b_remove_spaces
	move.b	(A0)+,D0
	subi.b	#'0',D0
	cmp.b	#9,D0
	bhi.s	.end		; not a decimal number
	lea	conf,A0
	bsr	terminal_putstring
.end	rts


; ----------------------------------------------------------------------
; Subroutine: b_remove_spaces
; Inputs:     A0 pointer to text in buffer
; Outputs:    A0 point to first item not #$20 (char OR 0 = EOL), D0.b contains content at A0
; ----------------------------------------------------------------------
b_remove_spaces
	move.b	#TERMINAL_WIDTH,D1
	subq.b	#1,D1
.start	move.b	(A0),D0
	cmp.b	#' ',D0
	beq.s	.next		; it is a space
	rts
.next	addq.l	#1,A0
	dbra	D1,.start	; TODO???? how to limit to a maximum if not found?


conf	dc.b	$a,"this starts with a number",0
b_prompt
	dc.b	$a,"Ready.",$a,0
