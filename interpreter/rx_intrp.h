//
// Created by adria on 29/03/2021.
//

#ifndef CREXX_RX_INTRP_H
#define CREXX_RX_INTRP_H

typedef struct bin_space program;

/* Signals an error - this function does not return */
void dosignal(int code);

int initialize();
int finalize();
int run(program *program, int argc, char *argv[]);

#endif //CREXX_RX_INTRP_H
