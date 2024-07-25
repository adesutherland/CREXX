/* REXX ASSEMBLER               */
/* The Assembler itself         */
#include <string.h>
#include <stdint.h>
#include "platform.h"
#include "rxasassm.h"
#include "rxvminst.h"

#include "rxastree.h"
#ifndef NUTF8
#include "utf.h"
#endif

/* Structure to handle "backpatching" - fixing forward references */
struct backpatching_references;
struct backpatching {
    int defined;
    size_t index;
//    Assembler_Token *def_token;
    struct backpatching_references *refs;
};

struct backpatching_references {
    size_t index;
    Assembler_Token *token;
    struct backpatching_references *link;
};

/* Frees Assembler Work Data */
void freeasbl(Assembler_Context *context) {
    if (context->string_constants_tree) free_tree(&context->string_constants_tree);
    if (context->proc_constants_tree) free_tree(&context->proc_constants_tree);
    if (context->label_constants_tree) free_tree(&context->label_constants_tree);
    if (context->extern_constants_tree) free_tree(&context->extern_constants_tree);
    if (context->extern_regs) free(context->extern_regs);
}

/* Backpatch Procedures, check references and free backpatch information */
static void backpatch_procedures(Assembler_Context *context) {
    struct string_wrapper *i;
    struct backpatching *patch;
    struct backpatching_references *ref, *nextref;

    /* Procedures - walk and process the tree */
    avl_tree_for_each_in_postorder(i, context->proc_constants_tree,
                                   struct string_wrapper, index_node) {
        patch = (struct backpatching*)(i->value);

        if (patch->defined == 0) {
            ref = patch->refs;
            while(ref) {
                rxaserat(context, ref->token, "unknown procedure");
                ref = ref->link;
            }
        }

        ref = patch->refs;
        while(ref) {
            nextref = ref->link;
            free(ref);
            ref = nextref;
        }
        free(patch);
    }
}

/* This finds the label backpatch index that branch source belongs to */
static struct backpatching* find_patch_index(Assembler_Context *context, size_t source) {
    struct string_wrapper *i;
    struct backpatching *patch;
    struct backpatching_references *ref;

    avl_tree_for_each_in_postorder(i, context->label_constants_tree,
                                       struct string_wrapper, index_node) {
        patch = (struct backpatching *) (i->value);
        ref = patch->refs;
        while (ref) {
            if (ref->index == source) return patch;
            ref = ref->link;
        }
    }
    return 0;
}

/* Optimise Labels - optimising branches to an unconditional branch by
 * converting the destination to go to the destination of the second branch */
static void optimise_labels(Assembler_Context *context) {
    struct string_wrapper *i;
    struct backpatching *patch, *p;

    Instruction *br = src_inst("br", OP_ID, OP_NONE, OP_NONE);
    int changed = 1;

    /* Labels - walk and process the tree of labels */
    while (changed) {
        changed = 0;
        avl_tree_for_each_in_postorder(i, context->label_constants_tree,
                                       struct string_wrapper, index_node) {
            patch = (struct backpatching *) (i->value);

            if (patch->defined != 0) { /* Note that unknown Symbol Errors handled later */

                /* Check if the destination is an unconditional branch */
                if (context->binary.binary[patch->index].instruction.opcode == br->opcode) {
                    /* Yes - find the patch index for this jump source */
                    p = find_patch_index(context, patch->index + 1);
                    if (p && p != patch && p->defined) {
                        /* if found, and if not self referring, and if defined */
                        /* Otherwise, an error - the optimiser should duck out */
                        patch->index = p->index; /* make the target of this label the next
                                                  * branch target */
                        changed = 1;
                    }
                }
            }
        }
    }
}

/* Backpatch Labels, check references and free backpatch information */
static void backpatch_labels(Assembler_Context *context) {
    struct string_wrapper *i;
    struct backpatching *patch;
    struct backpatching_references *ref, *nextref;

    /* Labels - walk and process the tree */
    avl_tree_for_each_in_postorder(i, context->label_constants_tree,
                                   struct string_wrapper, index_node) {
        patch = (struct backpatching*)(i->value);

        if (patch->defined == 0) {
            ref = patch->refs;
            while(ref) {
                rxaserat(context, ref->token, "unknown label");
                ref = ref->link;
            }
        }
        else {
            ref = patch->refs;
            while(ref) {
                context->binary.binary[ref->index].index = patch->index;
                ref = ref->link;
            }
        }

        ref = patch->refs;
        while(ref) {
            nextref = ref->link;
            free(ref);
            ref = nextref;
        }
        free(patch);
    }
}

