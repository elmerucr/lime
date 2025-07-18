all: rom.hpp

rom.hpp: mk_rom rom.bin
	./mk_rom

rom.bin: rom.s definitions.i
	asm6809 -B -o rom.bin -l rom.list -s rom.symbols rom.s

mk_rom: mk_rom.c
	gcc -o mk_rom mk_rom.c

clean:
	rm mk_rom rom.bin rom_MC6809.hpp rom.list rom.symbols
