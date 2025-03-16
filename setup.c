#include <stdio.h>
#include <stdint.h>
#include "setup.h"

#define UNUSED(x) (void)(x)

// 0000
int EQ(PSR *cpsr) {
	return (cpsr->z);
}

// 0001
int NE(PSR *cpsr) {
	return !(cpsr->z);
}

// 0010
int CS(PSR *cpsr) {
	return (cpsr->c);
}

// 0011
int CC(PSR *cpsr) {
	return !(cpsr->c);
}

// 0100
int MI(PSR *cpsr) {
	return (cpsr->n);
}

// 0101
int PL(PSR *cpsr) {
	return !(cpsr->n);
}

// 0110
int VS(PSR *cpsr) {
	return (cpsr->v);
}


// 0111
int VC(PSR *cpsr) {
	return !(cpsr->v);
}


// 1000
int HI(PSR *cpsr) {
	return (cpsr->c && !cpsr->z);
}


// 1001
int LS(PSR *cpsr) {
	return (!cpsr->c && cpsr->z);
}


// 1010
int GE(PSR *cpsr) {
	return (cpsr->n == cpsr->v);
}


// 1011
int LT(PSR *cpsr) {
	return (cpsr->n != cpsr->v);
}


// 1100
int GT(PSR *cpsr) {
	return (!cpsr->z && (cpsr->n == cpsr->v));
}

// 1101
int LE(PSR *cpsr) {
	return (cpsr->z || (cpsr->n != cpsr->v));
}

// 1110
int AL(PSR *cpsr) {
	(void)cpsr;  // Explicitly mark it as unused
	return 1;
}

// 1111 is not used

int (*condition_codes[15])(PSR *) = {
	EQ, NE, CS, CC, MI, PL, VS, VC, HI, LS, GE, LT, GT, LE, AL
};

uint8_t fetch_memory(Memory *memory, unsigned int address) {
	if (address < 0x00003FFF) {
		return memory->bios[address];
	}

	if (address < 0x0203FFFF) {
		return memory->wram1[address - 0x02000000];
	}

	if (address < 0x03007FFF) {
		return memory->wram2[address - 0x03000000];
	}

	if (address < 0x040003FE) {
		return memory->io[address - 0x04000000];
	}

	if (address < 0x050003FF) {
		return memory->bg_obj_palette_ram[address - 0x0500000];
	}

	if (address < 0x06017FFF) {
		return memory->vram[address - 0x06000000];
	}

	if (address < 0x070003FF) {
		return memory->obj_attributes[address - 0x07000000];
	}

	if (address < 0x09FFFFFF) {
		return memory->rom[address - 0x08000000];
	}

	fprintf(stderr, "Invalid address: %d\n", address);
	return -1;
}

uint32_t fetch_instruction_arm(Memory *memory, uint32_t pc) {
    if (pc < 0x08000000 || pc > 0x09FFFFFF) {
        fprintf(stderr, "Invalid rom pc: %d\n", pc);
        return -1;
    }

    // fetches the 4 bytes at pc
    uint32_t  instruction = *(unsigned int *)&memory->rom[pc - 0x08000000];

    return instruction;
}

uint16_t fetch_instruction_thumb(Memory *memory, uint32_t pc) {
    if (pc < 0x08000000 || pc > 0x09FFFFFF) {
        fprintf(stderr, "Invalid rom pc: %d\n", pc);
        return -1;
    }

    // fetches the 2 bytes at pc
    uint16_t instruction = *(uint16_t *)&memory->rom[pc - 0x08000000];

    return instruction;
}


// Setup ARM instructions


// Data processing
/*
    Create an array where each element is a poitner to a function that does a specific operation on regsiters.

*/

void update_carry_boolean(int *dest, PSR *cpsr) {
    cpsr->n = (*dest < 0);
    cpsr->z = (*dest == 0);
    cpsr->c = 0;
    cpsr->v = 0;
}

// 0000
void AND(int op1, int *dest, int op2, PSR *cpsr, int update_flags) {
    *dest = op1 & op2;

    if (update_flags) {
        update_carry_boolean(dest, cpsr);
    }
}

// 0001
void EOR(int op1, int *dest, int op2, PSR *cpsr, int update_flags) {
    *dest = op1 ^ op2;

    if (update_flags) {
        update_carry_boolean(dest, cpsr);
    }
}

// 0010
void SUB(int op1, int *dest, int op2, PSR *cpsr, int update_flags) {
    *dest = op1 - op2;

    if (update_flags) {
        cpsr->n = (*dest < 0);
        cpsr->z = (*dest == 0);
        cpsr->c = op1 >= op2;
        cpsr->v = (op1 >= 0 && op2 <= 0 && *dest < 0) | (op1 <= 0 && op2 >= 0 && *dest > 0);
    }
}

// 0011
void RSB(int op1, int *dest, int op2, PSR *cpsr, int update_flags) {
    *dest = op2 - op1;

    // TODO, check if this logic is correct (Carry)
    if (update_flags) {
        cpsr->n = (*dest < 0);
        cpsr->z = (*dest == 0);
        cpsr->c = op2 >= op1;
        cpsr->v = (op1 >= 0 && op2 <= 0 && *dest < 0) | (op1 <= 0 && op2 >= 0 && *dest > 0);
    }
}

