#include <stdio.h>
#include <string.h>
#include "rxvml.h"

#ifndef CREXX_TEST_LIBRARY_PATH
#define CREXX_TEST_LIBRARY_PATH "library"
#endif

#ifndef CREXX_TEST_ADDRESS_CALLBACK_MODULE
#define CREXX_TEST_ADDRESS_CALLBACK_MODULE "address_callback_host.rxbin"
#endif

typedef struct host_state {
    int calls;
    char envs[4][32];
    char commands[4][64];
    int binding_counts[4];
    char first_binding_name[32];
    char first_binding_value[64];
} host_state;

static void copy_text(char* dest, size_t dest_len, const char* src) {
    if (!dest || dest_len == 0) return;
    if (!src) src = "";
    strncpy(dest, src, dest_len - 1);
    dest[dest_len - 1] = 0;
}

static void print_last_error(rxvml_context* ctx, const char* prefix) {
    const char* err = NULL;
    rxvml_last_error(ctx, &err);
    fprintf(stderr, "%s: %s\n", prefix, err ? err : "unknown error");
}

static int editor_callback(
    rxvml_context* ctx,
    const rxvml_address_request* request,
    rxvml_address_response* response,
    void* userdata) {

    host_state* state = (host_state*)userdata;
    int slot;

    (void)ctx;

    if (!state || !request || !response) return -99;
    if (state->calls >= 4) return -98;

    slot = state->calls;
    copy_text(state->envs[slot], sizeof(state->envs[slot]), request->environment_name);
    copy_text(state->commands[slot], sizeof(state->commands[slot]), request->command);
    state->binding_counts[slot] = (int)request->binding_count;

    if (slot == 0 && request->binding_count > 0) {
        copy_text(state->first_binding_name, sizeof(state->first_binding_name), request->bindings[0].internal_name);
        copy_text(state->first_binding_value, sizeof(state->first_binding_value), request->bindings[0].value);
    }

    state->calls++;

    if (strcmp(request->command, "RETURN 42") == 0) {
        response->rc = 42;
        return 0;
    }

    response->rc = 0;
    return 0;
}

static int check_equal(const char* label, const char* actual, const char* expected) {
    if (strcmp(actual, expected) == 0) return 0;
    fprintf(stderr, "%s: expected '%s', got '%s'\n", label, expected, actual);
    return 1;
}

static void dump_state(const host_state* state) {
    int i;
    fprintf(stderr, "callback calls=%d\n", state ? state->calls : -1);
    if (!state) return;
    for (i = 0; i < state->calls && i < 4; i++) {
        fprintf(stderr, "call %d env='%s' command='%s' bindings=%d\n",
                i + 1, state->envs[i], state->commands[i], state->binding_counts[i]);
    }
}

int main(void) {
    rxvml_context* ctx = NULL;
    rxvml_value* result = NULL;
    rxinteger result_code = -1;
    host_state state;
    int status = 1;

    memset(&state, 0, sizeof(state));

    ctx = rxvml_create(NULL, 0);
    if (!ctx) {
        fprintf(stderr, "Failed to create rxvml context\n");
        return 1;
    }

    if (rxvml_load_module_file(ctx, CREXX_TEST_LIBRARY_PATH) <= 0) {
        print_last_error(ctx, "Failed to load library");
        goto cleanup;
    }

    if (rxvml_load_module_file(ctx, CREXX_TEST_ADDRESS_CALLBACK_MODULE) <= 0) {
        print_last_error(ctx, "Failed to load address callback fixture");
        goto cleanup;
    }

    if (rxvml_address_register_callback_environment(ctx, "EDITOR", editor_callback, &state) != 0) {
        print_last_error(ctx, "Failed to register native callback address environment");
        goto cleanup;
    }

    if (rxvml_call_procedure(ctx, "address_callback_host.address_callback_host", 0, NULL, &result) != 0 || !result) {
        print_last_error(ctx, "Failed to run address callback fixture");
        goto cleanup;
    }

    if (rxvml_to_int(ctx, result, &result_code) != 0 || result_code != 0) {
        fprintf(stderr, "Fixture returned %lld\n", (long long)result_code);
        dump_state(&state);
        goto cleanup;
    }

    if (state.calls != 3) {
        fprintf(stderr, "Expected 3 callback calls, got %d\n", state.calls);
        goto cleanup;
    }

    if (check_equal("call 1 env", state.envs[0], "EDITOR")) goto cleanup;
    if (check_equal("call 1 command", state.commands[0], "OPEN demo.txt")) goto cleanup;
    if (state.binding_counts[0] != 1) {
        fprintf(stderr, "Expected 1 binding on first call, got %d\n", state.binding_counts[0]);
        goto cleanup;
    }
    if (check_equal("first binding name", state.first_binding_name, "buffer")) goto cleanup;
    if (check_equal("first binding value", state.first_binding_value, "example-value")) goto cleanup;

    if (check_equal("call 2 env", state.envs[1], "EDITOR")) goto cleanup;
    if (check_equal("call 2 command", state.commands[1], "CURSOR 7 9")) goto cleanup;
    if (state.binding_counts[1] != 0) {
        fprintf(stderr, "Expected 0 bindings on implicit call, got %d\n", state.binding_counts[1]);
        goto cleanup;
    }

    if (check_equal("call 3 env", state.envs[2], "EDITOR")) goto cleanup;
    if (check_equal("call 3 command", state.commands[2], "RETURN 42")) goto cleanup;

    status = 0;

cleanup:
    if (result) rxvml_value_free(result);
    rxvml_destroy(ctx);
    return status;
}
