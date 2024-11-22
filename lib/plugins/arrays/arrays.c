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

    to     = GETARRAYHI(ARG(0));   // number of contained array items
    offset = GETINT(ARG(1))-1;     // make offset
    order  = GETSTRING(ARG(2));    // sort order

    from = 1;
    k = (from + to) / 2;
    while (k > 0) {
        for (;;) {
            complete = 1;
            for (i = 0; i < to - k; ++i) {
                j = i + k;
                val1 = GETSARRAY(ARG(0), i);
                val2 = GETSARRAY(ARG(0), j);
                if (strcmp(val1 + offset, val2 + offset) > 0) {
                    SWAPARRAY(ARG(0), i, j);
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
            SWAPARRAY(ARG(0), i, to);
            to--;
        }
    }
    PROCRETURN
ENDPROC
}
PROCEDURE (reverse_array) {
    int i, k, to;
    to = GETARRAYHI(ARG(0))-1; // make max index to offset
    k = to / 2;      // split in halfs
    for (i = 0; i <= k; ++i) {
        SWAPARRAY(ARG(0), i, to);
        to--;
    }
     PROCRETURN
ENDPROC
}
PROCEDURE (search_array) {
    int i, from, to;

    to  = GETARRAYHI(ARG(0));
    from=GETINT(ARG(2))-1;

    for (i = from; i < to; ++i) {
        if (strstr(GETSARRAY(ARG(0), i),GETSTRING(ARG(1)))>0) {
            RETURNINT(i+1);
            PROCRETURN
        }
    }
    RETURNINT(0);
ENDPROC
}
PROCEDURE (delete_array) {
    int i, hi, from, to, del = 0, todel;
    hi = GETARRAYHI(ARG(0)) - 1; // make max index to offset
    from = GETINT(ARG(1)) - 1;     // get from offset
    to = GETINT(ARG(2)) - 1;     // get to offset
    if (from < 0 || from > hi || to < from || to < 0) {
        RETURNINT(0);
        PROCRETURN
    }
    if (from > hi) from = hi;
    if (to - from > hi) to = hi;
    todel = to - from + 1;
    while (todel >
           0) {                // position of index changes, therefore we always delete from, which is the last from+1
        REMOVEATTR(ARG(0), from);
        del++;
        todel--;
    }
    RETURNINT(del);
    PROCRETURN
    ENDPROC
}
PROCEDURE (insert_array) {
    int i, hi, from, new, del = 0, todel;
    hi = GETARRAYHI(ARG(0)) - 1; // make max index to offset
    from = GETINT(ARG(1)) - 1;     // get from offset
    new = GETINT(ARG(2));       // lines to insert
    if (from < 0 || new < 1) {
        RETURNINT(0);
        PROCRETURN
    }
    if (from > hi) from = hi+1;
    for (i = from; i < from + new; ++i) {
        INSERTATTR(ARG(0), i);
    }
    RETURNINT(new);
    PROCRETURN
ENDPROC
}
PROCEDURE (copy_array) {
    int i, j=0, hi, from, tto,add=0;
    char * val1;
    hi = GETARRAYHI(ARG(0));
    from= GETINT(ARG(2))-1;
    tto = GETINT(ARG(3))-1;

    if (from < 0 || from>hi || from > tto) {
        RETURNINT(0);
        PROCRETURN
    }

    if (tto>hi) tto=hi-1;

    SETARRAYHI(ARG(1),tto-from+1);

    for (i = from ; i <= tto; ++i,++j) {
        val1 = GETSARRAY(ARG(0), i);
        SETSARRAY(ARG(1), j,val1);
        add++;
    }
    SETARRAYHI(ARG(1),add);
    RETURNINT(add);
    PROCRETURN
ENDPROC
}
// Functions to be provided to rexx
LOADFUNCS
//      C Function, REXX namespace & name,      Option,Return Type, Arguments
//  !! Do not use "to" in the parm-list, make it for example "tto", else compile fails: "expose a = .string[],from=.int,tto=.int"
    ADDPROC(delete_array, "arrays.delete_array", "b",  ".int",      "expose a = .string[],from=.int,tto=.int");
    ADDPROC(insert_array, "arrays.insert_array", "b",  ".int",      "expose a = .string[],from=.int,new=.int");
    ADDPROC(bubble_sort,  "arrays.bubble_sort",  "b",  ".void",     "expose a = .string[]");
    ADDPROC(shell_sort,   "arrays.shell_sort",   "b",  ".void",     "expose a = .string[], offset=.int, order=.string");
    ADDPROC(reverse_array,"arrays.reverse_array","b",  ".void",     "expose a = .string[]");
    ADDPROC(search_array, "arrays.search_array", "b",  ".int",      "expose a = .string[],needle=.string,startrow=.int");
    ADDPROC(copy_array,   "arrays.copy_array",   "b",  ".int",      "expose a = .string[],b=.string[],from=.int,tto=.int");
ENDLOADFUNCS