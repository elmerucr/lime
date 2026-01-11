;-----------------------------------------------------------------------
; DECB binary format: http://www.lwtools.ca/manual/x27.html
;
; These binaries include the load address of the binary as well as
; encoding an execution address. These binaries may contain multiple
; loadable sections, each of which has its own load address.
;
; Each binary starts with a preamble. Each preamble is five bytes long.
; The first byte is zero. The next two bytes specify the number of bytes
; to load and the last two bytes specify the address to load the bytes
; at. Then, a string of bytes follows. After this string of bytes, there
; may be another preamble or a postamble. A postamble is also five bytes
; in length. The first byte of the postamble is $FF, the next two are
; zero, and the last two are the execution address for the binary.
;-----------------------------------------------------------------------

		include	"../../../rom/mc6809/definitions.i"

		org	RAM_START-5			; allocate 5 bytes for code_preamble

code_preamble	fcb	$00				; first byte
		fdb	graph_preamble-RAM_START	; size (16 bit unsigned)
		fdb	RAM_START			; where (16 bit unsigned)

code		include	"main.s"

graph_preamble	fcb	$00				; first byte
		fdb	postamble-tiles			; size (16 bit unsigned)
		fdb	$0800				; where (16 bit unsigned)

tiles		include	"tiles.s"

postamble	fcb	$ff,$00,$00
		fdb	main				; execution address
