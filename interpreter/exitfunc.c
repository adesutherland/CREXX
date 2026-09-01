/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Exit Function Support */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "rxpa.h"
#include "rxvmintp.h"

// Function Prototypes
void say_exit_default(char* message); // Default say exit function

#if defined(_MSC_VER)
#define RXVM_THREAD_LOCAL __declspec(thread)
#else
#define RXVM_THREAD_LOCAL __thread
#endif

/* Compatibility default for callers which configure output before VM entry. */
static RXVM_THREAD_LOCAL say_exit_func thread_say_exit = say_exit_default;

static say_exit_func rxvm_current_say_exit(void) {
    rxvm_context *context = rxvm_active_context_current();
    if (context && context->active.say_exit) return context->active.say_exit;
    return thread_say_exit ? thread_say_exit : say_exit_default;
}

/* Default Say Exit Function - prints to stdout */
void say_exit_default(char* message) {
    /* Print the message to stdout without a newline or any formatting */
    printf("%s", message);
    // Flush
    fflush(stdout);
}

/* Set the say exit function */
void rxvm_setsayexit(say_exit_func sayExitFunc) {
    rxvm_context *context = rxvm_active_context_current();
    say_exit_func selected = sayExitFunc ? sayExitFunc : say_exit_default;
    if (context) context->active.say_exit = selected;
    else thread_say_exit = selected;
}

/* Reset the say exit function */
void rxvm_resetsayexit() {
    rxvm_context *context = rxvm_active_context_current();
    if (context) context->active.say_exit = 0;
    else thread_say_exit = say_exit_default;
}

/* printf replacement - prints to the say exit function (or stdout) */
#define FIXED_BUFFER_SIZE 100 // Fixed buffer size for small messages
void rxvm_mprintf(const char* format, ...) {
    char *buffer;
    char fixed_buffer[FIXED_BUFFER_SIZE];
    size_t needed_len;
    va_list argptr;

    va_start(argptr, format);
    needed_len = vsnprintf(fixed_buffer, FIXED_BUFFER_SIZE, format, argptr) + 1;
    va_end(argptr);
    if (needed_len > FIXED_BUFFER_SIZE) {
        /* Buffer not big enough - do it again with a dynamic buffer now we know the size needed */
        buffer = rxvm_memory_alloc_bytes(rxvm_memory_current_worker(),
                                         needed_len);
        if (!buffer) return;
        va_start(argptr, format);
        vsnprintf(buffer, needed_len, format, argptr);
        va_end(argptr);
        rxvm_current_say_exit()(buffer);
        (void)rxvm_memory_release(buffer);
    }
    else {
        rxvm_current_say_exit()(fixed_buffer);
    }
}
