#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxvml.h"

#ifndef CREXX_NATIVE_KV_LIBRARY_PATH
#define CREXX_NATIVE_KV_LIBRARY_PATH "library"
#endif

#ifndef CREXX_NATIVE_KV_DEMO_MODULE
#define CREXX_NATIVE_KV_DEMO_MODULE "kv_address_demo.rxbin"
#endif

#define KV_MAX_ITEMS 32
#define KV_KEY_SIZE 64
#define KV_VALUE_SIZE 256
#define KV_RESULT_SIZE 2048

typedef struct kv_item {
    int used;
    char key[KV_KEY_SIZE];
    char value[KV_VALUE_SIZE];
} kv_item;

typedef struct kv_state {
    char instance_id[KV_KEY_SIZE];
    char result[KV_RESULT_SIZE];
    char update_name[KV_KEY_SIZE];
    char update_value[KV_VALUE_SIZE];
    rxvml_address_binding update_binding;
    kv_item items[KV_MAX_ITEMS];
} kv_state;

static void copy_text(char* dest, size_t dest_len, const char* src) {
    if (!dest || dest_len == 0) return;
    if (!src) src = "";
    strncpy(dest, src, dest_len - 1);
    dest[dest_len - 1] = 0;
}

static char upper_char(char ch) {
    return (char)toupper((unsigned char)ch);
}

static void normalize_word(char* text) {
    size_t i;
    if (!text) return;
    for (i = 0; text[i]; i++) text[i] = upper_char(text[i]);
}

static const char* skip_spaces(const char* text) {
    while (text && *text && isspace((unsigned char)*text)) text++;
    return text ? text : "";
}

static const char* read_word(const char* text, char* out, size_t out_len) {
    size_t i = 0;

    if (out && out_len > 0) out[0] = 0;
    text = skip_spaces(text);
    while (*text && !isspace((unsigned char)*text)) {
        if (out && out_len > 0 && i + 1 < out_len) out[i++] = *text;
        text++;
    }
    if (out && out_len > 0) out[i] = 0;
    return skip_spaces(text);
}

static void copy_trimmed_text(char* dest, size_t dest_len, const char* src) {
    const char* start = skip_spaces(src);
    const char* end = start + strlen(start);
    size_t len;

    if (!dest || dest_len == 0) return;
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    len = (size_t)(end - start);
    if (len >= dest_len) len = dest_len - 1;
    memcpy(dest, start, len);
    dest[len] = 0;
}

static int kv_anchor_name_start(char ch) {
    return ch == '_' || isalpha((unsigned char)ch);
}

static int kv_anchor_name_part(char ch) {
    return kv_anchor_name_start(ch) || isdigit((unsigned char)ch);
}

static int kv_copy_anchor_name(const char* name, size_t len, char* out, size_t out_len) {
    size_t i;

    if (!out || out_len == 0) return 0;
    out[0] = 0;
    if (!name || len == 0 || len >= out_len) return 0;
    if (!kv_anchor_name_start(name[0])) return 0;
    for (i = 1; i < len; i++) {
        if (!kv_anchor_name_part(name[i])) return 0;
    }

    memcpy(out, name, len);
    out[len] = 0;
    return 1;
}

static int kv_anchor_name(const char* token, char* out, size_t out_len) {
    const char* close;

    if (out && out_len > 0) out[0] = 0;
    if (!token || !*token) return 0;

    if (token[0] == ':') {
        return kv_copy_anchor_name(token + 1, strlen(token + 1), out, out_len);
    }

    if (token[0] == '$' && token[1] == '{') {
        close = strchr(token + 2, '}');
        if (close && close[1] == 0) {
            return kv_copy_anchor_name(token + 2, (size_t)(close - (token + 2)), out, out_len);
        }
    }

    return 0;
}

static int kv_target_name(const char* token, char* out, size_t out_len) {
    if (kv_anchor_name(token, out, out_len)) return 1;
    return kv_copy_anchor_name(token, strlen(token ? token : ""), out, out_len);
}

static int kv_resolve_token(
    const rxvml_address_request* request,
    const char* token,
    char* out,
    size_t out_len) {

    char anchor[KV_KEY_SIZE];
    if (kv_anchor_name(token, anchor, sizeof(anchor))) {
        return rxvml_address_binding_get(request, anchor, out, out_len) == 0 ? 0 : -1;
    }

    copy_text(out, out_len, token);
    return 0;
}

