#include <stdio.h>
#include <string.h>
#include "rxvml.h"

static void print_last_error(rxvml_context* ctx, const char* prefix) {
    const char* err = NULL;
    rxvml_last_error(ctx, &err);
    fprintf(stderr, "%s: %s\n", prefix, err ? err : "unknown error");
}

int main(void) {
    rxvml_context* ctx = NULL;
    rxvml_value* env_obj = NULL;
    rxvml_value* current_env = NULL;
    const char* current_name = NULL;
    const char* embed_name = "EMBEDCMS";
    int register_rc = -1;
    int set_rc = -1;
    int status = 1;

    ctx = rxvml_create(NULL, 0);
    if (!ctx) {
        fprintf(stderr, "Failed to create rxvml context\n");
        return 1;
    }

    if (rxvml_load_module_file(ctx, "library") <= 0) {
        print_last_error(ctx, "Failed to load library");
        goto cleanup;
    }

    if (rxvml_call_procedure(ctx, "_rxsysb.cmsaddressenvironment", 0, NULL, &env_obj) != 0 || !env_obj) {
        print_last_error(ctx, "Failed to obtain CMS address environment");
        goto cleanup;
    }

    register_rc = rxvml_address_register_environment(ctx, embed_name, env_obj);
    if (register_rc != 0) {
        if (register_rc < 0) print_last_error(ctx, "Failed to register address environment");
        else fprintf(stderr, "Address registration returned rc=%d\n", register_rc);
        goto cleanup;
    }

    set_rc = rxvml_address_set_environment(ctx, embed_name);
    if (set_rc != 0) {
        if (set_rc < 0) print_last_error(ctx, "Failed to seed current address environment");
        else fprintf(stderr, "Address set returned rc=%d\n", set_rc);
        goto cleanup;
    }

    if (rxvml_call_procedure(ctx, "_rxsysb._current_address_environment", 0, NULL, &current_env) != 0 || !current_env) {
        print_last_error(ctx, "Failed to query current address environment");
        goto cleanup;
    }

    if (rxvml_to_str(ctx, current_env, &current_name, NULL) != 0 || !current_name || strcmp(current_name, embed_name) != 0) {
        fprintf(stderr, "Unexpected current address environment: %s\n", current_name ? current_name : "(null)");
        goto cleanup;
    }

    status = 0;

cleanup:
    if (current_env) rxvml_value_free(current_env);
    rxvml_destroy(ctx);
    return status;
}
