/*
 * Emit Assembler
 */

#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "rxcpmain.h"
#include "rxcpbgmr.h"

/* Encodes a string to a buffer. Like snprintf() it returns the number of characters
 * that would have been written */
#define ADD_CHAR_TO_BUFFER(ch) {out_len++; if (buffer_len) { *(buffer++) = (ch); buffer_len--; }}
static size_t encode_print(char* buffer, size_t buffer_len, const char* string, size_t length) {

    size_t out_len = 0;
    while (length) {
        switch (*string) {
            case '\\':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('\\');
                break;
            case '\n':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('n');
                break;
            case '\t':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('t');
                break;
            case '\a':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('a');
                break;
            case '\b':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('b');
                break;
            case '\f':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('f');
                break;
            case '\r':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('r');
                break;
            case '\v':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('v');
                break;
            case '\'':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('\'');
                break;
            case '\"':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('\"');
                break;
            case 0:
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('0');
                break;
            case '\?':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('?');
                break;
            default:
                ADD_CHAR_TO_BUFFER(*string);
        }
        string++;
        length--;
    }
    if (buffer_len) *buffer = 0;
    return out_len;
}

/* Encodes a string to a buffer - just handling line breaks etc for comment strings */
static size_t encode_comment(char* buffer, size_t buffer_len, char* string, size_t length) {

    size_t out_len = 0;
    while (length) {
        switch (*string) {
            case '\n':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('n');
                break;
            case '\t':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('t');
                break;
            case '\f':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('f');
                break;
            case '\r':
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('r');
                break;
            case 0:
                ADD_CHAR_TO_BUFFER('\\');
                ADD_CHAR_TO_BUFFER('0');
                break;
            default:
                ADD_CHAR_TO_BUFFER(*string);
        }
        string++;
        length--;
    }
    if (buffer_len) *buffer = 0;
    return out_len;
}

#undef ADD_CHAR_TO_BUFFER


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

void f_output(OutputFragment *output) {
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
    ASTNode *child1, *child2, *c;

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
            case OP_AND:
            case OP_OR:
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
                    ret_reg(payload->current_scope, child1->register_num);
                if (child2->symbol == 0)
                    ret_reg(payload->current_scope, child2->register_num);

                /* Set result temporary register */
                if (node->register_num != -2)
                    /* -2 means that the register number will be set later */
                    node->register_num = get_reg(payload->current_scope);
                break;

            case OP_PREFIX:
                /* If it is a temporary mark the register for reuse */
                if (child1->symbol == 0)
                    ret_reg(payload->current_scope, child1->register_num);

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
                node->register_num = child1->register_num;
                break;

            case ADDRESS:
            case SAY:
            case IF:
                /* If it is a temporary mark the register for reuse */
                node->register_num = child1->register_num;
                if (child1->symbol == 0)
                    ret_reg(payload->current_scope, child1->register_num);
                break;

            case REPEAT:
                node->register_num = child1->register_num;
                break;

            case DO:
                /* We need to free temporary registers for the children
                 * TO/BY/FOR which is under REPEAT (child1) */
                c = child1->child->sibling; /* The second child under the REPEAT */
                while (c) {
                    if (c->child) {
                        c->register_num = c->child->register_num;
                        if (c->child->symbol == 0)
                            ret_reg(payload->current_scope, c->register_num);
                    }
                    c = c->sibling;
                }
                break;

            default:;
        }

        if (node->scope) payload->current_scope = payload->current_scope->parent;
    }

    return result_normal;
}

#define buf_len 512

static void get_comment(char* comment, ASTNode *node, char* prefix) {
    char temp[buf_len];
    encode_comment(temp, buf_len, node->source_start, (int)(node->source_end - node->source_start) + 1);
    if (prefix)
        snprintf(comment, buf_len, "   * Line %d: %s %s\n", node->line, prefix, temp);
    else
        snprintf(comment, buf_len, "   * Line %d: %s\n", node->line, temp);
}

/* Comment without quoting node text */
static void get_comment_line_number_only(char* comment, ASTNode *node, char* prefix) {
    if (prefix)
        snprintf(comment, buf_len, "   * Line %d: %s\n", node->line, prefix);
    else
        snprintf(comment, buf_len, "   * Line %d:\n", node->line);
}

static void type_promotion(ASTNode *node) {

    char *op1, *op2;
    char temp[buf_len];

    if (node->value_type != node->target_type) {

        switch (node->value_type) {
            case TP_INTEGER:
            case TP_BOOLEAN:
                op1 = "imaster";
                break;

            case TP_FLOAT:
                op1 = "fmaster";
                break;

            default:
                op1 = "smaster";
                break;
        }

        switch (node->target_type) {
            case TP_INTEGER:
            case TP_BOOLEAN:
                op2 = "iprime";
                break;

            case TP_FLOAT:
                op2 = "fprime";
                break;

            default:
                op2 = "sprime";
                break;
        }

        snprintf(temp, buf_len, "   %s r%d\n   %s r%d\n",
                 op1,
                 node->register_num,
                 op2,
                 node->register_num);
        node->output3 = output_fs(temp);
        output_append(node->output, node->output3);
    }
}

