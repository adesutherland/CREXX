/* REXX ASSEMBLER */
/* The Disassembler */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "platform.h"
#include "rxas.h"
#include "rxvminst.h"
#include "rxdadism.h"

/* Encodes a string to a buffer. Like snprintf() it returns the number of characters
 * that would have been written */
static size_t encode_print(char* buffer, size_t buffer_len, char* string, size_t length) {

#define ADD_CHAR_TO_BUFFER(ch) {out_len++; if (buffer_len) { *(buffer++) = (ch); buffer_len--; }}

    size_t out_len = 0;
    char hex_buffer[3];
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
                /* Should we escape this character? */
                if ((unsigned char)(*string) < 0x80) /* Not utf-8 */ {
                    if (isprint(*string)) {
                        ADD_CHAR_TO_BUFFER(*string) /* Normal Character */
                    }
                    else {
                        /* Escape as a hex character */
                        snprintf(hex_buffer, 3, "%02x", *string);
                        ADD_CHAR_TO_BUFFER('\\')
                        ADD_CHAR_TO_BUFFER('x')
                        ADD_CHAR_TO_BUFFER(hex_buffer[0])
                        ADD_CHAR_TO_BUFFER(hex_buffer[1])
                    }
                }
                else ADD_CHAR_TO_BUFFER(*string) /* Pass through UTF-8 */
        }
        string++;
        length--;
    }
    if (buffer_len) *buffer = 0;
    return out_len;
#undef ADD_CHAR_TO_BUFFER
}

/* Get the constant string
 * Returns the number of characters that would have been written assuming the
 * buffer was big enough - like snprintf() */
static size_t get_const_string(bin_space *pgm, char* buffer, size_t buffer_len, size_t ix) {

    size_t i;
    char *c;
    size_t sz;
    size_t out_len = 0;

    c = ((string_constant *)(pgm->const_pool + ix))->string;
    sz = ((string_constant *)(pgm->const_pool + ix))->string_len;

    out_len++; if (buffer_len) { *(buffer++) = '\"'; buffer_len--; }
    i = encode_print(buffer, buffer_len, c, sz);
    out_len += i;
    i = i>buffer_len?buffer_len:i;
    buffer += i; buffer_len -= i;
    out_len++; if (buffer_len) { *(buffer++) = '\"'; buffer_len--; }
    if (buffer_len) *buffer = 0;

    return out_len;
}

/* Get the function name string
 * Returns the number of characters that would have been written assuming the
 * buffer was big enough - like snprintf() */
static size_t get_func_string(bin_space *pgm, char* buffer, size_t buffer_len, size_t index) {
    return snprintf(buffer, buffer_len, "%s()", ((proc_constant *) (pgm->const_pool + index))->name);
}

/* Get the register name string
 * Returns the number of characters that would have been written assuming the
 * buffer was big enough - like snprintf() */
static size_t get_reg_string(bin_space *pgm, char* buffer, size_t buffer_len, int r, int globals, int locals) {
    size_t out_len;

    if (r < locals)
        out_len = snprintf(buffer, buffer_len, "r%d", r);
    else if (r < locals + globals)
        out_len = snprintf(buffer, buffer_len, "g%d /* aka r%d */", r - locals, r);
    else
        out_len = snprintf(buffer, buffer_len, "a%d /* aka r%d */", r - locals - globals, r);

    return out_len;
}

/* Disassemble an operand
 *
 * Returns the number of characters that would have been written assuming the
 * buffer was big enough - like snprintf() */
