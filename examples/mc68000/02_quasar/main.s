main	clr.b	VDC_BG_COLOR.w		; make sure bg color = 0
	move.l	#routine,VEC_TIMER0.w	; update vector for timer0 event
	move.w	#40000,TIMER0_BPM	; 40.000 beats per minute
	move.b	#$01,TIMER_CR.w		; activate timer 0
.1	bra	.1			; endless loop

routine	move.l	D0,-(SP)

	move.b	VDC_BG_COLOR.w,D0
	addq.b	#1,D0
	cmp.b	#$40,D0
	bne	.1
	clr.b	D0
.1	move.b	D0,VDC_BG_COLOR.w

	move.l	(SP)+,D0
	rts