// 0100
void ADD(int op1, int *dest, int op2, PSR *cpsr, int update_flags) {
    uint64_t temp = op1 + op2;
    // should cast nicely
    *dest = temp;

    if (update_flags) {
        cpsr->n = (*dest < 0);
        cpsr->z = (*dest == 0);
        cpsr->c = (temp >> 32) & 0x1;
        cpsr->v = (op1 > 0 && op2 > 0 && *dest < 0) | (op1 < 0 && op2 < 0 && *dest > 0);
    }
}

// 0101
void ADC(int op1, int *dest, int op2, PSR *cpsr, int update_flags) {
    uint64_t temp = op1 + op2 + cpsr->c;
    *dest = temp;

    if (update_flags) {
        cpsr->n = (*dest < 0);
        cpsr->z = (*dest == 0);
        cpsr->c = (temp >> 32) & 0x1;
        cpsr->v = (op1 > 0 && op2 > 0 && *dest < 0) | (op1 < 0 && op2 < 0 && *dest > 0);
    }
}

// 0110
void SBC(int op1, int *dest, int op2, PSR *cpsr, int update_flags) {
    *dest = op1 - op2 + cpsr->c - 1;

    if (update_flags) {
        cpsr->n = (*dest < 0);
        cpsr->z = (*dest == 0);
        cpsr->c = op1 >= (op2 + cpsr->c - 1);
        cpsr->v = (op1 >= 0 && op2 <= 0 && *dest < 0) | (op1 <= 0 && op2 >= 0 && *dest > 0);
    }
}

// 0111
void RSC(int op1, int *dest, int op2, PSR *cpsr, int update_flags) {
    *dest = op2 - op1 + cpsr->c - 1;

    if (update_flags) {
        cpsr->n = (*dest < 0);
        cpsr->z = (*dest == 0);
        cpsr->c = op2 >= (op1 + cpsr->c - 1);
        cpsr->v = (op1 >= 0 && op2 <= 0 && *dest < 0) | (op1 <= 0 && op2 >= 0 && *dest > 0);
    }
}

// 1000
void TST(int op1, int *dest, int op2, PSR *cpsr, int update_flags) {
    UNUSED(dest), UNUSED(update_flags);
    int temp = op1 & op2;

    cpsr->n = (temp < 0);
    cpsr->z = (temp== 0);
    // other flags not updated
}

// 1001
void TEQ(int op1, int *dest, int op2, PSR *cpsr, int update_flags) {
    UNUSED(dest), UNUSED(update_flags);
    int temp = op1 ^ op2;

    cpsr->n = (temp < 0);
    cpsr->z = (temp == 0);
    // other flags not updated
}

// 1010
void CMP(int op1, int *dest, int op2, PSR *cpsr, int update_flags) {
    UNUSED(dest), UNUSED(update_flags);
    int temp = op1 - op2;

    cpsr->n = (temp < 0);
    cpsr->z = (temp == 0);
    cpsr->c = op1 >= op2;
    cpsr->v = (op1 >= 0 && op2 <= 0 && *dest < 0) | (op1 <= 0 && op2 >= 0 && *dest > 0);
}

// 1011
void CMN(int op1, int *dest, int op2, PSR *cpsr, int update_flags) {
    UNUSED(dest), UNUSED(update_flags);
    uint64_t temp = op1 + op2;
    uint32_t real_temp = temp;

    cpsr->n = (real_temp < 0);
    cpsr->z = (real_temp == 0);
    cpsr->c = (temp >> 32) & 0x1;
    cpsr->v = (op1 > 0 && op2 > 0 && real_temp < 0) | (op1 < 0 && op2 < 0 && temp > 0);
}

// 1100
void ORR(int op1, int *dest, int op2, PSR *cpsr, int update_flags) {
    *dest = op1 | op2;

    if (update_flags) {
        cpsr->n = (*dest < 0);
        cpsr->z = (*dest == 0);
        cpsr->c = 0;
        cpsr->v = 0;
    }
}

// 1101
void MOV(int op1, int *dest, int op2, PSR *cpsr, int update_flags) {
    UNUSED(op1);
    *dest = op2;

    if (update_flags) {
        cpsr->n = (*dest < 0);
        cpsr->z = (*dest == 0);
        // other flags not effected
    }
}

// 1110
void BIC(int op1, int *dest, int op2, PSR *cpsr, int update_flags) {
    *dest = op1 & ~op2;

    if (update_flags) {
        cpsr->n = (*dest < 0);
        cpsr->z = (*dest == 0);
        // other flags not effected
    }
}

// 1111
void MVN(int op1, int *dest, int op2, PSR *cpsr, int update_flags) {
    UNUSED(op1);
    *dest = ~op2;

    if (update_flags) {
        cpsr->n = (*dest < 0);
        cpsr->z = (*dest == 0);
        // other flags not effected
    }
}
