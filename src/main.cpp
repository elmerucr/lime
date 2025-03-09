// ---------------------------------------------------------------------
// main.cpp
// lime
//
// Copyright © 2025 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include <cstdio>
#include "system.hpp"

int main(int argc, char *argv[])
{
	char *bin = NULL;

	if (argc > 2) {
		printf("[lime] Error: Too many command line arguments\n");
		return 1;
	} else {
		if (argc == 2) {
			bin = argv[1];
			// FILE *f = NULL;
			// f = fopen(argv[1], "r");
			// if (f) {
			// 	printf("opening %s\n", argv[1]);
			// 	fclose(f);
			// } else {
			// 	printf("can't open %s\n", argv[1]);
			// }
		}
	}

	system_t *system = new system_t(bin);
	system->run();
	delete system;

    return 0;
}
