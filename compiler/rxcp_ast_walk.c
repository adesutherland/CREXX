/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */

#include <stdio.h>
#include "rxcpmain.h"

/* And the walker itself
 * It returns
 *     result_normal - All OK - normal processing
 *     result_error - error condition
 */
walker_result ast_wlkr(ASTNode *tree, walker_handler handler, void *payload) {
    ASTNode *node;
    walker_result result;

    result = handler(in, tree, payload);
    if (result == result_abort || result == result_error || result == request_skip) return result;

    node = tree->child;
    while (node) {
        result = ast_wlkr(node, handler, payload);
        if (result == result_abort || result == result_error) return result;
        node = node->sibling;
    }

    return handler(out, tree, payload);
}
