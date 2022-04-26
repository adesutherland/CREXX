/*
 * Emit Assembler
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
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

/* Tests if a node is a constant */
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

/* Tests if a node is a symbol */
static int is_var_symbol(ASTNode* node) {
    if (node->symbol && node->node_type != FUNCTION) return 1;
    else return 0;
}

/* printf - but returns a malloced buffer with the result */
static char* printf_malloc(const char* format, ...) {
    char *buffer;
    size_t buffer_len;
    size_t needed_len;
    va_list argptr;

    /* Guess a length which is likely to be big enough */
    buffer_len = 100; /* A stab in the dark! */
    buffer = malloc(buffer_len);

    va_start(argptr, format);
    needed_len = vsnprintf(buffer, buffer_len, format, argptr) + 1;
    va_end(argptr);
    if (needed_len > buffer_len) {
        /* Buffer not big enough - do it again */
        buffer_len = needed_len;
        free(buffer);
        buffer = malloc(buffer_len);
        va_start(argptr, format);
        needed_len = vsnprintf(buffer, buffer_len, format, argptr) + 1;
        va_end(argptr);
    }
    return buffer;
}

/* Encodes a string to a buffer without overflow. Like snprintf() it returns the number of characters
 * that would have been written if the buffer had been big enough */
#define ADD_CHAR_TO_BUFFER(ch) {out_len++; if (buffer_len) { *(buffer++) = (ch); buffer_len--; }}
static size_t encode_print(char* buffer, size_t buffer_len, const char* string, size_t length) {

    size_t out_len = 0;
    if (!length) {
        *buffer = 0;
        return 0;
    }
    while (length) {
        switch (*string) {
            case '\\':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('\\')
                break;
            case '\n':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('n')
                break;
            case '\t':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('t')
                break;
            case '\a':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('a')
                break;
            case '\b':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('b')
                break;
            case '\f':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('f')
                break;
            case '\r':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('r')
                break;
            case '\v':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('v')
                break;
            case '\'':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('\'')
                break;
            case '\"':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('\"')
                break;
            case 0:
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('0')
                break;
            case '\?':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('?')
                break;
            default:
                ADD_CHAR_TO_BUFFER(*string)
        }
        string++;
        length--;
    }
    if (buffer_len) *buffer = 0;
    else *(buffer - 1) = 0;
    return out_len;
}

/* encode_print - but returns a malloced buffer with the result */
static char* encode_print_malloc(const char* string, size_t length) {
    char *buffer;
    size_t buffer_len;
    size_t needed_len;

    /* Guess a length which is likely to be big enough */
    buffer_len = (length * 2) + 1;
    buffer = malloc(buffer_len);

    needed_len = encode_print(buffer, buffer_len, string, length) + 1;
    if (needed_len > buffer_len) {
        /* Buffer not big enough - do it again */
        buffer_len = needed_len;
        free(buffer);
        buffer = malloc(buffer_len);
        encode_print(buffer, buffer_len, string, length);
    }
    return buffer;
}

/* Encodes a string to a buffer without buffer overflow - just handling line breaks etc for comment strings */
static size_t encode_comment(char* buffer, size_t buffer_len, const char* string, size_t length) {

    size_t out_len = 0;
    while (length) {
        switch (*string) {
            case '\n':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('n')
                break;
            case '\t':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('t')
                break;
            case '\f':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('f')
                break;
            case '\r':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('r')
                break;
            case 0:
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('0')
                break;
            default:
                ADD_CHAR_TO_BUFFER(*string)
        }
        string++;
        length--;
    }
    if (buffer_len) *buffer = 0;
    else *(buffer - 1) = 0;
    return out_len;
}

/* encode_comment - but returns a malloced buffer with the result */
static char* encode_comment_malloc(const char* string, size_t length) {
    char *buffer;
    size_t buffer_len;
    size_t needed_len;

    /* Guess a length which is likely to be big enough */
    buffer_len = (length * 2) + 1;
    buffer = malloc(buffer_len);

    needed_len = encode_comment(buffer, buffer_len, string, length) + 1;
    if (needed_len > buffer_len) {
        /* Buffer not big enough - do it again */
        buffer_len = needed_len;
        free(buffer);
        buffer = malloc(buffer_len);
        encode_comment(buffer, buffer_len, string, length);
    }
    return buffer;
}

/* Encodes a string to a buffer - stops at a line break or buffer end */
static size_t encode_line_source(char* buffer, size_t buffer_len, const char* string, size_t length) {

    size_t out_len = 0;
    while (length) {
        if (*string == '\n') break;
        switch (*string) {
            case '\"':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('\"')
                break;
            case '\t':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('t')
                break;
            case '\f':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('f')
                break;
            case '\r':
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('r')
                break;
            case 0:
                ADD_CHAR_TO_BUFFER('\\')
                ADD_CHAR_TO_BUFFER('0')
                break;
            default:
                ADD_CHAR_TO_BUFFER(*string)
        }
        string++;
        length--;
    }
    if (buffer_len) *buffer = 0;
    else *(buffer - 1) = 0;
    return out_len;
}

