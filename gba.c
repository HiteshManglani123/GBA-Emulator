#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "setup.h"

/*
todo:
setup environment
- registers
- memory map
- condition codes
- instructions

*/

/*

	Handle condition codes:
	- Given a 4 bit number, we can create an array where each element represents
		a pointer to a function that returns an expresison (1 or 0).

*/


int main(void) {
	FILE *rom = fopen("PokemonEmeraldRom.gba", "rb");

	if (rom == NULL) {
		printf("Error! could not open rom\n");
		exit(1);
	}
	
	
	fread(memory.rom, 1, ROM_SIZE, rom);

	fclose(rom);

	printf("first byte: %d\n", memory.rom[0]);

	exit(0);
}