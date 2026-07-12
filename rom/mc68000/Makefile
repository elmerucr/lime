all: rom_mc68000.hpp

rom_mc68000.hpp: mk_rom rom.bin
	./mk_rom

rom.bin: rom.s definitions.inc TBI68K.ASM
	vasmm68k_mot -devpac -no-opt -m68000 -Fbin -L rom.list -o rom.bin rom.s

mk_rom: mk_rom.c
	gcc -o mk_rom mk_rom.c

clean:
	rm mk_rom rom.bin rom.list rom_mc68000.hpp
