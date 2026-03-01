#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxcp_ast.h"
#include "rxcp_sym.h"
#include "rxcp_ctx.h"
#include "rxcpdary.h"

static void validate_node(ASTNode *node, int *errors) {
    ASTNode *child;
    if (!node) return;

    /* Check parent/child consistency */
    child = node->child;
    while (child) {
        if (child->parent != node) {
            fprintf(stderr, "AST Error: Node %d (%s) has child %d (%s) with wrong parent pointer %p (expected %p)\n",
                    node->node_number, ast_ndtp(node->node_type),
                    child->node_number, ast_ndtp(child->node_type),
                    (void*)child->parent, (void*)node);
            (*errors)++;
        }
        validate_node(child, errors);
        child = child->sibling;
    }

    /* Check symbol consistency */
    if (node->symbolNode) {
        if (node->symbolNode->node != node) {
            fprintf(stderr, "AST Error: Node %d (%s) has symbolNode pointing to different node %p\n",
                    node->node_number, ast_ndtp(node->node_type), (void*)node->symbolNode->node);
            (*errors)++;
        }
        if (node->symbolNode->symbol) {
            Symbol *sym = node->symbolNode->symbol;
            int found = 0;
            size_t i;
            for (i = 0; i < sym_nond(sym); i++) {
                if (sym_trnd(sym, i)->node == node) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                fprintf(stderr, "AST Error: Node %d (%s) linked to symbol %s, but symbol doesn't link back to node\n",
                        node->node_number, ast_ndtp(node->node_type), sym->name);
                (*errors)++;
            }
        }
    }

    /* Check scope consistency */
    if (node->node_type == INSTRUCTIONS && node->parent && (node->parent->node_type == INSTRUCTIONS || node->parent->node_type == IF)) {
        if (node->parent->scope && !node->scope) {
            fprintf(stderr, "AST Error: Node %d (%s) under %s missing SCOPE_LOCAL\n",
                    node->node_number, ast_ndtp(node->node_type), ast_ndtp(node->parent->node_type));
            (*errors)++;
        }
    }

    if (node->scope) {
        if (!node->scope->defining_node) {
            fprintf(stderr, "AST Error: Node %d (%s) points to scope %p which has no defining node\n",
                    node->node_number, ast_ndtp(node->node_type), (void*)node->scope);
            (*errors)++;
        }
    }
}

