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
}

enum InstructionIDs {
    I_NOOP = 0,

    
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

void interpret_bytecode(struct buffer* buffer) {
    byte next;
    int index = 0;
    while(index < buffer->length) {
        next = buffer->data[index];
        switch(next) {
            case I_NOOP:
                break;
            case I_TEST:
                printf("Hello");
            default:
                break;
        }
        index++;
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