/* Backpatch, check references and free backpatch information */
void backptch(Assembler_Context *context) {
    if (context->optimise) optimise_labels(context);
    backpatch_procedures(context);
    backpatch_labels(context);
}

/* Convert one hex digit to an int (-1 = error)*/
static int hexchar2int(char hexbyte) {
    int val = -1;

    // transform hex character to the 4bit equivalent number
    if (hexbyte >= '0' && hexbyte <= '9') val = hexbyte - '0';
    else if (hexbyte >= 'a' && hexbyte <='f') val = hexbyte - 'a' + 10;
    else if (hexbyte >= 'A' && hexbyte <='F') val = hexbyte - 'A' + 10;

    return val;
}
/* Unescape a string in place - returns the new string length */
static size_t unescape_string(char *to, char* from) {
    char *c, *d, *x;
    int h, hex;
    c = from;
    d = to;
    while (*c) {
        if (*c == '\\') {
            switch (*(++c)) {
                case '\\': *d = '\\'; break;
                case 'n': *d = '\n'; break;
                case 't': *d = '\t'; break;
                case 'a': *d = '\a'; break;
                case 'b': *d = '\b'; break;
                case 'f': *d = '\f'; break;
                case 'r': *d = '\r'; break;
                case 'v': *d = '\v'; break;
                case '\'': *d = '\''; break;
                case '\"': *d = '\"'; break;
                case '0': *d = '\0'; break;
                case '?': *d = '\?'; break;
                case 'x':
                    /* We support a simplified hex sequences \xhh - for single byte hex codes only */
                    x = c + 1;
                    hex = hexchar2int(*x);
                    if (hex != -1) { /* valid */
                        h = hex;
                        x++;
                        hex = hexchar2int(*x);
                        if (hex != -1) { /* valid */
                            h = (h << 4) | (hex & 0xF);
                        }
                    }
                    if (hex == -1) {
                        /* format error */
                        *d = '\\';
                        d++;
                        *d = *c;
                    }
                    else {
                        /* OK */
                        *d = (char)h;
                        c += 2;
                    }
                    break;
                case 0:
                    /* Escape sequence at end of string*/
                    *d = '\\';
                    d++;
                    /* Goto to break out of the switch() in a while() - sigh */
                    goto bad_escape_at_end_of_string;
                default:
                    /* unknown / invalid escape sequence */
                    *d = '\\';
                    d++;
                    *d = *c;
            }
        }
        else *d = *c;
        d++; c++;
    }
    bad_escape_at_end_of_string:
    *d = 0;
    return d - to;
}

/* Reserves space in the constant pool for an entry and returns its index;
 * the caller can then populate the entry.
 * NOTE - THIS CALL MIGHT MOVE THE CONSTANT POOL - CHANGING ENTRY ADDRESSES (USE OFFSETS!)
 * The 'size' parameter is the size of the payload including
 * space for chameleon_constant etc.
 * Returns the index to the entry (from binary.const_pool)
 */
static size_t reserve_in_const_pool(Assembler_Context *context, size_t size,
                                    enum const_pool_type type) {
    size_t index, new_size;
    chameleon_constant * entry;

    /* Extend the buffer if we need to */
    while (size + 8 > context->const_buffer_size - context->binary.const_size) { // +8 for the 8 bit alignment
        new_size = context->const_buffer_size * 2;
        context->binary.const_pool = realloc(context->binary.const_pool, new_size);
        if (!context->binary.const_pool) {
            fprintf(stderr, "PANIC: Out of memory\n");
            exit(-1);
        }
        memset(context->binary.const_pool + context->const_buffer_size, 0, context->const_buffer_size);
        context->const_buffer_size = new_size;
    }

    /* We are putting the entry at the top of the pool */
    index = context->binary.const_size;
    entry = (chameleon_constant*)(context->binary.const_pool + index);

    entry->type = type;

    /* Round up the size for alignment */
    size = (size + (size_t)7) & ~ (size_t)0x07; /* 8 byte alignment */

    /* Store the size */
    entry->size_in_pool = size;

    /* Move up the const_size "pointer" */
    context->binary.const_size += size;

    return index;
}

/*
 * Add an external index to the external tree
 */
