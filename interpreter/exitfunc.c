/* Exit Function Support */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "rxpa.h"

// Function Prototypes
void say_exit_default(char* message); // Default say exit function

// Global Variables
say_exit_func say_exit = say_exit_default;

/* Default Say Exit Function - prints to stdout */
void say_exit_default(char* message) {
    /* Print the message to stdout without a newline or any formatting */
    printf("%s", message);
    // Flush
    fflush(stdout);
}

/* Set the say exit function */
void rxpa_setsayexit(say_exit_func sayExitFunc) {
    say_exit = sayExitFunc;
}

/* Reset the say exit function */
void rxpa_resetsayexit() {
    say_exit = say_exit_default;
}

/* printf replacement - prints to the say exit function (or stdout) */
#define FIXED_BUFFER_SIZE 100 // Fixed buffer size for small messages
void mprintf(const char* format, ...) {
    char *buffer;
    char fixed_buffer[FIXED_BUFFER_SIZE];
    size_t needed_len;
    va_list argptr;

    va_start(argptr, format);
    needed_len = vsnprintf(fixed_buffer, FIXED_BUFFER_SIZE, format, argptr) + 1;
    va_end(argptr);
    if (needed_len > FIXED_BUFFER_SIZE) {
        /* Buffer not big enough - do it again with a dynamic buffer now we know the size needed */
        buffer = malloc(needed_len);
        va_start(argptr, format);
        vsnprintf(buffer, needed_len, format, argptr);
        va_end(argptr);
        say_exit(buffer);
        free(buffer);
    }
    else {
        say_exit(fixed_buffer);
    }
}
