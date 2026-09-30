/* MIT. Optional rxfnsg.llmnative driver using the existing bridge ownership graph.
 * Core declares the contracts; loading this plugin supplies their implementation.
 * Client attrs: diagnostic(0..2), config/runtime/model/session(3..6), capability(7),
 * closed(8), callback scratch(9..10), issued requests(11). No parallel registry.
 */
#ifndef DECL_ONLY
#include <stdio.h>
static rxllama_argument common_handle(rxpa_attribute_value v) {
    rxllama_argument a = {0}; a.handle = token(v); return a;
}
static rxllama_argument common_text(const char *s) {
    rxllama_argument a = {0}; a.text = s; a.text_length = strlen(s); return a;
}
static rxllama_argument common_int(rxinteger n) {
    rxllama_argument a = {0}; a.integer = n; return a;
}
static int common_call(rxpa_attribute_value owner, int op, rxllama_argument *args, rxllama_result *out) {
    rxllama_result diagnostic = {0};
    int status;
    memset(out, 0, sizeof(*out));
    status = adapter_call(op, args, out);
    adapter_call(RXLLAMA_DIAGNOSTIC, args, &diagnostic);
    typed_diagnostic(owner, status, diagnostic.operation, diagnostic.message);
    return status;
}
static int common_get(rxpa_attribute_value result, rxpa_attribute_value config,
                      const char *key, const char *descriptor, rxpa_attribute_value signal) {
    rxpa_attribute_value args[1];
    SETSTRING(GETATTR(result, 9), key); args[0] = GETATTR(result, 9);
    return CALLMETHODX(config, descriptor, 1, args, GETATTR(result, 10), signal);
}
static int common_get_text(rxpa_attribute_value result, rxpa_attribute_value config,
                           const char *key, rxpa_attribute_value signal) {
    return common_get(result, config, key, "rxsig1|text|.string|key=.string", signal);
}
static int common_copy_text(rxpa_attribute_value target, rxpa_attribute_value source) {
    const char *bytes; size_t length;
    if (current_vm->string_view(source, &bytes, &length)) return -1;
    return SETSTRINGLENGTH(current_vm->host, target, bytes, length);
}
static int common_set_int(rxpa_attribute_value owner, const char *key, rxinteger n) {
    rxllama_argument args[3]; rxllama_result out;
    args[0] = common_handle(GETATTR(owner, 3)); args[1] = common_text(key); args[2] = common_int(n);
    return common_call(owner, RXLLAMA_CONFIG_INT, args, &out);
}
static int common_closed(rxpa_attribute_value owner) {
    if (!GETINT(GETATTR(owner, 8))) return 0;
    typed_diagnostic(owner, -3, "client", "client is closed"); return 1;
}
/** Construct native owners from the core configuration, without loading per call.
 * @param config common setup @param capability generation or embedding
 * @return initialized native service; package/configuration failures signal.
 */