static void add_extern_index(Assembler_Context *context, Assembler_Token *token) {

    size_t dummy;

    /* Have we come across this symbol yet? */
    if (src_node(context->extern_constants_tree,
                 (char*)token->token_value.string,
                 &dummy)) {
        /* Yes - duplicate */
        rxaserat(context, token, "duplicate exposed index");
    }
    else {
        /* Create entry in the tree */
        add_node(&context->extern_constants_tree,
                 (char*)token->token_value.string,
                 (size_t)dummy);
    }
}

/* Set the number of globals */
void rxassetg(Assembler_Context *context, Assembler_Token *globalsToken) {

    /* Flush Keyhole Optimiser Queue */
    flushopt(context);

    if (context->binary.globals)
        rxaserat(context, globalsToken, "duplicate .globals directive (ignored)");
    else {
        context->binary.globals = (int) globalsToken->token_value.integer;
        context->extern_regs = calloc(context->binary.globals, sizeof(char));
    }
}

/* Expose a global register */
void rxasexre(Assembler_Context *context, Assembler_Token *registerToken,
              Assembler_Token *exposeToken) {
    size_t entry_size, entry_index;
    expose_reg_constant *centry;

    /* Flush Keyhole Optimiser Queue */
    flushopt(context);

    if (registerToken->token_value.integer >= context->binary.globals)
        rxaserat(context, registerToken, "global register number bigger than the number of globals");

    /* Duplicate extern index check */
    add_extern_index(context, exposeToken);

    /* Duplicate register check */
    if (context->extern_regs[(int)registerToken->token_value.integer]) {
        rxaserat(context, registerToken, "duplicate exposed register");
    }
    else context->extern_regs[(int)registerToken->token_value.integer] = 1;

    /* Add the entry to the constants pool */
    entry_size =
            sizeof(expose_reg_constant) +
            strlen((char*)exposeToken->token_value.string);

    entry_index =
            reserve_in_const_pool(context, entry_size,
                                  EXPOSE_REG_CONST);
    centry = (expose_reg_constant *) (context->binary.const_pool +
                                  entry_index);
    memcpy(centry->index, exposeToken->token_value.string,
           strlen((char*)exposeToken->token_value.string) + 1);

    centry->global_reg = (int)registerToken->token_value.integer;
}

static void gen_instr(Assembler_Context *context, Instruction *inst) {
    /* Extend the buffer if we need to */
    size_t new_size;
    if (context->inst_buffer_size <= context->binary.inst_size + 1) { /* +1 = Make room for the end null */
        new_size = context->inst_buffer_size * 2;
        context->binary.binary = realloc(context->binary.binary, new_size * sizeof(bin_code));
        memset(context->binary.binary + context->inst_buffer_size, 0,
               context->inst_buffer_size * sizeof(bin_code));
        context->inst_buffer_size = new_size;
    }

    context->binary.binary[context->binary.inst_size].instruction.opcode = inst->opcode;
    context->binary.binary[context->binary.inst_size++].instruction.no_ops = inst->operands;
}

static size_t add_string_to_pool(Assembler_Context *context, char* string) {
    string_constant *sentry;
    size_t entry_index;
    size_t entry_size;

    /* Search if the constant already exists */
    if (!src_node(context->string_constants_tree,string,&entry_index)) {
        /* No it doesn't create one */
        entry_size = sizeof(string_constant) + strlen(string);
        entry_index = reserve_in_const_pool(context, entry_size,STRING_CONST);

        sentry = (string_constant *) (context->binary.const_pool + entry_index);
        sentry-> string_len = unescape_string(sentry->string, string);
        sentry->string[sentry->string_len] = 0; /* Add a null ... just for safety */

#ifndef NUTF8
        sentry-> string_chars = utf8nlen(sentry->string, sentry->string_len);
#endif

        /* TODO resize/shrink entry after unescaping */

        /* Save it in the tree */
        add_node(&context->string_constants_tree, string,entry_index);
    }
    return entry_index;
}

