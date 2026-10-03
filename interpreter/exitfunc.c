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
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include "rxpa.h"
#include "rxvmintp.h"
#include "platform.h"

// Function Prototypes
void say_exit_default(char* message); // Default say exit function

#if defined(CREXX_VM_SINGLE_THREADED)
#define RXVM_THREAD_LOCAL
#elif defined(_MSC_VER)
#define RXVM_THREAD_LOCAL __declspec(thread)
#else
#define RXVM_THREAD_LOCAL __thread
#endif

/* Compatibility default for callers which configure output before VM entry. */
static RXVM_THREAD_LOCAL say_exit_func thread_say_exit = say_exit_default;

static void rxvm_say_output_error(void) {
    raise_signal(errno == EILSEQ ? RXSIGNAL_UNICODE_ERROR : RXSIGNAL_NOTREADY);
}

/* Default Say Exit Function - prints to stdout */
void say_exit_default(char* message) {
    /* Print the message to stdout without a newline or any formatting */
    errno = 0;
    if (platform_console_text_write(stdout, message, strlen(message))) {
        rxvm_say_output_error();
        return;
    }
    // Flush
    if (fflush(stdout)) raise_signal(RXSIGNAL_NOTREADY);
}

/* Set the say exit function */
void rxvm_setsayexit(say_exit_func sayExitFunc) {
    rxvm_context *context = rxvm_active_context_current();
    say_exit_func selected = sayExitFunc ? sayExitFunc : say_exit_default;
    if (context) {
        context->active.say_exit = selected;
        context->active.say_exit_bytes = 0;
    }
    else thread_say_exit = selected;
}

/* Reset the say exit function */
void rxvm_resetsayexit() {
    rxvm_context *context = rxvm_active_context_current();
    if (context) {
        context->active.say_exit = 0;
        context->active.say_exit_bytes = 0;
    }
    else thread_say_exit = say_exit_default;
}

/* SAY and SAYX are byte spans in the VM. The legacy callback can only express
 * terminated text, so reject an embedded NUL before invoking it. */
void rxvm_say_write(const char *message, size_t length, int newline) {
    rxvm_context *context = rxvm_active_context_current();
    void (*bytes_exit)(const char *, size_t) =
        context ? context->active.say_exit_bytes : NULL;
    say_exit_func legacy_exit = context && context->active.say_exit
                                    ? context->active.say_exit
                                    : thread_say_exit;
    size_t output_length;
    char fixed_buffer[100];
    char *buffer;

    if (!message && length) {
        errno = EINVAL;
        rxvm_say_output_error();
        return;
    }
    if (length > SIZE_MAX - (newline ? 2u : 1u)) {
        errno = ENOMEM;
        rxvm_say_output_error();
        return;
    }
    output_length = length + (newline ? 1u : 0u);
    if (!bytes_exit && (!legacy_exit || legacy_exit == say_exit_default)) {
        errno = 0;
        if ((length && platform_console_text_write(stdout, message, length)) ||
            (newline && platform_console_text_write(stdout, "\n", 1u))) {
            rxvm_say_output_error();
            return;
        }
        if (fflush(stdout)) raise_signal(RXSIGNAL_NOTREADY);
        return;
    }
    if (!bytes_exit && length && memchr(message, 0, length)) {
        errno = EINVAL;
        rxvm_say_output_error();
        return;
    }

    buffer = output_length + 1u <= sizeof(fixed_buffer)
                 ? fixed_buffer
                 : rxvm_memory_alloc_bytes(rxvm_memory_current_worker(),
                                           output_length + 1u);
    if (!buffer) {
        errno = ENOMEM;
        rxvm_say_output_error();
        return;
    }
    if (length) memcpy(buffer, message, length);
    if (newline) buffer[length] = '\n';
    buffer[output_length] = 0;
    if (bytes_exit) bytes_exit(buffer, output_length);
    else legacy_exit(buffer);
    if (buffer != fixed_buffer) (void)rxvm_memory_release(buffer);
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
        rxvm_say_write(buffer, needed_len - 1u, 0);
        (void)rxvm_memory_release(buffer);
    }
    else {
        rxvm_say_write(fixed_buffer, needed_len - 1u, 0);
    }
}
