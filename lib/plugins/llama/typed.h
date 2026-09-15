/* cREXX License (MIT). Typed llama.rexx facade, included by the RXPA adapter.
 *
 * Native owners use the same checked token and payload hooks as rxllama.
 * Attributes 0..2 hold this value's last status/operation/message. Copying an
 * owner aliases the resource while retaining a private diagnostic snapshot.
 * Results own packed bytes; attributes 3..4 contain rows and dimensions.
 * There is no second native registry or inference implementation here.
 */
#ifndef DECL_ONLY
static void typed_diagnostic(rxpa_attribute_value value, int code,
                             const char *operation, const char *message) {
    SETINT(GETATTR(value, 0), code);
    SETSTRING(GETATTR(value, 1), operation ? operation : "");
    SETSTRING(GETATTR(value, 2), message ? message : "");
}
static int typed_initialize(rxpa_attribute_value value, const char *type, int attrs) {
    /* Return slots may be reused. A failed construction must not retain an
     * earlier owner's handle or an earlier successful result's bytes. */
    if (SETNATIVEPAYLOAD(value, NULL, 0, NULL, 0)) return -1;
    SETNUMATTRS(value, attrs);
    typed_diagnostic(value, 0, "", "");
    return !current_vm || SETOBJECTTYPE(current_vm->host, value, type);
}
/* Capture the bridge diagnostic immediately, including successful operations.
 * DIAGNOSTIC itself neither collects resources nor invalidates output views. */
static int typed_call(rxpa_attribute_value owner, int op, const char *signature,
                      rxpa_attribute_value *args, rxllama_result *output) {
    rxllama_argument inputs[6] = {{0}};
    rxllama_result diagnostic = {0};
    size_t i;
    int status;
    memset(output, 0, sizeof(*output));
    if (!current_vm) {
        typed_diagnostic(owner, -7, "adapter", "native session is unavailable");
        return -7;
    }
    for (i = 0; signature[i]; ++i) {
        switch (signature[i]) {
        case 'h': inputs[i].handle = token(args[i]); break;
        case 'i': inputs[i].integer = GETINT(args[i]); break;
        case 'f': inputs[i].number = GETFLOAT(args[i]); break;
        case 's':
            if (current_vm->string_view(args[i], &inputs[i].text, &inputs[i].text_length)) {
                typed_diagnostic(owner, -1, "adapter", "complete string view is unavailable");
                return -1;
            }
            break;
        }
    }
    status = rxllama_call(current_vm->vm, op, inputs, output);
    rxllama_call(current_vm->vm, RXLLAMA_DIAGNOSTIC, inputs, &diagnostic);
    typed_diagnostic(owner, status, diagnostic.operation, diagnostic.message);
    return status;
}
static void typed_handle(rxpa_attribute_value value, rxllama_token handle) {
    if (SETNATIVEPAYLOAD(value, &handle, sizeof(handle), &payload_ops, 0)) {
        rxllama_release(handle);
        typed_diagnostic(value, -5, "publish", "native owner payload allocation failed");
    }
}
#define TYPED_INIT(value, type, attrs) \
    if (typed_initialize(value, "llama." type "_impl", attrs)) { \
        RETURNSIGNAL(SIGNAL_FAILURE, "native llama object publication is unavailable") }

/** Construct a configuration with the bridge's validated defaults.
 * @return initialized configuration; inspect status before use, close after use.
 */
PROCEDURE(typed_configuration) {
    rxllama_result out;
    TYPED_INIT(RETURN, "configuration", 3)
    if (!typed_call(RETURN, RXLLAMA_CONFIG_CREATE, "", _arg, &out)) typed_handle(RETURN, out.handle);
    RESETSIGNAL
}
/** Open one VM-local runtime. Configuration is copied; it may later be closed.
 * @param config validated configuration
 * @return initialized owner, including on failure; close after its children.
 */
PROCEDURE(typed_runtime) {
    rxllama_result out;
    TYPED_INIT(RETURN, "runtime", 3)
    if (!typed_call(RETURN, RXLLAMA_RUNTIME_OPEN, "h", _arg, &out)) typed_handle(RETURN, out.handle);
    RESETSIGNAL
}
PROCEDURE(typed_match) { SETINT(RETURN, 1); RESETSIGNAL }

/** Read this value's last operation status without altering its diagnostic.
 * @return zero on success, otherwise the existing rxllama negative status.
 */
