/* cREXX License (MIT), Copyright (c) 2026 the cREXX contributors. */
#include "rxvml.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static int checks, failures;
#define CHECK(x) do { ++checks; if (!(x)) { ++failures; fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); } } while (0)
int main(int argc, char **argv) {
    unsigned round;
    if (argc != 2) return 2;
    for (round = 0; round < 8; ++round) {
        rxvml_context *ctx = rxvml_create(NULL, 0);
        rxvml_value *arg, *result = NULL, *array, *copy;
        rxinteger number = 0;
        const char *error = NULL;
        rxvml_address_request request;
        unsigned char legacy = 0;
        CHECK(ctx != NULL);
        if (!ctx) return 1;
        CHECK(rxvml_load_module_file(ctx, argv[1]) > 0);
        arg = rxvml_value_new(ctx);
        CHECK(arg != NULL);
        if (!arg) return 1;
        rxvml_set_int(arg, INT64_C(4294967317));
        CHECK(rxvml_call_procedure_descriptor(ctx,
            "rxsig1|labembed.twice|.int|input=.int", 1, &arg, &result) == 0);
        CHECK(result != NULL);
        CHECK(result && rxvml_to_int(ctx, result, &number) == 0 && number == INT64_C(8589934634));
        rxvml_value_free(result); result = NULL;
        CHECK(rxvml_call_procedure_descriptor(ctx,
            "rxsig1|labembed.absent|.int|input=.int", 1, &arg, &result) != 0);
        CHECK(result == NULL);
        CHECK(rxvml_call_procedure_descriptor(ctx,
            "rxsig1|labembed.twice|.string|input=.int", 1, &arg, &result) != 0);
        CHECK(result == NULL);
        array = rxvml_array_new(ctx, 2);
        CHECK(array != NULL);
        CHECK(rxvml_array_set(ctx, array, 1, arg) == 0);
        copy = rxvml_get_attribute(ctx, array, 1);
        CHECK(copy && rxvml_to_int(ctx, copy, &number) == 0 && number == INT64_C(4294967317));
        rxvml_value_free(copy);
        CHECK(rxvml_array_set(ctx, array, 3, arg) != 0);
        /* A legacy opaque redirect must fail before interpreting its bytes. */
        memset(&request, 0, sizeof(request));
        arg->binary_value = &legacy; arg->binary_length = 1;
        request.stdout_endpoint = arg; request.stderr_endpoint = arg;
        CHECK(rxvml_address_emit_output(ctx, &request, "must not print") == -1);
        CHECK(rxvml_address_emit_error(ctx, &request, "must not print") == -1);
        (void)rxvml_last_error(ctx, &error);
        CHECK(error && strstr(error, "not supported") != NULL);
        arg->binary_value = NULL; arg->binary_length = 0;
        rxvml_value_free(array); rxvml_value_free(arg);
        rxvml_destroy(ctx);
    }
    printf("Embedding: %d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
