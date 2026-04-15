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
 * AST Printing/Debugging Utilities
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxcpmain.h"
#include "rxcp_source_tree.h"

void print_error(ASTNode* node, FILE* stream, char* prefix) {

    if (node->is_duplicate_warning) return;

    /* Try and set error position if not already set */
    ASTNode *p = node;
    while (p && p->line == -1 && !p->token) {
        p = p->parent;
    }
    if (p) {
        if (node->line == -1) {
            if (p->line != -1) node->line = p->line;
            else if (p->token) node->line = p->token->line;
        }
        if (node->column == -1) {
            if (p->column != -1) node->column = p->column;
            else if (p->token) node->column = p->token->column;
        }
        if (!node->source_start) {
            if (p->source_start) node->source_start = p->source_start;
            else if (p->token) node->source_start = p->token->token_string;
        }
        if (!node->source_end) {
            if (p->source_end) node->source_end = p->source_end;
            else if (p->token) node->source_end = p->token->token_string + p->token->length - 1;
        }
    }

    if (node->token) {
        if (node->line == -1) node->line = node->token->line;
        if (node->column == -1) node->column = node->token->column;
        if (!node->source_start) node->source_start = node->token->token_string;
        if (!node->source_end) node->source_end = node->token->token_string + node->token->length - 1;
    }
    if (node->child && node->child->token) {
        if (node->line == -1) node->line = node->child->token->line;
        if (node->column == -1) node->column = node->child->token->column;
        if (!node->source_start) node->source_start = node->child->token->token_string;
        if (!node->source_end) node->source_end = node->child->token->token_string + node->child->token->length - 1;
    }

    /* Print error - truncate source to one line */
    int len = (int) (node->source_end - node->source_start + 1);
    int i;
    for (i=0; i<len; i++) {
        if (!node->source_start || node->source_start[i] == '\n') {
            len = i;
            break;
        }
    }
    if (len) {
        fprintf(stream, "%s %s @ %d:%d - #%s, \"", prefix, node->file_name, node->line + 1,
                node->column + 1, node->node_string);
        prt_unex(stream, node->source_start, len);
        fprintf(stream, "\"\n");
    }
    else {
        fprintf(stream, "%s %s @ %d:%d - #%s\n", prefix, node->file_name, node->line + 1,
                node->column + 1, node->node_string);
    }
}

walker_result prnt_walker_handler(walker_direction direction,
                                        ASTNode* node,
                                  __attribute__((unused)) void *payload) {
    if (direction == in) {
        if (node->child) { /* Non-terminal node */
            printf(" ^(");
        } else printf(" ");
        if (node->node_string_length) {
            printf("%s=", ast_ndtp(node->node_type));
            printf("\"");
            prt_unex(stdout, node->node_string,
                     (int) node->node_string_length);
            printf("\"");
        }
        else {
            printf("%s", ast_ndtp(node->node_type));
        }
    }
    else {
        if (node->child) { /* Non-terminal node */
            printf(")");
        }
    }
    return result_normal;
}

static walker_result print_error_walker(walker_direction direction,
                                  ASTNode* node,
                                  __attribute__((unused)) void *payload) {

    int *errors = (int*)payload;

    if (direction == in) {
        if (node->node_type == ERROR && !node->is_duplicate_warning) {
            print_error(node, stderr, "Error in");
            (*errors)++;
        }
    }
    return result_normal;
}

static walker_result print_warning_walker(walker_direction direction,
                                        ASTNode* node,
                                        __attribute__((unused)) void *payload) {

    int *errors = (int*)payload;

    if (direction == in) {
        if (node->node_type == WARNING && !node->is_duplicate_warning) {
            print_error(node, stderr, "Warning in");
            (*errors)++;
        }
    }
    return result_normal;
}

static void print_source_diagnostic(SourceDiagnostic *diag, FILE *stream, const char *prefix) {
    int len;
    int i;

    if (!diag || !stream || !prefix) return;

    len = 0;
    if (diag->source_start && diag->source_end && diag->source_end >= diag->source_start) {
        len = (int)(diag->source_end - diag->source_start + 1);
        for (i = 0; i < len; i++) {
            if (diag->source_start[i] == '\n') {
                len = i;
                break;
            }
        }
    }

    if (len > 0) {
        fprintf(stream, "%s %s @ %d:%d - #%s, \"",
                prefix,
                diag->file_name ? diag->file_name : "<unknown>",
                diag->line + 1,
                diag->column + 1,
                diag->message ? diag->message : "Syntax Error");
        prt_unex(stream, diag->source_start, len);
        fprintf(stream, "\"\n");
    } else {
        fprintf(stream, "%s %s @ %d:%d - #%s\n",
                prefix,
                diag->file_name ? diag->file_name : "<unknown>",
                diag->line + 1,
                diag->column + 1,
                diag->message ? diag->message : "Syntax Error");
    }
}