static void validate_scope(Scope *scope, int *errors) {
    size_t i;
    dpa *child_array;
    if (!scope) return;

    /* Check defining node */
    if (scope->defining_node) {
        if (scope->defining_node->scope != scope) {
            fprintf(stderr, "Scope Error: Scope %p (%s) points to defining node %d (%s), but node points to different scope %p\n",
                    (void*)scope, scope->name ? scope->name : "unnamed",
                    scope->defining_node->node_number, ast_ndtp(scope->defining_node->node_type),
                    (void*)scope->defining_node->scope);
            (*errors)++;
        }
    }

    /* Check hierarchy */
    switch (scope->type) {
        case SCOPE_UNIVERSE:
            if (scope->parent) {
                fprintf(stderr, "Scope Hierarchy Error: UNIVERSE scope %p has parent\n", (void*)scope);
                (*errors)++;
            }
            if (scope->defining_node && scope->defining_node->node_type != REXX_UNIVERSE) {
                fprintf(stderr, "Scope Hierarchy Error: UNIVERSE scope %p has wrong defining node type %s\n",
                        (void*)scope, ast_ndtp(scope->defining_node->node_type));
                (*errors)++;
            }
            break;
        case SCOPE_NAMESPACE:
            if (!scope->parent || (scope->parent->type != SCOPE_UNIVERSE && scope->parent->type != SCOPE_NAMESPACE)) {
                fprintf(stderr, "Scope Hierarchy Error: NAMESPACE scope %p has invalid parent type %d\n",
                        (void*)scope, scope->parent ? scope->parent->type : -1);
                (*errors)++;
            }
            break;
        case SCOPE_CLASS:
            if (!scope->parent || scope->parent->type != SCOPE_NAMESPACE) {
                fprintf(stderr, "Scope Hierarchy Error: CLASS scope %p has invalid parent type %d\n",
                        (void*)scope, scope->parent ? scope->parent->type : -1);
                (*errors)++;
            }
            if (scope->defining_node && scope->defining_node->node_type != CLASS_DEF) {
                fprintf(stderr, "Scope Hierarchy Error: CLASS scope %p has wrong defining node type %s\n",
                        (void*)scope, ast_ndtp(scope->defining_node->node_type));
                (*errors)++;
            }
            break;
        case SCOPE_PROCEDURE:
            if (!scope->parent || (scope->parent->type != SCOPE_NAMESPACE && scope->parent->type != SCOPE_CLASS)) {
                fprintf(stderr, "Scope Hierarchy Error: PROCEDURE scope %p has invalid parent type %d\n",
                        (void*)scope, scope->parent ? scope->parent->type : -1);
                (*errors)++;
            }
            if (scope->defining_node && scope->defining_node->node_type != PROCEDURE &&
                scope->defining_node->node_type != METHOD && scope->defining_node->node_type != FACTORY) {
                fprintf(stderr, "Scope Hierarchy Error: PROCEDURE scope %p has wrong defining node type %s\n",
                        (void*)scope, ast_ndtp(scope->defining_node->node_type));
                (*errors)++;
            }
            break;
        case SCOPE_LOCAL:
            if (!scope->parent || (scope->parent->type != SCOPE_PROCEDURE && scope->parent->type != SCOPE_LOCAL)) {
                fprintf(stderr, "Scope Hierarchy Error: LOCAL scope %p has invalid parent type %d\n",
                        (void*)scope, scope->parent ? scope->parent->type : -1);
                (*errors)++;
            }
            break;
    }

    /* Check symbols in this scope */
    Symbol **symbols = scp_syms(scope);
    if (symbols) {
        for (i = 0; symbols[i]; i++) {
            Symbol *sym = symbols[i];
            if (sym->scope != scope) {
                fprintf(stderr, "Scope Error: Symbol %s in scope %p (%s) points to different scope %p\n",
                        sym->name, (void*)scope, scope->name ? scope->name : "unnamed", (void*)sym->scope);
                (*errors)++;
            }
            /* Check SymbolNode links */
            size_t j;
            for (j = 0; j < sym_nond(sym); j++) {
                SymbolNode *sn = sym_trnd(sym, j);
                if (sn->symbol != sym) {
                    fprintf(stderr, "Symbol Error: Symbol %s has SymbolNode %d pointing to different symbol %p\n",
                            sym->name, (int)j, (void*)sn->symbol);
                    (*errors)++;
                }
                if (sn->node->symbolNode != sn) {
                    fprintf(stderr, "Symbol Error: Symbol %s has SymbolNode %d pointing to node %d (%s), but node doesn't point back to this SymbolNode\n",
                            sym->name, (int)j, sn->node->node_number, ast_ndtp(sn->node->node_type));
                    (*errors)++;
                }
            }
        }
        free(symbols);
    }

    /* Check child scopes */
    child_array = (dpa*)scope->child_array;
    if (child_array) {
        for (i = 0; i < child_array->size; i++) {
            Scope *child = (Scope*)child_array->pointers[i];
            if (child->parent != scope) {
                fprintf(stderr, "Scope Error: Scope %p (%s) has child scope %p (%s) with wrong parent pointer %p\n",
                        (void*)scope, scope->name ? scope->name : "unnamed",
                        (void*)child, child->name ? child->name : "unnamed",
                        (void*)child->parent);
                (*errors)++;
            }
            validate_scope(child, errors);
        }
    }
}

void rxcp_validate_ast_and_symbols(ASTNode *root) {
    int errors = 0;
    if (!root) return;
    validate_node(root, &errors);
    if (root->scope) {
        Scope *s = root->scope;
        while (s->parent) s = s->parent;
        validate_scope(s, &errors);
    }
    if (errors) {
        fprintf(stderr, "AST/Symbol Validation failed with %d errors\n", errors);
        exit(1);
    }
}
