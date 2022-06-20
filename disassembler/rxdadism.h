#ifndef CREXX_RXDADISM_H
#define CREXX_RXDADISM_H

#include "rxas.h"
#include "rxbin.h"

#define rxversion "cREXX F0042"

/* Disassembler */
void disassemble(bin_space *pgm, module_file *module, FILE *stream, int print_all_constant_pool);

#endif //CREXX_RXDADISM_H