METHODPROCEDURE(typed_status) { SETINT(RETURN, GETINT(GETATTR(ARG0, 0))); RESETSIGNAL }
/** Snapshot this value's last operation diagnostic.
 * @return independent diagnostic; later calls and owner closure cannot alter it.
 */
METHODPROCEDURE(typed_snapshot) {
    TYPED_INIT(RETURN, "diagnostic", 3)
    typed_diagnostic(RETURN, GETINT(GETATTR(ARG0, 0)),
                     GETSTRING(GETATTR(ARG0, 1)), GETSTRING(GETATTR(ARG0, 2)));
    RESETSIGNAL
}
/** @return captured status code; inspecting a snapshot never calls the engine. */
METHODPROCEDURE(typed_code) { SETINT(RETURN, GETINT(GETATTR(ARG0, 0))); RESETSIGNAL }
/** @return captured native operation name (or adapter/publication failure). */
METHODPROCEDURE(typed_operation) { SETSTRING(RETURN, GETSTRING(GETATTR(ARG0, 1))); RESETSIGNAL }
/** @return captured diagnostic message, retained independently of later calls. */
METHODPROCEDURE(typed_message) { SETSTRING(RETURN, GETSTRING(GETATTR(ARG0, 2))); RESETSIGNAL }
/** Close an owner after closing children. Copies refer to the same resource.
 * @return status; failed construction and repeated close safely return zero.
 */
METHODPROCEDURE(typed_close) {
    rxllama_result out;
    rxllama_token t = token(ARG0);
    int status = 0;
    if (!t.owner && !t.id) typed_diagnostic(ARG0, 0, "close", "");
    else status = typed_call(ARG0, RXLLAMA_CLOSE, "h", _arg, &out);
    SETINT(RETURN, status); RESETSIGNAL
}
#define TYPED_STATUS_CALL(name, op, sig) METHODPROCEDURE(name) { \
    rxllama_result out; SETINT(RETURN, typed_call(ARG0, op, sig, _arg, &out)); RESETSIGNAL }
#define TYPED_INT_CALL(name, op, sig) METHODPROCEDURE(name) { \
    rxllama_result out; int status = typed_call(ARG0, op, sig, _arg, &out); \
    SETINT(RETURN, status ? 0 : out.integer); RESETSIGNAL }
#define TYPED_TEXT_CALL(name, op, sig) METHODPROCEDURE(name) { \
    rxllama_result out; int status = typed_call(ARG0, op, sig, _arg, &out); \
    SETSTRING(RETURN, !status && out.text ? out.text : ""); RESETSIGNAL }
/** Set an existing integer option; configuration affects subsequent construction.
 * @param key option name @param value bounded integer @return status
 */
TYPED_STATUS_CALL(typed_set_int, RXLLAMA_CONFIG_INT, "hsi")
/** Set an existing float option. Only qualified greedy temperature is supported.
 * @param key option name @param value float @return status
 */
TYPED_STATUS_CALL(typed_set_float, RXLLAMA_CONFIG_FLOAT, "hsf")
/** Set an existing text option; all bytes are validated, including embedded NUL.
 * @param key option name @param value option text @return status
 */
TYPED_STATUS_CALL(typed_set_text, RXLLAMA_CONFIG_TEXT, "hss")
/** @return discovered device count, or zero on failure; inspect status. */
TYPED_INT_CALL(typed_device_count, RXLLAMA_DEVICE_COUNT, "h")
/** Inspect a discovered device.
 * @param index one-based device index @param key property name
 * @return property text, or empty on failure; inspect status.
 */
TYPED_TEXT_CALL(typed_device_info, RXLLAMA_DEVICE_INFO, "his")
/** Inspect an owner using existing bridge properties, without changing placement.
 * @param key integer property name @return value, or zero on failure; inspect status.
 */
TYPED_INT_CALL(typed_info_int, RXLLAMA_INFO_INT, "hs")
/** Inspect identity, placement or lifecycle details.
 * @param key text property name @return text, or empty on failure; inspect status.
 */
TYPED_TEXT_CALL(typed_info_text, RXLLAMA_INFO_TEXT, "hs")
/** Start asynchronous loading; compatible owners share immutable model weights.
 * @param path local GGUF @param sha256 pinned digest @param profile supported profile
 * @param config bounded configuration @return model owner; poll state then prepare.
 */
