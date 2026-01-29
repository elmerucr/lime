;-----------------------------------------------------------------------
;
;
;-----------------------------------------------------------------------

		include	"../../../rom/mc68000/definitions.i"

		org	RAM_START-9			; allocate 9 bytes for preamble

		dc.b	$01				; first byte
		dc.l	graph_preamble-RAM_START	; size (16 bit unsigned)
		dc.l	RAM_START			; where (16 bit unsigned)

		include	"main.s"

graph_preamble	dc.b	$01				; first byte
		dc.l	postamble-tiles			; size (16 bit unsigned)
		dc.l	$0800				; where (16 bit unsigned)

tiles		include	"tiles.s"

postamble	dc.b	$fe,$00,$00,$00,$00
		dc.l	main				; execution address
