/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * Validation Pass: Semantic Checks
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "rxcp_val.h"

/* Get the assembler operandtype from the astnode type for the ASSEMBLER instruction */
static OperandType nodetype_to_operandtype(NodeType ntype) {
    switch (ntype) {
        case INTEGER: return OP_INT;
        case FLOAT: return OP_FLOAT;
        case DECIMAL:  return OP_DECIMAL;
        case STRING: return OP_STRING;
        case FUNC_SYMBOL: return OP_FUNC;
        default: return OP_REG;
    }
}

/* Step 1
 * - Fixes up procedure / class tree structures.
 *   Note: This stage is critical for Level B Rexx as the single-lookahead Lemon parser
 *   initially produces a flat AST for routines and their bodies. This walker restructures
 *   the AST into a hierarchical form where ARGS and INSTRUCTIONS are children of the
 *   PROCEDURE node. It essentially fixes parsing weaknesses of the single lookahead parser.
 * - Sets the token and source start / finish position for each node
 * - Fixes SCONCAT to CONCAT
 * - Removes excess NOPs
 * - Process OPTIONS
 * - Validate REPEAT BY/FOR/DO
 * - Validate ASSEMBLER instructions
 */


/*
 * ast_structure_fixup_walker
 * - Fixes up procedure / class tree structures.
 */
