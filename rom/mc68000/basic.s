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
	bsr.s	b_get_number
	tst.b	D1
	beq.s	.end
	move.l	D0,-(SP)
	move.b	#$a,D0
	moveq	#1,D1
	trap	#15
	move.l	(SP)+,D0
	moveq	#8,D1
	bsr	terminal_put_hex_number
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


; ----------------------------------------------------------------------
; Subroutine: b_get_number (number in decimal)
; Inputs:     A0 points to text
; Outputs:    D0.l contains the number (32 bits), D1 contains the number
;             of digits (so is 0 when NAN)
;             A0 remains (1) the same if NAN or (2) points to next char
; ----------------------------------------------------------------------
b_get_number
	move.l	D2,-(SP)
	clr.l	D0		; will contain end result
	clr.l	D1		; is zero if NAN
.start	cmp.b	#'0',(A0)
	blo	.end
	cmp.b	#'9',(A0)
	bhi	.end
	move.l	D0,D2		; times ten: first save D0 into D2
	asl.l	#2,D0		; double D0
	add.l	D2,D0		; now D0 is times 5
	asl.l	#1,D0		; double D0 (times 10)
	move.b	(A0)+,D2
	andi.l	#$f,D2
	add.l	D2,D0
	addq.l	#1,D1
	bra	.start
.end	move.l	(SP)+,D2
	rts


b_prompt
	dc.b	$a,"ready.",$a,0