static size_t disassemble_operand(bin_space *pgm, char* buffer, size_t buffer_len,
                                  OperandType type, size_t index, int globals, int locals) {

    size_t out_len;
    size_t ix;
    int r;

    switch(type) {
        case OP_ID:
            out_len = snprintf(buffer, buffer_len, "lb_%x", (int)pgm->binary[index].index);
            break;
        case OP_REG:
            r = (int)pgm->binary[index].index;
            out_len = get_reg_string(pgm, buffer, buffer_len, r, globals, locals);
            break;
        case OP_FUNC:
            ix = pgm->binary[index].index;
            out_len = get_func_string(pgm, buffer, buffer_len, ix);
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
            out_len = get_const_string(pgm, buffer, buffer_len, ix);
            break;
        default:
            out_len = snprintf(buffer, buffer_len, "*INTERNAL_ERROR*");
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
 * and print it out. However, the complexity is to work out where and which labels
 * to include and where to put the procedure details.
 * So we have to run through the code and flag where to add label and procedure
 * details - in sum, 2 passes */

/* Max buffer size - todo change to a dynamic solution */
#define MAX_LINE_SIZE 5000

/* Get the address of the first meta constant entry at address (or -1 if none) */
static int get_first_meta_at(module_file *module, size_t address) {
    meta_entry *entry;
    int addr =  module->header.meta_head;
    while (addr != -1) {
        entry =  (meta_entry *)(module->constant +  addr);
        if (entry->address == address) return addr;
        else if (entry->address > address) return -1;
        addr = entry->next;
    }
    return -1;
}

/* Print Meta Data for a single procedure - an imported one */
static void output_imported_proc_meta(FILE *stream, module_file *module, bin_space *pgm, size_t func) {
    char line_buffer[MAX_LINE_SIZE];
    int  m = module->header.meta_head;
    while (m != -1) {
        switch ( ((chameleon_constant*)(module->constant + m))->type ) {

            case META_FUNC: {
                /* META function symbol - .meta "MAIN"="B" ".INT" main() "" "" */
                meta_func_constant *mentry = ((meta_func_constant *) (module->constant + m));
                if (mentry->func == func) {
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, ".meta %s=",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                    fprintf(stream, "%s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                    fprintf(stream, " %s",line_buffer);
                    get_func_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->func);
                    fprintf(stream, " %s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->args);
                    fprintf(stream, " %s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->inliner);
                    fprintf(stream, " %s\n",line_buffer);
                }
                m = mentry->base.next;
            }
            break;

            default: {
                meta_entry *mentry = ((meta_entry *) (module->constant + m));
                m = mentry->next;
            }
        }
    }
}

/* Print Meta Data for address starting a procedure - before the procedure header  */
static void output_meta_pre_proc(FILE *stream, module_file *module, bin_space *pgm, size_t address, int globals, int locals) {
    char line_buffer[MAX_LINE_SIZE];
    int  m;
    m  = get_first_meta_at(module, address);
    while (m != -1) {
        switch ( ((chameleon_constant*)(module->constant + m))->type ) {
            case META_SRC: {
                /* META Source */
                meta_src_constant *mentry = ((meta_src_constant *) (module->constant + m));
                if (mentry->base.address == address) {
                    m = mentry->base.next;
                }
                else m =  -1;
            }
            break;

            case META_FUNC: {
                /* META function symbol */
                meta_func_constant *mentry = ((meta_func_constant *) (module->constant + m));
                if (mentry->base.address == address) {
                    m = mentry->base.next;
                }
                else m =  -1;
            }
            break;

            case META_FILE: {
                /* META file - .srcfile="scratch" */
                meta_file_constant *mentry = ((meta_file_constant *) (module->constant + m));
                if (mentry->base.address == address) {
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->file);
                    fprintf(stream, ".srcfile=%s\n", line_buffer);
                    m = mentry->base.next;
                }
                else m =  -1;
            }
            break;

            case META_REG: {
                /* META clear symbol */
                meta_reg_constant *mentry = ((meta_reg_constant *) (module->constant + m));
                if (mentry->base.address == address) {
                    m = mentry->base.next;
                }
                else m =  -1;
            }
            break;

            case META_CONST: {
                /* META const symbol */
                meta_const_constant *mentry = ((meta_const_constant *) (module->constant + m));
                if (mentry->base.address == address) {
                    m = mentry->base.next;
                }
                else m =  -1;
            }
            break;

            case META_CLEAR: {
                /* META clear symbol - .meta "MAIN:I" */
                meta_clear_constant *mentry = ((meta_clear_constant *) (module->constant + m));
                if (mentry->base.address == address) {
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, "                .meta %s\n", line_buffer);
                    m = mentry->base.next;
                }
                else m =  -1;
            }
            break;

            default: /* Should never happen */
                m = -1;
        }
    }
}

/* Print Meta Data for address starting a procedure - after the procedure header  */
static void output_meta_post_proc(FILE *stream, module_file *module, bin_space *pgm, size_t address, int globals, int locals) {
    char line_buffer[MAX_LINE_SIZE];
    int  m;
    m  = get_first_meta_at(module, address);
    while (m != -1) {
        switch ( ((chameleon_constant*)(module->constant + m))->type ) {
            case META_SRC: {
                /* META Source */
                meta_src_constant *mentry = ((meta_src_constant *) (module->constant + m));
                if (mentry->base.address == address) {
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->source);
                    fprintf(stream, "                .src %d:%d=%s\n", (int) mentry->line, (int) mentry->column,
                            line_buffer);
                    m = mentry->base.next;
                }
                else m =  -1;
            }
            break;

            case META_FUNC: {
                /* META function symbol - .meta "MAIN"="B" ".INT" main() "" "" */

                meta_func_constant *mentry = ((meta_func_constant *) (module->constant + m));
                if (mentry->base.address == address) {

                    /* Only print procedure that are not imported  - because these are handled in the header */
                    if (((proc_constant *)(module->constant + mentry->func))->start != SIZE_MAX) {

                        get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                        fprintf(stream, "                .meta %s=", line_buffer);
                        get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                        fprintf(stream, "%s", line_buffer);
                        get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                        fprintf(stream, " %s", line_buffer);
                        get_func_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->func);
                        fprintf(stream, " %s", line_buffer);
                        get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->args);
                        fprintf(stream, " %s",line_buffer);
                        get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->inliner);
                        fprintf(stream, " %s\n",line_buffer);

                    }
                    m = mentry->base.next;
                }
                else m =  -1;
            }
            break;

            case META_FILE: {
                /* META file - .srcfile="scratch" */
                meta_file_constant *mentry = ((meta_file_constant *) (module->constant + m));
                if (mentry->base.address == address) {
                    m = mentry->base.next;
                }
                else m =  -1;
            }
            break;

            case META_REG: {
                /* META clear symbol - .meta "PROC:I"="B" ".INT" a1 */
                meta_reg_constant *mentry = ((meta_reg_constant *) (module->constant + m));
                if (mentry->base.address == address) {
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, "                .meta %s=",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                    fprintf(stream, "%s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                    fprintf(stream, " %s",line_buffer);
                    get_reg_string(pgm, line_buffer, MAX_LINE_SIZE, (int)mentry->reg, globals, locals);
                    fprintf(stream, " %s\n",line_buffer);

                    m = mentry->base.next;
                }
                else m =  -1;
            }
            break;

            case META_CONST: {
                /* META const symbol - .meta "MAIN:A"="B" ".STRING" "Hello" */
                meta_const_constant *mentry = ((meta_const_constant *) (module->constant + m));
                if (mentry->base.address == address) {
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, "                .meta %s=",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                    fprintf(stream, "%s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                    fprintf(stream, " %s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->constant);
                    fprintf(stream, " %s\n",line_buffer);

                    m = mentry->base.next;
                }
                else m =  -1;
            }
            break;

            case META_CLEAR: {
                /* META clear symbol - .meta "MAIN:I" */
                meta_clear_constant *mentry = ((meta_clear_constant *) (module->constant + m));
                if (mentry->base.address == address) {
                    m = mentry->base.next;
                }
                else m =  -1;
            }
            break;

            default: /* Should never happen */
                m = -1;
        }
    }
}

