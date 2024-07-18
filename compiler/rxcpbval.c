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

/* Helper function to compare node string value with a string
 * Used for builtin class names (int, float etc) - so don't need to worry
 * about utf
 * NOTE value MUST be in lower case! */
static int is_node_string(ASTNode* node, const char* value) {
    int i;
    /* If it is a different length it can't be the same! */
    if (strlen(value) != node->node_string_length) return 0;

    for (i=0; i < node->node_string_length; i++) {
        if (tolower(node->node_string[i]) != value[i]) return 0;
    }
    return 1;
}

/* Convert a node (i.e. type INTEGER) to an integer - no error correction as the lexer will have done that */
static int node_to_integer(ASTNode* node) {
    int result;
    char *buffer = malloc(node->node_string_length + 1);

    /* Null terminated buffer - {sigh} */
    buffer[node->node_string_length] = 0;
    memcpy(buffer, node->node_string, node->node_string_length);

    result = atoi(buffer);

    free(buffer);

    return result;
}

/*
 * Helper function to set dimensions based on a node
 * Sets dims to the number of dimensions (or 0 if not an array)
 *      dim_base and dim_elements are malloced and set as appropriately
 */
static void node_to_dims(ASTNode* node, size_t *dims, int** dim_base, int** dim_elements) {
    *dims = 0;
    int i;
    ASTNode* n;
    ASTNode* temp;
    ASTNode* min;
    ASTNode* max;

    if (*dim_base) free(*dim_base);
    *dim_base = 0;

    if (*dim_elements) free(*dim_elements);
    *dim_elements = 0;

    if (!node) return;

    *dims = ast_nchd(node);
    if (*dims) {
        *dim_base = malloc(sizeof(int) * (*dims));
        *dim_elements = malloc(sizeof(int) * (*dims));

        /* We are an array - determine the array bounds */
        if (node->node_type == CLASS) {
            /* Type definition - can have specific bounds */
            n = ast_chdn(node,0); /* n is a RANGE for that dimension */
            for (i = 0; i < *dims; i++) {
                /* Child 1 - n->child - is the base */
                min = ast_chdn(n,0);
                if (min->node_type == NOVAL) (*dim_base)[i] = 1;
                else if (min->node_type == OP_NEG) (*dim_base)[i] = -node_to_integer(ast_chdn(min, 0));
                else (*dim_base)[i] = node_to_integer(min);

                /* Child 2 - n->child->sibling - is the max index value (which we convert to number of elements) */
                max = ast_chdn(n,1);
                if (max->node_type == NOVAL) (*dim_elements)[i] = 0; /* Infinity */

                else if ((*dim_base)[i] == 1 && max->node_type == INTEGER && node_to_integer(max) == 0)
                    (*dim_elements)[i] = 0; /* Syntax candy to make 0 Infinity for base 1 */

                else {
                    if (max->node_type == OP_NEG)
                        (*dim_elements)[i] =
                                -node_to_integer(ast_chdn(max, 0)) - (*dim_base)[i] + 1;
                    else (*dim_elements)[i] = node_to_integer(max) - (*dim_base)[i] + 1;

                    if ((*dim_elements)[i] < 1) {
                        if (max->node_type == OP_NEG) {
                            /* One child expected for the INTEGER otherwise an error MUST have been added already */
                            if (ast_nchd(max) == 1) mknd_err(max, "LESS_THAN_BASE");
                        } else {
                            /* No child expected otherwise an error MUST have been added already */
                            if (ast_nchd(max) == 0) mknd_err(max, "LESS_THAN_BASE");
                        }
                    }
                }

                n = ast_nsib(n);
            }
        } else {
            /* Implicit definition - default bounds */
            for (i = 0; i < *dims; i++) {
                (*dim_base)[i] = 1;
                (*dim_elements)[i] = 0; /* Infinity */
            }
        }
    }
}

/*
 * Helper function to set the arrays
 * Sets dims to the number of dimensions (or 0 if not an array)
 *      class_name is set to either zero of to a class name (if not an in built class)
 *                 So if it is not zero it needs to be free()d
 *      dim_base and dim_elements are malloced and set as appropriately
 */
static ValueType node_to_type(ASTNode *node, size_t *dims, int **dim_base, int **dim_elements, char **class_name) {
    int i;
    ASTNode *n;
    ASTNode* min;
    ASTNode* max;

    *dims = 0;

    if (*class_name) free(*class_name);
    *class_name = 0;

    if (*dim_base) free(*dim_base);
    *dim_base = 0;

    if (*dim_elements) free(*dim_elements);
    *dim_elements = 0;

    if (!node) return TP_VOID;

    if (node->value_type != TP_UNKNOWN) {
        /* The Node Type has already been determined */
        *dims = node->value_dims;

        if (*dims) {
            *dim_base = malloc(sizeof(int) * (*dims));
            memcpy(*dim_base, node->value_dim_base, sizeof(int) * (*dims));

            *dim_elements = malloc(sizeof(int) * (*dims));
            memcpy(*dim_elements, node->value_dim_elements, sizeof(int) * (*dims));
        }

        if (node->value_class) {
            *class_name = malloc(strlen(node->value_class) + 1);
            strcpy(*class_name, node->value_class);
        }
        return node->value_type;
    }

    /* If we don't let's see if we can determine it now */
    if (node->node_type == CLASS) {
        /* Class and Class Arrays */
        *dims = ast_nchd(node);
        if (*dims) {
            *dim_base = malloc(sizeof(int) * (*dims));
            *dim_elements = malloc(sizeof(int) * (*dims));

            /* We are an array - determine the array bounds */
            /* Type definition - can have specific bounds */
            /* n is a RANGE for that dimension */
            n = ast_chdn(node, 0);
            for (i = 0; i < *dims; i++) {

                /* Child 1 - n->child - is the base */
                min = ast_chdn(n, 0);
                if (min->node_type == NOVAL) (*dim_base)[i] = 1;
                else if (min->node_type == OP_NEG) (*dim_base)[i] = -node_to_integer(ast_chdn(min, 0));
                else (*dim_base)[i] = node_to_integer(min);

                /* Child 2 - n->child->sibling - is the max index value (which we convert to number of elements) */
                max = ast_chdn(n, 1);
                if (max->node_type == NOVAL) (*dim_elements)[i] = 0; /* Infinity */

                else if ((*dim_base)[i] == 1 && max->node_type == INTEGER && node_to_integer(max) == 0)
                    (*dim_elements)[i] = 0; /* Syntax candy to make 0 Infinity for base 1 */

                else {
                    if (max->node_type == OP_NEG)
                        (*dim_elements)[i] =
                                -node_to_integer(ast_chdn(max, 0)) - (*dim_base)[i] + 1;
                    else (*dim_elements)[i] = node_to_integer(max) - (*dim_base)[i] + 1;

                    if ((*dim_elements)[i] < 1) {
                        if (max->node_type == OP_NEG) {
                            /* One child expected for the INTEGER otherwise an error MUST have been added already */
                            if (ast_nchd(max) == 1) mknd_err(max, "LESS_THAN_BASE");
                        } else {
                            /* No child expected otherwise an error MUST have been added already */
                            if (ast_nchd(max) == 0) mknd_err(max, "LESS_THAN_BASE");
                        }
                    }
                }
                n = ast_nsib(n);
            }
        }

        /* Work out the base type */
        if (is_node_string(node, ".void")) return TP_VOID;
        if (is_node_string(node, ".int")) return TP_INTEGER;
        if (is_node_string(node, ".float")) return TP_FLOAT;
        if (is_node_string(node, ".string")) return TP_STRING;
        if (is_node_string(node, ".binary")) return TP_BINARY;
        if (is_node_string(node, ".boolean")) return TP_BOOLEAN;
        *class_name = malloc(node->node_string_length + 1);
        memcpy(*class_name, node->node_string, node->node_string_length);
        (*class_name)[node->node_string_length] = 0;
        return TP_OBJECT;
    }

    /* Otherwise leaf types */
    switch (node->node_type) {
        case FLOAT:
            return TP_FLOAT;
        case INTEGER:
            return TP_INTEGER;
        case STRING:
            return TP_STRING;
        case BINARY:
            return TP_BINARY;
        case VOID:
            return TP_VOID;
        default:
            return TP_UNKNOWN;
    }
}

