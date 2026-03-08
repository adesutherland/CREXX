// rxpa (plugin architecture) test plugin 1

#include <stdio.h>
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file

// Procedure to echo a string
PROCEDURE(echo)
{
    char buffer[100];
    if( NUM_ARGS != 1) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")
    strcpy(buffer, "In plugin2.echo()\n");
    strcat(buffer, GETSTRING(ARG(0)));
    SETSTRING(RETURN, buffer);
    RESETSIGNAL
}

// Functions to be provided to rexx - these are loaded either when the plugin is loaded (dynamic) or
// before main() is called (static)
LOADFUNCS
//      C Function__, REXX namespace & name, Option_, Return Type_, Arguments
ADDPROC(echo, "multplugin2.plugin2_echo", "b", ".string", "s=.string");
ENDLOADFUNCS