static size_t add_func_to_pool(Assembler_Context *context, Assembler_Token* token) {
    size_t entry_index;
    size_t entry_size;
    struct backpatching *ref_header;

    /* Have we come across this symbol yet? */
    if (src_node(context->proc_constants_tree,
                 (char*)token->token_value.string,
                 &entry_index)) {
        /* Yes */
        ref_header = (struct backpatching *)entry_index;
    }
    else {
        /* No - Create entry in the tree */
        proc_constant *centry;
        ref_header = malloc(sizeof(struct backpatching));
        add_node(&context->proc_constants_tree,
                 (char*)token->token_value.string,
                 (size_t) ref_header);

        /* Add the entry to the constants pool */
        entry_size =
                sizeof(proc_constant) +
                strlen((char*)token->token_value.string);
        entry_index =
                reserve_in_const_pool(context, entry_size,
                                      PROC_CONST);
        centry = (proc_constant *) (context->binary.const_pool +
                                    entry_index);
        centry->locals = -1;
        centry->start = SIZE_MAX;
        centry->exposed = SIZE_MAX;
        memcpy(centry->name, token->token_value.string,
               strlen((char*)token->token_value.string) + 1 );

        ref_header->defined = 0;
        ref_header->index = entry_index;
//      ref_header->def_token = 0;
        ref_header->refs = 0;
    }

    if (ref_header->defined == 0) {
        /* keep references for error messages generated during backpatching */
        struct backpatching_references* ref = malloc(sizeof(struct backpatching_references));
        ref->index = -1; /* No back-patching */
        ref->token = token;
        ref->link = ref_header->refs;
        ref_header->refs = ref;
    }

    return ref_header->index;
}

static size_t get_reg_number(Assembler_Context *context, Assembler_Token* token) {
    switch(token->token_type) {
        case RREG:
            if (token->token_value.integer >= context->current_locals)
                rxaserat(context, token, "register number bigger than the number of locals");

            return token->token_value.integer;

        case GREG:
            if (token->token_value.integer >= context->binary.globals)
                rxaserat(context, token, "global register number bigger than the number of globals");

            return token->token_value.integer + context->current_locals;

        case AREG:
            return token->token_value.integer + context->current_locals + context->binary.globals;
    }
    return 0; /* Should never happen */
}

static void gen_operand(Assembler_Context *context, Assembler_Token *operandToken) {
    size_t s_index;
    /* Extend the buffer if we need to */
    size_t new_size;
    if (context->inst_buffer_size <= context->binary.inst_size + 1) { /* +1 = Make room for the end null */
        new_size = context->inst_buffer_size * 2;
        context->binary.binary = realloc(context->binary.binary, new_size * sizeof(bin_code));
        memset(context->binary.binary + context->inst_buffer_size, 0,
               context->inst_buffer_size * sizeof(bin_code));
        context->inst_buffer_size = new_size;
    }

    size_t entry_index;
    struct backpatching *ref_header;

    switch(operandToken->token_type) {
        case ID:
            /* Have we come across this symbol yet? */
            if (src_node(context->label_constants_tree,
                         (char*)operandToken->token_value.string,
                         &entry_index)) {
                /* Yes */
                ref_header = (struct backpatching *)entry_index;
            }
            else {
                /* No - Create entry in the tree */
                ref_header = malloc(sizeof(struct backpatching));
                add_node(&context->label_constants_tree,
                         (char*)operandToken->token_value.string,
                         (size_t) ref_header);

                ref_header->defined = 0;
                ref_header->index = 0;
//                ref_header->def_token = 0;
                ref_header->refs = 0;
            }

            /* keep references for backpatching the above */
            struct backpatching_references* ref = malloc(sizeof(struct backpatching_references));
            ref->index = context->binary.inst_size;
            ref->token = operandToken;
            ref->link = ref_header->refs;
            ref_header->refs = ref;
            context->binary.binary[context->binary.inst_size++].index = 0;
            return;

        case RREG:
        case GREG:
        case AREG:
            context->binary.binary[context->binary.inst_size++].index =
                    get_reg_number(context, operandToken);
            return;

        case FUNC:
            s_index = add_func_to_pool(context, operandToken);
            context->binary.binary[context->binary.inst_size++].index = s_index;
            return;

        case INT:
            context->binary.binary[context->binary.inst_size++].iconst =
                    operandToken->token_value.integer;
            return;
        case FLOAT:
            context->binary.binary[context->binary.inst_size++].fconst =
                    operandToken->token_value.real;
            return;
        case CHAR:
            context->binary.binary[context->binary.inst_size++].cconst =
                    (char)operandToken->token_value.character;
            return;
        case STRING:
            s_index = add_string_to_pool(context, (char*)operandToken->token_value.string);;
            context->binary.binary[context->binary.inst_size++].index = s_index;
            return;

        default:
            printf("**gen_operand() error**\n");
            return;
    }

}

static OperandType token_to_operand_type(int token_type) {
    switch(token_type) {
        case ID: return OP_ID;
        case RREG:
        case GREG:
        case AREG:
            return OP_REG;
        case FUNC: return OP_FUNC;
        case INT: return OP_INT;
        case FLOAT: return OP_FLOAT;
        case CHAR: return OP_CHAR;
        case STRING: return OP_STRING;
        default: return OP_NONE;
    }
}

