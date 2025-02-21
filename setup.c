#include <stdio.h>
#include <stdint.h>
#include "setup.h"


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

int registers[16] = {0};

// Memory



Memory memory = {};


uint8_t memory_look_up(unsigned int address) {
	if (address < 0x00003FFF) {
		return memory.bios[address];
	}
	
	if (address < 0x0203FFFF) {
		return memory.wram1[address - 0x02000000];
	}
	
	if (address < 0x03007FFF) {
		return memory.wram2[address - 0x03000000];
	}
	
	if (address < 0x040003FE) {
		return memory.io[address - 0x04000000];
	}
	
	if (address < 0x050003FF) {
		return memory.bg_obj_palette_ram[address - 0x0500000];
	}

	if (address < 0x06017FFF) {
		return memory.vram[address - 0x06000000];
	}

	if (address < 0x070003FF) {
		return memory.obj_attributes[address - 0x07000000];
	}

	if (address < 0x09FFFFFF) {
		return memory.rom[address - 0x08000000];
	}
	
	fprintf(stderr, "Invalid address: %d\n", address); 
	return -1;
}


