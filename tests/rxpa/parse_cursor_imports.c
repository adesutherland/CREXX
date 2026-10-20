/* cREXX License (MIT). Repeated, disk-independent compiler metadata requests. */
#include "rxcpmain.h"
#include "rxcp_frontend.h"
#include "../../interpreter/rxvmparse_bindings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    int request;
    for (request = 0; request < 3; ++request) {
        Context *context = cntx_f();
        size_t i, functions = 0;
        context->master_context = context;
        context->file_name = "core-parse-import.crexx";
        context->level = LEVELB;
        rxcp_use_full_frontend(context);
        /* An explicit empty search list has no incidental provider or file. */
        context->import_locations = calloc(1, sizeof(char *));
        context->importable_file_list = calloc(1, sizeof(importable_file *));
        if (!context->import_locations || !context->importable_file_list ||
            !rxcp_scan_imports(context)) return 1;
        if (!context->core_parse_metadata_loaded) return 2;
        for (i = 0; i < rxvm_parse_cursor_binding_count; ++i) {
            const rxvm_parse_binding *binding = &rxvm_parse_cursor_bindings[i];
            imported_func *function;
            if (binding->kind != RXVM_PARSE_BIND_FUNCTION) continue;
            ++functions;
            function = rxcp_find_imported_function_exact(context, binding->symbol);
            if (!function || function->error_state ||
                strcmp(function->type, binding->type) || strcmp(function->args, binding->args)) {
                fprintf(stderr, "FAIL: request %d: %s\n", request, binding->symbol);
                return 3;
            }
        }
        if (functions != 7 || rxcp_scan_imports(context)) return 4;
        fre_cntx(context);
    }
    puts("PARSE_CURSOR_IMPORTS|requests=3|callables=7|failures=0");
    return 0;
}
