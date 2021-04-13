#include <stdio.h>
#include <stdlib.h>
#include <operands.h>
#include "rx_intrp.h"
#include "rxas.h"

int main(int argc, char *argv[]) {


    FILE *inFile;

    bin_space pgm;

    size_t fileSize;
    size_t bytesRead;

    /* TODO: temp. writing to disk, must be made stable */
    printf("DBG> Reading file %s\n", argv[1]);

    inFile = fopen(argv[1], "rb");

    bytesRead = fread(&pgm.globals,    1, sizeof(int),    inFile);
    bytesRead = fread(&pgm.inst_size,  1, sizeof(size_t), inFile);
    bytesRead = fread(&pgm.const_size, 1, sizeof(size_t), inFile);

    pgm.binary     = calloc(pgm.inst_size, sizeof(bin_code));
    pgm.const_pool = calloc(pgm.const_size, 1);

    bytesRead = fread(pgm.binary, sizeof(bin_code), pgm.inst_size, inFile);
    bytesRead = fread(pgm.const_pool, 1, pgm.const_size, inFile);

    fclose(inFile);


    init_ops();

    printf("Running byte code \n");
    char *prog_argv[1];
    prog_argv[0] = "Hello Rene - REXX Assembler is born!";


    run(&pgm, 1 , prog_argv);
}