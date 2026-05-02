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
    char envs[7][32];
    char commands[7][64];
    int binding_counts[7];
    char first_binding_name[32];
    char first_binding_value[64];
    char updated_value[64];
    char sandbox_value[64];
    char stem_value[64];
    char sandbox_updated_value[96];
    int function_calls;
    char function_names[3][32];
    char function_first_args[3][64];
    char function_result[128];
    rxvml_address_binding updates[1];
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
    size_t i;

    if (!state || !request || !response) return -99;
    if (state->calls >= 7) return -98;

    slot = state->calls;
    copy_text(state->envs[slot], sizeof(state->envs[slot]), request->environment_name);
    copy_text(state->commands[slot], sizeof(state->commands[slot]), request->command);
    state->binding_counts[slot] = (int)request->binding_count;

    if (slot == 0 && request->binding_count > 0) {
        copy_text(state->first_binding_name, sizeof(state->first_binding_name), request->bindings[0].internal_name);
        copy_text(state->first_binding_value, sizeof(state->first_binding_value), request->bindings[0].value);
    }

    state->calls++;

    if (strcmp(request->command, "OPEN demo.txt") == 0 && request->binding_count > 0) {
        copy_text(state->updated_value, sizeof(state->updated_value), "native-updated");
        state->updates[0].kind = "var";
        state->updates[0].internal_name = request->bindings[0].internal_name;
        state->updates[0].external_alias = request->bindings[0].external_alias;
        state->updates[0].value = state->updated_value;
        state->updates[0].flags = "";
        response->updated_binding_count = 1;
        response->updated_bindings = state->updates;
    }

    if (strcmp(request->command, "RETURN 42") == 0) {
        response->rc = 42;
        return 0;
    }

    if (strcmp(request->command, "SANDBOX DIRECT METHOD") == 0) {
        if (rxvml_address_sandbox_set(ctx, request, "direct", "native-direct") != 0) {
            response->rc = -8;
            return 0;
        }
        response->rc = 0;
        return 0;
    }

    if (strcmp(request->command, "SANDBOX ROUNDTRIP") == 0) {
        char updated[96];
        if (rxvml_address_sandbox_get(request, "VALUE.3", state->sandbox_value, sizeof(state->sandbox_value)) != 0) {
            response->rc = -7;
            return 0;
        }
        snprintf(updated, sizeof(updated), "%s:native", state->sandbox_value);
        copy_text(state->sandbox_updated_value, sizeof(state->sandbox_updated_value), updated);
        state->updates[0].kind = "SANDBOX";
        state->updates[0].internal_name = "result";
        state->updates[0].external_alias = "result";
        state->updates[0].value = state->sandbox_updated_value;
        state->updates[0].flags = "";
        response->updated_binding_count = 1;
        response->updated_bindings = state->updates;
        response->rc = 0;
        return 0;
    }

    if (strcmp(request->command, "EXPOSE ARRAY") == 0) {
        for (i = 0; i < request->binding_count; i++) {
            const rxvml_address_binding* binding = &request->bindings[i];
            if (strcmp(binding->kind, "stem") == 0 &&
                strcmp(binding->internal_name, "items") == 0) {
                int helper_rc = rxvml_address_binding_stem_get(binding, "1", state->stem_value, sizeof(state->stem_value));
                if (helper_rc != 0) {
                    fprintf(stderr, "stem get helper returned %d\n", helper_rc);
                    response->rc = -9;
                    return 0;
                }
                helper_rc = rxvml_address_binding_stem_set(ctx, binding, "2", "native-two-updated");
                if (helper_rc != 0) {
                    fprintf(stderr, "stem set helper for 2 returned %d\n", helper_rc);
                    response->rc = -10;
                    return 0;
                }
                helper_rc = rxvml_address_binding_stem_set(ctx, binding, "3", "native-three");
                if (helper_rc != 0) {
                    fprintf(stderr, "stem set helper for 3 returned %d\n", helper_rc);
                    response->rc = -10;
                    return 0;
                }
                helper_rc = rxvml_address_binding_stem_set(ctx, binding, "0", "3");
                if (helper_rc != 0) {
                    fprintf(stderr, "stem set helper for 0 returned %d\n", helper_rc);
                    response->rc = -10;
                    return 0;
                }
            }
        }
        response->rc = 0;
        return 0;
    }

    response->rc = 0;
    return 0;
}

