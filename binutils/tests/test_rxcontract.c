/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#include "rxcontract.h"
#include "rxsignature.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "contract conformance failed at %d: %s\n", __LINE__, #x); return 1; } } while (0)
int main(void) {
    rx_contract_type t;
    rx_contract_callable c;
    rx_contract_attribute a;
    rx_contract_class_header h;
    rx_contract_error error;
    rx_callable_signature signature;
    const char *valid[] = { ".int", ".binary", ".ns..item", ".int[]", ".int[0]",
        ".string[-2 to 3, *]", ".int.2.", "reference .ns..item[]", "REFERENCE .int" };
    const char *bad[] = { "", ".", ".int trailing", ".int[", ".int[1,", ".void[]",
        "reference .void", ".int[1 + 2]", ".int; say 1", "say .int", ".int[foo]" };
    const char *bad_args[] = { "value=.", "value=.void", ",value=.int", "value=.int,",
        "value=.int,,other=.int", "...=.int,other=.int", "?...=.int", "value=.int; say 1",
        "value=1+2", "value='unterminated", "value=.int\nsay 1" };
    size_t i;
    char exact[] = { '.', 'i', 'n', 't', 0, 'x' };
    for (i = 0; i < sizeof(valid)/sizeof(*valid); ++i) {
        CHECK(rx_contract_parse_type(valid[i], strlen(valid[i]), &t, &error));
        rx_contract_type_free(&t);
    }
    CHECK(rx_contract_parse_type(valid[5], strlen(valid[5]), &t, &error));
    CHECK(t.dimensions == 2 && !strcmp(t.ranges[0].lower, "-2") && !strcmp(t.ranges[0].upper, "3"));
    CHECK(t.ranges[1].lower == NULL && t.ranges[1].upper == NULL);
    rx_contract_type_free(&t);
    for (i = 0; i < sizeof(bad)/sizeof(*bad); ++i) {
        CHECK(!rx_contract_parse_type(bad[i], strlen(bad[i]), &t, &error));
        CHECK(error.field && error.message && t.name == NULL);
        rx_contract_type_free(&t);
    }
    CHECK(!rx_contract_parse_type(exact, sizeof(exact), &t, &error));
    rx_contract_type_free(&t);
    CHECK(rx_contract_parse_callable(RX_CONTRACT_PROCEDURE, "process", ".ns..item",
        "items=.ns..item[-2 to 3, *],expose counter=.int,?mode=.string,...=.string", 1, &c, &error));
    CHECK(c.task && c.arg_count == 4 && c.args[0].type.dimensions == 2);
    CHECK(c.args[1].by_reference && c.args[2].optional && c.args[3].vararg);
    rx_contract_callable_free(&c);
    CHECK(rx_contract_parse_callable(RX_CONTRACT_PROCEDURE, "pos", ".int",
        "needle=.string,haystack=.string,start=1", 0, &c, &error));
    CHECK(c.args[2].optional && c.args[2].default_kind == RX_CONTRACT_INTEGER_DEFAULT);
    CHECK(!strcmp(c.args[2].default_literal, "1")); rx_contract_callable_free(&c);
    CHECK(rx_contract_parse_callable(RX_CONTRACT_METHOD, "zero", NULL, " \t", 0, &c, &error));
    CHECK(c.arg_count == 0 && !strcmp(c.result.name, ".void")); rx_contract_callable_free(&c);
    CHECK(rx_contract_parse_callable(RX_CONTRACT_FACTORY, "*", ".void", "v=.string", 0, &c, &error));
    rx_contract_callable_free(&c);
    for (i = 0; i < sizeof(bad_args)/sizeof(*bad_args); ++i) {
        CHECK(!rx_contract_parse_callable(RX_CONTRACT_PROCEDURE, "process", ".int", bad_args[i], 0, &c, &error));
        rx_contract_callable_free(&c);
    }
    CHECK(rx_contract_parse_attribute("enabled", ".boolean", 0, "flags.31", &a, &error));
    CHECK(a.register_index == 0 && !strcmp(a.view, "flags.31")); rx_contract_attribute_free(&a);
    CHECK(!rx_contract_parse_attribute("bad", ".void", 0, "int", &a, &error)); rx_contract_attribute_free(&a);
    CHECK(rx_contract_parse_class_header("provider", "item", 0, &h, &error));
    CHECK(!h.is_interface && !strcmp(h.namespace_name, "provider")); rx_contract_class_header_free(&h);
    CHECK(rx_contract_parse_class_header("provider", "contract", 1, &h, &error));
    CHECK(h.is_interface); rx_contract_class_header_free(&h);
    CHECK(!rx_sig_parse_descriptor("rxsig2|name|.int|", &signature)); rx_sig_free(&signature);
    puts("Independent metadata contract grammar, ownership and rejection boundary pass");
    return 0;
}
