all: rom_m68k.hpp

rom_m68k.hpp: mk_rom rom.bin
	./mk_rom

rom.bin: rom.s
	vasmm68k_mot -no-opt -m68000 -Fbin -L rom.list -o rom.bin rom.s

mk_rom: mk_rom.c
	gcc -o mk_rom mk_rom.c

clean:
	rm mk_rom rom.bin rom.list rom_m68k.hpp
