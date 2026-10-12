/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#include <stdlib.h>
#include <string.h>
#include "rxcpmain.h"
#include "rxcc.h"
#include "rxcp_frontend.h"
#include "rxcp_levelc_lower.h"
#include "rxcp_source_tree.h"
#include "rxcp_srcmap.h"

struct RxccUnit { Context *context; char *name, *location, *failure; int status; };
int rxcp_classic_parse_map(Context *, const char **);

RxccUnit *rxcc_parse(const RxccRequest *request) {
    RxccUnit *unit;
    Context *c;
    char *buffer;
    const char *reason = NULL;
    if (!request || (!request->source && request->length) || request->length > (size_t)-1 - 2)
        return NULL;
    unit = calloc(1, sizeof(*unit));
    buffer = calloc(request->length + 2, 1);
    if (!unit || !buffer) { free(unit); free(buffer); return NULL; }
    if (request->length) memcpy(buffer, request->source, request->length);
    c = cntx_f();
    unit->context = c;
    unit->name = strdup(request->name ? request->name : "buffer.rexx");
    unit->location = request->location ? strdup(request->location) : NULL;
    c->file_name = unit->name;
    c->location = unit->location;
    c->frontend = &rxcp_classic_frontend;
    c->master_context = c;
    c->cli_default_level = LEVELC;
    c->levelc_routine_mode = (char)request->routine_mode;
    c->optimise = !request->noopt;
    if (request->binary_imports) {
        c->import_locations = rxcp_split_location_list((char *)request->binary_imports);
        c->import_locations_owns_buffer = 1;
    }
    if (request->source_imports) {
        c->source_import_locations = rxcp_split_location_list((char *)request->source_imports);
        c->source_import_locations_owns_buffer = 1;
    }
    cntx_buf(c, buffer, request->length);
    opt_pars(c);
    free_ast(c);
    free_tok(c);
    cntx_buf(c, buffer, request->length);
    if (c->source_has_srcmap) {
        char *mapped = NULL;
        size_t length = 0;
        if (rxcp_srcmap_preprocess(c, &mapped, &length)) {
            unit->status = 1;
            return unit;
        }
        free(buffer);
        cntx_buf(c, mapped, length);
    }
    unit->status = rxcp_classic_parse_map(c, &reason);
    if (reason) unit->failure = strdup(reason);
    return unit;
}

int rxcc_status(const RxccUnit *unit) { return unit ? unit->status : 1; }
const char *rxcc_failure(const RxccUnit *unit) { return unit ? unit->failure : NULL; }
Context *rxcc_context(RxccUnit *unit) { return unit ? unit->context : NULL; }
void rxcc_destroy(RxccUnit *unit) {
    if (!unit) return;
    fre_cntx(unit->context);
    free(unit->name); free(unit->location); free(unit->failure); free(unit);
}