METHODPROCEDURE(typed_model) {
    rxllama_result out;
    TYPED_INIT(RETURN, "model", 3)
    if (!typed_call(RETURN, RXLLAMA_MODEL_OPEN, "hsssh", _arg, &out)) typed_handle(RETURN, out.handle);
    typed_diagnostic(ARG0, GETINT(GETATTR(RETURN, 0)), GETSTRING(GETATTR(RETURN, 1)), GETSTRING(GETATTR(RETURN, 2)));
    RESETSIGNAL
}
/** @return loading/ready/failed/cancelled/closed; empty on call failure, inspect status. */
TYPED_TEXT_CALL(typed_model_state, RXLLAMA_MODEL_STATE, "h")
/** Cancel model ownership; live sessions must close first. @return status */
TYPED_STATUS_CALL(typed_model_cancel, RXLLAMA_MODEL_CANCEL, "h")
/** Create private mutable embedding state on a ready model; does not reload it.
 * @param config bounded configuration @return session; explicitly prepare before requests.
 */
METHODPROCEDURE(typed_session) {
    rxllama_argument args[3] = {{0}};
    rxllama_result out = {0}, diagnostic = {0};
    int status;
    TYPED_INIT(RETURN, "embedding_session", 3)
    args[0].handle = token(ARG0); args[1].text = "embedding"; args[1].text_length = 9;
    args[2].handle = token(ARG1);
    status = rxllama_call(current_vm->vm, RXLLAMA_SESSION_OPEN, args, &out);
    rxllama_call(current_vm->vm, RXLLAMA_DIAGNOSTIC, args, &diagnostic);
    typed_diagnostic(RETURN, status, diagnostic.operation, diagnostic.message);
    if (!status) typed_handle(RETURN, out.handle);
    typed_diagnostic(ARG0, GETINT(GETATTR(RETURN, 0)), GETSTRING(GETATTR(RETURN, 1)), GETSTRING(GETATTR(RETURN, 2)));
    RESETSIGNAL
}
/** Explicitly warm private state once; subsequent requests reuse it.
 * @param work_tokens positive work budget @return preparing/ready, empty on failure.
 */
TYPED_TEXT_CALL(typed_prepare, RXLLAMA_PREPARE, "hi")
/** Create one bounded building request; at most one active request per session.
 * @param config compatible request limits @return request owner, including on failure.
 */
METHODPROCEDURE(typed_request) {
    rxllama_result out;
    TYPED_INIT(RETURN, "embedding_request", 3)
    if (!typed_call(RETURN, RXLLAMA_REQUEST_OPEN, "hh", _arg, &out)) typed_handle(RETURN, out.handle);
    typed_diagnostic(ARG0, GETINT(GETATTR(RETURN, 0)), GETSTRING(GETATTR(RETURN, 1)), GETSTRING(GETATTR(RETURN, 2)));
    RESETSIGNAL
}
/** Add complete text without submitting; the model's role preparation is applied.
 * @param text complete input @param role query/document
 * @return one-based row, zero on failure; status contains the failure code.
 */
TYPED_INT_CALL(typed_add, RXLLAMA_ADD_EMBEDDING, "hss")
/** Fill one request in order without submitting. A row failure cancels it and
 * retains that row's diagnostic, never a silently accepted shorter batch.
 * @param texts input string array @param role query/document @return status
 */
METHODPROCEDURE(typed_add_all) {
    rxllama_result out;
    rxpa_attribute_value args[3];
    rxinteger i, count = GETNUMATTRS(ARG1);
    int status = 0;
    args[0] = ARG0; args[2] = ARG2;
    typed_diagnostic(ARG0, 0, "add_all", "");
    for (i = 0; i < count; ++i) {
        args[1] = GETATTR(ARG1, i);
        status = typed_call(ARG0, RXLLAMA_ADD_EMBEDDING, "hss", args, &out);
        if (status) {
            rxllama_argument input = {0};
            input.handle = token(ARG0);
            /* Do not overwrite the row diagnostic already owned by ARG0. */
            rxllama_call(current_vm->vm, RXLLAMA_CANCEL, &input, &out);
            break;
        }
    }
    SETINT(RETURN, status); RESETSIGNAL
}
/** Submit validated rows once; input mutation then stops. @return status */
TYPED_STATUS_CALL(typed_submit, RXLLAMA_SUBMIT, "h")
/** Process a bounded batch. Noncausal attention is one indivisible compute unit.
 * @param work_tokens positive budget @return state, empty on failure; inspect status.
 */
