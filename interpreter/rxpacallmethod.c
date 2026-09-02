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

/*
 * RXPA's nested method-call bridge belongs to the complete VM core because it
 * re-enters the interpreter. Keep it separate from rxpafuncs.c: that file also
 * supplies context-light value and UTF-8 helpers used by focused unit targets.
 */
#include "crexxpa.h"
#include "rxvmintp.h"
#include "rxvmvars.h"

static void rxpa_callmethod_set_signal(value *signal,
                                       rxsignal code,
                                       const char *message) {
    if (!signal) return;
    if (signal->num_attributes) set_num_attributes(signal, 0);
    set_int(signal, (rxinteger)code);
    set_null_string(signal, message ? message : "");
}

int rxvm_callmethod(rxpa_attribute_value receiver,
                    const char *method_descriptor,
                    rxinteger argc,
                    rxpa_attribute_value *args,
                    rxpa_attribute_value result,
                    rxpa_attribute_value signal) {
    rxvm_context *context = rxvm_active_context_current();
    int signal_code;

    if (!context || argc < 0) {
        rxpa_callmethod_set_signal((value*)signal, SIGNAL_INVALID_ARGUMENTS,
                                   "CALLMETHOD requires an active RXVM call");
        return -1;
    }

    signal_code = rxvm_invoke_method_descriptor(
            context,
            (value*)receiver,
            method_descriptor,
            (size_t)argc,
            (value**)args,
            (value*)result);
    if (signal_code != SIGNAL_NONE) {
        const char *message = "CALLMETHOD failed";
        if (signal_code == SIGNAL_FUNCTION_NOT_FOUND) {
            message = "CALLMETHOD could not resolve the method descriptor";
        } else if (signal_code == SIGNAL_OBJECT_NOT_INITIALIZED) {
            message = "CALLMETHOD receiver is not initialized";
        } else if (signal_code == SIGNAL_INVALID_ARGUMENTS) {
            message = "CALLMETHOD received invalid arguments";
        }
        rxpa_callmethod_set_signal((value*)signal, (rxsignal)signal_code,
                                   message);
        return -1;
    }
    return 0;
}