PROCEDURE(common_driver) {
    static const char *texts[] = {"hardware_mode", "backend", "devices", "sampler", "pooling",
        "normalization", "query_prefix", "document_prefix", "chat_template", "system_prompt", NULL};
    static const char *integers[] = {"memory_bytes", "vram_bytes", "max_sessions", "threads",
        "batch_threads", "request_bytes", "batch_tokens", "seed", NULL};
    static const char *limits[] = {"context_tokens", "request_rows", "request_tokens", "output_tokens"};
    rxinteger bounds[] = {512, 8, 4096, 32};
    rxllama_argument args[6] = {{0}}; rxllama_result out;
    rxpa_attribute_value config = ARG0;
    int i;
    TYPED_INIT(RETURN, "common_driver", 12)
    SETSTRING(GETATTR(RETURN, 7), GETSTRING(ARG1));
    SETINT(GETATTR(RETURN, 8), 0); SETNUMATTRS(GETATTR(RETURN, 11), 0);
    if (strcmp(GETSTRING(ARG1), "generation") && strcmp(GETSTRING(ARG1), "embedding")) {
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "unsupported_operation: native capability")
    }
    if (common_call(RETURN, RXLLAMA_CONFIG_CREATE, args, &out)) return;
    typed_handle(GETATTR(RETURN, 3), out.handle);
    for (i = 0; texts[i]; ++i) {
        if (common_get(RETURN, config, texts[i], "rxsig1|has_text|.boolean|key=.string", SIGNAL)) return;
        if (!GETINT(GETATTR(RETURN, 10))) continue;
        if (common_get_text(RETURN, config, texts[i], SIGNAL)) return;
        args[0] = common_handle(GETATTR(RETURN, 3)); args[1] = common_text(texts[i]);
        if (current_vm->string_view(GETATTR(RETURN, 10), &args[2].text, &args[2].text_length)) {
            RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "configuration_error: complete text is unavailable")
        }
        if (common_call(RETURN, RXLLAMA_CONFIG_TEXT, args, &out)) goto config_error;
    }
    for (i = 0; integers[i]; ++i) {
        if (common_get(RETURN, config, integers[i], "rxsig1|integer|.int|key=.string", SIGNAL)) return;
        if (GETINT(GETATTR(RETURN, 10)) >= 0 && common_set_int(RETURN, integers[i], GETINT(GETATTR(RETURN, 10)))) goto config_error;
    }
    for (i = 0; i < 4; ++i) {
        if (common_get(RETURN, config, limits[i], "rxsig1|integer|.int|key=.string", SIGNAL)) return;
        if (GETINT(GETATTR(RETURN, 10)) >= 0) bounds[i] = GETINT(GETATTR(RETURN, 10));
    }
    /* Validate through the public bridge while keeping every intermediate valid. */
    if (common_set_int(RETURN, "request_tokens", 1) || common_set_int(RETURN, "output_tokens", 1)) goto config_error;
    for (i = 0; i < 4; ++i) if (common_set_int(RETURN, limits[i], bounds[i])) goto config_error;
    args[0] = common_handle(GETATTR(RETURN, 3));
    if (common_call(RETURN, RXLLAMA_RUNTIME_OPEN, args, &out)) {
        char message[1024];
        snprintf(message, sizeof(message), "provider_incompatible: llama: %s", GETSTRING(GETATTR(RETURN, 2)));
        RETURNSIGNAL(SIGNAL_NOTREADY, message)
    }
    typed_handle(GETATTR(RETURN, 4), out.handle);
    /* Callback values must be retained across the next callback. */
    if (common_get_text(RETURN, config, "model", SIGNAL)) return;
    /* Use temporary attributes on the model slot for owned path/hash/profile. */
    SETNUMATTRS(GETATTR(RETURN, 5), 3);
    if (common_copy_text(GETATTR(GETATTR(RETURN, 5), 0), GETATTR(RETURN, 10))) goto text_error;
    if (common_get_text(RETURN, config, "sha256", SIGNAL)) return;
    if (common_copy_text(GETATTR(GETATTR(RETURN, 5), 1), GETATTR(RETURN, 10))) goto text_error;
    if (common_get_text(RETURN, config, "profile", SIGNAL)) return;
    if (current_vm->string_view(GETATTR(RETURN, 10), &args[0].text, &args[0].text_length)) goto text_error;
    if (!args[0].text_length) SETSTRING(GETATTR(GETATTR(RETURN, 5), 2), GETSTRING(GETATTR(RETURN, 7)));
    else if (common_copy_text(GETATTR(GETATTR(RETURN, 5), 2), GETATTR(RETURN, 10))) goto text_error;
    args[0] = common_handle(GETATTR(RETURN, 4));
    for (i = 0; i < 3; ++i)
        if (current_vm->string_view(GETATTR(GETATTR(RETURN, 5), i), &args[i+1].text, &args[i+1].text_length)) goto text_error;
    args[4] = common_handle(GETATTR(RETURN, 3));
    if (!common_call(RETURN, RXLLAMA_MODEL_OPEN, args, &out)) typed_handle(GETATTR(RETURN, 5), out.handle);
    RESETSIGNAL
    return;
text_error:
    RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "configuration_error: complete model selector text is unavailable")
