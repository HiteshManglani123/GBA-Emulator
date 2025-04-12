#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/*

- Data Processing / PSR Transfer
- Multiply
- Multiply Long
- Single Data Swap
- Branch and Exchange
- Halfword Data Transfer: Register offset
- Halfword Data Transfer: Immediate offset
- Single Data Transfer
- Undefined
- Block Data Transfer
- Branch
- Software Interrupt

These are not implemented in the GBA:
- Coprocessor Data Transfer
- Coprocessor Data Operation
- Coprocessor Register Transfer


Goal: Find something unique for each to distinguish them.

All formats start with a 4 bit condition field (at MSB)


*/


char *register_names[] = {
    "RO",
    "R1",
    "R2",
    "R3",
    "R4",
    "R5",
    "R6",
    "R7",
    "R8",
    "R9",
    "R10",
    "R11",
    "R12",
    "R13",
    "R14",
    "R15"
};

char *condition_names[] = {
    "EQ",
    "NE",
    "CS",
    "CC",
    "MI",
    "PL",
    "VS",
    "VC",
    "HI",
    "LS",
    "GE",
    "LT",
    "GT",
    "LE",
    "\0" // supposed to be AL but this is default and thus not visible.
};


// TODO see if i can combine these 2 some how (ARM and THUMB)
char *opcode_names[] = {
    "AND",
    "EOR",
    "SUB",
    "RSB",
    "ADD",
    "ADC",
    "SBC",
    "RSC",
    "TST",
    "TEQ",
    "CMP",
    "CMN",
    "ORR",
    "MOV",
    "BIC",
    "MVN"
};

char *opcode_names_thumb[] = {
    "AND",
    "EOR",
    "LSL",
    "LSR",
    "ASR",
    "ADC",
    "SBC",
    "ROR",
    "TST",
    "NEG",
    "CMP",
    "CMN",
    "ORR",
    "MUL",
    "BIC",
    "MVN"
};


char *shift_types[] = {
    "LSL", // logical shift left
    "LSR", // logical shift right
    "ASR", // arithmetic shift right
    "ROR"  // rotate right - not available for thumb
};

char *sh_data_transfer_types[] = {
    "INVALID (SWAP)", // not possible, should be a swap
    "H", // unsigned half word
    "SB", // signed byte
    "SH" // signed half word
};

char *sh_load_store_sign_extended_byte_halfword[] = {
    "STRH",
    "LDRH",
    "LDSB",
    "LDSH"
};


// TODO: check if these 2 are right
char *block_data_transfer_addressing_names_stack[] = {
    "ED",
    "EA",
    "FD",
    "FA",
    "FA",
    "FD",
    "EA",
    "ED"
};

char *block_data_transfer_addressing_names_other[] = {
    "DA",
    "IA",
    "DB",
    "IB",
    "DA",
    "IA",
    "DB",
    "IB"
};

#define MAX_SWI_BIOS_FUNCTIONS 43

char *swi_bios_functions[MAX_SWI_BIOS_FUNCTIONS] = {
    "SoftReset",
    "RegisterRamReset",
    "Halt",
    "Stop",
    "IntrWait",
    "VBlankIntrWait",
    "Div",
    "DivArm",
    "Sqrt",
    "ArcTan",
    "ArcTan2",
    "CPUSet",
    "CPUFastSet",
    "BiosChecksum",
    "BgAffineSet",
    "ObjAffineSet",

    "BitUnPack",
    "LZ77UnCompWRAM",
    "LZ77UnCompVRAM",
    "HuffUnComp",
    "RLUnCompWRAM",
    "RLUnCompVRAM",
    "Diff8bitUnFilterWRAM",
    "Diff8bitUnfilterVRAM",
    "Diff16bitUnFilter",
    "SoundBiasChange",
    "SoundDriverInit",
    "SoundDriverMode",
    "SoundDriverMain",
    "SoundDriverVSync",
    "SoundChannelClear",
    "MIDIKey2Freq",

    "MusicPlayerOpen",
    "MusicPlayerStart",
    "MusicPlayerStop",
    "MusicPlayerContinue",
    "MusicPlayerFadeOut",
    "MultiBoot",
    "HardReset",
    "CustomHalt",
    "SoundDriverVSyncOff",
    "SoundDriverVSyncOn",
    "GetJumpList"
};

char * thumb_general_immediate_operation[] = {
    "MOV",
    "CMP",
    "ADD",
    "SUB"
};

char *hi_register_operations[] = {
    "ADD",
    "CMP",
    "MOV"
};

/*
TODO:
     - Check Branch and Exchange
     - Check Branch
     - Check Data processing
     - Check PSR Transfer
     - Check Multiply
     - Check Multiply Long
     - Single Data transfer!!! (lol wtf is this, check 0x0800004c especially)
     - Check Halfword and Signed Data Transfer
     - Check Block Data Transfer
     - Check Software Interrupt
*/

