;-----------------------------------------------------------------------
		;rsset	$....	no need, rs continues where it left of
basic_start	rs.l	1
basic_end	rs.b	0
;-----------------------------------------------------------------------

basic_cold_start
	move.l	$20000,basic_start	; start of basic memory
	rts

basic_warm_start
	lea	basic_prompt,A0
	bsr	terminal_putstring
	rts

basic_process_buffer
	lea	terminal_buf_1,A0
.bpb1	move.b	(A0)+,D0
	cmp.b	#' ',D0
	beq.s	.bpb1		; remove leading spaces
	subi.b	#'0',D0
	cmp.b	#9,D0
	bhi.s	.end		; not a decimal number
	lea	conf,A0
	bsr	terminal_putstring
.end	rts

conf	dc.b	$a,"this starts with a number",0
basic_prompt
	dc.b	$a,"Ready.",$a,0
