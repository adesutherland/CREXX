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
 * - Process OPTIONS
 * - Validate REPEAT BY/FOR/DO
 * - Validate ASSEMBLER instructions
 */

static walker_result step1_walker(walker_direction direction,
                                  ASTNode* node, __attribute__((unused)) void *payload) {

    ASTNode *child, *next_child, *new_child, *next, *last;
    int has_to;
    int has_for;
    int has_by;
    int has_assign;
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
            }
        }
        else if (node->node_type == PROCEDURE) {
            if (node->parent->node_type != PROGRAM_FILE) {
                /* We can only define procedures in classes */
                mknd_err(node, "CANT_DEFINE_PROC_HERE");
            }
            else {
                /* Move node siblings (aka next instructions) until the next procedure to be node
                 * grand-children under a new INSTRUCTIONS node child */

                /* Process ARG */
                /* Process each sibling until the next PROCEDURE */
                ASTNode *args_node = 0;
                char first_instruction = 1;
                next = node->sibling;
                while (next && next->node_type != PROCEDURE) {
                    if (next->node_type == ARGS) {
                        if (args_node) {
                            /* Error - you can only have one arg statement */
                            mknd_err(next, "REPEATED_ARG");
                            next = next->sibling;
                        }
                        else if (!first_instruction) {
                            /* Error - arg must be the first statement */
                            mknd_err(next, "ARG_NOT_FIRST_INST");
                            next = next->sibling;
                        }
                        else {
                            args_node = next;
                            /* Disconnect/remove node from the AST tree */
                            node->sibling = next->sibling;
                            next->sibling = 0;
                            next->parent = 0;
                            /* And add under the procedure */
                            add_ast(node, next);
                            next = node->sibling;
                        }
                    }
                    else {
                        next = next->sibling;
                        first_instruction = 0;
                    }
                }
                /* Add an empty ARGS node if no arguments have been specified */
                if (!args_node) {
                    new_child = ast_ft(context,ARGS);
                    add_ast(node,new_child);
                }

                /* Make the new INSTRUCTIONS child */
                new_child = ast_ft(context,INSTRUCTIONS);
                add_ast(node,new_child);

                last = NULL;

                /* For each sibling until the next PROCEDURE */
                while ((next = node->sibling) && next->node_type != PROCEDURE) {
                    last = next; /* To check that there is a return */
                    /* 2. Disconnect/remove next node from the AST tree */
                    node->sibling = next->sibling;
                    next->sibling = 0;
                    next->parent = 0;
                    /* 3. add next under INSTRUCTIONS child */
                    add_ast(new_child,next);
                }

                if (last) { /* If there are no instructions at all it is a declaration */
                    if (last->node_type != RETURN) {
                        /* We need to add a return */
                        new_child = ast_ft(context,RETURN);
                        add_ast(last->parent,new_child); /* Adds as the last sibling */
                    }
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
         * handled by a parent or grandparent node
         *
         * And we do the same thing for functions checking the ( after the function name
         *
         * And for exposed arguments (to include the expose)
         */
        if (node->token_start) {
            if (node->node_type == VAR_REFERENCE) {
                node->token_start = node->token_start->token_prev;
            }
            else if (node->node_type == FUNCTION) {
                /* Function brackets */
                left = node->token_start->token_next; /* I.e. after the function name */
                right = node->token_end->token_next;
                if (left && right && left->token_type == TK_OPEN_BRACKET &&
                       right->token_type == TK_CLOSE_BRACKET) {
                    node->token_end = right;
                }
            }
            else {
                /* Other brackets */
                left = node->token_start->token_prev;
                right = node->token_end->token_next;
                while (left && right && left->token_type == TK_OPEN_BRACKET &&
                       right->token_type == TK_CLOSE_BRACKET) {
                    node->token_start = left;
                    node->token_end = right;
                    left = left->token_prev;
                    right = right->token_next;
                }
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
            /* This is a leaf without a token - so need to estimate a position */
            ASTNode *older = 0;
            ASTNode *n;

            /* In case we fail to estimate */
            node->source_start = 0;
            node->source_end = 0;
            node->line = -1;
            node->column = -1;

            /* Older Sibling ? */
            n = node->parent->child;
            while (n != node) {
                if (!n) {
                    /* Internal Error - bail */
                    fprintf(stderr, "Internal Error: Node is not one of its parent's children\n");
                    exit(1);
                }
                older = n;
                n = n->sibling;
            }
            if (older && older->line != -1) { /* Check if the older has valid line number (it should!) */
                node->source_start = older->source_end + 1;
                node->source_end = node->source_start ? (node->source_start - 1) : 0;
                node->line = older->line;
                node->column = older->column + (int)(older->source_end - older->source_start) + 1;
            }
            else {
                /* No older sibling - use the parent, grandparent, until we find a token */
                n = node->parent;
                while (n) {
                    if (n->token) {
                        node->source_start = n->token->token_string + n->token->length;
                        node->source_end = node->source_start ? (node->source_start - 1) : 0;
                        node->line = n->token->line;
                        node->column = n->token->column + n->token->length;
                        break;
                    }
                    n = n->parent;
                }
            }
        }

        if (node->node_type == PROGRAM_FILE) {
            /* Set namespace if not already set */
            if (!context->namespace) {
                node->node_string = context->file_name;
                node->node_string_length = strlen(context->file_name);
            }
            else {
                node->node_string = context->namespace->node_string;
                node->node_string_length = context->namespace->node_string_length;
            }
        }
        else if (node->node_type == REXX_OPTIONS) {
            /* TODO Process any REXX options specific for levelb */
        }
        else if (node->node_type == NAMESPACE) {
            if (!context->namespace) {
                context->namespace = node->child;
            }
            else {
                mknd_err(node, "MULTIPLE_NAMESPACE");
            }
        }
        else if (node->node_type == REPEAT) {
            /* Validate Sub-commands - Error 27.1 */
            has_to = 0;
            has_for = 0;
            has_by = 0;
            has_assign = 0;
            child = node->child;
            while (child) {
                if (child->node_type == ASSIGN) {
                    has_assign = 1;
                }
                else if (child->node_type == BY) {
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
            if (has_assign && !has_by) {
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

/* Step 1b
 * - Set node ordinal values
 */
static walker_result step1b_walker(walker_direction direction,
                                   ASTNode* node,
                                   void *payload) {
    int* ordinal_counter = (int*)payload;

    if (direction == out) {
        /* BOTTOM-UP */
        node->high_ordinal = (*ordinal_counter)++;

        if (node->child) node->low_ordinal = node->child->low_ordinal;
        else node->low_ordinal = node->high_ordinal;
    }
    return result_normal;
}

/* Step 2a
 * - Builds the Symbol Table
 */
static walker_result step2a_walker(walker_direction direction,
                                  ASTNode* node,
                                  void *payload) {

    Context *context = (Context*)payload;
    Symbol *symbol;

    if (direction == in) {
        /* IN - TOP DOWN */
        if (node->node_type == REXX_UNIVERSE) {
            /* This top level scope will contain the project file scope & imported file scopes */
            context->current_scope = scp_f(context->current_scope, node);
        }

        else if (node->node_type == PROGRAM_FILE) {
            /* Now create the namespace symbol and scope */
            /* Make the new symbol */
            symbol = sym_f(context->current_scope, node);
            symbol->symbol_type = NAMESPACE_SYMBOL;
            sym_adnd(symbol, node, 1, 1);
            if (context->namespace) sym_adnd(symbol, context->namespace, 0, 1);

            /* Move down to the project file scope */
            context->current_scope = scp_f(context->current_scope, node);

            /* Set the scope name to be the new symbol name */
            (context->current_scope)->name = symbol->name;
        }

        else if (node->node_type == PROCEDURE) {
            node->node_string_length--; /* Remove the ':' */

            /* Check for duplicated */
            symbol = sym_rslv(context->current_scope, node);
            if (symbol) {
                if (symbol->scope == context->current_scope) {
                    /* Error */
                    mknd_err(node, "DUPLICATE_SYMBOL");
                }
                else symbol = 0;
            }

            /* Make a new symbol */
            if (!symbol) { /* If there is a symbol we are in an error condition but are pressing on */
                symbol = sym_f(context->current_scope, node);
                symbol->symbol_type = FUNCTION_SYMBOL;
            }

            sym_adnd(symbol, node, 0, 1);

            /* Move down to the procedure scope */
            context->current_scope = scp_f(context->current_scope, node);

            /* Set the scope name to be the procedure symbol name */
            (context->current_scope)->name = symbol->name;
        }

        else if (node->node_type == IMPORT) {
            /* Get the toplevel scope that contains all the namespaces */
            Scope* namespaces = context->ast->scope;
            Scope* imported_namespace;

            /* Now create the imported namespace symbol and scope */
            /* Make the new symbol */
            symbol = sym_f(namespaces, node->child);
            if (symbol) {
                symbol->symbol_type = NAMESPACE_SYMBOL;
                sym_adnd(symbol, node->child, 0, 1);

                /* New scope scope */
                imported_namespace = scp_f(namespaces, node->child);
                imported_namespace->name = symbol->name;
            }
            else {
                mknd_err(node->child, "DUPLICATE_NAMESPACE");
            }
        }

        else if (node->node_type == VAR_TARGET || node->node_type == VAR_REFERENCE) {
            node->scope = context->current_scope;
            /* Find the symbol */
            if (node->parent->node_type == ARG) /* Only search current scope */
                symbol = sym_lrsv(context->current_scope, node);
            else /* Search parent scopes */
                symbol = sym_rslv(context->current_scope, node);

            /* Make a new symbol if it does not exist */
            if (!symbol) {
                symbol = sym_f(context->current_scope, node);
            }
            else if (symbol->symbol_type == FUNCTION_SYMBOL) {
                mknd_err(node, "IS_A_FUNCTION");
            }
            else if (symbol->symbol_type == CLASS_SYMBOL) {
                mknd_err(node, "IS_A_CLASS");
            }
            else if (symbol->symbol_type == NAMESPACE_SYMBOL) {
                mknd_err(node, "IS_A_NAMESPACE");
            }

            sym_adnd(symbol, node, 0, 1);
        }

        else if (node->node_type == VAR_SYMBOL) {
            node->scope = context->current_scope;
            /* Find the symbol */
            symbol = sym_rslv(context->current_scope, node);

            /* Make a new symbol if it does not exist */
            if (!symbol) {
                symbol = sym_f(context->current_scope, node);
            }
            else if (symbol->symbol_type == FUNCTION_SYMBOL) {
                mknd_err(node, "IS_A_FUNCTION");
            }
            else if (symbol->symbol_type == CLASS_SYMBOL) {
                mknd_err(node, "IS_A_CLASS");
            }
            else if (symbol->symbol_type == NAMESPACE_SYMBOL) {
                mknd_err(node, "IS_A_NAMESPACE");
            }

            if (node->parent->node_type == ASSEMBLER) {
                /* If an assembler instruction we need to assume read/write
                 * access - and therefore disable some optimisations */
                sym_adnd(symbol, node, 1, 1);
            }
            else sym_adnd(symbol, node, 1, 0);
        }

        else if (node->node_type == TO) {
            node->scope = context->current_scope;
            /* Find the symbol, the parents (REPEAT)'s first child (ASSIGN)'s
             * first child (VAR_TARGET)'s symbol
             * Note: If the REPEAT has a TO it has an assign */
            symbol = node->parent->child->child->symbolNode->symbol;
            sym_adnd(symbol, node, 1, 0);
        }

        else if (node->node_type == BY) {
            node->scope = context->current_scope;
            /* Find the symbol, the parents (REPEAT)'s first child (ASSIGN)'s
             * first child (VAR_TARGET)'s symbol
             * Note: If the REPEAT has a BY it has an assign*/
            symbol = node->parent->child->child->symbolNode->symbol;
            sym_adnd(symbol, node, 1, 1); /* Increment = read & write */
        }

        else {
            node->scope = context->current_scope;
        }
    }

    else {
        if (node->parent) context->current_scope = node->parent->scope;
        else context->current_scope = 0;
    }

    return result_normal;
}

/* Step 2b
 * - Resolve Symbols
 */
static walker_result step2b_walker(walker_direction direction,
                                  ASTNode* node,
                                  void *payload) {

    Context *context = (Context*)payload;
    Symbol *symbol;

    if (direction == in) {
        /* IN - TOP DOWN */
    }
    else {
        if (node->node_type == FUNCTION) {
            /* Find the symbol */
            symbol = sym_rslv(node->scope, node);

            /* If there is a symbol and it's a function - found  */
            if (symbol && symbol->symbol_type == FUNCTION_SYMBOL ) {
                sym_adnd(symbol, node, 1, 0);
            }

            /* If there is a symbol and it's not a function - error */
            else if (symbol && symbol->symbol_type != FUNCTION_SYMBOL ) {
                mknd_err(node, "NOT_A_FUNCTION");
            }

            /* We have to try and import the function  */
            else {
                symbol = sym_imfn(context, node);
                if (!symbol) mknd_err(node, "FUNCTION_NOT_FOUND");
                else sym_adnd(symbol, node, 1, 0);
            }
        }
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
        if (tolower(node->node_string[i]) != value[i]) return 0;
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
        case VOID:
            return TP_VOID;
        case CLASS:
            if (is_node_string(node, ".int")) return TP_INTEGER;
            if (is_node_string(node, ".float")) return TP_FLOAT;
            if (is_node_string(node, ".string")) return TP_STRING;
            if (is_node_string(node, ".boolean")) return TP_BOOLEAN;
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
    else if (symbol->symbol_type != NAMESPACE_SYMBOL) {
        /* Used without definition/declaration - Taken Constant */
        /* TODO - for Level A/C/D we will need flow analysis to determine taken constant status */
        symbol->type = TP_STRING;
        symbol->symbol_type = CONSTANT_SYMBOL;
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
const ValueType promotion[7][7] = {
/*                TP_UNKNOWN, TP_VOID,  TP_BOOLEAN, TP_INTEGER, TP_FLOAT,   TP_STRING,  TP_OBJECT */
/* TP_UNKNOWN */ {TP_UNKNOWN, TP_VOID,  TP_BOOLEAN, TP_INTEGER, TP_FLOAT,   TP_FLOAT,   TP_FLOAT},
/* TP_VOID */    {TP_VOID,    TP_VOID,  TP_VOID,    TP_VOID,    TP_VOID,    TP_VOID,    TP_VOID},
/* TP_BOOLEAN */ {TP_BOOLEAN, TP_VOID,  TP_BOOLEAN, TP_INTEGER, TP_FLOAT,   TP_BOOLEAN, TP_BOOLEAN},
/* TP_INTEGER */ {TP_INTEGER, TP_VOID,  TP_INTEGER, TP_INTEGER, TP_FLOAT,   TP_INTEGER, TP_INTEGER},
/* TP_FLOAT */   {TP_FLOAT,   TP_VOID,  TP_FLOAT,   TP_FLOAT,   TP_FLOAT,   TP_FLOAT,   TP_FLOAT},
/* TP_STRING */  {TP_FLOAT,   TP_VOID,  TP_BOOLEAN, TP_INTEGER, TP_FLOAT,   TP_FLOAT,   TP_FLOAT},
/* TP_OBJECT */  {TP_FLOAT,   TP_VOID,  TP_BOOLEAN, TP_INTEGER, TP_FLOAT,   TP_FLOAT,   TP_FLOAT}
};

static walker_result step4_walker(walker_direction direction,
                                  ASTNode* node,
                                  void *payload) {

    Context *context = (Context*)payload;
    ASTNode *child1, *child2, *n1, *n2;
    ValueType max_type = TP_UNKNOWN;

    if (direction == in) {
        /* IN - TOP DOWN */
        context->current_scope = node->scope;
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

            case OP_NOT:
                node->value_type = TP_BOOLEAN;
                child1->target_type = TP_BOOLEAN;
                break;

            case OP_PLUS:
            case OP_NEG:
                node->value_type = promotion[child1->value_type][TP_UNKNOWN];
                child1->target_type = promotion[child1->value_type][TP_UNKNOWN];
                break;

            case FUNCTION:
                node->value_type = node->symbolNode->symbol->type;
                node->target_type = node->value_type;
                break;

            case VAR_SYMBOL:
                if (node->symbolNode->symbol->type == TP_UNKNOWN)
                    node->symbolNode->symbol->type = TP_STRING;
                node->value_type = node->symbolNode->symbol->type;
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

            case CLASS:
                node->value_type = node_to_type(node);
                node->target_type = node->value_type;
                break;

            case ASSIGN:
                if (child2->value_type == TP_VOID) {
                    mknd_err(child2, "RETURNS_VOID");
                }
                else {
                    if (child1->symbolNode->symbol->type == TP_UNKNOWN) {
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
                        child1->symbolNode->symbol->type = child1->value_type;
                    } else {
                        /* The Target Symbol has a type */
                        child1->value_type = child1->symbolNode->symbol->type;
                        child1->target_type = child1->symbolNode->symbol->type;
                        child2->target_type = child1->symbolNode->symbol->type;
                        node->value_type = child1->symbolNode->symbol->type;
                        node->target_type = child1->symbolNode->symbol->type;
                        child1->symbolNode->symbol->type = child1->value_type;
                    }
                }
                break;

            case ARG:
                if (child1->symbolNode->symbol->type == TP_UNKNOWN) {
                    /* If the symbol does not have a known type yet */
                    child1->value_type = child2->value_type;
                    child1->target_type = child1->value_type;
                    child2->target_type = child1->value_type;
                    node->value_type = child1->value_type;
                    node->target_type = child1->value_type;
                    child1->symbolNode->symbol->type = child1->value_type;
                }
                else {
                    /* The Target Symbol has a type */
                    child1->value_type = child1->symbolNode->symbol->type;
                    child1->target_type = child1->symbolNode->symbol->type;
                    child2->target_type = child1->symbolNode->symbol->type;
                    node->value_type = child1->symbolNode->symbol->type;
                    node->target_type = child1->symbolNode->symbol->type;
                    child1->symbolNode->symbol->type = child1->value_type;
                }
                if (child2->node_type == CLASS) node->is_opt_arg = 0;
                else node->is_opt_arg = 1;
                if (child1->node_type == VAR_REFERENCE) node->is_ref_arg = 1;
                else node->is_ref_arg = 0;
                break;

            case ADDRESS:
            case SAY:
                if (child1) child1->target_type = TP_STRING;
                break;

            case RETURN:
                /* Type is the scope > procedure > type */
                node->value_type = context->current_scope->defining_node->value_type;
                node->target_type = node->value_type;
                if (node->value_type == TP_VOID) {
                    if (child1) mknd_err(child1, "EXTRANEOUS_RETVAL");
                }
                else {
                    if (child1) child1->target_type = node->value_type;
                    else mknd_err(node, "RETVAL_MISSING");
                }
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

            case UNTIL:
            case WHILE:
                if (child1) child1->target_type = TP_BOOLEAN;
                break;

            case LEAVE:
            case ITERATE:
                /* Link to relevant DO */
                if (node->child) {
                    /* Symbol specified - so we need to find it */
                    n1 = node->parent;
                    while (1) {
                        if (!n1) {
                            mknd_err(node, "INVALID_CONTROL_VARIABLE"); /* 28.3 */
                            break;
                        }
                        else if (n1->node_type == DO) {
                            /* Find the ASSIGN */
                            n2 = n1->child; /* REPEAT */
                            n2 = n2->child; /* First child of REPEAT */
                            while (n2) {
                                if (n2->node_type == ASSIGN) {
                                    /* Same Symbol? */
                                    if (n2->child->symbolNode->symbol ==
                                        node->child->symbolNode->symbol) {
                                        node->association = n1;
                                        goto found;
                                    }
                                    else break;
                                }
                                n2 = n2->sibling; /* Next REPEAT Child */
                            }
                        }
                        n1 = n1->parent;
                    };
                    found:;
                }
                else {
                    /* Symbol not specified - just find inner DO */
                    n1 = node->parent;
                    while (1) {
                        if (!n1) {
                            mknd_err(node, "NOT_IN_LOOP"); /* 28.1, 28.2 */
                            break;
                        }
                        else if (n1->node_type == DO) {
                            node->association = n1;
                            break;
                        }
                        n1 = n1->parent;
                    };
                }
                break;

            default:;
        }

        context->current_scope = node->scope;
    }

    return result_normal;
}

/* Fix up types for function arguments - needs to be done after the procedure
 * arguments have been processed in step 4 */
static walker_result step5_walker(walker_direction direction,
                                  ASTNode* node,
                                  void *payload) {

    Context *context = (Context*)payload;
    ASTNode *child1, *child2, *n1, *n2;
    ValueType max_type = TP_UNKNOWN;
    int arg_num;

    if (direction == in) {
        /* IN - TOP DOWN */
        context->current_scope = node->scope;
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

            case FUNCTION:
                /* Process all the arguments */
                n1 = node->child;
                n2 = sym_trnd(node->symbolNode->symbol, 0)->node;
                /* n2 is PROCEDURE. Go to the first arg */
                n2 = n2->child->sibling->child;

                /* Check each argument */
                arg_num = 0;
                while (n1) {
                    arg_num++;
                    if (!n2) {
                        mknd_err(n1, "UNEXPECTED_ARGUMENT %d", arg_num);
                        break;
                    }
                    n1->target_type = n2->value_type;
                    n1->is_opt_arg = n2->is_opt_arg;
                    if (n1->node_type == NOVAL) {
                        n1->value_type = n1->target_type;
                        if (!n1->is_opt_arg) {
                            mknd_err(n1, "ARGUMENT_REQUIRED %d %s", arg_num, n2->child->symbolNode->symbol->name);
                        }
                    }
                    if (n2->child->node_type == VAR_REFERENCE) {
                        n1->is_ref_arg = 1;
                        if (n1->symbolNode) {
                            if (n1->target_type != n1->value_type) {
                                /* Cannot change type of pass by reference symbol */
                                mknd_err(n1, "REFERENCE_TYPE_MISMATCH %d %s", arg_num, n2->child->symbolNode->symbol->name);
                            }
                            /* Mark as write access for the optimiser */
                            n1->symbolNode->writeUsage = 1;
                        }
                    }
                    n1 = n1->sibling;
                    n2 = n2->sibling;
                }
                while (n2) {
                    arg_num++;
                    n1 = ast_ft(context, NOVAL);
                    n1->target_type = n2->value_type;
                    n1->value_type = n1->target_type;
                    n1->is_opt_arg = n2->is_opt_arg;
                    add_ast(node, n1);
                    if (!n1->is_opt_arg) {
                        mknd_err(n1, "ARGUMENT_REQUIRED %d %s", arg_num, n2->child->symbolNode->symbol->name);
                    }
                    n2 = n2->sibling;
                }
                break;

            default:;
        }

        context->current_scope = node->scope;
    }

    return result_normal;
}

/* Validate AST */
void validate(Context *context) {
    Scope *current_scope;
    int ordinal_counter = 0;

    /* Step 1
     * - Sets the source start / finish for eac node
     * - Fixes SCONCAT to CONCAT
     * - Other AST fixups (TBC)
     */
    ast_wlkr(context->ast, step1_walker, (void *) context);

    /* 1b - set node ordinal values */
    ast_wlkr(context->ast, step1b_walker, (void *)&ordinal_counter);

    /* Step 2
     * - Builds the Symbol Table
     */
    /* Mainly build symbols - procedures, members */
    context->current_scope = 0;
    ast_wlkr(context->ast, step2a_walker, (void *) context);

    /* Mainly resolve symbols - function uses */
    context->current_scope = 0;
    ast_wlkr(context->ast, step2b_walker, (void *) context);

    /* Step 3
     * - Validate Symbols
     */
    validate_symbols(context->ast->scope);

    /* Step 4
     * - Type Safety
     */
    context->current_scope = 0;
    ast_wlkr(context->ast, step4_walker, (void *)context);

    /* Step 5
     * - Type Safety for function arguments
     */
    context->current_scope = 0;
    ast_wlkr(context->ast, step5_walker, (void *)context);

    // Free Instruction Database
    if (context->level == LEVELB) free_ops();
 }