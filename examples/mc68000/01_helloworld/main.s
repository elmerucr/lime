main	move.b	#'@',-(SP)
	move.b	#1,-(SP)
	trap	#15
	addq	#4,SP
	bra.s	main
