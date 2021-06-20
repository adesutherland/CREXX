/* REXX ASSEMBLER               */
/* The Assembler itself         */
#include <string.h>
#include <stdint.h>
#include "platform.h"
#include "rxasassm.h"
#include "rxvminst.h"
#include "rxastree.h"

/* Structure to handle "backpatching" - fixing forward references */
struct backpatching_references;
struct backpatching {
    int defined;
    size_t index;
//    Token *def_token;
    struct backpatching_references *refs;
};

struct backpatching_references {
    size_t index;
    Token *token;
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
                err_at(context, ref->token, "unknown procedure");
                ref = ref->link;;
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

/* Backpatch Labels, check references and free backpatch information */
static void backpatch_labels(Assembler_Context *context) {
    struct string_wrapper *i;
    struct backpatching *patch;
    struct backpatching_references *ref, *nextref;

    /* Procedures - walk and process the tree */
    avl_tree_for_each_in_postorder(i, context->label_constants_tree,
                                   struct string_wrapper, index_node) {
        patch = (struct backpatching*)(i->value);

        if (patch->defined == 0) {
            ref = patch->refs;
            while(ref) {
                err_at(context, ref->token, "unknown label");
                ref = ref->link;;
            }
        }
        else {
            ref = patch->refs;
            while(ref) {
                context->binary.binary[ref->index].index = patch->index;
                ref = ref->link;;
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
    backpatch_procedures(context);
    backpatch_labels(context);
}

/* Unescape a string in place - returns the new string length */
static size_t unescape_string(char *to, char* from) {
    char *c, *d;
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
                    /* TODO add octal, hex, unicode */
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
 * The 'size' parameter is the size of the payload including
 * space for chameleon_constant etc.
 * Returns the index to the entry (from binary.const_pool)
 */
static size_t reserve_in_const_pool(Assembler_Context *context, size_t size,
                                    enum const_pool_type type) {
    size_t index;
    chameleon_constant * entry;

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
static void add_extern_index(Assembler_Context *context, Token *token) {

    size_t dummy;

    /* Have we come across this symbol yet? */
    if (src_node(context->extern_constants_tree,
                 (char*)token->token_value.string,
                 &dummy)) {
        /* Yes - duplicate */
        err_at(context, token, "duplicate exposed index");
    }
    else {
        /* Create entry in the tree */
        add_node(&context->extern_constants_tree,
                 (char*)token->token_value.string,
                 (size_t)dummy);
    }
}

/* Set the number of globals */
void rxassetg(Assembler_Context *context, Token *globalsToken) {
    if (context->binary.globals)
        err_at(context, globalsToken, "duplicate .globals directive (ignored)");
    else {
        context->binary.globals = (int) globalsToken->token_value.integer;
        context->extern_regs = calloc(context->binary.globals, sizeof(char));
    }
}

/* Expose a global register */
void rxasexre(Assembler_Context *context, Token *registerToken,
              Token *exposeToken) {
    size_t entry_size, entry_index;
    expose_reg_constant *centry;

    if (registerToken->token_value.integer >= context->binary.globals)
        err_at(context, registerToken, "global register number bigger than the number of globals");

    /* Duplicate extern index check */
    add_extern_index(context, exposeToken);

    /* Duplicate register check */
    if (context->extern_regs[(int)registerToken->token_value.integer]) {
        err_at(context, registerToken, "duplicate exposed register");
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
    context->binary.binary[context->binary.inst_size].instruction.opcode = inst->opcode;
    context->binary.binary[context->binary.inst_size++].instruction.no_ops = inst->operands;
}

static void gen_operand(Assembler_Context *context, Token *operandToken) {
    string_constant *sentry;
    size_t entry_index;
    size_t entry_size;
    proc_constant *centry;
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

            if (ref_header->defined == 0) {
                /* keep references for backpatching the above */
                struct backpatching_references* ref = malloc(sizeof(struct backpatching_references));
                ref->index = context->binary.inst_size;
                ref->token = operandToken;
                ref->link = ref_header->refs;
                ref_header->refs = ref;
                context->binary.binary[context->binary.inst_size++].index = 0;
            }
            else {
                context->binary.binary[context->binary.inst_size++].index = ref_header->index;
            }
            return;
        case RREG:
            if (operandToken->token_value.integer >= context->current_locals)
                err_at(context, operandToken, "register number bigger than the number of locals");

            context->binary.binary[context->binary.inst_size++].index =
                    operandToken->token_value.integer;
            return;
        case GREG:
            if (operandToken->token_value.integer >= context->binary.globals)
                err_at(context, operandToken, "global register number bigger than the number of globals");

            context->binary.binary[context->binary.inst_size++].index =
                    operandToken->token_value.integer +
                    context->current_locals;
            return;
        case AREG:
            context->binary.binary[context->binary.inst_size++].index =
                    operandToken->token_value.integer +
                    context->current_locals +
                    context->binary.globals;
            return;
        case FUNC:
            /* Have we come across this symbol yet? */
            if (src_node(context->proc_constants_tree,
                         (char*)operandToken->token_value.string,
                         &entry_index)) {
                /* Yes */
                ref_header = (struct backpatching *)entry_index;
            }
            else {
                /* No - Create entry in the tree */
                ref_header = malloc(sizeof(struct backpatching));
                add_node(&context->proc_constants_tree,
                         (char*)operandToken->token_value.string,
                         (size_t) ref_header);

                /* Add the entry to the constants pool */
                entry_size =
                        sizeof(proc_constant) +
                        strlen((char*)operandToken->token_value.string);
                entry_index =
                        reserve_in_const_pool(context, entry_size,
                                              PROC_CONST);
                centry = (proc_constant *) (context->binary.const_pool +
                                            entry_index);
                centry->locals = -1;
                centry->start = SIZE_MAX;
                centry->exposed = SIZE_MAX;
                memcpy(centry->name, operandToken->token_value.string,
                       strlen((char*)operandToken->token_value.string) + 1 );

                ref_header->defined = 0;
                ref_header->index = entry_index;
//                ref_header->def_token = 0;
                ref_header->refs = 0;
            }

            context->binary.binary[context->binary.inst_size++].index = ref_header->index;
            if (ref_header->defined == 0) {
                /* keep references for backpatching (well in this case just
                 * for procs just making a good error message) */
                struct backpatching_references* ref = malloc(sizeof(struct backpatching_references));
                ref->index = context->binary.inst_size;
                ref->token = operandToken;
                ref->link = ref_header->refs;
                ref_header->refs = ref;
            }
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
            /* Search if the constant already exists */
            if (!src_node(context->string_constants_tree,
                          (char*)operandToken->token_value.string,
                         &entry_index)) {
                /* No it doesn't create one */
                entry_size =
                        sizeof(string_constant) +
                        strlen((char*)operandToken->token_value.string);
                entry_index =
                        reserve_in_const_pool(context, entry_size,
                                              STRING_CONST);

                sentry =
                        (string_constant *) (context->binary.const_pool +
                                             entry_index);
                sentry->string_len = unescape_string(sentry->string,
                                                     (char*)operandToken->token_value
                                                             .string);
                sentry->string[sentry->string_len] =
                        0; /* Add a null ... just for safety */

                /* TODO resize/shrink entry after unescaping */

                /* Save it in the tree */
                add_node(&context->string_constants_tree,
                         (char*)operandToken->token_value.string,
                         entry_index);
            }
            context->binary.binary[context->binary.inst_size++].index = entry_index;
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

static Instruction *validate_instruction(Assembler_Context* context, Token *instrToken,
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
    if (!possible_inst) err_at(context, instrToken, "invalid instruction mnemonic");
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
        err_aftr(context, instrToken, errorBuffer);
    }
    return 0;
}

/** Generate code for an instruction with no operands */
void rxasgen0(Assembler_Context *context, Token *instrToken) {

    Instruction *inst=validate_instruction(context, instrToken,
                                           0,
                                           0,
                                           0 );
    if (inst) {
        gen_instr(context, inst);
    }
}

/** Generate code for an instruction with one operand */
void rxasgen1(Assembler_Context *context, Token *instrToken, Token *operand1Token) {

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
void rxasgen2(Assembler_Context *context, Token *instrToken, Token *operand1Token,
              Token *operand2Token) {

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
void rxasgen3(Assembler_Context *context, Token *instrToken, Token *operand1Token,
              Token *operand2Token, Token *operand3Token) {

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

static size_t define_proc(Assembler_Context *context, Token *funcToken) {
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
            err_at(context, funcToken, "duplicate procedure definition");
            /* TODO - Message, proc defined at ref_header->def_token */
            return 0;
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

    centry->module = 0;
    ref_header->defined = 1;
//    ref_header->def_token = funcToken;

    return ref_header->index;
}

/* Procedures Definition */
void rxasproc(Assembler_Context *context, Token *funcToken, Token *localsToken) {

    proc_constant *centry;
    size_t entry_index;

    entry_index = define_proc(context, funcToken);
    centry = (proc_constant*)(context->binary.const_pool + entry_index);

    /* Add / update entry details */
    centry->locals = (int)localsToken->token_value.integer;
    centry->start = context->binary.inst_size;

    /* Store the current number of locals */
    context->current_locals = (int)localsToken->token_value.integer;
}

/* Label Definition */
void rxaslabl(Assembler_Context *context, Token *labelToken) {
    struct backpatching *ref_header;
    size_t tree_index;

    /* Have we come across this symbol yet? */
    if (src_node(context->label_constants_tree,
                 (char*)labelToken->token_value.string,
                 &tree_index)) {
        /* Yes - check duplicate definition */
        ref_header = (struct backpatching *)tree_index;
        if (ref_header->defined) {
            err_at(context, labelToken, "duplicate label definition");
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
void rxasexpc(Assembler_Context *context, Token *funcToken, Token *localsToken,
              Token *exposeToken) {

    proc_constant *pentry;
    size_t entry_size, entry_index, pentry_index;
    expose_proc_constant *centry;

    /* Create Procedure Entry */
    pentry_index = define_proc(context, funcToken);
    pentry = (proc_constant*)(context->binary.const_pool + pentry_index);

    /* Add / update entry details */
    pentry->locals = (int)localsToken->token_value.integer;
    pentry->start = context->binary.inst_size;

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

    /* Proc Entry has a pointer to the external entry */
    pentry->exposed = entry_index;
}

/* Declare a required / imported procedure */
void rxasdecl(Assembler_Context *context, Token *funcToken,
              Token *exposeToken) {

    proc_constant *pentry;
    size_t entry_size, entry_index, pentry_index;
    expose_proc_constant *centry;

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

    /* Proc Entry has a pointer to the external entry */
    pentry->exposed = entry_index;
}