static Instruction *validate_instruction(Assembler_Context* context, Assembler_Token *instrToken,
                                         OperandType type1,
                                         OperandType type2,
                                         OperandType type3 ) {
    char errorBuffer[MAX_ERROR_LENGTH];
    size_t i;
    Instruction *possible_inst, *next_possible_inst;
    Instruction *inst = src_inst((char*)instrToken->token_value.string, type1, type2, type3);

    if (inst) return inst;

    /* Make a useful error message */
    possible_inst = fst_inst((char*)instrToken->token_value.string);
    if (!possible_inst) rxaserat(context, instrToken, "invalid instruction mnemonic");
    else {
        strncpy(errorBuffer, "invalid operand, expecting ", MAX_ERROR_LENGTH - 1);
        i = strlen(errorBuffer);
        exp_opds(possible_inst, errorBuffer + i, MAX_ERROR_LENGTH - 1 - i);
        possible_inst = nxt_inst(possible_inst);
        while (possible_inst) {
            next_possible_inst = nxt_inst(possible_inst);
            if (next_possible_inst) strncat(errorBuffer,", ", MAX_ERROR_LENGTH - 1);
            else strncat(errorBuffer," or ", MAX_ERROR_LENGTH - 1);
            i = strlen(errorBuffer);
            exp_opds(possible_inst, errorBuffer + i, MAX_ERROR_LENGTH - 1 - i);
            possible_inst = next_possible_inst;
        }
        rxaseaft(context, instrToken, errorBuffer);
    }
    return 0;
}

/** Generate code for an instruction with no operands */
void rxasgen0(Assembler_Context *context, Assembler_Token *instrToken) {

    Instruction *inst=validate_instruction(context, instrToken,
                                           0,
                                           0,
                                           0 );
    if (inst) {
        gen_instr(context, inst);
    }
}

/** Generate code for an instruction with one operand */
void rxasgen1(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token) {

    Instruction *inst=validate_instruction(context, instrToken,
                                           token_to_operand_type(operand1Token->token_type),
                                           0,
                                           0 );
    if (inst) {
        gen_instr(context, inst);
        gen_operand(context, operand1Token);
    }
}

/** Generate code for an instruction with two operand */
void rxasgen2(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token,
              Assembler_Token *operand2Token) {

    Instruction *inst=validate_instruction(context, instrToken,
                                           token_to_operand_type(operand1Token->token_type),
                                           token_to_operand_type(operand2Token->token_type),
                                           0 );
    if (inst) {
        gen_instr(context, inst);
        gen_operand(context, operand1Token);
        gen_operand(context, operand2Token);
    }
}

/** Generate code for an instruction with three operands */
void rxasgen3(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token,
              Assembler_Token *operand2Token, Assembler_Token *operand3Token) {

    Instruction *inst=validate_instruction(context, instrToken,
                                           token_to_operand_type(operand1Token->token_type),
                                           token_to_operand_type(operand2Token->token_type),
                                           token_to_operand_type(operand3Token->token_type));
    if (inst) {
        gen_instr(context, inst);
        gen_operand(context, operand1Token);
        gen_operand(context, operand2Token);
        gen_operand(context, operand3Token);
    }
}

/** Generate code for an instruction with up to three operands
 *  NULLS in the operandToken's are used to detect the number of operands */
void rxasgen(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token,
             Assembler_Token *operand2Token, Assembler_Token *operand3Token) {

    Instruction *inst=validate_instruction(context, instrToken,
                                           operand1Token?token_to_operand_type(operand1Token->token_type):0,
                                           operand2Token?token_to_operand_type(operand2Token->token_type):0,
                                           operand3Token?token_to_operand_type(operand3Token->token_type):0);
    if (inst) {
        gen_instr(context, inst);
        if (operand1Token) gen_operand(context, operand1Token);
        if (operand2Token) gen_operand(context, operand2Token);
        if (operand3Token) gen_operand(context, operand3Token);
    }
}

