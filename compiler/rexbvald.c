/*
 * rexbvald.c
 * REXX Level B Validations
 */

#include "compiler.h"
#include "rexbgrmr.h"

/* Step 1
 * - Sets the source start / finish position for each node
 * - Fixes SCONCAT to CONCAT
 * - Other AST fixups (TBC)
 */

#define print_node printf("Line %d: %.*s\n", node->line + 1,(int)(node->source_end - node->source_start) + 1, node->source_start)

static walker_result step1_walker(walker_direction direction,
                                  ASTNode* node, __attribute__((unused)) void *payload) {

    ASTNode* child;
    char *ch;

    if (direction == out) {
        if (node->token) {
            node->source_start = node->token->token_string;
            node->source_end = node->token->token_string +
                               node->token->length - 1;
            node->line = node->token->line;
            node->column = node->token->column;
        } else {
            node->source_start = 0;
            node->source_end = 0;
            node->line = 0;
            node->column = 0;
        }

        child = node->child;
        while (child) {
            /* A non-terminal node  - so look at children */
            /* The children's source_start etc. will have been set already */
            if (child->source_start) {
                if (node->source_start) {
                    if (child->source_start < node->source_start) {
                        node->source_start = child->source_start;
                        node->line = child->line;
                        node->column = child->column;
                    }
                    if (child->source_end > node->source_end)
                        node->source_end = child->source_end;
                } else {
                    node->source_start = child->source_start;
                    node->source_end = child->source_end;
                    node->line = child->line;
                    node->column = child->column;
                }
            }
            child = child->sibling;
        }
/*
        // Prints out the AST source string
        if (node->source_start) {
            printf("%s=",ast_nodetype(node->node_type));
            print_unescaped(stdout, node->source_start,
                            (int)(node->source_end - node->source_start + 1));
            printf("\n");

        }
*/
        if (node->node_type == PROGRAM_FILE) {
            /* prune REXX_OPTIONS */
            if (node->child->node_type == REXX_OPTIONS) {
                node->child = node->child->sibling;
            }
        }
        if (node->value_type == REXX_OPTIONS) {
            /* TODO Process any REXX options */
        }
        else if (node->node_type == OP_SCONCAT) {
            /* We need to decide if there is white space between the tokens */
            if (node->child->sibling->source_start - node->child->source_end == 1)
                node->node_type = OP_CONCAT; /* No gap */
            {
                /* OK There is a gap but we need to check it is actually
                 * whitespace - it could be a ( or ) for example */
                for (ch=node->child->source_end + 1;
                    ch<node->child->sibling->source_start; ch++) {
                    if (*ch == ' ' || *ch == '\t'
                        || *ch == '\v' || *ch == '\f') break;
                }
                if (ch == node->child->sibling->source_start)
                    node->node_type = OP_CONCAT; /* No WS Found */
            }
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
            symbol_add_astnode(symbol, node);
        }
    }

    return result_normal;
}

/*
 * Step 3 - Validate Symbols
 */
void validate_symbol_in_scope(Symbol *symbol, void *payload) {
    /* For REXX Level B the variable type is defined by its first use */
    ASTNode* n = symbol_astnode(symbol,0);
    ASTNode* parent;
    ASTNode* expr;
    if (n->node_type == VAR_SYMBOL) {
        /* Used without definition/declaration - Taken Constant */
        symbol->type = TP_STRING;
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
    int i = 0;
    ASTNode *p;
    if (!scope) return;

    p = scope->defining_node;
    /*
    if (p->node_type == PROGRAM_FILE) {
        printf("SCOPE PROGRAM\n");
    }
    else {
        printf("SCOPE \"%.*s\"\n",
               (int) (p->source_end - p->source_start + 1),
               p->source_start);
    }
    */
    scp_for_all(scope, validate_symbol_in_scope, NULL);
    for (i=0; i < scope_num_children(scope); i++) {
        validate_symbols(scope_child(scope,i));
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

            case OP_COMPARE:
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
                child1->value_type = child2->value_type;
                child1->target_type = child2->value_type;
                child2->target_type = child2->value_type;
                node->value_type = child2->value_type;
                node->target_type = child2->value_type;
                if (child1->symbol->type == TP_UNKNOWN)
                    child1->symbol->type = child2->value_type;
                break;

            case ADDRESS:
            case SAY:
                if (child1) child1->target_type = TP_STRING;
                break;

            case IF:
                if (child1) child1->target_type = TP_BOOLEAN;
                break;

            default:;
        }

        if (node->scope) *current_scope = (*current_scope)->parent;
    }

    return result_normal;
}



/* Step 5
 * - Print errors
 */
static walker_result step5_walker(walker_direction direction,
                                  ASTNode* node,
                                  __attribute__((unused)) void *payload) {

    if (direction == in) {
        if (node->node_type == ERROR) {
            printf("Error @ %d.%d - %s \"", node->line+1, node->column+1, node->node_string);
            print_unescaped(stdout, node->source_start,
                            (int)(node->source_end - node->source_start + 1));
            printf("\"\n");
        }
    }
    return result_normal;
}

void validate(Context *context) {
    Scope *current_scope;

    /* Step 1
     * - Sets the source start / finish for eac node
     * - Fixes SCONCAT to CONCAT
     * - Other AST fixups (TBC)
     */
    ast_walker(context->ast, step1_walker, 0);

    /* Step 2
     * - Builds the Symbol Table
     */
    current_scope = 0;
    ast_walker(context->ast, step2_walker, (void*)&current_scope);

    /* Step 3
     * - Validate Symbols
     */
//    validate_symbols(context->ast->scope);

    /* Step 4
     * - Type Safety
     */
    current_scope = 0;
    ast_walker(context->ast, step4_walker, (void*)&current_scope);

    /* Step 5
     * - Print errors
     */
    ast_walker(context->ast, step5_walker, 0);
}