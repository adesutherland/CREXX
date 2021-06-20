/* REXX ASSEMBLER */
/* The Disassembler */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "platform.h"
#include "rxas.h"
#include "rxvminst.h"
#include "rxdadism.h"

/* Encodes a string to a buffer. Like snprintf() it returns the number of characters
 * that would have been written */
static size_t encode_print(char* buffer, size_t buffer_len, char* string, size_t length) {

#define ADD_CHAR_TO_BUFFER(ch) {out_len++; if (buffer_len) { *(buffer++) = (ch); buffer_len--; }}

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
#undef ADD_CHAR_TO_BUFFER
}

/* Disassemble an operand
 *
 * Returns the number of characters that would have been written assuming the
 * buffer was big enough - like snprintf() */
static size_t disassemble_operand(bin_space *pgm, char* buffer, size_t buffer_len,
                                  OperandType type, int index, int globals, int locals) {

    size_t ix, i;
    char *c;
    size_t sz;
    size_t out_len = 0;
    int r;

    switch(type) {
        case OP_ID:
            out_len = snprintf(buffer, buffer_len, "lb_%x", (int)pgm->binary[index].index);
            break;
        case OP_REG:
            r = (int)pgm->binary[index].index;
            if (r < locals)
                out_len = snprintf(buffer, buffer_len, "r%d", r);
            else if (r < locals + globals)
                out_len = snprintf(buffer, buffer_len, "g%d /* aka r%d */", r - locals, r);
            else
                out_len = snprintf(buffer, buffer_len, "a%d /* aka r%d */", r - locals - globals, r);
            break;
        case OP_FUNC:
            out_len = snprintf(buffer, buffer_len, "%s()", ((proc_constant*)(pgm->const_pool
            + pgm->binary[index].index))->name);
            break;
        case OP_INT:
            out_len = snprintf(buffer, buffer_len, "%d", (unsigned int)pgm->binary[index].iconst);
            break;
        case OP_FLOAT:
            out_len = snprintf(buffer, buffer_len, "%f", pgm->binary[index].fconst);
            break;
        case OP_CHAR:
            out_len = snprintf(buffer, buffer_len, "\'%c\'", pgm->binary[index].cconst);
            break;
        case OP_STRING:
            ix = pgm->binary[index].index;
            c = ((string_constant *)(pgm->const_pool + ix))->string;
            sz = ((string_constant *)(pgm->const_pool + ix))->string_len;

            out_len++; if (buffer_len) { *(buffer++) = '\"'; buffer_len--; };
            i = encode_print(buffer, buffer_len, c, sz);
            out_len += i;
            i = i>buffer_len?buffer_len:i;
            buffer += i; buffer_len -= i;
            out_len++; if (buffer_len) { *(buffer++) = '\"'; buffer_len--; };
            if (buffer_len) *buffer = 0;
            break;
        default:
            out_len = snprintf(buffer, buffer_len, "*ERROR*");
            break;
    }
    return out_len;
}

/* Stores the output of pass 1 and 1b */
typedef struct code_line {
    enum {
        operand=0, normal, show_label, show_proc
    } flags;
    size_t proc_index;
    Instruction *inst;
    char *comment;
} code_line;


/* Disassembly is quite straightforward - just needs to look up the instructions
 * and print it out. However the complexity is to work out where and which labels
 * to include and where to put the procedure details.
 * So we have to run through the code and flag where to add label and procedure
 * details - in sum, 2 passes */