/* Prints errors and returns the number of errors in the AST Tree */
int prnterrs(Context *context) {
    int errors = 0;
    ASTNode *diag;
    SourceDiagnostic *source_diag;
    const char *prefix;

    if (context->source_tree && context->source_diagnostics_list) {
        source_diag = context->source_diagnostics_list;
        while (source_diag) {
            if (source_diag->severity == SOURCE_DIAG_ERROR) {
                prefix = source_diag->is_internal ? "Internal error in" : "Error in";
                print_source_diagnostic(source_diag, stderr, prefix);
                errors++;
            }
            source_diag = source_diag->next_in_context;
        }
        return errors;
    }

    if (context->ast) {
        ast_wlkr(context->ast, print_error_walker, &errors);
    }
    diag = (ASTNode*)context->diagnostics_list;
    while (diag) {
        if (diag->node_type == ERROR && !diag->is_duplicate_warning) {
            print_error(diag, stderr, "Error in");
            errors++;
        }
        diag = diag->sibling;
    }
    return errors;
}

int prntwars(Context *context) {
    int errors = 0;
    ASTNode *diag;
    SourceDiagnostic *source_diag;
    const char *prefix;

    if (context->source_tree && context->source_diagnostics_list) {
        source_diag = context->source_diagnostics_list;
        while (source_diag) {
            if (source_diag->severity == SOURCE_DIAG_WARNING) {
                prefix = source_diag->is_internal ? "Internal warning in" : "Warning in";
                print_source_diagnostic(source_diag, stderr, prefix);
                errors++;
            }
            source_diag = source_diag->next_in_context;
        }
        return errors;
    }

    if (context->ast) {
        ast_wlkr(context->ast, print_warning_walker, &errors);
    }
    diag = (ASTNode*)context->diagnostics_list;
    while (diag) {
        if (diag->node_type == WARNING && !diag->is_duplicate_warning) {
            print_error(diag, stderr, "Warning in");
            errors++;
        }
        diag = diag->sibling;
    }
    return errors;
}

void prnt_ast(ASTNode *node) {
    ast_wlkr(node, prnt_walker_handler, NULL);
}

/* Prints to dot file one symbol */
void pdot_scope(Symbol *symbol, void *payload) {
    char reg[20];
    char *arr;
    char *name;
    char *clas;

    if (symbol->register_num >= 0)
        sprintf(reg,"%c%d",symbol->register_type,symbol->register_num);
    else
        reg[0] = 0;

    name = sym_frnm(symbol);

    arr = ast_astr(symbol->value_dims, symbol->dim_base, symbol->dim_elements);

    if (symbol->value_class)
        clas = symbol->value_class;
    else
        clas = type_nm(symbol->type);

    switch (symbol->symbol_type) {
        case CLASS_SYMBOL:
        case NAMESPACE_SYMBOL:
            fprintf((FILE *) payload,
                    "\"s%p%d_%s\"[style=filled fillcolor=green shape=box label=\"%s\"]\n",
                    (void*)symbol->scope->defining_node->context,
                    symbol->scope->defining_node->node_number,
                    symbol->name,
                    name);
            break;

        case FUNCTION_SYMBOL:
            fprintf((FILE *) payload,
                    "\"s%p%d_%s\"[style=filled fillcolor=pink shape=box label=\"%s\\n(%s%s)\\n%s\"]\n",
                    (void*)symbol->scope->defining_node->context,
                    symbol->scope->defining_node->node_number,
                    symbol->name,
                    name,
                    clas, arr,
                    reg);
            break;
        default:
            fprintf((FILE *) payload,
                    "\"s%p%d_%s\"[style=filled fillcolor=cyan shape=box label=\"%s\\n(%s%s)\\n%s\"]\n",
                    (void*)symbol->scope->defining_node->context,
                    symbol->scope->defining_node->node_number,
                    symbol->name,
                    name,
                    clas, arr,
                    reg);
    }
    free(arr);
    free(name);
}

