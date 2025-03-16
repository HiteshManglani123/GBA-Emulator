#ifndef SETUP_H
#define SETUP_H
#include <stdint.h>
#define ROM_SIZE 32 * 1024 * 1024

typedef struct ProgramStatusRegister {
	unsigned int m0: 	1;
	unsigned int m1: 	1;
	unsigned int m2: 	1;
	unsigned int m3: 	1;
	unsigned int m4: 	1;
	unsigned int t: 	1;
	unsigned int f:		1;
	unsigned int i:		1;
	unsigned int pad: 20;
	unsigned int v:		1;
	unsigned int c:		1;
	unsigned int z:		1;
	unsigned int n: 	1;
} PSR;

typedef struct {

// General Internal Memory
uint8_t bios[16 * 1024]; 								// 0x00000000-0x00003FFF 16 	KBytes
uint8_t wram1[256 * 1024]; 							    // 0x02000000-0x0203FFFF 256 	KBytes
uint8_t wram2[32 * 1024];								// 0x03000000-0x03007FFF 32 	KBytes
uint8_t io[1024];										// 0x04000000-0x040003FE 1		KByte


// Internal Display Memory
uint8_t bg_obj_palette_ram[1024]; 			            // 0x05000000-0x050003FF 1		KByte
uint8_t vram[96 * 1024];								// 0x06000000-0x06017FFF 96		KBytes
uint8_t obj_attributes[1024];						    // 0x07000000-0x070003FF 1		KByte
uint8_t	rom[ROM_SIZE];									// 0x08000000-0x09FFFFFF 32     MB
} Memory;

extern int registers[16];
extern int (*condition_codes[15])(PSR *);

uint8_t fetch_memory(Memory *memory, uint32_t address);
uint32_t fetch_instruction_arm(Memory *memory, uint32_t address);
uint16_t fetch_instruction_thumb(Memory *memory, uint32_t address);

#endif
