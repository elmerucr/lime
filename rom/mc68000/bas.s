;-----------------------------------------------------------------------
		;rsset	$....	no need, rs continues where it left of
bas_start	rs.b	1
bas_buf		rs.l	256
bas_end
;-----------------------------------------------------------------------

dummy	move.b	#1,D0
	move.b	D0,bas_start
	move.b	D0,bas_buf
	bra.s	dummy
