/*
 * rexbvald.c
 * REXX Level B Validations
 */

#include <string.h>
#include <stdlib.h>
#include "rxcpmain.h"
#include "rxcpbgmr.h"

/* Step 1
 * - Sets the token and source start / finish position for each node
 * - Fixes SCONCAT to CONCAT
 * - Removes excess NOPs
 * - Other AST fixups / validations
 */

#define print_node printf("Line %d: %.*s\n", node->line + 1,(int)(node->source_end - node->source_start) + 1, node->source_start)

static walker_result step1_walker(walker_direction direction,
                                  ASTNode* node, __attribute__((unused)) void *payload) {

    ASTNode *child, *next_child;
    int has_to;
    int has_for;
    int has_by;
    Token *left, *right;

    Context *context = (Context*)payload;

    if (direction == out) {
        if (node->token) {
            node->token_start = node->token;
            node->token_end = node->token;
        } else {
            node->token_start = 0;
            node->token_end = 0;
        }

        child = node->child;
        while (child) {
            /* A non-terminal node  - so look at children */
            /* The children's token_start etc. will have been set already */
            if (child->token_start) {
                if (node->token_start) {
                    if (child->token_start->token_number < node->token_start->token_number)
                        node->token_start = child->token_start;
                    if (child->token_end->token_number > node->token_end->token_number)
                        node->token_end = child->token_end;
                } else {
                    node->token_start = child->token_start;
                    node->token_end = child->token_end;
                }
            }
            child = child->sibling;
        }

        /* To pretty generated comments, and for code re-writing, and proper
         * source code analysis, we need to expand the source code to include
         * any '('s to the left and ')'s to the right as these are removed from
         * the AST. What we are doing is expanding the selection to include
         * *matching* ('s and )'s. Where they don't match, ignore, they will be
         * handled by a parent or grandparent node */
        left = node->token_start->token_prev;
        right = node->token_end->token_next;
        while (left && right && left->token_type==TK_OPEN_BRACKET && right->token_type==TK_CLOSE_BRACKET) {
            node->token_start = left;
            node->token_end = right;
            left = left->token_prev;
            right = right->token_next;
        }

        /* Set code start and end position */
        if (node->token_start) {
            node->source_start = node->token_start->token_string;
            node->source_end = node->token_end->token_string +
                               node->token_end->length - 1;
            node->line = node->token_start->line;
            node->column = node->token_start->column;
        } else {
            node->source_start = 0;
            node->source_end = 0;
            node->line = 0;
            node->column = 0;
        }

        if (node->node_type == PROGRAM_FILE) {
            /* prune REXX_OPTIONS */
            if (node->child->node_type == REXX_OPTIONS) {
                node->child = node->child->sibling;
            }
        }
        else if (node->node_type == REXX_OPTIONS) {
            /* TODO Process any REXX options */
        }
        else if (node->node_type == REPEAT) {
            /* Validate Sub-commands - Error 27.1 */
            has_to = 0;
            has_for = 0;
            has_by = 0;
            child = node->child->sibling; /* Second Child - the first is the assignment */
            while (child) {
                if (child->node_type == BY) {
                    if (has_by) mknd_err(child, "27.1");
                    else has_by = 1;
                }
                else if (child->node_type == FOR) {
                    if (has_for) mknd_err(child, "27.1");
                    else has_for = 1;
                }
                else if (child->node_type == TO) {
                        if (has_to) mknd_err(child, "27.1");
                        else has_to = 1;
                }
                child = child->sibling;
            }
            if (has_to && !has_by) {
                /* Need to add implicit "BY" node - to avoid an infinite loop! */
                add_ast(node, ast_ft(context, BY));
            }
        }
        else if (node->node_type == OP_SCONCAT) {
            /* We need to decide if there is white space between the tokens */
            if (node->child->sibling->source_start - node->child->source_end == 1)
                node->node_type = OP_CONCAT; /* No gap */
        }

        else if (node->node_type == INSTRUCTIONS) {
            /* Remove Excess NOPs */
            child = node->child;
            while (child) {
                next_child = child->sibling;
                if (child->node_type == NOP)
                    ast_del(child);
                child = next_child;
            }
            if (!node->child)
                node->node_type = NOP; /* Convert empty STATEMENTS to NOP */
        }
    }
    return result_normal;
}

/* Step 2
 * - Builds the Symbol Table
 */
