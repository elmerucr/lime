;-----------------------------------------------------------------------
		;rsset	$....	no need, rs continues where it left of
basic_start	rs.b	0
basic_buf_1	rs.b	128
basic_buf_2	rs.b	128
basic_end	rs.b	0
;-----------------------------------------------------------------------

dummy	move.b	#1,D0
	move.b	D0,basic_start
	move.b	D0,basic_buf_1
	bra.s	dummy

basic_process_buffer
	lea	basic_buf_1,A0
	move.b	(A0)+,D0
	subi.b	#'0',D0
	cmp.b	#9,D0
	bhi.s	.end		; not a decimal number
	pea	conf
	jsr	terminal_putstring
	addq.l	#4,SP
.end	rts

conf	dc.b	$a,"number",0
