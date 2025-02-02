#include <stdint.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[])
{
	time_t t;
	time(&t);

	uint8_t romdata[256];

	// read
	FILE *f;
	f = fopen("rom.bin", "r");

	fseek(f, 0L, SEEK_END);
	long pos = ftell(f);

	printf("[mk_rom] rom.bin size: %lu bytes\n", pos);
	if (pos > 256L) {
		printf("[mk_rom] too large, exiting...\n");
		fclose(f);
		return 1;
	}

	// go back to beginning of file
	// read data
	rewind(f);
	fread(romdata, pos, 1, f);

	// fill up the rest of the final rom with zeroes
	for (int i=pos; i < 256; i++) {
		romdata[i] = 0x00;
	}

	// close original unpatched bin file
	fclose(f);

	// write 8k output to cpp file
	printf("[mk_rom] writing 256b image 'rom.bin' and 'rom.cpp' for inclusion in punch\n");
	f = fopen("rom.hpp","w");

	fprintf(f, "// ---------------------------------------------------------------------\n");
	fprintf(f, "// rom.hpp\n");
	fprintf(f, "// lime\n");
	fprintf(f, "//\n");
	fprintf(f, "// Copyright (C)2025 elmerucr. All rights reserved.\n");
	fprintf(f, "// %s",ctime(&t));
	fprintf(f, "// ---------------------------------------------------------------------\n\n");

	fprintf(f, "#ifndef ROM_HPP\n");
	fprintf(f, "#define ROM_HPP\n\n");

	fprintf(f, "#include <cstdint>\n\n");

	fprintf(f, "class rom_t {\n");
	fprintf(f, "public:\n");
	fprintf(f, "\tconst uint8_t data[256] = {");

	for(int i = 0; i < 255; i++) {
		if(i%8 == 0) fprintf(f, "\n\t\t");
		fprintf(f, "0x%02x,", romdata[i]);
	}

	fprintf(f, "0x%02x\n\t};\n", romdata[(256)-1]);

	fprintf(f, "};\n");

	fprintf(f, "\n#endif\n");

	fclose(f);

	// write 256b output to bin file
	f = fopen("rom.bin","wb");
	fwrite(romdata, sizeof(romdata), 1, f);
	fclose(f);

	return 0;
}
