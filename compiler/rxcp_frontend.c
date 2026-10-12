/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#include "rxcp_frontend.h"
#include "rxcp_exit_keywords.h"
#include "platform.h"
#include <string.h>

const RxcpFrontend *rxcp_frontend(Context *c) {
    Context *root = c->master_context ? c->master_context : c;
    return c->frontend ? c->frontend : root->frontend;
}
int rxcp_frontend_parse(Context *c) {
    const RxcpFrontend *f = rxcp_frontend(c);
    return f && f->parse ? f->parse(c) : 1;
}
void rxcp_init_exits(Context *c) {
    const RxcpFrontend *f = rxcp_frontend(c);
    if (f && f->init_exits) f->init_exits(c);
}
void rxcp_free_exits(Context *c) {
    const RxcpFrontend *f = rxcp_frontend(c);
    if (f && f->free_exits) f->free_exits(c);
}
int rxcp_is_exit_primary(Context *c, const char *s, size_t n) {
    const RxcpFrontend *f = rxcp_frontend(c);
    return f && f->exit_primary ? f->exit_primary(c, s, n) : 0;
}
int rxcp_is_exit_additional(Context *c, const char *s, size_t n) {
    const RxcpFrontend *f = rxcp_frontend(c);
    return f && f->exit_additional ? f->exit_additional(c, s, n) : 0;
}
unsigned int rxcp_get_exit_flags(Context *c, const char *s, size_t n) {
    const RxcpFrontend *f = rxcp_frontend(c);
    return f && f->exit_flags ? f->exit_flags(c, s, n) : 0;
}
int rxcp_exit_bridge_invoke(Context *c, ASTNode *node) {
    const RxcpFrontend *f = rxcp_frontend(c);
    return f && f->exit_invoke ? f->exit_invoke(c, node) : 0;
}
int rxcp_exit_bridge_plan_invoke(Context *c, ASTNode *node) {
    const RxcpFrontend *f = rxcp_frontend(c);
    return f && f->exit_plan ? f->exit_plan(c, node) : 0;
}
void rxcp_exit_rewrite_diagnostics(Context *c) {
    const RxcpFrontend *f = rxcp_frontend(c);
    if (f && f->exit_diagnostics) f->exit_diagnostics(c);
}

const char *rxcp_match_certified_exit_primary(const char *keyword, size_t len) {
    int i;

    if (!keyword || len == 0) return NULL;

    for (i = 0; certified_exit_specs[i].primary_keyword; i++) {
        const char *primary = certified_exit_specs[i].primary_keyword;
        if (len == strlen(primary) && strncasecmp(primary, keyword, len) == 0) {
            return primary;
        }
    }
    return NULL;
}
