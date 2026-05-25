#include <stdio.h>
#include <string.h>
#include "crexxsaa.h"

#ifndef CREXX_TEST_LOCATION
#define CREXX_TEST_LOCATION "."
#endif

#ifndef CREXX_TEST_LIBRARY_RXBIN
#define CREXX_TEST_LIBRARY_RXBIN "library.rxbin"
#endif

#ifndef CREXX_TEST_CREXXSAA_VARIABLE_MODULE
#define CREXX_TEST_CREXXSAA_VARIABLE_MODULE "crexxsaa_variables_host.rxbin"
#endif

typedef struct host_state {
    int calls;
    int direct_seen;
    int scalar_compat_seen;
    int stem_seen;
    int sandbox_seen;
    int invalid_utf8_seen;
} host_state;

static int check_text(const char* label, const char* actual, const char* expected) {
    if (strcmp(actual ? actual : "", expected ? expected : "") == 0) return 0;
    fprintf(stderr, "%s: expected '%s', got '%s'\n", label, expected, actual ? actual : "");
    return 1;
}

static int get_variable_text(
    const crexxsaa_address_request* request,
    const char* name,
    char** value_out) {

    size_t value_len = 0;
    int rc;

    *value_out = NULL;
    rc = crexxsaa_address_variable_get_alloc(request->context, name, value_out, &value_len);
    if (rc != CREXXSAA_VARIABLE_OK) {
        fprintf(stderr, "get %s failed: %s\n", name, crexxsaa_last_error(request->context));
    }
    return rc;
}

static int set_variable_text(
    const crexxsaa_address_request* request,
    const char* name,
    const char* value) {

    int rc = crexxsaa_address_variable_set(request->context, name, value, strlen(value));
    if (rc != CREXXSAA_VARIABLE_OK) {
        fprintf(stderr, "set %s failed: %s\n", name, crexxsaa_last_error(request->context));
    }
    return rc;
}

static int reject_invalid_variable_text(
    const crexxsaa_address_request* request,
    const char* name) {

    char invalid_utf8[] = { (char)0xff, '\0' };
    int rc = crexxsaa_address_variable_set(request->context, name, invalid_utf8, 1);
    if (rc == CREXXSAA_VARIABLE_OK) {
        fprintf(stderr, "set %s with invalid UTF-8 unexpectedly succeeded\n", name);
        return 1;
    }
    if (!strstr(crexxsaa_last_error(request->context), "Invalid UTF-8")) {
        fprintf(stderr,
                "set %s with invalid UTF-8 failed with unexpected error: %s\n",
                name,
                crexxsaa_last_error(request->context));
        return 1;
    }
    return 0;
}

