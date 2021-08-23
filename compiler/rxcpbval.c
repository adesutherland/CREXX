/*
 * rexbvald.c
 * REXX Level B Validations
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "rxcpmain.h"
#include "rxcpbgmr.h"
#include "rxvminst.h"

/* Get the assembler operandtype from the astnode type for the ASSEMBLER instruction */
static OperandType nodetype_to_operandtype(NodeType ntype) {
    switch (ntype) {
        case INTEGER: return OP_INT;
        case FLOAT: return OP_FLOAT;
        case STRING: return OP_STRING;
        default: return OP_REG;
    }
}

/* Step 1
 * - Fixes up procedure / class tree structures
 * - Sets the token and source start / finish position for each node
 * - Fixes SCONCAT to CONCAT
 * - Removes excess NOPs
 * - Process and Prune OPTIONS
 * - Validate REPEAT BY/FOR/DO
 * - Validate ASSEMBLER instructions
 */

static walker_result step1_walker(walker_direction direction,
                                  ASTNode* node, __attribute__((unused)) void *payload) {

    ASTNode *child, *next_child, *new_child, *next;
    int has_to;
    int has_for;
    int has_by;
    Token *left, *right;
    char *buffer;
    char *c;

    Context *context = (Context*)payload;

    /* Top down - Move instructions under the right procedure */
    if (direction == in) {

        if (node->node_type == PROGRAM_FILE) {
            /* Fix up the top of the tree */

            /* Remove the top INSTRUCTIONS node (2nd child) and promote its children */
            child = node->child->sibling->child; /* first child of INSTRUCTIONS */
            node->child->sibling = child; /* So REXX_OPTIONS sibling is the first instruction */
            while (child) {
                /* Fix the parent reference */
                child->parent = node;
                child = child->sibling;
            }

            if (node->child->sibling->node_type != PROCEDURE) {
                /* If the first instruction is not a PROCEDURE then we need to
                * add an implicit "main" PROCEDURE */
                child = ast_ftt(context, PROCEDURE, "main:");
                child->parent = node;
                child->sibling = node->child->sibling;
                node->child->sibling = child;

                /* Add function return type */
                add_ast(child,ast_ftt(context, CLASS, ".int"));

                /* Add function arguments (none) */
                add_ast(child,ast_ft(context, ARGS));
            }
        }
        else if (node->node_type == PROCEDURE) {
            if (node->parent->node_type != PROGRAM_FILE) {
                /* We can only define procedures in classes */
                mknd_err(node, "CANT_DEFINE_PROC_HEER");
            }
            else {
                /* Move node siblings (aka next instructions) until the next procedure to be node
                 * grand-children under a new INSTRUCTIONS node child */

                /* 1 .Make the new INSTRUCTIONS child */
                new_child = ast_ft(context,INSTRUCTIONS);
                add_ast(node,new_child);

                /* For each sibling until the next PROCEDURE */
                while ((next = node->sibling) && next->node_type != PROCEDURE) {
                    /* 2. Disconnect/remove next node from the AST tree */
                    node->sibling = next->sibling;
                    next->sibling = 0;
                    next->parent = 0;
                    /* 3. add next under INSTRUCTIONS child */
                    add_ast(new_child,next);
                }
            }
        }
    } else {
        /* Bottom up - source code positions, concat, etc */
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
        if (node->token_start) {
            left = node->token_start->token_prev;
            right = node->token_end->token_next;
            while (left && right && left->token_type==TK_OPEN_BRACKET && right->token_type==TK_CLOSE_BRACKET) {
                node->token_start = left;
                node->token_end = right;
                left = left->token_prev;
                right = right->token_next;
            }
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

        else if (node->node_type == ASSEMBLER) {
            /* ASSEMBLER operand types */
            OperandType type1, type2, type3;

            if (context->level != LEVELB) {
                /* ASSEMBLER is only valid in level b */
                mknd_err(node, "ASSEMBLER_ONLY_LEVELB");
            }

            else {

                child = node->child;
                if (child) {
                    type1 = nodetype_to_operandtype(child->node_type);
                    child = child->sibling;
                    if (child) {
                        type2 = nodetype_to_operandtype(child->node_type);
                        child = child->sibling;
                        if (child) type3 = nodetype_to_operandtype(child->node_type);
                        else type3 = OP_NONE;
                    }
                    else {
                        type2 = OP_NONE;
                        type3 = OP_NONE;
                    }
                }
                else {
                    type1 = OP_NONE;
                    type2 = OP_NONE;
                    type3 = OP_NONE;
                }

                /* Lookup Instruction */

                /* We need to copy it to a null terminated buffer and lowercase it! */
                buffer = malloc(node->node_string_length + 1);
                memcpy(buffer, node->node_string, node->node_string_length);
                buffer[node->node_string_length] = 0;
                for (c = buffer; *c; ++c) *c = (char) tolower(*c);

                /* Lookup */
                if (!src_inst(buffer, type1, type2, type3)) {
                    /* Invalid Instruction */
                    mknd_err(node, "INVALID_ASSEMBLER");
                }
                free(buffer);
            }
        }
    }
    return result_normal;
}

/* Step 2a
 * - Builds the Symbol Table
 */
static walker_result step2a_walker(walker_direction direction,
                                  ASTNode* node,
                                  void *payload) {

    Scope **current_scope = (Scope**)payload;
    Symbol *symbol;

    if (direction == in) {
        /* IN - TOP DOWN */
        if (node->node_type == PROGRAM_FILE) {
            *current_scope = scp_f(*current_scope, node);
        }
        else if (node->node_type == PROCEDURE) {
            node->node_string_length--; /* Remove the ':' */

            /* Check for duplicated */
            symbol = sym_rslv(*current_scope, node);
            if (symbol) {
                if (symbol->scope == *current_scope) {
                    /* Error */
                    mknd_err(node, "DUPLICATE_SYMBOL");
                }
            }

            /* Make a new symbol */
            symbol = sym_f(*current_scope, node);
            symbol->is_function = 1;

            sym_adnd(symbol, node, 0, 1);

            /* Move down to the procedure scope */
            *current_scope = scp_f(*current_scope, node);
        }

        if (node->node_type == VAR_TARGET || node->node_type == VAR_REFERENCE) {
            /* Find the symbol */
            symbol = sym_rslv(*current_scope, node);

            /* Make a new symbol if it does not exist */
            if (!symbol) {
                symbol = sym_f(*current_scope, node);
            }

            sym_adnd(symbol, node, 0, 1);
        }

        else if (node->node_type == VAR_SYMBOL) {
            /* Find the symbol */
            symbol = sym_rslv(*current_scope, node);

            /* Make a new symbol if it does not exist */
            if (!symbol) {
                symbol = sym_f(*current_scope, node);
            }

            if (node->parent->node_type == ASSEMBLER) {
                /* If an assembler instruction we need to assume read/write
                 * access - and therefore disable some optimisations */
                sym_adnd(symbol, node, 1, 1);
            }
            else sym_adnd(symbol, node, 1, 0);
        }

        else if (node->node_type == TO) {
            /* Find the symbol, the parents (REPEAT)'s first child (ASSIGN)'s
             * first child (VAR_TARGET)'s symbol */
            symbol = node->parent->child->child->symbol->symbol;
            sym_adnd(symbol, node, 1, 0);
        }

        else if (node->node_type == BY) {
            /* Find the symbol, the parents (REPEAT)'s first child (ASSIGN)'s
             * first child (VAR_TARGET)'s symbol */
            symbol = node->parent->child->child->symbol->symbol;
            sym_adnd(symbol, node, 1, 1); /* Increment = read & write */
        }
    }
    else {
        if (node->scope) *current_scope = (*current_scope)->parent;
    }

    return result_normal;
}

/* Step 2b
 * - Resolve Symbols
 */
static walker_result step2b_walker(walker_direction direction,
                                  ASTNode* node,
                                  void *payload) {

    Scope **current_scope = (Scope**)payload;
    Symbol *symbol;

    if (direction == in) {
        /* IN - TOP DOWN */
        if (node->scope) {
            *current_scope = node->scope;
        }
    }
    else {
        if (node->node_type == FUNCTION) {
            /* Find the symbol */
            symbol = sym_rslv(*current_scope, node);

            /* If there is not a symbol or it's not a function  */
            if (!symbol || !symbol->is_function) {
                mknd_err(node, "NOT_A_FUNCTION");
            }

            sym_adnd(symbol, node, 1, 0);
        }

        if (node->scope) *current_scope = (*current_scope)->parent;
    }

    return result_normal;
}

/*
 * Step 3 - Validate Symbols
 */
/* Helper function to compare node string value with a string
 * Used for builtin class names (int, float etc) - so don't need to worry
 * about utf
 * NOTE value MUST be in upper case! */
static int is_node_string(ASTNode* node, const char* value) {
    int i;
    /* If it is a different length it can't be the same! */
    if (strlen(value) != node->node_string_length) return 0;

    for (i=0; i < node->node_string_length; i++) {
        if (toupper(node->node_string[i]) != value[i]) return 0;
    }
    return 1;
}

/* Helper function to check for built-in classes */
static ValueType node_to_type(ASTNode* node) {
    switch (node->node_type) {
        case FLOAT:
            return TP_FLOAT;
        case INTEGER:
            return TP_INTEGER;
        case STRING:
            return TP_STRING;
        case CLASS:
            if (is_node_string(node, ".INT")) return TP_INTEGER;
            if (is_node_string(node, ".FLOAT")) return TP_FLOAT;
            if (is_node_string(node, ".STRING")) return TP_STRING;
            if (is_node_string(node, ".BOOLEAN")) return TP_BOOLEAN;
            return TP_OBJECT;
        default:
            return TP_UNKNOWN;
    }
}
static void validate_symbol_in_scope(Symbol *symbol, void *payload) {
    /* For REXX Level B the variable type is defined by its first use */
    SymbolNode *n = sym_trnd(symbol, 0);
    ASTNode* expr;
    char *buffer;
    size_t length;
    size_t i;

    if (n->node->node_type == PROCEDURE) {
        symbol->type = node_to_type(n->node->child);
        n->node->value_type = symbol->type;
        n->node->target_type = symbol->type;
    }
    else if (n->node->node_type == VAR_TARGET || n->node->node_type == VAR_REFERENCE) {
        symbol->type = node_to_type(n->node->sibling);
        n->node->value_type = symbol->type;
        n->node->parent->value_type = symbol->type;
        n->node->target_type = symbol->type;
        n->node->parent->target_type = symbol->type;
    }
    else {
        /* Used without definition/declaration - Taken Constant */
        /* TODO - for Level A/C/D we will need flow analysis to determine taken constant status */
        symbol->type = TP_STRING;
        symbol->is_constant = 1;
        /* Update all the attached AST Nodes to be constants */
        for (i=0; i<sym_nond(symbol); i++) {
            n = sym_trnd(symbol, i);
            if (n->writeUsage) {
                /* This means we are trying to write to a TAKEN CONSTANT
                 * which is illegal in Levels B/G/L */
                mknd_err(n->node, "UPDATING_TAKEN_CONSTANT");
            }
            else {
                n->node->node_type = STRING;
                length = strlen(symbol->name);
                buffer = malloc(length);
                memcpy(buffer, symbol->name, length);
                ast_sstr(n->node, buffer, length);
            }
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
    ASTNode *child1, *child2, *n1, *n2;
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
                if (node->symbol->symbol->type == TP_UNKNOWN)
                    node->symbol->symbol->type = TP_STRING;
                node->value_type = node->symbol->symbol->type;
                node->target_type = node->value_type;
                break;

            case FUNCTION:
                node->value_type = node->symbol->symbol->type;
                node->target_type = node->value_type;
                /* Process all the arguments */
                n1 = node->child;
                n2 = sym_trnd(node->symbol->symbol, 0)->node;
                while (n1) {
                    if (!n2) {
                        mknd_err(n1, "UNEXPECTED_ARGUMENT");
                        break;
                    }
                    n1->target_type = n2->value_type;
                    n1 = n1->sibling;
                    n2 = n2->sibling;
                }
                if (n2) {
                    mknd_err(node, "TOO_FEW_ARGUMENTS");
                }
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
                if (child1->symbol->symbol->type == TP_UNKNOWN) {
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
                    child1->symbol->symbol->type = child1->value_type;
                }
                else {
                    /* The Target Symbol has a type */
                    child1->value_type = child1->symbol->symbol->type;
                    child1->target_type = child1->symbol->symbol->type;
                    child2->target_type = child1->symbol->symbol->type;
                    node->value_type = child1->symbol->symbol->type;
                    node->target_type = child1->symbol->symbol->type;
                    child1->symbol->symbol->type = child1->value_type;
                }
                break;

            case ADDRESS:
            case SAY:
                if (child1) child1->target_type = TP_STRING;
                break;

            case RETURN:
                /* Type is the scope > procedure > type */
                if (child1) child1->target_type = (*current_scope)->defining_node->value_type;
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

    /* We need the assembler db for ASSEMBLE */
    if (context->level == LEVELB) init_ops();

    /* Step 1
     * - Sets the source start / finish for eac node
     * - Fixes SCONCAT to CONCAT
     * - Other AST fixups (TBC)
     */
    ast_wlkr(context->ast, step1_walker, (void *) context);

    /* Step 2
     * - Builds the Symbol Table
     */
    /* Mainly build symbols - procedures, members */
    current_scope = 0;
    ast_wlkr(context->ast, step2a_walker, (void *) &current_scope);

    /* Mainly resolve symbols - function uses */
    current_scope = 0;
    ast_wlkr(context->ast, step2b_walker, (void *) &current_scope);

    /* Step 3
     * - Validate Symbols
     */
    validate_symbols(context->ast->scope);

    /* Step 4
     * - Type Safety
     */
    current_scope = 0;
    ast_wlkr(context->ast, step4_walker, (void *) &current_scope);

    // Free Instruction Database
    if (context->level == LEVELB) free_ops();
 }