static walker_result step2_walker(walker_direction direction,
                                  ASTNode* node,
                                  void *payload) {

    Scope **current_scope = (Scope**)payload;
    ASTNode* child;
    Symbol *symbol;

    if (direction == in) {
        /* IN - TOP DOWN */
        if (node->node_type == PROGRAM_FILE) {
            *current_scope = scp_f(*current_scope, node);
        }
        else if (node->node_type == PROCEDURE) {
            // TBC
        }
    }
    else {
        /* OUT - BOTTOM UP */
        if (node->node_type == PROGRAM_FILE) {
            *current_scope = (*current_scope)->parent;
        }
        else if (node->node_type == PROCEDURE) {
            // TBC
        }
        else if (node->node_type == VAR_TARGET ||
                 node->node_type == VAR_SYMBOL) {
            /* Find the symbol */
            symbol = sym_rslv(*current_scope, node);

            /* Make a new symbol if it does not exist */
            if (!symbol) {
                symbol = sym_f(*current_scope, node);
            }

            node->symbol = symbol;
            sym_adnd(symbol, node);
        }
    }

    return result_normal;
}

/*
 * Step 3 - Validate Symbols
 */
static void validate_symbol_in_scope(Symbol *symbol, void *payload) {
    /* For REXX Level B the variable type is defined by its first use */
    ASTNode* n = sym_trnd(symbol, 0);
    ASTNode* parent;
    ASTNode* expr;
    char *buffer;
    size_t length;
    size_t i;

    if (n->node_type == VAR_SYMBOL) {
        /* Used without definition/declaration - Taken Constant */
        /* TODO - for Level A/C/D we will need flow analysis to determine taken constant status */
        symbol->type = TP_STRING;
        symbol->is_constant = 1;
        /* Update all the attached AST Nodes to be constants */
        for (i=0; i<sym_nond(symbol); i++) {
            n = sym_trnd(symbol, i);
            if (n->node_type != VAR_SYMBOL) {
                /* This means we are trying to write to a TAKEN CONSTANT
                 * which is illegal in Levels B/G/L */
                mknd_err(n, "UPDATING_TAKEN_CONSTANT");
            }
            else {
                n->node_type = STRING;
                length = strlen(symbol->name);
                buffer = malloc(length);
                memcpy(buffer, symbol->name, length);
                ast_sstr(n, buffer, length);
            }
        }
    }
    else {
        expr = n->sibling;
        switch (expr->node_type) {
            case FLOAT:
                symbol->type = TP_FLOAT;
                break;
            case INTEGER:
                symbol->type = TP_INTEGER;
                break;
            case STRING:
                symbol->type = TP_STRING;
                break;
            default:;
        }
    }
}

static void validate_symbols(Scope* scope) {
    int i;
    if (!scope) return;

    scp_4all(scope, validate_symbol_in_scope, NULL);
    for (i=0; i < scp_noch(scope); i++) {
        validate_symbols(scp_chd(scope, i));
    }
}

/* Step 4
 * - Type Safety
 */

/* Type promotion matrix for numeric operators */
const ValueType promotion[6][6] = {
/*                TP_UNKNOWN, TP_BOOLEAN, TP_INTEGER, TP_FLOAT, TP_STRING,  TP_OBJECT */
/* TP_UNKNOWN */ {TP_UNKNOWN, TP_BOOLEAN, TP_INTEGER, TP_FLOAT, TP_FLOAT,   TP_FLOAT},
/* TP_BOOLEAN */ {TP_BOOLEAN, TP_BOOLEAN, TP_INTEGER, TP_FLOAT, TP_BOOLEAN, TP_BOOLEAN},
/* TP_INTEGER */ {TP_INTEGER, TP_INTEGER, TP_INTEGER, TP_FLOAT, TP_INTEGER, TP_INTEGER},
/* TP_FLOAT */   {TP_FLOAT,   TP_FLOAT,   TP_FLOAT,   TP_FLOAT, TP_FLOAT,   TP_FLOAT},
/* TP_STRING */  {TP_FLOAT,   TP_BOOLEAN, TP_INTEGER, TP_FLOAT, TP_FLOAT,   TP_FLOAT},
/* TP_OBJECT */  {TP_FLOAT,   TP_BOOLEAN, TP_INTEGER, TP_FLOAT, TP_FLOAT,   TP_FLOAT}
};