static size_t define_proc(Assembler_Context *context, Assembler_Token *funcToken) {
    proc_constant *centry;
    size_t entry_index;
    size_t entry_size;
    struct backpatching *ref_header;

    /* Have we come across this symbol yet? */
    if (src_node(context->proc_constants_tree,
                 (char*)funcToken->token_value.string,
                 &entry_index)) {
        /* Yes - check duplicate definition */
        ref_header = (struct backpatching *)entry_index;
        if (ref_header->defined) {
            rxaserat(context, funcToken, "duplicate procedure definition");
            /* TODO - Message, proc defined at ref_header->def_token */
        }
        centry = (proc_constant*)(context->binary.const_pool + ref_header->index);
    }
    else {
        /* No - Create entry in the tree */
        ref_header = malloc(sizeof(struct backpatching));
        add_node(&context->proc_constants_tree,
                 (char*)funcToken->token_value.string,
                 (size_t)ref_header);

        /* Add the entry to the constants pool */
        entry_size =
                sizeof(proc_constant) +
                strlen((char*)funcToken->token_value.string);
        entry_index =
                reserve_in_const_pool(context, entry_size,
                                      PROC_CONST);
        centry = (proc_constant *) (context->binary.const_pool +
                                    entry_index);
        memcpy(centry->name, funcToken->token_value.string, strlen((char*)funcToken->token_value.string) + 1);
        ref_header->refs = 0;
        ref_header->index = entry_index;
    }

    /* Add / update entry details */
    centry->locals = -1;
    centry->start = SIZE_MAX;
    centry->exposed = SIZE_MAX;

    centry->binarySpace = 0;
    ref_header->defined = 1;
//    ref_header->def_token = funcToken;

    return ref_header->index;
}

/* Procedures Definition */
void rxasproc(Assembler_Context *context, Assembler_Token *funcToken, Assembler_Token *localsToken) {

    proc_constant *centry;
    size_t entry_index;

    /* Flush Keyhole Optimiser Queue */
    flushopt(context);

    entry_index = define_proc(context, funcToken);
    centry = (proc_constant*)(context->binary.const_pool + entry_index);

    /* Add / update entry details */
    centry->locals = (int)localsToken->token_value.integer;
    centry->start = context->binary.inst_size;

    /* Chain the exposed constant entries */
    if (context->proc_head != -1) {
        ((proc_constant*)(context->binary.const_pool + context->proc_tail))->next = (int)entry_index;
        context->proc_tail = (int)entry_index;
        centry->next = -1;
    }
    else {
        context->proc_head = (int)entry_index;
        context->proc_tail = (int)entry_index;
        centry->next = -1;
    }

    /* Store the current number of locals */
    context->current_locals = (int)localsToken->token_value.integer;
}

/* Label Definition */
void rxaslabl(Assembler_Context *context, Assembler_Token *labelToken) {
    struct backpatching *ref_header;
    size_t tree_index;

    /* Have we come across this symbol yet? */
    if (src_node(context->label_constants_tree,
                 (char*)labelToken->token_value.string,
                 &tree_index)) {
        /* Yes - check duplicate definition */
        ref_header = (struct backpatching *)tree_index;
        if (ref_header->defined) {
            rxaserat(context, labelToken, "duplicate label definition");
            /* TODO - Message, label defined at ref_header->def_token */
            return;
        }
    }
    else {
        /* No - Create entry in the tree */
        ref_header = malloc(sizeof(struct backpatching));
        add_node(&context->label_constants_tree,
                 (char*)labelToken->token_value.string,
                 (size_t)ref_header);

        ref_header->refs = 0;
    }

    /* Add / update entry details */
    ref_header->defined = 1;
//    ref_header->def_token = labelToken;
    ref_header->index = context->binary.inst_size;
}

