#include <stdio.h>
#include <string.h>
#include "rxvml.h"

#ifndef CREXX_TEST_LIBRARY_PATH
#define CREXX_TEST_LIBRARY_PATH "library"
#endif

#ifndef CREXX_TEST_DYNAMIC_LOAD_HOST_MODULE
#define CREXX_TEST_DYNAMIC_LOAD_HOST_MODULE "dynamic_load_host.rxbin"
#endif

#ifndef CREXX_TEST_DYNAMIC_LOAD_PROVIDER_MODULE
#define CREXX_TEST_DYNAMIC_LOAD_PROVIDER_MODULE "dynamic_load_provider.rxbin"
#endif

static void print_last_error(rxvml_context* ctx, const char* prefix) {
    const char* err = NULL;
    rxvml_last_error(ctx, &err);
    fprintf(stderr, "%s: %s\n", prefix, err ? err : "unknown error");
}

int main(void) {
    rxvml_context* ctx = NULL;
    rxvml_value* provider_path = NULL;
    rxvml_value* result = NULL;
    rxvml_value* args[1];
    rxinteger rc = -1;
    int status = 1;

    ctx = rxvml_create(NULL, 0);
    if (!ctx) {
        fprintf(stderr, "Failed to create rxvml context\n");
        return 1;
    }

    if (rxvml_load_module_file(ctx, CREXX_TEST_LIBRARY_PATH) <= 0) {
        print_last_error(ctx, "Failed to load library");
        goto cleanup;
    }

    if (rxvml_load_module_file(ctx, CREXX_TEST_DYNAMIC_LOAD_HOST_MODULE) <= 0) {
        print_last_error(ctx, "Failed to load dynamic load host fixture");
        goto cleanup;
    }

    provider_path = rxvml_value_new(ctx);
    if (!provider_path) {
        fprintf(stderr, "Failed to allocate provider path value\n");
        goto cleanup;
    }
    rxvml_set_str(provider_path,
                  CREXX_TEST_DYNAMIC_LOAD_PROVIDER_MODULE,
                  strlen(CREXX_TEST_DYNAMIC_LOAD_PROVIDER_MODULE));
    args[0] = provider_path;

    if (rxvml_call_procedure(ctx, "dynamic_load_host.dynamic_load_host", 1, args, &result) != 0 || !result) {
        print_last_error(ctx, "Failed to run dynamic load fixture");
        goto cleanup;
    }

    if (rxvml_to_int(ctx, result, &rc) != 0 || rc != 0) {
        fprintf(stderr, "Dynamic load fixture returned %lld\n", (long long)rc);
        goto cleanup;
    }

    status = 0;

cleanup:
    if (provider_path) rxvml_value_free(provider_path);
    if (result) rxvml_value_free(result);
    rxvml_destroy(ctx);
    return status;
}
