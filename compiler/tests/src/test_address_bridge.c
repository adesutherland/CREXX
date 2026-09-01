#include <stdio.h>
#include <string.h>
#include "rxvml.h"

#ifndef CREXX_TEST_LIBRARY_PATH
#define CREXX_TEST_LIBRARY_PATH "library"
#endif

#ifndef CREXX_TEST_CMS_ADDRESS_MODULE
#define CREXX_TEST_CMS_ADDRESS_MODULE "address_cms_provider.rxbin"
#endif

#ifndef CREXX_TEST_CMS_ADDRESS_HOST_MODULE
#define CREXX_TEST_CMS_ADDRESS_HOST_MODULE "address_cms_host.rxbin"
#endif

static void print_last_error(rxvml_context* ctx, const char* prefix) {
    const char* err = NULL;
    rxvml_last_error(ctx, &err);
    fprintf(stderr, "%s: %s\n", prefix, err ? err : "unknown error");
}

int main(void) {
    rxvml_context* ctx = NULL;
    rxvml_value* env_obj = NULL;
    rxvml_value* current_env = NULL;
    rxvml_value* host_result = NULL;
    const char* current_name = NULL;
    const char* embed_name = "EMBEDCMS";
    rxinteger host_errors = -1;
    int register_rc = -1;
    int set_rc = -1;
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

    if (rxvml_load_module_file(ctx, CREXX_TEST_CMS_ADDRESS_MODULE) <= 0) {
        print_last_error(ctx, "Failed to load CMS address provider fixture");
        goto cleanup;
    }

    if (rxvml_load_module_file(ctx, CREXX_TEST_CMS_ADDRESS_HOST_MODULE) <= 0) {
        print_last_error(ctx, "Failed to load CMS address host fixture");
        goto cleanup;
    }

    if (rxvml_call_procedure_descriptor(
            ctx,
            "rxsig1|address_cms_host.address_cms_host|.int|",
            0,
            NULL,
            &host_result) != 0 || !host_result) {
        print_last_error(ctx, "Failed to run CMS address host fixture");
        goto cleanup;
    }

    if (rxvml_to_int(ctx, host_result, &host_errors) != 0 || host_errors != 0) {
        fprintf(stderr, "CMS address host fixture reported %lld errors\n", (long long)host_errors);
        goto cleanup;
    }

    if (rxvml_address_create_environment(ctx, "CMS", &env_obj) != 0 || !env_obj) {
        print_last_error(ctx, "Failed to obtain CMS address environment through the ADDRESS factory");
        goto cleanup;
    }

    register_rc = rxvml_address_register_environment(ctx, embed_name, env_obj);
    if (register_rc != 0) {
        if (register_rc < 0) print_last_error(ctx, "Failed to register address environment");
        else fprintf(stderr, "Address registration returned rc=%d\n", register_rc);
        goto cleanup;
    }
    rxvml_value_free(env_obj);
    env_obj = NULL;

    set_rc = rxvml_address_set_environment(ctx, embed_name);
    if (set_rc != 0) {
        if (set_rc < 0) print_last_error(ctx, "Failed to seed current address environment");
        else fprintf(stderr, "Address set returned rc=%d\n", set_rc);
        goto cleanup;
    }

    if (rxvml_call_procedure_descriptor(
            ctx,
            "rxsig1|_rxsysb._current_address_environment|.string|",
            0,
            NULL,
            &current_env) != 0 || !current_env) {
        print_last_error(ctx, "Failed to query current address environment");
        goto cleanup;
    }

    if (rxvml_to_str(ctx, current_env, &current_name, NULL) != 0 || !current_name || strcmp(current_name, embed_name) != 0) {
        fprintf(stderr, "Unexpected current address environment: %s\n", current_name ? current_name : "(null)");
        goto cleanup;
    }

    status = 0;

cleanup:
    if (env_obj) rxvml_value_free(env_obj);
    if (current_env) rxvml_value_free(current_env);
    if (host_result) rxvml_value_free(host_result);
    rxvml_destroy(ctx);
    return status;
}
