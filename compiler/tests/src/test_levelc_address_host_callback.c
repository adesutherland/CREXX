#include <stdio.h>
#include <string.h>

#include "rxvml.h"

#ifndef CREXX_TEST_BINDIR
#define CREXX_TEST_BINDIR "."
#endif

#ifndef CREXX_TEST_LIBRARY_PATH
#define CREXX_TEST_LIBRARY_PATH "library"
#endif
#ifndef CREXX_TEST_CLASSLIB_PATH
#define CREXX_TEST_CLASSLIB_PATH "classlib"
#endif
#ifndef CREXX_TEST_RXFNSC_PATH
#define CREXX_TEST_RXFNSC_PATH "rxfnsc"
#endif
#ifndef CREXX_TEST_ADDRESS_MODULE
#define CREXX_TEST_ADDRESS_MODULE "levelc_address_host_callback.rxbin"
#endif

typedef struct callback_state {
    int calls;
    int invalid;
} callback_state;

static int check_file(const char *path, const char *expected) {
    char buffer[128];
    FILE *file = fopen(path, "rb");
    size_t size = strlen(expected);
    int okay = file && size < sizeof(buffer) &&
               fread(buffer, 1, size, file) == size &&
               fgetc(file) == EOF && memcmp(buffer, expected, size) == 0;
    if (file) fclose(file);
    if (!okay) fprintf(stderr, "ADDRESS stream content differs: %s\n", path);
    return okay ? 0 : 1;
}

static int editor_command(rxvml_context *ctx,
                          const rxvml_address_request *request,
                          rxvml_address_response *response,
                          void *userdata) {
    callback_state *state = (callback_state *)userdata;
    if (!state || !request || !response ||
        strcmp(request->environment_name, "EDITOR") != 0) return -1;
    state->calls++;
    if (state->calls == 1 && strcmp(request->command, "héllo") == 0) {
        if (rxvml_address_emit_output(ctx, request, "native:héllo\n") != 0)
            state->invalid = 1;
        response->rc = 0;
        return 0;
    }
    if (state->calls == 2 && strcmp(request->command, "write") == 0) {
        if (rxvml_address_emit_output(ctx, request, "host-one\n") != 0)
            state->invalid = 1;
        return 0;
    }
    if (state->calls == 3 && strcmp(request->command, "append") == 0) {
        if (rxvml_address_emit_output(ctx, request, "漢🙂\n") != 0)
            state->invalid = 1;
        return 0;
    }
    if (state->calls == 4 && strcmp(request->command, "err") == 0) {
        if (rxvml_address_emit_error(ctx, request, "err-🙂\n") != 0)
            state->invalid = 1;
        return 0;
    }
    if (state->calls == 5 && strcmp(request->command, "errappend") == 0) {
        if (rxvml_address_emit_error(ctx, request, "tail-🙂\n") != 0)
            state->invalid = 1;
        return 0;
    }
    if (state->calls == 6 && strcmp(request->command, "errstem") == 0) {
        if (rxvml_address_emit_error(ctx, request, "err-one\n") != 0)
            state->invalid = 1;
        return 0;
    }
    if (state->calls == 7 && strcmp(request->command, "errstemappend") == 0) {
        if (rxvml_address_emit_error(ctx, request, "err-two\n") != 0)
            state->invalid = 1;
        return 0;
    }
    if (state->calls == 8 && strcmp(request->command, "fail") == 0) {
        response->rc = -9;
        response->condition_name = "FAILURE";
        response->diagnostic = "native editor rejected command";
        return 0;
    }
    if (state->calls == 9 && strcmp(request->command, "error") == 0) {
        response->rc = 7;
        response->condition_name = "ERROR";
        response->diagnostic = "native editor reported command error";
        return 0;
    }
    if (state->calls == 10 && strcmp(request->command, "héllo🙂") == 0) {
        if (rxvml_address_emit_output(ctx, request, "native:héllo🙂\n") != 0)
            state->invalid = 1;
        return 0;
    }
    if (state->calls == 11 && strcmp(request->command, "signalfail") == 0) {
        response->rc = -9;
        response->condition_name = "FAILURE";
        response->diagnostic = "native editor rejected command";
        return 0;
    }
    if (state->calls == 12 && strcmp(request->command, "signalerror") == 0) {
        response->rc = 7;
        response->condition_name = "ERROR";
        response->diagnostic = "native editor reported command error";
        return 0;
    }
    state->invalid = 1;
    response->rc = -10;
    return 0;
}

int main(void) {
    rxvml_context *ctx = rxvml_create(CREXX_TEST_BINDIR, 0);
    callback_state state = {0, 0};
    const char *error = NULL;
    int program_rc = -1;
    int failed = 0;
    if (!ctx) return 1;

    if (rxvml_load_module_file(ctx, CREXX_TEST_LIBRARY_PATH) <= 0 ||
        rxvml_load_module_file(ctx, CREXX_TEST_CLASSLIB_PATH) <= 0 ||
        rxvml_load_module_file(ctx, CREXX_TEST_RXFNSC_PATH) <= 0 ||
        rxvml_load_module_file(ctx, CREXX_TEST_ADDRESS_MODULE) <= 0) {
        rxvml_last_error(ctx, &error);
        fprintf(stderr, "ADDRESS fixture load failed: %s\n",
                error ? error : "unknown error");
        failed = 1;
        goto done;
    }
    if (rxvml_address_register_callback_environment(
            ctx, "EDITOR", "levelc-editor", editor_command, NULL, &state) != 0) {
        rxvml_last_error(ctx, &error);
        fprintf(stderr, "ADDRESS callback registration failed: %s\n",
                error ? error : "unknown error");
        failed = 1;
        goto done;
    }
    if (rxvml_run(ctx, 0, NULL, &program_rc) != 0) {
        rxvml_last_error(ctx, &error);
        fprintf(stderr, "ADDRESS host run failed: %s\n",
                error ? error : "unknown error");
        failed = 1;
        goto done;
    }
    if (program_rc != 0 || state.calls != 12 || state.invalid) {
        fprintf(stderr, "ADDRESS host result: rc=%d calls=%d invalid=%d\n",
                program_rc, state.calls, state.invalid);
        failed = 1;
    }
    failed |= check_file("levelc-address-host-output.txt", "host-one\n漢🙂\n");
    failed |= check_file("levelc-address-host-error.txt", "err-🙂\ntail-🙂\n");

done:
    remove("levelc-address-host-output.txt");
    remove("levelc-address-host-error.txt");
    rxvml_destroy(ctx);
    return failed;
}