walker_result pdot_walker_handler(walker_direction direction,
                                  ASTNode* node, void *payload) {
    FILE* output = (FILE*)payload;

    char *attributes;
    int only_type = 0;
    int only_label = 0;
    int child_index;
    ASTNode *first_node;
    char value_type_buffer[200];
    char* varr;
    char* vclas;
    char* tarr;
    char* tclas;

    if (direction == in) {
        /* IN - TOP DOWN */

        child_index = ast_chdi(node);

        /* Scope == DOT Subgraph */
        if (!node->parent || node->scope != node->parent->scope) {
            if (node->scope) fprintf(output, "subgraph scope_%p{\n", (void*)node->scope);
        }

        /* Attributes */
        switch (node->node_type) {

            /* Groupings */
            case REXX_UNIVERSE:
            case PROGRAM_FILE:
            case IMPORTED_FILE:
            case INSTRUCTIONS:
            case DO:
            case BY:
            case IF:
            case REXX_OPTIONS:
            case IMPORT:
            case NAMESPACE:
            case EXPOSED:
            case TO:
                attributes = "color=blue";
                only_type = 1;
                break;

            case ASSIGN:
            case CALL:
            case DEFINE:
            case DEC_DIGITS:
            case DEC_FUZZ:
            case DEC_FORM:
            case DEC_STANDARD:
            case DEC_CASE:
            case ENVIRONMENT:
            case FOR:
            case WHILE:
            case UNTIL:
            case ITERATE:
            case LEAVE:
            case LEAVE_WITH:
            case NOP:
            case OPTIONS:
            case PULL:
            case RANGE:
            case REPEAT:
            case REDIRECT_IN:
            case REDIRECT_OUT:
            case REDIRECT_ERROR:
            case REDIRECT_EXPOSE:
            case RETURN:
            case EXIT:
            case SAY:
            case UPPER:
            case PARSE:
            case BLOCK_EXPR:
                attributes = "color=green4";
                only_type = 1;
                break;

            case ASSEMBLER:
                attributes = "color=green4";
                break;

            case FUNCTION:
            case FUNC_SYMBOL:
            case PROCEDURE:
                attributes = "color=pink";
                break;

                /* Address is often a sign of a parsing error */
            case ADDRESS:
                attributes = "style=filled fillcolor=orange";
                only_type = 1;
                break;

            case OP_ADD:
            case OP_MINUS:
            case OP_AND:
            case OP_ARGS:
            case OP_ARG_VALUE:
            case OP_ARG_EXISTS:
            case OP_ARG_IX_EXISTS:
            case OP_CONCAT:
            case OP_MULT:
            case OP_DIV:
            case OP_IDIV:
            case OP_MOD:
            case OP_OR:
            case OP_POWER:
            case OP_NOT:
            case OP_PLUS:
            case OP_NEG:
            case OP_SCONCAT:
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
            case OP_MAKE_ARRAY:
            case NOVAL:
                attributes = "color=darkcyan";
                only_type = 1;
                break;

            case ARG:
            case ARGS:
            case PATTERN:
            case CLASS:
            case VOID:
            case REL_POS:
            case ABS_POS:
            case SIGN:
            case TARGET:
            case TEMPLATES:
                attributes = "color=green";
                break;

            case VAR_SYMBOL:
            case VAR_TARGET:
            case VAR_REFERENCE:
            case VARG_REFERENCE:
            case VARG:
            case CONST_SYMBOL:
            case LITERAL:
                attributes = "color=cyan3 shape=cds";
//                only_label = 1;
                break;

            case STRING:
            case BINARY:
            case INTEGER:
            case FLOAT:
            case DECIMAL:
            case CONSTANT:
                attributes = "color=cyan3 shape=box";
                only_label = 1;
                break;

            case LABEL:
                attributes = "color=green4";
                break;

                /* Errors */
            case TOKEN:
                attributes = "style=filled fillcolor=indianred1";
                only_label = 1;
                break;

//            case ERROR:
//                attributes = "style=filled fillcolor=indianred1";
//                //           only_type = 1;
//                break;

            default:
                attributes = "style=filled fillcolor=indianred1";
                break;
        }

        varr = ast_astr(node->value_dims, node->value_dim_base, node->value_dim_elements);
        tarr = ast_astr(node->target_dims, node->target_dim_base, node->target_dim_elements);

        if (node->value_class) vclas = node->value_class;
        else vclas = type_nm(node->value_type);

        if (node->target_class) tclas = node->target_class;
        else if (node->target_type == TP_OBJECT && node->value_class) tclas = node->value_class;
        else tclas = type_nm(node->target_type);

        if (node->register_num >= 0) {
            if (node->num_additional_registers)
                sprintf(value_type_buffer,"\n%c%d, r%d-r%d",
                        node->register_type, node->register_num,
                        node->additional_registers,
                        node->additional_registers + node->num_additional_registers - 1);
            else
                sprintf(value_type_buffer,"\n%c%d", node->register_type, node->register_num);
        }
        else
            value_type_buffer[0] = 0;

        if (node->value_type != TP_UNKNOWN || node->value_dims ||
            node->target_type != TP_UNKNOWN || node->target_dims) {
            if (node->value_type == node->target_type && strcmp(vclas,tclas) == 0 && strcmp(varr,tarr) == 0) {
                strcat(value_type_buffer, "\n(");
                strcat(value_type_buffer, vclas);
                strcat(value_type_buffer, varr);
                strcat(value_type_buffer, ")");
            } else {
                strcat(value_type_buffer, "\n(");
                strcat(value_type_buffer, vclas);
                strcat(value_type_buffer, varr);
                strcat(value_type_buffer, "->");
                strcat(value_type_buffer, tclas);
                strcat(value_type_buffer, tarr);
                strcat(value_type_buffer, ")");
            }
        }

        free(varr);
        free(tarr);

        if (only_type) {
            fprintf(output, "n%p%d[ordering=\"out\" label=\"%s%s", (void*)node->context,node->node_number,
                    ast_ndtp(node->node_type), value_type_buffer);
        } else if (only_label) {
            fprintf(output, "n%p%d[ordering=\"out\" label=\"", (void*)node->context,node->node_number);
            prt_unex(output, node->node_string,
                     (int) node->node_string_length);
            fprintf(output, "%s", value_type_buffer);
        } else {
            fprintf(output, "n%p%d[ordering=\"out\" label=\"%s\\n", (void*)node->context,node->node_number,
                    ast_ndtp(node->node_type));
            prt_unex(output, node->node_string,
                     (int) node->node_string_length);
            fprintf(output, "%s", value_type_buffer);
        }
        fprintf(output, "\" %s]\n", attributes);

        /* Link to Parent */
        if (node->parent) {
            fprintf(output,"n%p%d -> n%p%d [xlabel=\"%d\"]\n",
                    (void*)node->parent->context,  node->parent->node_number,
                    (void*)node->context,node->node_number, child_index);
        }

        /* Link to Associated Node */
        if (node->association) {
            fprintf(output,"n%p%d -> n%p%d [color=red dir=\"forward\"]\n",
                    (void*)node->context,node->node_number,
                    (void*)node->association->context,node->association->node_number);
        }

        /* Link to Symbol */
        if (node->symbolNode) {
            if (node->symbolNode->writeUsage && node->symbolNode->readUsage) {
                fprintf(output,"n%p%d -> \"s%p%d_%s\" [color=cyan dir=\"both\"]\n",
                        (void*)node->context,node->node_number,
                        (void*)node->symbolNode->symbol->scope->defining_node->context,node->symbolNode->symbol->scope->defining_node->node_number,
                        node->symbolNode->symbol->name);
            }
            else if (node->symbolNode->writeUsage) {
                fprintf(output,"n%p%d -> \"s%p%d_%s\" [color=cyan dir=\"forward\"]\n",
                        (void*)node->context,node->node_number,
                        (void*)node->symbolNode->symbol->scope->defining_node->context,node->symbolNode->symbol->scope->defining_node->node_number,
                        node->symbolNode->symbol->name);
            }
            else if (node->symbolNode->readUsage) {
                fprintf(output,"n%p%d -> \"s%p%d_%s\" [color=cyan dir=\"back\"]\n",
                        (void*)node->context,node->node_number,
                        (void*)node->symbolNode->symbol->scope->defining_node->context,node->symbolNode->symbol->scope->defining_node->node_number,
                        node->symbolNode->symbol->name);
            }
            else {
                fprintf(output,"n%p%d -> \"s%p%d_%s\" [color=cyan dir=\"none\"]\n",
                        (void*)node->context,node->node_number,
                        (void*)node->symbolNode->symbol->scope->defining_node->context,node->symbolNode->symbol->scope->defining_node->node_number,
                        node->symbolNode->symbol->name);
            }
        }
    }

    else {
        /* OUT - Bottom Up */
        /* Scope Symbols */
        if (!node->parent || node->scope != node->parent->scope) {
            if (node->scope) scp_4all(node->scope, pdot_scope, output);
        }

        /* Scope == DOT Subgraph */
        if (!node->parent || node->scope != node->parent->scope) {
            if (node->scope) fprintf(output, "}\n");
        }
    }

    return result_normal;
}