int get_imm(uint32_t instruction) {
    // operand 2 is an immediate operand rotate
    uint8_t rotate = (instruction >> 8) & 0xF; // 4 bit value
    // according to the ARM7TDMI datasheet, imm should be zero extended to a 32 bit value that goes from unsigned to signed
    int imm = instruction & 0xFF;

    // according to datasheet, shift_amount = val at rotate * 2
    uint8_t shift_amount = rotate * 2;

   unsigned int mask = 1;

   mask = (mask << shift_amount) - 1;

   int masked_val = imm & mask;

   imm = imm >> shift_amount;

    // imm is 32 bit (zero extended)
    imm = (masked_val << (32 - shift_amount)) | imm;

    return imm;
}


void print_data_processing(uint32_t instruction, uint8_t is_imm) {
    // printf("Data Processing / PSR Transfer 1\n");
    uint8_t condition = (instruction >> 28) & 0xF;
    uint8_t opcode = (instruction >> 21) & 0xF;
    uint8_t update_condition = (instruction >> 20) & 0x1;

    uint8_t first_operand = (instruction >> 16) & 0xF;
    uint8_t dest_operand = (instruction >> 12) & 0xF;
    uint8_t second_operand = instruction & 0xF;
    /*
        1 MOV,MVN (single operand instructions.) 15 13
        <opcode>{cond}{S} Rd,<Op2>

        3 AND,EOR,SUB,RSB,ADD,ADC,SBC,RSC,ORR,BIC (!(> 7 && < 12))
        <opcode>{cond}{S} Rd,Rn,<Op2>
    */



    // if its not a condition code
    if (!(opcode > 7 && opcode < 12)) {
        printf("%s", opcode_names[opcode]);
        printf("%s", condition_names[condition]);
        printf("%c ", update_condition ? 'S' : '\0');
        printf("%s,", register_names[dest_operand]);

        if (opcode != 15 && opcode != 13) {
            printf("%s,", register_names[first_operand]);
        }

        if (is_imm) {
            int imm = get_imm(instruction);
            printf("#%d", imm);
        } else {
            // Todo: Doing some duplicate work in single data transfer for printing shifts
            printf("%s,", register_names[second_operand]);

            uint8_t shift_operation_type = (instruction >> 4) & 0x1;
            uint8_t shift_type = (instruction >> 5) & 0x3;

            printf("%s ", shift_types[shift_type]);

            if (shift_operation_type == 0) {
                uint8_t shift_amount = (instruction >> 7) & 0x1F;
                printf("#%d", shift_amount);
            } else {
                uint8_t shift_register = (instruction >> 8) * 0xF;
                printf("%s", register_names[shift_register]);
            }
        }

        printf("\n");
        return;
    } else {
        // its either CMP, CMN, TEQ, TST or PSR Transfer
        // check sign bit is off
        if (!((instruction >> 20) & 0x1)) {

            if ((instruction >> 25) & 0x1) {
                int imm = get_imm(instruction);
                // MSR{cond} <psrf>,<#expression>
                printf("MSR");
                printf("%s ", condition_names[condition]);
                printf("CPSR_flg,"); // I believe GameBoyAdvance is always in User Mode
                printf("#%d", imm);
            } else {
                uint8_t transfer_type = (instruction >> 16) & 0x3F;

                if (transfer_type == 15) {
                    /*
                        MRS (transfer PSR contents to a register)
                        MRS{cond} Rd,<psr>
                    */
                    printf("MRS");
                    printf("%s ", condition_names[condition]);
                    printf("%s,", register_names[dest_operand]);
                    printf("CPSR"); // GBA runs in User Mode
                } else if (transfer_type == 41) {
                    /*
                        SHOULD NOT BE POSSIBLE I BELIEVE!
                        MSR (transfer register contents to PSR)
                        MSR{cond} <psr>,Rm
                    */
                    fprintf(stderr, "Don't think its possible to have  MSR (transfer register contents to PSR): %.8x\n", instruction);
                    // exit(1);
                } else if (transfer_type == 40) {
                    /*
                        MSR (transfer register contents or immdiate value to PSR flag bits only)
                        MSR{cond} <psrf>,Rm
                    */
                    printf("CPSR_flg,"); // I believe GameBoyAdvance is always in User Mode
                    printf("%s,", register_names[dest_operand]);
                } else {
                    fprintf(stderr, "Unknown instruction: %.8x\n", instruction);
                    // exit(1);
                }

            }
        } else {
            /*
                2 CMP,CMN,TEQ,TST (instructions which do not produce a result.)
                <opcode>{cond} Rn,<Op2>
            */
            printf("%s", opcode_names[opcode]);
            printf("%s ", condition_names[condition]);
            printf("%s,", register_names[first_operand]);
            if (is_imm) {
                int imm = get_imm(instruction);
                printf("#%d", imm);
            } else {
                // Todo: doing same code above, make a func for it
                printf("%s,", register_names[second_operand]);

                uint8_t shift_operation_type = (instruction >> 4) & 0x1;
                uint8_t shift_type = (instruction >> 5) & 0x3;

                printf("%s ", shift_types[shift_type]);

                if (shift_operation_type == 0) {
                    uint8_t shift_amount = (instruction >> 7) & 0x1F;
                    printf("#%d", shift_amount);
                } else {
                    uint8_t shift_register = (instruction >> 8) * 0xF;
                    printf("%s", register_names[shift_register]);
                }
            }

        }


    }

    printf("\n");
    return;
}