static int kv_resolve_value_text(
    const rxvml_address_request* request,
    const char* text,
    char* out,
    size_t out_len) {

    char trimmed[KV_VALUE_SIZE];
    char anchor[KV_KEY_SIZE];

    copy_trimmed_text(trimmed, sizeof(trimmed), text);
    if (kv_anchor_name(trimmed, anchor, sizeof(anchor))) {
        return rxvml_address_binding_get(request, anchor, out, out_len) == 0 ? 0 : -1;
    }

    copy_text(out, out_len, text);
    return 0;
}

static int kv_prepare_var_update(
    kv_state* state,
    const rxvml_address_request* request,
    rxvml_address_response* response,
    const char* target,
    const char* value) {

    char current[KV_VALUE_SIZE];

    if (rxvml_address_binding_get(request, target, current, sizeof(current)) != 0) return -1;

    copy_text(state->update_name, sizeof(state->update_name), target);
    copy_text(state->update_value, sizeof(state->update_value), value);
    state->update_binding.kind = "var";
    state->update_binding.internal_name = state->update_name;
    state->update_binding.external_alias = state->update_name;
    state->update_binding.value = state->update_value;
    state->update_binding.value_object = NULL;
    state->update_binding.flags = "";

    response->updated_binding_count = 1;
    response->updated_bindings = &state->update_binding;
    response->rc = 0;
    return 0;
}

static int kv_find(const kv_state* state, const char* key) {
    char normalized[KV_KEY_SIZE];
    int i;

    copy_text(normalized, sizeof(normalized), key);
    normalize_word(normalized);
    for (i = 0; i < KV_MAX_ITEMS; i++) {
        if (state->items[i].used && strcmp(state->items[i].key, normalized) == 0) return i;
    }
    return -1;
}

static int kv_count(const kv_state* state) {
    int i;
    int count = 0;
    for (i = 0; i < KV_MAX_ITEMS; i++) {
        if (state->items[i].used) count++;
    }
    return count;
}

static int kv_set(kv_state* state, const char* key, const char* value) {
    char normalized[KV_KEY_SIZE];
    int slot;
    int i;

    copy_text(normalized, sizeof(normalized), key);
    normalize_word(normalized);
    if (normalized[0] == 0) return -1;

    slot = kv_find(state, normalized);
    if (slot < 0) {
        for (i = 0; i < KV_MAX_ITEMS; i++) {
            if (!state->items[i].used) {
                slot = i;
                break;
            }
        }
    }
    if (slot < 0) return -2;

    state->items[slot].used = 1;
    copy_text(state->items[slot].key, sizeof(state->items[slot].key), normalized);
    copy_text(state->items[slot].value, sizeof(state->items[slot].value), value);
    return 0;
}

static int kv_delete(kv_state* state, const char* key) {
    int slot = kv_find(state, key);
    if (slot < 0) return 1;
    state->items[slot].used = 0;
    state->items[slot].key[0] = 0;
    state->items[slot].value[0] = 0;
    return 0;
}

static void kv_clear(kv_state* state) {
    int i;
    for (i = 0; i < KV_MAX_ITEMS; i++) {
        state->items[i].used = 0;
        state->items[i].key[0] = 0;
        state->items[i].value[0] = 0;
    }
}

static void kv_keys(kv_state* state) {
    int i;
    state->result[0] = 0;
    for (i = 0; i < KV_MAX_ITEMS; i++) {
        if (!state->items[i].used) continue;
        if (state->result[0] != 0) strncat(state->result, "; ", sizeof(state->result) - strlen(state->result) - 1);
        strncat(state->result, state->items[i].key, sizeof(state->result) - strlen(state->result) - 1);
    }
}

static int kv_emit(rxvml_context* ctx, const rxvml_address_request* request, const char* text) {
    return rxvml_address_emit_output(ctx, request, text);
}

static int kv_fail(
    rxvml_address_response* response,
    int rc,
    const char* diagnostic) {

    response->rc = rc;
    response->condition_name = "FAILURE";
    response->diagnostic = diagnostic;
    return 0;
}

