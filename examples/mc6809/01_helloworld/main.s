main		clr	VDC_CURRENT_LAYER	; points to layer 0
		lda	#%00000101
		sta	VDC_LAYER_FLAGS0	; activate, use transparency ofr 0b00

		lda	#' '
		ldx	#VDC_LAYER0_TILES
1		sta	,x+
		cmpx	#VDC_LAYER0_TILES+(64*23)
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
		ldd #$0800
		std VDC_SPRITE_TILESET_ADDRESS
		lda	#56
		sta	VDC_SPRITE_X_LSB
		clr	VDC_SPRITE_Y_LSB

1		bra	1b			; loop / do nothing

text		fcn	"Hello w rld!"		; 'o' replaced by sprite