/* Validates a node promotion is correct adding error nodes if not */
static void validate_node_promotion(ASTNode* node) {
    size_t i;
    if (node->target_type == TP_UNKNOWN) return; /* Can't validate yet - will be done later after the target is set */
    if (node->value_type == TP_UNKNOWN) return; /* Can't validate yet - will be done later after the value is set */

    /* Ignore error nodes */
    if (node->node_type == ERROR) return;
    if (node->node_type == WARNING) return;

    if (node->value_dims != node->target_dims) {
        if (!node->value_dims) mknd_err(node, "EXPECTING_ARRAY");
        else if (!node->target_dims)
            mknd_err(node, "UNEXPECTED_ARRAY");
        else mknd_err(node, "ARRAY_DIMS_MISMATCH");
    }
    else if (node->value_dims) {
        /* Check Dimension base/values */
        for (i = 0; i<node->value_dims; i++) {
            if (node->value_dim_base[i] != node->target_dim_base[i])
                mknd_err(node, "INCOMPATIBLE_DIM_BASE dim=%d from=%d to=%d",
                         i + 1, node->value_dim_base[i], node->target_dim_base[i]);
            else if (node->value_dim_elements[i] != node->target_dim_elements[i] && node->target_dim_elements[i])
                mknd_err(node, "INCOMPATIBLE_DIM_SIZE dim=%d from=%d to=%d",
                         i + 1, node->value_dim_elements[i], node->target_dim_elements[i]);
        }
    }

    if (node->value_dims && node->value_type != node->target_type) mknd_err(node, "ARRAY_ELEMENT_TYPE_MISMATCH");

    if (node->value_type == TP_VOID && node->target_type != TP_VOID) mknd_err(node, "MISSING_VALUE");

    /* Binary cant to cast */
    if (node->value_type != node->target_type &&
        node->value_type == TP_BINARY &&
        node->target_type != TP_BINARY) mknd_err(node, "CANNOT_CAST_BINARY");

    if (node->value_type != TP_VOID && node->target_type == TP_VOID) mknd_err(node, "UNEXPECTED_VALUE");

    /* TODO Class / Object Support */
    if (node->value_type == TP_OBJECT || node->target_type == TP_OBJECT)
        mknd_err(node, "CLASSES_NOT_SUPPORTED");
}

/* Validates a node promotion is correct for a call by reference (of symbols) adding error nodes if not */
static void validate_node_promotion_for_ref(ASTNode* node) {
    size_t i;

    if (node->target_type == TP_UNKNOWN || node->value_type == TP_UNKNOWN) {
        mknd_err(node, "UNKNOWN_TYPE");
        return;
    }

    /* Ignore error nodes */
    if (node->node_type == ERROR) return;
    if (node->node_type == WARNING) return;

    if (node->value_dims != node->target_dims) mknd_err(node, "REFERENCE_TYPE_MISMATCH");
    else if (node->value_dims) {
        /* Check Dimension base/values */
        for (i = 0; i<node->value_dims; i++) {
            if (node->value_dim_base[i] != node->target_dim_base[i]) mknd_err(node, "REFERENCE_TYPE_MISMATCH");
            else if (node->value_dim_elements[i] != node->target_dim_elements[i]) mknd_err(node, "REFERENCE_TYPE_MISMATCH");
        }
    }

    if (node->value_type != node->target_type) mknd_err(node, "REFERENCE_TYPE_MISMATCH");

    /* TODO Class / Object Support */
    if (node->value_type == TP_OBJECT || node->target_type == TP_OBJECT)
        mknd_err(node, "CLASSES_NOT_SUPPORTED");
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

static walker_result initial_checks_walker(walker_direction direction,
                                           ASTNode* node, __attribute__((unused)) void *payload) {

    ASTNode *child, *next_child, *new_child, *next, *last, *n;
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
            if (node->child->sibling) {
                child = node->child->sibling->child; /* first child of INSTRUCTIONS */
                node->child->sibling = child; /* So REXX_OPTIONS sibling is the first instruction */
                while (child) {
                    /* Fix the parent reference */
                    child->parent = node;
                    child = child->sibling;
                }
            }
            else return result_normal; /* No instructions at all! */

            if (node->child->sibling->node_type != PROCEDURE) {
                /* If the first instruction is not a PROCEDURE then we need to
                * add an implicit "main" PROCEDURE */
                child = ast_ftt(context, PROCEDURE, "main:");
                child->parent = node;
                child->sibling = node->child->sibling;
                node->child->sibling = child;

                /* Add function return type */
                /* To work out the return type we walk the tree from here until we find the first return or a procedure */
                n = child->sibling;
                while (1) {
                    if (!n) {
                        /* No return statement so must be returning null */
                        add_ast(child,ast_ft(context, VOID));
                        break;
                    }
                    if (n->node_type == PROCEDURE) {
                        /* No return statement so must be returning null */
                        add_ast(child,ast_ft(context, VOID));
                        break;
                    }
                    if (n->node_type == RETURN) {
                        if (n->child) add_ast(child,ast_ftt(context, CLASS, ".int")); /* Must return an int */
                        else add_ast(child,ast_ft(context, VOID)); /* No return value - returning void */
                        break;
                    }

                    /* Find the next node to look at */
                    if (n->child) n = n->child;
                    else if (n->sibling) n = n->sibling;
                    else {
                        n = n->parent;
                        while (n) {
                            if (n == child->sibling->parent) n = 0; /* end of sub-tree */
                            else if (n->sibling) {
                                n = n->sibling;
                                break;
                            }
                            else n = n->parent;
                        }
                    }
                }
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
                        } else if (!first_instruction) {
                            /* Error - arg must be the first statement */
                            mknd_err(next, "ARG_NOT_FIRST_INST");
                        }
                        args_node = next;

                        /* Disconnect/remove node from the AST tree */
                        node->sibling = next->sibling;
                        next->sibling = 0;
                        next->parent = 0;
                        /* And add under the procedure */
                        add_ast(node, next);
                        next = node->sibling;
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
                size_t i;
                node->node_string = node->file_name;
                node->node_string_length = strlen(node->file_name);
                for (i = 0; i < node->node_string_length; i++) {
                    if (node->node_string[i] == '.' || node->node_string[i] == ' ') {
                        node->node_string_length = i;
                        break;
                    }
                }
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
                    if (has_by) mknd_err(child, "INVALID_DO");
                    else has_by = 1;
                }
                else if (child->node_type == FOR) {
                    if (has_for) mknd_err(child, "INVALID_DO");
                    else has_for = 1;
                }
                else if (child->node_type == TO) {
                        if (has_to) mknd_err(child, "INVALID_DO");
                        else has_to = 1;
                }
                child = child->sibling;
            }
            if (has_assign && !has_by) {
                /* Need to add implicit "BY" node - to avoid an infinite loop! */
                add_ast(node, ast_ft(context, BY));
            }
        }

        else if (node->node_type == ADDRESS) {
            /* Validate Sub-commands - Error 27.1 */
            ASTNode* input_node = 0;
            ASTNode* output_node = 0;
            ASTNode* error_node = 0;
            ASTNode* expose_node = 0;
            ASTNode* bad_nodes = 0;

            /* Collate all the redirects */
            child = node->child->sibling->sibling; /* Child1 = environment, child2 = command */
            while (child) {
                ASTNode* next_child = child->sibling; /* Before we do any rewrites! */
                ast_del(child); /* We are going to place the node back into the right place later */
                child->parent = node; /* We need to keep the link to the parent */
                if (child->node_type == REDIRECT_IN) {
                    if (input_node) {
                        /* Duplicate - mark error */
                        mknd_err(child, "DUPLICATE");
                        /* And chain to bad_nodes */
                        child->sibling = bad_nodes;
                        bad_nodes = child;
                    }
                    else input_node = child;
                }
                else if (child->node_type == REDIRECT_OUT) {
                    if (output_node) {
                        /* Duplicate - mark error */
                        mknd_err(child, "DUPLICATE");
                        /* And chain to bad_nodes */
                        child->sibling = bad_nodes;
                        bad_nodes = child;
                    }
                    else output_node = child;
                }
                else if (child->node_type == REDIRECT_ERROR) {
                    if (error_node) {
                        /* Duplicate - mark error */
                        mknd_err(child, "DUPLICATE");
                        /* And chain to bad_nodes */
                        child->sibling = bad_nodes;
                        bad_nodes = child;
                    }
                    else error_node = child;
                }
                else if (child->node_type == REDIRECT_EXPOSE) {
                    if (expose_node) {
                        /* Duplicate - mark error */
                        mknd_err(child, "DUPLICATE");
                        /* And chain to bad_nodes */
                        child->sibling = bad_nodes;
                        bad_nodes = child;
                    }
                    else expose_node = child;
                }
                child = next_child;
            }

            /* Now add the nodes in the right order */
            child = node->child; /* Child1 = environment - make it a STRING (from a LITERAL) */
            if (child->node_type != NOVAL) child->node_type = STRING;

            child = child->sibling; /* child2 = command */

            /* Input */
            if (input_node) child->sibling = input_node;
            else {
                add_ast(
                        add_sbtr(child,ast_ft(context, REDIRECT_IN)),
                        ast_ft(context, NOVAL)
                );
            }
            child = child->sibling;

            /* Output */
            if (output_node) child->sibling = output_node;
            else {
                add_ast(
                        add_sbtr(child,ast_ft(context, REDIRECT_OUT)),
                        ast_ft(context, NOVAL)
                );
            }
            child = child->sibling;

            /* Error */
            if (error_node) child->sibling = error_node;
            else {
                add_ast(
                        add_sbtr(child,ast_ft(context, REDIRECT_ERROR)),
                        ast_ft(context, NOVAL)
                );
            }
            child = child->sibling;

            /* Exposed Variables */
            if (expose_node) child->sibling = expose_node;
            else {
                add_sbtr(child,ast_ft(context, REDIRECT_EXPOSE));
             }
            child = child->sibling;

            /* Errors / Junk */
            child->sibling = bad_nodes;
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

        /* OP_ARG_VALUE to OP_ARGS Rewrites */
        else if (node->node_type == OP_ARG_VALUE) {
            if (node->child && ( (node->child->node_type == NOVAL) ||
                                 (node->child->node_type == INTEGER && node_to_integer(node->child) == 0)) ) {
                ast_del(node->child);
                node->node_type = OP_ARGS;
            }
        }

    }
    return result_normal;
}

