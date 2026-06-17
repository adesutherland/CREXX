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

//
// Created by Adrian Sutherland on 01/04/2025.
//

#ifndef RXSIGNAL_H
#define RXSIGNAL_H

/* Interrupt / Signal Codes */
#define RXSIGNAL_NONE                 0  /* No Signal */
#define RXSIGNAL_KILL                 1  /* Unmaskable Kill */
#define RXSIGNAL_FAILURE              2  /* Error in an external function or subroutine */
#define RXSIGNAL_ERROR                3  /* Syntax error */
#define RXSIGNAL_OVERFLOW_UNDERFLOW   4  /* Numeric overflow or underflow */
#define RXSIGNAL_DIVISION_BY_ZERO     5  /* Divide by zero */
#define RXSIGNAL_CONVERSION_ERROR     6  /* Conversion error between types */
#define RXSIGNAL_INVALID_ARGUMENTS    7  /* Invalid arguments passed to a function or subroutine */
#define RXSIGNAL_OUT_OF_RANGE         8  /* Access a child element out of range */
#define RXSIGNAL_UNICODE_ERROR        9  /* Unicode error */

#define RXSIGNAL_UNKNOWN_INSTRUCTION  10 /* An unknown RXAS instruction - interpreter mode only */
#define RXSIGNAL_FUNCTION_NOT_FOUND   11 /* An unknown function */
#define RXSIGNAL_NOT_IMPLEMENTED      12 /* Instruction not implemented */
#define RXSIGNAL_INVALID_SIGNAL_CODE  13 /* Invalid signal code */
#define RXSIGNAL_REFERENCE_INVALID    14 /* Reference value no longer points to live storage */

#define RXSIGNAL_NOTREADY             15 /* Input/output error, e.g., file not ready (POSIX SIGPIPE) */
#define RXSIGNAL_OBJECT_NOT_INITIALIZED 16 /* Object value has a compatible type but no initialized instance */

#define RXSIGNAL_QUIT                 19 /* Request to halt execution (POSIX SIGQUIT / Windows SIGBREAK) */
#define RXSIGNAL_TERM                 20 /* Request to halt execution (POSIX SIGTERM) */
#define RXSIGNAL_POSIX_INT            21 /* POSIX SIGINT (user interrupt) */
#define RXSIGNAL_POSIX_HUP            22 /* POSIX SIGHUP (reload configuration) */
#define RXSIGNAL_POSIX_USR1           23 /* POSIX SIGUSR1 (user-defined signal 1) */
#define RXSIGNAL_POSIX_USR2           24 /* POSIX SIGUSR2 (user-defined signal 2) */
#define RXSIGNAL_POSIX_CHLD           25 /* POSIX SIGCHLD (child process terminated) */

#define RXSIGNAL_OTHER                30 /* Other Interrupt */
#define RXSIGNAL_BREAKPOINT           31 /* Breakpoint (called after each instruction if enabled) */
#define RXSIGNAL_MAX                  32 /* Maximum Interrupt Code - this is not a valid signal code */

#endif //RXSIGNAL_H
