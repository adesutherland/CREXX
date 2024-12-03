//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // For POSIX systems (Linux/macOS)
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file

/* -------------------------------------------------------------------------------------
 * Bubble Sorts as a poor example not to be used
 * -------------------------------------------------------------------------------------
 */
PROCEDURE(bubble_sort)
{
    int i, j;
    int size;

    // Check the number of arguments
    if (NUM_ARGS != 1) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")

    // Get the array size
    size = GETNUMATTRS(ARG0);

    // Bubble-sort the array
    for (i = 0; i < size - 1; i++) {
        for (j = 0; j < size - i - 1; j++) {
            if (GETSTRING(GETATTR(ARG0,j)) > GETSTRING(GETATTR(ARG0,j + 1))) {
                SWAPATTRS(ARG0, j, j + 1);
            }
        }
    }
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Shell Sort
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (shell_sort) {
    int from, to, offset;
    int i, j, gap;
    char *val1, *order;

    to     = GETARRAYHI(ARG0);   // number of contained array items
    offset = GETINT(ARG1)-1;     // make offset
    order  = GETSTRING(ARG2);    // sort order

    from = 1;
    for (gap = to / 2; gap > 0; gap /= 2) {
        for (i = gap; i < to; i++) {
            val1 = GETSARRAY(ARG0, i);
            for (j = i; j >= gap && strcmp(GETSARRAY(ARG0,j-gap)+offset, val1+offset) > 0; j -= gap) {
                SWAPARRAY(ARG0, j, j - gap);
            }
            SETSARRAY(ARG0, j, val1);
        }
    }
    if (order[0] == 'D' || order[0] == 'd') {
        to--;        // make index to offset
        j = to / 2;      // split in halfs
        for (i = 0; i <= j; ++i) {
            SWAPARRAY(ARG0, i, to);
            to--;
        }
    }
    PROCRETURN
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Reverse array order
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (reverse_array) {
    int i, k, to;
    to = GETARRAYHI(ARG0)-1; // make max index to offset
    k = to / 2;      // split in halfs
    for (i = 0; i <= k; ++i) {
        SWAPARRAY(ARG0, i, to);
        to--;
    }
     PROCRETURN
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Serach certain strin in array and return its index or zero
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (search_array) {
    int i, from, to;

    to  = GETARRAYHI(ARG0);
    from=GETINT(ARG2)-1;

    for (i = from; i < to; ++i) {
        if (strstr(GETSARRAY(ARG0, i),GETSTRING(ARG1))>0) {
            RETURNINT(i+1);
            PROCRETURN
        }
    }
    RETURNINT(0);
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Delete item(s) in an array and shift elements accordingly
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (delete_array) {
    int i, hi, from, to, del = 0, todel;
    hi = GETARRAYHI(ARG0) - 1; // make max index to offset
    from = GETINT(ARG1) - 1;     // get from offset
    to = GETINT(ARG2) - 1;     // get to offset
    if (from < 0 || from > hi || to < from || to < 0) {
        RETURNINT(0);
        PROCRETURN
    }
    if (from > hi) from = hi;
    if (to - from > hi) to = hi;
    todel = to - from + 1;
    while (todel >
           0) {                // position of index changes, therefore we always delete from, which is the last from+1
        REMOVEATTR(ARG0, from);
        del++;
        todel--;
    }
    RETURNINT(del);
    PROCRETURN
    ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Insert empty item(s) in an array and shift elements accordingly
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (insert_array) {
    int i, hi, from, new, del = 0, todel;
    hi = GETARRAYHI(ARG0) - 1; // make max index to offset
    from = GETINT(ARG1) - 1;     // get from offset
    new = GETINT(ARG2);       // lines to insert
    if (from < 0 || new < 1) {
        RETURNINT(0);
        PROCRETURN
    }
    if (from > hi) from = hi+1;
    for (i = from; i < from + new; ++i) {
        INSERTATTR(ARG0, i);
    }
    RETURNINT(new);
    PROCRETURN
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Copy an entire array or a range of into a target array
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (copy_array) {
    int i, j=0, hi, from, tto,add=0;
    char * val1;
    hi = GETARRAYHI(ARG0);
    from= GETINT(ARG2)-1;
    tto = GETINT(ARG3)-1;

    if (from < 0 || from>hi || from > tto) {
        RETURNINT(0);
        PROCRETURN
    }

    if (tto>hi) tto=hi-1;

    SETARRAYHI(ARG1,tto-from+1);

    for (i = from ; i <= tto; ++i,++j) {
        val1 = GETSARRAY(ARG0, i);
        SETSARRAY(ARG1, j,val1);
        add++;
    }
    SETARRAYHI(ARG1,add);
    RETURNINT(add);
    PROCRETURN
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Merge an array into an existing one, both arrays must be sorted
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (merge_array) {
    int i, j, hi1,hi2;
    hi1 = GETARRAYHI(ARG0);
    hi2 = GETARRAYHI(ARG1);
    for (i = 0, j=0 ; i < hi1+hi2 && j<hi2; ++i) {
         if (strcmp(GETSARRAY(ARG0, i), GETSARRAY(ARG1, j)) > 0) {
            INSERTSARRAY(ARG0, i, GETSARRAY(ARG1, j));  // insert at position i a new entry
            j++;
        }
    }
    hi1=GETARRAYHI(ARG0);
    for (j = j; j < hi2; ++j,++hi1) {
        INSERTSARRAY(ARG0, hi1, GETSARRAY(ARG1, j));  // insert at position hi1 a new entry
    }
    PROCRETURN;
    RETURNINT(0);
    ENDPROC
}
/* -------------------------------------------------------------------------------------
 * list contents of an array
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (list_array)  {
    int i,from,to,hi;
    hi  = GETARRAYHI(ARG0);
    from= GETINT(ARG1);
    to  = GETINT(ARG2);
    if (to<=0) to=hi;

    if (from<1) from=1;
    if (from>hi) from=hi;
    if (to>hi || to<1) to=hi;
    printf("      Entries of String Array \n");
    printf("Entry     Data   Range %d-%d\n",from,to);
    printf("-------------------------------------------------------\n");
    for (i=from-1;i<to;i++) {
        printf("%0.7d   %s\n",i+1, GETSARRAY(ARG0,i));
    }
    printf("%d Entries\n",to);
}
/* -------------------------------------------------------------------------------------
 * Functions to be provided to rexx
 * -------------------------------------------------------------------------------------
 */
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
    ADDPROC(merge_array,  "arrays.merge_array",  "b",  ".int",      "expose a = .string[],expose b=.string[]");
    ADDPROC(list_array,   "arrays.list_array",   "b",  ".void",     "expose a = .string[],from=.int,tto=.int");
ENDLOADFUNCS