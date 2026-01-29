main	move.b	#$37,VDC_BG_COLOR	; pink background color
	clr.b	VDC_CURRENT_LAYER
	andi.b	#%11110111,VDC_LAYER_FLAGS0	; turn off color per tile

.1	clr.b	D0	; prng is function 0
	trap	#14	; from trap 14

	move.b	D0,D1	; move result from D0 to D1
	move.b	#1,D0	; putchar is function 1
	trap	#15	; from trap 15

	bra	.1