void pdot_tree(ASTNode *tree, char* output_file, char* prefix) {
/*
    char dot_filename[250];
    char command[250];
    FILE *output;

    snprintf(dot_filename, 250, "%s.%s.dot", prefix, output_file);
    output = fopen(dot_filename, "w");

    if (tree) {
        fprintf(output, "digraph REXXAST { pad=0.25\n");
        ast_wlkr(tree, pdot_walker_handler, (void *) output);
        fprintf(output, "\n}\n");
    }
    fclose(output);

    // Get dot from https://graphviz.org/download/
    snprintf(command, 250, "dot %s.%s.dot -Tpng -o %s.%s.png", prefix, output_file, prefix, output_file);
    system(command);
*/
}

void ast_dump_text(FILE* out, ASTNode* node, int indent) {
    int i;
    if (!node) return;

    for (i = 0; i < indent; i++) fprintf(out, "  ");
    fprintf(out, "[AST] %s (Type %d) [Line %d]", node_type_to_string(node->node_type), node->node_type, node->line);

    if (node->node_string) {
         fprintf(out, " \"");
         prt_unex(out, node->node_string, (int)node->node_string_length);
         fprintf(out, "\"");
    }
    fprintf(out, "\n");

    ASTNode *child = node->child;
    while (child) {
        ast_dump_text(out, child, indent + 1);
        child = child->sibling;
    }
}