static walker_result emit_walker(walker_direction direction,
                                  ASTNode* node,
                                  void *pl) {

    walker_payload *payload = (walker_payload*) pl;
    ASTNode *child1, *child2, *child3, *n;
    char *op;
    char *tp_prefix;
    OutputFragment *o;
    char temp1[buf_len];
    char temp2[buf_len];
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
                snprintf(temp1, buf_len, "/* REXX COMPILER PoC */\n"
                                         "\n"
                                         ".globals=0\n"
                                         "\n"
                                         "main()   .locals=%d\n",
                         node->scope->num_registers);
                node->output = output_fs(temp1);
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
                op="concat";
            case OP_SCONCAT:
                if (!op) op="sconcat";
                node->output = output_f();
                if (child1->output) output_append(node->output, child1->output);
                if (child2->output) output_append(node->output, child2->output);
                snprintf(temp1, buf_len, "   %s r%d,r%d,r%d\n",
                         op,
                         node->register_num,
                         child1->register_num,
                         child2->register_num);
                node->output2 = output_fs(temp1);
                output_append(node->output, node->output2);
                type_promotion(node);
            break;

            case OP_COMPARE_EQUAL:
                if (!op) op="eq";
            case OP_COMPARE_NEQ:
                if (!op) op="ne";
            case OP_COMPARE_GT:
                if (!op) op="gt";
            case OP_COMPARE_LT:
                if (!op) op="lt";
            case OP_COMPARE_GTE:
                if (!op) op="gte";
            case OP_COMPARE_LTE:
                if (!op) op="lte";
            case OP_COMPARE_S_EQ:
                if (!op) op="eqs";
            case OP_COMPARE_S_NEQ:
                if (!op) op="nes";
            case OP_COMPARE_S_GT:
                if (!op) op="gts";
            case OP_COMPARE_S_LT:
                if (!op) op="lts";
            case OP_COMPARE_S_GTE:
                if (!op) op="gtes";
            case OP_COMPARE_S_LTE:
                if (!op) op="ltes";
            case OP_AND:
                if (!op) op="and";
            case OP_OR:
                if (!op) op="or";
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
                snprintf(temp1, buf_len, "   %s%s r%d,r%d,r%d\n",
                         tp_prefix,
                         op,
                         node->register_num,
                         child1->register_num,
                         child2->register_num);
                node->output2 = output_fs(temp1);
                output_append(node->output, node->output2);
                type_promotion(node);
                break;

            case OP_PREFIX:
                node->output = output_f();
                if (child1->output) output_append(node->output, child1->output);
                snprintf(temp1, buf_len, "   prefix_todo r%d\n",
                         child1->register_num);
                node->output2 = output_fs(temp1);
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
            case STRING:
                /* TODO - X and B suffix */
                encode_print(temp2, buf_len, node->node_string + 1, node->node_string_length - 2);
                snprintf(temp1, buf_len, "   load r%d,\"%s\"\n",
                         node->register_num, temp2);
                node->output = output_fs(temp1);
                type_promotion(node);
                break;

            case FLOAT:
            case INTEGER:
                snprintf(temp1, buf_len, "   load r%d,%.*s\n",
                         node->register_num,
                         node->node_string_length,
                         node->node_string);
                node->output = output_fs(temp1);
                type_promotion(node);
                break;

            case ASSIGN:
                get_comment(comment,node, NULL);
                node->output = output_fs(comment);
                output_append(node->output, child2->output);
                if (child1->register_num != child2->register_num) {
                    snprintf(temp1, buf_len, "   copy r%d,r%d\n",
                             child1->register_num,
                             child2->register_num);
                    node->output2 = output_fs(temp1);
                    output_append(node->output, node->output2);
                }
                break;

            case ADDRESS:
                get_comment(comment,node,NULL);
                node->output = output_fs(comment);
                output_append(node->output, child1->output);
                snprintf(temp1, buf_len, "   address r%d\n",
                         node->register_num);
                node->output2 = output_fs(temp1);
                output_append(node->output, node->output2);
                break;

            case SAY:
                get_comment(comment,node, NULL);
                node->output = output_fs(comment);
                output_append(node->output, child1->output);
                snprintf(temp1, buf_len, "   ssay r%d\n   say \"\\n\"\n",
                         node->register_num);
                node->output2 = output_fs(temp1);
                output_append(node->output, node->output2);
                break;

            case IF:
                if (child2) child3 = child2->sibling;
                else child3 = NULL;
                get_comment(comment,child1, "{IF}");
                node->output = output_fs(comment);
                if (child1->output) output_append(node->output, child1->output);
                get_comment_line_number_only(comment,child2,"{THEN}");
                snprintf(temp1, buf_len, "   brf l%d,r%d\n%s",
                         node->node_number,
                         node->register_num,
                         comment);
                node->output2 = output_fs(temp1);
                output_append(node->output, node->output2);
                output_append(node->output,child2->output);
                if (child3) {
                    get_comment_line_number_only(comment,child3,"{ELSE}");
                    snprintf(temp1, buf_len, "   br l%d\n%sl%d:\n",
                             child3->node_number,
                             comment,
                             node->node_number);
                    node->output3 = output_fs(temp1);
                    output_append(node->output, node->output3);
                    output_append(node->output,child3->output);

                    snprintf(temp1, buf_len, "l%d:\n",
                             child3->node_number);
                    node->output4 = output_fs(temp1);
                    output_append(node->output, node->output4);
                }
                else {
                    snprintf(temp1, buf_len, "l%d:\n",
                             node->node_number);
                    node->output3 = output_fs(temp1);
                    output_append(node->output, node->output3);
                }
                break;

            case DO: /* DO LOOP */
                /* Loop Assignments REPEAT->output */
                get_comment_line_number_only(comment,child1, "{DO}");
                node->output = output_fs(comment);
                output_append(node->output, child1->output);

                /* Loop Start */
                snprintf(temp1, buf_len, "l%d:\n",
                         node->node_number);
                node->output2 = output_fs(temp1);
                output_append(node->output, node->output2);

                /* Loop Checks REPEAT->output2 */
                output_append(node->output, child1->output2);

                /* Loop Body - instructions */
                output_append(node->output,child2->output);

                /* Loop increments REPEAT->output3 */
                output_append(node->output, child1->output3);

                /* Loop End */
                get_comment_line_number_only(comment,child1, "{DO-END}");
                node->output3 = output_fs(comment);
                output_append(node->output, node->output3);
                snprintf(temp1, buf_len, "   br l%d\nl%d:\n",
                         node->node_number, child1->node_number);
                node->output4 = output_fs(temp1);
                output_append(node->output, node->output4);
                break;

            case REPEAT:
                node->output = output_f();
                output_append(node->output, child1->output);
                node->output2 = output_f();
                node->output3 = output_f();
                while (child2) {
                    if (child2->output) output_append(node->output, child2->output);
                    if (child2->output2) output_append(node->output2, child2->output2);
                    if (child2->output3) output_append(node->output3, child2->output3);
                    child2 = child2->sibling;
                }
                break;

            case TO:
                get_comment(comment,node, NULL);
                node->output = output_fs(comment);
                output_append(node->output, child1->output);

                node->output2 = output_fs(comment);
                snprintf(temp1, buf_len, "   %sgt r0,r%d,r%d\n   brt l%d,r0\n",
                         tp_prefix,
                         node->parent->register_num,
                         node->child->register_num,
                         node->parent->node_number);
                node->output4 = output_fs(temp1);
                output_append(node->output2, node->output4);
                break;

            case BY:
                if (child1) {
                    /* BY explicitly stated */
                    get_comment(comment, node, NULL);
                    node->output = output_fs(comment);
                    output_append(node->output, child1->output);

                    node->output3 = output_fs(comment);
                    snprintf(temp1, buf_len, "   %sadd r%d,r%d,r%d\n",
                             tp_prefix,
                             node->parent->register_num,
                             node->child->register_num,
                             node->parent->register_num);
                    node->output4 = output_fs(temp1);
                    output_append(node->output3, node->output4);
                }
                else {
                    /* BY Added implicitly - increment by 1 */
                    get_comment_line_number_only(comment, node->parent, "{Implicit \"BY 1\"}");

                    node->output3 = output_fs(comment);
                    if (*tp_prefix == 'i') {
                        snprintf(temp1, buf_len, "   inc r%d\n",
                                 node->parent->register_num);
                    }
                    else {
                        snprintf(temp1, buf_len, "   %sadd r%d,r%d,1.0\n",
                                 tp_prefix,
                                 node->parent->register_num,
                                 node->child->register_num,
                                 node->parent->register_num);
                    }
                    node->output4 = output_fs(temp1);
                    output_append(node->output3, node->output4);
                }
                break;

            default:;
        }

        if (node->scope) payload->current_scope = payload->current_scope->parent;
    }

    return result_normal;
}

void emit(Context *context, FILE *output) {
    walker_payload payload;

    payload.current_scope = 0;
    ast_wlkr(context->ast, register_walker, (void *) &payload);

    payload.file = output;
    payload.current_scope = 0;
    ast_wlkr(context->ast, emit_walker, (void *) &payload);
}

