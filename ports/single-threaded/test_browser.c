/* cREXX License (MIT). Static Classic providers and embedded source entry. */
#include "rxvml.h"
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #c); } } while (0)

static int field_is(const char *wire, const char *key, const char *expected) {
    char needle[64];
    const char *start, *end;
    int count = snprintf(needle, sizeof(needle), "\n%s=", key);
    if (count < 0 || (size_t)count >= sizeof(needle)) return 0;
    start = strstr(wire, needle);
    if (!start) return 0;
    start += count;
    end = strchr(start, '\n');
    return end && (size_t)(end - start) == strlen(expected) &&
        !memcmp(start, expected, (size_t)(end - start));
}

static rxvml_value *run_source(rxvml_context *ctx, const char *source,
        const char *name, rxvml_value *arguments) {
    rxvml_value *args[3] = {NULL, NULL, arguments}, *result = NULL;
    const char *error = NULL;
    int status;
    args[0] = rxvml_value_new(ctx);
    args[1] = rxvml_value_new(ctx);
    CHECK(args[0] && args[1]);
    if (!args[0] || !args[1]) goto done;
    status = rxvml_set_str(args[0], source, strlen(source));
    CHECK(status == 0);
    if (status) goto done;
    status = rxvml_set_str(args[1], name, strlen(name));
    CHECK(status == 0);
    if (status) goto done;
    status = rxvml_call_procedure_descriptor(ctx,
        "rxsig1|llcbrowser.run|.string|source=.string,filename=.string,arguments=.string[*]",
        3, args, &result);
    if (status) {
        (void)rxvml_last_error(ctx, &error);
        fprintf(stderr, "Browser entry failed: %s\n", error ? error : "unknown");
    }
    CHECK(status == 0 && result);
done:
    rxvml_value_free(args[0]);
    rxvml_value_free(args[1]);
    return result;
}

static const char *result_wire(rxvml_context *ctx, rxvml_value *result) {
    const char *wire = NULL;
    size_t length = 0;
    CHECK(result != NULL);
    if (!result) return "";
    CHECK(rxvml_to_str(ctx, result, &wire, &length) == 0);
    if (!wire) return "";
    CHECK(length >= 5 && !memcmp(wire, "LLB1\n", 5));
    CHECK(memchr(wire, 0, length) == NULL);
    return wire;
}

int main(int argc, char **argv) {
    static const char argument[] = {'A', 0, (char)0xCE, (char)0xBB};
    static const char source[] = "/* \xCE\xBB */ parse source platform mode resource; "
        "return resource || '|' || arg() || '|' || arg(1,'E') || '|' || arg(1) || '|' || arg(2)";
    rxvml_context *ctx;
    rxvml_value *arguments = NULL, *item = NULL, *result = NULL;
    const char *wire;
    int status;
    if (argc != 2) return 2;
    ctx = rxvml_create(NULL, 0); /* Static providers only; no search directory. */
    CHECK(ctx != NULL);
    if (!ctx) return 1;
    status = rxvml_load_module_file(ctx, argv[1]);
    CHECK(status > 0);
    if (status <= 0) goto done;
    arguments = rxvml_array_new(ctx, 2);
    item = rxvml_value_new(ctx);
    CHECK(arguments && item);
    if (!arguments || !item) goto done;
    status = rxvml_set_str(item, "", 0);
    CHECK(status == 0);
    if (status) goto done;
    CHECK(rxvml_array_set(ctx, arguments, 1, item) == 0);
    status = rxvml_set_str(item, argument, sizeof(argument));
    CHECK(status == 0);
    if (status) goto done;
    CHECK(rxvml_array_set(ctx, arguments, 2, item) == 0);

    result = run_source(ctx, source, "browser-\xCE\xBB.rexx", arguments);
    wire = result_wire(ctx, result);
    CHECK(field_is(wire, "indicator", "R"));
    CHECK(field_is(wire, "hasStatus", "1") && field_is(wire, "status", "0"));
    CHECK(field_is(wire, "hasValue", "1"));
    /* browser-lambda.rexx|2|1||A, NUL, lambda, in exact UTF-8 bytes. */
    CHECK(field_is(wire, "valueHex", "62726F777365722DCEBB2E726578787C327C317C7C4100CEBB"));
    CHECK(field_is(wire, "hasPrimary", "0") && field_is(wire, "hasCleanup", "0"));
    CHECK(field_is(wire, "truncated", "0"));
    rxvml_value_free(result); result = NULL;

    result = run_source(ctx, "if then", "syntax.rexx", arguments);
    wire = result_wire(ctx, result);
    CHECK(field_is(wire, "indicator", "E"));
    CHECK(field_is(wire, "hasStatus", "0") && field_is(wire, "status", "0"));
    CHECK(field_is(wire, "hasValue", "0"));
    CHECK(field_is(wire, "hasPrimary", "1"));
    CHECK(field_is(wire, "primaryNameHex", "434C41535349435F53594E544158"));
    CHECK(strstr(wire, "\nprimaryMessageHex=") && !field_is(wire, "primaryMessageHex", ""));
    CHECK(field_is(wire, "hasCleanup", "0") && field_is(wire, "truncated", "0"));
done:
    rxvml_value_free(result);
    rxvml_value_free(item);
    rxvml_value_free(arguments);
    rxvml_destroy(ctx);
    if (!failures) puts("BROWSER EMBEDDING PASS");
    return failures ? 1 : 0;
}