TYPED_TEXT_CALL(typed_process, RXLLAMA_PROCESS, "hi")
/** @return building/running/complete/cancelled/failed/closed, empty on call failure. */
METHODPROCEDURE(typed_request_state) {
    rxllama_argument args[2] = {{0}};
    rxllama_result out = {0}, diagnostic = {0};
    int status;
    args[0].handle = token(ARG0); args[1].text = "state"; args[1].text_length = 5;
    status = rxllama_call(current_vm->vm, RXLLAMA_INFO_TEXT, args, &out);
    rxllama_call(current_vm->vm, RXLLAMA_DIAGNOSTIC, args, &diagnostic);
    typed_diagnostic(ARG0, status, diagnostic.operation, diagnostic.message);
    SETSTRING(RETURN, !status && out.text ? out.text : ""); RESETSIGNAL
}
/** Cancel a building/running request and release active admission. @return status */
TYPED_STATUS_CALL(typed_cancel, RXLLAMA_CANCEL, "h")
/** Capture only a complete result, copying the existing host-native packed doubles.
 * @return owned result surviving request close; failed result is empty with status.
 */
METHODPROCEDURE(typed_result) {
    rxllama_result out;
    TYPED_INIT(RETURN, "embedding_result", 5)
    SETINT(GETATTR(RETURN, 3), 0); SETINT(GETATTR(RETURN, 4), 0);
    if (!typed_call(RETURN, RXLLAMA_EMBEDDINGS, "h", _arg, &out)) {
        if (SETNATIVEPAYLOAD(RETURN, out.values, (size_t)out.value_count * sizeof(double), NULL, 0))
            typed_diagnostic(RETURN, -5, "result", "packed result allocation failed");
        else { SETINT(GETATTR(RETURN, 3), out.rows); SETINT(GETATTR(RETURN, 4), out.dimensions); }
    }
    typed_diagnostic(ARG0, GETINT(GETATTR(RETURN, 0)), GETSTRING(GETATTR(RETURN, 1)), GETSTRING(GETATTR(RETURN, 2)));
    RESETSIGNAL
}
/** @return input row count, zero on failed result; no native owner remains. */
METHODPROCEDURE(typed_rows) { SETINT(RETURN, GETINT(GETATTR(ARG0, 3))); RESETSIGNAL }
/** @return vector dimensions per row, zero on failed result. */
METHODPROCEDURE(typed_dimensions) { SETINT(RETURN, GETINT(GETATTR(ARG0, 4))); RESETSIGNAL }
/** Return independent owned packed data in input-row-major order.
 * @return packedfloat compatible with rxvector; failure yields empty data and status.
 */
METHODPROCEDURE(typed_values) {
    size_t size = 0;
    const void *data = GETNATIVEPAYLOAD(ARG0, &size, NULL, NULL);
    if (SETOBJECTTYPE(current_vm->host, RETURN, "rxfnsg.packedfloat")) {
        RETURNSIGNAL(SIGNAL_FAILURE, "packedfloat runtime type is unavailable; import rxfnsg")
    }
    if (SETNATIVEPAYLOAD(RETURN, data, size, NULL, 0))
        typed_diagnostic(ARG0, -5, "values", "packed value allocation failed");
    RESETSIGNAL
}
#endif

/* Metadata is also usable by RXPA's declaration-only compilation mode.
 * Public contracts are interfaces; implementation class names are internal.
 */
#define LLAMA_TYPE(name) \
    ADDINTERFACE("llama." name); ADDCLASS("llama." name "_impl"); \
    ADDIMPLEMENTS("llama." name "_impl", "llama." name);
#define LLAMA_METHOD(name, fn, member, result, args) \
    ADDMETHOD("llama." name, member, result, args); \
    ADDMETHODPROC(fn, "llama." name "_impl", member, result, args);
#define LLAMA_STATUS(name) \
    LLAMA_METHOD(name, typed_status, "status", ".int", "") \
    LLAMA_METHOD(name, typed_snapshot, "diagnostic", ".llama..diagnostic", "")
#define LLAMA_OWNER(name) \
    LLAMA_TYPE(name) LLAMA_STATUS(name) LLAMA_METHOD(name, typed_close, "close", ".int", "")
