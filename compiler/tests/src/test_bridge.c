#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "rxvml.h"

int main(int argc, char** argv) {
    rxvml_context* ctx;
    rxvml_value* tok_array;
    rxvml_value* tok_obj = NULL;
    rxvml_value* response = NULL;
    const char* code;

    setbuf(stdout, NULL);

    printf("Starting Bridge Test...\n");

    /* 1. Initialize Context */
    ctx = rxvml_create(NULL, 0);
    if (!ctx) {
        fprintf(stderr, "Failed to create rxvml context\n");
        return 1;
    }

    /* 2. Load the plugin */
    /* rxvml_load_module_file adds .rxbin extension */
    if (rxvml_load_module_file(ctx, "stub_plugin") <= 0) {
        const char* err;
        rxvml_last_error(ctx, &err);
        fprintf(stderr, "Failed to load stub_plugin: %s\n", err ? err : "unknown error");
        return 1;
    }

    /* 3. Manually construct a token array */
    tok_array = rxvml_array_new(ctx, 1);
    
    rxvml_value* args[9];
    int i;
    for (i=0; i<9; i++) args[i] = rxvml_value_new(ctx);
    rxvml_set_int(args[0], 100); /* DUMMY_TYPE */
    rxvml_set_int(args[1], 0);
    rxvml_set_str(args[2], "PARSE", 5);
    rxvml_set_int(args[3], 10);
    rxvml_set_int(args[4], 1);
    rxvml_set_int(args[5], 5);
    rxvml_set_str(args[6], "test.rexx", 9);
    rxvml_set_int(args[7], 0);
    rxvml_set_int(args[8], 0);
    
    if (rxvml_call_factory(ctx, "stub.token", 9, args, &tok_obj) != 0 || !tok_obj) {
        fprintf(stderr, "Failed to call factory for stub.token\n");
        return 1;
    }
    for (i=0; i<9; i++) rxvml_value_free(args[i]);

    rxvml_array_set(ctx, tok_array, 1, tok_obj);
    rxvml_value_free(tok_obj);

    /* 4. Call the plugin (procedure) */
    printf("Calling stub.stub_plugin...\n");
    rxvml_value* call_args[1] = { tok_array };
    if (rxvml_call_procedure(ctx, "stub.stub_plugin", 1, call_args, &response) != 0) {
        const char* err;
        rxvml_last_error(ctx, &err);
        fprintf(stderr, "Failed to call stub_plugin: %s\n", err ? err : "unknown error");
        return 1;
    }

    /* 5. Assert results */
    if (response == NULL) {
        fprintf(stderr, "Plugin returned NULL response\n");
        return 1;
    }

    if (rxvml_to_str(ctx, response, &code, NULL) == 0) {
        printf("Plugin returned code (string): %s\n", code ? code : "NULL");
    } else {
        printf("Plugin returned non-string result\n");
        return 1;
    }

    if (code && strcmp(code, "SAY 'HELLO FROM BRIDGE'") == 0) {
        printf("SUCCESS: Bridge works!\n");
    } else {
        printf("FAILURE: Unexpected returned code\n");
        return 1;
    }

    if (response) rxvml_value_free(response);
    rxvml_value_free(tok_array);
    rxvml_destroy(ctx);
    return 0;
}
