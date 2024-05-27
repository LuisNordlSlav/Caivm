#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "descriptive_types.h"


enum Registers {
    // R0-R9 before this, then:
    R_SP = 10,
    R_PC,
    R_A,
    R_B,
    R_C,
    R_D,
};

enum MathOps {
    OP_EQ = 0,
    OP_GT = 1,
    OP_LT = 2,
    OP_ZE = 3,
};

enum InstructionIDs {
    I_NOOP = 0,

    I_LoadImmediateByte = 0x10,
    I_LoadImmediateShort,
    I_LoadImmediateInt,
    I_LoadImmediateLong,

    I_AddRegistersByte = 0x20,
    I_AddRegistersShort,
    I_AddRegistersInt,
    I_AddRegistersLong,

    I_CmpRegisters = 0xA0,
    I_JumpEqReg,
    I_JumpGtReg,
    I_JumpLtReg,
    I_JumpZeReg,

    I_JumpEqImm,
    I_JumpGtImm,
    I_JumpLtImm,
    I_JumpZeImm,

    I_DBG_PrintRegister = 0xff,
};

struct buffer {
    unsigned char* data;
    size_t length;
} CONTEXT;

void usage(const char* this) {
    printf("USAGE: %s filename\n", this);
    exit(0);
}

struct buffer read_file(const char* path) {
    int size;
    {
        struct stat st;

        if (stat(path, &st) == 0)
            size = st.st_size;
        else
            return (struct buffer){};
    }
    char* buf = malloc(size+1);
    FILE* fd = fopen(path, "r");
    if (fread(buf, 1, size, fd) == 0)
        return (struct buffer) {};
    fclose(fd);
    buf[size] = 0;
    struct buffer res = (struct buffer) {
        .data = (unsigned char*)buf,
        .length = size,
    };
    return res;
}

u8 next_byte(u64* index) {
    u8 b = *(u8*)(CONTEXT.data + *(index + R_PC));
    *(index + R_PC) += 1;
    return b;
}

u16 next_short(u64* index) {
    u16 b = *(u16*)(CONTEXT.data + *(index + R_PC));
    *(index + R_PC) += 1;
    return b;
}

u32 next_int(u64* index) {
    u32 b = *(u32*)(CONTEXT.data + *(index + R_PC));
    *(index + R_PC) += 1;
    return b;
}

u64 next_long(u64* index) {
    u64 b = *(u64*)(CONTEXT.data + *(index + R_PC));
    *(index + R_PC) += 1;
    return b;
}

void* exec_bytecode(void* _start_offset) {
    u64 registers[15];
    for (int i = 0; i < 15; i++) registers[i] = 0;
    registers[R_PC] = (u64)_start_offset;
    u8 flags = 0;

    while (registers[R_PC] < CONTEXT.length) {
        byte next = next_byte(&registers[0]);
        switch (next) {
            case I_NOOP:
                break;
            case I_LoadImmediateByte: {
                u8 reg = next_byte(&registers[0]);
                u8 val = next_byte(&registers[0]);
                *(u8*)&registers[reg] = val;
            } break;
            case I_LoadImmediateShort: {
                u8 reg = next_byte(&registers[0]);
                u16 val = next_short(&registers[0]);
                *(u16*)&registers[reg] = val;
            } break;
            case I_LoadImmediateInt: {
                u8 reg = next_byte(&registers[0]);
                u32 val = next_int(&registers[0]);
                *(u32*)&registers[reg] = val;
            } break;
            case I_LoadImmediateLong: {
                u8 reg = next_byte(&registers[0]);
                u64 val = next_long(&registers[0]);
                *(u64*)&registers[reg] = val;
            } break;
            case I_AddRegistersByte: {
                u8 r1 = next_byte(&registers[0]);
                u8 r2 = next_byte(&registers[0]);
                *(u8*)&registers[r1] += *(u8*)&registers[r2];
            } break;
            case I_AddRegistersShort: {
                u8 r1 = next_byte(&registers[0]);
                u8 r2 = next_byte(&registers[0]);
                *(u16*)&registers[r1] += *(u16*)&registers[r2];
            } break;
            case I_AddRegistersInt: {
                u8 r1 = next_byte(&registers[0]);
                u8 r2 = next_byte(&registers[0]);
                *(u32*)&registers[r1] += *(u32*)&registers[r2];
            } break;
            case I_AddRegistersLong: {
                u8 r1 = next_byte(&registers[0]);
                u8 r2 = next_byte(&registers[0]);
                *(u64*)&registers[r1] += *(u64*)&registers[r2];
            } break;
            case I_CmpRegisters: {
                u8 r1 = next_byte(&registers[0]);
                u8 r2 = next_byte(&registers[0]);
                flags = (registers[r1] == registers[r2]) << OP_EQ |
                        (registers[r1] >  registers[r2]) << OP_GT |
                        (registers[r1] <  registers[r2]) << OP_LT |
                        (registers[r1] == 0) << OP_ZE
                ;
            } break;
            case I_JumpEqReg: {
                u8 target_reg = next_byte(&registers[0]);
                if (flags & (1 >> OP_EQ))
                    registers[R_PC] = registers[target_reg];
            } break;
            case I_JumpGtReg: {
                u8 target_reg = next_byte(&registers[0]);
                if (flags & (1 >> OP_GT))
                    registers[R_PC] = registers[target_reg];
            } break;
            case I_JumpLtReg: {
                u8 target_reg = next_byte(&registers[0]);
                if (flags & (1 >> OP_LT))
                    registers[R_PC] = registers[target_reg];
            } break;
            case I_JumpZeReg: {
                u8 target_reg = next_byte(&registers[0]);
                if (flags & (1 >> OP_ZE))
                    registers[R_PC] = registers[target_reg];
            } break;
            case I_JumpEqImm: {
                u64 imm = next_long(&registers[0]);
                if (flags & (1 >> OP_EQ))
                    registers[R_PC] = imm;
            } break;

            case I_DBG_PrintRegister: {
                u8 reg = next_byte(&registers[0]);
                u64 val = registers[reg];
                printf("r_%d = %lu\n", reg, val);
            } break;

            default:
                printf("unexpected byte encountered: %x\n", next);
            break;
        }
    }

    return NULL;
}

int main(int argc, char** argv) {
    if (argc != 2)
        usage(*argv);
    char* path = argv[1];
    CONTEXT = read_file(path);

    (void)exec_bytecode(0);

    free(CONTEXT.data);
    return 0;
}