static int editor_callback(
    const crexxsaa_address_request* request,
    crexxsaa_address_response* response,
    void* userdata) {

    host_state* state = (host_state*)userdata;
    char* value = NULL;

    if (!request || !request->context || !response || !state) return -1;
    state->calls++;
    response->rc = 0;

    if (strcmp(request->command, "DIRECT") == 0) {
        if (get_variable_text(request, "DIRECT", &value) != CREXXSAA_VARIABLE_OK) {
            response->rc = 10;
            return 0;
        }
        if (check_text("direct input", value, "client-input")) {
            crexxsaa_free(value);
            response->rc = 11;
            return 0;
        }
        crexxsaa_free(value);
        if (set_variable_text(request, "direct", "client-updated") != CREXXSAA_VARIABLE_OK) {
            response->rc = 12;
            return 0;
        }
        if (reject_invalid_variable_text(request, "direct")) {
            response->rc = 13;
            return 0;
        }
        state->invalid_utf8_seen = 1;
        state->direct_seen = 1;
        return 0;
    }

    if (strcmp(request->command, "SCALAR_COMPAT") == 0) {
        if (get_variable_text(request, "scalar.0", &value) != CREXXSAA_VARIABLE_OK) {
            response->rc = 40;
            return 0;
        }
        if (check_text("scalar.0", value, "1")) {
            crexxsaa_free(value);
            response->rc = 41;
            return 0;
        }
        crexxsaa_free(value);

        if (get_variable_text(request, "scalar.1", &value) != CREXXSAA_VARIABLE_OK) {
            response->rc = 42;
            return 0;
        }
        if (check_text("scalar.1", value, "scalar-before")) {
            crexxsaa_free(value);
            response->rc = 43;
            return 0;
        }
        crexxsaa_free(value);

        if (set_variable_text(request, "scalar.0", "ignored-zero") != CREXXSAA_VARIABLE_OK) {
            response->rc = 44;
            return 0;
        }
        if (get_variable_text(request, "scalar", &value) != CREXXSAA_VARIABLE_OK) {
            response->rc = 45;
            return 0;
        }
        if (check_text("scalar after scalar.0 write", value, "scalar-before")) {
            crexxsaa_free(value);
            response->rc = 46;
            return 0;
        }
        crexxsaa_free(value);

        if (set_variable_text(request, "scalar.9", "scalar-from-tail") != CREXXSAA_VARIABLE_OK) {
            response->rc = 47;
            return 0;
        }
        if (get_variable_text(request, "scalar.1", &value) != CREXXSAA_VARIABLE_OK) {
            response->rc = 48;
            return 0;
        }
        if (check_text("scalar.1 after scalar.9 write", value, "scalar-from-tail")) {
            crexxsaa_free(value);
            response->rc = 49;
            return 0;
        }
        crexxsaa_free(value);

        state->scalar_compat_seen = 1;
        return 0;
    }

    if (strcmp(request->command, "STEM") == 0) {
        if (set_variable_text(request, "STEM.1", "stem-one") != CREXXSAA_VARIABLE_OK ||
            set_variable_text(request, "stem.0", "1") != CREXXSAA_VARIABLE_OK) {
            response->rc = 20;
            return 0;
        }
        state->stem_seen = 1;
        return 0;
    }

    if (strcmp(request->command, "SANDBOX") == 0) {
        if (get_variable_text(request, "sandbox_in", &value) != CREXXSAA_VARIABLE_OK) {
            response->rc = 30;
            return 0;
        }
        if (check_text("sandbox input", value, "sandpit-input")) {
            crexxsaa_free(value);
            response->rc = 31;
            return 0;
        }
        crexxsaa_free(value);
        if (set_variable_text(request, "SANDBOX_IN", "sandpit-input-read") != CREXXSAA_VARIABLE_OK ||
            set_variable_text(request, "sandbox_out", "sandpit-output") != CREXXSAA_VARIABLE_OK) {
            response->rc = 32;
            return 0;
        }
        state->sandbox_seen = 1;
        return 0;
    }

    response->rc = 99;
    return 0;
}

int main(void) {
    crexxsaa_context* ctx = NULL;
    host_state state;
    int program_rc = -1;
    int status = 1;

    memset(&state, 0, sizeof(state));

    if (crexxsaa_create(CREXX_TEST_LOCATION, CREXX_TEST_LIBRARY_RXBIN, &ctx) != 0 || !ctx) {
        fprintf(stderr, "Failed to create crexxsaa context\n");
        return 1;
    }

    if (crexxsaa_register_address_environment(ctx, "EDITOR", editor_callback, &state) != 0) {
        fprintf(stderr, "Failed to register EDITOR environment: %s\n", crexxsaa_last_error(ctx));
        goto cleanup;
    }

    if (crexxsaa_set_address_environment(ctx, "EDITOR") != 0) {
        fprintf(stderr, "Failed to set EDITOR environment: %s\n", crexxsaa_last_error(ctx));
        goto cleanup;
    }

    if (crexxsaa_run_rxbin(ctx, CREXX_TEST_CREXXSAA_VARIABLE_MODULE, 0, NULL, &program_rc) != 0) {
        fprintf(stderr, "Failed to run variable fixture: %s\n", crexxsaa_last_error(ctx));
        goto cleanup;
    }

    if (program_rc != 0) {
        fprintf(stderr, "Variable fixture returned %d\n", program_rc);
        goto cleanup;
    }

    if (state.calls != 4 ||
        !state.direct_seen ||
        !state.scalar_compat_seen ||
        !state.stem_seen ||
        !state.sandbox_seen ||
        !state.invalid_utf8_seen) {
        fprintf(stderr,
                "Unexpected callback coverage: calls=%d direct=%d scalar_compat=%d stem=%d sandbox=%d invalid_utf8=%d\n",
                state.calls,
                state.direct_seen,
                state.scalar_compat_seen,
                state.stem_seen,
                state.sandbox_seen,
                state.invalid_utf8_seen);
        goto cleanup;
    }

    status = 0;

cleanup:
    crexxsaa_destroy(ctx);
    return status;
}
