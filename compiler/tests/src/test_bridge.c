#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "rxvml.h"

#ifndef CREXX_TEST_RXCEXITS_MODULE
#define CREXX_TEST_RXCEXITS_MODULE "rxcexits"
#endif

int main(int argc, char** argv) {
    rxvml_context* ctx;
    rxvml_value* tok_array;
    rxvml_value* tok_obj = NULL;
    rxvml_value* exit_arg = NULL;
    rxvml_value* exit_obj = NULL;
    rxvml_value* response = NULL;
    rxvml_value* bad_response = NULL;
    rxvml_value* args[9];
    rxvml_value* call_args[1];
    const char* code;
    const char* err;
    int i;

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
    
    for (i=0; i<9; i++) args[i] = rxvml_value_new(ctx);
    rxvml_set_int(args[0], 100); /* DUMMY_TYPE */
    rxvml_set_int(args[1], 0);
    rxvml_set_str(args[2], "PARSE", 5);
    rxvml_set_int(args[3], 10);
    rxvml_set_int(args[4], 1);
    rxvml_set_int(args[5], 5);
    rxvml_set_str(args[6], "test.crexx", 9);
    rxvml_set_int(args[7], 0);
    rxvml_set_int(args[8], 0);

    if (rxvml_call_factory_descriptor(
            ctx,
            "stub.token",
            "rxsig1|\xc2\xa7""factory|.string|t=.int,st=.int,txt=.string,l=.int,c=.int,len=.int,f=.string,nt=.int,vt=.int",
            9,
            args,
            &bad_response) == 0) {
        if (bad_response) rxvml_value_free(bad_response);
        fprintf(stderr, "Factory descriptor mismatch was accepted\n");
        return 1;
    }
    if (bad_response) {
        rxvml_value_free(bad_response);
        bad_response = NULL;
    }
    
    if (rxvml_call_factory_descriptor(
            ctx,
            "stub.token",
            "rxsig1|\xc2\xa7""factory|.stub..token|t=.int,st=.int,txt=.string,l=.int,c=.int,len=.int,f=.string,nt=.int,vt=.int",
            9,
            args,
            &tok_obj) != 0 || !tok_obj) {
        fprintf(stderr, "Failed to call factory for stub.token\n");
        return 1;
    }
    for (i=0; i<9; i++) rxvml_value_free(args[i]);

    rxvml_array_set(ctx, tok_array, 1, tok_obj);
    rxvml_value_free(tok_obj);

    /* 4. Call the plugin (procedure) */
    printf("Calling stub.stub_plugin...\n");
    call_args[0] = tok_array;

    if (rxvml_call_procedure_descriptor(
            ctx,
            "rxsig1|stub.stub_plugin|.int|tokens=.token[*]",
            1,
            call_args,
            &bad_response) == 0) {
        if (bad_response) rxvml_value_free(bad_response);
        fprintf(stderr, "Procedure descriptor mismatch was accepted\n");
        return 1;
    }
    if (bad_response) {
        rxvml_value_free(bad_response);
        bad_response = NULL;
    }

    if (rxvml_call_procedure_descriptor(
            ctx,
            "rxsig1|stub.stub_plugin|.string|tokens=.token[*]",
            1,
            call_args,
            &response) != 0) {
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

    if (rxvml_load_module_file(ctx, CREXX_TEST_RXCEXITS_MODULE) <= 0) {
        rxvml_last_error(ctx, &err);
        fprintf(stderr, "Failed to load rxcexits: %s\n", err ? err : "unknown error");
        return 1;
    }

    exit_arg = rxvml_value_new(ctx);
    rxvml_set_int(exit_arg, 0);
    if (rxvml_call_factory_descriptor(
            ctx,
            "rxcpexits.addressexit",
            "rxsig1|\xc2\xa7""factory|.rxcpexits..addressexit|nid=.int",
            1,
            &exit_arg,
            &exit_obj) != 0 || !exit_obj) {
        rxvml_last_error(ctx, &err);
        fprintf(stderr, "Failed to call factory for rxcpexits.addressexit: %s\n", err ? err : "unknown error");
        return 1;
    }
    printf("SUCCESS: Compiler exit factory descriptor works!\n");

    if (exit_obj) rxvml_value_free(exit_obj);
    if (exit_arg) rxvml_value_free(exit_arg);
    if (response) rxvml_value_free(response);
    rxvml_value_free(tok_array);
    rxvml_destroy(ctx);
    return 0;
}
