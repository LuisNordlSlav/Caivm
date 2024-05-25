#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>

typedef unsigned char byte;

enum Registers {
    // R0-R9 before this, then:
    R_SP = 10,
    R_PC,
    R_A,
    R_B,
    R_C,
    R_D,
};

enum InstructionIDs {
    I_NOOP = 0,

    I_LoadImmediateByte,
    I_AddRegistersByte,



    I_DBG_PrintRegister = 0xff,
};

struct buffer {
    unsigned char* data;
    size_t length;
};

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
    fread(buf, 1, size, fd);
    fclose(fd);
    buf[size] = 0;
    struct buffer res = (struct buffer) {
        .data = (unsigned char*)buf,
        .length = size,
    };
    return res;
}

byte next_byte(struct buffer* buffer, int *index) {
    byte b = buffer->data[*index];
    *index += 1;
    return b;
}

void interpret_bytecode(struct buffer* buffer) {
    byte next;
    int index = 0;
    unsigned long long registers[16];
    for(int i = 0; i < 16; i++)
        registers[i] = 0;
    while(index < buffer->length) {
        next = next_byte(buffer, &index);
        byte reg, val, r1, r2;
        switch(next) {
            case I_NOOP:
                break;
            case I_LoadImmediateByte:
                reg = next_byte(buffer, &index);
                val = next_byte(buffer, &index);
                registers[reg] = val;
                break;
            case I_AddRegistersByte:
                r1 = next_byte(buffer, &index);
                r2 = next_byte(buffer, &index);
                registers[r1] = (byte)registers[r1] + (byte)registers[r2];
                break;
            case I_DBG_PrintRegister:
                reg = next_byte(buffer, &index);
                printf("r_%d = %llu", reg, registers[reg]);
                break;
            default:
                break;
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 2)
        usage(*argv);
    char* path = argv[1];
    struct buffer fdata = read_file(path);

    interpret_bytecode(&fdata);

    free(fdata.data);
    return 0;
}

