/*
 * Emit Assembler
 */

#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "rexbgrmr.h"

/* Output Marshalling Functions */
static OutputFragment *output_f(){
    OutputFragment *output = malloc(sizeof(OutputFragment));
    output->output = 0;
    output->after = NULL;
    output->before = NULL;
    return output;
}

static OutputFragment *output_fs(char* text){
    OutputFragment *output = malloc(sizeof(OutputFragment));
    output->after = NULL;
    output->before = NULL;
    output->output = malloc(strlen(text) + 1);
    strcpy(output->output, text);
    return output;
}

void free_output(OutputFragment *output) {
    if (output->output) free(output->output);
    free(output);
}

static void output_insert_before(OutputFragment* existing, OutputFragment* before) {
    before->before = existing->before;
    before->after = existing;
    existing->before = before;
}

static void output_insert_after(OutputFragment* existing, OutputFragment* after) {
    after->after = existing->after;
    after->before = existing;
    existing->after = after;
}

static void output_append(OutputFragment* before, OutputFragment* after) {
    while (before->after) before = before->after;
    before->after = after;
    after->before = before;
}

static void print_output(FILE* file, OutputFragment* existing) {
    while (existing) {
        if (existing->output) fputs(existing->output, file);
        existing = existing->after;
    }
}

typedef struct walker_payload {
    Scope *current_scope;
    FILE *file;
} walker_payload;

/* Assign registers */
static walker_result register_walker(walker_direction direction,
                                 ASTNode* node,
                                 void *pl) {

    walker_payload *payload = (walker_payload*) pl;
    ASTNode *child1, *child2;

    if (direction == in) {
        /* IN - TOP DOWN */
        if (node->scope) {
            payload->current_scope = node->scope;
        }
        if (node->node_type == ASSIGN) {
            /* If an assignment from an expression (rather than a symbol) then
             * then mark the register so we can assign it to the target
             * register on the way out (bottom up)
             */
            if (node->child->sibling->symbol == 0)
                node->child->sibling->register_num = -2;
        }
    }
    else {
        /* OUT - BOTTOM UP */
        child1 = node->child;
        if (child1) child2 = child1->sibling;
        else child2 = NULL;

        switch (node->node_type) {

            case OP_AND:
            case OP_OR:
            case OP_COMPARE:
            case OP_CONCAT:
            case OP_SCONCAT:
            case OP_ADD:
            case OP_MINUS:
            case OP_MULT:
            case OP_POWER:
            case OP_DIV:
            case OP_IDIV:
            case OP_MOD:
                /* If it is a temporary mark the register for reuse */
                if (child1->symbol == 0)
                    return_reg(payload->current_scope, child1->register_num);
                if (child2->symbol == 0)
                    return_reg(payload->current_scope, child2->register_num);

                /* Set result temporary register */
                if (node->register_num != -2)
                    /* -2 means that the register number will be set later */
                    node->register_num = get_reg(payload->current_scope);
                break;

            case OP_PREFIX:
                /* If it is a temporary mark the register for reuse */
                if (child1->symbol == 0)
                    return_reg(payload->current_scope, child1->register_num);

                /* Set result temporary register */
                if (node->register_num != -2)
                    /* -2 means that the register number will be set later */
                    node->register_num = get_reg(payload->current_scope);
                break;

            case VAR_SYMBOL:
            case VAR_TARGET:
                /* Set the symbols register */
                if (node->symbol->register_num == -1)
                    node->symbol->register_num = get_reg(payload->current_scope);
                /* The node uses the symbol register number */
                node->register_num = node->symbol->register_num;
                break;

            case CONST_SYMBOL: /* TODO */
            case FLOAT:
            case INTEGER:
            case STRING:
                /* Set result temporary register */
                if (node->register_num != -2)
                    /* -2 means that the register number will be set later */
                    node->register_num = get_reg(payload->current_scope);
                break;

            case ASSIGN:
                if (child2->register_num == -2)
                    /* Marked earlier so set the register to the target register */
                    child2->register_num = child1->register_num;
                break;

            case ADDRESS:
            case SAY:
            case IF:
                /* If it is a temporary mark the register for reuse */
                node->register_num = child1->register_num;
                if (child1->symbol == 0)
                    return_reg(payload->current_scope, child1->register_num);
                break;

            default:;
        }

        if (node->scope) payload->current_scope = payload->current_scope->parent;
    }

    return result_normal;
}

#define buf_len 512
#define get_comment snprintf(comment, buf_len, "   * Line %d: %.*s\n", node->line + 1,(int)(node->source_end - node->source_start) + 1, node->source_start)

void type_promotion(ASTNode *node) {
    char *op;
    char temp[buf_len];

    if (node->value_type != node->target_type) {
        switch (node->target_type) {
            case TP_INTEGER:
            case TP_BOOLEAN:
                op = "iprime";
                break;

            case TP_FLOAT:
                op = "fprime";
                break;

            default:
                op = "sprime";
                break;
        }
        snprintf(temp, buf_len, "   %s r%d\n",
                 op,
                 node->register_num);
        node->output3 = output_fs(temp);
        output_append(node->output, node->output3);
    }
}

