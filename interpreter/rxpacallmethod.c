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

static int native_module_declares_class(const module_file *file, const char *name) {
    int offset;
    size_t length = strlen(name);
    if (!file || !file->native) return 0;
    /* Native metadata is created by the checked loader callbacks, not read
     * from an unchecked RXBIN. Only the declaring module needs a graph. */
    for (offset = file->header.meta_head; offset != -1;) {
        const meta_entry *entry = (const meta_entry *)(file->constant + offset);
        if (entry->base.type == META_CLASS) {
            const meta_class_constant *cls = (const meta_class_constant *)entry;
            const string_constant *symbol = (const string_constant *)(file->constant + cls->symbol);
            if (symbol->string_len == length && !memcmp(symbol->string, name, length)) return 1;
        }
        offset = entry->next;
    }
    return 0;
}

/* Native construction is deliberately separate from the context-light RXPA
 * value helpers. The descriptor belongs to a loaded immutable graph; it is
 * never plugin-owned and cannot outlive this VM's values. Resolve everything
 * before touching the destination, including rejecting interface/builtin types.
 * This is the same final publication as SETOBJTYPE, with a checked name lookup
 * for C callers that cannot embed an RXAS graph operand. */
int rxvm_object_set_type(rxpa_attribute_value destination,
                         const char *class_name) {
    rxvm_context *context = rxvm_active_context_current();
    const RxGraphTypeRef *type_ref = NULL;
    char *canonical;
    size_t index;
    if (!context || !destination || !class_name || !*class_name) return -1;
    canonical = rx_graph_normalize_type_name(class_name);
    if (!canonical) return -1;
    for (index = 0; index < context->num_modules; ++index) {
        module_file *file = context->modules[index]->file;
        const RxGraph *graph;
        if (!file) continue;
        if (!file->semantic_graph && native_module_declares_class(file, canonical)) {
            char *error = NULL;
            /* Native module files are VM-local and immutable after load.
             * Cache once on that owner; free_module releases it after values.
             * A failed build never publishes an incomplete graph. */
            file->semantic_graph = rx_graph_build_crexx(&file, 1u, &error);
            free(error);
        }
        graph = file->semantic_graph;
        if (!graph) continue;
        RxGraphId type = rx_graph_find_type(graph, canonical);
        if (type != RX_GRAPH_NONE && rx_graph_type_kind(graph, type) == RX_GRAPH_TYPE_CLASS) {
            type_ref = rx_graph_type_ref(graph, type);
            break;
        }
    }
    free(canonical);
    if (!type_ref) return -1;
    ((value *)destination)->object_type = type_ref;
    clear_value_uninitialized_object((value *)destination);
    return 0;
}

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