void rxcp_debug_header(const char *stage_name, int iteration) {
    fprintf(stderr, "\n--- %s ", stage_name);
    if (iteration >= 0) {
        fprintf(stderr, "(Iteration %d) ", iteration);
    }
    fprintf(stderr, "---\n");
}

void rxcp_print_ast_recursive(ASTNode *node, int indent) {
    if (!node) return;
    int i;
    for (i = 0; i < indent; i++) fprintf(stderr, "  ");
    fprintf(stderr, "%s : \"", ast_ndtp(node->node_type));
    prt_unex(stderr, node->node_string, (int)node->node_string_length);
    fprintf(stderr, "\" (%d:%d) [Type: %s]\n", node->line, node->column, type_nm(node->value_type));

    ASTNode *child = node->child;
    while (child) {
        rxcp_print_ast_recursive(child, indent + 1);
        child = child->sibling;
    }
}

void rxcp_print_symbol_table(Scope *scope, int depth) {
    if (!scope) return;
    int i;
    for (i = 0; i < depth; i++) fprintf(stderr, "  ");
    fprintf(stderr, "Scope [%d]: %s\n", depth, scope->name ? scope->name : "Unnamed");

    Symbol **symbols = scp_syms(scope);
    if (symbols) {
        for (i = 0; symbols[i]; i++) {
            int j;
            for (j = 0; j < depth + 1; j++) fprintf(stderr, "  ");
            char *type_str = sym_2tp(symbols[i]);
            fprintf(stderr, "%s (%s) -> %s", symbols[i]->name, stype_nm(symbols[i]->symbol_type), type_str);
            if (symbols[i]->is_global_var) fprintf(stderr, " [GLOBAL]");
            if (symbols[i]->is_shadowing) fprintf(stderr, " [SHADOWS_GLOBAL]");
            if (symbols[i]->exposed) fprintf(stderr, " [EXPOSED]");
            fprintf(stderr, "\n");
            free(type_str);
        }
        free(symbols);
    }

    size_t num_children = scp_noch(scope);
    size_t k;
    for (k = 0; k < num_children; k++) {
        rxcp_print_symbol_table(scp_chd(scope, k), depth + 1);
    }
}
