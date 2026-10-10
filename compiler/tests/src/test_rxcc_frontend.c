/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#include <stdio.h>
#include <string.h>
#include "rxcc.h"
#include "rxcp_ctx.h"
#include "rxcpmain.h"
#include "rxcp_val.h"
#include "rxcp_levelc_lower.h"
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "check failed: %s:%d: %s\n", __FILE__, __LINE__, #x); return 1; } } while (0)
int main(void) {
    char source[] = "say 'a\0b'\nx=1\nif x=1 then do\n say x\nend\n";
    const char *reason = NULL;
    RxccRequest request;
    RxccUnit *first, *second;
    Context *c;
    int i;
    memset(&request, 0, sizeof(request));
    request.source = source; request.length = sizeof(source) - 1;
    request.name = "exact-source.rexx";
    first = rxcc_parse(&request);
    CHECK(first && rxcc_status(first) == 0);
    c = rxcc_context(first);
    CHECK(c && c->buff_end - c->buff_start == (ptrdiff_t)request.length);
    CHECK(c->buff_start[6] == 0 && c->buff_start[7] == 'b');
    CHECK(rxcp_levelc_verify_lowered_tree(c->ast, &reason));
    source[0] = 'X';
    CHECK(c->buff_start[0] == 's');
    for (i = 0; i < 20; ++i) {
        request.source = "if 1 then"; request.length = strlen(request.source);
        second = rxcc_parse(&request);
        CHECK(second && rxcc_status(second) != 0);
        CHECK(rxcc_context(second)->source_diagnostics_list != NULL ||
              rxcc_context(second)->diagnostics_list != NULL);
        rxcc_destroy(second);
        request.source = "say 'ok'"; request.length = strlen(request.source);
        second = rxcc_parse(&request);
        CHECK(second && rxcc_status(second) == 0);
        CHECK(rxcc_context(second)->master_context != c->master_context);
        rxcc_destroy(second);
    }
    request.source = "interpret 'say 1'"; request.length = strlen(request.source);
    second = rxcc_parse(&request);
    CHECK(second && rxcc_status(second) != 0 && rxcc_failure(second));
    rxcc_destroy(second);
    request.source = "call missing_entry"; request.length = strlen(request.source);
    request.binary_imports = "/definitely-missing-crexx-frontend-import-root";
    second = rxcc_parse(&request);
    CHECK(second && rxcc_status(second) == 0);
    rxcp_val(rxcc_context(second));
    CHECK(prnterrs(rxcc_context(second)) > 0);
    rxcc_destroy(second);
    rxcc_destroy(first);
    rxcc_destroy(NULL);
    puts("Classic frontend length, failure recovery and request ownership pass");
    return 0;
}
