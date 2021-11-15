#include <stdlib.h>

#include "types.h"
#include "rxlntest.h"
#include "platform.h"
#include "rxas.h"

typedef struct module {
    BOOL loaded;
    char *name;
    bin_space segment;
} module;


void test(char *fileName) {

    /*
    module program;

    printf("DEBUG: Binary file %s will be loaded. \n", fileName);

    program = loadModule(fileName, &program);

    if (program.loaded) {
        getProcedures(&program);
    }  else {
        fprintf(stderr, "ERROR: Binary file %s could not be loaded.", fileName);
        exit(-1);
    }
     */
}