/* Define an exposed procedure */
void rxasexpc(Assembler_Context *context, Assembler_Token *funcToken, Assembler_Token *localsToken,
              Assembler_Token *exposeToken) {

    proc_constant *pentry;
    size_t entry_size, entry_index, pentry_index;
    expose_proc_constant *centry;

    /* Flush Keyhole Optimiser Queue */
    flushopt(context);

    /* Create Procedure Entry */
    pentry_index = define_proc(context, funcToken);
    pentry = (proc_constant*)(context->binary.const_pool + pentry_index);

    /* Add / update entry details */
    pentry->locals = (int)localsToken->token_value.integer;
    pentry->start = context->binary.inst_size;

    /* Chain the exposed constant entries */
    if (context->proc_head != -1) {
        ((proc_constant*)(context->binary.const_pool + context->proc_tail))->next = (int)pentry_index;
        context->proc_tail = (int)pentry_index;
        pentry->next = -1;
    }
    else {
        context->proc_head = (int)pentry_index;
        context->proc_tail = (int)pentry_index;
        pentry->next = -1;
    }

    /* Store the current number of locals */
    context->current_locals = (int)localsToken->token_value.integer;

    /* Duplicate extern index check */
    add_extern_index(context, exposeToken);

    /* Add the entry to the constants pool */
    entry_size =
            sizeof(expose_proc_constant) +
            strlen((char*)exposeToken->token_value.string);

    entry_index =
            reserve_in_const_pool(context, entry_size,
                                  EXPOSE_PROC_CONST);
    centry = (expose_proc_constant *) (context->binary.const_pool +
                                  entry_index);
    memcpy(centry->index, exposeToken->token_value.string,
           strlen((char*)exposeToken->token_value.string) + 1);

    centry->procedure = pentry_index;
    centry->imported = 0;

    /* Chain the exposed constant entries */
    if (context->expose_head != -1) {
        ((expose_proc_constant*)(context->binary.const_pool + context->expose_tail))->next = (int)entry_index;
        context->expose_tail = (int)entry_index;
        centry->next = -1;
    }
    else {
        context->expose_head = (int)entry_index;
        context->expose_tail = (int)entry_index;
        centry->next = -1;
    }

    /* Proc Entry has a pointer to the external entry */
    pentry->exposed = entry_index;
}

/* Declare a required / imported procedure */
void rxasdecl(Assembler_Context *context, Assembler_Token *funcToken,
              Assembler_Token *exposeToken) {

    proc_constant *pentry;
    size_t entry_size, entry_index, pentry_index;
    expose_proc_constant *centry;

    /* Flush Keyhole Optimiser Queue */
    flushopt(context);

    /* Create Procedure Entry */
    pentry_index = define_proc(context, funcToken);
    pentry = (proc_constant*)(context->binary.const_pool + pentry_index);

    /* Add / update entry details */
    pentry->locals = -1;
    pentry->start = SIZE_MAX;

    /* Duplicate extern index check */
    add_extern_index(context, exposeToken);

    /* Add the entry to the constants pool */
    entry_size =
            sizeof(expose_proc_constant) +
            strlen((char*)exposeToken->token_value.string);

    entry_index =
            reserve_in_const_pool(context, entry_size,
                                  EXPOSE_PROC_CONST);
    centry = (expose_proc_constant *) (context->binary.const_pool +
                                  entry_index);
    memcpy(centry->index, exposeToken->token_value.string,
           strlen((char*)exposeToken->token_value.string) + 1);

    centry->procedure = pentry_index;
    centry->imported = 1;

    /* Chain the exposed constant entries */
    if (context->expose_head != -1) {
        ((expose_proc_constant*)(context->binary.const_pool + context->expose_tail))->next = (int)entry_index;
        context->expose_tail = (int)entry_index;
        centry->next = -1;
    }
    else {
        context->expose_head = (int)entry_index;
        context->expose_tail = (int)entry_index;
        centry->next = -1;
    }

    /* Proc Entry has a pointer to the external entry */
    pentry = (proc_constant*)(context->binary.const_pool + pentry_index); /* It might have moved */
    pentry->exposed = entry_index;
}

/* Metadata Implementation */

/* Add a meta entry to tge constant pool - takes care of prev/next pointers  */
/* Returns entry offset */
static size_t add_meta_entry(Assembler_Context *context, size_t entry_size, enum const_pool_type type) {
    meta_entry *entry;
    size_t entry_index;

    entry_index = reserve_in_const_pool(context,entry_size, type);

    entry = (meta_entry*)(context->binary.const_pool + entry_index);
    if (context->meta_head != -1) {
        ((meta_entry*)(context->binary.const_pool + context->meta_tail))->next = (int)entry_index;
        entry->prev = context->meta_tail;
        context->meta_tail = (int)entry_index;
        entry->next = -1;
    }
    else {
        context->meta_head = (int)entry_index;
        context->meta_tail = (int)entry_index;
        entry->next = -1;
        entry->prev = -1;
    }

    entry->address = context->binary.inst_size;

    return entry_index;
}

/* Source filename */
void rxasmefl(Assembler_Context *context, Assembler_Token *file) {
    size_t entry = add_meta_entry(context,sizeof(meta_file_constant),META_FILE);
    size_t sentry = add_string_to_pool(context, (char*)file->token_value.string);

    /* NOTE the address in memory of the entry may change as we add (and therefor grow) the constant pool */
    ((meta_file_constant*)(context->binary.const_pool + entry))->file = sentry;
}