/*
 * Converts EXIT Instruction to _exit System Function
 */
static walker_result rewrite_exit_walker(walker_direction direction,
                                            ASTNode* node, __attribute__((unused)) void *payload) {

    Context *context = (Context*)payload;

    ASTNode* args_node;
    ASTNode* function_node;
    ASTNode* temp_node;
    ASTNode* current_child;
    ASTNode* last_child;
    ASTNode* next_child;
    ASTNode* var_name;

    if (direction == out) {
        /* Bottom Up */
        switch (node->node_type) {

            case EXIT:
                /* Rewrite to call of _exit */

                /* Assignment node and remember the command */
                node->node_type = CALL;

                /* Function */
                function_node = ast_ft(context, FUNCTION);
                ast_str(function_node, "_exit");
                /* Fix up position for error messages */
                function_node->column = node->column;
                function_node->line = node->line;

                /* Move Param(s) */
                args_node = node->child;
                while (args_node) {
                    ast_del(args_node);
                    add_ast(function_node, args_node);
                    args_node = node->child;
                }

                /* Add Function */
                add_ast(node, function_node);
                break;

            default: ;
        }
    }

    return result_normal;
}

/*
 * Converts ADDRESS Instruction to _address and redirect system functions
 */
static walker_result rewrite_address_walker(walker_direction direction,
                                            ASTNode* node, __attribute__((unused)) void *payload) {

    Context *context = (Context*)payload;

    ASTNode* args_node;
    ASTNode* function_node;
    ASTNode* temp_node;
    ASTNode* current_child;
    ASTNode* last_child;
    ASTNode* next_child;
    ASTNode* var_name;

    if (direction == out) {
        /* Bottom Up */
        switch (node->node_type) {

            case ADDRESS:
                /* Rewrite to an assignment from a function */

                /* Assignment node and remember the command */
                node->node_type = ASSIGN;

                /* Function */
                function_node = ast_ft(context, FUNCTION);
                ast_str(function_node, "_address");
                /* Fix up position for error messages */
                function_node->column = node->column;
                function_node->line = node->line;

                /* Move Params */
                args_node = node->child;
                while (args_node) {
                    ast_del(args_node);
                    add_ast(function_node,args_node);
                    args_node = node->child;
                }

                /* rc is the target */
                temp_node = ast_ft(context, VAR_TARGET);
                ast_str(temp_node, "rc");
                add_ast(node,temp_node);

                /* Add Function */
                add_ast(node,function_node);
                break;

            case REDIRECT_IN:
                if (node->child->value_type == TP_VOID) {
                    /* Just remove the node and convert to a noredir function */
                    ast_del(node->child);
                    node->node_type = FUNCTION;
                    ast_str(node, "_noredir");
                }
                else if (node->child->value_dims) {
                    /* Array Redirect */
                    node->node_type = FUNCTION;
                    ast_str(node, "_array2redir");
                }
                else {
                    /* String Redirect */
                    node->node_type = FUNCTION;
                    ast_str(node, "_string2redir");
                }
                break;

            case REDIRECT_OUT:
            case REDIRECT_ERROR:
                if (node->child->value_type == TP_VOID) {
                    /* Just remove the node and convert to a noredir function */
                    ast_del(node->child);
                    node->node_type = FUNCTION;
                    ast_str(node, "_noredir");
                }
                else if (node->child->value_dims) {
                    /* Array Redirect */
                    node->node_type = FUNCTION;
                    ast_str(node, "_redir2array");
                }
                else {
                    /* String Redirect */
                    node->node_type = FUNCTION;
                    ast_str(node, "_redir2string");
                }
                break;

            case REDIRECT_EXPOSE:
                /* Replace this node with its children (if any)
                 * Each child turns into a string (name of variable) followed by the variable itself */
                if (node->child) {
                    last_child = ast_chdn(node->parent, ast_chdi(node) - 1); /* node's older sibling */
                    current_child = node->child;
                    next_child = current_child->sibling;
                    while (current_child) {
                        /* Link in new string */
                        var_name = ast_fstk(context, current_child);
                        var_name->node_type = STRING;
                        var_name->parent = node->parent;
                        last_child->sibling = var_name;

                        /* Link in current_child - as VAR_SYMBOL */
                        current_child->parent = node->parent;
                        var_name->sibling = current_child;

                        /* Next child */
                        last_child = current_child;
                        current_child = next_child;
                        if (current_child) next_child = current_child->sibling;
                        else next_child = 0;
                    }
                    /* Link to the next node */
                    last_child->sibling = node->sibling;

                    /* Remove NODE safely */
                    node->sibling = 0;
                    node->child = 0;
                    ast_del(node);
                }
                else {
                    /* No children / environment variables - delete node */
                    ast_del(node);
                }
                break;

            default:;
        }
    }

    return result_normal;
}

/*
 * Adds rxsysb if needed
 */
static walker_result add_rxsysb_walker(walker_direction direction,
                                       ASTNode* node, __attribute__((unused)) void *payload) {

    Context *context = (Context*)payload;

    ASTNode* _rxsysb_import_node;
    ASTNode* _rxsysb_node;

    if (direction == out) {
        /* Bottom Up */
        if (node->node_type == REXX_OPTIONS) {
            if (context->need_rxsysb && !context->has_rxsysb) {
                /* we need to import _rxsysb */
                _rxsysb_import_node = ast_ft(context, IMPORT);
                add_ast(node,_rxsysb_import_node);

                _rxsysb_node = ast_ft(context, LITERAL);
                ast_str(_rxsysb_node, "_rxsysb");
                add_ast(_rxsysb_import_node,_rxsysb_node);
            }
        }

        else if (node->node_type == IMPORT) {
            /* Have we imported _rxsysb already */
            if (node->child && is_node_string(node->child, "_rxsysb")) context->has_rxsysb = 1;
        }

        else if (node->node_type == NAMESPACE) {
            /* Have we imported _rxsysb already */
            if (node->child && is_node_string(node->child, "_rxsysb")) context->has_rxsysb = 1;
        }
    }

    return result_normal;
}

/*
 * `Sees if rxsysb is needed
 * - If ADDRESS is used
 */
static walker_result needs_rxsysb_walker(walker_direction direction,
                                       ASTNode* node, __attribute__((unused)) void *payload) {

    Context *context = (Context*)payload;

    if (direction == out) {
        /* Bottom Up */
        if (node->node_type == ADDRESS) {
            context->need_rxsysb = 1;
        }
        else if (node->node_type == EXIT) {
            context->need_rxsysb = 1;
        }
    }

    return result_normal;
}

/* Step 1b
 * - Set node ordinal values
 */
