#ifndef INSTRUCT_PARSER
#define INSTRUCT_PARSER
#include <stdint.h>

void decode_instruction_arm(uint32_t instruction);
void decode_instruction_thumb(uint16_t instruction, uint32_t pc, uint32_t stack_pointer, uint32_t link_register);
#endif