static walker_result emit_walker(walker_direction direction,
                                  ASTNode* node,
                                  void *pl) {

    walker_payload *payload = (walker_payload*) pl;
    ASTNode *child1, *child2, *n;
    char *op;
    char *tp_prefix;
    OutputFragment *o;
    char temp[buf_len];
    char comment[buf_len];

    if (direction == in) {
        /* IN - TOP DOWN */
        if (node->scope) {
            payload->current_scope = node->scope;
        }
    }
    else {
        /* OUT - BOTTOM UP */
        child1 = node->child;
        if (child1) child2 = child1->sibling;
        else child2 = NULL;

        /* Operator and type prefix */
        op = 0;
        switch (node->value_type) {
            case TP_BOOLEAN:
            case TP_INTEGER:
                tp_prefix = "i"; break;
            case TP_STRING:
                tp_prefix = "s"; break;
            case TP_FLOAT:
                tp_prefix = "f"; break;
            default:
                tp_prefix = "";
        }

        switch (node->node_type) {

            case PROGRAM_FILE:
                snprintf(temp, buf_len,"/* REXX COMPILER PoC */\n"
                                         "\n"
                                         ".globals=0\n"
                                         "\n"
                                         "main()   .locals=%d\n",
                                         node->scope->num_registers);
                node->output = output_fs(temp);
                n = child1;
                while (n) {
                    if (n->output) output_append(node->output, n->output);
                    n = n->sibling;
                }

                node->output2 = output_fs("   exit\n"); /* TODO PoC Hack! */
                output_append(node->output, node->output2);

                print_output(payload->file, node->output);
                break;

            case INSTRUCTIONS:
                node->output = output_f();
                n = child1;
                while (n) {
                    if (n->output) output_append(node->output, n->output);
                    n = n->sibling;
                }
                break;

            case OP_CONCAT:
            case OP_SCONCAT: /* TODO */
                op="concats";
                node->output = output_f();
                if (child1->output) output_append(node->output, child1->output);
                if (child2->output) output_append(node->output, child2->output);
                snprintf(temp, buf_len, "   %s r%d,r%d,r%d\n",
                         op,
                         node->register_num,
                         child1->register_num,
                         child2->register_num);
                node->output2 = output_fs(temp);
                output_append(node->output, node->output2);
                type_promotion(node);
            break;

            case OP_AND:
                if (!op) op="and";
            case OP_OR:
                if (!op) op="or";
            case OP_COMPARE:
                if (!op) op="compare"; /* TODO */
            case OP_ADD:
                if (!op) op="add";
            case OP_MINUS:
                if (!op) op="sub";
            case OP_MULT:
                if (!op) op="mult";
            case OP_POWER:
                if (!op) op="pow";
            case OP_DIV:
                if (!op) op="div";
            case OP_IDIV:
                if (!op) op="divi";
            case OP_MOD:
                if (!op) op="mod";
                node->output = output_f();
                if (child1->output) output_append(node->output, child1->output);
                if (child2->output) output_append(node->output, child2->output);
                snprintf(temp, buf_len, "   %s%s r%d,r%d,r%d\n",
                         tp_prefix,
                         op,
                         node->register_num,
                         child1->register_num,
                         child2->register_num);
                node->output2 = output_fs(temp);
                output_append(node->output, node->output2);
                type_promotion(node);
                break;

            case OP_PREFIX:
                node->output = output_f();
                if (child1->output) output_append(node->output, child1->output);
                snprintf(temp, buf_len, "   prefix_todo r%d\n",
                         child1->register_num);
                node->output2 = output_fs(temp);
                output_append(node->output, node->output2);
                type_promotion(node);
                break;

            case VAR_SYMBOL:
                node->output = output_f();
                type_promotion(node);
                break;

            case VAR_TARGET:
                break;

            case CONST_SYMBOL: /* TODO */
            case FLOAT:
            case INTEGER:
            case STRING:
                snprintf(temp, buf_len, "   load r%d,%.*s\n",
                         node->register_num,
                         node->node_string_length,
                         node->node_string);
                node->output = output_fs(temp);
                type_promotion(node);
                break;

            case ASSIGN:
                get_comment;
                node->output = output_fs(comment);
                if (child2->output) output_append(node->output, child2->output);
                if (child1->register_num != child2->register_num) {
                    snprintf(temp, buf_len, "   copy r%d,r%d\n",
                             child1->register_num,
                             child2->register_num);
                    node->output2 = output_fs(temp);
                    output_append(node->output, node->output2);
                }
                break;

            case ADDRESS:
                get_comment;
                node->output = output_fs(comment);
                if (child1->output) output_append(node->output, child1->output);
                snprintf(temp, buf_len, "   address r%d\n",
                         node->register_num);
                node->output2 = output_fs(temp);
                output_append(node->output, node->output2);
                break;

            case SAY:
                get_comment;
                node->output = output_fs(comment);
                if (child1->output) output_append(node->output, child1->output);
                snprintf(temp, buf_len, "   say r%d\n   say \"\\n\"\n",
                         node->register_num);
                node->output2 = output_fs(temp);
                output_append(node->output, node->output2);
                break;

            case IF:
                break;

            default:;
        }

        if (node->scope) payload->current_scope = payload->current_scope->parent;
    }

    return result_normal;
}

void emit(Context *context, char *output_file) {
    walker_payload payload;
    FILE *output;

    if (output_file) output = fopen(output_file, "w");
    else output = stdout;

    payload.file = output;
    payload.current_scope = 0;

    ast_walker(context->ast, register_walker, (void*)&payload);
    ast_walker(context->ast, emit_walker, (void*)&payload);

    fclose(output);
}

