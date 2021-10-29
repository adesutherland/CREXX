/*
 * Emit Assembler
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "platform.h"
#include "rxcpmain.h"
#include "rxcpbgmr.h"

#define UNSET_REGISTER (-1)
#define DONT_ASSIGN_REGISTER (-2)

/* Register Type Flag Byte Values */
/* Used for optional arguments ONLY
 * set (1) means the register has a specified value */
#define REGTP_VAL 1

/* Used for "pass be value" large (strings, objects) registers ONLY
 * set (2) means that it is not a symbol so does not need copying as even if it is
 * changed the caller will not use its original value
 * Note: Small registers (int, float) are always copied as this is fatser than
 *       setting and checking this flag anyway */
#define REGTP_NOTSYM 2

/* Tests if a node is not a constant */
static int is_constant(ASTNode* node) {
    switch (node->node_type) {
        case CONSTANT: /* This is what the optimiser changes all constants to */
        case CONST_SYMBOL: /* Should not be being used in the AST at this stage - but for safety */
        case STRING: /* This and following will exist if the optimiser has not been run */
        case FLOAT:
        case INTEGER:
            if (node->value_type == node->target_type)
                return 1; /* No type promotion - so a constant */
            else
                return 0; /* Type promotion needed - so not a constant */
        default:
            return 0;
    }
}

/* Tests if a node is not a constant */
static int is_var_symbol(ASTNode* node) {
    if (node->symbol && node->node_type != FUNCTION) return 1;
    else return 0;
}

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