#define LLAMA_INFO(name) \
    LLAMA_METHOD(name, typed_info_int, "info_int", ".int", "key=.string") \
    LLAMA_METHOD(name, typed_info_text, "info_text", ".string", "key=.string")
#define LLAMA_FACTORY(name, fn, args) \
    ADDFACTORY("llama." name, "*", ".llama.." name, args); \
    ADDFACTORYPROC(fn, "llama." name "_impl", ".llama.." name "_impl", args); \
    ADDMATCHPROC(typed_match, "llama." name "_impl", args);

/** @docType native-module
 * llama.rexx: VM-local typed owners for persistent CPU/GPU embeddings.
 * Import llama and rxfnsg. Construct configuration/runtime, load model, poll,
 * prepare a session, repeat requests, then close children before parents.
 * Check each operation's status. Failed constructors are initialized values.
 * Worker inputs/outputs are data; construct owners inside each worker.
 * Configuration owns option values; runtime owns model reservations; model
 * shares immutable weights; embedding_session owns a private context;
 * embedding_request owns admission and rows; embedding_result owns copied
 * packed bytes; diagnostic owns a snapshot. The last two need no close.
 */
#define LLAMA_TYPED_DECLARATIONS \
    LLAMA_TYPE("diagnostic") \
    LLAMA_METHOD("diagnostic", typed_code, "code", ".int", "") \
    LLAMA_METHOD("diagnostic", typed_operation, "operation", ".string", "") \
    LLAMA_METHOD("diagnostic", typed_message, "message", ".string", "") \
    LLAMA_OWNER("configuration") LLAMA_FACTORY("configuration", typed_configuration, "") \
    LLAMA_METHOD("configuration", typed_set_int, "set_int", ".int", "key=.string,value=.int") \
    LLAMA_METHOD("configuration", typed_set_float, "set_float", ".int", "key=.string,value=.float") \
    LLAMA_METHOD("configuration", typed_set_text, "set_text", ".int", "key=.string,value=.string") \
    LLAMA_OWNER("runtime") LLAMA_FACTORY("runtime", typed_runtime, "config=.llama..configuration") LLAMA_INFO("runtime") \
    LLAMA_METHOD("runtime", typed_device_count, "device_count", ".int", "") \
    LLAMA_METHOD("runtime", typed_device_info, "device_info", ".string", "index=.int,key=.string") \
    LLAMA_METHOD("runtime", typed_model, "model", ".llama..model", "path=.string,sha256=.string,profile=.string,config=.llama..configuration") \
    LLAMA_OWNER("model") LLAMA_INFO("model") \
    LLAMA_METHOD("model", typed_model_state, "state", ".string", "") \
    LLAMA_METHOD("model", typed_model_cancel, "cancel", ".int", "") \
    LLAMA_METHOD("model", typed_session, "embedding_session", ".llama..embedding_session", "config=.llama..configuration") \
    LLAMA_OWNER("embedding_session") LLAMA_INFO("embedding_session") \
    LLAMA_METHOD("embedding_session", typed_prepare, "prepare", ".string", "work_tokens=.int") \
    LLAMA_METHOD("embedding_session", typed_request, "request", ".llama..embedding_request", "config=.llama..configuration") \
    LLAMA_OWNER("embedding_request") LLAMA_INFO("embedding_request") \
    LLAMA_METHOD("embedding_request", typed_add, "add", ".int", "text=.string,role=.string") \
    LLAMA_METHOD("embedding_request", typed_add_all, "add_all", ".int", "texts=.string[*],role=.string") \
    LLAMA_METHOD("embedding_request", typed_submit, "submit", ".int", "") \
    LLAMA_METHOD("embedding_request", typed_process, "process", ".string", "work_tokens=.int") \
    LLAMA_METHOD("embedding_request", typed_request_state, "state", ".string", "") \
    LLAMA_METHOD("embedding_request", typed_cancel, "cancel", ".int", "") \
    LLAMA_METHOD("embedding_request", typed_result, "result", ".llama..embedding_result", "") \
    LLAMA_TYPE("embedding_result") LLAMA_STATUS("embedding_result") \
    LLAMA_METHOD("embedding_result", typed_values, "values", ".rxfnsg..packedfloat", "") \
    LLAMA_METHOD("embedding_result", typed_rows, "rows", ".int", "") \
    LLAMA_METHOD("embedding_result", typed_dimensions, "dimensions", ".int", "")