config_error:
    {
        char message[1024];
        snprintf(message, sizeof(message), "configuration_error: llama: %s", GETSTRING(GETATTR(RETURN, 2)));
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, message)
    }
}
static const char *common_prepare(rxpa_attribute_value owner, rxinteger work) {
    rxllama_argument args[3] = {{0}}; rxllama_result out;
    if (common_closed(owner) || GETINT(GETATTR(owner, 0))) return "failed";
    args[0] = common_handle(GETATTR(owner, 5));
    if (common_call(owner, RXLLAMA_MODEL_STATE, args, &out)) return "failed";
    if (!strcmp(out.text, "loading")) return "loading";
    if (strcmp(out.text, "ready")) {
        args[1] = common_text("error");
        if (!common_call(owner, RXLLAMA_INFO_TEXT, args, &out)) typed_diagnostic(owner, -6, "modelopen", out.text);
        return "failed";
    }
    if (!token(GETATTR(owner, 6)).id) {
        args[1] = common_text(GETSTRING(GETATTR(owner, 7))); args[2] = common_handle(GETATTR(owner, 3));
        if (common_call(owner, RXLLAMA_SESSION_OPEN, args, &out)) return "failed";
        typed_handle(GETATTR(owner, 6), out.handle);
    }
    args[0] = common_handle(GETATTR(owner, 6)); args[1] = common_int(work);
    if (common_call(owner, RXLLAMA_PREPARE, args, &out)) return "failed";
    return out.text;
}
/** @param work_tokens native bounded preparation budget @return current state. */
METHODPROCEDURE(common_driver_prepare) { SETSTRING(RETURN, common_prepare(ARG0, GETINT(ARG1))); RESETSIGNAL }
/** @return last owned diagnostic text; no engine call. */
METHODPROCEDURE(common_error) { SETSTRING(RETURN, GETSTRING(GETATTR(ARG0, 2))); RESETSIGNAL }
static int common_request(rxpa_attribute_value owner, rxpa_attribute_value result) {
    rxllama_argument args[2]; rxllama_result out;
    rxpa_attribute_value requests = GETATTR(owner, 11);
    rxinteger i;
    if (common_closed(owner)) return -3;
    args[0] = common_handle(GETATTR(owner, 6)); args[1] = common_handle(GETATTR(owner, 3));
    if (common_call(owner, RXLLAMA_REQUEST_OPEN, args, &out)) return (int)GETINT(GETATTR(owner, 0));
    typed_handle(result, out.handle);
    /* Retain only live issued requests, so convenience calls cannot grow this list. */
    for (i = GETNUMATTRS(requests); i > 0; --i) {
        rxllama_argument probe[2]; rxllama_result state = {0};
        probe[0] = common_handle(GETATTR(requests, i - 1)); probe[1] = common_text("state");
        if (!adapter_call(RXLLAMA_INFO_TEXT, probe, &state) && !strcmp(state.text, "closed")) REMOVEATTR(requests, i - 1);
    }
    i = GETNUMATTRS(requests); SETNUMATTRS(requests, i + 1);
    if (rxllama_retain(out.handle)) typed_handle(GETATTR(requests, i), out.handle);
    return 0;
}
/** @return one native generation request, retaining existing session ownership. */
METHODPROCEDURE(common_driver_request) {
    TYPED_INIT(RETURN, "common_request", 3)
    if (strcmp(GETSTRING(GETATTR(ARG0, 7)), "generation")) typed_diagnostic(RETURN, -4, "request", "unsupported_operation: embedding driver");
    else if (common_request(ARG0, RETURN)) typed_diagnostic(RETURN, GETINT(GETATTR(ARG0, 0)), GETSTRING(GETATTR(ARG0, 1)), GETSTRING(GETATTR(ARG0, 2)));
    RESETSIGNAL
}
/** Read state without erasing a retained inference failure. */
METHODPROCEDURE(common_request_state) {
    rxllama_argument args[2]; rxllama_result out = {0};
    args[0] = common_handle(ARG0); args[1] = common_text("state");
    if (adapter_call(RXLLAMA_INFO_TEXT, args, &out)) SETSTRING(RETURN, "failed");
    else SETSTRING(RETURN, out.text);
    RESETSIGNAL
}
/** @param row one-based row @return owned core chunk, including any failure. */
METHODPROCEDURE(common_read) {
    rxllama_argument args[2]; rxllama_result out;
    int status;
    if (SETNATIVEPAYLOAD(RETURN, NULL, 0, NULL, 0) || SETOBJECTTYPE(current_vm->host, RETURN, "rxfnsg.llmchunk")) {
        RETURNSIGNAL(SIGNAL_FAILURE, "common LLM chunk type is unavailable")
    }
    SETNUMATTRS(RETURN, 6);
    SETSTRING(GETATTR(RETURN, 0), ""); SETINT(GETATTR(RETURN, 1), 0);
    SETSTRING(GETATTR(RETURN, 2), ""); SETINT(GETATTR(RETURN, 3), GETINT(ARG1));
    status = (int)GETINT(GETATTR(ARG0, 0));
    if (status) {
        SETINT(GETATTR(RETURN, 4), status); SETSTRING(GETATTR(RETURN, 5), GETSTRING(GETATTR(ARG0, 2)));
        SETSTRING(GETATTR(RETURN, 2), "error"); RESETSIGNAL; return;
    }
    args[0] = common_handle(ARG0); args[1] = common_int(GETINT(ARG1));
    status = common_call(ARG0, RXLLAMA_READ_TEXT, args, &out);
    if (!status) {
        if (SETSTRINGLENGTH(current_vm->host, GETATTR(RETURN, 0), out.text, out.text_length)) {
            status = -5; typed_diagnostic(ARG0, status, "read", "complete text publication failed");
        } else { SETINT(GETATTR(RETURN, 1), out.integer); SETSTRING(GETATTR(RETURN, 2), out.finish); }
    }
    SETINT(GETATTR(RETURN, 4), status); SETSTRING(GETATTR(RETURN, 5), GETSTRING(GETATTR(ARG0, 2)));
    RESETSIGNAL
}
/** @return owned embedding-space identity. */
METHODPROCEDURE(common_specification) { SETSTRING(RETURN, GETSTRING(GETATTR(ARG0, 5))); RESETSIGNAL }
/** One admitted embedding batch; preparation stays explicit and persistent.
 * @param texts input rows @param role query/document @return owned packed result.
 */