static int editor_function_callback(
    rxvml_context* ctx,
    const rxvml_address_function_request* request,
    rxvml_address_function_response* response,
    void* userdata) {

    host_state* state = (host_state*)userdata;
    int slot;

    (void)ctx;

    if (!state || !request || !response) return -99;
    if (state->function_calls >= 3) return -98;

    slot = state->function_calls;
    copy_text(state->function_names[slot], sizeof(state->function_names[slot]), request->function_name);
    if (request->argc > 0) {
        copy_text(state->function_first_args[slot], sizeof(state->function_first_args[slot]), request->args[0]);
    }
    state->function_calls++;

    if (strcmp(request->function_name, "describe") == 0) {
        snprintf(state->function_result, sizeof(state->function_result), "native:client-7:%s",
                 request->argc > 0 ? request->args[0] : "");
        response->rc = 0;
        response->result = state->function_result;
        return 0;
    }

    if (strcmp(request->function_name, "id") == 0) {
        copy_text(state->function_result, sizeof(state->function_result), "client-7");
        response->rc = 0;
        response->result = state->function_result;
        return 0;
    }

    response->rc = -3;
    response->condition_name = "FAILURE";
    response->diagnostic = "unknown native function";
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
    for (i = 0; i < state->calls && i < 7; i++) {
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

    if (rxvml_address_register_callback_environment(
            ctx, "EDITOR", "client-7", editor_callback, editor_function_callback, &state) != 0) {
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

    if (state.calls != 7) {
        fprintf(stderr, "Expected 7 callback calls, got %d\n", state.calls);
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
    if (check_equal("updated value", state.updated_value, "native-updated")) goto cleanup;

    if (check_equal("call 2 env", state.envs[1], "EDITOR")) goto cleanup;
    if (check_equal("call 2 command", state.commands[1], "CURSOR 7 9")) goto cleanup;
    if (state.binding_counts[1] != 0) {
        fprintf(stderr, "Expected 0 bindings on implicit call, got %d\n", state.binding_counts[1]);
        goto cleanup;
    }

    if (check_equal("call 3 env", state.envs[2], "EDITOR")) goto cleanup;
    if (check_equal("call 3 command", state.commands[2], "RETURN 42")) goto cleanup;

    if (check_equal("call 4 env", state.envs[3], "EDITOR")) goto cleanup;
    if (check_equal("call 4 command", state.commands[3], "SANDBOX DIRECT METHOD")) goto cleanup;

    if (check_equal("call 5 env", state.envs[4], "EDITOR")) goto cleanup;
    if (check_equal("call 5 command", state.commands[4], "SANDBOX ROUNDTRIP")) goto cleanup;
    if (check_equal("sandbox value", state.sandbox_value, "native-input")) goto cleanup;

    if (check_equal("call 6 env", state.envs[5], "EDITOR")) goto cleanup;
    if (check_equal("call 6 command", state.commands[5], "SANDBOX DIRECT METHOD")) goto cleanup;

    if (check_equal("call 7 env", state.envs[6], "EDITOR")) goto cleanup;
    if (check_equal("call 7 command", state.commands[6], "EXPOSE ARRAY")) goto cleanup;
    if (state.binding_counts[6] != 1) {
        fprintf(stderr, "Expected 1 binding on array call, got %d\n", state.binding_counts[6]);
        goto cleanup;
    }
    if (check_equal("stem value", state.stem_value, "native-one")) goto cleanup;
    if (state.function_calls != 3) {
        fprintf(stderr, "Expected 3 function callback calls, got %d\n", state.function_calls);
        goto cleanup;
    }
    if (check_equal("function 1 name", state.function_names[0], "describe")) goto cleanup;
    if (check_equal("function 1 first arg", state.function_first_args[0], "alpha")) goto cleanup;
    if (check_equal("function 2 name", state.function_names[1], "id")) goto cleanup;
    if (check_equal("function 3 name", state.function_names[2], "id")) goto cleanup;

    status = 0;

cleanup:
    if (result) rxvml_value_free(result);
    rxvml_destroy(ctx);
    return status;
}
