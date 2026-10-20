/*
 * GVPC C -- GLOBALV (VM/370plus): the globalv command for Linux, macOS
 * and Windows.  C89.  The stores and the terminal: GVFS_PC C.
 *
 *   globalv [INIT] [SELECT group] verb ...       (as on CMS)
 *   eval "$(globalv select g get a b)"           GET into the shell
 *   globalv stack a | rxqueue                    STACK for REXX
 *
 * Build 2, 19 October 2026.
 */
#include <stddef.h>          /* first: gcclib31 string.h needs size_t */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gvint.h"

/* ---------------------------------------------------------------- */
/* main: the operands as one string                                 */
/* ---------------------------------------------------------------- */

int main(int argc, char **argv)
{
    char  *args;
    size_t n = 1;
    int    i, rc;

    if (argc == 2 && (strcmp(argv[1], "--version") == 0 ||
                      strcmp(argv[1], "-v") == 0)) {
        printf("%s\n", gv_version());
        return 0;
    }
    for (i = 1; i < argc; i++)
        n += strlen(argv[i]) + 1;
    args = (char *)malloc(n);
    if (args == NULL)
        return 104;
    args[0] = '\0';
    for (i = 1; i < argc; i++) {
        if (i > 1)
            strcat(args, " ");
        strcat(args, argv[i]);
    }
    rc = gv_cmdio(args, (int)strlen(args), &gv_io_default);
    free(args);
    fflush(stdout);
    return rc;
}