METHODPROCEDURE(common_embed) {
    rxllama_argument args[3] = {{0}}; rxllama_result out;
    rxllama_token request = {0}; rxinteger i;
    TYPED_INIT(RETURN, "common_embedding_result", 6)
    SETINT(GETATTR(RETURN, 3), 0); SETINT(GETATTR(RETURN, 4), 0); SETSTRING(GETATTR(RETURN, 5), "");
    if (common_closed(ARG0)) { typed_diagnostic(RETURN, -3, "embed", "client is closed"); goto finish; }
    if (strcmp(GETSTRING(GETATTR(ARG0, 7)), "embedding")) { typed_diagnostic(RETURN, -4, "embed", "unsupported_operation: generation driver"); goto finish; }
    args[0] = common_handle(GETATTR(ARG0, 6)); args[1] = common_handle(GETATTR(ARG0, 3));
    if (common_call(RETURN, RXLLAMA_REQUEST_OPEN, args, &out)) goto finish;
    request = out.handle;
    args[0].handle = request;
    if (current_vm->string_view(ARG2, &args[2].text, &args[2].text_length)) goto text_error;
    for (i = 0; i < GETNUMATTRS(ARG1); ++i) {
        if (current_vm->string_view(GETATTR(ARG1, i), &args[1].text, &args[1].text_length)) goto text_error;
        if (common_call(RETURN, RXLLAMA_ADD_EMBEDDING, args, &out)) goto finish;
    }
    if (common_call(RETURN, RXLLAMA_SUBMIT, args, &out)) goto finish;
    args[1] = common_int(128);
    if (common_call(RETURN, RXLLAMA_PROCESS, args, &out)) goto finish;
    if (common_call(RETURN, RXLLAMA_EMBEDDINGS, args, &out)) goto finish;
    if (SETNATIVEPAYLOAD(RETURN, out.values, (size_t)out.value_count * sizeof(double), NULL, 0)) {
        typed_diagnostic(RETURN, -5, "embed", "packed output allocation failed"); goto finish;
    }
    SETINT(GETATTR(RETURN, 3), out.rows); SETINT(GETATTR(RETURN, 4), out.dimensions);
    args[0] = common_handle(GETATTR(ARG0, 6)); args[1] = common_text("embedding_spec");
    if (!common_call(RETURN, RXLLAMA_INFO_TEXT, args, &out)) SETSTRING(GETATTR(RETURN, 5), out.text);
    goto finish;
text_error:
    typed_diagnostic(RETURN, -1, "embed", "complete input text is unavailable");
finish:
    if (request.id) {
        int cleanup;
        args[0].handle = request;
        /* Preserve result/error snapshots across cleanup. */
        cleanup = adapter_call(RXLLAMA_CLOSE, args, &out);
        if (cleanup && !GETINT(GETATTR(RETURN, 0))) {
            adapter_call(RXLLAMA_DIAGNOSTIC, args, &out);
            typed_diagnostic(RETURN, cleanup, out.operation, out.message);
        }
        rxllama_release(request);
    }
    typed_diagnostic(ARG0, GETINT(GETATTR(RETURN, 0)), GETSTRING(GETATTR(RETURN, 1)), GETSTRING(GETATTR(RETURN, 2)));
    RESETSIGNAL
}
/** Close issued requests and children in reverse ownership order; idempotent. */
METHODPROCEDURE(common_driver_close) {
    rxpa_attribute_value requests = GETATTR(ARG0, 11);
    rxllama_argument args[1]; rxllama_result out;
    int i;
    if (GETINT(GETATTR(ARG0, 8))) { RESETSIGNAL; return; }
    for (i = (int)GETNUMATTRS(requests) - 1; i >= 0; --i) {
        args[0] = common_handle(GETATTR(requests, i));
        if (common_call(ARG0, RXLLAMA_CLOSE, args, &out)) { RESETSIGNAL; return; }
    }
    SETNUMATTRS(requests, 0);
    for (i = 6; i >= 3; --i) {
        args[0] = common_handle(GETATTR(ARG0, i));
        if (args[0].handle.id && common_call(ARG0, RXLLAMA_CLOSE, args, &out)) { RESETSIGNAL; return; }
    }
    SETINT(GETATTR(ARG0, 8), 1); RESETSIGNAL
}
METHODPROCEDURE(common_driver_info_int) {
    rxpa_attribute_value args[2] = {GETATTR(ARG0, 5), ARG1}; rxllama_result out;
    int status = typed_call(ARG0, RXLLAMA_INFO_INT, "hs", args, &out);
    SETINT(RETURN, status ? 0 : out.integer); RESETSIGNAL
}
METHODPROCEDURE(common_driver_info_text) {
    rxpa_attribute_value args[2] = {GETATTR(ARG0, 5), ARG1}; rxllama_result out;
    int status = typed_call(ARG0, RXLLAMA_INFO_TEXT, "hs", args, &out);
    SETSTRING(RETURN, status ? "" : out.text); RESETSIGNAL
}
#endif