static walker_result set_node_ordinals_walker(walker_direction direction,
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
static walker_result build_symbols_walker(walker_direction direction,
                                          ASTNode* node,
                                          void *payload) {

    Context *context = (Context*)payload;
    Symbol *symbol;
    ASTNode *n;

    if (direction == in) {

        /* IN - TOP DOWN */
        if (node->scope) {
            context->current_scope = node->scope;
            return result_normal; /* Skip if we have already processed this node */
        }

        if (node->node_type == REXX_UNIVERSE) {
            /* This top level scope will contain the project file scope & imported file scopes */
            context->current_scope = scp_f(context->current_scope, node, 0);
            node->scope = context->current_scope;
        }

        else if (node->node_type == PROGRAM_FILE) {
            /* Now create the namespace symbol and scope */
            /* Make the new symbol */
            symbol = sym_f(context->current_scope, node);
            symbol->symbol_type = NAMESPACE_SYMBOL;
            sym_adnd(symbol, node, 1, 1);
            if (context->namespace) sym_adnd(symbol, context->namespace, 0, 1);

            /* Move down to the project file scope */
            context->current_scope = scp_f(context->current_scope, node, symbol);
            node->scope = context->current_scope;
        }

        else if (node->node_type == PROCEDURE) {
            node->node_string_length--; /* Remove the ':' */

            /* Set the return value node value_type */
            n = ast_chld(node, CLASS, VOID);
            n->value_type = node_to_type(n,
                                         &(n->value_dims), &(n->value_dim_base), &(n->value_dim_elements),
                                         &(n->value_class));

            /* Reset node Target Type to be the same as the node Value Type */
            ast_rttp(n);

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
            context->current_scope = scp_f(context->current_scope, node, symbol);
            node->scope = context->current_scope;
        }

        else if (node->node_type == IMPORT) {
            /* Get the toplevel scope that contains all the namespaces */
            Scope* namespaces = context->ast->scope;

            /* Now create the imported namespace symbol and scope */
            /* Make the new symbol */
            symbol = sym_f(namespaces, node->child);
            if (symbol) {
                symbol->symbol_type = NAMESPACE_SYMBOL;
                sym_adnd(symbol, node->child, 0, 1);

                /* New scope scope */
                context->current_scope = scp_f(namespaces, node->child, symbol);
                node->scope = context->current_scope;
            }
            else {
                mknd_err(node->child, "DUPLICATE_NAMESPACE");
                node->scope = namespaces;
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
            else if (node->parent->node_type == DEFINE) {
                mknd_err(node, "ALREADY_DECLARED");
            }

            /* Set Argument flags - set by the parser grammar */
            if (node->parent->node_type == ARG) {
                symbol->is_arg = 1;
                symbol->is_opt_arg = node->parent->is_opt_arg;
                symbol->is_ref_arg = node->parent->is_ref_arg;
                /* Add the count of fixed args for the procedure symbol */
                ast_proc(node)->symbolNode->symbol->fixed_args++;
            }

            if (node->parent->node_type == ASSIGN) sym_adnd(symbol, node, 0, 1);
            else sym_adnd(symbol, node, 0, 0);
        }

        else if (node->node_type == VARG || node->node_type == VARG_REFERENCE) {
            /* Set the procedure symbol has_vargs flag */
            ast_proc(node)->symbolNode->symbol->has_vargs = 1;
        }

        else if (node->node_type == OP_ARG_EXISTS) {
            node->scope = context->current_scope;
            /* Find the symbol */
            symbol = sym_rslv(context->current_scope, node);

            /* At this point the arguments will have been processed so no need to attempt to add a symbol */
            if (!symbol) {
                mknd_err(node, "UNKNOWN_SYMBOL");
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
            else if (!symbol->is_arg) {
                mknd_err(node, "NOT_AN_ARGUMENT");
            }
            else if (!symbol->is_opt_arg) {
                /* It's not options, so it must always exist (be specified in the call) */
                node->node_type = INTEGER;
                node->value_type = TP_BOOLEAN;
                node->int_value = 1;
                node->node_string = "1";
                node->node_string_length = 1;
            }
            else sym_adnd(symbol, node, 0, 0);
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
        /* OUT - BOTTOM UP */
        if (node->parent) context->current_scope = node->parent->scope;
        else context->current_scope = 0;
    }

    return result_normal;
}

/* Step 2b
 * - Resolve Function Symbols
 */
static walker_result resolve_functions_walker(walker_direction direction,
                                              ASTNode* node,
                                              void *payload) {

    Context *context = (Context*)payload;
    Symbol *symbol;

    if (direction == in) {
        /* IN - TOP DOWN */
    }
    else {
        if (node->node_type == FUNCTION) {
            if (node->symbolNode) return result_normal; /* Already Processed */
            if (ast_chld(node,ERROR,0)) return result_normal; /* Has an error - already processed */

            /* Find the symbol */
            symbol = sym_rvfc(context->ast, node);

            /* If there is a symbol and it's a function - found  */
            if (symbol && symbol->symbol_type == FUNCTION_SYMBOL ) {
                sym_adnd(symbol, node, 1, 0);
            }

            /* If there is a symbol (and it's not a function) - error */
            else if (symbol) {
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

/* Step 2c
 * - Resolve Exposed Namespace Symbols
 */
static walker_result exposed_symbols_walker(walker_direction direction,
                                            ASTNode* node,
                                            void *payload) {

    Context *context = (Context*)payload;
    Symbol *symbol, *merged_symbol;
    ASTNode *temp_node, *proc_node;
    int found;

    if (direction == in) {
        /* IN - TOP DOWN */
        if (node->node_type == EXPOSED) {
            if (node->parent->node_type == NAMESPACE) {
                /* We are exposing functions / variables globally to user functions */
                ASTNode* n = node->child;
                while (n) {
                    symbol = sym_rvfc(context->ast, n); /* Is this a procedure? */
                    if (!symbol) {
                        /* Procedure Symbol not found so it "must" be a variable we need to expose from our procedures */
                        found = 0;
                        /* We need to loop through all the procedures in the program_file in order */
                        for (proc_node = context->ast->child->child; proc_node; proc_node = proc_node->sibling) {
                            if (proc_node->node_type != PROCEDURE) continue;

                            /* We might be exposing one of the procedure's variables */
                            symbol = sym_lrsv(proc_node->scope, n); /* find it */
                            if (symbol && symbol->symbol_type == VARIABLE_SYMBOL && !symbol->is_arg) {
                                if (symbol->exposed == 0) { /* Avoid double processing */
                                    /* We found a variable to expose - so expose it by moving its scope */
                                    merged_symbol = sym_merg(symbol->scope->parent, symbol);
                                    /* Link to the exposed node */
                                    sym_adnd(merged_symbol, n, 1, 1);
                                    /* Link to the Procedure's INSTRUCTION node */
                                    sym_adnd(merged_symbol, ast_chld(proc_node, INSTRUCTIONS, NOP), 0, 0);
                                    merged_symbol->exposed = 1;
                                }
                                found = 1;
                            }
                        }
                        if (!found) {
                            /* Add an error - if it has not already errored */
                            if (ast_chld(n, ERROR, 0) == 0)
                                mknd_err(n, "UNKNOWN_SYMBOL");
                        }
                    }
                    else if (symbol->symbol_type ==  FUNCTION_SYMBOL) {
                        temp_node = sym_proc(symbol); /* Procedure */
                        temp_node = ast_chld(temp_node, INSTRUCTIONS, NOP); /* Instructions */
                        if (temp_node && temp_node->node_type == INSTRUCTIONS) {
                            if (symbol->exposed == 0) {
                                /* Link and expose - if not already processed */
                                sym_adnd(symbol, n, 1, 1);
                                symbol->exposed = 1;
                            }
                        }
                        else {
                            /* Add an error - if it has not already errored */
                            if (ast_chld(n, ERROR, 0) == 0)
                                mknd_err(n, "IMPORTED_FUNCTION");
                        }
                    }
                    else if (symbol->symbol_type ==  VARIABLE_SYMBOL) {
                        /* Add a warning - if it has not already errored/warned */
                        if (!context->after_rewrite && ast_chld(n, ERROR, WARNING) == 0)
                            mknd_war(n, "DUPLICATE_SYMBOL");
                    }
                    else {
                        /* Add an error - if it has not already errored */
                        if (ast_chld(n, ERROR, 0) == 0)
                            mknd_err(n, "INVALID_SYMBOL_TYPE");
                    }
                    n = n->sibling;
                }
            }

            else if (node->parent->node_type == PROCEDURE) {
                /* We are exposing variables in a procedure */
                ASTNode* n = node->child;
                while (n) {
                    /* Check if it is a global symbol (already) */
                    symbol = sym_rvfc(context->ast, n); /* Is this is a procedure/function? */
                    if (!symbol) {
                        /* It is not global yet, so we should be exposing one of the procedure's variables */
                        symbol = sym_lrsv(node->parent->scope, n); /* find it */
                        if (symbol && symbol->symbol_type == VARIABLE_SYMBOL && !symbol->is_arg) {
                            if (symbol->exposed == 0) { /* Avoid double processing */
                                /* We found a variable to expose - so expose it by moving its scope */
                                merged_symbol = sym_merg(symbol->scope->parent, symbol);
                                /* Link to the exposed node */
                                sym_adnd(merged_symbol, n, 1, 1);
                                /* Link to the Procedure's INSTRUCTION node */
                                sym_adnd(merged_symbol, ast_chld(node->parent, INSTRUCTIONS, NOP), 0, 0);
                                merged_symbol->exposed = 1;
                            }
                        }
                        else {
                            /* Add an error - if it has not already errored */
                            if (ast_chld(n, ERROR, 0) == 0) {
                                if (symbol->is_arg) mknd_err(n, "CANNOT_EXPOSED_ARG");
                                else mknd_err(n, "UNKNOWN_SYMBOL");
                            }
                        }
                    }

                    else if (symbol->symbol_type ==  VARIABLE_SYMBOL) {
                        /* Already a global symbol - does it exist in the procedure? */
                        Symbol *proc_symbol = sym_lrsv(node->parent->scope, n);
                        if (proc_symbol) {
                            if (proc_symbol->is_arg) {
                                /* If it is an arg it can't b e exposed */
                                /* Add an error - if it has not already errored */
                                if (ast_chld(n, ERROR, 0) == 0)
                                    mknd_err(n, "CANNOT_EXPOSED_ARG");
                            }
                            else {
                                /* Expose it */
                                if (proc_symbol->exposed == 0) { /* Avoid double processing */
                                    /* We need to promote this symbol to the parent scope */
                                    merged_symbol = sym_merg(proc_symbol->scope->parent, proc_symbol);
                                    /* Link to the exposed node */
                                    sym_adnd(merged_symbol, n, 1, 1);
                                    /* Link to the Procedure's INSTRUCTION node */
                                    sym_adnd(merged_symbol, ast_chld(node->parent, INSTRUCTIONS, NOP), 0, 0);
                                    merged_symbol->exposed = 1;
                                }
                            }
                        }
                        else {
                            /* Either we have already processed this symbol (duplicate) or it is not used in the proc at all */

                            if (symislnk(ast_chld(node->parent, INSTRUCTIONS, NOP), symbol)) {
                                /* It's linked to the procedure's instructions - therefore a duplicate */
                                /* Add a warning - if it has not already errored/warned */
                                if (!context->after_rewrite && ast_chld(n, ERROR, WARNING) == 0)
                                    mknd_war(n, "DUPLICATE_SYMBOL");
                            }
                            else {
                                /* Must be unused */
                                /* Add an error - if it has not already errored */
                                if (ast_chld(n, ERROR, 0) == 0)
                                    mknd_err(n, "UNKNOWN_SYMBOL");
                            }
                        }
                    }

                    else {
                        /* Add an error - if it has not already errored */
                        if (ast_chld(n, ERROR, 0) == 0)
                            mknd_err(n, "INVALID_SYMBOL_TYPE");
                    }

                    n = n->sibling;
                }
            }
        }
    }

    return result_normal;
}

/*
 * Step 3 - Validate Symbols
 */

/* This is called for every symbol */
static void validate_symbol_in_scope(Symbol *symbol, void *payload) {
    Scope* scope = (Scope*)payload;
    SymbolNode *defining_node_link;
    ASTNode *proc, *p, *p_type;
    char *buffer;
    size_t length;
    size_t i, nix;

    if (symbol->type != TP_UNKNOWN) return; /* Already processed */

    /* Process special symbols */
    if (strcmp(symbol->name,"rc") == 0) {
        symbol->type = TP_INTEGER;
        symbol->symbol_type = VARIABLE_SYMBOL;
        symbol->value_dims = 0;
        return;
    }

    /* This sees if we are looking at exposed (global) variables by looking at the scope defining node type */
    if ( scope->defining_node->node_type == PROGRAM_FILE && symbol->symbol_type == VARIABLE_SYMBOL) {

        proc = 0;
        /* An Exposed Symbol needs special processing */

        /* Looking at every node using this symbol */
        for (nix = 0; nix < sym_nond(symbol); nix++) {

            /* We want to find the first usage of the symbol in each procedure it is used in */
            defining_node_link = sym_trnd(symbol, nix);

            p = ast_proc(defining_node_link->node);
            if (p == proc) continue; /* We have already looked at this proc so skip */
            proc = p;

            /* Ok we are the first usage in this proc */
            if (defining_node_link->node->node_type == PROCEDURE) {
                p_type = ast_chld(defining_node_link->node, CLASS, VOID);
                symbol->type = node_to_type(p_type,
                                            &(symbol->value_dims), &(symbol->dim_base), &(symbol->dim_elements),
                                            &(symbol->value_class));

                ast_svtp(defining_node_link->node, symbol);

                ast_svtp(p_type, symbol);
                break;
            }

            if (defining_node_link->node->node_type == VAR_REFERENCE) {
                symbol->type = node_to_type(defining_node_link->node->sibling,
                                            &(symbol->value_dims), &(symbol->dim_base), &(symbol->dim_elements),
                                            &(symbol->value_class));
                ast_svtp(defining_node_link->node, symbol);
                ast_svtn(defining_node_link->node->parent, defining_node_link->node);
                break;
            }

            if (defining_node_link->node->node_type == VAR_TARGET) {
                symbol->type = node_to_type(defining_node_link->node->sibling,
                                            &(symbol->value_dims), &(symbol->dim_base), &(symbol->dim_elements),
                                            &(symbol->value_class));

                /* The dimensions can be defined on the left-hand side (lhs) or rhs but not both and not if the rhs is a class */
                /* node_to_type() above has checked the rhs - so now we look at the lhs */
                if (symbol->value_dims == 0 && defining_node_link->node->sibling->node_type != CLASS)
                    node_to_dims(defining_node_link->node, &(symbol->value_dims),
                                 &(symbol->dim_base), &(symbol->dim_elements));

                ast_svtp(defining_node_link->node, symbol);
                ast_svtn(defining_node_link->node->parent, defining_node_link->node);
                break;
            }

            /* That's it - a global/exposed symbol can't be a taken constant so the symbol type is defined in
             * a following procedure */
        }
        /* Search other modules */
        sym_imva(scope->defining_node->context, symbol);
    }
    else {

        /* For REXX Level B the variable type is defined by its first use */
        defining_node_link = sym_trnd(symbol, 0);
        if (defining_node_link->node->node_type == PROCEDURE) {
            /* This sets the procedure symbol type */
            p_type = ast_chld(defining_node_link->node, CLASS, VOID);
            symbol->type = node_to_type(p_type,
                                        &(symbol->value_dims), &(symbol->dim_base), &(symbol->dim_elements),
                                        &(symbol->value_class));

            ast_svtp(defining_node_link->node, symbol);
            ast_svtp(p_type, symbol);
        }

        else if (defining_node_link->node->node_type == VAR_REFERENCE) {
            /* This set the variable type */
            symbol->type = node_to_type(defining_node_link->node->sibling,
                                        &(symbol->value_dims), &(symbol->dim_base), &(symbol->dim_elements),
                                        &(symbol->value_class));
            ast_svtp(defining_node_link->node, symbol);
            ast_svtn(defining_node_link->node->parent, defining_node_link->node);
        }

        else if (defining_node_link->node->node_type == VAR_TARGET) {
            /* This set the variable type */
            symbol->type = node_to_type(defining_node_link->node->sibling,
                                        &(symbol->value_dims), &(symbol->dim_base), &(symbol->dim_elements),
                                        &(symbol->value_class));

            /* The dimensions can be defined on the left-hand side (lhs) or rhs but not both and not if the rhs is a class */
            /* node_to_type() above has checked the rhs - so now we look at the lhs */
            if (symbol->value_dims == 0 && defining_node_link->node->sibling->node_type != CLASS)
                node_to_dims(defining_node_link->node, &(symbol->value_dims),
                             &(symbol->dim_base), &(symbol->dim_elements));

            ast_svtp(defining_node_link->node, symbol);
            ast_svtn(defining_node_link->node->parent, defining_node_link->node);
        }

        else if (symbol->symbol_type != NAMESPACE_SYMBOL) {
            /* Used without definition/declaration - Taken Constant */
            /* TODO - for Level A/C/D we will need flow analysis to determine taken constant status */
            symbol->type = TP_STRING;
            symbol->symbol_type = CONSTANT_SYMBOL;
            symbol->value_dims = 0;
            /* Update all the attached AST Nodes to be constants */
            for (i = 0; i < sym_nond(symbol); i++) {
                defining_node_link = sym_trnd(symbol, i);
                if (defining_node_link->writeUsage) {
                    /* This means we are trying to write to a TAKEN CONSTANT
                     * which is illegal in Levels B/G/L */
                    mknd_err(defining_node_link->node, "UPDATING_TAKEN_CONSTANT");
                }
                defining_node_link->node->node_type = STRING;
                defining_node_link->node->value_dims = 0;
                length = strlen(symbol->name);
                buffer = malloc(length);
                memcpy(buffer, symbol->name, length);
                ast_sstr(defining_node_link->node, buffer, length);
            }
        }
    }
}

static void validate_symbols(Scope* scope) {
    int i;
    if (!scope) return;

    scp_4all(scope, validate_symbol_in_scope, scope);
    for (i=0; i < scp_noch(scope); i++) {
        validate_symbols(scp_chd(scope, i));
    }
}

/* Step 4
 * - Type Safety
 */

/* Type promotion matrix for numeric operators */
static const ValueType promotion[8][8] = {
/*                  TP_UNKNOWN, TP_VOID,    TP_BOOLEAN, TP_INTEGER, TP_FLOAT,   TP_STRING,  TP_BINARY,   TP_OBJECT */

/* TP_UNKNOWN */ {TP_UNKNOWN, TP_UNKNOWN, TP_UNKNOWN, TP_UNKNOWN, TP_UNKNOWN, TP_UNKNOWN, TP_UNKNOWN,  TP_UNKNOWN},
/* TP_VOID    */ {TP_UNKNOWN, TP_VOID,    TP_BOOLEAN, TP_INTEGER, TP_FLOAT,   TP_FLOAT,   TP_BINARY,   TP_OBJECT},
/* TP_BOOLEAN */ {TP_UNKNOWN, TP_BOOLEAN, TP_BOOLEAN, TP_INTEGER, TP_FLOAT,   TP_BOOLEAN, TP_UNKNOWN,  TP_OBJECT},
/* TP_INTEGER */ {TP_UNKNOWN, TP_INTEGER, TP_INTEGER, TP_INTEGER, TP_FLOAT,   TP_INTEGER, TP_UNKNOWN,  TP_OBJECT},
/* TP_FLOAT   */ {TP_UNKNOWN, TP_FLOAT,   TP_FLOAT,   TP_FLOAT,   TP_FLOAT,   TP_FLOAT,   TP_UNKNOWN,  TP_OBJECT},
/* TP_STRING  */ {TP_UNKNOWN, TP_FLOAT,   TP_BOOLEAN, TP_INTEGER, TP_FLOAT,   TP_FLOAT,   TP_UNKNOWN,  TP_OBJECT},
/* TP_BINARY  */ {TP_UNKNOWN, TP_BINARY,  TP_UNKNOWN, TP_UNKNOWN, TP_UNKNOWN, TP_UNKNOWN, TP_UNKNOWN,  TP_OBJECT},
/* TP_OBJECT  */ {TP_UNKNOWN, TP_OBJECT,  TP_OBJECT,  TP_OBJECT,  TP_OBJECT,  TP_OBJECT,  TP_OBJECT,   TP_OBJECT},
};

/* Returns the value_type of a node - arrays changes to TP_OBJECT */
static ValueType node_type(ASTNode* node) {
    if (node->value_dims) return TP_OBJECT;
    return node->value_type;
}

/* Returns the highest value_type of the node's children nodes */
static ValueType max_type(ASTNode* node) {
    ASTNode *child;
    ValueType max_type = TP_UNKNOWN;

    child = node->child;
    while (child) {
        if (child->value_type > max_type) max_type = node_type(child);
        child = child->sibling;
    }

    return max_type;
}

/* Set the node value and target type to a simple type (not an array or class name) */
static void set_node_type(ASTNode* node, ValueType type) {
    node->value_type = type;
    node->value_dims = 0;
    if (node->value_class) {
        free(node->value_class);
        node->value_class = 0;
    }
    node->target_type = type;
    node->target_dims = 0;
    if (node->target_class) {
        free(node->target_class);
        node->target_class = 0;
    }
}

/* Set the target value to a simple target_type (not an array or class name)
 * and validates that it is convertable from the nodes value target_type */
static void set_node_target_type(ASTNode* node, ValueType target_type) {
    node->target_type = target_type;
    node->target_dims = 0;
    if (node->target_class) {
        free(node->target_class);
        node->target_class = 0;
    }

    validate_node_promotion(node);
}

/* This walker does the basic value types of operations
 * No errors generated - just simple "guesses" as to types */
static walker_result set_node_types_walker(walker_direction direction,
                                           ASTNode* node,
                                           void *payload) {

    Context *context = (Context*)payload;
    ASTNode *child1, *child2, *n1, *n2;
    int val, ix;

    if (direction == in) {
        /* IN - TOP DOWN */
        context->current_scope = node->scope;
    }
    else {
        /* OUT - BOTTOM UP */
        child1 = ast_chdn(node, 0);
        child2 = ast_chdn(node, 1);

        switch (node->node_type) {

            case OP_AND:
            case OP_OR:
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
            case OP_ARG_EXISTS:
            case OP_ARG_IX_EXISTS:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, TP_BOOLEAN);
                }
                break;

            case OP_CONCAT:
            case OP_SCONCAT:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, TP_STRING);
                }
                break;

            case OP_ADD:
            case OP_MINUS:
            case OP_MULT:
            case OP_POWER:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, promotion[child1->value_type][child2->value_type]);
                }
                break;

            case OP_DIV:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, TP_FLOAT);
                }
                break;

            case OP_IDIV:
            case OP_MOD:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, TP_INTEGER);
                }
                break;

            case OP_NOT:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, TP_BOOLEAN);
                }
                break;

            case OP_PLUS:
            case OP_NEG:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, promotion[child1->value_type][TP_VOID]);
                }
                break;

            case FUNCTION:
                if (node->symbolNode) { /* Otherwise, an error node will have been added */
                    if (node->value_type == TP_UNKNOWN) {
                        context->changed = 1;
                        ast_svtp(node, node->symbolNode->symbol);
                    }
                }
                break;

            case VAR_SYMBOL:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;

                    ast_svtp(node, node->symbolNode->symbol);
                    if (child1) {
                        /* We have array parameters */
                        n1 = child1;
                        while (n1) {
                            if (n1->value_type == TP_VOID && !ast_nsib(n1)) {
                                /* The last parameter is VOID - this is a special case, we
                                 * are returning the number of elements as an integer */
                                break;
                            }

                            if (n1->node_type == INTEGER && !ast_nsib(n1) &&
                                node->symbolNode->symbol->dim_base &&
                                node->symbolNode->symbol->dim_base[ast_chdi(n1)] == 1 &&
                                node_to_integer(n1) == 0) {
                                /* Special case - last parameter and 1-base and is "0"
                                 * this is a syntax candy for VOID - returning the number of elements as an integer */
                                n1->value_type = TP_VOID;
                                n1->node_type = NOVAL;
                                break;
                            }

                            n1 = ast_nsib(n1);
                        }

                        if (n1 && n1->value_type == TP_VOID) {
                            /* The last parameter is VOID - this is a special case, we
                             * are returning the number of elements as an integer */
                            set_node_type(node, TP_INTEGER);
                        } else {
                            /* We are returning the array element */
                            node->value_dims = 0; /* We are 'returning' a single value */
                            /* Reset Node Target Type to be the same as the node value type */
                            ast_rttp(node);
                        }
                    }
                }
                break;

            case ASSIGN:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    if (child1->symbolNode->symbol->type == TP_UNKNOWN) {
                        /* If the symbol does not have a known type yet - then determine it */
                        if (node->parent->node_type == REPEAT) {
                            /* Special logic for LOOP Assignment - type must be numeric */
                            child1->symbolNode->symbol->value_dims = 0;
                            if (child1->symbolNode->symbol->value_class) free(child1->symbolNode->symbol->value_class);
                            child1->symbolNode->symbol->value_class = 0;
                            child1->symbolNode->symbol->type = promotion[child2->value_type][TP_INTEGER];
                        } else {
                            child1->symbolNode->symbol->type =
                                    node_to_type(child2,
                                                 &(child1->symbolNode->symbol->value_dims),
                                                 &(child1->symbolNode->symbol->dim_base),
                                                 &(child1->symbolNode->symbol->dim_elements),
                                                 &(child1->symbolNode->symbol->value_class));

                            if (child1->symbolNode->symbol->value_dims == 0 && child2->node_type != CLASS)
                                node_to_dims(child1, &(child1->symbolNode->symbol->value_dims),
                                             &(child1->symbolNode->symbol->dim_base), &(child1->symbolNode->symbol->dim_elements));

                        }
                    }
                }
                break;

            case CONST_SYMBOL:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, TP_STRING);
                }
                break;

            case FLOAT:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, TP_FLOAT);
                }
                break;

            case INTEGER:
            case OP_ARGS:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, TP_INTEGER);
                }
                break;

            case STRING:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, TP_STRING);
                }
                break;

            case NOVAL:
            case RANGE:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    set_node_type(node, TP_VOID);
                }
                break;

            case CLASS:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;

                    node->value_type = node_to_type(node, &(node->value_dims),
                                                    &(node->value_dim_base), &(node->value_dim_elements),
                                                    &(node->value_class));

                    /* Reset Node Target Type to be the same as the node value type */
                    ast_rttp(node);
                }
                break;

            case DEFINE:
                if (node->value_type == TP_UNKNOWN) {
                    context->changed = 1;
                    if (child1->symbolNode->symbol->type == TP_UNKNOWN) {
                        /* If the symbol does not have a known type yet - then determine it */
                        child1->symbolNode->symbol->type =
                                node_to_type(child2,
                                             &(child1->symbolNode->symbol->value_dims),
                                             &(child1->symbolNode->symbol->dim_base),
                                             &(child1->symbolNode->symbol->dim_elements),
                                             &(child1->symbolNode->symbol->value_class));
                        ast_svtp(child1, child1->symbolNode->symbol);
                    }
                }
                break;

            case ARG:
                if (node->value_type == TP_UNKNOWN) {
                    if (node->child->node_type == VARG || node->child->node_type == VARG_REFERENCE) {
                        /* Ellipse */
                        context->changed = 1;
                        child1->value_type = node_to_type(child2,
                                                          &(child1->value_dims),
                                                          &(child1->value_dim_base),
                                                          &(child1->value_dim_elements),
                                                          &(child1->value_class));
                        ast_rttp(child1);
                        ast_svtn(node, child1);
                    }
                    else {
                        /* Normal Arg */
                        if (child1->symbolNode) {
                            context->changed = 1;
                            if (child1->symbolNode->symbol->type == TP_UNKNOWN) {
                                /* If the symbol does not have a known type yet */
                                child1->symbolNode->symbol->type = node_to_type(child2,
                                                                                &(child1->symbolNode->symbol->value_dims),
                                                                                &(child1->symbolNode->symbol->dim_base),
                                                                                &(child1->symbolNode->symbol->dim_elements),
                                                                                &(child1->symbolNode->symbol->value_class));
                            }
                            ast_svtp(child1, child1->symbolNode->symbol);
                            ast_svtn(node, child1);
                        }
                    }
                }
                break;

            case OP_ARG_VALUE:
                if (node->value_type == TP_UNKNOWN) {
                    /* Find the procedure ellipse type node */
                    n1 = ast_proc(node); /* Procedure node */
                    n1 = ast_chld(n1, ARGS, 0); /* ARGS Node */
                    if (ast_nchd(n1) == 0) break; /* No ARGS at all! */
                    n1 = ast_chdn(n1, ast_nchd(n1) - 1); /* Last ARG */
                    n1 = n1->child; /* The VARG */
                    if (n1->node_type != VARG) break; /* No VARGS */

                    n1 = n1->sibling; /* This is the CLASS of the VARGS */
                    ast_svtn(node, n1);
                    context->changed = 1;
                }
                break;

            default:;
        }

        context->current_scope = node->scope;
    }

    return result_normal;
}

