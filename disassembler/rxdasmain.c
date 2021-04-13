#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "rxasdisa.h"
#include <operands.h>
#include "rxas.h"

int main(int argc, char *argv[]) {

    FILE *inFile;

    bin_space pgm;

    size_t fileSize;
    size_t bytesRead;

    Assembler_Context scanner;

    /* TODO: temp. writing to disk, must be made stable */
    printf("DBG> Reading file %s\n", argv[1]);

    {
        inFile = fopen(argv[1], "rb");

        bytesRead = fread(&pgm.globals,    1, sizeof(int),    inFile);
        bytesRead = fread(&pgm.inst_size,  1, sizeof(size_t), inFile);
        bytesRead = fread(&pgm.const_size, 1, sizeof(size_t), inFile);

        pgm.binary     = malloc(pgm.inst_size);
        pgm.const_pool = malloc(pgm.const_size);

        bytesRead = fread(pgm.binary,     1, pgm.inst_size, inFile);
        bytesRead = fread(pgm.const_pool, 1, pgm.const_size, inFile);

        fclose(inFile);
    }

    init_ops();
    scanner.binary = pgm;

    printf("Running Disassembler\n\n");
    disassemble(&scanner, stdout);

}