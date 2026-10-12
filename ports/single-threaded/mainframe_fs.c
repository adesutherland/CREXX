/* MIT licence. The filesystem subset required by the canonical Classic/G
 * libraries on CMS/TSO. Logical native names use the platform open boundary;
 * unresolved pathname services fail rather than invent a POSIX directory.
 * This does not claim the desktop rxfs filesystem/locking contract. */
#include "crexxpa.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#undef INITIALIZER
#define INITIALIZER(f) static void f(void) {

PROCEDURE(native_abspath) {
    if(NUM_ARGS!=1) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"RXFS.ABSPATH expects one path") }
    RETURNSIGNAL(SIGNAL_FAILURE,"Native absolute pathname resolution is unavailable")
}
PROCEDURE(native_isfile) {
    FILE *file;
    if(NUM_ARGS!=1) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"RXFS.ISFILE expects one path") }
    file=platform_fopen(GETSTRING(ARG0),"rb");
    if(file) { fclose(file); RETURNINT(1); }
    else RETURNINT(0);
    RESETSIGNAL
}
PROCEDURE(native_loadpath) {
    if(NUM_ARGS!=0) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,"RXFS.LOADPATH expects no arguments") }
    RETURNSIGNAL(SIGNAL_FAILURE,"Native executable pathname is unavailable")
}
LOADFUNCS
ADDPROC(native_abspath,"rxfs.abspath","b",".string","path=.string");
ADDPROC(native_isfile,"rxfs.isfile","b",".int","path=.string");
ADDPROC(native_loadpath,"rxfs.loadpath","b",".string","");
ENDLOADFUNCS
void crexx_mainframe_fs_init(void) {
    static int initialized;
    if(initialized) return;
    initialized=1;
    UNIQUE_INIT_FUNCTION_NAME(PLUGIN_ID)();
}