static walker_result type_safety_walker(walker_direction direction,
                                        ASTNode* node,
                                        void *payload) {

    Context *context = (Context*)payload;
    ASTNode *child1, *child2, *n1, *n2;
    int val, ix;

    if (direction == in) {
        /* IN - TOP DOWN */
        context->current_scope = node->scope;
    }
    else {
        /* OUT - BOTTOM UP */
        child1 = ast_chdn(node, 0);
        child2 = ast_chdn(node, 1);

        switch (node->node_type) {
            case PROCEDURE:
                if (strcmp(node->symbolNode->symbol->name,"main") == 0) {
                    /* Validate main() return values */
                    if (node->value_type != TP_VOID && node->value_type != TP_INTEGER) {
                        /* Must be an string array */
                        mknd_err(ast_chld(node,CLASS,0),"MAIN_RETURNS_INTEGER");
                    }
                }
                break;

            case OP_AND:
            case OP_OR:
                set_node_target_type(child1, TP_BOOLEAN);
                set_node_target_type(child2, TP_BOOLEAN);
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
                set_node_target_type(child1, max_type(node));
                set_node_target_type(child2, max_type(node));
                break;

            case OP_CONCAT:
            case OP_SCONCAT:
                set_node_target_type(child1, node->value_type);
                set_node_target_type(child2, node->value_type);
                break;

            case OP_ADD:
            case OP_MINUS:
            case OP_MULT:
            case OP_POWER:
                set_node_target_type(child1, node->value_type);
                set_node_target_type(child2, node->value_type);
                break;

            case OP_DIV:
                set_node_target_type(child1, node->value_type);
                set_node_target_type(child2, node->value_type);
                break;

            case OP_IDIV:
            case OP_MOD:
                set_node_target_type(child1, node->value_type);
                set_node_target_type(child2, node->value_type);
                break;

            case OP_NOT:
                set_node_target_type(child1, node->value_type);
                break;

            case OP_PLUS:
            case OP_NEG:
                set_node_type(node, promotion[child1->value_type][TP_VOID]);
                set_node_target_type(child1, node->value_type);
                break;

            case VAR_SYMBOL:
                if (node->value_type == TP_UNKNOWN) ast_svtp(node, node->symbolNode->symbol);
                if (node->value_type == TP_UNKNOWN) mknd_err(node, "UNKNOWN_TYPE");

                if (ast_nchd(node) && !node->symbolNode->symbol->value_dims) {
                    mknd_err(node, "NOT_AN_ARRAY");
                }
                else if (child1) {
                    /* We have array parameters */

                    /* Set array parameter type to integer */
                    n1 = child1;
                    while (n1) {
                        if (n1->value_type == TP_VOID && !ast_nsib(n1)) {
                            /* The last parameter is VOID - this is a special case, we
                             * are returning the number of elements as an integer */
                            break;
                        }

                        set_node_target_type(n1, TP_INTEGER);

                        if (n1->node_type == INTEGER) {
                            /* As a constant integer we can check it is in range */
                            val = node_to_integer(n1);
                            ix = ast_chdi(n1);

                            if (val < n1->parent->symbolNode->symbol->dim_base[ix])
                                mknd_err(n1, "OUT_OF_RANGE");

                            else if (n1->parent->symbolNode->symbol->dim_elements[ix]) {
                                /* There is a max number of elements - so check it */
                                if (val > n1->parent->symbolNode->symbol->dim_base[ix] +
                                                      n1->parent->symbolNode->symbol->dim_elements[ix] - 1)
                                    mknd_err(n1, "OUT_OF_RANGE");
                            }
                        }

                        n1 = ast_nsib(n1);
                    }

                    if (n1 && n1->value_type == TP_VOID) {
                        /* The last parameter is VOID - this is a special case, we
                         * are returning the number of elements as an integer */

                        /* We can have fewer parameters when we are getting the number of elements */
                        if (node->symbolNode->symbol->value_dims < ast_nchd(node))
                            mknd_err(node, "ARRAY_DIMS_MISMATCH");
                    }

                    else {
                        /* We are returning the array element */
                        if (node->symbolNode->symbol->value_dims != ast_nchd(node))
                            mknd_err(node, "ARRAY_DIMS_MISMATCH");
                    }
                }
                break;

            case CLASS:
                if (node->value_dims) {
                    /* We are an array */
                    if (node->parent->node_type == PROCEDURE) {
                        /* In a procedure definition the array params should be null */
                        n1 = child1;
                        while (n1) {
                            set_node_target_type(n1, TP_VOID);
                            n1 = ast_nsib(n1);
                        }
                    }
                }
                // else we are a class TODO

                break;

            case DEFINE:
                if (child1->symbolNode->symbol->type == TP_UNKNOWN) {
                    /* If the symbol does not have a known type yet - then determine it */
                    child1->symbolNode->symbol->type =
                            node_to_type(child2,
                                         &(child1->symbolNode->symbol->value_dims),
                                         &(child1->symbolNode->symbol->dim_base),
                                         &(child1->symbolNode->symbol->dim_elements),
                                         &(child1->symbolNode->symbol->value_class));
                    ast_svtp(child1, child1->symbolNode->symbol);

                    if (child1->symbolNode->symbol->type == TP_UNKNOWN) mknd_err(node, "UNKNOWN_TYPE");
                }

                if (ast_nchd(child1)) {
                    /* We have unexpected array parameters */
                    mknd_err(ast_chdn(child1,0), "INVALID_LHS_ARRAY");
                }

                ast_sttn(child2, child1);
                validate_node_promotion(child2);
                ast_svtn(node, child1);
                break;

            case ASSIGN:
                if (child2->value_type == TP_VOID) {
                    mknd_err(child2, "RETURNS_VOID");
                }
                else {
                    if (child1->symbolNode->symbol->type == TP_UNKNOWN) {
                        /* If the symbol does not have a known type yet - then determine it */
                        child1->symbolNode->symbol->type =
                                node_to_type(child2,
                                             &(child1->symbolNode->symbol->value_dims),
                                             &(child1->symbolNode->symbol->dim_base),
                                             &(child1->symbolNode->symbol->dim_elements),
                                             &(child1->symbolNode->symbol->value_class));

                        if (child1->symbolNode->symbol->value_dims == 0 && child2->node_type != CLASS)
                            node_to_dims(child1, &(child1->symbolNode->symbol->value_dims),
                                         &(child1->symbolNode->symbol->dim_base), &(child1->symbolNode->symbol->dim_elements));
                    }
                    ast_svtp(child1, child1->symbolNode->symbol);

                    if (child1->symbolNode->symbol->type == TP_UNKNOWN) mknd_err(node, "UNKNOWN_TYPE");

                    if (ast_nchd(child1)) {
                        /* We have array parameters */
                        /* Set array parameter type to integer */
                        n1 = child1->child;
                        while (n1) {
                            set_node_target_type(n1, TP_INTEGER);

                            if (n1->node_type == INTEGER) {
                                /* As a constant integer we can check it is in range */
                                val = node_to_integer(n1);
                                ix = ast_chdi(n1);

                                if (ix < child1->value_dims) {
                                    if (val < n1->parent->symbolNode->symbol->dim_base[ix])
                                        mknd_err(n1, "OUT_OF_RANGE");

                                    else if (n1->parent->symbolNode->symbol->dim_elements[ix]) {
                                        /* There is a max number of elements - so check it */
                                        if (val > n1->parent->symbolNode->symbol->dim_base[ix] +
                                                  n1->parent->symbolNode->symbol->dim_elements[ix] - 1)
                                            mknd_err(n1, "OUT_OF_RANGE");
                                    }
                                }
                            }

                            n1 = n1->sibling;
                        }

                        if (!child1->value_dims)
                            mknd_err(child1, "NOT_AN_ARRAY");
                        else if (child1->value_dims != ast_nchd(child1)) mknd_err(node, "ARRAY_DIMS_MISMATCH");

                        child1->value_dims = 0; /* We are a single value */
                        child1->target_dims = 0;
                    }

                    ast_sttn(child2, child1);
                    validate_node_promotion(child2);
                    ast_svtn(node, child1);
                }
                break;

            case ARGS:
                if (strcmp(node->parent->symbolNode->symbol->name,"main") == 0) {
                    /* Validate the signature of the main() functions */
                    if (ast_nchd(node) == 0) break; /* A main() can ignore arguments */
                    if (ast_nchd(node) != 1) {
                        /* Should only have 1 argument */
                        mknd_err(node,"INVALID_MAIN_ARGS");
                        break;
                    }
                    if (child1->value_dims != 1) {
                        /* Must be a 1 dimensional array */
                        mknd_err(node,"INVALID_MAIN_ARGS");
                        break;
                    }
                    if (child1->value_dim_elements[0] != 0 ) {
                        /* Must be a dynamic array */
                        mknd_err(node,"INVALID_MAIN_ARGS");
                        break;
                    }
                    if (child1->value_type != TP_STRING ) {
                        /* Must be an string array */
                        mknd_err(node,"INVALID_MAIN_ARGS");
                        break;
                    }
                }
                break;

            case ARG:
                if (ast_chld(node->parent->parent, INSTRUCTIONS, NOP)->node_type == INSTRUCTIONS) {
                    /* In a function implementation - in this case the optional flag '?' is invalid for a class type as a
                     * definition needs to know the default value that can only be defined by the expression on the
                     * right-hand-side */
                    if (child2->node_type == CLASS && node->is_opt_arg && !node->value_dims) {
                        /* Optional but the CLASS doesn't give the needed default value */
                        /* NOTE Arrays are an exception - their default value is a "blank" array */
                        mknd_err(node, "NO_DEFAULT_VALUE");
                    }
                }
                break;

            case OP_ARG_VALUE:
            case OP_ARG_IX_EXISTS:
                if (!ast_proc(node)->symbolNode->symbol->has_vargs) mknd_err(node,"NO_PROC_VARGS");
                set_node_target_type(child1,TP_INTEGER);

                if (child1->node_type == INTEGER) {
                    /* As a constant integer we can check it is in range
                     * In fact this never works as the node will be OP_NEG (but leaving it here in case it is needed in the future) */
                    val = node_to_integer(child1);
                    if (val < 1)
                        mknd_err(child1, "OUT_OF_RANGE");
                }
                break;

            case OP_ARGS:
                if (!ast_proc(node)->symbolNode->symbol->has_vargs) mknd_err(node,"NO_PROC_VARGS");
                break;

//            case ADDRESS:
            case SAY:
                if (child1) set_node_target_type(child1, TP_STRING);
                break;

            case RETURN:
                /* Type is the scope > procedure > type */
                ast_svtp(node, context->current_scope->defining_node->symbolNode->symbol);
                if (node->value_type == TP_VOID) {
                    if (child1) mknd_err(child1, "EXTRANEOUS_RETVAL");
                }
                else {
                    if (child1) {
                        ast_sttn(child1, node);
                        validate_node_promotion(child1);
                    }
                    else mknd_err(node, "RETVAL_MISSING");
                }
                break;

            case IF:
                if (child1) set_node_target_type(child1, TP_BOOLEAN);
                break;

                /* Loops */
            case TO:
            case BY:
                /* The TO/BY value type needs to be the same as the assigment type */
                if (child1) set_node_target_type(child1, node->parent->child->value_type);
                set_node_type(node, node->parent->child->value_type);
                break;

            case FOR:
                set_node_target_type(child1, TP_INTEGER);
                set_node_type(node, TP_INTEGER);
                break;

            case UNTIL:
            case WHILE:
                if (child1) set_node_target_type(child1, TP_BOOLEAN);
                set_node_type(node, TP_BOOLEAN);
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
                    }
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
                    }
                }
                break;


            case STRING:
                if (ast_nchd(node)) mknd_err(ast_chdn(node,0), "TAKENCONSTANT_ARRAY");
                break;

            default:;
        }

        context->current_scope = node->scope;
    }

    return result_normal;
}

