main	move.b	#$37,VDC_BG_COLOR	; pink background color
	clr.b	VDC_CURRENT_LAYER
	andi.b	#%11110111,VDC_LAYER_FLAGS0	; turn off color per tile

	clr.b	VDC_CURRENT_SPRITE
	clr.b	VDC_SPRITE_INDEX
	move.b	#%00000101,VDC_SPRITE_FLAGS0
	move.w	#$0800,VDC_SPRITE_TILESET_ADDR
	move.b	#56,VDC_SPRITE_X_LSB
	clr.b	VDC_SPRITE_Y_LSB

	move.b	#2,D0
	lea.l	hello,A0
	trap	#15

.1	bra	.1

hello	dc.b	"Hello w rld!",0
