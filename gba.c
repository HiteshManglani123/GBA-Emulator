#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "setup.h"
#include "instruction_parser.h"

enum INSTRUCTION_MODE {
    ARM = 0,
    THUMB = 1
};

int load_rom(Memory *memory) {

    FILE *fp = fopen("PokemonEmeraldRom.gba", "rb");

    if (fp == NULL) {
        return 0;
	}

    fread(memory->rom, 1, ROM_SIZE, fp);

	fclose(fp);

	return 1;
}

// todo, add better validation to this.
uint32_t get_digit(char *s) {
    uint32_t val = 0;

    int sign = (*s == '-') ? -1 : 1;

    if (*s == '-') {
        s++;
    }

    if (!(*s >= '0' && *s <= '9')) {
        fprintf(stderr, "%c is not a valid digit!\n", *s);
    }


    for ( ; *s >= '0' && *s <= '9'; s++) {
        val = val * 10 + (*s - '0');
    }

    return val * sign;
}

int main(int argc, char *argv[]) {
    uint32_t amount_to_deocde = 10;

    if (argc > 1) {
        amount_to_deocde = get_digit(argv[1]);
    }

    Memory *memory = (Memory *) malloc(sizeof(Memory));
    int registers[16] = {0};     // register[15] = PC

    if (load_rom(memory) == 0) {
        fprintf(stderr, "Error! could not open rom\n");
        exit(1);
    }

    // Rom starts at this location
    int *pc = &registers[15];
    *pc = 0x08000000;

    uint32_t address = 134217728; // 0x08000000

    uint8_t instruction_mode = ARM;

    while (amount_to_deocde > 0) {

        // TODO create a way to know when to decode ARM vs Thumb
        if (instruction_mode == ARM) {
            uint32_t instruction = fetch_instruction_arm(memory, *pc);
            printf("0x%.8x: %.8x ", address, instruction);
            decode_instruction_arm(instruction);

            *pc += 4;
            address += 4;

        } else {
            // THUMB instruction
            uint16_t instruction = fetch_instruction_thumb(memory, *pc);
            printf("0x%.8x: %.4x ", address, instruction);
            decode_instruction_thumb(instruction);
            *pc += 2;
            address += 2;
        }



        // decode_instruction_arm(instruction);
        amount_to_deocde--;
	}

	free(memory);

	exit(0);
}
