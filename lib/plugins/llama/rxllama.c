/* cREXX License (MIT). Session-affine RXPA adapter for llama.rexx. */
#include "crexxpa.h"
#include "bridge.h"
#include <string.h>
#include <stdlib.h>
#if defined(_MSC_VER)
#define LOCAL __declspec(thread)
#else
#define LOCAL __thread
#endif
typedef struct { void *vm; rxpa_string_view_v1 string_view; const rxpa_host_services_v1 *host; } adapter_session;
static LOCAL adapter_session *current_vm;
static void *unsupported_host(void) { return NULL; }
static void *create(const rxpa_host_services_v1 *host) {
    adapter_session *session;
    if (!rxpa_host_has_string_view(host)) return NULL;
    session = malloc(sizeof(*session));
    if (!session) return NULL;
    session->vm = rxllama_vm_create(); session->string_view = host->string_view; session->host = host;
    if (!session->vm) { free(session); return NULL; }
    return session;
}
static void destroy(void *opaque) {
    adapter_session *session = opaque;
    if (session) { rxllama_vm_destroy(session->vm); free(session); }
}
static int enter(void *vm, uint32_t caps, void **previous) {
    if (!vm || !previous || caps != RXPA_PROCEDURE_CAP_SESSION_AFFINE) return -1;
    *previous = current_vm; current_vm = vm; return 0;
}
static void leave(void *previous) { current_vm = previous; }
static uint32_t capabilities(const char *name) {
    return name ? RXPA_PROCEDURE_CAP_SESSION_AFFINE : 0;
}
RXPA_PLUGIN_SESSION_WITH_HOST(unsupported_host, destroy, enter, leave, capabilities, create)
static void copy_payload(void *, void *);
static void finalize_payload(void *);
static const rxpa_native_payload_ops payload_ops = {
    "crexx.rxllama.handle.v1", copy_payload, finalize_payload
};
static rxllama_token token(rxpa_attribute_value value) {
    size_t size = 0;
    const rxpa_native_payload_ops *ops = NULL;
    const void *data = GETNATIVEPAYLOAD(value, &size, &ops, NULL);
    rxllama_token result = {0, 0};
    if (data && size == sizeof(result) && ops == &payload_ops) memcpy(&result, data, size);
    return result;
}
static void copy_payload(void *destination, void *source) {
    rxllama_token t = token(source);
    if (rxllama_retain(t)) {
        if (SETNATIVEPAYLOAD(destination, &t, sizeof(t), &payload_ops, 0)) rxllama_release(t);
    }
}
static void finalize_payload(void *value) { rxllama_release(token(value)); }
/* Signature codes: h opaque handle, i integer, f float, s text, H/I/S output.
 * All public functions return status, including failures to publish handles. */
