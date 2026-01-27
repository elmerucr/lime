main	move.b	#1,D0		; putchar
	move.b	#'@',D1		; '@'
	trap	#15		; call trap 15
	bra.s	main
