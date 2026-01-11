main		ldx	#timer0_irq		; update the timer0 vector
		stx	VECTOR_TIMER0_INDIRECT
		ldd	#40000			; 40000bpm for timer0
		std	TIMER0_BPM
		lda	#1			; activate timer0
		sta	TIMER_CR
		andcc	#%11101111		; enable irq's
1		sync				; rest until interrupt
		bra	1b

timer0_irq	lda	VDC_BG_COLOR		; load current bg color
		inca				; increase
		cmpa	#$40			; compare to 64
		bne	1f
		clra				; clear if 64
1		sta	VDC_BG_COLOR		; store
		rti				; end of interrupt