void print_register_list(uint32_t instruction, uint8_t amount_of_registers) {

    // {amount_of_registers} possible register 2-3 bytes each with {amount_of_registers} ,'s and a \0
    char register_names_string[amount_of_registers * 3 + amount_of_registers + 1];
    uint8_t register_names_pos = 0;

    for (int i = 0; i < amount_of_registers; i++) {
        if ((instruction >> i) & 0x1) {

            char *register_name = register_names[i];

            while (*register_name) {
                register_names_string[register_names_pos++] = *register_name++;
            }

            register_names_string[register_names_pos++] = ',';
        }
    }

    // remove trailing ','. this should always be over 0 but just incase
    if (register_names_pos > 0) {
        register_names_string[register_names_pos - 1] = '\0';
    }

    printf("%s", register_names_string);

}

void decode_instruction_arm(uint32_t instruction) {
    uint8_t condition = (instruction >> 28) & 0xF;

    uint8_t first_three = (instruction >> 25) & 0x7;
    uint8_t next_bit;

    if (first_three == 1) {
       print_data_processing(instruction, 1);
       return;
    }

    if (first_three == 2 || first_three == 3) {
        //printf("Single Data Transfer\n");
        // could also be undefined
        // <LDR|STR>{cond}{B}{T} Rd,<Address>

        uint8_t register_offset =  (instruction >> 25) & 0x1; // 1 is (shifted) register, 0 is immediate
        uint8_t pre_index =  (instruction >> 24) & 0x1; // 1 is pre, 0 is off
        uint8_t up_bit =  (instruction >> 23) & 0x1; // 1 is up, 0 is down
        uint8_t transfer_byte =  (instruction >> 22) & 0x1; // 1 is byte, 0 is word
        uint8_t write_back =  (instruction >> 21) & 0x1; // 1 is write back, 0 is no-write back
        uint8_t load =  (instruction >> 20) & 0x1; // 1 is load, 0 is store

        uint8_t rn_register = (instruction >> 16) & 0xF; // base register
        uint8_t rd_register = (instruction >> 12) & 0xF; // source/destination register
        uint16_t offset = instruction & 0xFFF;

        printf("%s", load ? "LDR" : "STR");
        printf("%s", condition_names[condition]);
        printf("%s", transfer_byte ? "B ": " ");
        printf("%s,", register_names[rd_register]);

        if (pre_index) {
            if (register_offset == 0) {
                // immediate offset
                if (offset == 0) {
                    // [Rn]
                    printf("[%s]", register_names[rn_register]);
                    // no write back
                    return;
                }
            }
        }

        // doing pre and post index, they almost have the same format
        /*
            [Rn],<#expression> offset of <expression> bytes
            [Rn],{+/-}Rm{,<shift>} offset of +/- contents of index register, shifted as by <shift>.
        */

        if (pre_index) {
            printf("[%s,", register_names[rn_register]);
        } else {
            printf("[%s],", register_names[rn_register]);
        }

        if (register_offset == 0) {
            printf("#%d", offset);
            if (pre_index) {
                printf("]");
            }
        }  else {
            if (up_bit == 0) {
                printf("-");
            }
            uint8_t rm_register = instruction & 0xF;
            printf("%s,", register_names[rm_register]);

            uint8_t shift_operation_type = (instruction >> 4) & 0x1;
            uint8_t shift_type = (instruction >> 5) & 0x3;

            printf("%s ", shift_types[shift_type]);

            if (shift_operation_type == 0) {
                uint8_t shift_amount = (instruction >> 7) & 0x1F;
                printf("#%d", shift_amount);
            } else {
                uint8_t shift_register = (instruction >> 8) * 0xF;
                printf("%s", register_names[shift_register]);
            }
        }

        if (pre_index) {
            printf("]");
        }


        if (pre_index && write_back) {
            printf("!");
        }

        printf("\n");
        return;
    }

    if (first_three == 4) {
        // printf("Block Data Transfer\n");

        // <LDM|STM>{cond}<FD|ED|FA|EA|IA|IB|DA|DB> Rn{!},<Rlist>{^}

        uint8_t pre_index = (instruction >> 24) & 0x1; // 1 is pre, 0 is off
        uint8_t up_bit = (instruction >> 23) & 0x1; // 1 is up, 0 is down
        uint8_t load_psr = (instruction >> 22) & 0x1; // 1 is load psr/force user mode, 0 is no load/force user mode
        uint8_t write_back = (instruction >> 21) & 0x1; // 1 is write back, 0 is no-write back
        uint8_t load = (instruction >> 20) & 0x1; // 1 is load, 0 is stor

        uint8_t rn_register = (instruction >> 16) & 0xF; // base register

        uint8_t addressing_name = (load << 2) | (pre_index << 1) | up_bit;



        printf("%s", load ? "LDM" : "STM");
        printf("%s", condition_names[condition]);

        if (rn_register == 13) {
            // based on stack
            printf("%s ", block_data_transfer_addressing_names_stack[addressing_name]);
        } else {
            printf("%s ", block_data_transfer_addressing_names_other[addressing_name]);
        }

        printf("%s", register_names[rn_register]);

        if (write_back) {
            printf("!");
        }

        printf(",");
        printf("{");
        print_register_list(instruction, 16);
        printf("}");
        // not sure about this tbh
        if (load_psr) {
            printf("^");
        }

        printf("\n");
        return;
    }

    if (first_three == 5) {
        // B{L}{cond} <expression>
        uint8_t link_bit = (instruction >> 24) & 0x1;
        uint32_t offset = instruction & ((1 << 24) - 1);

        printf("B");
        if (link_bit) {
            printf("L");
        }
        printf("%s ", condition_names[condition]);
        printf("#%d\n", offset);
        return;
    }

    if (first_three == 6) {
        printf("INVALID -> Coprocessor Data Transfer\n");
        return;
    }

    if (first_three == 7) {
        next_bit = (instruction >> 24) & 0x1;
        if (next_bit) {
            // printf("Software Interrupt\n");
            // SWI{cond} <expression>
            uint32_t swi_number = instruction & 0xFFFFFF; // bits 0-8 of instruction is for bios (swi) fucntions
            printf("SWI");
            printf("%s ", condition_names[condition]);
            if (swi_number >= MAX_SWI_BIOS_FUNCTIONS) {
                printf("#%.2x ; unknown number please check", swi_number);
            } else {
                printf("%s", swi_bios_functions[swi_number]);
            }

            printf("\n");
            return;
        }

        printf("- INVALID -> Coprocessor Data Operation\n");
        printf("- INVALID -> Coprocessor Register Transfer\n");
        return;
    }

    /*
        now first_three has to be 0 and can be the following formats:
            - Data Processing / PSR Transfer
            - Multiply
            - Multiply Long
            - Single Data Swap
            - Branch and Exchange
            - Halfword Data Transfer: Register offset
            - Halfword Data Transfer: Immediate offset
    */

    next_bit = ((instruction >> 7) & 0x1) & ((instruction >> 4) & 0x1);

    // more readable than !next_bit imo
    if (next_bit == 0) {
        /*
            Either:
                - Data Processing / PSR Transfer
                - Branch and Exchange
        */

        // extract possible opcode part
        uint32_t middle = (instruction >> 4) & ((1 << 24) - 1);

        if (middle == 1245169) {
            uint8_t operand_register = instruction & 0xF;
            printf("BX %s\n", opcode_names[operand_register]);
            return;
        }
        /*
        this is either a Data Processing instruction or a PSR Transfer.

        PSR Transfer possibilities:
                - MRS (transfer PSR contents to a register)
                - MSR (transfer register contents to PSR)
                - MSR (transfer register contents or immdiate value to PSR flag bits only)
        */
        print_data_processing(instruction, 0);
        return;
    }


    /*
        next_bit is equal to 1 (bit 7 and 4 are 1).
        Possible values:
            - Multiply
            - Multiply Long
            - Single Data Swap
            - Halfword Data Transfer: Register offset
            - Halfword Data Transfer: Immediate offset
    */

    // extract bit 6-5
    next_bit = (instruction >> 5) & 0x3;

    if (next_bit == 0) {
        /*
            Possible Values:
                - Multiply
                - Multiply Long
                - Single Data Swap
        */

        next_bit = (instruction >> 23) & 0x1;

        if (next_bit == 0) {
            next_bit = (instruction >> 22) & 0x1;

            if (next_bit == 0) {
                /*
                    printf("Multiply\n");
                    MUL{cond}{S} Rd,Rm,Rs
                    MLA{cond}{S} Rd,Rm,Rs,Rn
                */

                uint8_t accumulate = (instruction >> 21) & 0x1;
                uint8_t update_condition = (instruction >> 20) & 0x1;

                uint8_t rd = (instruction >> 16) & 0xF;
                uint8_t rn = (instruction >> 12) & 0xF;
                uint8_t rs = (instruction >> 7) & 0xF;
                uint8_t rm = instruction & 0xF;

                printf("%s", accumulate ? "MLA" : "MUL");
                printf("%s", condition_names[condition]);
                printf("%c ", update_condition ? 'S' : '\0');

                printf("%s,", register_names[rd]);
                printf("%s,", register_names[rm]);
                printf("%s", register_names[rs]);

                if (accumulate) {
                    printf(",%s", register_names[rn]);
                }

                printf("\n");
                return;
            }



            printf("\nDOING SWAP\n");
            // printf("Single Data Swap\n");
            uint8_t swap_byte = (instruction >> 22) & 0x1; // 1 is swap byte quantity, 0 is swap word quantity
            uint8_t rn_register = (instruction >> 16) & 0xF; // base register
            uint8_t rd_register = (instruction >> 12) & 0xF; // destination register
            uint8_t rm_register = instruction & 0xF; // source register

            // <SWP>{cond}{B} Rd,Rm,[Rn]

            printf("SWP");
            printf("%s", condition_names[condition]);

            printf("%s", swap_byte ? "B " : " ");

            printf("%s,", register_names[rd_register]);
            printf("%s,", register_names[rm_register]);
            printf("[%s]", register_names[rn_register]);

            printf("\n");
            return;
        }



        //printf("Multiply Long\n");
        /*
            UMULL{cond}{S} RdLo,RdHi,Rm,Rs Unsigned Multiply
            UMLAL{cond}{S} RdLo,RdHi,Rm,Rs Unsigned Multiply & Accumulate Long
            SMULL{cond}{S} RdLo,RdHi,Rm,Rs Signed Multiply Long
            SMLAL{cond}{S} RdLo,RdHi,Rm,Rs Signed Multiply & Accumulate Long
        */

        uint8_t is_unsigned = (instruction >> 22) & 0x1;
        uint8_t has_accumulate = (instruction >> 21) & 0x1;
        uint8_t update_condition = (instruction >> 20) & 0x1;

        uint8_t high_register = (instruction >> 16) & 0xF;
        uint8_t low_register = (instruction >> 12) & 0xF;

        uint8_t rs_register = (instruction >> 8) & 0xF;
        uint8_t rm_register = instruction & 0xF;

        printf("%c", is_unsigned ? 'U' : 'S');
        printf("%s", has_accumulate ? "MLAL" : "MULL");
        printf("%s", condition_names[condition]);

        printf("%s", update_condition ? "S " : " ");

        printf("%s,", register_names[low_register]);
        printf("%s,", register_names[high_register]);
        printf("%s,", register_names[rm_register]);
        printf("%s", register_names[rs_register]);
        printf("\n");
        return;
    }

    /*
        Possible values:
            - Halfword Data Transfer: Register offset
            - Haldword Data Transfer: Immediate offset

            they have ALMOST the same format.
    */

    // TODO check if this is 22 or 21. it was 21 but it seems like 22 makes more sense


    uint8_t pre_index =  (instruction >> 24) & 0x1; // 1 is pre, 0 is off
    uint8_t up_bit =  (instruction >> 23) & 0x1; // 1 is up, 0 is down
    uint8_t write_back =  (instruction >> 21) & 0x1; // 1 is write back, 0 is no-write back
    uint8_t load =  (instruction >> 20) & 0x1; // 1 is load, 0 is store

    uint8_t rn_register = (instruction >> 16) & 0xF; // base register
    uint8_t rd_register = (instruction >> 12) & 0xF; // source/destination register

    uint8_t sh = (instruction >> 5) & 0x3;
    uint8_t rm_register = instruction & 0xF; // offset  register

    printf("%s", load ? "LDR" : "STR");
    printf("%s", condition_names[condition]);
    printf("%s ", sh_data_transfer_types[sh]);
    printf("%s,", register_names[rd_register]);

    // to distinguish between register and immediate value transfer
    next_bit = (instruction >> 22) & 0x1;

    if (next_bit == 0) {
        // printf("Halfword Data Transfer: Register offset\n");
        // <LDR|STR>{cond}<H|SH|SB> Rd,<address>

        // some duplicate work for pre- and post-index. Check Single Data Transfer aswell.
        if (pre_index) {
            // [Rn,{+/-}Rm]{!}
            printf("[%s,", register_names[rn_register]);
            if (up_bit == 0) {
                printf("-");
            }
            printf("%s]", register_names[rm_register]);
            if (write_back) {
                printf("!");
            }
        } else {
            // [Rn],{+/-}Rm
            printf("[%s],", register_names[rn_register]);
            if (up_bit == 0) {
                printf("-");
            }
            printf("%s", register_names[rm_register]);
        }

        printf("\n");
        return;
    }

    // printf("Halfword Data Transfer: Immediate offset\n");
    // 8-bit unsigned binary immediate value
    uint8_t high_nibble = (instruction >> 8) & 0xF;
    uint8_t low_nibble = instruction & 0xF;
    uint8_t offset = (high_nibble << 4) | low_nibble;

    if (pre_index) {
        /*
            [Rn] offset of zero
            [Rn,<#expression>]{!} offset of <expression> bytes
        */
        if (offset == 0) {
            printf("[%s]", register_names[rn_register]);
        } else {
            printf("[%s,", register_names[rn_register]);
            printf("#%d]", offset);

            if (write_back) {
                printf("!");
            }
        }
    } else {
        // post index

        // [Rn],<#expression>

        printf("[%s],", register_names[rn_register]);
        printf("#%d", offset);
    }

    printf("\n");

    return;
}


