/* PoC Plugin Module - Experimental. To be removed or migrated. */
#include "rxcp_poc_plug.h"
#include "rxcp_ast.h"
#include "rxcp_types.h"

#include <stdio.h>

PluginStatus plugin_poc_math(PluginContext* pctx, ASTNode* node) {
    if (!node || !node->child) return PLUGIN_CONTINUE;

    ASTNode* arg = node->child;

    /* Safety: If pctx->iteration > 12, default to TP_FLOAT to prevent infinite loops */
    if (pctx->iteration > 12) {
        if (node->value_type == TP_UNKNOWN) {
            node->value_type = TP_FLOAT;
            return PLUGIN_DIRTY;
        }
        return PLUGIN_OK;
    }

    /* Recursion Support: If the child is a Function Call, checking its type might be premature. */
    if (arg->node_type == FUNCTION) {
        if (arg->value_type == TP_UNKNOWN) {
            return PLUGIN_NEED_MORE;
        }
    }

    /* Type Logic */
    if (arg->value_type == TP_INTEGER) {
        if (node->value_type == TP_INTEGER) return PLUGIN_OK;
        node->value_type = TP_INTEGER;
        return PLUGIN_DIRTY;
    }

    if (arg->value_type == TP_FLOAT || arg->value_type == TP_DECIMAL) {
        if (node->value_type == TP_FLOAT) return PLUGIN_OK;
        node->value_type = TP_FLOAT;
        return PLUGIN_DIRTY;
    }

    if (arg->value_type == TP_UNKNOWN) {
        return PLUGIN_NEED_MORE;
    }

    return PLUGIN_CONTINUE;
}
