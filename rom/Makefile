all: rom.hpp

rom.hpp: mk_rom rom.bin
	./mk_rom

rom.bin: rom.s definitions.inc basic.s
	vasmm68k_mot -devpac -no-opt -m68000 -Fbin -L rom.list -o rom.bin rom.s

mk_rom: mk_rom.c
	gcc -o mk_rom mk_rom.c

clean:
	rm mk_rom rom.bin rom.list rom.hpp