static walker_result step4_walker(walker_direction direction,
                                  ASTNode* node,
                                  void *payload) {

    Scope **current_scope = (Scope**)payload;
    ASTNode *child1, *child2;
    Symbol *symbol;
    ValueType max_type = TP_UNKNOWN;

    if (direction == in) {
        /* IN - TOP DOWN */
        if (node->scope) {
            *current_scope = node->scope;
        }
    }
    else {
        /* OUT - BOTTOM UP */

        child1 = node->child;
        if (child1) {
            child2 = child1->sibling;
            if (child2) {
                if (child1->value_type > child2->value_type)
                    max_type = child1->value_type;
                else
                    max_type = child2->value_type;
            }
            else {
                max_type = child1->value_type;
            }
        }
        else {
            child2 = NULL;
        }

        switch (node->node_type) {

            case OP_AND:
            case OP_OR:
                node->value_type = TP_BOOLEAN;
                child1->target_type = TP_BOOLEAN;
                child2->target_type = TP_BOOLEAN;
                break;

            case OP_COMPARE_EQUAL:
            case OP_COMPARE_NEQ:
            case OP_COMPARE_GT:
            case OP_COMPARE_LT:
            case OP_COMPARE_GTE:
            case OP_COMPARE_LTE:
            case OP_COMPARE_S_EQ:
            case OP_COMPARE_S_NEQ:
            case OP_COMPARE_S_GT:
            case OP_COMPARE_S_LT:
            case OP_COMPARE_S_GTE:
            case OP_COMPARE_S_LTE:
                node->value_type = TP_BOOLEAN;
                child1->target_type = max_type;
                child2->target_type = max_type;
                break;

            case OP_CONCAT:
            case OP_SCONCAT:
                node->value_type = TP_STRING;
                child1->target_type = TP_STRING;
                child2->target_type = TP_STRING;
                break;

            case OP_ADD:
            case OP_MINUS:
            case OP_MULT:
            case OP_POWER:
                node->value_type = promotion[child1->value_type][child2->value_type];
                child1->target_type = promotion[child1->value_type][child2->value_type];
                child2->target_type = promotion[child1->value_type][child2->value_type];
                break;

            case OP_DIV:
                node->value_type = TP_FLOAT;
                child1->target_type = TP_FLOAT;
                child2->target_type = TP_FLOAT;
                break;

            case OP_IDIV:
            case OP_MOD:
                node->value_type = TP_INTEGER;
                child1->target_type = TP_INTEGER;
                child2->target_type = TP_INTEGER;
                break;

            case OP_PREFIX:
                child1 = node->child;
                if (node->token->token_type == TK_NOT) {
                    node->value_type = TP_BOOLEAN;
                    child1->target_type = TP_BOOLEAN;
                    break;
                }
                node->value_type = child1->value_type;
                child1->target_type = child1->value_type;
                break;

            case VAR_SYMBOL:
                if (node->symbol->type == TP_UNKNOWN)
                    node->symbol->type = TP_STRING;
                node->value_type = node->symbol->type;
                node->target_type = node->value_type;
                break;

            case CONST_SYMBOL:
                node->value_type = TP_STRING;
                node->target_type = node->value_type;
                break;

            case FLOAT:
                node->value_type = TP_FLOAT;
                node->target_type = node->value_type;
                break;

            case INTEGER:
                node->value_type = TP_INTEGER;
                node->target_type = node->value_type;
                break;

            case STRING:
                node->value_type = TP_STRING;
                node->target_type = node->value_type;
                break;

            case ASSIGN:
                if (child1->symbol->type == TP_UNKNOWN) {
                    /* If the symbol does not have a known type yet */
                    if (node->parent->node_type == REPEAT) {
                        /* Special logic for LOOP Assignment - type must be numeric */
                        child1->value_type =
                                promotion[child2->value_type][TP_INTEGER];
                    } else {
                        child1->value_type = child2->value_type;
                    }
                    child1->target_type = child1->value_type;
                    child2->target_type = child1->value_type;
                    node->value_type = child1->value_type;
                    node->target_type = child1->value_type;
                    child1->symbol->type = child1->value_type;
                }
                else {
                    /* The Target Symbol has a type */
                    child1->value_type = child1->symbol->type;
                    child1->target_type = child1->symbol->type;
                    child2->target_type = child1->symbol->type;
                    node->value_type = child1->symbol->type;
                    node->target_type = child1->symbol->type;
                    child1->symbol->type = child1->value_type;
                }
                break;

            case ADDRESS:
            case SAY:
                if (child1) child1->target_type = TP_STRING;
                break;

            case IF:
                if (child1) child1->target_type = TP_BOOLEAN;
                break;

            /* Loops */
            case TO:
            case BY:
                /* The TO/BY value type needs to be the same as the assigment type */
                if (child1) child1->target_type = node->parent->child->value_type;
                node->value_type = node->parent->child->value_type;
                node->target_type = node->parent->child->value_type;
                break;

            case FOR:
                /* The TO/BY value type needs to be the same as the assigment type */
                child1->target_type = TP_INTEGER;
                node->value_type = child1->target_type;
                node->target_type = child1->target_type;
                break;

            default:;
        }

        if (node->scope) *current_scope = (*current_scope)->parent;
    }

    return result_normal;
}

/* Validate AST */
void validate(Context *context) {
    Scope *current_scope;

    /* Step 1
     * - Sets the source start / finish for eac node
     * - Fixes SCONCAT to CONCAT
     * - Other AST fixups (TBC)
     */
    ast_wlkr(context->ast, step1_walker, (void *) context);

    /* Step 2
     * - Builds the Symbol Table
     */
    current_scope = 0;
    ast_wlkr(context->ast, step2_walker, (void *) &current_scope);

    /* Step 3
     * - Validate Symbols
     */
    validate_symbols(context->ast->scope);

    /* Step 4
     * - Type Safety
     */
    current_scope = 0;
    ast_wlkr(context->ast, step4_walker, (void *) &current_scope);
 }