static OutputFragment *output_fs(char* text) {
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

static void output_append_text(OutputFragment* before, char* after) {
    char *buffer;
    while (before->after) before = before->after;
    if (before->output) {
        buffer = malloc(strlen(before->output) + strlen(after) + 1);
        strcpy(buffer, before->output);
    }
    else {
        buffer = malloc(strlen(after) + 1);
        buffer[0] = 0;
    }
    strcat(buffer, after);
    free(before->output);
    before->output = buffer;
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
    ASTNode *child1, *child2, *child3, *c;
    int a, i;

    child1 = node->child;
    if (child1) child2 = child1->sibling;
    else child2 = NULL;
    if (child2) child3 = child2->sibling;
    else child3 = NULL;

    if (direction == in) {
        /* IN - TOP DOWN */
        if (node->scope) {
            payload->current_scope = node->scope;
        }
        switch (node->node_type) {
            case ARGS:
                /*
                 * Assign Argument registers to Arguments
                 */
                /* Loop through arguments setting the symbol register */
                c = node->child;
                a = 1;
                while (c) {
                    c->register_num = a;
                    c->register_type ='a';
                    if (c->is_ref_arg) {
                        /* Pass by reference - no copy so just use the 'a' register */
                        c->child->symbol->symbol->register_num = a;
                        c->child->symbol->symbol->register_type = 'a';
                    }
                    /* Otherwise, a normal register will be assigned to the symbol later */

                    a++;
                    c = c->sibling;
                }
                break;

            case ASSIGN:
                /*
                 * If an assignment from an expression (rather than a symbol) then
                 * then mark the register as don't assign (DONT_ASSIGN_REGISTER) so we can assign
                 * it to the target register on the way out (bottom up) and save
                 * a copy instruction
                 */
                if (!is_var_symbol(child2) || is_constant(child2))
                    child2->register_num = DONT_ASSIGN_REGISTER; /* DONT_ASSIGN_REGISTER Don't assign register */
                break;

            case ARG:
                /*
                 * If there is a default value (not CLASS node) and if it is
                 * from an expression then mark the register as don't assign
                 * (DONT_ASSIGN_REGISTER) so we can assign it to the target
                 * register on the way out (bottom up) and save a copy instruction
                 */
                if (child2->node_type != CLASS && (!is_var_symbol(child2) || is_constant(child2)))
                    child2->register_num = DONT_ASSIGN_REGISTER; /* DONT_ASSIGN_REGISTER Don't assign register */
                break;

            case FUNCTION:
                /* Additional Registers for Arguments - need to be assigned now */
                i = 0;
                c = child1;
                while (c) {
                    i++;
                    c = c->sibling;
                }
                node->num_additional_registers = i + 1;
                node->additional_registers = get_regs(payload->current_scope, node->num_additional_registers);

                /* The children register need to be assigned */
                c = child1;
                i = node->additional_registers + 1; /* First one is the number of arguments */
                while (c) {
                    /* DONT_ASSIGN_REGISTER if it makes sense to directly assign
                     * the register later from the call sequence of registers
                     * used in the call instruction
                     * 1. If it is a symbol with call by reference it may be possible to
                     *    assign the symbol to the right register
                     * In this case we try to set the symbol register */
                    if (c->symbol && c->is_ref_arg) {
                        /* If the register has not been assigned a register set it
                         * to the arguments register - later the node will therefore be giving
                         * this register too */

                        /* NOTE This does nothing as the symbols have always
                         * already been assigned :-( TODO to solve this */

                        if (c->symbol->symbol->register_num == UNSET_REGISTER)
                            c->symbol->symbol->register_num = i;
                    }

                     /* 2. If it is a non-symbol expression we set the register later */
                    else if (!is_var_symbol(c) || is_constant(c))
                        c->register_num = DONT_ASSIGN_REGISTER;

                    c = c->sibling;
                    i++;
                }
                break;

            case SAY:
                /*
                 * We do not need a register as we can "say" a constant directly
                 */
                if (is_constant(child1)) child1->register_num = DONT_ASSIGN_REGISTER;
                break;

            case RETURN:
                /*
                 * We do not need a register as we can "ret" a constant directly
                 */
                if (child1 && is_constant(child1)) child1->register_num = DONT_ASSIGN_REGISTER;
                break;

            case ASSEMBLER:
                /*
                * Constants do not need a register
                */
                if (child1 && is_constant(child1)) child1->register_num = DONT_ASSIGN_REGISTER;
                if (child2 && is_constant(child2)) child2->register_num = DONT_ASSIGN_REGISTER;
                if (child3 && is_constant(child3)) child3->register_num = DONT_ASSIGN_REGISTER;
                break;

                /* The order of the operands of these instructions are not order
                 * specific but the instructions only support operand 3 being a
                 * constant */
            case OP_COMPARE_EQUAL:
            case OP_COMPARE_NEQ:
            case OP_ADD:
            case OP_MULT:
                if (is_constant(child2)) child2->register_num = DONT_ASSIGN_REGISTER;
                else if (is_constant(child1)) {
                    /* We need to swap the two children round because the last one needs
                     * to be the constant */
                    child1->parent->child = child2;
                    child2 = child1;
                    child2->sibling = 0;
                    child1 = child2->parent->child;
                    child1->sibling = child2;

                    child2->register_num = DONT_ASSIGN_REGISTER;
                }
                break;

                /* The order of the operands of these instructions are significant
                 * however the instructions do not support both being a constant */
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
            case OP_MINUS:
            case OP_POWER:
            case OP_DIV:
            case OP_IDIV:
            case OP_MOD:
            case OP_CONCAT:
            case OP_SCONCAT:
                /* one or the other can be a constant - not both */
                if (is_constant(child1)) child1->register_num = DONT_ASSIGN_REGISTER;
                else if (is_constant(child2)) child2->register_num = DONT_ASSIGN_REGISTER;
                break;

            case OP_AND:
            case OP_OR:
                /*  These should not have constants if the optimiser has been run and
                  * anyway the instructions cannot accept constants
                  * But we do want this node and all children to have the
                  * same register if possible to about register copies */
                if (!is_var_symbol(child1)) child1->register_num = DONT_ASSIGN_REGISTER;
                if (!is_var_symbol(child2)) child2->register_num = DONT_ASSIGN_REGISTER;

            default:
                ;
        }
    }
    else {
        /* OUT - BOTTOM UP */
        switch (node->node_type) {

            /* The order of the operands if these instructions are not order
             * specific but the instructions only support operand 3 being a
             * constant */
            case OP_COMPARE_EQUAL:
            case OP_COMPARE_NEQ:
            case OP_ADD:
            case OP_MULT:

            /* The order of the operands of these instructions are significant
             * however the instructions do not support both being a constant */
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
            case OP_MINUS:
            case OP_POWER:
            case OP_DIV:
            case OP_IDIV:
            case OP_MOD:
            case OP_CONCAT:
            case OP_SCONCAT:

                /* If it is a temporary mark the register for reuse */
                if (!is_var_symbol(child1) && child1->register_num != DONT_ASSIGN_REGISTER)
                    ret_reg(payload->current_scope, child1->register_num);
                if (!is_var_symbol(child2) && child2->register_num != DONT_ASSIGN_REGISTER)
                    ret_reg(payload->current_scope, child2->register_num);

                /* Set result temporary register */
                if (node->register_num != DONT_ASSIGN_REGISTER)
                    /* DONT_ASSIGN_REGISTER means that the register number will be set later */
                    node->register_num = get_reg(payload->current_scope);
                break;

            case OP_AND:
            case OP_OR:
                /* What we try and do here is use the same register for the
                 * node and children to avoid copies */
                if (child1->register_num == DONT_ASSIGN_REGISTER ||
                    child2->register_num == DONT_ASSIGN_REGISTER) {
                    /* If we are assigning a register to either children we
                     * will assign to this node and children, overriding/ignoring
                     * any DONT_ASSIGN_REGISTER flag for this node */
                    node->register_num = get_reg(payload->current_scope);
                    if (child1->register_num == DONT_ASSIGN_REGISTER)
                        child1->register_num = node->register_num;
                    if (child2->register_num == DONT_ASSIGN_REGISTER)
                        child2->register_num = node->register_num;
                }
                else {
                    /* Else both children have a register assigned (and must be symbols)
                     * so just set the node's register */
                    if (node->register_num != DONT_ASSIGN_REGISTER)
                        /* DONT_ASSIGN_REGISTER means that the register number will be set later */
                        node->register_num = get_reg(payload->current_scope);
                }
                break;

            case OP_NOT:
            case OP_NEG:
            case OP_PLUS:
                /* If it is a temporary mark the register for reuse */
                if (!is_var_symbol(child1))
                    ret_reg(payload->current_scope, child1->register_num);

                /* Set result temporary register */
                if (node->register_num != DONT_ASSIGN_REGISTER)
                    /* DONT_ASSIGN_REGISTER means that the register number will be set later */
                    node->register_num = get_reg(payload->current_scope);
                break;

            case VAR_SYMBOL:
            case VAR_TARGET:
            case VAR_REFERENCE:
                /* Set the symbols register */
                if (node->symbol->symbol->register_num == UNSET_REGISTER)
                    node->symbol->symbol->register_num = get_reg(payload->current_scope);
                /* The node uses the symbol register number */
                node->register_num = node->symbol->symbol->register_num;
                node->register_type = node->symbol->symbol->register_type;
                break;

            case FLOAT:
            case INTEGER:
            case STRING:
            case CONSTANT:
            case CONST_SYMBOL:
                /* Set result temporary register */
                if (node->register_num != DONT_ASSIGN_REGISTER)
                    /* DONT_ASSIGN_REGISTER means that the register number will be set later (or is not needed) */
                    node->register_num = get_reg(payload->current_scope);
                break;

            case FUNCTION:
                /* Set result temporary register */
                if (node->register_num != DONT_ASSIGN_REGISTER)
                    /* DONT_ASSIGN_REGISTER means that the register number will be set later (or is not needed) */
                    node->register_num = get_reg(payload->current_scope);

                /* Assign additional Registers for arguments if assignment was deferred  */
                i = node->additional_registers + 1; /* First one is the number of arguments */
                c = child1;
                while (c) {
                    if (c->register_num == DONT_ASSIGN_REGISTER)
                        c->register_num = i;
                    i++;
                    c = c->sibling;
                }

                /* Free registers except where it has been given to a symbol */
                ret_reg(payload->current_scope, node->additional_registers); /* First one is the number of arguments */
                i = node->additional_registers + 1;
                c = child1;
                while (c) {
                    /* If it is a symbol with the same register as i don't return the register */
                    if ( !( is_var_symbol(c) &&
                            c->symbol->symbol->register_num == i &&
                            c->symbol->symbol->register_type == 'r') )
                        ret_reg(payload->current_scope, i);

                    i++;
                    c = c->sibling;
                }

                break;

            case ASSIGN:
                if (child2->register_num == DONT_ASSIGN_REGISTER) {
                    /* Marked earlier so set the register to the target register */
                    child2->register_num = child1->register_num;
                    child2->register_type = child1->register_type;
                }
                node->register_num = child1->register_num;
                node->register_type = child1->register_type;
                break;

            case ARG:
                if (child2->node_type != CLASS && child2->register_num == DONT_ASSIGN_REGISTER) {
                    /* Marked earlier so set the register to the target register */
                    child2->register_num = child1->register_num;
                    child2->register_type = child1->register_type;
                }
                break;

            case SAY:
                node->register_num = child1->register_num;
                node->register_type = child1->register_type;
                /* If a register is needed at all ... */
                if (node->register_num != DONT_ASSIGN_REGISTER) {
                    /* Then if it is a temporary mark the register for reuse */
                    if (!is_var_symbol(child1))
                        ret_reg(payload->current_scope, child1->register_num);
                }
                break;

            case RETURN:
                if (child1) {
                    node->register_num = child1->register_num;
                    node->register_type = child1->register_type;
                    /* If a register is needed at all ... */
                    if (node->register_num != DONT_ASSIGN_REGISTER) {
                        /* Then if it is a temporary mark the register for reuse */
                        if (!is_var_symbol(child1))
                            ret_reg(payload->current_scope,
                                    child1->register_num);
                    }
                }
                break;

            case ADDRESS:
            case IF:
                node->register_num = child1->register_num;
                node->register_type = child1->register_type;
                /* If it is a temporary mark the register for reuse */
                if (!is_var_symbol(child1))
                    ret_reg(payload->current_scope, child1->register_num);
                break;

            case REPEAT:
                node->register_num = child1->register_num;
                node->register_type = child1->register_type;
                break;

            case DO:
                /* We need to free temporary registers for the children
                 * TO/BY/FOR which is under REPEAT (child1) */
                c = child1->child->sibling; /* The second child under the REPEAT */
                while (c) {
                    if (c->child) {
                        c->register_num = c->child->register_num;
                        c->register_type = c->child->register_type;
                        if (!is_var_symbol(c->child))
                            ret_reg(payload->current_scope, c->register_num);
                    }
                    c = c->sibling;
                }
                break;

            /* NOTE than ASSEMBLER children should never have temporary registers
             * than need returning as they are either symbols or constants,
             * so we do not have a case ASSEMBLER: */
            default:;
        }

        if (node->scope) payload->current_scope = payload->current_scope->parent;
    }

    return result_normal;
}

#define buf_len 512

static void get_comment(char* comment, ASTNode *node, char* prefix) {
    char temp[buf_len];
    if (!node->source_start) {
        comment[0] = 0;
    }
    else {
        encode_comment(temp, buf_len, node->source_start,
                       (int) (node->source_end - node->source_start) + 1);
        if (prefix)
            snprintf(comment, buf_len, "   * Line %d: %s %s\n", node->line,
                     prefix, temp);
        else
            snprintf(comment, buf_len, "   * Line %d: %s\n", node->line, temp);
    }
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
                op1 = "i";
                break;

            case TP_FLOAT:
                op1 = "f";
                break;

            default:
                op1 = "s";
                break;
        }

        switch (node->target_type) {
            case TP_BOOLEAN:
                if (node->value_type == TP_FLOAT) op2 = "b";
                else op2 = "i";
                break;

            case TP_INTEGER:
                op2 = "i";
                break;

            case TP_FLOAT:
                op2 = "f";
                break;

            default:
                op2 = "s";
                break;
        }

        if (*op1 != *op2) { /* Check that there is a promotion (i.e. boolean / integer) */
            snprintf(temp, buf_len, "   %sto%s %c%d\n",
                     op1,
                     op2,
                     node->register_type,
                     node->register_num);
            node->output3 = output_fs(temp);
            output_append(node->output, node->output3);
        }
    }
}

