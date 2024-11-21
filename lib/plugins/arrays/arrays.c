//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // For POSIX systems (Linux/macOS)
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file
#include "C:/Users/PeterJ/CLionProjects/CREXX/F0049/interpreter/rxvalue.h"

// Bubble Sorts an array of integers
PROCEDURE(bubble_sort)
{
    int i, j;
    int size;

    // Check the number of arguments
    if (NUM_ARGS != 1) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")

    // Get the array size
    size = GETNUMATTRS(ARG(0));

    // Bubble-sort the array
    for (i = 0; i < size - 1; i++) {
        for (j = 0; j < size - i - 1; j++) {
            if (GETSTRING(GETATTR(ARG(0),j)) > GETSTRING(GETATTR(ARG(0),j + 1))) {
                SWAPATTRS(ARG(0), j, j + 1);
            }
        }
    }
ENDPROC
}

PROCEDURE (shell_sort) {
    int from, to, offset;
    int i, j, k, complete;
    char *val1, *val2, *order;
    offset = GETINT(ARG(1));
    offset--;
    order = GETSTRING(ARG(2));
    to = GETNUMATTRS(ARG(0));
    from = 1;
    k = (from + to) / 2;
    while (k > 0) {
        for (;;) {
            complete = 1;
            for (i = 0; i < to - k; ++i) {
                j = i + k;
                val1 = GETSTRING(GETATTR(ARG(0), i));
                val2 = GETSTRING(GETATTR(ARG(0), j));
                if (strcmp(val1 + offset, val2 + offset) > 0) {
                    SWAPATTRS(ARG(0), i, j);
                    complete = 0;
                }
            }
            if (complete) break;
        }
        k = k / 2;
    }
    if (order[0] == 'D' || order[0] == 'd') {
        to--;        // make index to offset
        k = to / 2;      // split in halfs
        for (i = 0; i <= k; ++i) {
            SWAPATTRS(ARG(0), i, to);
            to--;
        }
    }
    PROCRETURN
    ENDPROC
}
PROCEDURE (reverse_array) {
    int i, k, to;
    to = GETNUMATTRS(ARG(0))-1; // make max index to offset
    k = to / 2;      // split in halfs
    for (i = 0; i <= k; ++i) {
        SWAPATTRS(ARG(0), i, to);
        to--;
    }
    PROCRETURN
    ENDPROC
}
PROCEDURE (search_array) {
    int i, from, to;
    to = GETNUMATTRS(ARG(0));
    from=GETINT(ARG(2))-1;
    for (i = from; i < to; ++i) {
        if (strstr(GETSTRING(GETATTR(ARG(0), i)),GETSTRING(ARG(1)))>0) {
            RETURNINT(i+1);
            PROCRETURN
        }
    }
    RETURNINT(0);
    ENDPROC
}
// Functions to be provided to rexx
LOADFUNCS
//      C Function, REXX namespace & name, Option, Return Type, Arguments
    ADDPROC(bubble_sort,"arrays.bubble_sort","b",    ".void",     "expose a = .string[]");
    ADDPROC(shell_sort, "arrays.shell_sort", "b",    ".void",     "expose a = .string[], offset=.int, order=.string");
    ADDPROC(reverse_array,"arrays.reverse_array","b",".void",     "expose a = .string[]");
    ADDPROC(search_array,"arrays.search_array","b",  ".int",      "expose a = .string[],needle=.string,startrow=.int");
ENDLOADFUNCS