static int kv_command(
    rxvml_context* ctx,
    const rxvml_address_request* request,
    rxvml_address_response* response,
    void* userdata) {

    kv_state* state = (kv_state*)userdata;
    char op[32];
    char key[KV_KEY_SIZE];
    char resolved_key[KV_KEY_SIZE];
    char target_token[KV_KEY_SIZE];
    char target_name[KV_KEY_SIZE];
    char value_text[KV_VALUE_SIZE];
    char word[32];
    char count_text[32];
    const char* rest;
    int slot;
    int i;

    if (!state || !request || !response) return -1;

    rest = read_word(request->command, op, sizeof(op));
    normalize_word(op);

    if (strcmp(op, "PUT") == 0) {
        rest = read_word(rest, key, sizeof(key));
        if (key[0] == 0) return kv_fail(response, 8, "PUT requires a key");
        if (kv_resolve_token(request, key, resolved_key, sizeof(resolved_key)) != 0)
            return kv_fail(response, 8, "PUT key anchor was not exposed");
        if (kv_resolve_value_text(request, rest, value_text, sizeof(value_text)) != 0)
            return kv_fail(response, 8, "PUT value anchor was not exposed");
        if (kv_set(state, resolved_key, value_text) != 0) return kv_fail(response, 12, "KV store is full");
        response->rc = 0;
        return 0;
    }

    if (strcmp(op, "GET") == 0) {
        rest = read_word(rest, key, sizeof(key));
        if (key[0] == 0) return kv_fail(response, 8, "GET requires a key");
        if (kv_resolve_token(request, key, resolved_key, sizeof(resolved_key)) != 0)
            return kv_fail(response, 8, "GET key anchor was not exposed");
        slot = kv_find(state, resolved_key);
        if (slot < 0) return kv_fail(response, 4, "Key not found");

        rest = read_word(rest, word, sizeof(word));
        if (word[0] != 0) {
            normalize_word(word);
            if (strcmp(word, "INTO") != 0) return kv_fail(response, 8, "GET accepts only optional INTO target");
            read_word(rest, target_token, sizeof(target_token));
            if (!kv_target_name(target_token, target_name, sizeof(target_name)))
                return kv_fail(response, 8, "GET INTO requires a host variable target");
            if (kv_prepare_var_update(state, request, response, target_name, state->items[slot].value) != 0)
                return kv_fail(response, 8, "GET INTO target was not exposed");
            return 0;
        }

        snprintf(state->result, sizeof(state->result), "%s\n", state->items[slot].value);
        return kv_emit(ctx, request, state->result);
    }

    if (strcmp(op, "DEL") == 0 || strcmp(op, "DELETE") == 0) {
        read_word(rest, key, sizeof(key));
        if (kv_resolve_token(request, key, resolved_key, sizeof(resolved_key)) != 0)
            return kv_fail(response, 8, "DEL key anchor was not exposed");
        if (kv_delete(state, resolved_key) != 0) return kv_fail(response, 4, "Key not found");
        response->rc = 0;
        return 0;
    }

    if (strcmp(op, "CLEAR") == 0) {
        kv_clear(state);
        response->rc = 0;
        return 0;
    }

    if (strcmp(op, "COUNT") == 0) {
        rest = read_word(rest, word, sizeof(word));
        if (word[0] != 0) {
            normalize_word(word);
            if (strcmp(word, "INTO") != 0) return kv_fail(response, 8, "COUNT accepts only optional INTO target");
            read_word(rest, target_token, sizeof(target_token));
            if (!kv_target_name(target_token, target_name, sizeof(target_name)))
                return kv_fail(response, 8, "COUNT INTO requires a host variable target");
            snprintf(count_text, sizeof(count_text), "%d", kv_count(state));
            if (kv_prepare_var_update(state, request, response, target_name, count_text) != 0)
                return kv_fail(response, 8, "COUNT INTO target was not exposed");
            return 0;
        }

        snprintf(state->result, sizeof(state->result), "%d\n", kv_count(state));
        return kv_emit(ctx, request, state->result);
    }

    if (strcmp(op, "DUMP") == 0) {
        state->result[0] = 0;
        for (i = 0; i < KV_MAX_ITEMS; i++) {
            if (!state->items[i].used) continue;
            strncat(state->result, state->items[i].key, sizeof(state->result) - strlen(state->result) - 1);
            strncat(state->result, "=", sizeof(state->result) - strlen(state->result) - 1);
            strncat(state->result, state->items[i].value, sizeof(state->result) - strlen(state->result) - 1);
            strncat(state->result, "\n", sizeof(state->result) - strlen(state->result) - 1);
        }
        if (state->result[0] == 0) copy_text(state->result, sizeof(state->result), "(empty)\n");
        return kv_emit(ctx, request, state->result);
    }

    if (strcmp(op, "HELP") == 0 || op[0] == 0) {
        return kv_emit(ctx, request, "PUT key value | GET key [INTO target] | DEL key | CLEAR | COUNT [INTO target] | DUMP\n");
    }

    return kv_fail(response, -3, "Unknown KV command");
}

