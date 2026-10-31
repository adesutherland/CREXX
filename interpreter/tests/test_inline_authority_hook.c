/* cREXX License (MIT). Private context routing and native result aliasing.
 * This callback is an independent dispatch oracle, not a selector resolver. */
#include "rxvml_inline_private.h"
#include <stdio.h>
#include <string.h>

static int failures, live_payloads;
static void copy_payload(void *destination, void *source);
static void finalize_payload(void *item);
static const rxvm_native_payload_ops payload_ops = {
    "inline-hook-test.payload", copy_payload, finalize_payload
};
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #c); ++failures; } } while (0)

static int marked(rxvml_value *item) {
    const rxvm_native_payload_ops *ops = NULL;
    size_t length = 0u;
    unsigned int marker = 0u;
    void *data = rxvml_get_native_payload(item, &length, &ops, NULL);
    if (!data || ops != &payload_ops || length != sizeof(marker)) return 0;
    memcpy(&marker, data, sizeof(marker));
    return marker == 0x5142u;
}
static int mark(rxvml_value *item) {
    unsigned int marker = 0x5142u;
    if (rxvml_set_native_payload(item, &marker, sizeof(marker), &payload_ops, 0)) return 0;
    ++live_payloads;
    return 1;
}
static void copy_payload(void *destination, void *source) {
    CHECK(marked(source));
    CHECK(mark(destination));
}
static void finalize_payload(void *item) {
    CHECK(marked(item));
    --live_payloads;
}
typedef struct hook_state { int calls, outcome; } hook_state;
static int hook(rxvml_context *ctx, void *userdata,
        enum rxvml_inline_operation operation, size_t argc,
        rxvml_value *const *arguments, rxvml_value *result) {
    hook_state *state = userdata;
    rxinteger value = 0;
    size_t base = operation == RXVML_INLINE_CHECK ? 1u : 0u;
    ++state->calls;
    CHECK(argc == base + 5u);
    CHECK(marked(arguments[0]) && marked(arguments[base]));
    CHECK(!rxvml_to_int(ctx, arguments[base + 1u], &value) && value == 17);
    CHECK(marked(arguments[base + 2u]));
    CHECK(!rxvml_to_int(ctx, arguments[base + 3u], &value) && value == 23);
    CHECK(!rxvml_to_int(ctx, arguments[base + 4u], &value) && value == 0);
    if (operation == RXVML_INLINE_TOKEN) CHECK(mark(result));
    else rxvml_set_int(result, 37);
    return state->outcome;
}
static void invoke(rxvml_context *ctx, rxvml_value *argument,
        const char *descriptor, int token, int expected) {
    rxvml_value *result = NULL;
    rxinteger number = -1;
    int status = rxvml_call_procedure_descriptor(ctx, descriptor, 1u, &argument, &result);
    if (status) {
        const char *message = NULL;
        rxvml_last_error(ctx, &message);
        fprintf(stderr, "hook invoke: %s\n", message ? message : "unavailable");
    }
    CHECK(!status && result);
    if (result) {
        if (token) CHECK(marked(result));
        else CHECK(!rxvml_to_int(ctx, result, &number) && number == expected);
    }
    CHECK(marked(argument));
    rxvml_value_free(result);
}
int main(int argc, char **argv) {
    rxvml_context *ctx;
    rxvml_value *argument;
    hook_state state = {0, 1};
    if (argc != 2) return 2;
    ctx = rxvml_create(NULL, 0u);
    CHECK(ctx != NULL);
    if (!ctx) return 1;
    CHECK(rxvml_load_module_file(ctx, argv[1]) > 0);
    argument = rxvml_value_new(ctx);
    CHECK(argument && mark(argument));
    if (argument) {
        invoke(ctx, argument, "rxsig1|inline_hook.checkcap|.int|capability=.object", 0, 0);
        CHECK(state.calls == 0);
        CHECK(rxvml_inline_set_callback(ctx, hook, &state) != 0);
        CHECK(rxvml_inline_seal_context(ctx) == 0);
        CHECK(!rxvml_inline_set_callback(ctx, hook, &state));
        CHECK(rxvml_inline_set_callback(ctx, hook, &state) != 0);
        invoke(ctx, argument, "rxsig1|inline_hook.checkcap|.int|capability=.object", 0, 37);
        invoke(ctx, argument, "rxsig1|inline_hook.checkowner|.int|capability=.object", 0, 37);
        invoke(ctx, argument, "rxsig1|inline_hook.tokenowner|.object|capability=.object", 1, 0);
        CHECK(state.calls == 3);
        state.outcome = -1;
        invoke(ctx, argument, "rxsig1|inline_hook.checkcap|.int|capability=.object", 0, 0);
        CHECK(!rxvml_inline_set_callback(ctx, NULL, NULL));
        invoke(ctx, argument, "rxsig1|inline_hook.checkowner|.int|capability=.object", 0, 0);
        CHECK(state.calls == 4);
        rxvml_value_free(argument);
    }
    rxvml_destroy(ctx);
    CHECK(live_payloads == 0);
    return failures ? 1 : 0;
}
