main	clr.b	d1	; prng is function 0
	trap	#14	; from trap 14, leaves random number in d0
	move.b	#1,d1	; putchar is function 1
	trap	#15	; from trap 15, takes what is in d0
	bra	main
