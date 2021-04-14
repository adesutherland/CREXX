#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "operands.h"
#include "rx_intrp.h"
#include "rxas.h"

int main(int argc, char *argv[]) {

    FILE *fp;

    bin_space pgm;

    char *fileName;

    printf("REXX Interpreter Testbed Version: PoC 2\n\n");

    if (argc < 2) {
        printf("Invalid Arguments\nFormat: rxdas file_name\n");
        exit (-1);
    }
    if (strcmp(argv[1],"-v") == 0) {
        printf("Version: PoC 2 Build 2\n");
        exit (0);
    }
    fileName = argv[1];

    fp = fopen(fileName, "rb");

    fread(&pgm.globals, 1, sizeof(int), fp);
    fread(&pgm.inst_size, 1, sizeof(size_t), fp);
    fread(&pgm.const_size, 1, sizeof(size_t), fp);

    pgm.binary     = calloc(pgm.inst_size, sizeof(bin_code));
    pgm.const_pool = calloc(pgm.const_size, 1);

    fread(pgm.binary, sizeof(bin_code), pgm.inst_size, fp);
    fread(pgm.const_pool, 1, pgm.const_size, fp);

    fclose(fp);

    init_ops();




    run(&pgm, argc-2 , argv+2);
}