#define MAX_LINE_SIZE 5000 /* TODO */
void disassemble(bin_space *pgm, FILE *stream) {
    size_t i, j;
    chameleon_constant *entry;
    proc_constant* pentry;
    expose_proc_constant* eentry;
    char line_buffer[MAX_LINE_SIZE];
    size_t line_len;

    /* Prepare to Print Code */
    i = 0;

    /* calloc() so values will be zeroed */
    code_line *source = calloc(pgm->inst_size, sizeof(code_line));

    /* Pass 1 - Go through the code, decode and flag destination labels */
    while (i < pgm->inst_size) {
        j = i;
        int opcode = pgm->binary[i++].instruction.opcode;
        Instruction *inst = get_inst(opcode);

        if (inst->operands != pgm->binary[j].instruction.no_ops) {
            printf("BINARY ERROR - Instruction operand count mismatch @ 0x%.6x\n",(int)j);
        }

        switch(inst->operands) {
            case 0:
                break;
            case 1:
                /* Flag the destination (e.g. of a br) to be shown as a label in the listing */
                if (inst->op1_type == OP_ID) source[pgm->binary[i].index].flags = show_label;
                i++;
                break;
            case 2:
                if (inst->op1_type == OP_ID) source[pgm->binary[i].index].flags = show_label;
                i++;
                if (inst->op2_type == OP_ID) source[pgm->binary[i].index].flags = show_label;
                i++;
                break;
            case 3:
                if (inst->op1_type == OP_ID) source[pgm->binary[i].index].flags = show_label;
                i++;
                if (inst->op2_type == OP_ID) source[pgm->binary[i].index].flags = show_label;
                i++;
                if (inst->op3_type == OP_ID) source[pgm->binary[i].index].flags = show_label;
                i++;
                break;
            default: ;
        }
        source[j].inst = inst;
        source[j].comment = inst->desc;
        if (source[j].flags == operand) source[j].flags = normal;
    }

    /* Pass 1b - Print Constant Pool and add Proc entry flags to the code listing */
    fprintf(stream, "* CONSTANT POOL - Size 0x%x\n\n", (unsigned int)pgm->const_size);
    i = 0;
    while (i < pgm->const_size) {
        entry = (chameleon_constant *)(pgm->const_pool + i);
        switch(entry->type) {
            case STRING_CONST:
                encode_print(line_buffer, MAX_LINE_SIZE, ((string_constant*)entry)->string,
                             ((string_constant*)entry)->string_len);
                fprintf(stream, "* 0x%.6x STRING \"%s\"\n", (unsigned int)i, line_buffer);
                break;
            case PROC_CONST:
                if ( ((proc_constant*)entry)->start == SIZE_MAX ) {
                    fprintf(stream,
                            "* 0x%.6x PROC   %s() exposed from external\n",
                            (unsigned int) i,
                            ((proc_constant *) entry)->name
                    );
                }
                else {
                    fprintf(stream,
                            "* 0x%.6x PROC   %s() @ 0x%.6x (locals=%d)\n",
                            (unsigned int) i,
                            ((proc_constant *) entry)->name,
                            (unsigned int) ((proc_constant *) entry)->start,
                            ((proc_constant *) entry)->locals
                    );
                    source[((proc_constant *) entry)->start].flags = show_proc;
                    source[((proc_constant *) entry)->start].proc_index = i;
                }
                break;

            case EXPOSE_REG_CONST:
                fprintf(stream,
                        "* 0x%.6x EXPOSED-REG g%d <-> as %s\n",
                        (unsigned int) i,
                        ((expose_reg_constant *) entry)->global_reg,
                        ((expose_reg_constant *) entry)->index);
                break;

            case EXPOSE_PROC_CONST:
                pentry = (proc_constant *)(pgm->const_pool + ((expose_proc_constant*)entry)->procedure);

                if (((expose_proc_constant *)entry)->imported) {
                    fprintf(stream,
                            "* 0x%.6x EXPOSED-PROC %s() <-- as %s\n",
                            (unsigned int) i,
                            pentry->name,
                            ((expose_proc_constant *) entry)->index
                        );
                    }
                else {
                    fprintf(stream,
                            "* 0x%.6x EXPOSED-PROC %s() --> as %s\n",
                            (unsigned int) i,
                            pentry->name,
                            ((expose_proc_constant *) entry)->index
                    );
                }
                break;

            default:
                fprintf(stream, "* 0x%.6x UNKNOWN \n", (unsigned int)i);
        }

        i += entry->size_in_pool;
    }

    /* Pass 2a - Generate listing output - number of globals & header information */
    int globals = pgm->globals;
    int locals = 0;
    fprintf(stream, "\n* CODE SEGMENT - Size 0x%x\n", (unsigned int)pgm->inst_size);
    fprintf(stream, "\n.globals=%d\n", globals);

    /* Pass 2b - Exposed Registers and procedures exposed from external modules */
    i = 0;
    while (i < pgm->const_size) {
        entry = (chameleon_constant *)(pgm->const_pool + i);
        switch(entry->type) {
            case PROC_CONST:
                if ( ((proc_constant*)entry)->start == SIZE_MAX ) {
                    eentry = (expose_proc_constant *)(pgm->const_pool + ((proc_constant *) entry)->exposed);
                    fprintf(stream,
                            "%s() .expose=%s\n",
                            ((proc_constant *) entry)->name,
                            eentry->index
                    );
                }
                break;

            case EXPOSE_REG_CONST:
                fprintf(stream,
                        "g%d .expose=%s\n",
                        ((expose_reg_constant *) entry)->global_reg,
                        ((expose_reg_constant *) entry)->index
                );
                break;

            default: ;
        }

        i += entry->size_in_pool;
    }

    /* Pass 2c - The assembler code itself */
    i = 0;
    while (i < pgm->inst_size) {
        if (source[i].flags == show_proc) {
            pentry = (proc_constant *)(pgm->const_pool +
                                       source[i].proc_index);
            if (pentry->exposed == SIZE_MAX) {
                locals = pentry->locals;
                snprintf(line_buffer, MAX_LINE_SIZE, "%s()", pentry->name);
                fprintf(stream, "\n%-15s .locals=%d\n", line_buffer, locals);
                line_buffer[0] = 0;
            }
            else {
                eentry = (expose_proc_constant *)(pgm->const_pool +
                                             pentry->exposed);
                locals = pentry->locals;
                snprintf(line_buffer, MAX_LINE_SIZE, "%s()", pentry->name);
                fprintf(stream, "\n%-15s .locals=%d .expose=%s\n",
                        line_buffer,
                        locals,
                        eentry->index);
                line_buffer[0] = 0;
            }
        }
        else if (source[i].flags == show_label) {
            snprintf(line_buffer, MAX_LINE_SIZE, "lb_%x:", (unsigned int)i);
        }
        else line_buffer[0] = 0;
        fprintf(stream, "%-15s",line_buffer);
            j = i++;
            line_len = snprintf(line_buffer, MAX_LINE_SIZE, " %s ", source[j].inst->instruction);

            switch(source[j].inst->operands) {
                case 0:
                    break;
                case 1:
                    disassemble_operand(pgm, line_buffer + line_len, MAX_LINE_SIZE-line_len, source[j].inst->op1_type, i++, globals, locals);
                    break;
                case 2:
                    line_len += disassemble_operand(pgm, line_buffer + line_len, MAX_LINE_SIZE-line_len, source[j].inst->op1_type, i++, globals, locals);
                    line_len += snprintf(line_buffer + line_len, MAX_LINE_SIZE-line_len, ",");
                    disassemble_operand(pgm, line_buffer + line_len, MAX_LINE_SIZE-line_len, source[j].inst->op2_type, i++, globals, locals);
                    break;
                case 3:
                    line_len += disassemble_operand(pgm, line_buffer + line_len, MAX_LINE_SIZE-line_len, source[j].inst->op1_type, i++, globals, locals);
                    line_len += snprintf(line_buffer + line_len, MAX_LINE_SIZE-line_len, ",");
                    line_len += disassemble_operand(pgm, line_buffer + line_len, MAX_LINE_SIZE-line_len, source[j].inst->op2_type, i++, globals, locals);
                    line_len += snprintf(line_buffer + line_len, MAX_LINE_SIZE-line_len, ",");
                    disassemble_operand(pgm, line_buffer + line_len, MAX_LINE_SIZE-line_len, source[j].inst->op3_type, i++, globals, locals);
                    break;
                default:
                    snprintf(line_buffer + line_len, MAX_LINE_SIZE-line_len,"*INTERNAL_ERROR_NUM_OPS*");
            }
        fprintf(stream, "%-45s * 0x%.6x %s\n", line_buffer, (unsigned int)j, source[j].comment);
    }

    /* Free memory */
    free(source);
}



