//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // For POSIX systems (Linux/macOS)
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file
#include <math.h>

/* ----------------------------------------------------------------------
 * STACK primitve Linked List
 * * --------------------------------------------------------------------------------------------
 */

PROCEDURE(additem) {
    int hi;
    hi = GETARRAYHI(ARG0)+1;
    SETARRAYHI(ARG0, hi);
    SETSARRAY(ARG0, hi-1, GETSTRING(ARG1));
    RETURNINT(hi);
    PROCRETURN
ENDPROC
}
PROCEDURE(queue) {
    INSERTATTR(ARG0, 0);
    SETSARRAY(ARG0, 0, GETSTRING(ARG1));
    RETURNINT(1);
    PROCRETURN
    ENDPROC
}
PROCEDURE(insertitem) {
    int hi, indx;
    hi = GETARRAYHI(ARG0);
    indx = GETINT(ARG1)+1;
    if (indx > hi) indx = hi + 1;
    if (indx < 0) indx = 0;
    if (indx <= hi) {
        INSERTATTR(ARG0, indx - 1);   // insert sets new hi directly (at least it looks like)
        SETSARRAY(ARG0, indx - 1, GETSTRING(ARG2));  // offset = index -1
    } else {
        SETARRAYHI(ARG0, hi + 1);
        SETSARRAY(ARG0, indx - 1, GETSTRING(ARG2));  // offset = index -1
    }
    RETURNINT(indx);
    PROCRETURN
    ENDPROC
}
PROCEDURE(delitem) {
    int hi, indx;
    hi = GETARRAYHI(ARG0);
    indx = GETINT(ARG1);
    if (indx > hi || indx <=0) {
        RETURNINT(4);
        PROCRETURN
    }
    REMOVEATTR(ARG0, indx - 1);   // remove entry
    RETURNINT(indx);              // set to next item after deleted one
    PROCRETURN
ENDPROC
}
PROCEDURE(pull) {
    int hi;
    hi = GETARRAYHI(ARG0);
    RETURNSTR(GETSARRAY(ARG0, hi - 1));     // set to next item after deleted one
    REMOVEATTR(ARG0, hi - 1);           // remove entry
    PROCRETURN
ENDPROC
}
PROCEDURE(pullq) {
    RETURNSTR(GETSARRAY(ARG0,0));     // set to next item after deleted one
    REMOVEATTR(ARG0, 0);              // remove entry
    PROCRETURN
ENDPROC
}
PROCEDURE(moveitem) {
    int hi, from,to;
    char * val;
    hi   = GETARRAYHI(ARG0);
    from = GETINT(ARG1);
    to   = GETINT(ARG2);
    if (from > hi || from <=0) {
        RETURNINT(4);
        PROCRETURN
    }
    if (to>hi) to=hi;
    val= GETSARRAY(ARG0,from-1);
    INSERTATTR(ARG0, to);   // insert sets new hi directly (at least it looks like)
    SETSARRAY(ARG0, to, val);  // offset = index -1
    if (to>from) REMOVEATTR(ARG0,from-1);
    else  REMOVEATTR(ARG0,from);
    RETURNINT(to);
    PROCRETURN
ENDPROC
}

PROCEDURE(list) {
        int i,hi;
        hi  = GETARRAYHI(ARG0);
        printf("      Entries of Stack \n");
        printf("Entry     Data   \n");
        printf("-------------------------------------------------------\n");
        for (i=0;i<hi;i++) {
            printf("%0.7d   %s\n",i+1, GETSARRAY(ARG0,i));
        }
        printf("%d Entries\n",hi);
}
PROCEDURE(finditem) {
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
ENDPROC}

PROCEDURE(swapitem) {
    int hi,ix1,ix2;

    hi  = GETARRAYHI(ARG0);
    ix1=GETINT(ARG1);
    ix2=GETINT(ARG2);

    if (ix1>hi || ix1<1 || ix2>hi || ix2<1){
        RETURNINT(4);
        PROCRETURN
    }
    SWAPARRAY(ARG0, ix1-1, ix2-1);
    RETURNINT(0);
ENDPROC}

PROCEDURE(create) {
    int hi, i;
    hi = GETARRAYHI(ARG0);
    for (i = 0; i < hi; ++i) {
        REMOVEATTR(ARG0, 0);   // remove entry
        printf("CREATE %d %d \n",i,GETARRAYHI(ARG0));
    }
    PROCRETURN
ENDPROC
}

// Linked List definitions
LOADFUNCS
    ADDPROC(additem,  "stack.additem",  "b",     ".int",   "expose list=.string[],ll_arg=.string");
    ADDPROC(additem,  "stack.push",     "b",     ".int",   "expose list=.string[],ll_arg=.string");
    ADDPROC(pull,     "stack.pull",     "b",     ".string","expose list=.string[]");
    ADDPROC(pullq,    "stack.pullq",    "b",     ".string","expose list=.string[]");
    ADDPROC(queue,    "stack.queue",    "b",    ".string","expose list=.string[],ll_arg=.string");
    ADDPROC(insertitem,"stack.insertitem", "b", ".int",   "expose list=.string[],ll_index=.int,ll_arg=.string");
    ADDPROC(delitem,  "stack.delitem",  "b",    ".int",   "expose list=.string[],ll_index=.int");
    ADDPROC(moveitem, "stack.moveitem", "b",    ".int",   "expose list=.string[],ll_from=.int,ll_to=.int");
    ADDPROC(list,     "stack.listitems","b",    ".int",   "expose list=.string[]");
    ADDPROC(finditem, "stack.finditem", "b",    ".int",   "expose list=.string[],ll_arg=.string,ll_index=.int");
    ADDPROC(swapitem, "stack.swapitem", "b",    ".int",   "expose list=.string[],ll_indx1=.int,ll_indx2=.int");
    ADDPROC(create,   "stack.createll", "b",    ".int",   "expose list=.string[],ll_arg=.string");
ENDLOADFUNCS