/* Fix up types for function arguments and OP_ARG_VALUE nodes
 * Needs to be done after the procedure arguments have been processed */
static walker_result func_type_safety_walker(walker_direction direction,
                                             ASTNode* node,
                                             void *payload) {

    Context *context = (Context*)payload;
    ASTNode *n1, *n2;
    int arg_num;

    if (direction == in) {
        /* IN - TOP DOWN */
        context->current_scope = node->scope;
    }
    else {
        /* OUT - BOTTOM UP */
        switch (node->node_type) {

            case FUNCTION:
                /* Process all the arguments */
                n1 = node->child;
                if  (node->symbolNode) {
                    n2 = sym_trnd(node->symbolNode->symbol, 0)->node;
                    /* n2 is PROCEDURE. Go to the first arg */
                    n2 = ast_chld(n2, ARGS, 0);
                    n2 = n2->child;
                }
                else n2 = 0;
                /* Check each argument */
                arg_num = 0;
                while (n1) {
                    arg_num++;
                    if (!n2) {
                        /* Its not an error for the first NOVAL argument */
                        if (arg_num > 1 || n1->node_type != NOVAL) mknd_err(n1, "UNEXPECTED_ARGUMENT, %d", arg_num);
                        else if (n1->node_type == NOVAL) {
                            /* Prune the unwanted NOVAL - the parser grammar just added it */
                            ast_del(n1);
                        }
                        break;
                    }

                    if (n2->child->node_type == VARG || n2->child->node_type == VARG_REFERENCE) {
                        /* Last ellipsis */
                        n1->is_opt_arg = 0;

                        if (n1->node_type == NOVAL) {
                            if (n1->sibling) {
                                /* If n1 is not the last argument then it can't be NOVAL */
                                mknd_err(n1, "ARGUMENT_REQUIRED, %d, \"...\"", arg_num);
                            }
                            else {
                                /* Prune the unwanted NOVAL - the parser grammar just added it */
                                ast_del(n1);
                            }
                        } else {
                            ast_sttn(n1, n2);
                            validate_node_promotion(n1);


                            if (n2->child->node_type == VAR_REFERENCE) {
                                n1->is_ref_arg = 1;
                                if (n1->symbolNode) {
                                    validate_node_promotion_for_ref(n1);

                                    /* Mark as write access for the optimiser */
                                    n1->symbolNode->writeUsage = 1;
                                }
                            }
                        }
                    }

                    else {
                        /* Normal Argument */
                        n1->is_opt_arg = n2->is_opt_arg;
                        if (n1->node_type == NOVAL) {
                            ast_svtn(n1, n2);
                            if (!n1->is_opt_arg) {
                                mknd_err(n1, "ARGUMENT_REQUIRED, %d, \"%s\"", arg_num,
                                         n2->child->symbolNode->symbol->name);
                            }
                        } else {
                            ast_sttn(n1, n2);
                            validate_node_promotion(n1);
                        }

                        if (n2->child->node_type == VAR_REFERENCE) {
                            n1->is_ref_arg = 1;
                            if (n1->symbolNode) {
                                validate_node_promotion_for_ref(n1);

                                /* Mark as write access for the optimiser */
                                n1->symbolNode->writeUsage = 1;
                            }
                        }
                        n2 = n2->sibling;
                    }
                    n1 = n1->sibling;
                }

                /* Skip an ellipse */
                if (n2 && (n2->child->node_type == VARG || n2->child->node_type == VARG_REFERENCE)) n2 = n2->sibling;

                while (n2) {
                    arg_num++;
                    n1 = ast_ft(context, NOVAL);
                    ast_svtn(n1,n2);
                    add_ast(node, n1);
                    n1->is_opt_arg = n2->is_opt_arg;
                    n1->is_ref_arg = n2->is_ref_arg;
                    if (!n1->is_opt_arg) {
                        mknd_err(n1, "ARGUMENT_REQUIRED, %d, \"%s\"", arg_num, n2->child->symbolNode->symbol->name);
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
void rxcp_val(Context *context) {
    int ordinal_counter;

    /* AST fixups */
    context->current_scope = 0;
    ast_wlkr(context->ast, initial_checks_walker, (void *) context);

    /* Adds rxsysb library (e.g. for ADDRESS and EXIT) */
    context->current_scope = 0;
    context->need_rxsysb = 0;
    ast_wlkr(context->ast, needs_rxsysb_walker, (void *) context);
    if (context->need_rxsysb) {
        context->current_scope = 0;
        context->has_rxsysb = 0;
        ast_wlkr(context->ast, add_rxsysb_walker, (void *) context);
    }

    /* Builds the Symbol Table */
    context->current_scope = 0;
    ast_wlkr(context->ast, build_symbols_walker, (void *) context);

    /* Mainly resolve symbols - functions */
    context->current_scope = 0;
    ast_wlkr(context->ast, resolve_functions_walker, (void *) context);

    /* Resolve exposed symbols */
    context->current_scope = 0;
    ast_wlkr(context->ast, exposed_symbols_walker, (void *) context);

    /* Validate Symbols */
    validate_symbols(context->ast->scope);

    /* Set Node Types */
    context->current_scope = 0;
    ast_wlkr(context->ast, set_node_types_walker, (void *) context);

    /* Re-write ADDRESS Instructions */
    context->current_scope = 0;
    ast_wlkr(context->ast, rewrite_address_walker, (void *) context);

    /* Re-write EXIT Instructions */
    context->current_scope = 0;
    ast_wlkr(context->ast, rewrite_exit_walker, (void *) context);

    context->after_rewrite = 1; /* Incremental update of symbols - So walkers can avoid duplicate processing */

    ordinal_counter = 0;
    ast_wlkr(context->ast, set_node_ordinals_walker, (void *) &ordinal_counter);
    context->current_scope = 0;
    ast_wlkr(context->ast, build_symbols_walker, (void *) context);
    context->current_scope = 0;
    ast_wlkr(context->ast, resolve_functions_walker, (void *) context);
    context->current_scope = 0;
    ast_wlkr(context->ast, exposed_symbols_walker, (void *) context);
    validate_symbols(context->ast->scope);
    context->current_scope = 0;
    ast_wlkr(context->ast, set_node_types_walker, (void *) context);

    /* Type Safety checks */
    context->current_scope = 0;
    ast_wlkr(context->ast, type_safety_walker, (void *)context);

    /* Type Safety for function arguments */
    context->current_scope = 0;
    ast_wlkr(context->ast, func_type_safety_walker, (void *)context);
 }

/* Basic validation for an AST (typically the AST will be attached to a main AST as part of
 * function import) */
void rxcp_bvl(Context *context) {
    int ordinal_counter = 0;

    /* Step 1
     * - Sets the source start / finish for eac node
     * - Fixes SCONCAT to CONCAT
     * - Other AST fixups (TBC)
     */
    ast_wlkr(context->ast, initial_checks_walker, (void *) context);

    /* 1b - set node ordinal values */
    ast_wlkr(context->ast, set_node_ordinals_walker, (void *) &ordinal_counter);

    /* Step 2
     * - Builds the Symbol Table
     */
    /* Mainly build symbols - procedures, members */
    context->current_scope = 0;
    ast_wlkr(context->ast, build_symbols_walker, (void *) context);
}