/* Print Meta Data for general address */
static void output_meta(FILE *stream, module_file *module, bin_space *pgm, size_t address, int globals, int locals) {
    char line_buffer[MAX_LINE_SIZE];
    int  m;
    m  = get_first_meta_at(module, address);
    while (m != -1) {
        switch ( ((chameleon_constant*)(module->constant + m))->type ) {
            case META_SRC: {
                /* META Source */
                meta_src_constant *mentry = ((meta_src_constant *) (module->constant + m));
                if (mentry->base.address == address) {
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->source);
                    fprintf(stream, "                .src %d:%d=%s\n", (int) mentry->line, (int) mentry->column,
                            line_buffer);
                    m = mentry->base.next;
                }
                else m =  -1;
            }
            break;

            case META_FUNC: {
                /* META function symbol - .meta "MAIN"="B" ".INT" main() "" "" */

                meta_func_constant *mentry = ((meta_func_constant *) (module->constant + m));
                if (mentry->base.address == address) {

                    /* Only print procedure that are not imported  - because these are handled in the header */
                    if (((proc_constant *)(module->constant + mentry->func))->start != SIZE_MAX) {

                        get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                        fprintf(stream, "                .meta %s=", line_buffer);
                        get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                        fprintf(stream, "%s", line_buffer);
                        get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                        fprintf(stream, " %s", line_buffer);
                        get_func_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->func);
                        fprintf(stream, " %s", line_buffer);
                        get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->args);
                        fprintf(stream, " %s",line_buffer);
                        get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->inliner);
                        fprintf(stream, " %s\n",line_buffer);

                    }
                    m = mentry->base.next;
                }
                else m =  -1;
            }
            break;

            case META_FILE: {
                /* META file - .srcfile="scratch" */
                meta_file_constant *mentry = ((meta_file_constant *) (module->constant + m));
                if (mentry->base.address == address) {
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->file);
                    fprintf(stream, "                .srcfile=%s\n", line_buffer);
                    m = mentry->base.next;
                }
                else m =  -1;
            }
            break;

            case META_REG: {
                /* META clear symbol - .meta "PROC:I"="B" ".INT" a1 */
                meta_reg_constant *mentry = ((meta_reg_constant *) (module->constant + m));
                if (mentry->base.address == address) {
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, "                .meta %s=",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                    fprintf(stream, "%s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                    fprintf(stream, " %s",line_buffer);
                    get_reg_string(pgm, line_buffer, MAX_LINE_SIZE, (int)mentry->reg, globals, locals);
                    fprintf(stream, " %s\n",line_buffer);

                    m = mentry->base.next;
                }
                else m =  -1;
            }
            break;

            case META_CONST: {
                /* META const symbol - .meta "MAIN:A"="B" ".STRING" "Hello" */
                meta_const_constant *mentry = ((meta_const_constant *) (module->constant + m));
                if (mentry->base.address == address) {
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, "                .meta %s=",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                    fprintf(stream, "%s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                    fprintf(stream, " %s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->constant);
                    fprintf(stream, " %s\n",line_buffer);

                    m = mentry->base.next;
                }
                else m =  -1;
            }
            break;

            case META_CLEAR: {
                /* META clear symbol - .meta "MAIN:I" */
                meta_clear_constant *mentry = ((meta_clear_constant *) (module->constant + m));
                if (mentry->base.address == address) {
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, "                .meta %s\n", line_buffer);
                    m = mentry->base.next;
                }
                else m =  -1;
            }
            break;

            default: /* Should never happen */
                m = -1;
        }
    }
}

