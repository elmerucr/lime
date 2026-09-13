;-----------------------------------------------------------------------
;
;
;-----------------------------------------------------------------------
b_start	rs.l	1	;rsset	$....	no need, rs continues
b_end	rs.l	1
;-----------------------------------------------------------------------


b_cold_start
	move.l	#$20000,b_start	; start of basic memory
	move.l	#$30000,b_end
	movea.l	b_start,A0
	clr.l	(A0)		; clear / new
	lea	b_greeter,A0
	bsr	terminal_putstring
	rts


b_warm_start
	lea	b_prompt,A0
	bsr	terminal_putstring
	rts


;-----------------------------------------------------------------------
; Subroutine: b_process_buffer
; Inputs:
; Outputs:
;
;-----------------------------------------------------------------------
b_process_buffer
	move.l	A2,-(SP)
	lea	terminal_buf_1,A0
	bsr	b_remove_spaces
	bsr	b_get_dec_number
	tst.b	D1			; check no of digits
	beq.s	.dm			; zero = NAN --> direct mode

					; store line mode
	lea	terminal_buf_2,A1	; this is where we put our result
	move.l	D0,(A1)+		; store line number in buffer
	movea.l	A1,A2			; at this pos, no of chars of basic line will be stored
	adda.l	#4,A1			; move pointer

	move.l	D0,-(SP)		; temp hack; print the hex number
	move.b	#$a,D0			; print newline
	moveq	#1,D1
	trap	#15
	move.l	(SP),D0
	moveq	#8,D1
	bsr	terminal_put_hex_number

	;move.l	D0,-(SP)
	move.b	#$a,D0			; print newline
	moveq	#1,D1
	trap	#15
	move.l	(SP)+,D0
	bsr	b_double_dabble
	bsr	terminal_put_bcd_number
	bsr	b_remove_spaces
	movea.l	(SP)+,A2
	rts

					; direct mode
.dm	;
	;
	;
	movea.l	(SP)+,A2
	rts

; ----------------------------------------------------------------------
; Routine: b_double_dabble (bin to bcd)
; Inputs:  D0.l 32bits unsigned value
; Outputs: D0-D1 combined holding 10 bcd's, big endian order
; ----------------------------------------------------------------------
b_double_dabble
	movem.l	D2-D3,-(SP)
	moveq	#32-1,D3	; counter for 32 shifts
	move.l	D0,D2		; D2 now holds input value
	moveq	#0,D0
	moveq	#0,D1

.start
	move.l	D1,-(SP)	; check individual numbers if >=5
	bsr.s	chk_nums
	move.l	(SP)+,D1
	exg	D0,D1
	move.l	D1,-(SP)
	bsr.s	chk_nums
	move.l	(SP)+,D1
	exg	D0,D1

	asl.l	D2
	roxl.l	D1
	roxl.l	D0

	dbra	D3,.start

	movem.l	(SP)+,D2-D3
	rts

chk_nums
	bsr.s	chk_nums_h1
	swap	D0
	bsr.s	chk_nums_h1
	swap	D0
	rts
chk_nums_h1
	move.l	D0,D1
	andi.b	#$0f,D1
	cmp.b	#$05,D1
	blo	.n1
	addi.b	#$3,D0
.n1	andi.w	#$0f00,D1
	cmp.w	#$500,D1
	blo	.n2
	addi.w	#$300,D0
.n2	move.l	D0,D1
	andi.b	#$f0,D1
	cmp.b	#$50,D1
	blo	.n3
	addi.b	#$30,D0
.n3	andi.w	#$f000,D1
	cmp.w	#$5000,D1
	blo	.end
	addi.w	#$3000,D0
.end	rts

;-----------------------------------------------------------------------
; Subroutine: b_remove_spaces
; Inputs:     A0 pointer to text in buffer
; Outputs:    A0 point to first item not equal to space ($20)
;-----------------------------------------------------------------------
b_remove_spaces
	cmp.b	#' ',(A0)
	bne.s	.end
	addq.l	#1,A0
	bra.s	b_remove_spaces
.end	rts


;-----------------------------------------------------------------------
; Subroutine: b_get_dec_number
; Inputs:     A0 points to ascii
; Outputs:    D0.l contains the number (32 bits), D1 contains the number
;             of digits it consumed (so is 0 when NAN)
;             A0 remains (1) the same if NAN or (2) points after number
;-----------------------------------------------------------------------
b_get_dec_number
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


b_greeter	dc.b	$a,"basic v0.0 <<this is WIP>>",0
b_prompt	dc.b	$a,"ready.",$a,0
