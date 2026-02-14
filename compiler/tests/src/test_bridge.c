#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "rxvml.h"

int main(int argc, char** argv) {
    rxvml_context* ctx;
    rxvml_token_desc d;
    rxvml_value* tok_array;
    rxvml_value* tok_obj;
    rxvml_value* response = NULL;
    const char* code;

    setbuf(stdout, NULL);

    printf("Starting Bridge Test...\n");

    /* 1. Initialize Context */
    ctx = rxvml_create(NULL, 1);
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

    /* 3. Manually construct a token array (Simulating rxcp_marshal_implicit_cmd) */
    tok_array = rxvml_array_new(ctx, 1);
    
    memset(&d, 0, sizeof(d));
    d.type = 100; /* DUMMY_TYPE */
    d.text = "PARSE";
    d.text_len = 5;
    d.line = 10;
    d.column = 1;
    d.file = "test.rexx";
    
    tok_obj = rxvml_make_token(ctx, &d);
    rxvml_array_set(ctx, tok_array, 1, tok_obj);

    /* 4. Call the plugin */
    printf("Calling stub.stub_plugin...\n");
    if (rxvml_call_plugin(ctx, "stub.stub_plugin", tok_array, &response) != 0) {
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
        code = rxvml_get_replacement_code(ctx, response);
        printf("Plugin returned code (object): %s\n", code ? code : "NULL");
    }

    if (code && strcmp(code, "SAY 'HELLO FROM BRIDGE'") == 0) {
        printf("SUCCESS: Bridge works!\n");
    } else {
        printf("FAILURE: Unexpected returned code\n");
        return 1;
    }

    rxvml_destroy(ctx);
    return 0;
}
