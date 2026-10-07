;-----------------------------------------------------------------------
; basic.s (included by rom.s)
; lime
;
; Copyright © 2025-2026 elmerucr. All rights reserved.
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
	movea.l	b_start,a0
	clr.l	(a0)		; clear / new
	lea	b_greeter,a0
	bsr	t_putstring
	rts


; ----------------------------------------------------------------------
; Subroutine: b_warm_start
; Inputs:
; Outputs:
; ----------------------------------------------------------------------
b_warm_start
	lea	b_prompt,a0
	bsr	t_putstring
	rts


;-----------------------------------------------------------------------
; Subroutine: b_process_buffer
; Inputs:
; Outputs:
;
;-----------------------------------------------------------------------
b_process_buffer
	movem.l	d2/a2,-(sp)
	moveq	#0,d2			; d2 serves as flag for line mode
	lea	t_buf_1,a0
	bsr	b_remove_spaces		; remove preceding spaces
	bsr	b_get_dec_number
	lea	t_buf_2,a1		; this is where we put our result
	tst.b	d1			; check no of digits
	beq.s	.1			; it's zero (so direct mode)
	moveq	#1,d2			; 1 or more, set flag = line mode

	move.l	d0,(a1)+		; store line number in buffer
	movea.l	a1,a2			; at this pos a2, no of chars of basic line will be stored
	adda.l	#2,a1			; move pointer (save space 2 bytes)

.1	bsr	b_remove_spaces		; remove potential space between line number and statements
	bsr.s	b_tokenize

	suba.l	#1,a1			; remove trailing spaces, but A1 for sure points at a zero when coming back from b_tokenize
.2	cmp.b	#' ',-(a1)
	beq	.2

	adda	#1,a1			; put end of line marker
	move.b	#0,(a1)
	movem.l	(sp)+,d2/a2
	rts


; ----------------------------------------------------------------------
; Subroutine: b_tokenize
; Inputs:     a0 contains read pointer, a1 write pointer
; ----------------------------------------------------------------------
b_tokenize
	movem.l	d2,-(sp)
	clr.b	d2		; D2 is flag for string copy mode

.start
	move.b	(a0)+,d0
	beq	.done		; if null ($00), end of line (regardless D2 flag)

	cmp.b	#$22,d0		; check start/stop string mode
	bne	.1
	eor.b	#1,d2

.1	tst.b	d2
	bne	.copy_raw	; we're in string mode


	; check for letter
	cmp.b	#'a',d0
	blo	.copy_raw
	cmp.b	#'z',d0
	bhi	.copy_raw

	; it's a letter: see if there's a match
	subq	#1,a0		; make sure A0 points again to start of potential keyword
	bsr	b_check_keyword
	tst.b	d0		; did we find a token? d0 != 0
	bne	.write_token	; yes, go write it

	; if it wasn't, restore char and copy as raw text
	move.b	(a0)+,d0

.copy_raw
	move.b	d0,(a1)+
	bra	.start

.write_token
	move.b	d0,(a1)+
	bra	.start

.done	move.b	d0,(a1)+
	movem.l	(sp)+,d2
	rts


; ----------------------------------------------------------------------
; Subroutine: b_check_keyword
; Input:      a0 pointer to input string
; Outputs:    d0 is $00 if no token found or token byte ($80-$ff) if found
;             a0 untouched when no match, A0 points to after keyword if match
; Destroyed:  d1
; ----------------------------------------------------------------------
b_check_keyword
	movem.l	a2-a3,-(sp)
	lea	b_token_table,a2

.loop_keywords
	tst.b	(a2)		; check the first character from the keyword
	beq	.not_found	; if $00, we hit end of table
	movea.l	a0,a3		; leave a0, use a3 for now

.compare_loop
	move.b	(a2)+,d1
	beq	.match_found	; we came this far and character = null = match found
	move.b	(a3)+,d0	; get character from input buffer
	cmp.b	d0,d1		; compare chars case sensitive
	beq	.compare_loop

.skip_keyword
	tst.b	(a2)+		; test table byte and advance pointer
	bne	.skip_keyword	; repeat as it wasn't zero
	addq.l	#1,a2		; it was zero, so advance pointer to skip token value
	bra	.loop_keywords	; try next keyword

.match_found
	move.b	(a2),d0		; load token value into d0
	movea.l	a3,a0		; advance original a0 pointer
	movem.l	(sp)+,a2-a3	; cleanup and return
	rts

.not_found
	moveq	#0,d0		; no match, a0 not advanced
	movem.l	(sp)+,a2-a3	; cleanup and return
	rts


; ----------------------------------------------------------------------
; Subroutine: b_double_dabble (bin to bcd)
; Inputs:     d0.l 32bits unsigned value
; Outputs:    d0-d1 combined holding 10 bcd's, big endian order
; ----------------------------------------------------------------------
b_double_dabble
	movem.l	d2-d3,-(sp)
	moveq	#32-1,d3	; counter for 32 shifts
	move.l	d0,d2		; D2 now holds input value
	moveq	#0,d0
	moveq	#0,d1

.start
	move.l	d1,-(sp)	; check individual numbers if >=5
	bsr.s	chk_nm
	move.l	(sp)+,d1
	exg	d0,d1
	move.l	d1,-(sp)
	bsr.s	chk_nm
	move.l	(sp)+,d1
	exg	d0,d1

	asl.l	d2
	roxl.l	d1
	roxl.l	d0

	dbra	d3,.start

	movem.l	(sp)+,d2-d3
	rts

chk_nm
	bsr.s	chk_nums_h1
	swap	d0
	bsr.s	chk_nums_h1
	swap	d0
	rts

chk_nums_h1
	move.l	d0,d1
	andi.b	#$0f,d1
	cmp.b	#$05,d1
	blo	.n1
	addi.b	#$3,d0

.n1
	andi.w	#$0f00,d1
	cmp.w	#$500,d1
	blo	.n2
	addi.w	#$300,d0

.n2
	move.l	d0,d1
	andi.b	#$f0,d1
	cmp.b	#$50,d1
	blo	.n3
	addi.b	#$30,d0

.n3
	andi.w	#$f000,d1
	cmp.w	#$5000,d1
	blo	.end
	addi.w	#$3000,d0

.end
	rts


;-----------------------------------------------------------------------
; Subroutine: b_remove_spaces
; Inputs:     a0 pointer to text in buffer
; Outputs:    a0 point to first item not equal to space ($20)
;-----------------------------------------------------------------------
b_remove_spaces
	cmp.b	#' ',(a0)
	bne.s	.end
	addq.l	#1,a0
	bra.s	b_remove_spaces

.end
	rts


;-----------------------------------------------------------------------
; Subroutine: b_get_dec_number
; Inputs:     a0 points to ascii
; Outputs:    d0.l contains the number (32 bits), d1 contains the number
;             of digits it consumed (so is 0 when NAN)
;             a0 remains (1) the same if NAN or (2) points after number
;-----------------------------------------------------------------------
b_get_dec_number
	move.l	d2,-(sp)
	clr.l	d0		; will contain end result
	clr.l	d1		; is zero if NAN

.start
	cmp.b	#'0',(a0)
	blo	.end
	cmp.b	#'9',(a0)
	bhi	.end
	move.l	d0,d2		; times ten: first save D0 into D2
	asl.l	#2,d0		; double D0
	add.l	d2,d0		; now D0 is times 5
	asl.l	#1,d0		; double D0 (times 10)
	move.b	(a0)+,d2
	andi.l	#$f,d2
	add.l	d2,d0
	addq.l	#1,d1
	bra	.start

.end
	move.l	(sp)+,d2
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