/* encode_line_source - but returns a malloced buffer with the result */
static char* encode_line_source_malloc(const char* string, size_t length) {
    char *buffer;
    size_t buffer_len;
    size_t needed_len;

    /* Guess a length which is likely to be big enough */
    buffer_len = (length * 2) + 1;
    buffer = malloc(buffer_len);

    needed_len = encode_line_source(buffer, buffer_len, string, length) + 1;
    if (needed_len > buffer_len) {
        /* Buffer not big enough - do it again */
        buffer_len = needed_len;
        free(buffer);
        buffer = malloc(buffer_len);
        encode_line_source(buffer, buffer_len, string, length);
    }
    return buffer;
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

static void output_concat(OutputFragment* before, OutputFragment* after) {
    if (before) {
        while (before->after) before = before->after;
    }
    if (after) {
        while (after->before) after = after->before;
    }
    if (before) before->after = after;
    if (after) after->before = before;
}

static void output_append_text(OutputFragment* before, char* after) {
    while (before->after) before = before->after;
    if (before->output) {
        before->output = realloc(before->output, strlen(before->output) + strlen(after) + 1);
        strcat(before->output, after);
    }
    else {
        before->output = malloc(strlen(after) + 1);
        strcpy(before->output, after);
    }
}

static void output_prepend_text(char* before, OutputFragment* after) {
    char* buffer;
    while (after->before) after = after->before;
    if (after->output) {
        buffer = malloc(strlen(after->output) + strlen(before) + 1);
        strcpy(buffer, before);
        strcat(buffer, after->output);
        free(after->output);
        after->output = buffer;
    }
    else {
        after->output = malloc(strlen(before) + 1);
        strcpy(after->output, before);
    }
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

            case TO:
            case BY:
            case UNTIL:
            case WHILE:
                /* Set the register number based on child */
                if (node->child) {
                    node->register_num = node->child->register_num;
                    node->register_type = node->child->register_type;
                }
                break;

            case FOR:
                if (!is_var_symbol(node->child)) {
                    /* Not a symbol - use the temp register */
                    node->register_num = node->child->register_num;
                    node->register_type = node->child->register_type;
                }
                /* Else new register for copy */
                else node->register_num = get_reg(payload->current_scope);
                break;

            case REPEAT:
                /* Set the register number to the assignment register number
                 * (if the assignment node exists) */
                c = child1;
                while (c) {
                    if (c->node_type == ASSIGN) {
                        node->register_num = c->register_num;
                        node->register_type = c->register_type;
                        break;
                    }
                    c = c->sibling;
                }
                break;

            case DO:
                /* We need to free temporary registers for the children of the
                 * REPEAT Node at this (the DO) level - because they need to retained
                 * while the do loop is in progress.
                 * Nodes TO/BY/FOR/UNTIL/WHILE or ASSIGN are under REPEAT (child1) */

                c = child1->child; /* The first child under the REPEAT */
                while (c) {
                    if (c->node_type == FOR) {
                        /* Always node register  */
                        ret_reg(payload->current_scope, c->register_num);
                    }
                    /* Don't do it for the ASSIGN node - it takes care of itself */
                    else if (c->node_type != ASSIGN && c->child) {
                        /* release the temporary register */
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

/* Returns the meta .src line in a malloced buffer */
static char* get_metaline(ASTNode *node) {
    char *result, *src;
    if (!node->source_start) {
        result = malloc(1);
        result[0] = 0;
    }
    else {
        src = encode_line_source_malloc(node->source_start,
                       (int) (node->source_end - node->source_start) + 1);
        result = printf_malloc("   .src %d:%d \"%s\"\n", node->line + 1, node->column, src);
        free(src);
    }
    return result;
}

/* Get Comment from a node (in a malloced buffer) */
static char* get_comment(ASTNode *node, char* prefix) {
    char *result, *src;
    if (!node->source_start) {
        result = malloc(1);
        result[0] = 0;
    }
    else {
        src = encode_comment_malloc(node->source_start,
                       (int) (node->source_end - node->source_start) + 1);
        if (prefix)
            result = printf_malloc("   * Line %d: %s %s\n", node->line + 1,
                     prefix, src);
        else
            result = printf_malloc("   * Line %d: %s\n", node->line + 1, src);
        free(src);
    }
    return result;
}

/* Comment without quoting node text (in a malloced buffer) */
static char* get_comment_line_number_only(ASTNode *node, char* prefix) {
    if (prefix)
        return printf_malloc("   * Line %d: %s\n", node->line + 1, prefix);
    else
        return printf_malloc("   * Line %d:\n", node->line + 1);
}

static void type_promotion(ASTNode *node) {

    char *op1, *op2;
    char *temp;

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
            temp = printf_malloc("   %sto%s %c%d\n",
                     op1,
                     op2,
                     node->register_type,
                     node->register_num);
            output_append_text(node->output, temp);
            free(temp);
        }
    }
}

/* Formats a constant value returend as a malloced buffer */
static char* format_constant(ValueType type, ASTNode* node) {
    char *buffer;
    int flag;
    size_t i;

    if (type == TP_STRING) {
        buffer = printf_malloc("\"%.*s\"",
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
            buffer = printf_malloc("%.*s.0",
                     node->node_string_length,
                     node->node_string);
        else
            buffer = printf_malloc("%.*s",
                     node->node_string_length,
                     node->node_string);
    }
    else {
        buffer = printf_malloc("%.*s",
                 node->node_string_length,
                 node->node_string);
    }
    return buffer;
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

/* Adds Register metadata */
void meta_reg_symbol(Symbol *symbol, void *payload) {
    ASTNode* value_node;
    OutputFragment *output = (OutputFragment*)payload;
    char* buffer;

    if (!symbol->is_function) {

        if (symbol->is_constant) {
            value_node = sym_trnd(symbol, 0)->node->sibling;
            buffer = printf_malloc("   .meta \"%s\" = \"B\" \"%s\" \"%.*s\"\n",
                                   symbol->name,
                                   type_nm(symbol->type),
                                   (int) value_node->node_string_length, value_node->node_string);
        }

        else if (symbol->register_num >= 0) {
            buffer = printf_malloc("   .meta %c%d = \"B\" \"%s\" \"%s\"\n",
                                   symbol->register_type, symbol->register_num,
                                   type_nm(symbol->type),
                                   symbol->name);
        }

        else return; /* No symbol information ... */

        /* Add the metadata to the output fragment */
        output_append_text(output,buffer);
        free(buffer);
    }
}

/* Returns the source code of a node in a malloced buffer with formatting removed / cleaned */
static char *clean_print_node(ASTNode *node) {
    ASTNode *n;
    Token *t;
    size_t buffer_len;
    char *buffer, *b;
    size_t i;

    /* Calculate required buffer length */
    buffer_len = 0;
    for  (t = node->token_start; t; t = t->token_next) {
        buffer_len += t->length + 1; /* +1 for space */
        if (t == node->token_end) break;
    }

    /* Empty Source Line */
    if (!buffer_len) {
        buffer = malloc(1);
        buffer[0] = 0;
        return buffer;
    }

    /* Create and write to buffer */
    b = buffer = malloc(buffer_len);
    for  (t = node->token_start; t; t = t->token_next) {
        if (t->token_type != TK_STRING)  {
            /* Upper case it - because we are REXX */
            for (i = 0; i < t->length; i++) {
                *(b++) = (char)toupper(t->token_string[i]);
            }
        }
        else {
            memcpy(b, t->token_string, t->length);
            b += t->length;
        }
        *(b++) = ' '; /* Add Space */
        if (t == node->token_end) break;
    }

    /* Turn the last space to a terminating null */
    *(--b) = 0;

    return buffer;
}
/* Returns the type of a node as a malloced buffer */
char* node_type(ASTNode *node) {
    char *buffer;
    ValueType type = node->value_type;

    if (type == TP_OBJECT) {
        buffer = clean_print_node(node);
        if (buffer[0]) return buffer;  /* I.e. not an empty line */
        else free(buffer); /* set to .OBJECT below */
    }
    buffer = malloc(sizeof(".BOOLEAN") + 1); /* Make it long enough for the longest option */
    switch (type) {
        case TP_BOOLEAN: strcpy(buffer, ".BOOLEAN"); break;
        case TP_INTEGER: strcpy(buffer, ".INT"); break;
        case TP_FLOAT:   strcpy(buffer, ".FLOAT"); break;
        case TP_STRING:  strcpy(buffer, ".STRING"); break;
        case TP_OBJECT:  strcpy(buffer, ".OBJECT"); break;
        default:         strcpy(buffer, ".VOID");
    }
    return buffer;
}

static walker_result emit_walker(walker_direction direction,
                                  ASTNode* node,
                                  void *pl) {

    walker_payload *payload = (walker_payload*) pl;
    ASTNode *child1, *child2, *child3, *n;
    char *op;
    char *tp_prefix;
    OutputFragment *o;
    char *temp1;
    char *temp2;
    char *comment;
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
            {
                char *buf = printf_malloc( "/*\n"
                                         " * cREXX COMPILER VERSION : %s\n"
                                         " * SOURCE                 : %.*s\n"
                                         " * BUILT                  : %d-%02d-%02d %02d:%02d:%02d\n"
                                         " */\n"
                                         "\n"
                                         ".srcfile \"%.*s\"\n"
                                         ".globals=0\n",
                         rxversion,
                         node->node_string_length, node->node_string,
                         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
                         node->node_string_length, node->node_string);


                node->output = output_fs(buf);
                free(buf);

                n = child1;
                while (n) {
                    if (n->output) output_concat(node->output, n->output);
                    n = n->sibling;
                }

                print_output(payload->file, node->output);
            }
            break;

            case PROCEDURE:
                if (!child3 || child3->node_type == NOP) {
                    /* A declaration - external */
                    char* type = node_type(child1);
                    char* source = clean_print_node(child2);
                    char* coded = encode_print_malloc(source, strlen(source));
                    char* buf = printf_malloc("\n%.*s() .expose=%.*s.%.*s\n"
                                              "   .meta %.*s() = \"B\" \"%s\" \"%s\"\n",
                                (int)node->node_string_length, node->node_string,
                                (int)node->node_string_length, node->node_string,
                                (int)node->node_string_length, node->node_string,
                                (int)node->node_string_length, node->node_string,
                                type,
                                coded
                             );
                    node->output = output_fs(buf);
                    free(type);
                    free(source);
                    free(coded);
                    free(buf);
                }
                else {
                    /* Definition */
                    char* type = node_type(child1);
                    char* source = clean_print_node(child2);
                    char* coded = encode_print_malloc(source, strlen(source));
                    char* buf = printf_malloc( "\n%.*s() .locals=%d .expose=%.*s.%.*s\n"
                                                "   .meta %.*s() = \"B\" \"%s\" \"%s\"\n",
                             (int)node->node_string_length, node->node_string,
                             (int)node->scope->num_registers,
                             (int)node->parent->node_string_length, node->parent->node_string,
                             (int)node->node_string_length, node->node_string,
                             (int)node->node_string_length, node->node_string,
                             type,
                             coded);
                    node->output = output_fs(buf);
                    free(type);
                    free(source);
                    free(coded);
                    free(buf);

                    if (node->scope) {
                        /* Add the Procedure's Symbols as metadata */
                        scp_4all(node->scope, meta_reg_symbol, node->output);
                    }

                    n = child1;
                    while (n) {
                        if (n->output) output_concat(node->output, n->output);
                        n = n->sibling;
                    }
                }
                break;

            case ARGS:
            case INSTRUCTIONS:
                node->output = output_f();
                n = child1;
                while (n) {
                    if (n->output) output_concat(node->output, n->output);
                    n = n->sibling;
                }
                break;

            case ARG:
                comment = get_comment(node, NULL);
                node->output = output_fs(comment);
                free(comment);

                char *meta_src = get_metaline(node);
                output_append_text(node->output, meta_src);
                free(meta_src);

                if (node->is_opt_arg) { /* Optional Argument */
                    /* If the register flag is set then an argument was specified */
                    temp1 = printf_malloc("   brtpandt l%da,%c%d,%d\n",
                             child1->node_number,
                             node->register_type,
                             node->register_num,
                             REGTP_VAL);
                    output_append_text(node->output, temp1);
                    free(temp1);

                    /* Set the default value */
                    output_concat(node->output, child2->output);

                    if (child1->register_num != child2->register_num ||
                        child1->register_type != child2->register_type) {
                        temp1 = printf_malloc("   copy %c%d,%c%d\n",
                                 child1->register_type,
                                 child1->register_num,
                                 child2->register_type,
                                 child2->register_num);
                        output_append_text(node->output, temp1);
                        free(temp1);
                    }

                    /* End of logic */
                    if (node->is_ref_arg) {
                        /* Reference so no copy needed */
                        temp1 = printf_malloc("l%da:\n", child1->node_number);
                        output_append_text(node->output, temp1);
                        free(temp1);
                    }
                    else {
                        /* Pass by value - so if the default is not used we may need to
                         * to do a copy - but check if the argument needs preserving */

                        /* Only worry about it if it is a big register */
                        if (node->value_type == TP_STRING || node->value_type == TP_OBJECT) {
                            temp1 = printf_malloc(
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
                            free(temp1);
                        }
                        else {
                            temp1 = printf_malloc("   br l%db\n"
                                                     "l%da:\n"
                                                     "   %scopy %c%d,%c%d\n"
                                                     "l%db:\n",
                                     child1->node_number, child1->node_number,
                                     tp_prefix, child1->register_type,
                                     child1->register_num,
                                     node->register_type, node->register_num,
                                     child1->node_number);
                            output_append_text(node->output, temp1);
                            free(temp1);
                        }
                    }
                }

                else if (!node->is_ref_arg) {
                    /* Copy by value so may need to do a copy - but check if the argument needs preserving */

                    /* Only worry about it if it is a big register */
                    if (node->value_type == TP_STRING || node->value_type == TP_OBJECT) {
                        temp1 = printf_malloc("   brtpandt l%dc,%c%d,%d\n"
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
                        free(temp1);
                    }
                    else {
                        /* Just need to copy register */
                        temp1 = printf_malloc("   %scopy %c%d,%c%d\n",
                                 tp_prefix, child1->register_type,
                                 child1->register_num,
                                 node->register_type, node->register_num);
                        output_append_text(node->output, temp1);
                        free(temp1);
                    }
                }
                break;

            case CALL:
                /* Comment */
                comment = get_comment(node, NULL);
                node->output = output_fs(comment);
                free(comment);
                /* TODO - set result */
                output_concat(node->output, child1->output);
                break;

            case FUNCTION:
                /* Return Registers */
                ret_type = node->register_type;
                ret_num = node->register_num;

                /* Comment */
                comment = get_comment(node, NULL);
                node->output = output_fs(comment);
                free(comment);

                /* Number of arguments */
                temp1 = printf_malloc("   load r%d,%d\n",
                         node->additional_registers,
                         node->num_additional_registers - 1);
                output_append_text(node->output, temp1);
                free(temp1);

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
                        temp1 = printf_malloc("   settp %c%d,%d\n",
                                 n->register_type,
                                 n->register_num,
                                 j);
                        output_append_text(n->output, temp1);
                        free(temp1);
                    }

                    if (n->register_type != 'r' ||  n->register_num != i) {
                        /* We need to swap registers to get it right for the call */
                        temp1 = printf_malloc("   swap r%d,%c%d\n",
                                 i, n->register_type, n->register_num);
                        output_append_text(n->output, temp1);
                        free(temp1);

                        /* Fix up return register so its swapped correctly */
                        if (node->register_type == n->register_type &&
                            node->register_num == n->register_num) {
                            ret_type = 'r';
                            ret_num = (int)i;
                        }
                    }

                    if (n->output) output_concat(node->output, n->output);
                    n = n->sibling; i++;
                }

                /* Actual Call */
                temp1 = printf_malloc("   call %c%d,%.*s(),r%d\n",
                         ret_type, ret_num,
                         node->node_string_length, node->node_string,
                         node->additional_registers);
                output_append_text(node->output, temp1);
                free(temp1);

                /* Step through for swapping registers back */
                n = child1;
                i = node->additional_registers + 1; /* First one is the number of arguments */
                while (n) {
                    if (n->register_num != i) {
                        /* We need to swap registers */
                        /* I have reversed arguments just for readability */
                        temp1 = printf_malloc("   swap %c%d,r%d\n",
                                 n->register_type, n->register_num,i);
                        output_append_text(node->output, temp1);
                        free(temp1);
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
                        output_concat(node->output, child2->output);
                    /* It MUST have been converted to a STRING
                     * We don't need to worry about ".0" to show a float literal */
                    temp1 = printf_malloc("   %s %c%d,\"%.*s\",%c%d\n",
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
                        output_concat(node->output, child1->output);
                    /* It MUST have been converted to a STRING
                     * We don't need to worry about ".0" to show a float literal */
                    temp1 = printf_malloc("   %s %c%d,%c%d,\"%.*s\"\n",
                             op,
                             node->register_type,
                             node->register_num,
                             child1->register_type,
                             child1->register_num,
                             child2->node_string_length, child2->node_string);
                }

                /* Neither are constants */
                else {
                    if (child1->output) output_concat(node->output, child1->output);
                    if (child2->output) output_concat(node->output, child2->output);
                    temp1 = printf_malloc("   %s %c%d,%c%d,%c%d\n",
                             op,
                             node->register_type,
                             node->register_num,
                             child1->register_type,
                             child1->register_num,
                             child2->register_type,
                             child2->register_num);
                }

                output_append_text(node->output, temp1);
                free(temp1);
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
                        output_concat(node->output, child2->output);
                    if (child1->target_type == TP_STRING) {
                        temp1 = printf_malloc("   %s%s %c%d,\"%.*s\",%c%d\n",
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
                            temp1 = printf_malloc("   %s%s %c%d,%.*s.0,%c%d\n",
                                     tp_prefix,
                                     op,
                                     node->register_type,
                                     node->register_num,
                                     child1->node_string_length, child1->node_string,
                                     child2->register_type,
                                     child2->register_num);
                        } else {
                            temp1 = printf_malloc("   %s%s %c%d,%.*s,%c%d\n",
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
                        temp1 = printf_malloc("   %s%s %c%d,%.*s,%c%d\n",
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
                        output_concat(node->output, child1->output);

                    if (child2->target_type == TP_STRING) {
                        temp1 = printf_malloc("   %s%s %c%d,%c%d,\"%.*s\"\n",
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
                            temp1 = printf_malloc("   %s%s %c%d,%c%d,%.*s.0\n",
                                     tp_prefix,
                                     op,
                                     node->register_type,
                                     node->register_num,
                                     child1->register_type,
                                     child1->register_num,
                                     child2->node_string_length, child2->node_string);
                        } else {
                            temp1 = printf_malloc("   %s%s %c%d,%c%d,%.*s\n",
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
                        temp1 = printf_malloc("   %s%s %c%d,%c%d,%.*s\n",
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
                        output_concat(node->output, child1->output);
                    if (child2->output)
                        output_concat(node->output, child2->output);
                    temp1 = printf_malloc("   %s%s %c%d,%c%d,%c%d\n",
                             tp_prefix,
                             op,
                             node->register_type,
                             node->register_num,
                             child1->register_type,
                             child1->register_num,
                             child2->register_type,
                             child2->register_num);
                }

                output_append_text(node->output, temp1);
                free(temp1);
                type_promotion(node);
                break;

            case OP_AND:
                node->output = output_f();
                output_concat(node->output, child1->output);
                if (node->register_num == child1->register_num &&
                    node->register_type == child1->register_type) {
                    /* If child1 and result are the same registers the logic
                     * is slightly shorter
                     *
                     * If result is false - we can just lazily set the result to false
                     * and not bother with the second expression */
                    temp1 = printf_malloc("   brf l%dandend,%c%d\n",
                             node->node_number,
                             child1->register_type,
                             child1->register_num);
                    output_append_text(node->output, temp1);
                    free(temp1);

                    /* Evaluate child2 */
                    output_concat(node->output, child2->output);

                    /* Result is child2's result */
                    if (! (node->register_num == child2->register_num &&
                           node->register_type == child2->register_type) ) {
                        temp1 = printf_malloc("   icopy %c%d,%c%d\n",
                                 node->register_type,
                                 node->register_num,
                                 child2->register_type,
                                 child2->register_num);
                        output_append_text(node->output, temp1);
                        free(temp1);
                    }

                    /* End of logic */
                    /* Result is already set */
                    temp1 = printf_malloc(
                             "l%dandend:\n",
                             node->node_number);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
                else {
                    /* If child1 and result are not the same registers the logic
                     * is slightly longer
                     *
                     * If result is false - we can just lazily set the result to false
                     * and not bother with the second expression */
                    temp1 = printf_malloc("   brf l%dandfalse,%c%d\n",
                             node->node_number,
                             child1->register_type,
                             child1->register_num);
                    output_append_text(node->output, temp1);
                    free(temp1);

                    /* Evaluate child2 */
                    output_concat(node->output, child2->output);

                    /* Result is child2's result & branch to end */
                    if (node->register_num == child2->register_num &&
                        node->register_type == child2->register_type) {
                        /* No need to copy if the registers are the same */
                        temp1 = printf_malloc("   br l%dandend\n", node->node_number);
                    }
                    else {
                        temp1 = printf_malloc("   icopy %c%d,%c%d\n   br l%dandend\n",
                                 node->register_type,
                                 node->register_num,
                                 child2->register_type,
                                 child2->register_num,
                                 node->node_number);
                    }
                    output_append_text(node->output, temp1);
                    free(temp1);

                    /* End of logic */
                    /* Result is 0/false */
                    temp1 = printf_malloc(
                             "l%dandfalse:\n   load %c%d,0\nl%dandend:\n",
                             node->node_number,
                             node->register_type,
                             node->register_num,
                             node->node_number);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
                type_promotion(node);
                break;

            case OP_OR:
                node->output = output_f();
                output_concat(node->output, child1->output);
                if (node->register_num == child1->register_num &&
                    node->register_type == child1->register_type) {
                    /* If child1 and result are the same registers the logic
                     * is slightly shorter
                     *
                     * If result is true - we can just lazily set the result to true
                     * and not bother with the second expression */
                    temp1 = printf_malloc("   brt l%dorend,%c%d\n",
                             node->node_number,
                             child1->register_type,
                             child1->register_num);
                    output_append_text(node->output, temp1);
                    free(temp1);

                    /* Evaluate child2 */
                    output_concat(node->output, child2->output);

                    /* Result is child2's result */
                    if (! (node->register_num == child2->register_num &&
                           node->register_type == child2->register_type) ) {
                        temp1 = printf_malloc("   icopy %c%d,%c%d\n",
                                 node->register_type,
                                 node->register_num,
                                 child2->register_type,
                                 child2->register_num);
                        output_append_text(node->output, temp1);
                        free(temp1);
                    }

                    /* End of logic */
                    /* Result is already set */
                    temp1 = printf_malloc(
                                 "l%dorend:\n",
                                 node->node_number);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
                else {
                    /* If child1 and result are not the same registers the logic
                     * is slightly longer
                     *
                     * If result is true - we can just lazily set the result to true
                     * and not bother with the second expression */
                    temp1 = printf_malloc("   brt l%dortrue,%c%d\n",
                             node->node_number,
                             child1->register_type,
                             child1->register_num);
                    output_append_text(node->output, temp1);
                    free(temp1);

                    /* Evaluate child2 */
                    output_concat(node->output, child2->output);

                    /* Result is child2's result & branch to end */
                    if (node->register_num == child2->register_num &&
                        node->register_type == child2->register_type) {
                        /* No need to copy if the registers are the same */
                        temp1 = printf_malloc("   br l%dorend\n", node->node_number);
                    }
                    else {
                        temp1 = printf_malloc("   icopy %c%d,%c%d\n   br l%dorend\n",
                                 node->register_type,
                                 node->register_num,
                                 child2->register_type,
                                 child2->register_num,
                                 node->node_number);
                    }
                    output_append_text(node->output, temp1);
                    free(temp1);

                    /* End of logic */
                    /* Result is 1/true */
                    temp1 = printf_malloc(
                                 "l%dortrue:\n   load %c%d,1\nl%dorend:\n",
                                 node->node_number,
                                 node->register_type,
                                 node->register_num,
                                 node->node_number);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
                type_promotion(node);
                break;

            case OP_NOT:
                node->output = output_f();
                if (child1->output) output_concat(node->output, child1->output);
                temp1 = printf_malloc("   not %c%d,%c%d\n",
                         node->register_type,
                         node->register_num,
                         child1->register_type,
                         child1->register_num);
                output_append_text(node->output, temp1);
                free(temp1);
                type_promotion(node);
                break;

            case OP_NEG:
                node->output = output_f();
                if (child1->output) output_concat(node->output, child1->output);
                if (node->value_type == TP_FLOAT) {
                    temp1 = printf_malloc("   fsub %c%d,0.0,%c%d\n",
                             node->register_type,
                             node->register_num,
                             child1->register_type,
                             child1->register_num);
                }
                else {
                    temp1 = printf_malloc("   isub %c%d,0,%c%d\n",
                             node->register_type,
                             node->register_num,
                             child1->register_type,
                             child1->register_num);
                }
                output_append_text(node->output, temp1);
                free(temp1);
                type_promotion(node);
                break;

            case OP_PLUS:
                node->output = output_f();
                if (child1->output) output_concat(node->output, child1->output);
                if (node->register_type != child1->register_type ||
                    node->register_num != child1->register_num) {
                    temp1 = printf_malloc("   %scopy %c%d,%c%d\n",
                             tp_prefix,
                             node->register_type,
                             node->register_num,
                             child1->register_type,
                             child1->register_num);
                }
                else {
                    temp1 = printf_malloc("   * \"+\" is a nop here\n");
                }
                output_append_text(node->output, temp1);
                free(temp1);
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
                    temp2 = format_constant(node->value_type, node);

                    /* Make the register load instruction */
                    temp1 = printf_malloc("   load %c%d,%s\n",
                             node->register_type,
                             node->register_num,
                             temp2);

                    /* Set the node output */
                    node->output = output_fs(temp1);
                    free(temp1);
                    free(temp2);

                    /* Do any type promotion */
                    type_promotion(node);
                }
                break;

            case ASSEMBLER: {
                char *arg1 = 0, *arg2 = 0, *arg3 = 0;

                comment = get_comment(node, NULL);
                node->output = output_fs(comment);
                free(comment);

                /* We will build the assembler instruction */
                /* First the command */
                char* inst = printf_malloc("   %.*s ",
                         node->node_string_length, node->node_string);

                /* Argument 1 */
                if (child1) {
                    if (child1->register_num == DONT_ASSIGN_REGISTER) { /* A constant */
                        arg1 = format_constant(child1->target_type, child1);
                    } else { /* A register */
                        output_concat(node->output, child1->output);
                        arg1 = printf_malloc("%c%d",
                                 child1->register_type,
                                 child1->register_num);
                    }
                }

                /* Argument 2 */
                if (child2) {
                    if (child2->register_num == DONT_ASSIGN_REGISTER) { /* A constant */
                        arg2 = format_constant(child2->target_type, child2);
                    } else { /* A register */
                        output_concat(node->output, child2->output);
                        arg2 = printf_malloc("%c%d",
                                             child2->register_type,
                                             child2->register_num);
                    }
                }

                /* Argument 3 */
                if (child3) {
                    if (child3->register_num == DONT_ASSIGN_REGISTER) { /* A constant */
                        arg3 = format_constant(child3->target_type, child3);
                    } else { /* A register */
                        output_concat(node->output, child3->output);
                        arg3 = printf_malloc("%c%d",
                                             child3->register_type,
                                             child3->register_num);
                    }
                }

                /* Create the whole instruction */
                if (arg3) temp1 = printf_malloc("%s %s,%s,%s\n", inst, arg1, arg2, arg3);
                else if (arg2) temp1 = printf_malloc("%s %s,%s\n", inst, arg1, arg2);
                else if (arg1) temp1 = printf_malloc("%s %s\n", inst, arg1);
                else temp1 = printf_malloc("%s\n", inst);

                /* Finally, append it to the output */
                output_append_text(node->output, temp1);

                /* Clean up */
                free(temp1);
                free(inst);
                if (arg1) free(arg1);
                if (arg2) free(arg2);
                if (arg3) free(arg3);
            }
            break;

            case ASSIGN:
                comment = get_comment(node, NULL);
                node->output = output_fs(comment);
                free(comment);
                output_concat(node->output, child2->output);
                if (child1->register_num != child2->register_num ||
                    child1->register_type != child2->register_type) {
                    temp1 = printf_malloc("   copy %c%d,%c%d\n",
                             child1->register_type,
                             child1->register_num,
                             child2->register_type,
                             child2->register_num);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
                break;

            case ADDRESS:
                comment = get_comment(node,NULL);
                node->output = output_fs(comment);
                free(comment);
                output_concat(node->output, child1->output);
                temp1 = printf_malloc("   address %c%d\n",
                         node->register_type,
                         node->register_num);
                output_append_text(node->output, temp1);
                free(temp1);
                break;

            case NOP:
                comment = get_comment(node, NULL);
                node->output = output_fs(comment);
                free(comment);
                break;

            case SAY:
                comment = get_comment(node, NULL);
                /* Child instructions */
                node->output = output_fs(comment);
                free(comment);
                if (child1->register_num == DONT_ASSIGN_REGISTER) {
                    /* If the register is not set then the child is a constant
                     * which we SAY directly. Get the constant string - target type */
                    temp2 = format_constant(child1->target_type, child1);
                    temp1 = printf_malloc("   say %s\n", temp2);
                    free(temp2);
                }
                else {
                    output_concat(node->output, child1->output);
                    temp1 = printf_malloc("   say %c%d\n",
                             child1->register_type,
                             child1->register_num);
                }
                output_append_text(node->output, temp1);
                free(temp1);

                break;

            case RETURN:
                comment = get_comment(node, NULL);
                /* Child instructions */
                node->output = output_fs(comment);
                free(comment);
                if (child1 == 0) {
                    temp1 = printf_malloc("   ret\n");
                }
                else if (child1->register_num == DONT_ASSIGN_REGISTER) {
                    /* If the register is not set then the child is a constant
                     * which we RET directly. Get the constant string - target type */
                    temp2 = format_constant(child1->target_type, child1);
                    temp1 = printf_malloc("   ret %s\n", temp2);
                    free(temp2);
                }
                else {
                    output_concat(node->output, child1->output);
                    temp1 = printf_malloc("   ret %c%d\n",
                             child1->register_type,
                             child1->register_num);
                }
                output_append_text(node->output, temp1);
                free(temp1);
                break;

            case IF:
                comment = get_comment(child1, "{IF}");
                node->output = output_fs(comment);
                free(comment);
                if (child1->output) output_concat(node->output, child1->output);
                comment = get_comment_line_number_only(child2,"{THEN}");
                temp1 = printf_malloc("   brf l%diffalse,%c%d\n%s",
                         node->node_number,
                         node->register_type,
                         node->register_num,
                         comment);
                output_append_text(node->output, temp1);
                free(temp1);
                free(comment);
                output_concat(node->output, child2->output);
                if (child3) {
                    comment = get_comment_line_number_only(child3,"{ELSE}");
                    temp1 = printf_malloc("   br l%difend\n%sl%diffalse:\n",
                             node->node_number,
                             comment,
                             node->node_number);
                    output_append_text(node->output, temp1);
                    free(temp1);
                    free(comment);
                    output_concat(node->output, child3->output);

                    temp1 = printf_malloc("l%difend:\n",
                             node->node_number);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
                else {
                    temp1 = printf_malloc("l%diffalse:\n",
                             node->node_number);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
                break;

            case DO: /* DO LOOP */
                /* Loop Assignments REPEAT->output */

                /* Loop output mapping / convention
                 * output =  Loop Assign / init instruction
                 * loopstartchecks = Loop iteration beginning exit checks
                 * loopinc = Loop iteration increments
                 * loopendchecks = Loop iteration end exit checks */

                comment = get_comment_line_number_only(child1, "{DO}");
                node->output = output_fs(comment);
                free(comment);
                /* Init */
                output_concat(node->output, child1->output);

                /* Loop Start */
                temp1 = printf_malloc("l%ddostart:\n",
                         node->node_number);
                output_append_text(node->output, temp1);
                free(temp1);

                /* Loop Begin Checks REPEAT->loopstartchecks */
                output_concat(node->output, child1->loopstartchecks);

                /* Loop Body - instructions */
                output_concat(node->output, child2->output);

                /* Loop End Checks REPEAT->loopendchecks */
                temp1 = printf_malloc("l%ddoinc:\n",
                         node->node_number);
                output_append_text(node->output, temp1);
                free(temp1);
                output_concat(node->output, child1->loopendchecks);

                /* Loop increments REPEAT->loopinc */
                output_concat(node->output, child1->loopinc);

                /* Loop End */
                comment = get_comment_line_number_only(child1, "{DO-END}");
                output_append_text(node->output, comment);
                temp1 = printf_malloc("   br l%ddostart\nl%ddoend:\n",
                         node->node_number, node->node_number);
                output_append_text(node->output, temp1);
                free(temp1);
                free(comment);
                break;

            case REPEAT:
                /* Loop output mapping / convention
                 * output =  Loop Assign / init instruction
                 * loopstartchecks = Loop iteration beginning exit checks
                 * loopinc = Loop iteration increments
                 * loopendchecks = Loop iteration end exit checks */
                node->output = output_f(); /* Assign / init instruction */
                node->loopstartchecks = output_f(); /* Begin Loop exit checks */
                node->loopinc = output_f(); /* Loop increments */
                node->loopendchecks = output_f(); /* End Loop exit checks */
                while (child1) {
                    if (child1->node_type == ASSIGN) {
                        /* Only output is valid - does not follow convention */
                        if (child1->output) output_concat(node->output, child1->output);
                    }
                    else {
                        if (child1->output)
                            output_concat(node->output, child1->output);
                        if (child1->loopstartchecks)
                            output_concat(node->loopstartchecks, child1->loopstartchecks);
                        if (child1->loopinc)
                            output_concat(node->loopinc, child1->loopinc);
                        if (child1->loopendchecks)
                            output_concat(node->loopendchecks, child1->loopendchecks);
                    }
                    child1 = child1->sibling;
                }
                break;

            case FOR:
                /* Loop output mapping / convention
                 * output =  Loop Assign / init instruction
                 * loopstartchecks = Loop iteration beginning exit checks
                 * loopinc = Loop iteration increments
                 * loopendchecks = Loop iteration end exit checks */
                comment = get_comment(node, NULL);
                node->output = output_fs(comment);
                free(comment);
                output_concat(node->output, child1->output);
                if (child1->register_num != node->register_num ||
                    child1->register_type != node->register_type) {
                    temp1 = printf_malloc("   icopy %c%d,%c%d\n",
                             node->register_type,
                             node->register_num,
                             child1->register_type,
                             child1->register_num);
                    output_append_text(node->output, temp1);
                    free(temp1);
                }
                node->loopstartchecks = output_fs(comment);
                temp1 = printf_malloc("   bcf l%ddoend,%c%d\n",
                         node->parent->parent->node_number,
                         node->register_type,
                         node->register_num);
                output_append_text(node->loopstartchecks, temp1);
                free(temp1);
                break;

            case TO:
                /* Loop output mapping / convention
                 * output =  Loop Assign / init instruction
                 * loopstartchecks = Loop iteration beginning exit checks
                 * loopinc = Loop iteration increments
                 * loopendchecks = Loop iteration end exit checks */

                comment = get_comment(node, NULL);
                node->output = output_fs(comment);
                output_concat(node->output, child1->output);

                /* Need to determine the sign of the BY */
                /* Find the BY */
                j = 1; /* J is the sign: 1 (default)=positive, -1=negative, 0=dynamic */
                n = node->parent->child; /* First sibling */
                while (n) {
                    if (n->node_type == BY) {
                        if (n->child) {
                            if (is_constant(n->child)) {
                                if (n->child->value_type == n->child->target_type) {
                                    /* Is a constant */
                                    if (n->child->value_type == TP_INTEGER) {
                                        if (n->child->int_value >= 0) j = 1;
                                        else j = -1;
                                    }
                                    else if (n->child->value_type == TP_FLOAT) {
                                        if (n->child->float_value >= 0.0) j = 1;
                                        else j = -1;
                                    }
                                    else j = 1;
                                }
                                else j = 0; /* Not a constant */
                            }
                            else j = 0; /* Not a constant */
                        }
                        else j = 1; /* Implicit by */
                        break;
                    }
                    n = n->sibling;
                }
                /* n is set the BY node (or NULL if n*/

                /* If the REPEAT has a TO it has an ASSIGN and its register
                 * number will have been set to the ASSIGN Variable */
                node->loopstartchecks = output_fs(comment);
                switch (j) {
                    case 1: /* Positive */
                        temp1 = printf_malloc("   %sgt r0,%c%d,%c%d\n   brt l%ddoend,r0\n", /* r0 - todo */
                                 tp_prefix,
                                 node->parent->register_type,
                                 node->parent->register_num,
                                 node->child->register_type,
                                 node->child->register_num,
                                 node->parent->parent->node_number);
                        break;
                    case -1: /* Negative */
                        temp1 = printf_malloc("   %slt r0,%c%d,%c%d\n   brt l%ddoend,r0\n", /* r0 - todo */
                                 tp_prefix,
                                 node->parent->register_type,
                                 node->parent->register_num,
                                 node->child->register_type,
                                 node->child->register_num,
                                 node->parent->parent->node_number);
                        break;
                    default: /* Dynamic by value */
                        /* We need a zero (int or flaot */
                        if (*tp_prefix == 'i') op = "0";
                        else op = "0.0";
                        temp1 = printf_malloc(
                                 "   %slt r0,%c%d,%s\n" /* Check the by value sign */
                                 "   brt l%ddoneg1,r0\n"    /* JMP to Negative BY (r0 - todo) */

                                 "   %sgt r0,%c%d,%c%d\n"   /* Pos BY */
                                 "   brtf l%ddoend,l%ddoneg2,r0\n" /* r0 - todo */

                                 "l%ddoneg1:\n"
                                 "   %slt r0,%c%d,%c%d\n"   /* Neg BY */
                                 "   brt l%ddoend,r0\n" /* r0 - todo */

                                 "l%ddoneg2:\n",

                                 tp_prefix,
                                 n->child->register_type,
                                 n->child->register_num,
                                 op,
                                 node->parent->parent->node_number,

                                 tp_prefix,
                                 node->parent->register_type,
                                 node->parent->register_num,
                                 node->child->register_type,
                                 node->child->register_num,
                                 node->parent->parent->node_number,
                                 node->parent->parent->node_number,

                                 node->parent->parent->node_number,
                                 tp_prefix,
                                 node->parent->register_type,
                                 node->parent->register_num,
                                 node->child->register_type,
                                 node->child->register_num,
                                 node->parent->parent->node_number,
                                 node->parent->parent->node_number);
                }
                output_append_text(node->loopstartchecks, temp1);
                free(temp1);
                free(comment);
                break;

            case BY:
                /* Loop output mapping / convention
                 * output =  Loop Assign / init instruction
                 * loopstartchecks = Loop iteration beginning exit checks
                 * loopinc = Loop iteration increments
                 * loopendchecks = Loop iteration end exit checks */

                /* If the REPEAT has a BY it has an ASSIGN and its register
                 * number will have been set to the ASSIGN Variable */

                if (child1) {
                    /* BY explicitly stated */
                    comment = get_comment(node, NULL);
                    node->output = output_fs(comment);
                    free(comment);
                    output_concat(node->output, child1->output);

                    node->loopinc = output_fs(comment);
                    temp1 = printf_malloc("   %sadd %c%d,%c%d,%c%d\n",
                             tp_prefix,
                             node->parent->register_type,
                             node->parent->register_num,
                             node->child->register_type,
                             node->child->register_num,
                             node->parent->register_type,
                             node->parent->register_num);
                    output_append_text(node->loopinc, temp1);
                    free(temp1);
                }
                else {
                    /* BY Added implicitly - increment by 1 */
                    comment = get_comment_line_number_only(node->parent, "{Implicit \"BY 1\"}");
                    node->loopinc = output_fs(comment);
                    free(comment);

                    if (*tp_prefix == 'i') {
                        temp1 = printf_malloc("   inc %c%d\n",
                                 node->parent->register_type,
                                 node->parent->register_num);
                    }
                    else {
                        temp1 = printf_malloc("   %sadd %c%d,%c%d,1.0\n",
                                 tp_prefix,
                                 node->parent->register_type,
                                 node->parent->register_num,
                                 node->parent->register_type,
                                 node->parent->register_num);
                    }
                    output_append_text(node->loopinc, temp1);
                    free(temp1);
                }
                break;

            case WHILE:
                /* Loop output mapping / convention
                 * output =  Loop Assign / init instruction
                 * loopstartchecks = Loop iteration beginning exit checks
                 * loopinc = Loop iteration increments
                 * loopendchecks = Loop iteration end exit checks */
                comment = get_comment(node, NULL);
                node->loopstartchecks = output_fs(comment);
                free(comment);
                output_concat(node->loopstartchecks, child1->output);
                temp1 = printf_malloc("   brf l%ddoend,%c%d\n",
                         node->parent->parent->node_number,
                         node->register_type,
                         node->register_num);
                output_append_text(node->loopstartchecks, temp1);
                free(temp1);
                break;

            case UNTIL:
                /* Loop output mapping / convention
                 * output =  Loop Assign / init instruction
                 * loopstartchecks = Loop iteration beginning exit checks
                 * loopinc = Loop iteration increments
                 * loopendchecks = Loop iteration end exit checks */
                comment = get_comment(node, NULL);
                node->loopendchecks = output_fs(comment);
                free(comment);
                output_concat(node->loopendchecks, child1->output);
                temp1 = printf_malloc("   brt l%ddoend,%c%d\n",
                         node->parent->parent->node_number,
                         node->register_type,
                         node->register_num);
                output_append_text(node->loopendchecks, temp1);
                free(temp1);
                break;

            case LEAVE:
                /* Leave Loop */
                comment = get_comment(node, NULL);
                node->output = output_fs(comment);
                free(comment);
                temp1 = printf_malloc("   br l%ddoend\n",
                         node->association->node_number);
                output_append_text(node->output, temp1);
                free(temp1);
                break;

            case ITERATE:
                /* Iterate Loop */
                comment = get_comment(node, NULL);
                node->output = output_fs(comment);
                free(comment);
                temp1 = printf_malloc("   br l%ddoinc\n",
                         node->association->node_number);
                output_append_text(node->output, temp1);
                free(temp1);
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