static int kv_function(
    rxvml_context* ctx,
    const rxvml_address_function_request* request,
    rxvml_address_function_response* response,
    void* userdata) {

    kv_state* state = (kv_state*)userdata;
    char name[64];
    int slot;

    (void)ctx;
    if (!state || !request || !response) return -1;

    copy_text(name, sizeof(name), request->function_name);
    normalize_word(name);

    if (strcmp(name, "ID") == 0) {
        response->result = state->instance_id;
        return 0;
    }

    if (strcmp(name, "GET") == 0) {
        slot = request->argc > 0 ? kv_find(state, request->args[0]) : -1;
        if (slot < 0) {
            response->rc = 4;
            response->condition_name = "FAILURE";
            response->diagnostic = "Key not found";
            response->result = "";
            return 0;
        }
        response->result = state->items[slot].value;
        return 0;
    }

    if (strcmp(name, "PUT") == 0) {
        if (request->argc < 2 || kv_set(state, request->args[0], request->args[1]) != 0) {
            response->rc = 8;
            response->condition_name = "FAILURE";
            response->diagnostic = "PUT requires key and value";
            response->result = "";
            return 0;
        }
        response->result = "OK";
        return 0;
    }

    if (strcmp(name, "COUNT") == 0) {
        snprintf(state->result, sizeof(state->result), "%d", kv_count(state));
        response->result = state->result;
        return 0;
    }

    if (strcmp(name, "EXISTS") == 0) {
        response->result = (request->argc > 0 && kv_find(state, request->args[0]) >= 0) ? "1" : "0";
        return 0;
    }

    if (strcmp(name, "KEYS") == 0) {
        kv_keys(state);
        response->result = state->result;
        return 0;
    }

    response->rc = -3;
    response->condition_name = "FAILURE";
    response->diagnostic = "Unknown KV function";
    response->result = "";
    return 0;
}

static void print_last_error(rxvml_context* ctx, const char* prefix) {
    const char* err = NULL;
    rxvml_last_error(ctx, &err);
    fprintf(stderr, "%s: %s\n", prefix, err ? err : "unknown error");
}

int main(int argc, const char** argv) {
    rxvml_context* ctx = NULL;
    kv_state state;
    int program_rc = 1;
    int status = 1;

    (void)argc;
    (void)argv;

    memset(&state, 0, sizeof(state));
    copy_text(state.instance_id, sizeof(state.instance_id), "native-kv-demo");

    ctx = rxvml_create(NULL, 0);
    if (!ctx) {
        fprintf(stderr, "Failed to create rxvml context\n");
        return 1;
    }

    if (rxvml_load_module_file(ctx, CREXX_NATIVE_KV_LIBRARY_PATH) <= 0) {
        print_last_error(ctx, "Failed to load library");
        goto cleanup;
    }

    if (rxvml_load_module_file(ctx, CREXX_NATIVE_KV_DEMO_MODULE) <= 0) {
        print_last_error(ctx, "Failed to load KV Rexx demo module");
        goto cleanup;
    }

    if (rxvml_address_register_callback_environment(
            ctx, "KV", state.instance_id, kv_command, kv_function, &state) != 0) {
        print_last_error(ctx, "Failed to register KV ADDRESS environment");
        goto cleanup;
    }

    if (rxvml_run(ctx, 0, NULL, &program_rc) != 0) {
        print_last_error(ctx, "Failed to run KV ADDRESS demo");
        goto cleanup;
    }

    status = program_rc;

cleanup:
    rxvml_destroy(ctx);
    return status;
}
