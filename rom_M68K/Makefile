rom.bin: rom.s
	vasmm68k_mot -no-opt -m68000 -Fbin -L rom.list -o rom.bin rom.s

clean:
	rm rom.bin rom.list