walker_result ast_structure_fixup_walker(walker_direction direction,
                                         ASTNode* node, __attribute__((unused)) void *payload) {
    ASTNode *child, *new_child, *next, *last, *n;
    Context *context = (Context*)payload;

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

            if (node->child->sibling && node->child->sibling->node_type != PROCEDURE && node->child->sibling->node_type != CLASS_DEF) {
                /* If the first instruction is not a PROCEDURE or CLASS, then we need to
                * add an implicit "main" PROCEDURE */
                child = ast_ftt(context, PROCEDURE, "main:");
                child->parent = node;
                child->sibling = node->child->sibling;
                node->child->sibling = child;

                /* Add a function return type */
                /* To work out the return type we walk the tree from here until we find the first return, a procedure or a class */
                n = child->sibling;
                while (1) {
                    if (!n) {
                        /* No return statement so must be returning null */
                        add_ast(child,ast_ft(context, VOID));
                        break;
                    }
                    if (n->node_type == PROCEDURE || n->node_type == CLASS_DEF) {
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
        else if (node->node_type == CLASS_DEF) {
            if (node->parent->node_type != PROGRAM_FILE) {
                mknd_err(node, "CANT_DEFINE_CLASS_HERE");
            }
            else {
                /* Hoist siblings until next PROCEDURE or CLASS_DEF */
                while ( ((next = node->sibling)) && next->node_type != PROCEDURE && next->node_type != CLASS_DEF) {
                    /* Disconnect/remove next node from the AST tree */
                    node->sibling = next->sibling;
                    next->sibling = 0;
                    next->parent = 0;
                    /* add next under CLASS_DEF child */
                    add_ast(node,next);
                }
            }
        }
        else if (node->node_type == NODE_REGISTER) {
            ASTNode *index = node->child;
            /* Handle INSTRUCTIONS wrapper if present */
            while (index && index->node_type == INSTRUCTIONS) index = index->child;
            if (index && index->node_type == INTEGER) {
                /* Validate index >= 0 */
                int idx = node_to_integer(index);
                node->int_value = idx;
                if (idx < 0) {
                    mknd_err(index, "REGISTER_INDEX_OUT_OF_RANGE");
                }
            }
            ASTNode *attr = index ? index->sibling : NULL;
            if (attr && attr->node_type == VAR_SYMBOL) {
                if (!nodeis(attr, "int") &&
                    !nodeis(attr, "string") &&
                    !nodeis(attr, "object") &&
                    !nodeis(attr, "float")) {
                    mknd_err(attr, "INVALID_REGISTER_ATTRIBUTE");
                }
            }
        }
        else if (node->node_type == PROCEDURE || node->node_type == METHOD || node->node_type == FACTORY) {
            if (node->node_type == PROCEDURE && node->parent->node_type != PROGRAM_FILE) {
                mknd_err(node, "CANT_DEFINE_PROC_HERE");
            }
            else if ((node->node_type == METHOD || node->node_type == FACTORY) && node->parent->node_type != CLASS_DEF) {
                mknd_err(node, "CANT_DEFINE_METHOD_HERE");
            }
            else {
                /* Move node siblings (aka next instructions) until the next procedure to be node
                 * grand-children under a new INSTRUCTIONS node child */

                /* Process ARG, DIGITS, FUZZ and FORM */
                /* Process each sibling until the next block */
                char done_digits = 0;
                char done_fuzz = 0;
                char done_form = 0;
                char done_case = 0;
                char done_standard = 0;
                ASTNode *args_node = 0;
                char first_instruction = 1;
                next = node->sibling;
                ASTNode *prev = node;
                while (next && next->node_type != PROCEDURE && next->node_type != CLASS_DEF &&
                       next->node_type != METHOD && next->node_type != FACTORY) {
                    switch (next->node_type) {
                        case ARGS:
                            if (args_node) {
                                /* Error - you can only have one arg statement */
                                mknd_err(next, "REPEATED_ARG");
                            } else if (!first_instruction) {
                                /* Error - arg must be the first statement */
                                mknd_err(next, "ARG_NOT_FIRST_INST");
                            }
                            first_instruction = 0;
                            args_node = next;
                            next = next->sibling;
                            /* Note that prev is unchanged as args_node is removed */

                            /* Disconnect/remove the args_node from the AST tree */
                            prev->sibling = args_node->sibling;
                            args_node->sibling = 0;
                            args_node->parent = 0;
                            /* And add it under the procedure node */
                            add_ast(node, args_node);
                            break;

                        case DEC_DIGITS:
                            if (done_digits) {
                                mknd_err(next, "REPEATED_NUMERIC_DIGITS");
                            } else if (!first_instruction) {
                                mknd_err(next, "NUMERIC_DIGITS_NOT_FIRST_INST");
                            }
                            done_digits = 1;
                            prev = next;
                            next = next->sibling;
                            break;

                        case DEC_FUZZ:
                            if (done_fuzz) {
                                mknd_err(next, "REPEATED_NUMERIC_FUZZ");
                            } else if (!first_instruction) {
                                mknd_err(next, "NUMERIC_FUZZ_NOT_FIRST_INST");
                            }
                            done_fuzz = 1;
                            prev = next;
                            next = next->sibling;
                            break;

                        case DEC_FORM:
                            if (done_form) {
                                mknd_err(next, "REPEATED_NUMERIC_FORM");
                            } else if (!first_instruction) {
                                mknd_err(next, "NUMERIC_FORM_NOT_FIRST_INST");
                            }
                            done_form = 1;
                            prev = next;
                            next = next->sibling;
                            break;

                        case DEC_CASE:
                            if (done_case) {
                                mknd_err(next, "REPEATED_NUMERIC_CASE");
                            } else if (!first_instruction) {
                                mknd_err(next, "NUMERIC_CASE_NOT_FIRST_INST");
                            }
                            done_case = 1;
                            prev = next;
                            next = next->sibling;
                            break;

                        case DEC_STANDARD:
                            if (done_standard) {
                                mknd_err(next, "REPEATED_NUMERIC_STANDARD");
                            } else if (!first_instruction) {
                                mknd_err(next, "NUMERIC_STANDARD_NOT_FIRST_INST");
                            }
                            done_standard = 1;
                            prev = next;
                            next = next->sibling;
                            break;

                        default:
                            first_instruction = 0;
                            prev = next;
                            next = next->sibling;
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

                /* For each sibling until the next block */
                while ( ((next = node->sibling)) && next->node_type != PROCEDURE && next->node_type != CLASS_DEF &&
                        next->node_type != METHOD && next->node_type != FACTORY) {
                    last = next; /* To check that there is a return */
                    /* 2. Disconnect/remove next node from the AST tree */
                    node->sibling = next->sibling;
                    next->sibling = 0;
                    next->parent = 0;
                    /* 3. add next under INSTRUCTIONS child */
                    add_ast(new_child,next);
                }

                if (last) { /* If there are no instructions at all it is a declaration */
                    if (last->node_type != RETURN && !context->in_exit_bridge) {
                        /* We need to add a return */
                        new_child = ast_ft(context,RETURN);
                        add_ast(last->parent,new_child); /* Adds as the last sibling */
                    }
                }
            }
        }

        }

    return result_normal;
}

/*
 * source_location_walker
 * - Sets the token and source start / finish position for each node
 */
walker_result source_location_walker(walker_direction direction,
                                     ASTNode* node, __attribute__((unused)) void *payload) {
    ASTNode *child, *n;
    Token *left, *right;
    
    if (direction == out) {
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
            else if (node->node_type == FUNCTION || node->node_type == FUNC_SYMBOL) {
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

            if (node->node_type == ERROR || node->node_type == WARNING) {
                node->line = node->parent->line;
                node->column = node->parent->column;
                node->source_start = node->parent->source_start;
                node->source_end = node->parent->source_end;
                node->token_start = node->parent->token_start;
                node->token_end = node->parent->token_end;
            }
            else {
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
        }
    }
    return result_normal;
}

/*
 * syntax_validation_walker
 * - Fixes SCONCAT to CONCAT, removes NOPs, options, etc.
 */
walker_result syntax_validation_walker(walker_direction direction,
                                       ASTNode* node, __attribute__((unused)) void *payload) {
    ASTNode *child, *next_child;
    int has_to = 0, has_for = 0, has_by = 0, has_assign = 0;
    char *buffer, *c;
    Context *context = (Context*)payload;

    if (direction == out) {
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
            /* Process any REXX options specific for levelb */
            /* Loop through the children */
            child = node->child;
            while (child) {
                if (child->node_type == LITERAL) {
                    /* Check the options */
                    // FLOATS
                    if (is_node_string(child, "floats_decimal")) {
                        if (context->floats_binary) {
                            /* Error - can't have both decimal and binary floats */
                            mknd_err(child, "INCOMPATIBLE_OPTIONS");
                        }
                        else {
                            context->floats_decimal = 1;
                            context->floats_binary = 0;
                        }
                    }
                    else if (is_node_string(child, "floats_binary")) {
                        if (context->floats_decimal) {
                         /* Error - can't have bothdecimal and binary floats */
                            mknd_err(child, "INCOMPATIBLE_OPTIONS");
                        }
                        else {
                            context->floats_decimal = 0;
                            context->floats_binary = 1;
                        }
                    }
                    // NUMERIC
                    else if (is_node_string(child, "numeric_classic")) {
                        if (context->numeric_common) {
                            /* Error - can't have both common and classic numeric */
                            mknd_err(child, "INCOMPATIBLE_OPTIONS");
                        }
                        else {
                            context->numeric_classic = 1;
                            context->numeric_common = 0;
                        }
                    }
                    else if (is_node_string(child, "numeric_common")) {
                        if (context->numeric_classic) {
                            /* Error - can't have both common and classic numeric */
                            mknd_err(child, "INCOMPATIBLE_OPTIONS");
                        }
                        else {
                            context->numeric_classic = 0;
                            context->numeric_common = 1;
                        }
                    }
                    // COMMENTS
                    else if (is_node_string(child, "comments_hash)")) {
                        if (context->comments_hash_specified) {
                            mknd_err(child, "INCOMPATIBLE_OPTIONS");
                        }
                        context->comments_hash_specified = 1;
                    }
                    else if (is_node_string(child, "comments_nohash)")) {
                        if (context->comments_hash_specified) {
                            mknd_err(child, "INCOMPATIBLE_OPTIONS");
                        }
                        context->comments_hash_specified = 1;
                    }
                    else if (is_node_string(child, "comments_dash)")) {
                        if (context->comments_dash_specified) {
                            mknd_err(child, "INCOMPATIBLE_OPTIONS");
                        }
                        context->comments_dash_specified = 1;
                    }
                    else if (is_node_string(child, "comments_nodash)")) {
                        if (context->comments_dash_specified) {
                            mknd_err(child, "INCOMPATIBLE_OPTIONS");
                        }
                        context->comments_dash_specified = 1;
                    }
                    else if (is_node_string(child, "comments_slash)")) {
                        if (context->comments_slash_specified) {
                            mknd_err(child, "INCOMPATIBLE_OPTIONS");
                        }
                        context->comments_slash_specified = 1;
                    }
                    else if (is_node_string(child, "comments_noslash)")) {
                        if (context->comments_slash_specified) {
                            mknd_err(child, "INCOMPATIBLE_OPTIONS");
                        }
                        context->comments_slash_specified = 1;
                    }
                }
                child = child->sibling;
            }
        }
        else if (node->node_type == NAMESPACE) {
            if (!context->namespace) {
                context->namespace = node->child;
            }
            else {
                mknd_err(node, "MULTIPLE_NAMESPACE");
            }
        }
        else if (node->node_type == ASSIGN) {
            if (node->parent && node->parent->node_type == CLASS_DEF) {
                mknd_err(node, "CANT_ASSIGN_IN_CLASS_DEF");
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

walker_result decimal_parameters_walker(walker_direction direction,
                                               ASTNode* node,
                                               void *payload) {

    Context *context = (Context*)payload;
    ASTNode *child;
    int val;

    if (direction == in) {
        /* IN - TOP DOWN */
        context->current_scope = node->scope;
    }
    else {
        /* OUT - BOTTOM UP */
        child = node->child;
        switch (node->node_type) {
            case DEC_DIGITS:
                if (child) {
                    /* Get the value */
                    val = node_to_integer(child);
                    if (val < DIGITS_MINIMUM) {
                        mknd_err(child, "DECIMAL_DIGITS_RANGE"); // Note that the parser captures a negative error beforehand
                        val = 1;
                    }
                    else if (val <= node->scope->num_context.fuzz) {
                        mknd_err(child, "DECIMAL_DIGITS_RANGE");
                        val = node->scope->num_context.fuzz + 1;
                    }
                }
                else {
                    val = -1; // Inherited
                }
                node->scope->num_context.digits = val;
                break;

            case DEC_FUZZ:
                if (child) {
                    /* Get the value */
                    val = node_to_integer(child);
                    if (val < 0) {
                        mknd_err(child, "DECIMAL_FUZZ_RANGE");  // Note that the parser captures this error beforehand
                        val = 0;
                    }
                    else if (val >= node->scope->num_context.digits) {
                        mknd_err(child, "DECIMAL_FUZZ_RANGE");
                        val = node->scope->num_context.digits - 1;
                    }
                }
                else {
                    val = -1; // Inherited
                }
                node->scope->num_context.fuzz = val;
                break;

            case DEC_FORM:
                if (child) {
                    if (nodeis(child, "scientific")) node->scope->num_context.form = NUMERIC_FORM_SCIENTIFIC;
                    else if (nodeis(child, "engineering")) node->scope->num_context.form = NUMERIC_FORM_ENGINEERING;
                    else if (nodeis(child, "inherited")) node->scope->num_context.form = NUMERIC_FORM_INHERIT;
                    else {
                        mknd_err(child, "DECIMAL_FORM_VALUE");
                        node->scope->num_context.form = NUMERIC_FORM_INHERIT;
                    }
                }
                else {
                    node->scope->num_context.form = NUMERIC_FORM_INHERIT; // Inherited
                }
                break;

           case DEC_CASE:
                if (child) {
                    if (nodeis(child, "lower")) node->scope->num_context.casetype = CASE_LOWER;
                    else if (nodeis(child, "upper")) node->scope->num_context.casetype = CASE_UPPER;
                    else if (nodeis(child, "inherited")) node->scope->num_context.casetype = CASE_INHERIT;
                    else {
                        mknd_err(child, "DECIMAL_CASE_VALUE");
                        node->scope->num_context.casetype = CASE_INHERIT;
                    }
                }
                else {
                    node->scope->num_context.casetype = CASE_INHERIT; // Inherited
                }
                break;

           case DEC_STANDARD:
                if (child) {
                    if (nodeis(child, "common")) node->scope->num_context.standard = NUMERIC_STANDARD_COMMON;
                    else if (nodeis(child, "classic")) node->scope->num_context.standard = NUMERIC_STANDARD_CLASSIC;
                    else if (nodeis(child, "inherited")) node->scope->num_context.standard = NUMERIC_STANDARD_INHERIT;
                    else {
                        mknd_err(child, "DECIMAL_STANDARD_VALUE");
                        node->scope->num_context.standard = NUMERIC_STANDARD_INHERIT;
                    }
                }
                else {
                    node->scope->num_context.standard = NUMERIC_STANDARD_INHERIT; // Inherited
                }
                break;

            default:;
        }

        context->current_scope = node->scope;
    }

    return result_normal;
}
