main		clr	VDC_CURRENT_LAYER	; points to layer 0
		lda	#%00000101
		sta	VDC_LAYER_FLAGS0	; activate, 00 transp

		ldx	#VDC_LAYER0_TILES
1		jsr	$ffed			; ac now contains prng number
		sta	,x+
		cmpx	#VDC_LAYER0_TILES+(32*20)
		bne	1b

		lda	#$37
		sta	VDC_BG_COLOR

		; print string
		ldx	#text
		ldy	#VDC_LAYER0_TILES
1		lda	,x+
		beq	2f
		sta	,y+
		bra	1b

2		clr	VDC_CURRENT_SPRITE
		clr	VDC_SPRITE_INDEX
		lda	#%00000101
		sta	VDC_SPRITE_FLAGS0
		lda	#56
		sta	VDC_SPRITE_X_LSB
		clr	VDC_SPRITE_Y_LSB

1		ldx	#VDC_LAYER0_TILES
2		jsr	$ffed
		sta	,x+
		cmpx	#(VDC_LAYER0_TILES+$400)
		bne	2b
		bra	1b			; loop / do nothing

text		fcn	"Hello w rld!"		; 'o' replaced by sprite