/* Actual Disassembler */
void disassemble(bin_space *pgm, module_file *module, FILE *stream, int print_all_constant_pool) {
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

    /* Module Header */
    fprintf(stream, "*******************************************************************************\n"
                    "* MODULE - %s\n"
                    "* DESCRIPTION - %s\n\n", module->name, module->description );

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
    fprintf(stream, "* CONSTANT POOL - Size 0x%x.  ", (unsigned int)pgm->const_size);
    if (print_all_constant_pool) fprintf(stream, "Dump of all entries (option -p used):\n\n");
    else fprintf(stream, "Dump of EXPOSED entries only (option -p not used):\n\n");

    i = 0;
    while (i < pgm->const_size) {
        entry = (chameleon_constant *)(pgm->const_pool + i);
        switch(entry->type) {
            case STRING_CONST:
                if (print_all_constant_pool) {
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, i);
                    fprintf(stream, "* 0x%.6x STRING %s\n", (unsigned int) i, line_buffer);
                }
                break;
            case PROC_CONST:
                if (((proc_constant *) entry)->start == SIZE_MAX) {
                    if (print_all_constant_pool) {
                        fprintf(stream,
                                "* 0x%.6x PROC %s() exposed from external <-- %s\n",
                                (unsigned int) i,
                                ((proc_constant *) entry)->name,
                                ((expose_proc_constant*)(pgm->const_pool + ((proc_constant *) entry)->exposed))->index
                                );
                    }
                } else {
                    if (print_all_constant_pool) {
                        fprintf(stream,
                                "* 0x%.6x PROC @ 0x%.6x %s() (locals=%d)\n",
                                (unsigned int) i,
                                (unsigned int) ((proc_constant *) entry)->start,
                                ((proc_constant *) entry)->name,
                                ((proc_constant *) entry)->locals
                            );
                    }
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
                pentry = (proc_constant *) (pgm->const_pool + ((expose_proc_constant *) entry)->procedure);

                if (((expose_proc_constant *) entry)->imported) {
                    fprintf(stream,
                            "* 0x%.6x EXPOSED-PROC %s() <-- as %s\n",
                            (unsigned int) i,
                            pentry->name,
                            ((expose_proc_constant *) entry)->index
                    );
                } else {
                    fprintf(stream,
                            "* 0x%.6x EXPOSED-PROC %s() --> as %s\n",
                            (unsigned int) i,
                            pentry->name,
                            ((expose_proc_constant *) entry)->index
                    );
                }
                break;

            case META_SRC:
                if (print_all_constant_pool) {
                    meta_src_constant *mentry = (meta_src_constant *) entry;
                    fprintf(stream, "* 0x%.6x META-SRC @0x%.6x %d:%d ",
                            (unsigned int) i, (unsigned int) mentry->base.address,
                            (int) mentry->line, (int) mentry->column);

                    /* Source */
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->source);
                    fprintf(stream, "%s\n", line_buffer);
                }
                break;

            case META_FUNC:
                if (print_all_constant_pool) {
                    /* META function symbol */
                    meta_func_constant *mentry = (meta_func_constant *)entry;
                    fprintf(stream, "* 0x%.6x META-FUNC @0x%.6x", (unsigned int) i, (unsigned int) mentry->base.address);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, " %s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                    fprintf(stream, " %s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                    fprintf(stream, " %s",line_buffer);
                    get_func_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->func);
                    fprintf(stream, " %s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->args);
                    fprintf(stream, " %s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->inliner);
                    fprintf(stream, " %s\n",line_buffer);

                }
                break;

            case META_FILE:
                if (print_all_constant_pool) {
                    /* META file */
                    meta_file_constant *mentry = (meta_file_constant *)entry;
                    fprintf(stream, "* 0x%.6x META-FILE @0x%.6x", (unsigned int) i, (unsigned int) mentry->base.address);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->file);
                    fprintf(stream, " %s\n", line_buffer);
                }
                break;

            case META_REG:
                if (print_all_constant_pool) {
                    /* META clear symbol */
                    meta_reg_constant *mentry = (meta_reg_constant *)entry;
                    fprintf(stream, "* 0x%.6x META-REG @0x%.6x", (unsigned int) i, (unsigned int) mentry->base.address);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, " %s=",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                    fprintf(stream, " %s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                    fprintf(stream, " %s",line_buffer);
                    fprintf(stream, " r%d\n",(int)mentry->reg);
                }
                break;

            case META_CONST:
                if (print_all_constant_pool) {
                    /* META const symbol */
                    meta_const_constant *mentry = (meta_const_constant *)entry;
                    fprintf(stream, "* 0x%.6x META-CONST @0x%.6x", (unsigned int) i, (unsigned int) mentry->base.address);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, " %s=",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->option);
                    fprintf(stream, " %s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->type);
                    fprintf(stream, " %s",line_buffer);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->constant);
                    fprintf(stream, " %s\n",line_buffer);
                }
                break;

            case META_CLEAR:
                if (print_all_constant_pool) {
                    /* META clear symbol */
                    meta_clear_constant *mentry = (meta_clear_constant *) entry;
                    fprintf(stream, "* 0x%.6x META-CLEAR @0x%.6x", (unsigned int) i, (unsigned int) mentry->base.address);
                    get_const_string(pgm, line_buffer, MAX_LINE_SIZE, mentry->symbol);
                    fprintf(stream, " %s\n", line_buffer);
                }
                break;

            default:
                fprintf(stream, "* 0x%.6x UNKNOWN\n", (unsigned int)i);
        }

        i += entry->size_in_pool;
    }

    /* Pass 2a - Generate listing output - number of globals & header information */
    int globals = pgm->globals;
    int locals = 0;
    fprintf(stream, "\n* CODE SEGMENT - Size 0x%x\n", (unsigned int)pgm->inst_size);
    fprintf(stream, "\n.globals=%d\n", globals);

    /* Pass 2b - Exposed Registers and procedures exposed from external modules
     * Note that the compiler does not put all these at the top - but hey this is the easiest way for us */
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
                    output_imported_proc_meta(stream, module, pgm, i);
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
            output_meta_pre_proc(stream, module, pgm, i, globals, locals);
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
            output_meta_post_proc(stream, module, pgm, i, globals, locals);
        }
        else if (source[i].flags == show_label) {
            snprintf(line_buffer, MAX_LINE_SIZE, "lb_%x:", (unsigned int)i);
            output_meta(stream, module, pgm, i, globals, locals);
        }
        else {
            line_buffer[0] = 0;
            output_meta(stream, module, pgm, i, globals, locals);
        }

        j = i++;
        fprintf(stream, "%-15s",line_buffer);
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
        fprintf(stream, "%-45s * 0x%.6x:%.4x %s\n", line_buffer, (unsigned int)j, source[j].inst->opcode, source[j].comment);
    }

    /* Final Meta entries at the code address + 1 */
    output_meta(stream, module, pgm, i, globals, locals);

    fprintf(stream, "*******************************************************************************\n\n");

    /* Free memory */
    free(source);
}