#define COMMON_METHOD(type, fn, name, result, args) \
    ADDMETHODPROC(fn, "llama." type "_impl", name, result, args);
#define LLAMA_COMMON_DECLARATIONS \
    ADDCLASS("llama.common_driver_impl"); \
    ADDIMPLEMENTS("llama.common_driver_impl", "rxfnsg.llmnative"); \
    ADDFACTORYPROC(common_driver, "llama.common_driver_impl", ".llama..common_driver_impl", "config=.rxfnsg..llmconfig,capability=.string"); \
    ADDMATCHPROC(typed_match, "llama.common_driver_impl", "config=.rxfnsg..llmconfig,capability=.string"); \
    COMMON_METHOD("common_driver", common_driver_prepare, "prepare", ".string", "work_tokens=.int") \
    COMMON_METHOD("common_driver", common_driver_request, "request", ".rxfnsg..llmrequest", "") \
    COMMON_METHOD("common_driver", common_embed, "embed", ".rxfnsg..embeddingresult", "texts=.string[*],role=.string") \
    COMMON_METHOD("common_driver", typed_status, "status", ".int", "") \
    COMMON_METHOD("common_driver", common_error, "error", ".string", "") \
    COMMON_METHOD("common_driver", common_driver_info_int, "info_int", ".int", "key=.string") \
    COMMON_METHOD("common_driver", common_driver_info_text, "info_text", ".string", "key=.string") \
    COMMON_METHOD("common_driver", common_driver_close, "close", ".void", "") \
    ADDCLASS("llama.common_request_impl"); \
    ADDIMPLEMENTS("llama.common_request_impl", "rxfnsg.llmrequest"); \
    COMMON_METHOD("common_request", typed_add_prompt, "add", ".int", "system=.string,prompt=.string") \
    COMMON_METHOD("common_request", typed_add_prompts, "add_all", ".int", "prompts=.string[*],system=.string") \
    COMMON_METHOD("common_request", typed_submit, "submit", ".int", "") \
    COMMON_METHOD("common_request", typed_process, "process", ".string", "work_tokens=.int") \
    COMMON_METHOD("common_request", common_request_state, "state", ".string", "") \
    COMMON_METHOD("common_request", common_read, "read", ".rxfnsg..llmchunk", "row=.int") \
    COMMON_METHOD("common_request", typed_cancel, "cancel", ".int", "") \
    COMMON_METHOD("common_request", typed_status, "status", ".int", "") \
    COMMON_METHOD("common_request", common_error, "error", ".string", "") \
    COMMON_METHOD("common_request", typed_close, "close", ".int", "") \
    ADDCLASS("llama.common_embedding_result_impl"); \
    ADDIMPLEMENTS("llama.common_embedding_result_impl", "rxfnsg.embeddingresult"); \
    COMMON_METHOD("common_embedding_result", typed_values, "values", ".rxfnsg..packedfloat", "") \
    COMMON_METHOD("common_embedding_result", typed_rows, "rows", ".int", "") \
    COMMON_METHOD("common_embedding_result", typed_dimensions, "dimensions", ".int", "") \
    COMMON_METHOD("common_embedding_result", common_specification, "specification", ".string", "") \
    COMMON_METHOD("common_embedding_result", typed_status, "status", ".int", "") \
    COMMON_METHOD("common_embedding_result", common_error, "error", ".string", "")