static void invoke(int op, const char *signature, rxinteger count, rxpa_attribute_value *args,
                   rxpa_attribute_value result, rxpa_attribute_value signal) {
    rxllama_argument inputs[6] = {{0}};
    rxllama_result output = {{0}, 0, "", "", ""};
    size_t i, n = strlen(signature);
    int status;
    if ((size_t)count != n || n > 6) {
        SETINT(result, -1); SETINT(signal, SIGNAL_NONE); SETSTRING(signal, ""); return;
    }
    if (!current_vm) { status = -7; goto finish; }
    for (i = 0; i < n; ++i) {
        switch (signature[i]) {
            case 'h': inputs[i].handle = token(args[i]); break;
            case 'i': inputs[i].integer = GETINT(args[i]); break;
            case 'f': inputs[i].number = GETFLOAT(args[i]); break;
            case 's':
                if (current_vm->string_view(args[i], &inputs[i].text, &inputs[i].text_length)) {
                    status = -1; goto finish;
                }
                break;
        }
    }
    status = rxllama_call(current_vm->vm, op, inputs, &output);
    if (!status) {
        if (op == RXLLAMA_DIAGNOSTIC) {
            SETINT(args[0], output.integer); SETSTRING(args[1], output.operation); SETSTRING(args[2], output.message);
        } else if (op == RXLLAMA_EMBEDDINGS) {
            size_t bytes = (size_t)output.value_count * sizeof(double);
            uint64_t begin = output.probes ? rxllama_probe_clock_ns() : 0;
            if (!ISINITIALIZED(args[1]) ||
                SETNATIVEPAYLOAD(args[1], output.values, bytes, NULL, 0)) status = -5;
            else {
                if (output.probes) {
                    output.probes->copy_ns += rxllama_probe_clock_ns() - begin;
                    output.probes->bytes += bytes; ++output.probes->copies;
                }
                SETINT(args[2], output.rows); SETINT(args[3], output.dimensions);
            }
        } else for (i = 0; i < n; ++i) {
            switch (signature[i]) {
                case 'H':
                    if (SETNATIVEPAYLOAD(args[i], &output.handle, sizeof(output.handle), &payload_ops, 0)) {
                        rxllama_release(output.handle); status = -5;
                    }
                    break;
                case 'I': SETINT(args[i], output.integer); break;
                case 'S': SETSTRING(args[i], output.text); break;
            }
        }
    }
finish:
    SETINT(result, status); SETINT(signal, SIGNAL_NONE); SETSTRING(signal, "");
}
#define WRAP(name, op, sig) PROCEDURE(name) { invoke(op, sig, NUM_ARGS, _arg, RETURN, SIGNAL); }
WRAP(configcreate, RXLLAMA_CONFIG_CREATE, "H")
WRAP(configint, RXLLAMA_CONFIG_INT, "hsi")
WRAP(configfloat, RXLLAMA_CONFIG_FLOAT, "hsf")
WRAP(configtext, RXLLAMA_CONFIG_TEXT, "hss")
WRAP(runtimeopen, RXLLAMA_RUNTIME_OPEN, "hH")
WRAP(devicecount, RXLLAMA_DEVICE_COUNT, "hI")
WRAP(deviceinfo, RXLLAMA_DEVICE_INFO, "hisS")
WRAP(modelopen, RXLLAMA_MODEL_OPEN, "hssshH")
WRAP(modelstate, RXLLAMA_MODEL_STATE, "hS")
WRAP(modelcancel, RXLLAMA_MODEL_CANCEL, "h")
WRAP(sessionopen, RXLLAMA_SESSION_OPEN, "hshH")
WRAP(prepare, RXLLAMA_PREPARE, "hiS")
WRAP(infotext, RXLLAMA_INFO_TEXT, "hsS")
WRAP(infoint, RXLLAMA_INFO_INT, "hsI")
WRAP(diagnostic, RXLLAMA_DIAGNOSTIC, "ISS")
WRAP(close_resource, RXLLAMA_CLOSE, "h")
WRAP(requestopen, RXLLAMA_REQUEST_OPEN, "hhH")
WRAP(addembedding, RXLLAMA_ADD_EMBEDDING, "hssI")
WRAP(submit, RXLLAMA_SUBMIT, "h")
WRAP(process, RXLLAMA_PROCESS, "hiS")
WRAP(embeddings, RXLLAMA_EMBEDDINGS, "hPII")
WRAP(cancel, RXLLAMA_CANCEL, "h")
#include "typed.h"
LOADFUNCS
LLAMA_TYPED_DECLARATIONS
ADDPROC(configcreate, "rxllama.configcreate", "b", ".int", "expose config=.binary");
ADDPROC(configint, "rxllama.configint", "b", ".int", "config=.binary, key=.string, value=.int");
ADDPROC(configfloat, "rxllama.configfloat", "b", ".int", "config=.binary, key=.string, value=.float");
ADDPROC(configtext, "rxllama.configtext", "b", ".int", "config=.binary, key=.string, value=.string");
ADDPROC(runtimeopen, "rxllama.runtimeopen", "b", ".int", "config=.binary, expose runtime=.binary");
ADDPROC(devicecount, "rxllama.devicecount", "b", ".int", "runtime=.binary, expose count=.int");
ADDPROC(deviceinfo, "rxllama.deviceinfo", "b", ".int", "runtime=.binary, index=.int, key=.string, expose value=.string");
ADDPROC(modelopen, "rxllama.modelopen", "b", ".int", "runtime=.binary, path=.string, sha256=.string, profile=.string, config=.binary, expose model=.binary");
ADDPROC(modelstate, "rxllama.modelstate", "b", ".int", "model=.binary, expose state=.string");
ADDPROC(modelcancel, "rxllama.modelcancel", "b", ".int", "model=.binary");
ADDPROC(sessionopen, "rxllama.sessionopen", "b", ".int", "model=.binary, capability=.string, config=.binary, expose session=.binary");
ADDPROC(prepare, "rxllama.prepare", "b", ".int", "session=.binary, work_tokens=.int, expose state=.string");
ADDPROC(infotext, "rxllama.infotext", "b", ".int", "resource=.binary, key=.string, expose value=.string");
ADDPROC(infoint, "rxllama.infoint", "b", ".int", "resource=.binary, key=.string, expose value=.int");
ADDPROC(diagnostic, "rxllama.diagnostic", "b", ".int", "expose code=.int, expose operation=.string, expose message=.string");
ADDPROC(close_resource, "rxllama.close", "b", ".int", "resource=.binary");
ADDPROC(requestopen, "rxllama.requestopen", "b", ".int", "session=.binary, config=.binary, expose request=.binary");
ADDPROC(addembedding, "rxllama.addembedding", "b", ".int", "request=.binary, text=.string, role=.string, expose row=.int");
ADDPROC(submit, "rxllama.submit", "b", ".int", "request=.binary");
ADDPROC(process, "rxllama.process", "b", ".int", "request=.binary, work_tokens=.int, expose state=.string");
ADDPROC(embeddings, "rxllama.embeddings", "b", ".int", "request=.binary, expose values=.packedfloat, expose rows=.int, expose dimensions=.int");
ADDPROC(cancel, "rxllama.cancel", "b", ".int", "request=.binary");
ENDLOADFUNCS
