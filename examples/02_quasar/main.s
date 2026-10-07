main	clr.b	VDC_BG_COLOR.w		; make sure bg color = 0
	move.l	#routine,VEC_TIMER0.w	; update vector for timer0 event
	move.w	#65535,TIMER0_BPM	; 40.000 beats per minute
	move.b	#$01,TIMER_CR.w		; activate timer 0
.1	bra	.1			; endless loop

routine	move.l	d0,-(sp)

	move.b	VDC_BG_COLOR.w,d0
	addq.b	#1,d0
	;cmp.b	#$40,d0
	;bne	.1
	;clr.b	d0
	move.b	d0,VDC_BG_COLOR.w

	move.l	(sp)+,d0
	rts
