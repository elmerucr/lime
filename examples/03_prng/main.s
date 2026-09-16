main	clr.b	D1	; prng is function 0
	trap	#14	; from trap 14, leaves random number in D0
	move.b	#1,D1	; putchar is function 1
	trap	#15	; from trap 15, takes what is in D0
	bra	main
