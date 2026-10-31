/* cREXX License (MIT). Exact private entry ownership and context lifetime. */
#include "rxvml_inline_private.h"
#include <stdio.h>

static int failures;
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #c); ++failures; } } while (0)

static rxvml_inline_procedure *resolve(rxvml_context *ctx, size_t first,
        size_t last, const char *member, const char *descriptor, int method) {
    rxvml_inline_procedure *pin = NULL;
    int status = rxvml_inline_resolve_owned(ctx, first, last, member,
            descriptor, method, &pin);
    if (status) {
        const char *message = NULL;
        rxvml_last_error(ctx, &message);
        fprintf(stderr, "owned resolve %s: %d %s\n", descriptor, status,
                message ? message : "unavailable");
    }
    CHECK(status == 0 && pin);
    return pin;
}

static void expect_int(rxvml_context *ctx, rxvml_inline_procedure *pin,
        rxvml_value *receiver, size_t argc, rxvml_value **arguments,
        rxinteger expected, const char *label) {
    rxvml_value *result = NULL;
    rxinteger number = 0;
    int status = rxvml_inline_call_owned(ctx, pin, receiver, argc, arguments, &result);
    int matches = result && !rxvml_to_int(ctx, result, &number) && number == expected;
    if (status || !matches) {
        const char *message = NULL;
        rxvml_last_error(ctx, &message);
        fprintf(stderr, "owned call %s: status=%d result=%s value=%lld expected=%lld: %s\n",
                label, status, result ? "present" : "absent", (long long)number,
                (long long)expected, message ? message : "unavailable");
    }
    CHECK(status == 0);
    CHECK(matches);
    rxvml_value_free(result);
}

int main(int argc, char **argv) {
    rxvml_context *ctx, *other, *bad, *replacement;
    rxvml_inline_procedure *echo = NULL, *method = NULL, *imported = NULL, *absent = NULL;
    rxvml_value *argument = NULL, *receiver = NULL, *field = NULL, *result = NULL;
    size_t shadow_first, shadow_last, first, last;
    int loaded;
    if (argc != 5) return 2;
    ctx = rxvml_create(NULL, 0u); other = rxvml_create(NULL, 0u);
    CHECK(ctx && other);
    if (!ctx || !other) {
        rxvml_destroy(ctx); rxvml_destroy(other); return 1;
    }
    shadow_first = rxvml_inline_module_count(ctx) + 1u;
    loaded = rxvml_load_module_file(ctx, argv[1]); CHECK(loaded > 0);
    shadow_last = (size_t)loaded;
    first = rxvml_inline_module_count(ctx) + 1u;
    loaded = rxvml_load_module_file(ctx, argv[2]); CHECK(loaded > 0);
    last = (size_t)loaded;
    CHECK(last > first); /* One load contains the owner and its import member. */
    CHECK(rxvml_inline_seal_context(ctx) == 0);
    echo = resolve(ctx, first, last, "rxvml_inline_owner",
            "rxsig1|authority.echo|.int|x=.int", 0);
    method = resolve(ctx, first, last, "rxvml_inline_owner.rxas",
            "rxsig1|authority.box.add|.int|x=.int", 1);
    imported = resolve(ctx, first, last, "rxvml_inline_import",
            "rxsig1|authority.imported|.int|", 0);
    CHECK(rxvml_inline_resolve_owned(ctx, shadow_first, shadow_last,
            "rxvml_inline_owner", "rxsig1|authority.echo|.int|x=.int", 0, &absent) == 1 && !absent);
    rxvml_inline_procedure_free(absent); absent = NULL;
    CHECK(rxvml_inline_resolve_owned(ctx, first, last,
            "rxvml_inline_owner", "rxsig1|authority.absent|.int|", 0, &absent) == 1 && !absent);
    rxvml_inline_procedure_free(absent); absent = NULL;
    CHECK(rxvml_inline_resolve_owned(ctx, first, last,
            "rxvml_inline_owner", "rxsig1|authority.echo|.float|x=.int", 0, &absent) == -1 && !absent);
    rxvml_inline_procedure_free(absent); absent = NULL;
    CHECK(rxvml_inline_resolve_owned(ctx, first, last,
            "rxvml_inline_import", "rxsig1|authority.helper|.int|", 0, &absent) == -1 && !absent);
    rxvml_inline_procedure_free(absent); absent = NULL;
    argument = rxvml_value_new(ctx); receiver = rxvml_array_new(ctx, 1u);
    field = rxvml_value_new(ctx); CHECK(argument && receiver && field);
    if (argument && receiver && field && echo && method && imported) {
        rxvml_set_int(argument, 37);
        expect_int(ctx, echo, NULL, 1u, &argument, 37, "direct private echo");
        rxvml_set_int(argument, 5); rxvml_set_int(field, 8);
        CHECK(rxvml_array_set(ctx, receiver, 1u, field) == 0);
        expect_int(ctx, method, receiver, 1u, &argument, 13, "receiver method add");
        expect_int(ctx, imported, NULL, 0u, NULL, 43, "owned imported helper");
        CHECK(rxvml_inline_call_owned(ctx, method, NULL, 1u, &argument, &result) == -1 && !result);
        CHECK(rxvml_inline_call_owned(ctx, echo, receiver, 1u, &argument, &result) == -1 && !result);
        CHECK(rxvml_inline_call_owned(other, echo, NULL, 1u, &argument, &result) == -1 && !result);
        CHECK(rxvml_load_module_file(ctx, argv[3]) > 0);
        CHECK(rxvml_inline_call_owned(ctx, echo, NULL, 1u, &argument, &result) == -1 && !result);
        CHECK(rxvml_inline_seal_context(ctx) == -1);
    }
    rxvml_value_free(field); rxvml_value_free(receiver); rxvml_value_free(argument);
    rxvml_destroy(ctx);
    replacement = rxvml_create(NULL, 0u); CHECK(replacement);
    CHECK(rxvml_inline_call_owned(replacement, echo, NULL, 0u, NULL, &result) == -1 && !result);
    CHECK(rxvml_inline_call_owned(other, echo, NULL, 0u, NULL, &result) == -1 && !result);
    rxvml_inline_procedure_free(echo); rxvml_inline_procedure_free(method);
    rxvml_inline_procedure_free(imported); rxvml_destroy(other); rxvml_destroy(replacement);
    bad = rxvml_create(NULL, 0u); CHECK(bad);
    if (bad) {
        CHECK(rxvml_load_module_file(bad, argv[2]) > 0);
        CHECK(rxvml_load_module_file(bad, argv[4]) > 0);
        CHECK(rxvml_inline_seal_context(bad) == -1);
        rxvml_destroy(bad);
    }
    printf("PRIVATE_INLINE_OWNED|failures=%d\n", failures);
    return failures ? 1 : 0;
}
