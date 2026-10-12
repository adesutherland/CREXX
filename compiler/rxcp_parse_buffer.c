/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#include <stdlib.h>
#include <string.h>
#include "rxcpmain.h"
#include "rxcp_frontend.h"
Context *rxcp_parse_buffer(char *source, int debug_mode) {
    Context *c = cntx_f();
    rxcp_use_full_frontend(c);
    size_t n = strlen(source);
    char *buffer = calloc(n + 2, 1);
    memcpy(buffer, source, n);
    cntx_buf(c, buffer, n);
    c->level = LEVELB; c->debug_mode = debug_mode;
    c->master_context = c; c->file_name = "fragment";
    rexbpars(c);
    return c;
}
