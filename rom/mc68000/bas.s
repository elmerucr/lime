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