void decode_instruction_thumb(uint16_t instruction, uint32_t pc, uint32_t stack_pointer, uint32_t link_register) {
    uint8_t next_bits = (instruction >> 13) & 0x7; // first 3 bits starting at msb

    if (next_bits == 0) {
        /*
            Possible values:
                Move shifted register
                Add/subtract
        */

        // opcode for move shifted register can't be 11
        next_bits = (instruction >> 11) & 0x3;

        if (next_bits == 3) {
            // printf("Add/subtract\n");
            uint8_t is_immediate = (instruction >> 10) & 0x1;
            uint8_t is_sub = (instruction >> 9) & 0x1;
            uint8_t register_or_offset = (instruction >> 6) & 0x7;
            uint8_t rs_register = (instruction >> 3) & 0x7; // source register
            uint8_t rd_register = instruction & 0x7; // destination register

            // format = ADD/SUB RD, RS, RN/#Offset3
            printf("%s ", is_sub ? "SUB" : "ADD");
            printf("%s,", register_names[rd_register]);
            printf("%s,", register_names[rs_register]);
            if (is_immediate) {
                printf("#%d", register_or_offset);
            } else {
                printf("%s", register_names[register_or_offset]);
            }

            printf("\n");
            return;
        }

        // printf("Move shifted register\n");
        uint8_t opcode = (instruction >> 11) & 0x3;
        uint8_t offset5 = (instruction >> 6) & 0x1F;
        uint8_t rs = (instruction >> 3) & 0x7; // source register
        uint8_t rd = instruction & 0x7; // destination register

        // there is no 11 shift_type (rotate right)

        // format = shift_type RD, RS, #Offset5

        printf("%s ", shift_types[opcode]);
        printf("%s,", register_names[rd]);
        printf("%s,", register_names[rs]);
        printf("#%d", offset5);

        printf("\n");
        return;
    }

    if (next_bits == 1) {
        // printf("Move/compare/add/subtract immediate\n");
        uint8_t opcode = (instruction >> 11) & 0x3;
        uint8_t rd_register = (instruction >> 8) & 0x7; // source/destination register
        uint8_t offset8 = instruction & 0xFF;

        // format = MOV/CMP/ADD/SUB RD, #Offset8

        printf("%s ", thumb_general_immediate_operation[opcode]);
        printf("%s,", register_names[rd_register]);
        printf("#%d", offset8);

        printf("\n");
        return;
    }

    if (next_bits == 2) {
        /*
            Possible values:
                ALU operations
                Hi register operations/branch exchange
                PC-relative load
                Load/store with register offset
                Load/store sign-extended byte/halfword
        */

        next_bits = (instruction >> 12) & 0x1;

        if (next_bits == 1) {
            /*
                Possible values:
                    Load/store with register offset
                    Load/store sign-extended byte/halfword
            */

            next_bits = (instruction >> 9) & 0x1;

            if (next_bits == 1) {
                // printf("Load/store sign-extended byte/halfword\n");

                uint8_t sh_flag = (instruction >> 10) & 0x3;

                uint8_t ro = (instruction >> 6) & 0x7; // offset register
                uint8_t rb = (instruction >> 3) & 0x7; // base register
                uint8_t rd = instruction & 0x7; // destination register

                printf("%s ", sh_load_store_sign_extended_byte_halfword[sh_flag]);
                printf("%s, ", register_names[rd]);
                printf("[%s,%s]", register_names[rb], register_names[ro]);

                printf("\n");
                return;
            }

            //printf("Load/store with register offset\n");

            uint8_t load = (instruction >> 11) & 0x1;
            uint8_t transfer_byte = (instruction >> 10) & 0x1;
            uint8_t ro = (instruction >> 6) & 0x7; // offset register
            uint8_t rb = (instruction >> 3) & 0x7; // base register
            uint8_t rd = instruction & 0x7; // source/dest register

            /*
                STR(B) Rd, [Rb, Ro]
                LDR(B) Rd, [Rb, Ro]
            */

            printf("%s", load ? "LDR" : "STR");

            printf("%s", transfer_byte ? "B " : " ");

            printf("%s, ", register_names[rd]);
            printf("[%s,%s]", register_names[rb], register_names[ro]);

            printf("\n");
            return;
        }

        /*
            Possible values:
                ALU operations
                Hi register operations/branch exchange
                PC-relative laod
        */

        next_bits = (instruction >> 11) & 0x1;

        if (next_bits == 1) {
            // printf("PC-relative load\n");
            uint8_t rd_register = (instruction >> 8) & 0x7;
            uint16_t word8 = instruction & 0xFF;

            word8 <<= 2;

            // format LDR RD, [PC, #Imm]
            printf("LDR ");
            printf("%s,", register_names[rd_register]);
            /*
                From datasheet:
                    The value of the PC will be 4 bytes greater than the address of this instruction, but bit
                    1 of the PC is forced to 0 to ensure it is word aligned.
            */

            /*
                a bit confusing if they mean word or half word aligned
                i don't really see scenarios where just bit 1 is cleared
            */
            uint32_t pc_val = (pc += 4) & (~3);
            printf("[%.8x, #%.10d]", pc_val, word8);

            printf("\n");
            return;
        }

        next_bits = (instruction >> 10) & 0x1;

        if (next_bits == 1) {
            // printf("Hi register operations/branch exchange\n");
            uint8_t opcode = (instruction >> 8) & 0x3;
            uint8_t hi_operand_flag_1 = (instruction >> 7) & 0x1;
            uint8_t hi_operand_flag_2 = (instruction >> 6) & 0x1;
            uint8_t rs_register = (instruction >> 3) & 0x7;// source register (or HS)
            uint8_t rd_register = instruction & 0x7; // destination register (or HD)

            if (hi_operand_flag_1) {
                rd_register += 8;
            }

            if (hi_operand_flag_2) {
                rs_register += 8;
            }

            if (opcode == 0x3) {
                // format = BX RS/HS
                printf("BX ");
                printf("%s", register_names[rs_register]);

                printf("\n");
                return;
            }

            // format = ADD/CMP/MOV RD/HD, HS/RS
            printf("%s ", hi_register_operations[opcode]);
            printf("%s,", register_names[rd_register]);
            printf("%s", register_names[rs_register]);

            printf("\n");
            return;
        }

        // printf("ALU operations\n");
        uint8_t opcode = (instruction >> 6) & 0xF;
        uint8_t rs_register = (instruction >> 3) & 0x7; // source register 2
        uint8_t rd_register = instruction & 0x7; // source/destination register

        // format = OPCODE Rd, Rs
        printf("%s ", opcode_names_thumb[opcode]);
        printf("%s,", register_names[rd_register]);
        printf("%s", register_names[rs_register]);

        printf("\n");
        return;
    }


    if (next_bits == 3) {
        // printf("Load/store with immediate offset\n");

        uint8_t transfer_byte = (instruction >> 12) & 0x1; // 0 = word, 1 = byte
        uint8_t load = (instruction >> 11) & 0x1;
        uint8_t offset5 = (instruction >> 6) & 0x1F; // either 5 or 7 bit offset
        uint8_t rb = (instruction >> 3) & 0x7; // base register
        uint8_t rd = instruction & 0x7; // source/destination register

        // for word accesss (transfer_byte = 0),  #imm is 7 bit address (assembler does >> 2)
        if (transfer_byte == 0) {
            offset5 <<= 2;
        }

        printf("%s", load ? "LDR" : "STR");
        printf("%s", transfer_byte ? "B " : " ");
        printf("%s,", register_names[rd]);
        printf("[%s, #%d]", register_names[rb], offset5);

        printf("\n");
        return;
    }

    if (next_bits == 4) {
        /*
            Possible values:
                Load/store halfword
                SP-relative load/store
        */

        next_bits = (instruction >> 12) & 0x1;

        if (next_bits == 1) {
            // printf("SP-relative load/store\n");
            uint8_t load = (instruction >> 11) & 0x1;
            uint8_t rd = (instruction >> 8) & 0x7; // destination register
            uint16_t word8 = instruction & 0xFF;

            // offset (word8) is a 10 bit value, assembler does #imm >> 2

            word8 <<= 2;

            /*
                STR RD, [SP, #IMM]
                LDR RD, [SP, #IMM]
            */

            printf("%s ", load ? "LDR" : "STR");
            printf("%s, ", register_names[rd]);
            printf("[%.8x, #%d]", stack_pointer, word8);

            printf("\n");
            return;
        }

        // printf("Load/store halfword\n");

        uint8_t load = (instruction >> 11) & 0x1; // either load or store
        uint8_t offset5 = (instruction >> 6) & 0x1F;
        uint8_t rb = (instruction >> 3) & 0x7; // base register
        uint8_t rd = instruction & 0x7; // rd register

        // offset is a 6 bit (address/value)m assenbler does #imm >> 1
        offset5 <<= 1;

        /*
            STRH Rd, [Rb, #Imm]
            LDRH Rd, [Rb, #Imm]
        */

        printf("%s ", load ? "LDRH" : "STRH");
        printf("%s, ", register_names[rd]);
        printf("[%s, #%d]", register_names[rb], offset5);

        printf("\n");
        return;
    }

    if (next_bits == 5) {
        /*
            Possible values:
                Load address
                Add offset to stack pointer
                Push/Pop registers
        */

        next_bits = (instruction >> 12) & 0x1;

        if (next_bits == 0) {
            // printf("Load address\n");
            // The CPSR condition codes are unaffected by these instructions.

            uint8_t source = (instruction >> 11); // 0 is PC, 1 is SP
            uint8_t rd = (instruction >> 8) & 0x7;
            uint8_t word8 = instruction & 0xFF; // 8-bit unsigned constant

            // offset (word8) is a 10 bit value (asssembler does >> 2)
            word8 <<= 2;

            /*
            TODO:
                Where the PC is used as the source register (SP = 0), bit 1 of the PC is always read
                as 0. The value of the PC will be 4 bytes greater than the address of the instruction
                before bit 1 is forced to 0.
            */

            printf("ADD ");
            printf("%s,", register_names[rd]);\
            printf("%.8x,", source ? stack_pointer : pc);
            printf("#%d", word8);

            printf("\n");
            return;
        }

        next_bits = (instruction >> 10) & 0x1;

        if (next_bits == 1) {
            // printf("Push/Pop registers\n");

            uint8_t load = (instruction >> 11) & 0x1;
            uint8_t store_load = (instruction >> 8) & 0x1; // 1 is store LR/Load PC, 0 is Do not store LR/Load PC

            /*
                PUSH/POP {RList, LR/PC}
            */

            printf("%s ", load ? "POP" : "PUSH");
            printf("{");
            print_register_list(instruction, 7);

            if (store_load) {
                printf(",%.8x", load ? pc : link_register);
            }

            printf("}");

            printf("\n");
            return;
        }

        // printf("Add offset to stack pointer\n");

        // The condition codes are not set by this instruction.

        uint8_t sign_flag = (instruction >> 7) & 0x1; // 0 is positive, 1 is negative
        int16_t word7 = instruction & 0x3F;

        if (sign_flag) {
            word7 *= -1;
        }

        printf("#ADD %.8x,", stack_pointer);
        printf("%d", word7);

        printf("\n");
        return;
    }

    if (next_bits == 6) {
        /*
            Possible values:
                Multiple load/store
                Conditional branch
                Software interrupt
        */

        next_bits = (instruction >> 12) & 0x1;

        if (next_bits == 0) {
            // printf("Multiple load/store\n");

            uint8_t load = (instruction >> 11) & 0x1; // 1 = load, 0 = store
            uint8_t rb = (instruction >> 8) & 0x7;

            /*
                STMIA Rb!,{Rlist}
                LDMIA Rb!,{Rlist}
            */

            printf("%s ", load ? "LDMIA" : "STMIA");
            printf("%s!,", register_names[rb]);
            printf("{");
            print_register_list(instruction, 7);
            printf("}");

            printf("\n");
            return;
        }

        next_bits = (instruction >> 8) & 0xF;

        if (next_bits == 0xF) {
            // printf("Software interrupt\n");
            // processor switches into ARM state and enters Supervisor (SVC) mode

            // Todo: check if this is signed or unsigned
            uint8_t value8 = instruction & 0xFF;

            printf("SWI ");
            printf("$%d", value8);

            printf("\n");
            return;
        }

        // printf("Conditional branch\n");
        uint8_t cond = (instruction >> 8) & 0xF;
        int8_t offset8_signed = instruction & 0xFF; // 8-bit signed immediate

        // assembler places label >> 1 in offset
        offset8_signed <<= 1;

        /*
            TODO: The branch offset must take account of the prefetrch operation,
                which causes the PC to be 1 word (4 bytes) ahead of the current instruciton
        */

        printf("B");
        printf("%s ", condition_names[cond]);
        printf("#%d", offset8_signed);
        printf("\n");
        return;
    }

    if (next_bits == 7) {
        /*
            Possible values:
                Long branch with link
                Unconditional branch
        */
        next_bits = (instruction >> 12) & 0x1;

        if (next_bits == 1) g{
            // printf("Long branch with link\n");
            // Todo: Implment long branch disassembler
            uint8_t offset_low = (instruction >> 11) & 0x1; // 1 = offset low, 0 = offset high
            int16_t offset10 = instruction & 0x7FF;

            printf("BL ");
            printf("#%.8x", offset10);

            printf("\n");
            return;
        }

        // printf("Uncoditional branch\n");
        // Todo: Check if this is correct representaiton of a 11 bit signed number
        int16_t offset11 = (int16_t)((instruction & 0x7FF) << 5) >> 5;

        offset11 <<= 1;

        printf("B ");
        printf("#%.8x", offset11);

        printf("\n");
        return;
    }
}
