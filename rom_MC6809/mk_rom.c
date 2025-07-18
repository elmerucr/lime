#include <stdint.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[])
{
	time_t t;
	time(&t);

	uint8_t romdata[512];

	// read
	FILE *f;
	f = fopen("rom.bin", "r");

	fseek(f, 0L, SEEK_END);
	long pos = ftell(f);

	printf("[mk_rom] rom.bin size: %lu bytes\n", pos);
	if (pos > 512L) {
		printf("[mk_rom] too large, exiting...\n");
		fclose(f);
		return 1;
	}

	// go back to beginning of file
	// read data
	rewind(f);
	fread(romdata, pos, 1, f);

	// fill up the rest of the final rom with zeroes
	for (int i=pos; i < 512; i++) {
		romdata[i] = 0x00;
	}

	// close original unpatched bin file
	fclose(f);

	// write 8k output to cpp file
	printf("[mk_rom] writing 512b image 'rom.bin' and 'rom_MC6809.hpp' for inclusion in punch\n");
	f = fopen("rom_MC6809.hpp","w");

	fprintf(f, "// ---------------------------------------------------------------------\n");
	fprintf(f, "// rom_MC6809.hpp\n");
	fprintf(f, "// lime\n");
	fprintf(f, "//\n");
	fprintf(f, "// Copyright (C)2025 elmerucr. All rights reserved.\n");
	fprintf(f, "// %s",ctime(&t));
	fprintf(f, "// ---------------------------------------------------------------------\n\n");

	fprintf(f, "#ifndef ROM_MC6809_HPP\n");
	fprintf(f, "#define ROM_MC6809_HPP\n\n");

	fprintf(f, "#include <cstdint>\n\n");

	fprintf(f, "class rom_MC6809_t {\n");
	fprintf(f, "public:\n");
	fprintf(f, "\tconst uint8_t data[512] = {");

	for(int i = 0; i < 511; i++) {
		if(i%8 == 0) fprintf(f, "\n\t\t");
		fprintf(f, "0x%02x,", romdata[i]);
	}

	fprintf(f, "0x%02x\n\t};\n", romdata[(512)-1]);

	fprintf(f, "};\n");

	fprintf(f, "\n#endif\n");

	fclose(f);

	// write 512b output to bin file
	f = fopen("rom.bin","wb");
	fwrite(romdata, sizeof(romdata), 1, f);
	fclose(f);

	return 0;
}
