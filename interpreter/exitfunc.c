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

#if defined(CREXX_VM_SINGLE_THREADED)
#define RXVM_THREAD_LOCAL
#elif defined(_MSC_VER)
#define RXVM_THREAD_LOCAL __declspec(thread)
#else
#define RXVM_THREAD_LOCAL __thread
#endif

/* Callers may configure output before VM entry. NULL selects the console. */
static RXVM_THREAD_LOCAL say_exit_bytes_func thread_say_exit_bytes;

static void rxvm_say_output_error(void) {
    raise_signal(errno == EILSEQ ? RXSIGNAL_UNICODE_ERROR : RXSIGNAL_NOTREADY);
}

/* Only VM text-console routes enter here. Native diagnostics and binary I/O
 * retain their existing platform/stdio owners. Stream IDs match RXVML/SAA. */
int rxvm_console_text_write(rxvm_context *context, FILE *stream,
                            const char *bytes, size_t length) {
    if (!bytes && length) {
        errno = EINVAL;
        return -1;
    }
    if (context && context->active.output_writer &&
        (stream == stdout || stream == stderr)) {
        if (context->active.output_writer(stream == stderr ? 2 : 1,
                                          bytes ? bytes : "", length,
                                          context->active.output_userdata)) {
            errno = EIO;
            return -1;
        }
        return 0;
    }
    return platform_console_text_write(stream, bytes, length);
}

int rxvm_console_flush(rxvm_context *context, FILE *stream) {
    if (context && context->active.output_writer &&
        (stream == stdout || stream == stderr)) return 0;
    return fflush(stream);
}

/* Set the say exit function */
void rxvm_setsayexit_bytes(say_exit_bytes_func sayExitFunc) {
    rxvm_context *context = rxvm_active_context_current();
    if (context) context->active.say_exit_bytes = sayExitFunc;
    else thread_say_exit_bytes = sayExitFunc;
}

/* Reset the say exit function */
void rxvm_resetsayexit() {
    rxvm_context *context = rxvm_active_context_current();
    if (context) context->active.say_exit_bytes = 0;
    else thread_say_exit_bytes = 0;
}

/* SAY and SAYX are byte spans in the VM. */
int rxvm_say_write_checked(rxvm_context *context, const char *message,
                           size_t length, int newline) {
    say_exit_bytes_func bytes_exit = context && context->active.say_exit_bytes
                                         ? context->active.say_exit_bytes
                                         : thread_say_exit_bytes;
    size_t output_length;
    char fixed_buffer[100];
    char *buffer;
    int output_status = 0;

    if (!message && length) {
        errno = EINVAL;
        return -1;
    }
    if (length > SIZE_MAX - (newline ? 2u : 1u)) {
        errno = ENOMEM;
        return -1;
    }
    output_length = length + (newline ? 1u : 0u);
    if (!bytes_exit && !(context && context->active.output_writer)) {
        errno = 0;
        if ((length && rxvm_console_text_write(context, stdout, message, length)) ||
            (newline && rxvm_console_text_write(context, stdout, "\n", 1u))) {
            return -1;
        }
        if (rxvm_console_flush(context, stdout)) {
            errno = EIO;
            return -1;
        }
        return 0;
    }
    buffer = output_length + 1u <= sizeof(fixed_buffer)
                 ? fixed_buffer
                 : rxvm_memory_alloc_bytes(context ? context->worker.memory_worker
                                                  : rxvm_memory_current_worker(),
                                           output_length + 1u);
    if (!buffer) {
        errno = ENOMEM;
        return -1;
    }
    if (length) memcpy(buffer, message, length);
    if (newline) buffer[length] = '\n';
    buffer[output_length] = 0;
    if (bytes_exit) bytes_exit(buffer, output_length);
    else output_status = rxvm_console_text_write(context, stdout, buffer, output_length);
    if (buffer != fixed_buffer) (void)rxvm_memory_release(buffer);
    if (output_status) errno = EIO;
    return output_status;
}

void rxvm_say_write(const char *message, size_t length, int newline) {
    if (rxvm_say_write_checked(rxvm_active_context_current(), message, length, newline))
        rxvm_say_output_error();
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