/* Source Line */
void rxasmesr(Assembler_Context *context, Assembler_Token *line, Assembler_Token *column, Assembler_Token *source) {
    size_t entry = add_meta_entry(context,sizeof(meta_src_constant),META_SRC);
    size_t sentry;

    /* NOTE the address in memory of the entry may change as we add (and therefor grow) the constant pool */
    ((meta_src_constant*)(context->binary.const_pool + entry))->line = line->token_value.integer;
    ((meta_src_constant*)(context->binary.const_pool + entry))->column = column->token_value.integer;
    sentry = add_string_to_pool(context, (char*)source->token_value.string);
    ((meta_src_constant*)(context->binary.const_pool + entry))->source = sentry;
}

/* Function Metadata */
void rxasmefu(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *func, Assembler_Token *args, Assembler_Token *inliner) {
    size_t entry = add_meta_entry(context,sizeof(meta_func_constant),META_FUNC);
    size_t sentry;

    /* NOTE the address in memory of the entry may change as we add (and therefor grow) the constant pool */
    sentry = add_string_to_pool(context, (char*)symbol->token_value.string);
    ((meta_func_constant*)(context->binary.const_pool + entry))->symbol = sentry;
    sentry = add_string_to_pool(context, (char*)option->token_value.string);
    ((meta_func_constant*)(context->binary.const_pool + entry))->option = sentry;
    sentry = add_string_to_pool(context, (char*)type->token_value.string);
    ((meta_func_constant*)(context->binary.const_pool + entry))->type = sentry;
    sentry = add_func_to_pool(context, func);
    ((meta_func_constant*)(context->binary.const_pool + entry))->func = sentry;
    sentry = add_string_to_pool(context, (char*)args->token_value.string);
    ((meta_func_constant*)(context->binary.const_pool + entry))->args = sentry;
    if (inliner) {
        sentry = add_string_to_pool(context, (char*)inliner->token_value.string);
        ((meta_func_constant*)(context->binary.const_pool + entry))->inliner = sentry;
    }
    else {
        sentry = add_string_to_pool(context, "");
        ((meta_func_constant *) (context->binary.const_pool + entry))->inliner = sentry;
    }
}

/* Register Metadata */
void rxasmere(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *reg) {
    size_t entry = add_meta_entry(context,sizeof(meta_reg_constant),META_REG);
    size_t sentry;

    /* NOTE the address in memory of the entry may change as we add (and therefor grow) the constant pool */
    sentry = add_string_to_pool(context, (char*)symbol->token_value.string);
    ((meta_reg_constant*)(context->binary.const_pool + entry))->symbol = sentry;
    sentry = add_string_to_pool(context, (char*)option->token_value.string);
    ((meta_reg_constant*)(context->binary.const_pool + entry))->option = sentry;
    sentry = add_string_to_pool(context, (char*)type->token_value.string);
    ((meta_reg_constant*)(context->binary.const_pool + entry))->type = sentry;
    ((meta_reg_constant*)(context->binary.const_pool + entry))->reg = get_reg_number(context, reg);
}

/* Constant Symbol Metadata */
void rxasmect(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *constant){
    size_t entry = add_meta_entry(context,sizeof(meta_const_constant),META_CONST);
    size_t sentry;

    /* NOTE the address in memory of the entry may change as we add (and therefor grow) the constant pool */
    sentry = add_string_to_pool(context, (char*)symbol->token_value.string);
    ((meta_const_constant*)(context->binary.const_pool + entry))->symbol = sentry;
    sentry = add_string_to_pool(context, (char*)option->token_value.string);
    ((meta_const_constant*)(context->binary.const_pool + entry))->option = sentry;
    sentry = add_string_to_pool(context, (char*)type->token_value.string);
    ((meta_const_constant*)(context->binary.const_pool + entry))->type = sentry;
    sentry = add_string_to_pool(context, (char*)constant->token_value.string);
    ((meta_const_constant*)(context->binary.const_pool + entry))->constant = sentry;
}

/* Clear Symbol Metadata */
void rxasmecl(Assembler_Context *context, Assembler_Token *symbol) {
    size_t entry = add_meta_entry(context,sizeof(meta_clear_constant),META_CLEAR);
    size_t sentry = add_string_to_pool(context, (char*)symbol->token_value.string);

    /* NOTE the address in memory of the entry may change as we add (and therefor grow) the constant pool */
    ((meta_clear_constant*)(context->binary.const_pool + entry))->symbol = sentry;
}
