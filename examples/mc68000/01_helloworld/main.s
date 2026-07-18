main	move.b	#$37,VDC_BG_COLOR	; "orange" background color
	clr.b	VDC_CURRENT_LAYER
	andi.b	#%11110111,VDC_LAYER_FLAGS0	; turn off color per tile
	move.w	#$fff6,VDC_LAYER_Y_MSB

	clr.b	VDC_CURRENT_SPRITE
	clr.b	VDC_SPRITE_INDEX
	move.b	#%00000101,VDC_SPRITE_FLAGS0
	move.b	#$08,VDC_SPRITE_TILESET_PAGE
	move.b	#27,VDC_SPRITE_X_LSB
	move.b	#$a,VDC_SPRITE_Y_LSB

	lea.l	hello,A0
	move.b	#2,D1
	trap	#15

.1	bra	.1	; loop

hello	dc.b	"Hello W  rld!",0
