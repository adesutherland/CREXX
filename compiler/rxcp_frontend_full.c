/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#include <stdio.h>
#include "rxcpmain.h"
#include "rxcp_frontend.h"
#include "rxcp_task_lower.h"

#include "rxcp_exit_typed.h"
void rxcp_inject_concurrency_imports(Context *);
walker_result rxcp_task_target_lowering_walker(walker_direction, ASTNode *, void *);
#ifdef ENABLE_PARSER_MODE
int rxc_parser_mode_main(int, int, const char *, int);
#define PARSER_SERVICE rxc_parser_mode_main
#else
#define PARSER_SERVICE NULL
#endif

static int full_parse(Context *c) {
    if (c->level == LEVELC) {
        typed_rxcp_init_exits(c);
        return rxcp_parse_classic_frontend(c);
    }
    if (c->level != LEVELB && c->level != LEVELG && c->level != LEVELL) {
        fprintf(stderr, "REXX Level A/D (cREXX Classic) - Not supported yet\n");
        return 1;
    }
    typed_rxcp_init_exits(c);
    rexbpars(c);
    if (c->debug_mode && c == c->master_context) {
        rxcp_debug_header("STAGE_RAW", -1);
        rxcp_print_ast_recursive(c->ast, 0);
    }
    return 0;
}

const RxcpFrontend rxcp_full_frontend = {
    full_parse, PARSER_SERVICE, NULL,
    typed_rxcp_init_exits, typed_rxcp_free_exits,
    typed_rxcp_is_exit_primary, typed_rxcp_is_exit_additional,
    typed_rxcp_get_exit_flags, typed_rxcp_exit_bridge_invoke,
    typed_rxcp_exit_bridge_plan_invoke, typed_rxcp_exit_rewrite_diagnostics,
    rxcp_inject_concurrency_imports, rxcp_task_target_lowering_walker,
    rxcp_validate_task_scope_reuse, rxcp_task_calls_walker
};

void rxcp_use_full_frontend(Context *c) { c->frontend = &rxcp_full_frontend; }
int rxcmain(int argc, char **argv) {
    return rxcp_driver_main(argc, argv, &rxcp_full_frontend);
}
