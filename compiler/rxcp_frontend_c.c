/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#include <stdio.h>
#include "rxcpmain.h"
#include "rxcp_frontend.h"
#include "rxcp_levelc_lower.h"
#include "rxcp_source_tree.h"

int rxcp_classic_source_import(Context *, const char *, const char *, RexxLevel);
int rxcp_classic_parser_mode(int, int, const char *, int);

static int has_errors(ASTNode *node) {
    for (; node; node = node->sibling) {
        if (node->node_type == ERROR || has_errors(node->child)) return 1;
    }
    return 0;
}

static int has_source_errors(Context *c) {
    SourceDiagnostic *d;
    for (d = c->source_diagnostics_list; d; d = d->next_in_context)
        if (d->severity == SOURCE_DIAG_ERROR) return 1;
    return 0;
}

int rxcp_classic_parse_map(Context *c, const char **reason) {
    if (c->level != LEVELC) {
        *reason = "rxc-c requires Level C source (use rxc for typed source)";
        return 1;
    }
    rexcpars(c);
    if (c->debug_mode && c == c->master_context) {
        rxcp_debug_header("STAGE_RAW", -1);
        rxcp_print_ast_recursive(c->ast, 0);
    }
    if (!c->ast) return 1;
    rxcp_levelc_prepare_source_ast(c);
    source_tree_sync_diagnostics(c);
    if (rxcp_levelc_validate_recovered_syntax(c)) source_tree_sync_diagnostics(c);
    if (has_errors(c->ast) || has_source_errors(c)) return 1;
    if (!rxcp_levelc_lower_to_canonical(c, reason)) return 1;
    source_tree_sync_diagnostics(c);
    if (has_errors(c->ast) || has_source_errors(c)) return 1;
    if (c->debug_mode && c == c->master_context) {
        rxcp_debug_header("STAGE_LEVELC_LOWERED", -1);
        rxcp_print_ast_recursive(c->ast, 0);
    }
    return 0;
}

int rxcp_parse_classic_frontend(Context *c) {
    const char *reason = NULL;
    int status = rxcp_classic_parse_map(c, &reason);
    if (status) {
        if (reason) fprintf(stderr, "%s: %s\n", rxcp_levelc_compile_unsupported_message(), reason);
        else {
            int errors = prnterrs(c);
            if (errors) fprintf(stderr, "%d error(s) in source file\n", errors);
        }
    }
    return status;
}

const RxcpFrontend rxcp_classic_frontend = {
    rxcp_parse_classic_frontend, rxcp_classic_parser_mode, rxcp_classic_source_import,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
