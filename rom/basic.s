;-----------------------------------------------------------------------
;
;
;-----------------------------------------------------------------------
b_start	rs.l	1	;rsset	$....	no need, rs continues
b_end	rs.l	1
;-----------------------------------------------------------------------

; ----------------------------------------------------------------------
; Subroutine: b_cold_start
; Inputs:
; Outputs:
; ----------------------------------------------------------------------
b_cold_start
	move.l	#$20000,b_start	; start of basic memory
	move.l	#$30000,b_end
	movea.l	b_start,A0
	clr.l	(A0)		; clear / new
	lea	b_greeter,A0
	bsr	t_putstring
	rts


; ----------------------------------------------------------------------
; Subroutine: b_warm_start
; Inputs:
; Outputs:
; ----------------------------------------------------------------------
b_warm_start
	lea	b_prompt,A0
	bsr	t_putstring
	rts


;-----------------------------------------------------------------------
; Subroutine: b_process_buffer
; Inputs:
; Outputs:
;
;-----------------------------------------------------------------------
b_process_buffer
	movem.l	D2/A2,-(SP)
	moveq	#0,D2			; D2 serves as flag for line mode
	lea	t_buf_1,A0
	bsr	b_remove_spaces		; remove preceding spaces
	bsr	b_get_dec_number
	lea	t_buf_2,A1	; this is where we put our result
	tst.b	D1			; check no of digits
	beq.s	.1			; it's zero (so direct mode)
	moveq	#1,D2			; 1 or more, set flag = line mode

	move.l	D0,(A1)+		; store line number in buffer
	movea.l	A1,A2			; at this pos A2, no of chars of basic line will be stored
	adda.l	#2,A1			; move pointer (save space 2 bytes)

.1	bsr	b_remove_spaces		; remove potential space between line number and statements
	bsr.s	b_tokenize

	suba.l	#1,A1			; remove trailing spaces, but A1 for sure points at a zero when coming back from b_tokenize
.2	cmp.b	#' ',-(A1)
	beq	.2

	adda	#1,A1			; put end of line marker
	move.b	#0,(A1)
	movem.l	(SP)+,D2/A2
	rts


; ----------------------------------------------------------------------
; Subroutine: b_tokenize
; Inputs:     A0 contains read pointer, A1 write pointer
; ----------------------------------------------------------------------
b_tokenize
	movem.l	D2,-(SP)
	clr.b	D2		; D2 is flag for string copy mode

.start
	move.b	(A0)+,D0
	beq	.done		; if null ($00), end of line (regardless D2 flag)

	cmp.b	#$22,D0		; check start/stop string mode
	bne	.1
	eor.b	#1,D2

.1	tst.b	D2
	bne	.copy_raw	; we're in string mode


	; check for letter
	cmp.b	#'a',D0
	blo	.copy_raw
	cmp.b	#'z',D0
	bhi	.copy_raw

	; it's a letter: see if there's a match
	subq	#1,A0		; make sure A0 points again to start of potential keyword
	bsr	b_check_keyword
	tst.b	D0		; did we find a token? D0 != 0
	bne	.write_token	; yes, go write it

	; if it wasn't, restore char and copy as raw text
	move.b	(A0)+,D0

.copy_raw
	move.b	D0,(A1)+
	bra	.start

.write_token
	move.b	D0,(A1)+
	bra	.start

.done	move.b	D0,(A1)+
	movem.l	(SP)+,D2
	rts


; ----------------------------------------------------------------------
; Subroutine: b_check_keyword
; Input:      A0 pointer to input string
; Outputs:    D0 is $00 if no token found or token byte ($80-$ff) if found
;             A0 untouched when no match, A0 points to after keyword if match
; Destroyed:  D1
; ----------------------------------------------------------------------
b_check_keyword
	movem.l	A2-A3,-(SP)
	lea	b_token_table,A2

.loop_keywords
	tst.b	(A2)		; check the first character from the keyword
	beq	.not_found	; if $00, we hit end of table
	movea.l	A0,A3		; leave A0, use A3 for now

.compare_loop
	move.b	(A2)+,D1
	beq	.match_found	; we came this far and character = null = match found
	move.b	(A3)+,D0	; get character from input buffer
	cmp.b	D0,D1		; compare chars case sensitive
	beq	.compare_loop

.skip_keyword
	tst.b	(A2)+		; test table byte and advance pointer
	bne	.skip_keyword	; repeat as it wasn't zero
	addq.l	#1,A2		; it was zero, so advance pointer to skip token value
	bra	.loop_keywords	; try next keyword

.match_found
	move.b	(A2),D0		; load token value into D0
	movea.l	A3,A0		; advance original A0 pointer
	movem.l	(SP)+,A2-A3	; cleanup and return
	rts

.not_found
	moveq	#0,D0		; no match, A0 not advanced
	movem.l	(SP)+,A2-A3	; cleanup and return
	rts


; ----------------------------------------------------------------------
; Subroutine: b_double_dabble (bin to bcd)
; Inputs:     D0.l 32bits unsigned value
; Outputs:    D0-D1 combined holding 10 bcd's, big endian order
; ----------------------------------------------------------------------
b_double_dabble
	movem.l	D2-D3,-(SP)
	moveq	#32-1,D3	; counter for 32 shifts
	move.l	D0,D2		; D2 now holds input value
	moveq	#0,D0
	moveq	#0,D1

.start
	move.l	D1,-(SP)	; check individual numbers if >=5
	bsr.s	chk_nm
	move.l	(SP)+,D1
	exg	D0,D1
	move.l	D1,-(SP)
	bsr.s	chk_nm
	move.l	(SP)+,D1
	exg	D0,D1

	asl.l	D2
	roxl.l	D1
	roxl.l	D0

	dbra	D3,.start

	movem.l	(SP)+,D2-D3
	rts

chk_nm
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

.n1
	andi.w	#$0f00,D1
	cmp.w	#$500,D1
	blo	.n2
	addi.w	#$300,D0

.n2
	move.l	D0,D1
	andi.b	#$f0,D1
	cmp.b	#$50,D1
	blo	.n3
	addi.b	#$30,D0

.n3
	andi.w	#$f000,D1
	cmp.w	#$5000,D1
	blo	.end
	addi.w	#$3000,D0

.end
	rts


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

.end
	rts


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

.start
	cmp.b	#'0',(A0)
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

.end
	move.l	(SP)+,D2
	rts


b_greeter
	dc.b	$a,"basic v0.0 <work in progress>",0

b_prompt
	dc.b	$a,"ready.",$a,0

b_token_table
	dc.b	"goto",0,$80
	dc.b	"list",0,$81
	dc.b	"peek",0,$82
	dc.b	"poke",0,$83
	dc.b	"print",0,$84
	dc.b	"rem",0,$85
	dc.b	"run",0,$86
	dc.b	0               ; Single $00 marks the absolute end of the table
