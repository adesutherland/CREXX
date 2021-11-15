#include <stdio.h>
#include <rxas.h>
#include <stdlib.h>
#include <string.h>

#include "rxarutil.h"

static abin loadModule(VFILE *vfile, abin *program) {

    if (!vfile->exists) {
        fprintf(stderr, "Internal error while loading the binary file. File does not exists!");
        exit(INTERNAL_ERROR);
    }

    vfile = vfopen(vfile, "rb");

    if (vfile->opened) {
        // read numeric fields
        fread(&(*program).segment.globals,    1, sizeof(int),    vfile->fp);
        fread(&(*program).segment.inst_size,  1, sizeof(size_t), vfile->fp);
        fread(&(*program).segment.const_size, 1, sizeof(size_t), vfile->fp);

        // skip code area
        fseek(vfile->fp, (long)(sizeof(bin_code) * (*program).segment.inst_size), SEEK_CUR);

        // read constants
        (*program).segment.const_pool = calloc((*program).segment.const_size, 1);
        fread((*program).segment.const_pool, 1, (*program).segment.const_size, vfile->fp);

        program->name   = vfile->basename;
        program->loaded = TRUE;

        vfclose(vfile);

    }

    return (*program);
}

static void unloadModule(abin *program) {
        if ((*program).segment.const_pool)
            free((*program).segment.const_pool);
}

QUEUE* findImportedProcedures(VFILE *vfile, QUEUE **procnames) {

    size_t ii = 0;

    abin program;

    chameleon_constant  *c_entry;
    proc_constant       *p_entry;

    size_t procedures = 0;

    loadModule(vfile, &program);

    if (program.loaded) {
        // first iteration to count exposed procedures
        while (ii < program.segment.const_size) {
            c_entry = (chameleon_constant *) (program.segment.const_pool + ii);

            if (c_entry->type == EXPOSE_PROC_CONST) {
                if (((expose_proc_constant *) c_entry)->imported) {
                    procedures++;
                }
            }

            ii += c_entry->size_in_pool;
        }

        // allocate queue
        *procnames = newqueue(procedures);

        // reset counter
        ii = 0;

        // second iteration fill queue with procedure names
        while (ii < program.segment.const_size) {
            c_entry = (chameleon_constant *) (program.segment.const_pool + ii);

            if (c_entry->type == EXPOSE_PROC_CONST) {
                //p_entry = (proc_constant *) (program.segment.const_pool + ((expose_proc_constant *) c_entry) ->procedure);

                if (((expose_proc_constant *) c_entry)->imported) {
                    RXLIB_BIN_PROC_NAME *procs_entry;

/*
                    procs_entry = malloc(sizeof(RXLIB_BIN_PROC_NAME) + strlen(p_entry->name));
                    procs_entry->pnlen = (short) strlen(p_entry->name);
                    strcpy(procs_entry->pname, p_entry->name);
*/

                    procs_entry = malloc(sizeof(RXLIB_BIN_PROC_NAME) +
                                         strlen(((expose_proc_constant *) c_entry)->index));
                    procs_entry->pnlen = (short) strlen(((expose_proc_constant *) c_entry)->index);

                    strcpy(procs_entry->pname, ((expose_proc_constant *) c_entry)->index);

                    enqueue(*procnames, procs_entry);
                }
            }

            ii += c_entry->size_in_pool;
        }

        unloadModule(&program);

        return *procnames;
    } else {
        return NULL;
    }

}

QUEUE* findExposedProcedures(VFILE *vfile, QUEUE **procnames) {

    size_t ii = 0;

    abin program;

    chameleon_constant  *c_entry;
    proc_constant       *p_entry;

    size_t procedures = 0;

    loadModule(vfile, &program);

    if (program.loaded) {
        // first iteration to count exposed procedures
        while (ii < program.segment.const_size) {
            c_entry = (chameleon_constant *) (program.segment.const_pool + ii);

            if (c_entry->type == EXPOSE_PROC_CONST) {
                if (!((expose_proc_constant *) c_entry)->imported) {
                    procedures++;
                }
            }

            ii += c_entry->size_in_pool;
        }

        // allocate queue
        *procnames = newqueue(procedures);

        // reset counter
        ii = 0;

        // second iteration fill queue with procedure names
        while (ii < program.segment.const_size) {
            c_entry = (chameleon_constant *) (program.segment.const_pool + ii);

            if (c_entry->type == EXPOSE_PROC_CONST) {
                //p_entry = (proc_constant *) (program.segment.const_pool + ((expose_proc_constant *) c_entry) ->procedure);

                if (!((expose_proc_constant *) c_entry)->imported) {
                    RXLIB_BIN_PROC_NAME *procs_entry;

/*
                    procs_entry = malloc(sizeof(RXLIB_BIN_PROC_NAME) + strlen(p_entry->name));
                    procs_entry->pnlen = (short) strlen(p_entry->name);
                    strcpy(procs_entry->pname, p_entry->name);
*/

                    procs_entry = malloc(sizeof(RXLIB_BIN_PROC_NAME) +
                            strlen(((expose_proc_constant *) c_entry)->index));
                    procs_entry->pnlen = (short) strlen(((expose_proc_constant *) c_entry)->index);

                    strcpy(procs_entry->pname, ((expose_proc_constant *) c_entry)->index);

                    enqueue(*procnames, procs_entry);
                }
            }

            ii += c_entry->size_in_pool;
        }

        unloadModule(&program);

        return *procnames;
    } else {
        return NULL;
    }
}