/* Formats a constant value into buffer */
static void format_constant(char* buffer, ValueType type, ASTNode* node) {
    int flag;
    size_t i;

    if (type == TP_STRING) {
        snprintf(buffer, buf_len, "\"%.*s\"",
                 node->node_string_length,
                 node->node_string);
    }
    else if (type == TP_FLOAT) {
        /* Need to make sure the float literal has an ".0" (for the assembler) */
        flag = 1; /* Assume we should add .0 */
        for (i=0; i<node->node_string_length; i++) {
            if (node->node_string[i] == '.' || node->node_string[i] == 'e') {
                /* Already in a float format */
                flag = 0; /* don't add .0 */
                break;
            }
        }
        if (flag)
            snprintf(buffer, buf_len, "%.*s.0",
                     node->node_string_length,
                     node->node_string);
        else
            snprintf(buffer, buf_len, "%.*s",
                     node->node_string_length,
                     node->node_string);
    }
    else {
        snprintf(buffer, buf_len, "%.*s",
                 node->node_string_length,
                 node->node_string);
    }
}

static char* type_to_prefix(ValueType value_type) {
    switch (value_type) {
        case TP_BOOLEAN:
        case TP_INTEGER:
            return "i";
        case TP_STRING:
            return "s";
        case TP_FLOAT:
            return "f";
        default:
            return "";
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
    size_t i;
    int j, k;
    int flag;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char ret_type;
    int ret_num;

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
        if (child2) child3 = child2->sibling;
        else child3 = NULL;

        /* Operator and type prefix */
        op = 0;
        tp_prefix = type_to_prefix(node->value_type);

        switch (node->node_type) {

            case PROGRAM_FILE:
                snprintf(temp1, buf_len, "/*\n"
                                         " * cREXX COMPILER VERSION : %s\n"
                                         " * SOURCE                 : %.*s\n"
                                         " * BUILT                  : %d-%02d-%02d %02d:%02d:%02d\n"
                                         " */\n"
                                         "\n"
                                         ".globals=0\n",
                    rxversion,
                    node->node_string_length, node->node_string,
                    tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

                node->output = output_fs(temp1);
                n = child1;
                while (n) {
                    if (n->output) output_append(node->output, n->output);
                    n = n->sibling;
                }

                print_output(payload->file, node->output);
                break;

            case PROCEDURE:
                if (!child3 || child3->node_type == NOP) {
                    /* A declaration - external */
                    snprintf(temp1, buf_len, ""
                                             "\n"
                                             "%.*s() .expose=%.*s.%.*s\n", /* TODO */
                             node->node_string_length, node->node_string,
                             node->node_string_length, node->node_string,
                             node->node_string_length, node->node_string);
                    node->output = output_fs(temp1);
                }
                else {
                    /* Definition */
                    snprintf(temp1, buf_len, ""
                                             "\n"
                                             "%.*s() .locals=%d .expose=%.*s.%.*s\n",
                             node->node_string_length,
                             node->node_string,
                             node->scope->num_registers,
                             node->parent->node_string_length, node->parent->node_string,
                             node->node_string_length, node->node_string);
                    node->output = output_fs(temp1);

                    n = child1;
                    while (n) {
                        if (n->output) output_append(node->output, n->output);
                        n = n->sibling;
                    }
                }
                break;

            case ARGS:
            case INSTRUCTIONS:
                node->output = output_f();
                n = child1;
                while (n) {
                    if (n->output) output_append(node->output, n->output);
                    n = n->sibling;
                }
                break;

            case ARG:
                get_comment(comment, node, NULL);
                node->output = output_fs(comment);

                if (node->is_opt_arg) { /* Optional Argument */
                    /* If the register flag is set then an argument was specified */
                    snprintf(temp1, buf_len, "   brtpandt l%da,%c%d,%d\n",
                             child1->node_number,
                             node->register_type,
                             node->register_num,
                             REGTP_VAL);
                    output_append_text(node->output, temp1);

                    /* Set the default value */
                    output_append(node->output, child2->output);

                    if (child1->register_num != child2->register_num ||
                        child1->register_type != child2->register_type) {
                        snprintf(temp1, buf_len, "   copy %c%d,%c%d\n",
                                 child1->register_type,
                                 child1->register_num,
                                 child2->register_type,
                                 child2->register_num);
                        node->output2 = output_fs(temp1);
                        output_append(node->output, node->output2);
                    }

                    /* End of logic */
                    if (node->is_ref_arg) {
                        /* Reference so no copy needed */
                        snprintf(temp1, buf_len, "l%da:\n",
                                 child1->node_number);
                        node->output3 = output_fs(temp1);
                        output_append(node->output, node->output3);
                    }
                    else {
                        /* Pass by value - so if the default is not used we may need to
                         * to do a copy - but check if the argument needs preserving */

                        /* Only worry about it if it is a big register */
                        if (node->value_type == TP_STRING || node->value_type == TP_OBJECT) {
                            snprintf(temp1, buf_len,
                                     "   br l%dd\n"
                                            "l%da:\n"
                                            "   brtpandt l%dc,%c%d,%d\n"
                                            "   %scopy %c%d,%c%d\n"
                                            "   br l%dd\n"
                                            "l%dc:\n"
                                            "   swap %c%d,%c%d\n"
                                            "l%dd:\n",
                                     child1->node_number, child1->node_number,
                                     child1->node_number,
                                     node->register_type, node->register_num,
                                     REGTP_NOTSYM,
                                     tp_prefix,
                                     child1->register_type, child1->register_num,
                                     node->register_type, node->register_num,
                                     child1->node_number,
                                     child1->node_number,
                                     child1->register_type, child1->register_num,
                                     node->register_type, node->register_num,
                                     child1->node_number);
                            output_append_text(node->output, temp1);
                        }
                        else {
                            snprintf(temp1, buf_len, "   br l%db\n"
                                                     "l%da:\n"
                                                     "   %scopy %c%d,%c%d\n"
                                                     "l%db:\n",
                                     child1->node_number, child1->node_number,
                                     tp_prefix, child1->register_type,
                                     child1->register_num,
                                     node->register_type, node->register_num,
                                     child1->node_number);
                            node->output3 = output_fs(temp1);
                            output_append(node->output, node->output3);
                        }
                    }
                }

                else if (!node->is_ref_arg) {
                    /* Copy by value so may need to do a copy - but check if the argument needs preserving */

                    /* Only worry about it if it is a big register */
                    if (node->value_type == TP_STRING || node->value_type == TP_OBJECT) {
                        snprintf(temp1, buf_len, "   brtpandt l%dc,%c%d,%d\n"
                                                "   %scopy %c%d,%c%d\n"
                                                "   br l%dd\n"
                                                "l%dc:\n"
                                                "   swap %c%d,%c%d\n"
                                                "l%dd:\n",
                                 child1->node_number,
                                 node->register_type, node->register_num,
                                 REGTP_NOTSYM,
                                 tp_prefix,
                                 child1->register_type, child1->register_num,
                                 node->register_type, node->register_num,
                                 child1->node_number,
                                 child1->node_number,
                                 child1->register_type, child1->register_num,
                                 node->register_type, node->register_num,
                                 child1->node_number);
                        output_append_text(node->output, temp1);
                    }
                    else {
                        /* Just need to copy register */
                        snprintf(temp1, buf_len, "   %scopy %c%d,%c%d\n",
                                 tp_prefix, child1->register_type,
                                 child1->register_num,
                                 node->register_type, node->register_num);
                        output_append_text(node->output, temp1);
                    }
                }
                break;

            case CALL:
                /* Comment */
                get_comment(comment, node, NULL);
                node->output = output_fs(comment);
                /* TODO - set result */
                output_append(node->output,child1->output);
                break;

            case FUNCTION:
                /* Return Registers */
                ret_type = node->register_type;
                ret_num = node->register_num;

                /* Comment */
                get_comment(comment, node, NULL);
                node->output = output_fs(comment);

                /* Number of arguments */
                snprintf(temp1, buf_len, "   load r%d,%d\n",
                         node->additional_registers,
                         node->num_additional_registers - 1);
                output_append_text(node->output, temp1);

                /* Step through the arguments - evaluating them */
                n = child1;
                i = node->additional_registers + 1; /* First one is the number of arguments */
                while (n) {
                    k = 0; /* 1 if we need to settp */
                    j = 0; /* The required value of settp */

                    /* Used for "pass be value" large (strings, objects) registers ONLY
                     * set (2) means that it is not a symbol so its value does not need
                     * preserving */
                    if (!n->is_ref_arg &&
                        (n->target_type == TP_STRING || n->target_type == TP_OBJECT)) {
                        k = 1; /* This means we will settp */
                        if (!n->symbol) j = REGTP_NOTSYM; /* Mark it as not a symbol */
                    }

                    /* Optional arguments need to use the settp flag */
                    if (n->is_opt_arg) {
                        k = 1; /* means we have to settp */
                        if (n->node_type != NOVAL) {
                            /* If it is an optional parameter with a value we need to set the type flag */
                            j = j | REGTP_VAL;
                        }
                    }
                    if (k) { /* We need to settp */
                        snprintf(temp1, buf_len, "   settp %c%d,%d\n",
                                 n->register_type,
                                 n->register_num,
                                 j);
                        output_append_text(n->output, temp1);
                    }

                    if (n->register_type != 'r' ||  n->register_num != i) {
                        /* We need to swap registers to get it right for the call */
                        snprintf(temp1, buf_len, "   swap r%d,%c%d\n",
                                 i, n->register_type, n->register_num);
                        output_append_text(n->output, temp1);

                        /* Fix up return register so its swapped correctly */
                        if (node->register_type == n->register_type &&
                            node->register_num == n->register_num) {
                            ret_type = 'r';
                            ret_num = (int)i;
                        }
                    }

                    if (n->output) output_append(node->output, n->output);
                    n = n->sibling; i++;
                }

                /* Actual Call */
                snprintf(temp1, buf_len, "   call %c%d,%.*s(),r%d\n",
                         ret_type, ret_num,
                         node->node_string_length, node->node_string,
                         node->additional_registers);
                output_append_text(node->output, temp1);

                /* Step through for swapping registers back */
                n = child1;
                i = node->additional_registers + 1; /* First one is the number of arguments */
                while (n) {
                    if (n->register_num != i) {
                        /* We need to swap registers */
                        /* I have reversed arguments just for readability */
                        snprintf(temp1, buf_len, "   swap %c%d,r%d\n",
                                 n->register_type, n->register_num,i);
                        output_append_text(node->output, temp1);
                    }
                    n = n->sibling; i++;
                }

                type_promotion(node);
                break;

            case OP_CONCAT:
                op="concat";
            case OP_SCONCAT:
                if (!op) op="sconcat";
                node->output = output_f();
                /* One or other of the operands may be a constant */
                /* If the register is not set then the child is a constant */
                if (child1->register_num == DONT_ASSIGN_REGISTER) {
                    if (child2->output)
                        output_append(node->output, child2->output);
                    /* It MUST have been converted to a STRING
                     * We don't need to worry about ".0" to show a float literal */
                    snprintf(temp1, buf_len, "   %s %c%d,\"%.*s\",%c%d\n",
                                 op,
                                 node->register_type,
                                 node->register_num,
                                 child1->node_string_length, child1->node_string,
                                 child2->register_type,
                                 child2->register_num);
                }

                /* If the register is not set then the child is a constant */
                else if (child2->register_num == DONT_ASSIGN_REGISTER) {
                    if (child1->output)
                        output_append(node->output, child1->output);
                    /* It MUST have been converted to a STRING
                     * We don't need to worry about ".0" to show a float literal */
                    snprintf(temp1, buf_len, "   %s %c%d,%c%d,\"%.*s\"\n",
                             op,
                             node->register_type,
                             node->register_num,
                             child1->register_type,
                             child1->register_num,
                             child2->node_string_length, child2->node_string);
                }

                /* Neither are constants */
                else {
                    if (child1->output) output_append(node->output, child1->output);
                    if (child2->output) output_append(node->output, child2->output);
                    snprintf(temp1, buf_len, "   %s %c%d,%c%d,%c%d\n",
                             op,
                             node->register_type,
                             node->register_num,
                             child1->register_type,
                             child1->register_num,
                             child2->register_type,
                             child2->register_num);
                }

                node->output2 = output_fs(temp1);
                output_append(node->output, node->output2);
                type_promotion(node);
            break;

            /* These operators have a prefix type of that of the first child */
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

                tp_prefix = type_to_prefix(child1->target_type);

            /* These operators use the type prefix already set (i.e. of their type) */
            case OP_ADD:
                if (!op) op="add";
            case OP_MULT:
                if (!op) op="mult";
            case OP_MINUS:
                if (!op) op="sub";
            case OP_POWER:
                if (!op) op="pow";
            case OP_DIV:
            case OP_IDIV:
                if (!op) op="div";
            case OP_MOD:
                if (!op) op="mod";

                node->output = output_f();
                /* One or other of the operands may be a constant */
                /* If the register is not set then the child is a constant */
                if (child1->register_num == DONT_ASSIGN_REGISTER) {
                    if (child2->output)
                        output_append(node->output, child2->output);
                    if (child1->target_type == TP_STRING) {
                        snprintf(temp1, buf_len, "   %s%s %c%d,\"%.*s\",%c%d\n",
                                 tp_prefix,
                                 op,
                                 node->register_type,
                                 node->register_num,
                                 child1->node_string_length, child1->node_string,
                                 child2->register_type,
                                 child2->register_num);
                    }

                    else if (child2->value_type == TP_FLOAT) {
                        /* Need to make sure the float literal has an ".0" */
                        flag = 1; /* Assume we should add .0 */
                        for (i = 0; i < child1->node_string_length; i++) {
                            if (child1->node_string[i] == '.' ||
                                child1->node_string[i] == 'e') {
                                /* Already in a float format */
                                flag = 0; /* don't add .0 */
                                break;
                            }
                        }
                        if (flag) {
                            snprintf(temp1, buf_len, "   %s%s %c%d,%.*s.0,%c%d\n",
                                     tp_prefix,
                                     op,
                                     node->register_type,
                                     node->register_num,
                                     child1->node_string_length, child1->node_string,
                                     child2->register_type,
                                     child2->register_num);
                        } else {
                            snprintf(temp1, buf_len, "   %s%s %c%d,%.*s,%c%d\n",
                                     tp_prefix,
                                     op,
                                     node->register_type,
                                     node->register_num,
                                     child1->node_string_length, child1->node_string,
                                     child2->register_type,
                                     child2->register_num);
                        }
                    }

                    /* INTEGER */
                    else {
                        snprintf(temp1, buf_len, "   %s%s %c%d,%.*s,%c%d\n",
                                 tp_prefix,
                                 op,
                                 node->register_type,
                                 node->register_num,
                                 child1->node_string_length, child1->node_string,
                                 child2->register_type,
                                 child2->register_num);
                    }
                }

                /* If the register is not set then the child is a constant */
                else if (child2->register_num == DONT_ASSIGN_REGISTER) {
                    if (child1->output)
                        output_append(node->output, child1->output);

                    if (child2->target_type == TP_STRING) {
                        snprintf(temp1, buf_len, "   %s%s %c%d,%c%d,\"%.*s\"\n",
                                 tp_prefix,
                                 op,
                                 node->register_type,
                                 node->register_num,
                                 child1->register_type,
                                 child1->register_num,
                                 child2->node_string_length, child2->node_string);
                    }

                    else if (child2->value_type == TP_FLOAT) {
                        /* Need to make sure the float literal has an ".0" */
                        flag = 1; /* Assume we should add .0 */
                        for (i = 0; i < child2->node_string_length; i++) {
                            if (child2->node_string[i] == '.' ||
                                child2->node_string[i] == 'e') {
                                /* Already in a float format */
                                flag = 0; /* don't add .0 */
                                break;
                            }
                        }
                        if (flag) {
                            snprintf(temp1, buf_len, "   %s%s %c%d,%c%d,%.*s.0\n",
                                     tp_prefix,
                                     op,
                                     node->register_type,
                                     node->register_num,
                                     child1->register_type,
                                     child1->register_num,
                                     child2->node_string_length, child2->node_string);
                        } else {
                            snprintf(temp1, buf_len, "   %s%s %c%d,%c%d,%.*s\n",
                                     tp_prefix,
                                     op,
                                     node->register_type,
                                     node->register_num,
                                     child1->register_type,
                                     child1->register_num,
                                     child2->node_string_length, child2->node_string);
                        }
                    }

                    /* INTEGER */
                    else {
                        snprintf(temp1, buf_len, "   %s%s %c%d,%c%d,%.*s\n",
                                 tp_prefix,
                                 op,
                                 node->register_type,
                                 node->register_num,
                                 child1->register_type,
                                 child1->register_num,
                                 child2->node_string_length, child2->node_string);
                    }
                }

                /* Neither are constants */
                else {
                    if (child1->output)
                        output_append(node->output, child1->output);
                    if (child2->output)
                        output_append(node->output, child2->output);
                    snprintf(temp1, buf_len, "   %s%s %c%d,%c%d,%c%d\n",
                             tp_prefix,
                             op,
                             node->register_type,
                             node->register_num,
                             child1->register_type,
                             child1->register_num,
                             child2->register_type,
                             child2->register_num);
                }

                node->output2 = output_fs(temp1);
                output_append(node->output, node->output2);
                type_promotion(node);
                break;

            case OP_AND:
                node->output = output_f();
                output_append(node->output, child1->output);
                if (node->register_num == child1->register_num &&
                    node->register_type == child1->register_type) {
                    /* If child1 and result are the same registers the logic
                     * is slightly shorter
                     *
                     * If result is false - we can just lazily set the result to false
                     * and not bother with the second expression */
                    snprintf(temp1, buf_len, "   brf l%dandend,%c%d\n",
                             node->node_number,
                             child1->register_type,
                             child1->register_num);
                    node->output2 = output_fs(temp1);
                    output_append(node->output, node->output2);

                    /* Evaluate child2 */
                    output_append(node->output, child2->output);

                    /* Result is child2's result */
                    if (! (node->register_num == child2->register_num &&
                           node->register_type == child2->register_type) ) {
                        snprintf(temp1, buf_len, "   icopy %c%d,%c%d\n",
                                 node->register_type,
                                 node->register_num,
                                 child2->register_type,
                                 child2->register_num);
                        node->output3 = output_fs(temp1);
                        output_append(node->output, node->output3);
                    }

                    /* End of logic */
                    /* Result is already set */
                    snprintf(temp1, buf_len,
                             "l%dandend:\n",
                             node->node_number);
                    node->output4 = output_fs(temp1);
                    output_append(node->output, node->output4);
                }
                else {
                    /* If child1 and result are not the same registers the logic
                     * is slightly longer
                     *
                     * If result is false - we can just lazily set the result to false
                     * and not bother with the second expression */
                    snprintf(temp1, buf_len, "   brf l%dandfalse,%c%d\n",
                             node->node_number,
                             child1->register_type,
                             child1->register_num);
                    node->output2 = output_fs(temp1);
                    output_append(node->output, node->output2);

                    /* Evaluate child2 */
                    output_append(node->output, child2->output);

                    /* Result is child2's result & branch to end */
                    if (node->register_num == child2->register_num &&
                        node->register_type == child2->register_type) {
                        /* No need to copy if the registers are the same */
                        snprintf(temp1, buf_len, "   br l%dandend\n", node->node_number);
                    }
                    else {
                        snprintf(temp1, buf_len, "   icopy %c%d,%c%d\n   br l%dandend\n",
                                 node->register_type,
                                 node->register_num,
                                 child2->register_type,
                                 child2->register_num,
                                 node->node_number);
                    }
                    node->output3 = output_fs(temp1);
                    output_append(node->output, node->output3);

                    /* End of logic */
                    /* Result is 0/false */
                    snprintf(temp1, buf_len,
                             "l%dandfalse:\n   load %c%d,0\nl%dandend:\n",
                             node->node_number,
                             node->register_type,
                             node->register_num,
                             node->node_number);
                    node->output4 = output_fs(temp1);
                    output_append(node->output, node->output4);
                }
                type_promotion(node);
                break;

            case OP_OR:
                node->output = output_f();
                output_append(node->output, child1->output);
                if (node->register_num == child1->register_num &&
                    node->register_type == child1->register_type) {
                    /* If child1 and result are the same registers the logic
                     * is slightly shorter
                     *
                     * If result is true - we can just lazily set the result to true
                     * and not bother with the second expression */
                    snprintf(temp1, buf_len, "   brt l%dorend,%c%d\n",
                             node->node_number,
                             child1->register_type,
                             child1->register_num);
                    node->output2 = output_fs(temp1);
                    output_append(node->output, node->output2);

                    /* Evaluate child2 */
                    output_append(node->output, child2->output);

                    /* Result is child2's result */
                    if (! (node->register_num == child2->register_num &&
                           node->register_type == child2->register_type) ) {
                        snprintf(temp1, buf_len, "   icopy %c%d,%c%d\n",
                                 node->register_type,
                                 node->register_num,
                                 child2->register_type,
                                 child2->register_num);
                        node->output3 = output_fs(temp1);
                        output_append(node->output, node->output3);
                    }

                    /* End of logic */
                    /* Result is already set */
                    snprintf(temp1, buf_len,
                                 "l%dorend:\n",
                                 node->node_number);
                    node->output4 = output_fs(temp1);
                    output_append(node->output, node->output4);
                }
                else {
                    /* If child1 and result are not the same registers the logic
                     * is slightly longer
                     *
                     * If result is true - we can just lazily set the result to true
                     * and not bother with the second expression */
                    snprintf(temp1, buf_len, "   brt l%dortrue,%c%d\n",
                             node->node_number,
                             child1->register_type,
                             child1->register_num);
                    node->output2 = output_fs(temp1);
                    output_append(node->output, node->output2);

                    /* Evaluate child2 */
                    output_append(node->output, child2->output);

                    /* Result is child2's result & branch to end */
                    if (node->register_num == child2->register_num &&
                        node->register_type == child2->register_type) {
                        /* No need to copy if the registers are the same */
                        snprintf(temp1, buf_len, "   br l%dorend\n", node->node_number);
                    }
                    else {
                        snprintf(temp1, buf_len, "   icopy %c%d,%c%d\n   br l%dorend\n",
                                 node->register_type,
                                 node->register_num,
                                 child2->register_type,
                                 child2->register_num,
                                 node->node_number);
                    }
                    node->output3 = output_fs(temp1);
                    output_append(node->output, node->output3);

                    /* End of logic */
                    /* Result is 1/true */
                    snprintf(temp1, buf_len,
                                 "l%dortrue:\n   load %c%d,1\nl%dorend:\n",
                                 node->node_number,
                                 node->register_type,
                                 node->register_num,
                                 node->node_number);
                    node->output4 = output_fs(temp1);
                    output_append(node->output, node->output4);
                }
                type_promotion(node);
                break;

            case OP_NOT:
                node->output = output_f();
                if (child1->output) output_append(node->output, child1->output);
                snprintf(temp1, buf_len, "   not %c%d,%c%d\n",
                         node->register_type,
                         node->register_num,
                         child1->register_type,
                         child1->register_num);
                node->output2 = output_fs(temp1);
                output_append(node->output, node->output2);
                type_promotion(node);
                break;

            case OP_NEG:
                node->output = output_f();
                if (child1->output) output_append(node->output, child1->output);
                if (node->value_type == TP_FLOAT) {
                    snprintf(temp1, buf_len, "   fsub %c%d,0.0,%c%d\n",
                             node->register_type,
                             node->register_num,
                             child1->register_type,
                             child1->register_num);
                }
                else {
                    snprintf(temp1, buf_len, "   isub %c%d,0,%c%d\n",
                             node->register_type,
                             node->register_num,
                             child1->register_type,
                             child1->register_num);
                }
                node->output2 = output_fs(temp1);
                output_append(node->output, node->output2);
                type_promotion(node);
                break;

            case OP_PLUS:
                node->output = output_f();
                if (child1->output) output_append(node->output, child1->output);
                if (node->register_type != child1->register_type ||
                    node->register_num != child1->register_num) {
                    snprintf(temp1, buf_len, "   %scopy %c%d,%c%d\n",
                             tp_prefix,
                             node->register_type,
                             node->register_num,
                             child1->register_type,
                             child1->register_num);
                }
                else {
                    strncpy(temp1,"   * \"+\" is a nop here\n",buf_len);
                }
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

            case NOVAL:
                /* Set the node output as null */
                node->output = output_f();
                break;

            case CONSTANT:
            case CONST_SYMBOL:
            case STRING:
            case FLOAT:
            case INTEGER:
                /* If register is not set then the parent node will handle this
                 * as a constant - we just set the value as a string */
                if (node->register_num != DONT_ASSIGN_REGISTER) {
                    /* Get the constant string */
                    format_constant(temp2, node->value_type, node);

                    /* Make the register load instruction */
                    snprintf(temp1, buf_len, "   load %c%d,%s\n",
                             node->register_type,
                             node->register_num,
                             temp2);

                    /* Set the node output */
                    node->output = output_fs(temp1);

                    /* Do any type promotion */
                    type_promotion(node);
                }
                break;

            case ASSEMBLER:
                get_comment(comment,node, NULL);
                /* Child instructions */
                node->output = output_fs(comment);

                /* We will build temp1 to be the assembler instruction */
                /* First the command */
                snprintf(temp1, buf_len, "   %.*s ",
                         node->node_string_length ,node->node_string);

                /* Argument 1 */
                if (child1) {
                    if (child1->register_num == DONT_ASSIGN_REGISTER) { /* A constant */
                        format_constant(temp2, child1->target_type, child1);
                    }
                    else { /* A register */
                        output_append(node->output, child1->output);
                        snprintf(temp2, buf_len, "%c%d",
                                 child1->register_type,
                                 child1->register_num);
                    }
                    strcat(temp1, temp2);
                }

                /* Argument 2 */
                if (child2) {
                    strcat(temp1, ",");
                    if (child2->register_num == DONT_ASSIGN_REGISTER) { /* A constant */
                        format_constant(temp2, child2->target_type, child2);
                    }
                    else { /* A register */
                        output_append(node->output, child2->output);
                        snprintf(temp2, buf_len, "%c%d",
                                 child2->register_type,
                                 child2->register_num);
                    }
                    strcat(temp1, temp2);
                }

                /* Argument 3 */
                if (child3) {
                    strcat(temp1, ",");
                    if (child3->register_num == DONT_ASSIGN_REGISTER) { /* A constant */
                        format_constant(temp2, child3->target_type, child3);
                    }
                    else { /* A register */
                        output_append(node->output, child3->output);
                        snprintf(temp2, buf_len, "%c%d",
                                 child3->register_type,
                                 child3->register_num);
                    }
                    strcat(temp1, temp2);
                }
                /* End of Line */
                strcat(temp1, "\n");

                /* Finally, append our output */
                node->output2 = output_fs(temp1);
                output_append(node->output, node->output2);
                break;

            case ASSIGN:
                get_comment(comment,node, NULL);
                node->output = output_fs(comment);
                output_append(node->output, child2->output);
                if (child1->register_num != child2->register_num ||
                    child1->register_type != child2->register_type) {
                    snprintf(temp1, buf_len, "   copy %c%d,%c%d\n",
                             child1->register_type,
                             child1->register_num,
                             child2->register_type,
                             child2->register_num);
                    node->output2 = output_fs(temp1);
                    output_append(node->output, node->output2);
                }
                break;

            case ADDRESS:
                get_comment(comment,node,NULL);
                node->output = output_fs(comment);
                output_append(node->output, child1->output);
                snprintf(temp1, buf_len, "   address %c%d\n",
                         node->register_type,
                         node->register_num);
                node->output2 = output_fs(temp1);
                output_append(node->output, node->output2);
                break;

            case NOP:
                get_comment(comment,node, NULL);
                node->output = output_fs(comment);
                break;

            case SAY:
                get_comment(comment,node, NULL);
                /* Child instructions */
                node->output = output_fs(comment);
                if (child1->register_num == DONT_ASSIGN_REGISTER) {
                    /* If the register is not set then the child is a constant
                     * which we SAY directly. Get the constant string - target type */
                    format_constant(temp2, child1->target_type, child1);
                    snprintf(temp1, buf_len, "   say %s\n", temp2);
                }
                else {
                    output_append(node->output, child1->output);
                    snprintf(temp1, buf_len, "   say %c%d\n",
                             child1->register_type,
                             child1->register_num);
                }
                node->output2 = output_fs(temp1);
                output_append(node->output, node->output2);
                break;

            case RETURN:
                get_comment(comment,node, NULL);
                /* Child instructions */
                node->output = output_fs(comment);
                if (child1 == 0) {
                    snprintf(temp1, buf_len, "   ret\n");
                }
                else if (child1->register_num == DONT_ASSIGN_REGISTER) {
                    /* If the register is not set then the child is a constant
                     * which we RET directly. Get the constant string - target type */
                    format_constant(temp2, child1->target_type, child1);
                    snprintf(temp1, buf_len, "   ret %s\n", temp2);
                }
                else {
                    output_append(node->output, child1->output);
                    snprintf(temp1, buf_len, "   ret %c%d\n",
                             child1->register_type,
                             child1->register_num);
                }
                node->output2 = output_fs(temp1);
                output_append(node->output, node->output2);
                break;

            case IF:
                get_comment(comment,child1, "{IF}");
                node->output = output_fs(comment);
                if (child1->output) output_append(node->output, child1->output);
                get_comment_line_number_only(comment,child2,"{THEN}");
                snprintf(temp1, buf_len, "   brf l%diffalse,%c%d\n%s",
                         node->node_number,
                         node->register_type,
                         node->register_num,
                         comment);
                node->output2 = output_fs(temp1);
                output_append(node->output, node->output2);
                output_append(node->output,child2->output);
                if (child3) {
                    get_comment_line_number_only(comment,child3,"{ELSE}");
                    snprintf(temp1, buf_len, "   br l%difend\n%sl%diffalse:\n",
                             node->node_number,
                             comment,
                             node->node_number);
                    node->output3 = output_fs(temp1);
                    output_append(node->output, node->output3);
                    output_append(node->output,child3->output);

                    snprintf(temp1, buf_len, "l%difend:\n",
                             node->node_number);
                    node->output4 = output_fs(temp1);
                    output_append(node->output, node->output4);
                }
                else {
                    snprintf(temp1, buf_len, "l%diffalse:\n",
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
                snprintf(temp1, buf_len, "l%ddostart:\n",
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
                snprintf(temp1, buf_len, "   br l%ddostart\nl%ddoend:\n",
                         node->node_number, node->node_number);
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
                snprintf(temp1, buf_len, "   %sgt r0,%c%d,%c%d\n   brt l%ddoend,r0\n", /* r0 - todo */
                         tp_prefix,
                         node->parent->register_type,
                         node->parent->register_num,
                         node->child->register_type,
                         node->child->register_num,
                         node->parent->parent->node_number);
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
                    snprintf(temp1, buf_len, "   %sadd %c%d,%c%d,%c%d\n",
                             tp_prefix,
                             node->parent->register_type,
                             node->parent->register_num,
                             node->child->register_type,
                             node->child->register_num,
                             node->parent->register_type,
                             node->parent->register_num);
                    node->output4 = output_fs(temp1);
                    output_append(node->output3, node->output4);
                }
                else {
                    /* BY Added implicitly - increment by 1 */
                    get_comment_line_number_only(comment, node->parent, "{Implicit \"BY 1\"}");

                    node->output3 = output_fs(comment);
                    if (*tp_prefix == 'i') {
                        snprintf(temp1, buf_len, "   inc %c%d\n",
                                 node->parent->register_type,
                                 node->parent->register_num);
                    }
                    else {
                        snprintf(temp1, buf_len, "   %sadd %c%d,%c%d,1.0\n",
                                 tp_prefix,
                                 node->parent->register_type,
                                 node->parent->register_num,
                                 node->parent->register_type,
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

