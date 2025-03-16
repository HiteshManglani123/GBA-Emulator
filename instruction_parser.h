#ifndef INSTRUCT_PARSER
#define INSTRUCT_PARSER
#include <stdint.h>

void decode_instruction_arm(uint32_t instruction);
void decode_instruction_thumb(uint16_t instruction);
#endif
