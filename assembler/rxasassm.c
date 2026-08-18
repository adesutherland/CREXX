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

/* REXX ASSEMBLER               */
/* The Assembler itself         */
#include <string.h>
#include <stdint.h>
#include "platform.h"
#include "rxasassm.h"
#include "../binutils/include/rxdefs.h"
#include "../binutils/include/rxjtable.h"
#include "../binutils/include/rxnumparse.h"
#include "../binutils/include/rxgraph.h"
#include "../binutils/include/opdata.c"
#include <ctype.h>
#include <limits.h>


static int mnemonic_matches(const char *mnemonic, const char *table_name) {
    int i = 0;
    while (mnemonic[i]) {
        if (toupper((unsigned char)mnemonic[i]) != table_name[i]) return 0;
        i++;
    }
    if (table_name[i] == 0 || table_name[i] == '_') return 1;
    return 0;
}

static const OpInfo *find_opcodev(const char *mnemonic,
                                  const OperandType *operands,
                                  size_t operand_count) {
    int i;
    for (i = 0; op_table[i].mnemonic != NULL; i++) {
        if (!rxop_is_source_mnemonic(op_table[i].mnemonic)) continue;
        if (rxop_format_matches(op_table[i].format, operands, operand_count)) {
            if (mnemonic_matches(mnemonic, op_table[i].mnemonic)) {
                return &op_table[i];
            }
        }
    }
    return NULL;
}

static const OpInfo *find_opcode(const char *mnemonic,
                                 OperandType t1,
                                 OperandType t2,
                                 OperandType t3) {
    OperandType operands[3];
    size_t operand_count = 0;

    if (t1 != OP_NONE) operands[operand_count++] = t1;
    if (t2 != OP_NONE) operands[operand_count++] = t2;
    if (t3 != OP_NONE) operands[operand_count++] = t3;
    return find_opcodev(mnemonic, operands, operand_count);
}

#include "rxastree.h"
#ifndef NUTF8
#include "utf.h"
#endif

struct float_wrapper {
    uint64_t bits;
    size_t value;
    struct avl_tree_node index_node;
};

#define GET_FLOAT_BITS(i) avl_tree_entry((i), struct float_wrapper, index_node)->bits
#define GET_FLOAT_VALUE(i) avl_tree_entry((i), struct float_wrapper, index_node)->value

static uint64_t float_to_bits(double value) {
    uint64_t bits = 0;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

static int compare_float_node_node(const struct avl_tree_node *node1,
                                   const struct avl_tree_node *node2) {
    uint64_t n1 = GET_FLOAT_BITS(node1);
    uint64_t n2 = GET_FLOAT_BITS(node2);
    if (n1 < n2) return -1;
    if (n1 > n2) return 1;
    return 0;
}

static int compare_float_node_value(const void *value,
                                    const struct avl_tree_node *nodeptr) {
    uint64_t n1 = *(const uint64_t *)value;
    uint64_t n2 = GET_FLOAT_BITS(nodeptr);
    if (n1 < n2) return -1;
    if (n1 > n2) return 1;
    return 0;
}

static void rxas_panic_oom_at(Assembler_Context *context, const char *operation,
                              size_t requested_bytes, const char *extra_detail,
                              const char *source_file, int source_line,
                              const char *function_name) {
    char detail[512];

    if (context) {
        snprintf(detail, sizeof(detail),
                 "input=%s; output=%s; source line=%d; inst_size=%llu; inst_buffer=%llu; const_size=%llu; const_buffer=%llu%s%s",
                 context->file_name ? context->file_name : "(none)",
                 context->output_file_name ? context->output_file_name : "(none)",
                 context->line,
                 (unsigned long long)context->binary.inst_size,
                 (unsigned long long)context->inst_buffer_size,
                 (unsigned long long)context->binary.const_size,
                 (unsigned long long)context->const_buffer_size,
                 extra_detail && extra_detail[0] ? "; " : "",
                 extra_detail && extra_detail[0] ? extra_detail : "");
    } else {
        snprintf(detail, sizeof(detail), "%s",
                 extra_detail && extra_detail[0] ? extra_detail : "");
    }

    rx_panic_out_of_memory(operation, requested_bytes, detail,
                           source_file, source_line, function_name);
}

#define RXAS_PANIC_OOM(context, operation, requested_bytes, extra_detail) \
    rxas_panic_oom_at((context), (operation), (requested_bytes), (extra_detail), \
                      __FILE__, __LINE__, RX_FUNCTION_NAME)

static int add_float_node(Assembler_Context *context, struct avl_tree_node **root,
                          uint64_t bits, size_t value) {
    struct float_wrapper *entry = malloc(sizeof(struct float_wrapper));
    if (!entry) {
        RXAS_PANIC_OOM(context, "malloc rxas float constant tree node",
                       sizeof(struct float_wrapper), 0);
    }
    entry->bits = bits;
    entry->value = value;
    if (avl_tree_insert(root, &entry->index_node, compare_float_node_node)) {
        free(entry);
        return 1;
    }
    return 0;
}

static int src_float_node(struct avl_tree_node *root, uint64_t bits, size_t *value) {
    struct avl_tree_node *result = avl_tree_lookup(root, &bits, compare_float_node_value);
    if (!result) return 0;
    *value = GET_FLOAT_VALUE(result);
    return 1;
}

static void free_float_tree(struct avl_tree_node **root) {
    struct float_wrapper *entry;

    avl_tree_for_each_in_postorder(entry, *root, struct float_wrapper, index_node) {
        free(entry);
    }
    *root = 0;
}

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

typedef struct rxas_constant_alias {
    OperandType operand_type;
    size_t pool_index;
} rxas_constant_alias;

#define RXAS_JTABLE_ALG_AUTO 0
#define RXAS_JTABLE_ALG_LINEAR 1
#define RXAS_JTABLE_ALG_OPENHASH 2
#define RXAS_JTABLE_ALG_ACPH 3

#define RXAS_JTABLE_KEY_NONE 0
#define RXAS_JTABLE_KEY_STRING 1
#define RXAS_JTABLE_KEY_BINARY 2
#define RXAS_JTABLE_KEY_INT 3

#define RXAS_JTABLE_MATCH_EXACT 0
#define RXAS_JTABLE_MATCH_PADDED 1
#define RXAS_JTABLE_MATCH_NUMERIC 2

struct rxas_jtable_case {
    Assembler_Token *label_token;
    Assembler_Token *value_token;
    unsigned char *key;
    size_t key_length;
    uint32_t target;
    struct rxas_jtable_case *next;
};

struct rxas_acph_node;

struct rxas_acph_slot {
    uint16_t symbol;
    unsigned char kind;
    struct rxas_jtable_case *leaf;
    struct rxas_acph_node *child;
    uint32_t packed_value_offset;
};

struct rxas_acph_node {
    uint32_t column;
    uint16_t slot_count;
    unsigned char prime;
    uint32_t packed_offset;
    struct rxas_acph_slot *slots;
    struct rxas_acph_node *next_all;
};

struct rxas_jtable_ref {
    size_t operand_index;
    Assembler_Token *token;
    struct rxas_jtable_ref *next;
};

struct rxas_jump_table {
    char *name;
    char *proc_name;
    int declared;
    int algorithm;
    int key_kind;
    int match_mode;
    int match_mode_set;
    int used_by_jumpbs;
    Assembler_Token *decl_token;
    struct rxas_jtable_case *cases;
    struct rxas_jtable_case *cases_tail;
    struct rxas_jtable_ref *refs;
    struct rxas_jump_table *next;
};

static char *rxas_strdup(Assembler_Context *context, const char *value) {
    char *copy;
    size_t length;

    if (!value) value = "";
    length = strlen(value);
    copy = malloc(length + 1);
    if (!copy) {
        RXAS_PANIC_OOM(context, "malloc rxas string copy", length + 1, 0);
    }
    memcpy(copy, value, length + 1);
    return copy;
}

static void free_jump_tables(Assembler_Context *context) {
    struct rxas_jump_table *table, *next_table;
    struct rxas_jtable_case *entry, *next_entry;
    struct rxas_jtable_ref *ref, *next_ref;

    table = context->jump_tables;
    while (table) {
        next_table = table->next;
        entry = table->cases;
        while (entry) {
            next_entry = entry->next;
            free(entry->key);
            free(entry);
            entry = next_entry;
        }
        ref = table->refs;
        while (ref) {
            next_ref = ref->next;
            free(ref);
            ref = next_ref;
        }
        free(table->name);
        free(table->proc_name);
        free(table);
        table = next_table;
    }
    context->jump_tables = 0;
}

static void free_constant_alias_tree(struct avl_tree_node **root) {
    struct string_wrapper *i;

    avl_tree_for_each_in_postorder(i, *root, struct string_wrapper, index_node) {
        free((void *)i->value);
        free(i);
    }
    *root = 0;
}

/* Frees Assembler Work Data */
void freeasbl(Assembler_Context *context) {
    if (context->string_constants_tree) free_tree(&context->string_constants_tree);
    if (context->decimal_constants_tree) free_tree(&context->decimal_constants_tree);
    if (context->float_constants_tree) free_float_tree(&context->float_constants_tree);
    if (context->binary_constants_tree) free_tree(&context->binary_constants_tree);
    if (context->constant_aliases_tree) free_constant_alias_tree(&context->constant_aliases_tree);
    if (context->proc_constants_tree) free_tree(&context->proc_constants_tree);
    if (context->label_constants_tree) free_tree(&context->label_constants_tree);
    if (context->extern_constants_tree) free_tree(&context->extern_constants_tree);
    if (context->jump_tables) free_jump_tables(context);
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

    const OpInfo *br = find_opcode("br", OP_ID, OP_NONE, OP_NONE);
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

static void backpatch_jump_tables(Assembler_Context *context);

/* Backpatch, check references and free backpatch information */
void backptch(Assembler_Context *context) {
    if (context->optimise) optimise_labels(context);
    backpatch_procedures(context);
    backpatch_jump_tables(context);
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
    void *new_pool;
    char detail[128];

    /* Extend the buffer if we need to */
    while (size + 8 > context->const_buffer_size - context->binary.const_size) { // +8 for the 8 bit alignment
        new_size = context->const_buffer_size * 2;
        new_pool = realloc(context->binary.const_pool, new_size);
        if (!new_pool) {
            snprintf(detail, sizeof(detail),
                     "entry_size=%llu; old_const_buffer=%llu; const_type=%d",
                     (unsigned long long)size,
                     (unsigned long long)context->const_buffer_size,
                     (int)type);
            RXAS_PANIC_OOM(context, "realloc rxas constant pool", new_size, detail);
        }
        context->binary.const_pool = new_pool;
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
        if (!context->extern_regs && context->binary.globals) {
            RXAS_PANIC_OOM(context, "calloc rxas extern register map",
                           (size_t)context->binary.globals * sizeof(char), 0);
        }
    }
}

/* Expose a global register */
static void append_expose_entry(Assembler_Context *context, size_t entry_index) {
    expose_reg_constant *tail_entry;
    expose_reg_constant *new_entry = (expose_reg_constant *)(context->binary.const_pool + entry_index);

    new_entry->next = -1;

    if (context->expose_head != -1) {
        tail_entry = (expose_reg_constant *)(context->binary.const_pool + context->expose_tail);
        tail_entry->next = (int)entry_index;
        context->expose_tail = (int)entry_index;
    }
    else {
        context->expose_head = (int)entry_index;
        context->expose_tail = (int)entry_index;
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
    append_expose_entry(context, entry_index);
}

static void gen_instr(Assembler_Context *context, int opcode, int operands) {
    /* Extend the buffer if we need to */
    size_t new_size;
    void *new_binary;
    if (context->inst_buffer_size <= context->binary.inst_size + 1) { /* +1 = Make room for the end null */
        new_size = context->inst_buffer_size * 2;
        new_binary = realloc(context->binary.binary, new_size * sizeof(bin_code));
        if (!new_binary) {
            RXAS_PANIC_OOM(context, "realloc rxas instruction buffer",
                           new_size * sizeof(bin_code), 0);
        }
        context->binary.binary = new_binary;
        memset(context->binary.binary + context->inst_buffer_size, 0,
               context->inst_buffer_size * sizeof(bin_code));
        context->inst_buffer_size = new_size;
    }

    context->binary.binary[context->binary.inst_size].instruction.opcode = opcode;
    context->binary.binary[context->binary.inst_size++].instruction.no_ops = operands;
}

static size_t add_string_to_pool(Assembler_Context *context, Assembler_Token *token, char* string) {
    string_constant *sentry;
    size_t entry_index;
    size_t entry_size;
    size_t string_len;
    char *unescaped;

    /* Search if the constant already exists */
    if (!src_node(context->string_constants_tree,string,&entry_index)) {
        /* No it doesn't create one */
        entry_size = sizeof(string_constant) + strlen(string);
        unescaped = malloc(entry_size);
        if (!unescaped) {
            RXAS_PANIC_OOM(context, "malloc rxas unescape buffer", entry_size, 0);
        }
        string_len = unescape_string(unescaped, string);
        unescaped[string_len] = 0; /* Add a null ... just for safety */
#ifndef NUTF8
        {
            size_t string_chars;
            void *invalid = utf8nvalid_count(unescaped, string_len, &string_chars);
            if (invalid) {
                char errorBuffer[MAX_ERROR_LENGTH];
                snprintf(errorBuffer, sizeof(errorBuffer),
                         "string constant is not valid UTF-8 at byte %lu; use .binary for byte data",
                         (unsigned long)((char *)invalid - unescaped));
                if (token) rxaserat(context, token, errorBuffer);
                else rxaserrf(context, context->line, 1, 1, errorBuffer);
                free(unescaped);
                return SIZE_MAX;
            }
        }
#endif
        entry_index = reserve_in_const_pool(context, entry_size,STRING_CONST);

        sentry = (string_constant *) (context->binary.const_pool + entry_index);
        sentry-> string_len = string_len;
        memcpy(sentry->string, unescaped, string_len + 1);

#ifndef NUTF8
        utf8nvalid_count(sentry->string, sentry->string_len, &sentry->string_chars);
#endif

        /* TODO resize/shrink entry after unescaping */

        /* Save it in the tree */
        add_node(&context->string_constants_tree, string,entry_index);
        free(unescaped);
    }
    return entry_index;
}

static size_t add_decimal_to_pool(Assembler_Context *context, char* decimal) {
    string_constant *sentry;
    size_t entry_index;
    size_t entry_size;

    /* Search if the constant already exists */
    if (!src_node(context->decimal_constants_tree,decimal,&entry_index)) {
        /* No it doesn't create one */
        entry_size = sizeof(string_constant) + strlen(decimal);
        entry_index = reserve_in_const_pool(context, entry_size,DECIMAL_CONST);

        sentry = (string_constant *) (context->binary.const_pool + entry_index);
        sentry-> string_len = strlen( decimal );
        memcpy(sentry->string, decimal, sentry->string_len);
        sentry->string[sentry->string_len] = 0; /* Add a null */
#ifndef NUTF8
        sentry->string_chars = sentry->string_len; // ASCII only
#endif
        /* Save it in the tree */
        add_node(&context->decimal_constants_tree, decimal,entry_index);
    }
    return entry_index;
}

static uint8_t trace_code_from_token(Assembler_Context *context, Assembler_Token *token, uint8_t empty_value) {
    char *value;

    if (!token) return empty_value;
    value = (char *) token->token_value.string;
    if (!value || !value[0]) return empty_value;
    if (value[1]) {
        rxaserat(context, token, "trace event code must be a one-character string");
        return empty_value;
    }
    return (uint8_t) value[0];
}

static size_t trace_ref_from_token(Assembler_Token *token) {
    if (!token || token->token_value.integer < 0) return RXBIN_TRACE_REF_NONE;
    return (size_t) token->token_value.integer;
}

static uint32_t trace_u32_from_token(Assembler_Token *token) {
    if (!token || token->token_value.integer < 0) return 0;
    return (uint32_t) token->token_value.integer;
}

static size_t add_optional_string_to_pool(Assembler_Context *context, Assembler_Token *token) {
    char *value;

    if (!token) return RXBIN_TRACE_REF_NONE;
    value = (char *) token->token_value.string;
    if (!value || !value[0]) return RXBIN_TRACE_REF_NONE;
    return add_string_to_pool(context, token, value);
}

static size_t add_float_to_pool(Assembler_Context *context, double value) {
    float_constant *entry;
    size_t entry_index;
    uint64_t bits = float_to_bits(value);

    if (!src_float_node(context->float_constants_tree, bits, &entry_index)) {
        entry_index = reserve_in_const_pool(context, sizeof(float_constant), FLOAT_CONST);
        entry = FLOAT_CONST_AT(context->binary.const_pool, entry_index);
        entry->double_value = value;
        add_float_node(context, &context->float_constants_tree, bits, entry_index);
    }

    return entry_index;
}

static size_t add_binary_to_pool(Assembler_Context *context, char* hex) {
    string_constant *sentry;
    size_t entry_index;
    size_t entry_size;
    hex += 2; // Skip the 0x prefix
    size_t hex_len = strlen(hex);
    size_t bin_len = (hex_len / 2);  // 2 chars per byte

    /* Search if the constant already exists */
    if (!src_node(context->binary_constants_tree,hex,&entry_index)) {
        /* No it doesn't create one */
        entry_size = sizeof(string_constant) + bin_len;
        entry_index = reserve_in_const_pool(context, entry_size,BINARY_CONST);
        sentry = (string_constant *) (context->binary.const_pool + entry_index);
        sentry->string_len = bin_len;

        // Convert the hex string to binary
        unsigned char *b = (unsigned char *)sentry->string;
        char *h = hex;
        while (*h) {
            int val = hexchar2int(*h);
            if (val == -1) {
                fprintf(stderr, "PANIC: Invalid hex character in binary constant\n");
                exit(-1);
            }
            *b = val << 4;
            h++;
            val = hexchar2int(*h);
            if (val == -1) {
                fprintf(stderr, "PANIC: Invalid hex character in binary constant\n");
                exit(-1);
            }
            *b |= val;
            b++;
            h++;
        }
        *b = 0; // Null terminate the binary string (for safety)

#ifndef NUTF8
        sentry->string_chars = bin_len; // Byte stream only
#endif
        /* Save it in the tree */
        add_node(&context->binary_constants_tree, hex,entry_index);
    }
    return entry_index;
}

static size_t add_raw_binary_to_pool(Assembler_Context *context, const unsigned char *data, size_t length) {
    string_constant *sentry;
    size_t entry_index;
    size_t entry_size;

    entry_size = sizeof(string_constant) + length;
    entry_index = reserve_in_const_pool(context, entry_size, BINARY_CONST);
    sentry = (string_constant *) (context->binary.const_pool + entry_index);
    sentry->string_len = length;
    if (length) memcpy(sentry->string, data, length);
    sentry->string[length] = 0;
#ifndef NUTF8
    sentry->string_chars = length;
#endif
    return entry_index;
}

static void rxas_write_u16le(unsigned char *target, uint16_t value) {
    target[0] = (unsigned char)(value & 0xffu);
    target[1] = (unsigned char)((value >> 8) & 0xffu);
}

static void rxas_write_u32le(unsigned char *target, uint32_t value) {
    target[0] = (unsigned char)(value & 0xffu);
    target[1] = (unsigned char)((value >> 8) & 0xffu);
    target[2] = (unsigned char)((value >> 16) & 0xffu);
    target[3] = (unsigned char)((value >> 24) & 0xffu);
}

static struct rxas_jump_table *find_jump_table(Assembler_Context *context, const char *name, const char *proc_name) {
    struct rxas_jump_table *table;

    if (!context || !name || !proc_name) return 0;
    table = context->jump_tables;
    while (table) {
        if (strcmp(table->name, name) == 0 && strcmp(table->proc_name, proc_name) == 0) return table;
        table = table->next;
    }
    return 0;
}

int rxas_jump_table_case_count(Assembler_Context *context,
                               Assembler_Token *tableToken,
                               size_t *count_out) {
    struct rxas_jump_table *table;
    struct rxas_jtable_case *entry;
    size_t count;

    if (count_out) *count_out = 0;
    if (!context || !context->current_proc_name || !tableToken ||
        tableToken->token_type != ID || !count_out)
        return 0;
    table = find_jump_table(context,
                            (const char *)tableToken->token_value.string,
                            context->current_proc_name);
    if (!table || !table->declared) return 0;
    count = 0;
    for (entry = table->cases; entry; entry = entry->next) count++;
    *count_out = count;
    return 1;
}

Assembler_Token *rxas_jump_table_case_label(Assembler_Context *context,
                                             Assembler_Token *tableToken,
                                             size_t case_index) {
    struct rxas_jump_table *table;
    struct rxas_jtable_case *entry;
    size_t index;

    if (!context || !context->current_proc_name || !tableToken ||
        tableToken->token_type != ID)
        return 0;
    table = find_jump_table(context,
                            (const char *)tableToken->token_value.string,
                            context->current_proc_name);
    if (!table || !table->declared) return 0;
    index = 0;
    for (entry = table->cases; entry; entry = entry->next) {
        if (index == case_index) return entry->label_token;
        index++;
    }
    return 0;
}

static struct rxas_jump_table *get_or_create_jump_table(Assembler_Context *context, Assembler_Token *nameToken) {
    struct rxas_jump_table *table;

    if (!context->current_proc_name) {
        rxaserat(context, nameToken, "jump table must be declared or used inside a procedure");
        return 0;
    }

    table = find_jump_table(context, (char *)nameToken->token_value.string, context->current_proc_name);
    if (table) return table;

    table = calloc(1, sizeof(*table));
    if (!table) {
        RXAS_PANIC_OOM(context, "calloc rxas jump table", sizeof(*table), 0);
    }
    table->name = rxas_strdup(context, (char *)nameToken->token_value.string);
    table->proc_name = rxas_strdup(context, context->current_proc_name);
    table->algorithm = RXAS_JTABLE_ALG_AUTO;
    table->next = context->jump_tables;
    context->jump_tables = table;
    return table;
}

static int parse_jump_table_algorithm(Assembler_Context *context, Assembler_Token *algorithmToken) {
    char *name;

    if (!algorithmToken) return RXAS_JTABLE_ALG_AUTO;
    name = (char *)algorithmToken->token_value.string;
    if (mnemonic_matches(name, "AUTO")) return RXAS_JTABLE_ALG_AUTO;
    if (mnemonic_matches(name, "LINEAR")) return RXAS_JTABLE_ALG_LINEAR;
    if (mnemonic_matches(name, "OPENHASH") || mnemonic_matches(name, "OPEN-HASH")) {
        return RXAS_JTABLE_ALG_OPENHASH;
    }
    if (mnemonic_matches(name, "ACPH")) return RXAS_JTABLE_ALG_ACPH;
    rxaserat(context, algorithmToken, "jump table algorithm must be auto, linear, openhash, or acph");
    return -1;
}

static const char *jump_table_key_kind_name(int kind) {
    switch (kind) {
        case RXAS_JTABLE_KEY_STRING: return "string";
        case RXAS_JTABLE_KEY_BINARY: return "binary";
        case RXAS_JTABLE_KEY_INT: return "integer";
        default: return "unknown";
    }
}

static int jump_table_key_kind_from_token(Assembler_Token *valueToken) {
    if (!valueToken) return RXAS_JTABLE_KEY_NONE;
    switch (valueToken->token_type) {
        case STRING: return RXAS_JTABLE_KEY_STRING;
        case HEX: return RXAS_JTABLE_KEY_BINARY;
        case INT: return RXAS_JTABLE_KEY_INT;
        default: return RXAS_JTABLE_KEY_NONE;
    }
}

static void mark_jump_table_key_kind(Assembler_Context *context, struct rxas_jump_table *table,
                                     int key_kind, Assembler_Token *token) {
    char errorBuffer[MAX_ERROR_LENGTH];

    if (!table || key_kind == RXAS_JTABLE_KEY_NONE) return;
    if (table->key_kind == RXAS_JTABLE_KEY_NONE) {
        table->key_kind = key_kind;
        return;
    }
    if (table->key_kind != key_kind) {
        snprintf(errorBuffer, sizeof(errorBuffer),
                 "jump table key type mismatch; table already uses %s keys",
                 jump_table_key_kind_name(table->key_kind));
        rxaserat(context, token, errorBuffer);
    }
}

static unsigned char *hex_token_to_bytes(Assembler_Context *context, Assembler_Token *token, size_t *length_out) {
    char *hex;
    unsigned char *bytes;
    unsigned char *b;
    size_t hex_len;
    size_t byte_len;

    hex = (char *)token->token_value.string + 2;
    hex_len = strlen(hex);
    byte_len = hex_len / 2;
    bytes = malloc(byte_len ? byte_len : 1);
    if (!bytes) {
        RXAS_PANIC_OOM(context, "malloc rxas jump table binary key", byte_len ? byte_len : 1, 0);
    }
    b = bytes;
    while (*hex) {
        int high = hexchar2int(*hex++);
        int low = hexchar2int(*hex++);
        if (high < 0 || low < 0) {
            free(bytes);
            rxaserat(context, token, "invalid hex byte in jump table key");
            return 0;
        }
        *b++ = (unsigned char)((high << 4) | low);
    }
    *length_out = byte_len;
    return bytes;
}

static unsigned char *string_token_to_key(Assembler_Context *context, Assembler_Token *token, size_t *length_out) {
    char *raw;
    unsigned char *key;
    size_t raw_length;
    size_t key_length;

    raw = (char *)token->token_value.string;
    raw_length = strlen(raw);
    key = malloc(raw_length + 1);
    if (!key) {
        RXAS_PANIC_OOM(context, "malloc rxas jump table string key", raw_length + 1, 0);
    }
    key_length = unescape_string((char *)key, raw);
#ifndef NUTF8
    {
        size_t chars;
        void *invalid = utf8nvalid_count((char *)key, key_length, &chars);
        if (invalid) {
            char errorBuffer[MAX_ERROR_LENGTH];
            snprintf(errorBuffer, sizeof(errorBuffer),
                     "jump table string key is not valid UTF-8 at byte %lu",
                     (unsigned long)((char *)invalid - (char *)key));
            rxaserat(context, token, errorBuffer);
            free(key);
            return 0;
        }
    }
#endif
    *length_out = key_length;
    return key;
}

static unsigned char *int_token_to_key(Assembler_Context *context, Assembler_Token *token, size_t *length_out) {
    unsigned char *key;
    uint64_t value;
    int i;

    key = malloc(8);
    if (!key) {
        RXAS_PANIC_OOM(context, "malloc rxas jump table integer key", 8, 0);
    }
    value = (uint64_t)(int64_t)token->token_value.integer;
    for (i = 0; i < 8; i++) {
        key[i] = (unsigned char)((value >> (i * 8)) & 0xffu);
    }
    *length_out = 8;
    return key;
}

static unsigned char *jump_table_key_from_token(Assembler_Context *context, Assembler_Token *token,
                                                int *kind_out, size_t *length_out) {
    *kind_out = jump_table_key_kind_from_token(token);
    switch (*kind_out) {
        case RXAS_JTABLE_KEY_STRING:
            return string_token_to_key(context, token, length_out);
        case RXAS_JTABLE_KEY_BINARY:
            return hex_token_to_bytes(context, token, length_out);
        case RXAS_JTABLE_KEY_INT:
            return int_token_to_key(context, token, length_out);
        default:
            rxaserat(context, token, "jump table case key must be a string, binary, or integer literal");
            return 0;
    }
}

static void append_jump_table_case(Assembler_Context *context, struct rxas_jump_table *table,
                                   struct rxas_jtable_case *entry) {
    (void)context;
    if (!table->cases) {
        table->cases = entry;
    } else table->cases_tail->next = entry;
    table->cases_tail = entry;
}

static void add_jump_table_ref(Assembler_Context *context, struct rxas_jump_table *table,
                               Assembler_Token *token, size_t operand_index) {
    struct rxas_jtable_ref *ref;

    ref = malloc(sizeof(*ref));
    if (!ref) {
        RXAS_PANIC_OOM(context, "malloc rxas jump table reference", sizeof(*ref), 0);
    }
    ref->operand_index = operand_index;
    ref->token = token;
    ref->next = table->refs;
    table->refs = ref;
}

static int jump_instruction_table_operand(Assembler_Token *instrToken, Assembler_Token *operand2Token,
                                          Assembler_Token *operand3Token, Assembler_Token **tableToken,
                                          int *key_kind, int *match_mode, int *is_jumpbs) {
    char *mnemonic;

    if (!instrToken || instrToken->token_type != ID) return 0;
    mnemonic = (char *)instrToken->token_value.string;
    *is_jumpbs = 0;
    *match_mode = RXAS_JTABLE_MATCH_EXACT;
    if (mnemonic_matches(mnemonic, "JUMPS")) {
        *tableToken = operand2Token;
        *key_kind = RXAS_JTABLE_KEY_STRING;
        return 1;
    }
    if (mnemonic_matches(mnemonic, "JUMPB")) {
        *tableToken = operand2Token;
        *key_kind = RXAS_JTABLE_KEY_BINARY;
        return 1;
    }
    if (mnemonic_matches(mnemonic, "JUMPBS")) {
        *tableToken = operand3Token;
        *key_kind = RXAS_JTABLE_KEY_BINARY;
        *is_jumpbs = 1;
        return 1;
    }
    if (mnemonic_matches(mnemonic, "JUMPI")) {
        *tableToken = operand2Token;
        *key_kind = RXAS_JTABLE_KEY_INT;
        return 1;
    }
    if (mnemonic_matches(mnemonic, "JUMPR")) {
        *tableToken = operand2Token;
        *key_kind = RXAS_JTABLE_KEY_STRING;
        *match_mode = RXAS_JTABLE_MATCH_PADDED;
        return 1;
    }
    if (mnemonic_matches(mnemonic, "JUMPN")) {
        *tableToken = operand2Token;
        *key_kind = RXAS_JTABLE_KEY_STRING;
        *match_mode = RXAS_JTABLE_MATCH_NUMERIC;
        return 1;
    }
    return 0;
}

static void prepare_jump_table_instruction(Assembler_Context *context, Assembler_Token *instrToken,
                                           Assembler_Token *operand2Token, Assembler_Token *operand3Token) {
    Assembler_Token *tableToken;
    struct rxas_jump_table *table;
    int key_kind;
    int match_mode;
    int is_jumpbs;

    if (!jump_instruction_table_operand(instrToken, operand2Token, operand3Token,
                                        &tableToken, &key_kind, &match_mode, &is_jumpbs)) return;
    if (!tableToken || tableToken->token_type != ID) {
        rxaserat(context, instrToken, "jump table instruction requires a .jtable name operand");
        return;
    }
    table = get_or_create_jump_table(context, tableToken);
    if (!table) return;
    mark_jump_table_key_kind(context, table, key_kind, tableToken);
    if (table->match_mode_set && table->match_mode != match_mode) {
        rxaserat(context, tableToken, "jump table cannot mix exact, padded, and numeric lookup instructions");
    } else {
        table->match_mode = match_mode;
        table->match_mode_set = 1;
    }
    if (is_jumpbs) table->used_by_jumpbs = 1;
}

void rxasjtab(Assembler_Context *context, Assembler_Token *nameToken, Assembler_Token *algorithmToken) {
    struct rxas_jump_table *table;
    int algorithm;

    table = get_or_create_jump_table(context, nameToken);
    if (!table) return;
    if (table->declared) {
        rxaserat(context, nameToken, "duplicate jump table declaration");
        return;
    }

    algorithm = parse_jump_table_algorithm(context, algorithmToken);
    if (algorithm < 0) return;
    table->declared = 1;
    table->decl_token = nameToken;
    table->algorithm = algorithm;
}

static void add_jump_table_case_for_label(Assembler_Context *context, Assembler_Token *labelToken,
                                          Assembler_Token *tableToken, Assembler_Token *valueToken) {
    struct rxas_jump_table *table;
    struct rxas_jtable_case *entry;
    int key_kind;
    size_t key_length;
    unsigned char *key;

    table = get_or_create_jump_table(context, tableToken);
    if (!table) return;

    key = jump_table_key_from_token(context, valueToken, &key_kind, &key_length);
    if (!key) return;
    mark_jump_table_key_kind(context, table, key_kind, valueToken);

    entry = calloc(1, sizeof(*entry));
    if (!entry) {
        free(key);
        RXAS_PANIC_OOM(context, "calloc rxas jump table case", sizeof(*entry), 0);
    }
    entry->label_token = labelToken;
    entry->value_token = valueToken;
    entry->key = key;
    entry->key_length = key_length;
    append_jump_table_case(context, table, entry);
}

void rxasjcase(Assembler_Context *context, Assembler_Token *labelToken, Assembler_Token *tableToken,
               Assembler_Token *valueToken) {
    rxasqlbl(context, labelToken);
    context->last_label_token = labelToken;
    add_jump_table_case_for_label(context, labelToken, tableToken, valueToken);
}

void rxasjcase_after_label(Assembler_Context *context, Assembler_Token *jcaseToken,
                           Assembler_Token *tableToken, Assembler_Token *valueToken) {
    if (!context->last_label_token || context->last_label_token->line != jcaseToken->line) {
        rxaserat(context, jcaseToken, ".jcase must decorate a same-line label");
        return;
    }
    add_jump_table_case_for_label(context, context->last_label_token, tableToken, valueToken);
}

struct rxas_jtable_info {
    size_t case_count;
    size_t key_blob_length;
    size_t fixed_key_length;
};

struct rxas_jtable_key_view {
    struct rxas_jtable_case *entry;
};

static int rxas_jtable_key_view_compare(const void *left, const void *right) {
    const struct rxas_jtable_key_view *left_view = left;
    const struct rxas_jtable_key_view *right_view = right;
    size_t left_length = left_view->entry->key_length;
    size_t right_length = right_view->entry->key_length;
    size_t common = left_length < right_length ? left_length : right_length;
    int comparison = memcmp(left_view->entry->key, right_view->entry->key, common);

    if (comparison) return comparison;
    if (left_length < right_length) return -1;
    if (left_length > right_length) return 1;
    return 0;
}

static int normalize_jump_table_keys(Assembler_Context *context, struct rxas_jump_table *table) {
    struct rxas_jtable_case *entry;
    struct rxas_jtable_key_view *views;
    size_t count = 0;
    size_t index;

    if (table->key_kind == RXAS_JTABLE_KEY_STRING &&
        table->match_mode != RXAS_JTABLE_MATCH_EXACT) {
        for (entry = table->cases; entry; entry = entry->next) {
            count++;
            if (table->match_mode == RXAS_JTABLE_MATCH_PADDED) {
                while (entry->key_length > 0 && entry->key[entry->key_length - 1] == ' ') {
                    entry->key_length--;
                }
            } else {
                unsigned char numeric_key[RX_NUMERIC_KEY_SIZE];
                unsigned char *replacement;
                int is_nan = 0;

                if (!rx_numeric_key_from_text(numeric_key,
                                              (const char *)entry->key,
                                              entry->key_length,
                                              &is_nan) || is_nan) {
                    rxaserat(context, entry->value_token,
                             "jumpn case key must be a non-NaN numeric string");
                    return 0;
                }
                replacement = malloc(sizeof(numeric_key));
                if (!replacement) {
                    RXAS_PANIC_OOM(context, "malloc numeric jump table key", sizeof(numeric_key), 0);
                }
                memcpy(replacement, numeric_key, sizeof(numeric_key));
                free(entry->key);
                entry->key = replacement;
                entry->key_length = sizeof(numeric_key);
            }
        }
    } else {
        for (entry = table->cases; entry; entry = entry->next) count++;
    }

    if (count > SIZE_MAX / sizeof(*views)) return 0;
    views = malloc((count ? count : 1) * sizeof(*views));
    if (!views) {
        RXAS_PANIC_OOM(context, "malloc jump table duplicate views",
                       (count ? count : 1) * sizeof(*views), 0);
    }
    index = 0;
    for (entry = table->cases; entry; entry = entry->next) views[index++].entry = entry;
    qsort(views, count, sizeof(*views), rxas_jtable_key_view_compare);
    for (index = 1; index < count; index++) {
        if (rxas_jtable_key_view_compare(&views[index - 1], &views[index]) == 0) {
            char *message = table->match_mode == RXAS_JTABLE_MATCH_EXACT ?
                            "duplicate jump table case key" :
                            "duplicate jump table case key after canonicalization";
            rxaserat(context, views[index].entry->value_token, message);
            free(views);
            return 0;
        }
    }
    free(views);

    if (table->match_mode == RXAS_JTABLE_MATCH_NUMERIC && table->cases) {
        struct rxas_jtable_case *alias = calloc(1, sizeof(*alias));
        unsigned char *key = malloc(RX_NUMERIC_KEY_SIZE);
        double nan_value = NAN;

        if (!alias || !key) {
            free(alias);
            free(key);
            RXAS_PANIC_OOM(context, "allocate numeric jump table NaN alias",
                           sizeof(*alias) + RX_NUMERIC_KEY_SIZE, 0);
        }
        rx_double_to_numeric_key(nan_value, key);
        alias->label_token = table->cases->label_token;
        alias->value_token = table->cases->value_token;
        alias->key = key;
        alias->key_length = RX_NUMERIC_KEY_SIZE;
        table->cases_tail->next = alias;
        table->cases_tail = alias;
    }
    return 1;
}

static int prepare_jump_table(Assembler_Context *context, struct rxas_jump_table *table,
                              struct rxas_jtable_info *info) {
    struct rxas_jtable_case *entry;
    struct rxas_jtable_ref *ref;
    int variable_length;

    memset(info, 0, sizeof(*info));
    if (!normalize_jump_table_keys(context, table)) return 0;
    variable_length = 0;
    entry = table->cases;
    while (entry) {
        size_t tree_index;
        struct backpatching *label_patch;

        if (entry->key_length > UINT32_MAX) {
            rxaserat(context, entry->value_token, "jump table key length is too large");
            return 0;
        }
        if (info->case_count == 0) info->fixed_key_length = entry->key_length;
        else if (entry->key_length != info->fixed_key_length) variable_length = 1;
        if (SIZE_MAX - info->key_blob_length < entry->key_length) {
            rxaserat(context, entry->value_token, "jump table key data is too large");
            return 0;
        }
        info->key_blob_length += entry->key_length;
        info->case_count++;

        if (!src_node(context->label_constants_tree, (char *)entry->label_token->token_value.string, &tree_index)) {
            rxaserat(context, entry->label_token, "unknown jump table case label");
            return 0;
        }
        label_patch = (struct backpatching *)tree_index;
        if (!label_patch->defined) {
            rxaserat(context, entry->label_token, "unknown jump table case label");
            return 0;
        }
        if (label_patch->index > UINT32_MAX) {
            rxaserat(context, entry->label_token, "jump table target address is too large");
            return 0;
        }
        entry->target = (uint32_t)label_patch->index;
        entry = entry->next;
    }

    if (info->case_count == 0) {
        rxaserat(context, table->decl_token, "jump table has no cases");
        return 0;
    }
    if (info->case_count > UINT32_MAX) {
        rxaserat(context, table->decl_token, "jump table has too many cases");
        return 0;
    }
    if (info->key_blob_length > UINT32_MAX - RX_JTABLE_HEADER_SIZE) {
        rxaserat(context, table->decl_token, "jump table key data is too large");
        return 0;
    }
    if (variable_length) info->fixed_key_length = 0;
    if (info->fixed_key_length > UINT32_MAX) {
        rxaserat(context, table->decl_token, "jump table key length is too large");
        return 0;
    }
    if (table->used_by_jumpbs && info->fixed_key_length == 0) {
        ref = table->refs;
        while (ref && !ref->token) ref = ref->next;
        rxaserat(context, ref ? ref->token : table->decl_token,
                 "jumpbs requires fixed-length non-empty binary keys");
        return 0;
    }
    return 1;
}

static unsigned char *build_linear_jump_table(Assembler_Context *context, struct rxas_jump_table *table,
                                              const struct rxas_jtable_info *info, size_t *length_out) {
    struct rxas_jtable_case *entry;
    unsigned char *payload;
    size_t entries_length;
    size_t total_length;
    size_t entry_offset;
    size_t key_offset;

    if (info->case_count > SIZE_MAX / RX_JTABLE_LINEAR_ENTRY_SIZE) {
        rxaserat(context, table->decl_token, "jump table entries are too large");
        return NULL;
    }
    entries_length = info->case_count * RX_JTABLE_LINEAR_ENTRY_SIZE;
    if (SIZE_MAX - RX_JTABLE_HEADER_SIZE < entries_length ||
        SIZE_MAX - RX_JTABLE_HEADER_SIZE - entries_length < info->key_blob_length) {
        rxaserat(context, table->decl_token, "jump table payload is too large");
        return NULL;
    }
    total_length = RX_JTABLE_HEADER_SIZE + entries_length + info->key_blob_length;
    if (total_length > UINT32_MAX) {
        rxaserat(context, table->decl_token, "jump table payload is too large");
        return NULL;
    }
    payload = calloc(total_length ? total_length : 1, 1);
    if (!payload) {
        RXAS_PANIC_OOM(context, "calloc rxas jump table payload", total_length ? total_length : 1, 0);
    }

    payload[0] = RX_JTABLE_ALG_LINEAR;
    payload[1] = 0;
    rxas_write_u16le(payload + 2, RX_JTABLE_HEADER_SIZE);
    rxas_write_u32le(payload + 4, (uint32_t)info->fixed_key_length);
    rxas_write_u32le(payload + 8, (uint32_t)info->case_count);

    entry_offset = RX_JTABLE_HEADER_SIZE;
    key_offset = RX_JTABLE_HEADER_SIZE + entries_length;
    entry = table->cases;
    while (entry) {
        rxas_write_u32le(payload + entry_offset, (uint32_t)key_offset);
        rxas_write_u32le(payload + entry_offset + 4, (uint32_t)entry->key_length);
        rxas_write_u32le(payload + entry_offset + 8, entry->target);
        if (entry->key_length) memcpy(payload + key_offset, entry->key, entry->key_length);
        key_offset += entry->key_length;
        entry_offset += RX_JTABLE_LINEAR_ENTRY_SIZE;
        entry = entry->next;
    }

    *length_out = total_length;
    return payload;
}

static unsigned char *build_openhash_jump_table(Assembler_Context *context, struct rxas_jump_table *table,
                                                const struct rxas_jtable_info *info, size_t *length_out) {
    struct rxas_jtable_case *entry;
    unsigned char *payload;
    size_t desired_slots;
    size_t slot_count;
    size_t slots_length;
    size_t total_length;
    size_t key_offset;
    size_t i;

    if (info->case_count > SIZE_MAX / 2u) {
        rxaserat(context, table->decl_token, "jump table has too many cases");
        return NULL;
    }
    desired_slots = info->case_count * 2u;
    slot_count = 1u;
    while (slot_count < desired_slots) {
        if (slot_count > UINT32_MAX / 2u) {
            rxaserat(context, table->decl_token, "jump table has too many cases");
            return NULL;
        }
        slot_count *= 2u;
    }
    if (slot_count > SIZE_MAX / RX_JTABLE_OPEN_SLOT_SIZE) {
        rxaserat(context, table->decl_token, "jump table entries are too large");
        return NULL;
    }
    slots_length = slot_count * RX_JTABLE_OPEN_SLOT_SIZE;
    if (SIZE_MAX - RX_JTABLE_OPEN_HEADER_SIZE < slots_length ||
        SIZE_MAX - RX_JTABLE_OPEN_HEADER_SIZE - slots_length < info->key_blob_length) {
        rxaserat(context, table->decl_token, "jump table payload is too large");
        return NULL;
    }
    total_length = RX_JTABLE_OPEN_HEADER_SIZE + slots_length + info->key_blob_length;
    if (total_length >= UINT32_MAX) {
        rxaserat(context, table->decl_token, "jump table payload is too large");
        return NULL;
    }
    payload = calloc(total_length, 1);
    if (!payload) RXAS_PANIC_OOM(context, "calloc open-hash jump table", total_length, 0);

    payload[0] = RX_JTABLE_ALG_OPENHASH;
    rxas_write_u16le(payload + 2, RX_JTABLE_OPEN_HEADER_SIZE);
    rxas_write_u32le(payload + 4, (uint32_t)info->fixed_key_length);
    rxas_write_u32le(payload + 8, (uint32_t)info->case_count);
    rxas_write_u32le(payload + 12, (uint32_t)slot_count);
    for (i = 0; i < slot_count; i++) {
        rxas_write_u32le(payload + RX_JTABLE_OPEN_HEADER_SIZE + i * RX_JTABLE_OPEN_SLOT_SIZE + 4,
                         RX_JTABLE_OPEN_EMPTY);
    }

    key_offset = RX_JTABLE_OPEN_HEADER_SIZE + slots_length;
    entry = table->cases;
    while (entry) {
        uint32_t hash = rx_jtable_hash_bytes(entry->key, entry->key_length);
        size_t slot = hash & (slot_count - 1u);
        size_t slot_offset;

        while ((uint32_t)payload[RX_JTABLE_OPEN_HEADER_SIZE + slot * RX_JTABLE_OPEN_SLOT_SIZE + 4] != 0xffu ||
               (uint32_t)payload[RX_JTABLE_OPEN_HEADER_SIZE + slot * RX_JTABLE_OPEN_SLOT_SIZE + 5] != 0xffu ||
               (uint32_t)payload[RX_JTABLE_OPEN_HEADER_SIZE + slot * RX_JTABLE_OPEN_SLOT_SIZE + 6] != 0xffu ||
               (uint32_t)payload[RX_JTABLE_OPEN_HEADER_SIZE + slot * RX_JTABLE_OPEN_SLOT_SIZE + 7] != 0xffu) {
            slot = (slot + 1u) & (slot_count - 1u);
        }
        slot_offset = RX_JTABLE_OPEN_HEADER_SIZE + slot * RX_JTABLE_OPEN_SLOT_SIZE;
        rxas_write_u32le(payload + slot_offset, hash);
        rxas_write_u32le(payload + slot_offset + 4, (uint32_t)key_offset);
        rxas_write_u32le(payload + slot_offset + 8, (uint32_t)entry->key_length);
        rxas_write_u32le(payload + slot_offset + 12, entry->target);
        if (entry->key_length) memcpy(payload + key_offset, entry->key, entry->key_length);
        key_offset += entry->key_length;
        entry = entry->next;
    }

    *length_out = total_length;
    return payload;
}

struct rxas_acph_job {
    struct rxas_acph_node **destination;
    struct rxas_jtable_case **cases;
    size_t case_count;
    struct rxas_acph_job *next;
};

static uint16_t acph_symbol_at(const struct rxas_jtable_case *entry, size_t column) {
    return column < entry->key_length ? entry->key[column] : RX_JTABLE_ACPH_END_SYMBOL;
}

static int acph_select_hash(struct rxas_jtable_case **cases, size_t case_count,
                            uint32_t *column_out, uint16_t *slot_count_out,
                            unsigned char *prime_out) {
    static const unsigned char primes[] = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53,
        59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 113, 127,
        131, 137, 149, 151, 157, 163, 167, 173, 211, 223, 227, 229,
        233, 239, 241, 251
    };
    size_t best_column = 0;
    size_t best_max_group = case_count + 1u;
    size_t best_unique = 0;
    unsigned char best_present[RX_JTABLE_ACPH_SYMBOL_COUNT] = {0};
    size_t max_length = 0;
    size_t column;
    size_t i;

    for (i = 0; i < case_count; i++) {
        if (cases[i]->key_length > max_length) max_length = cases[i]->key_length;
    }
    if (max_length > UINT32_MAX) return 0;

    for (column = 0; column <= max_length; column++) {
        size_t counts[RX_JTABLE_ACPH_SYMBOL_COUNT] = {0};
        size_t max_group = 0;
        size_t unique = 0;

        for (i = 0; i < case_count; i++) {
            uint16_t symbol = acph_symbol_at(cases[i], column);
            counts[symbol]++;
            if (counts[symbol] == 1u) unique++;
            if (counts[symbol] > max_group) max_group = counts[symbol];
        }
        if (max_group < best_max_group) {
            size_t symbol;
            best_column = column;
            best_max_group = max_group;
            best_unique = unique;
            for (symbol = 0; symbol < RX_JTABLE_ACPH_SYMBOL_COUNT; symbol++) {
                best_present[symbol] = counts[symbol] != 0;
            }
        }
    }
    if (case_count > 1u && best_unique < 2u) return 0;

    for (i = best_unique ? best_unique : 1u; i <= RX_JTABLE_ACPH_SYMBOL_COUNT; i++) {
        size_t prime_index;

        if (i == RX_JTABLE_ACPH_SYMBOL_COUNT) {
            *column_out = (uint32_t)best_column;
            *slot_count_out = (uint16_t)i;
            *prime_out = 1u;
            return 1;
        }
        for (prime_index = 0; prime_index < sizeof(primes); prime_index++) {
            uint16_t used[RX_JTABLE_ACPH_SYMBOL_COUNT];
            size_t symbol;
            int collision = 0;

            for (symbol = 0; symbol < i; symbol++) used[symbol] = UINT16_MAX;
            for (symbol = 0; symbol < RX_JTABLE_ACPH_SYMBOL_COUNT; symbol++) {
                size_t slot;

                if (!best_present[symbol]) continue;
                slot = rx_jtable_acph_hash((uint16_t)symbol, primes[prime_index], (uint16_t)i);
                if (used[slot] != UINT16_MAX && used[slot] != symbol) {
                    collision = 1;
                    break;
                }
                used[slot] = (uint16_t)symbol;
            }
            if (!collision) {
                *column_out = (uint32_t)best_column;
                *slot_count_out = (uint16_t)i;
                *prime_out = primes[prime_index];
                return 1;
            }
        }
    }
    return 0;
}

static void free_acph_nodes(struct rxas_acph_node *nodes) {
    while (nodes) {
        struct rxas_acph_node *next = nodes->next_all;
        free(nodes->slots);
        free(nodes);
        nodes = next;
    }
}

static void free_acph_jobs(struct rxas_acph_job *jobs) {
    while (jobs) {
        struct rxas_acph_job *next = jobs->next;
        free(jobs->cases);
        free(jobs);
        jobs = next;
    }
}

static int build_acph_nodes(Assembler_Context *context, struct rxas_jump_table *table,
                            const struct rxas_jtable_info *info, struct rxas_acph_node **root_out,
                            struct rxas_acph_node **nodes_out) {
    struct rxas_acph_job *jobs;
    struct rxas_acph_node *all_nodes = NULL;
    struct rxas_acph_node *all_tail = NULL;
    struct rxas_jtable_case *entry;
    size_t i;

    jobs = calloc(1, sizeof(*jobs));
    if (!jobs) RXAS_PANIC_OOM(context, "calloc ACPH build job", sizeof(*jobs), 0);
    jobs->cases = malloc(info->case_count * sizeof(*jobs->cases));
    if (!jobs->cases) RXAS_PANIC_OOM(context, "malloc ACPH root cases", info->case_count * sizeof(*jobs->cases), 0);
    jobs->case_count = info->case_count;
    jobs->destination = root_out;
    entry = table->cases;
    for (i = 0; i < info->case_count; i++) {
        jobs->cases[i] = entry;
        entry = entry->next;
    }

    while (jobs) {
        struct rxas_acph_job *job = jobs;
        struct rxas_acph_node *node;
        struct rxas_jtable_case **grouped;
        size_t counts[RX_JTABLE_ACPH_SYMBOL_COUNT] = {0};
        size_t offsets[RX_JTABLE_ACPH_SYMBOL_COUNT];
        size_t placed[RX_JTABLE_ACPH_SYMBOL_COUNT] = {0};
        size_t next_offset = 0;

        jobs = job->next;
        node = calloc(1, sizeof(*node));
        if (!node) RXAS_PANIC_OOM(context, "calloc ACPH node", sizeof(*node), 0);
        if (!acph_select_hash(job->cases, job->case_count, &node->column,
                              &node->slot_count, &node->prime)) {
            rxaserat(context, table->decl_token, "could not construct ACPH jump table");
            free(node);
            free(job->cases);
            free(job);
            free_acph_jobs(jobs);
            free_acph_nodes(all_nodes);
            return 0;
        }
        node->slots = calloc(node->slot_count, sizeof(*node->slots));
        if (!node->slots) RXAS_PANIC_OOM(context, "calloc ACPH slots", node->slot_count * sizeof(*node->slots), 0);
        *job->destination = node;
        if (all_tail) all_tail->next_all = node;
        else all_nodes = node;
        all_tail = node;

        for (i = 0; i < job->case_count; i++) {
            uint16_t symbol = acph_symbol_at(job->cases[i], node->column);
            size_t slot = rx_jtable_acph_hash(symbol, node->prime, node->slot_count);
            counts[slot]++;
            node->slots[slot].symbol = symbol;
        }
        for (i = 0; i < node->slot_count; i++) {
            offsets[i] = next_offset;
            next_offset += counts[i];
        }
        grouped = malloc(job->case_count * sizeof(*grouped));
        if (!grouped) RXAS_PANIC_OOM(context, "malloc ACPH grouped cases", job->case_count * sizeof(*grouped), 0);
        for (i = 0; i < job->case_count; i++) {
            uint16_t symbol = acph_symbol_at(job->cases[i], node->column);
            size_t slot = rx_jtable_acph_hash(symbol, node->prime, node->slot_count);
            grouped[offsets[slot] + placed[slot]++] = job->cases[i];
        }
        for (i = node->slot_count; i-- > 0;) {
            if (counts[i] == 1u) {
                node->slots[i].kind = RX_JTABLE_ACPH_SLOT_LEAF;
                node->slots[i].leaf = grouped[offsets[i]];
            }
            else if (counts[i] > 1u) {
                struct rxas_acph_job *child_job = calloc(1, sizeof(*child_job));
                if (!child_job) RXAS_PANIC_OOM(context, "calloc ACPH child job", sizeof(*child_job), 0);
                child_job->cases = malloc(counts[i] * sizeof(*child_job->cases));
                if (!child_job->cases) RXAS_PANIC_OOM(context, "malloc ACPH child cases", counts[i] * sizeof(*child_job->cases), 0);
                memcpy(child_job->cases, grouped + offsets[i], counts[i] * sizeof(*child_job->cases));
                child_job->case_count = counts[i];
                child_job->destination = &node->slots[i].child;
                child_job->next = jobs;
                jobs = child_job;
                node->slots[i].kind = RX_JTABLE_ACPH_SLOT_CHILD;
            }
        }
        free(grouped);
        free(job->cases);
        free(job);
    }

    *nodes_out = all_nodes;
    return 1;
}

static unsigned char *build_acph_jump_table(Assembler_Context *context, struct rxas_jump_table *table,
                                            const struct rxas_jtable_info *info, size_t *length_out) {
    struct rxas_acph_node *root = NULL;
    struct rxas_acph_node *nodes = NULL;
    struct rxas_acph_node *node;
    unsigned char *payload;
    size_t offset;
    size_t leaf_offset;
    size_t key_offset;
    size_t total_length;
    size_t i;

    if (!build_acph_nodes(context, table, info, &root, &nodes)) return NULL;
    offset = RX_JTABLE_ACPH_HEADER_SIZE;
    for (node = nodes; node; node = node->next_all) {
        size_t node_length = RX_JTABLE_ACPH_NODE_SIZE + (size_t)node->slot_count * RX_JTABLE_ACPH_SLOT_SIZE;
        if (SIZE_MAX - offset < node_length || offset + node_length > UINT32_MAX) {
            rxaserat(context, table->decl_token, "ACPH jump table nodes are too large");
            free_acph_nodes(nodes);
            return NULL;
        }
        node->packed_offset = (uint32_t)offset;
        offset += node_length;
    }
    if (info->case_count > (SIZE_MAX - offset) / RX_JTABLE_ACPH_LEAF_SIZE) {
        rxaserat(context, table->decl_token, "ACPH jump table leaves are too large");
        free_acph_nodes(nodes);
        return NULL;
    }
    leaf_offset = offset;
    offset += info->case_count * RX_JTABLE_ACPH_LEAF_SIZE;
    key_offset = offset;
    if (SIZE_MAX - key_offset < info->key_blob_length || key_offset + info->key_blob_length > UINT32_MAX) {
        rxaserat(context, table->decl_token, "ACPH jump table payload is too large");
        free_acph_nodes(nodes);
        return NULL;
    }
    total_length = key_offset + info->key_blob_length;

    for (node = nodes; node; node = node->next_all) {
        for (i = 0; i < node->slot_count; i++) {
            if (node->slots[i].kind == RX_JTABLE_ACPH_SLOT_LEAF) {
                node->slots[i].packed_value_offset = (uint32_t)leaf_offset;
                leaf_offset += RX_JTABLE_ACPH_LEAF_SIZE;
            }
        }
    }

    payload = calloc(total_length, 1);
    if (!payload) RXAS_PANIC_OOM(context, "calloc ACPH jump table", total_length, 0);
    payload[0] = RX_JTABLE_ALG_ACPH;
    rxas_write_u16le(payload + 2, RX_JTABLE_ACPH_HEADER_SIZE);
    rxas_write_u32le(payload + 4, (uint32_t)info->fixed_key_length);
    rxas_write_u32le(payload + 8, (uint32_t)info->case_count);
    rxas_write_u32le(payload + 12, root->packed_offset);

    for (node = nodes; node; node = node->next_all) {
        size_t node_offset = node->packed_offset;
        rxas_write_u32le(payload + node_offset, node->column);
        rxas_write_u16le(payload + node_offset + 4, node->slot_count);
        payload[node_offset + 6] = node->prime;
        for (i = 0; i < node->slot_count; i++) {
            struct rxas_acph_slot *slot = &node->slots[i];
            size_t slot_offset = node_offset + RX_JTABLE_ACPH_NODE_SIZE + i * RX_JTABLE_ACPH_SLOT_SIZE;
            rxas_write_u16le(payload + slot_offset, slot->symbol);
            payload[slot_offset + 2] = slot->kind;
            if (slot->kind == RX_JTABLE_ACPH_SLOT_LEAF) {
                struct rxas_jtable_case *leaf = slot->leaf;
                rxas_write_u32le(payload + slot_offset + 4, slot->packed_value_offset);
                rxas_write_u32le(payload + slot->packed_value_offset, (uint32_t)key_offset);
                rxas_write_u32le(payload + slot->packed_value_offset + 4, (uint32_t)leaf->key_length);
                rxas_write_u32le(payload + slot->packed_value_offset + 8, leaf->target);
                if (leaf->key_length) memcpy(payload + key_offset, leaf->key, leaf->key_length);
                key_offset += leaf->key_length;
            }
            else if (slot->kind == RX_JTABLE_ACPH_SLOT_CHILD) {
                rxas_write_u32le(payload + slot_offset + 4, slot->child->packed_offset);
            }
        }
    }
    free_acph_nodes(nodes);
    *length_out = total_length;
    return payload;
}

static int build_one_jump_table(Assembler_Context *context, struct rxas_jump_table *table) {
    struct rxas_jtable_ref *ref;
    struct rxas_jtable_info info;
    unsigned char *payload;
    size_t total_length;
    size_t pool_index;
    int algorithm;

    if (!table->declared) {
        if (table->refs) rxaserat(context, table->refs->token, "unknown jump table");
        else if (table->cases) rxaserat(context, table->cases->value_token, "unknown jump table");
        return 0;
    }
    if (!prepare_jump_table(context, table, &info)) return 0;

    algorithm = table->algorithm == RXAS_JTABLE_ALG_AUTO
                ? rx_jtable_select_auto(info.case_count, info.key_blob_length)
                : table->algorithm;
    switch (algorithm) {
        case RXAS_JTABLE_ALG_LINEAR:
            payload = build_linear_jump_table(context, table, &info, &total_length);
            break;
        case RXAS_JTABLE_ALG_OPENHASH:
            payload = build_openhash_jump_table(context, table, &info, &total_length);
            break;
        case RXAS_JTABLE_ALG_ACPH:
            payload = build_acph_jump_table(context, table, &info, &total_length);
            break;
        default:
            payload = NULL;
            rxaserat(context, table->decl_token, "unknown jump table algorithm");
            break;
    }
    if (!payload) return 0;

    pool_index = add_raw_binary_to_pool(context, payload, total_length);
    free(payload);

    ref = table->refs;
    while (ref) {
        context->binary.binary[ref->operand_index].index = pool_index;
        ref = ref->next;
    }

    return 1;
}

static void backpatch_jump_tables(Assembler_Context *context) {
    struct rxas_jump_table *table;

    table = context->jump_tables;
    while (table) {
        build_one_jump_table(context, table);
        table = table->next;
    }
}

static rxas_constant_alias *find_constant_alias(Assembler_Context *context, Assembler_Token *token) {
    size_t value;

    if (!context || !token || token->token_type != ID) return 0;
    if (!src_node(context->constant_aliases_tree, (char *)token->token_value.string, &value)) return 0;
    return (rxas_constant_alias *)value;
}

void rxasconst(Assembler_Context *context, Assembler_Token *nameToken, Assembler_Token *kindToken,
               Assembler_Token *valueToken) {
    rxas_constant_alias *alias;
    size_t existing_alias;
    size_t pool_index;
    OperandType operand_type;
    const char *kind = (const char *)kindToken->token_value.string;

    if (src_node(context->constant_aliases_tree, (char *)nameToken->token_value.string, &existing_alias)) {
        rxaserat(context, nameToken, "duplicate constant alias");
        return;
    }

    if (strcmp(kind, "binary") == 0) {
        if (valueToken->token_type != HEX) {
            rxaserat(context, valueToken, "binary constant alias requires a hex literal");
            return;
        }
        pool_index = add_binary_to_pool(context, (char *)valueToken->token_value.string);
        operand_type = OP_BINARY;
    }
    else if (strcmp(kind, "string") == 0) {
        if (valueToken->token_type != STRING) {
            rxaserat(context, valueToken, "string constant alias requires a string literal");
            return;
        }
        pool_index = add_string_to_pool(context, valueToken, (char *)valueToken->token_value.string);
        if (pool_index == SIZE_MAX) return;
        operand_type = OP_STRING;
    }
    else {
        rxaserat(context, kindToken, "constant alias kind must be binary or string");
        return;
    }

    alias = malloc(sizeof(*alias));
    if (!alias) {
        RXAS_PANIC_OOM(context, "malloc rxas constant alias", sizeof(*alias), 0);
    }
    alias->operand_type = operand_type;
    alias->pool_index = pool_index;
    if (add_node(&context->constant_aliases_tree, (char *)nameToken->token_value.string, (size_t)alias)) {
        free(alias);
        rxaserat(context, nameToken, "duplicate constant alias");
    }
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
        if (!ref_header) {
            RXAS_PANIC_OOM(context, "malloc rxas procedure backpatch header",
                           sizeof(struct backpatching), 0);
        }
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
        if (!ref) {
            RXAS_PANIC_OOM(context, "malloc rxas procedure backpatch reference",
                           sizeof(struct backpatching_references), 0);
        }
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
    void *new_binary;
    if (context->inst_buffer_size <= context->binary.inst_size + 1) { /* +1 = Make room for the end null */
        new_size = context->inst_buffer_size * 2;
        new_binary = realloc(context->binary.binary, new_size * sizeof(bin_code));
        if (!new_binary) {
            RXAS_PANIC_OOM(context, "realloc rxas instruction buffer for operand",
                           new_size * sizeof(bin_code), 0);
        }
        context->binary.binary = new_binary;
        memset(context->binary.binary + context->inst_buffer_size, 0,
               context->inst_buffer_size * sizeof(bin_code));
        context->inst_buffer_size = new_size;
    }

    size_t entry_index;
    struct backpatching *ref_header;

    switch(operandToken->token_type) {
        case ID:
            {
                rxas_constant_alias *alias = find_constant_alias(context, operandToken);
                if (alias) {
                    context->binary.binary[context->binary.inst_size++].index = alias->pool_index;
                    return;
                }
            }
            {
                struct rxas_jump_table *jump_table = find_jump_table(context, (char *)operandToken->token_value.string,
                                                                     context->current_proc_name);
                if (jump_table) {
                    add_jump_table_ref(context, jump_table, operandToken, context->binary.inst_size);
                    context->binary.binary[context->binary.inst_size++].index = 0;
                    return;
                }
            }
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
                if (!ref_header) {
                    RXAS_PANIC_OOM(context, "malloc rxas label backpatch header",
                                   sizeof(struct backpatching), 0);
                }
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
            if (!ref) {
                RXAS_PANIC_OOM(context, "malloc rxas label backpatch reference",
                               sizeof(struct backpatching_references), 0);
            }
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
            s_index = add_float_to_pool(context, operandToken->token_value.real);
            context->binary.binary[context->binary.inst_size++].index = s_index;
            return;
        case CHAR:
            context->binary.binary[context->binary.inst_size++].cconst =
                    (char)operandToken->token_value.character;
            return;
        case STRING:
            s_index = add_string_to_pool(context, operandToken, (char*)operandToken->token_value.string);;
            context->binary.binary[context->binary.inst_size++].index = s_index;
            return;
        case DECIMAL:
            s_index = add_decimal_to_pool(context, (char*)operandToken->token_value.string);
            context->binary.binary[context->binary.inst_size++].index = s_index;
            return;
        case HEX:
            s_index = add_binary_to_pool(context, (char*)operandToken->token_value.string);
            context->binary.binary[context->binary.inst_size++].index = s_index;
            return;
        default:
            printf("**gen_operand() error**\n");
            return;
    }

}

static OperandType token_to_operand_type(Assembler_Context *context, Assembler_Token *token) {
    if (!token) return OP_NONE;
    if (token->token_type == ID) {
        rxas_constant_alias *alias = find_constant_alias(context, token);
        if (alias) return alias->operand_type;
        if (context && context->current_proc_name &&
            find_jump_table(context, (char *)token->token_value.string, context->current_proc_name)) {
            return OP_BINARY;
        }
    }

    switch(token->token_type) {
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
        case DECIMAL: return OP_DECIMAL;
        case HEX: return OP_BINARY;
        default: return OP_NONE;
    }
}

static void convert_float_to_decimal(Assembler_Token *token) {
    token->token_type = DECIMAL;
    memcpy(token->token_value.string, token->token_source, token->length);
    token->token_value.string[token->length] = 0;
}

/* Convert FLOAT tokens to DECIMAL where a matching decimal overload requires it. */
void promote_floats_to_decimalsv(Assembler_Token *instrToken,
                                 Assembler_Token *const *operandTokens,
                                 size_t operandCount) {
    const char *mnemonic = (const char *)instrToken->token_value.string;
    OperandType *actual;
    size_t i;
    int candidate;

    if (!operandCount) return;
    actual = malloc(operandCount * sizeof(*actual));
    if (!actual) RXAS_PANIC_OOM(0, "malloc rxas operand types", operandCount * sizeof(*actual), 0);

    for (i = 0; i < operandCount; i++) actual[i] = token_to_operand_type(0, operandTokens[i]);
    if (find_opcodev(mnemonic, actual, operandCount)) {
        free(actual);
        return;
    }

    for (candidate = 0; op_table[candidate].mnemonic != NULL; candidate++) {
        int matches = mnemonic_matches(mnemonic, op_table[candidate].mnemonic) &&
                      rxop_format_operand_count(op_table[candidate].format) == operandCount;
        int promotes = 0;

        if (!rxop_is_source_mnemonic(op_table[candidate].mnemonic)) continue;
        for (i = 0; matches && i < operandCount; i++) {
            OperandType expected = rxop_format_operand_type(op_table[candidate].format, i);
            if (actual[i] == expected) continue;
            if (actual[i] == OP_FLOAT && expected == OP_DECIMAL) {
                promotes = 1;
                continue;
            }
            matches = 0;
        }
        if (matches && promotes) {
            for (i = 0; i < operandCount; i++) {
                if (actual[i] == OP_FLOAT &&
                    rxop_format_operand_type(op_table[candidate].format, i) == OP_DECIMAL) {
                    convert_float_to_decimal(operandTokens[i]);
                }
            }
            free(actual);
            return;
        }
    }
    free(actual);
}

static void append_format_description(OpFormat format, char *buffer, size_t buffer_len) {
    size_t num_ops = rxop_format_operand_count(format);
    size_t i;
    if (num_ops == 0) {
        strncat(buffer, "no operands", buffer_len - strlen(buffer) - 1);
        return;
    }
    for (i = 0; i < num_ops; i++) {
        if (i > 0) strncat(buffer, ", ", buffer_len - strlen(buffer) - 1);
        switch (rxop_format_operand_type(format, i)) {
            case OP_REG: strncat(buffer, "register", buffer_len - strlen(buffer) - 1); break;
            case OP_INT: strncat(buffer, "integer", buffer_len - strlen(buffer) - 1); break;
            case OP_FLOAT: strncat(buffer, "float", buffer_len - strlen(buffer) - 1); break;
            case OP_STRING: strncat(buffer, "string", buffer_len - strlen(buffer) - 1); break;
            case OP_ID: strncat(buffer, "label", buffer_len - strlen(buffer) - 1); break;
            case OP_FUNC: strncat(buffer, "procedure", buffer_len - strlen(buffer) - 1); break;
            case OP_DECIMAL: strncat(buffer, "decimal", buffer_len - strlen(buffer) - 1); break;
            case OP_CHAR: strncat(buffer, "character", buffer_len - strlen(buffer) - 1); break;
            case OP_BINARY: strncat(buffer, "binary", buffer_len - strlen(buffer) - 1); break;
            default: strncat(buffer, "unknown", buffer_len - strlen(buffer) - 1); break;
        }
    }
}

static const OpInfo *validate_instruction(Assembler_Context* context, Assembler_Token *instrToken,
                                         const OperandType *operandTypes,
                                         size_t operandCount) {
    char errorBuffer[MAX_ERROR_LENGTH];
    size_t i_len;
    int j;
    int first = 1;
    const char *mnemonic = (char*)instrToken->token_value.string;
    const OpInfo *inst = find_opcodev(mnemonic, operandTypes, operandCount);

    if (inst) return inst;

    /* Make a useful error message */
    errorBuffer[0] = 0;
    for (j = 0; op_table[j].mnemonic != NULL; j++) {
        if (!rxop_is_source_mnemonic(op_table[j].mnemonic)) continue;
        if (mnemonic_matches(mnemonic, op_table[j].mnemonic)) {
            if (first) {
                strncpy(errorBuffer, "invalid operand, expecting ", MAX_ERROR_LENGTH - 1);
            } else {
                strncat(errorBuffer, " or ", MAX_ERROR_LENGTH - strlen(errorBuffer) - 1);
            }
            first = 0;
            i_len = strlen(errorBuffer);
            append_format_description(op_table[j].format, errorBuffer + i_len, MAX_ERROR_LENGTH - 1 - i_len);
        }
    }

    if (first) {
        rxaserat(context, instrToken, "invalid instruction mnemonic");
    } else {
        rxaseaft(context, instrToken, errorBuffer);
    }
    return 0;
}

static int channel_instruction_has_two_outputs(int opcode) {
    return opcode == OP_CHANOPEN_REG_REG_REG_REG_REG ||
           opcode == OP_CHANSTART_REG_REG_REG_REG_REG ||
           opcode == OP_CHANWAIT_REG_REG_REG_REG;
}

static int same_register_operand(const Assembler_Token *left,
                                 const Assembler_Token *right) {
    return left && right && left->token_type == right->token_type &&
           left->token_value.integer == right->token_value.integer;
}

/** Generate code for an instruction with no operands */
void rxasgen0(Assembler_Context *context, Assembler_Token *instrToken) {
    rxasgenv(context, instrToken, 0, 0);
}

/** Generate code for an instruction with one operand */
void rxasgen1(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token) {
    Assembler_Token *operands[] = {operand1Token};
    rxasgenv(context, instrToken, operands, 1);
}

/** Generate code for an instruction with two operand */
void rxasgen2(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token,
              Assembler_Token *operand2Token) {
    Assembler_Token *operands[] = {operand1Token, operand2Token};
    rxasgenv(context, instrToken, operands, 2);
}

/** Generate code for an instruction with three operands */
void rxasgen3(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token,
              Assembler_Token *operand2Token, Assembler_Token *operand3Token) {
    Assembler_Token *operands[] = {operand1Token, operand2Token, operand3Token};
    rxasgenv(context, instrToken, operands, 3);
}

/** Compatibility wrapper for instruction producers that emit up to three operands. */
void rxasgen(Assembler_Context *context, Assembler_Token *instrToken, Assembler_Token *operand1Token,
             Assembler_Token *operand2Token, Assembler_Token *operand3Token) {
    Assembler_Token *operands[3];
    size_t operandCount = 0;

    if (operand1Token) operands[operandCount++] = operand1Token;
    if (operand2Token) operands[operandCount++] = operand2Token;
    if (operand3Token) operands[operandCount++] = operand3Token;
    rxasgenv(context, instrToken, operands, operandCount);
}

/** Generate code for an instruction with an arbitrary number of operands. */
void rxasgenv(Assembler_Context *context, Assembler_Token *instrToken,
              Assembler_Token *const *operandTokens, size_t operandCount) {
    OperandType *operandTypes = 0;
    const OpInfo *inst;
    size_t i;

    if (operandCount > INT_MAX) {
        rxaseaft(context, instrToken, "instruction has too many operands");
        return;
    }

    prepare_jump_table_instruction(context, instrToken,
                                   operandCount > 1 ? operandTokens[1] : 0,
                                   operandCount > 2 ? operandTokens[2] : 0);

    if (operandCount) {
        operandTypes = malloc(operandCount * sizeof(*operandTypes));
        if (!operandTypes) {
            RXAS_PANIC_OOM(context, "malloc rxas operand types",
                           operandCount * sizeof(*operandTypes), 0);
        }
        for (i = 0; i < operandCount; i++) {
            operandTypes[i] = token_to_operand_type(context, operandTokens[i]);
        }
    }

    inst = validate_instruction(context, instrToken, operandTypes, operandCount);

    if (inst) {
        if (channel_instruction_has_two_outputs(inst->opcode) &&
            same_register_operand(operandTokens[0], operandTokens[1])) {
            rxaseaft(context, operandTokens[1],
                     "channel instruction output registers must be distinct");
            free(operandTypes);
            return;
        }
        gen_instr(context, inst->opcode, (int)operandCount);
        for (i = 0; i < operandCount; i++) gen_operand(context, operandTokens[i]);
    }
    free(operandTypes);
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
        if (!ref_header) {
            RXAS_PANIC_OOM(context, "malloc rxas defined procedure backpatch header",
                           sizeof(struct backpatching), 0);
        }
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
    centry->next = -1;
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
    context->current_proc_name = (char *)funcToken->token_value.string;
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
        if (!ref_header) {
            RXAS_PANIC_OOM(context, "malloc rxas defined label backpatch header",
                           sizeof(struct backpatching), 0);
        }
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
    context->current_proc_name = (char *)funcToken->token_value.string;

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

    append_expose_entry(context, entry_index);

    /* Proc Entry has a pointer to the external entry */
    pentry = (proc_constant*)(context->binary.const_pool + pentry_index); /* It might have moved */
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
    context->current_proc_name = 0;

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

    append_expose_entry(context, entry_index);

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

/* Source Step */
void rxasmestp(Assembler_Context *context, Assembler_Token *step, Assembler_Token *clause, Assembler_Token *flags,
               Assembler_Token *file, Assembler_Token *line, Assembler_Token *start, Assembler_Token *end,
               Assembler_Token *source) {
    size_t entry = add_meta_entry(context, sizeof(meta_source_step_constant), META_SOURCE_STEP);
    size_t file_entry = add_string_to_pool(context, file, (char*)file->token_value.string);
    size_t source_entry = add_string_to_pool(context, source, (char*)source->token_value.string);
    meta_source_step_constant *meta;

    meta = (meta_source_step_constant*)(context->binary.const_pool + entry);
    meta->file = file_entry;
    meta->source_line = source_entry;
    meta->step_id = (uint32_t) step->token_value.integer;
    meta->clause_id = (uint32_t) clause->token_value.integer;
    meta->line = (uint32_t) line->token_value.integer;
    meta->active_start_column = (uint32_t) start->token_value.integer;
    meta->active_end_column = (uint32_t) end->token_value.integer;
    meta->flags = (uint32_t) flags->token_value.integer;
}

/* Trace Event */
void rxasmete(Assembler_Context *context, Assembler_Token *kind, Assembler_Token *mode_mask,
              Assembler_Token *value_source, Assembler_Token *value_type, Assembler_Token *register_type,
              Assembler_Token *value_ref, Assembler_Token *source_step, Assembler_Token *clause,
              Assembler_Token *flags, Assembler_Token *symbol, Assembler_Token *resolved_name) {
    size_t entry = add_meta_entry(context, sizeof(meta_trace_event_constant), META_TRACE_EVENT);
    size_t symbol_entry = add_optional_string_to_pool(context, symbol);
    size_t resolved_name_entry = add_optional_string_to_pool(context, resolved_name);
    meta_trace_event_constant *meta;

    meta = (meta_trace_event_constant*)(context->binary.const_pool + entry);
    meta->kind = trace_code_from_token(context, kind, 0);
    meta->mode_mask = trace_u32_from_token(mode_mask);
    meta->value_source = trace_code_from_token(context, value_source, RXBIN_TRACE_VALUE_NONE);
    meta->value_type = trace_code_from_token(context, value_type, 0);
    meta->register_type = trace_code_from_token(context, register_type, 0);
    meta->value_ref = trace_ref_from_token(value_ref);
    meta->source_step_id = trace_u32_from_token(source_step);
    meta->clause_id = trace_u32_from_token(clause);
    meta->flags = trace_u32_from_token(flags);
    meta->symbol = symbol_entry;
    meta->resolved_name = resolved_name_entry;
}

/* Function Metadata */
void rxasmefu(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *func, Assembler_Token *args) {
    size_t entry = add_meta_entry(context,sizeof(meta_func_constant),META_FUNC);
    size_t sentry;

    /* NOTE the address in memory of the entry may change as we add (and therefor grow) the constant pool */
    sentry = add_string_to_pool(context, symbol, (char*)symbol->token_value.string);
    ((meta_func_constant*)(context->binary.const_pool + entry))->symbol = sentry;
    sentry = add_string_to_pool(context, option, (char*)option->token_value.string);
    ((meta_func_constant*)(context->binary.const_pool + entry))->option = sentry;
    sentry = add_string_to_pool(context, type, (char*)type->token_value.string);
    ((meta_func_constant*)(context->binary.const_pool + entry))->type = sentry;
    sentry = add_func_to_pool(context, func);
    ((meta_func_constant*)(context->binary.const_pool + entry))->func = sentry;
    sentry = add_string_to_pool(context, args, (char*)args->token_value.string);
    ((meta_func_constant*)(context->binary.const_pool + entry))->args = sentry;
}

/* Register Metadata */
void rxasmere(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *reg) {
    size_t entry = add_meta_entry(context,sizeof(meta_reg_constant),META_REG);
    size_t sentry;

    /* NOTE the address in memory of the entry may change as we add (and therefor grow) the constant pool */
    sentry = add_string_to_pool(context, symbol, (char*)symbol->token_value.string);
    ((meta_reg_constant*)(context->binary.const_pool + entry))->symbol = sentry;
    sentry = add_string_to_pool(context, option, (char*)option->token_value.string);
    ((meta_reg_constant*)(context->binary.const_pool + entry))->option = sentry;
    sentry = add_string_to_pool(context, type, (char*)type->token_value.string);
    ((meta_reg_constant*)(context->binary.const_pool + entry))->type = sentry;
    ((meta_reg_constant*)(context->binary.const_pool + entry))->reg = get_reg_number(context, reg);
}

/* Constant Symbol Metadata */
void rxasmect(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *constant){
    size_t entry = add_meta_entry(context,sizeof(meta_const_constant),META_CONST);
    size_t sentry;

    /* NOTE the address in memory of the entry may change as we add (and therefor grow) the constant pool */
    sentry = add_string_to_pool(context, symbol, (char*)symbol->token_value.string);
    ((meta_const_constant*)(context->binary.const_pool + entry))->symbol = sentry;
    sentry = add_string_to_pool(context, option, (char*)option->token_value.string);
    ((meta_const_constant*)(context->binary.const_pool + entry))->option = sentry;
    sentry = add_string_to_pool(context, type, (char*)type->token_value.string);
    ((meta_const_constant*)(context->binary.const_pool + entry))->type = sentry;
    sentry = add_string_to_pool(context, constant, (char*)constant->token_value.string);
    ((meta_const_constant*)(context->binary.const_pool + entry))->constant = sentry;
}

/* Clear Symbol Metadata */
void rxasmecl(Assembler_Context *context, Assembler_Token *symbol) {
    size_t entry = add_meta_entry(context,sizeof(meta_clear_constant),META_CLEAR);
    size_t sentry = add_string_to_pool(context, symbol, (char*)symbol->token_value.string);

    /* NOTE the address in memory of the entry may change as we add (and therefor grow) the constant pool */
    ((meta_clear_constant*)(context->binary.const_pool + entry))->symbol = sentry;
}

/* Class Metadata */
void rxasmeclss(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type) {
    size_t entry = add_meta_entry(context, sizeof(meta_class_constant), META_CLASS);
    size_t s_sym, s_opt, s_typ;

    s_sym = add_string_to_pool(context, symbol, (char*)symbol->token_value.string);
    s_opt = add_string_to_pool(context, option, (char*)option->token_value.string);
    s_typ = add_string_to_pool(context, type, (char*)type->token_value.string);

    /* Recalculate pointer after potential pool growth */
    meta_class_constant *mentry = (meta_class_constant*)(context->binary.const_pool + entry);
    mentry->symbol = s_sym;
    mentry->option = s_opt;
    mentry->type = s_typ;
}

/* Attribute Metadata */
void rxasmeattr(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type, Assembler_Token *reg) {
    size_t entry = add_meta_entry(context, sizeof(meta_attr_constant), META_ATTR);
    size_t s_sym, s_opt, s_typ;

    s_sym = add_string_to_pool(context, symbol, (char*)symbol->token_value.string);
    s_opt = add_string_to_pool(context, option, (char*)option->token_value.string);
    s_typ = add_string_to_pool(context, type, (char*)type->token_value.string);

    /* Recalculate pointer after potential pool growth */
    meta_attr_constant *mentry = (meta_attr_constant*)(context->binary.const_pool + entry);
    mentry->symbol = s_sym;
    mentry->option = s_opt;
    mentry->type = s_typ;
    mentry->reg = (size_t)reg->token_value.integer;
}

/* Interface Metadata */
void rxasmeintf(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *type) {
    size_t entry = add_meta_entry(context, sizeof(meta_interface_constant), META_INTERFACE);
    size_t s_sym, s_opt, s_typ;

    s_sym = add_string_to_pool(context, symbol, (char*)symbol->token_value.string);
    s_opt = add_string_to_pool(context, option, (char*)option->token_value.string);
    s_typ = add_string_to_pool(context, type, (char*)type->token_value.string);

    meta_interface_constant *mentry = (meta_interface_constant*)(context->binary.const_pool + entry);
    mentry->symbol = s_sym;
    mentry->option = s_opt;
    mentry->type = s_typ;
}

/* Implements Metadata */
void rxasmeimpl(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *interface_symbol) {
    size_t entry = add_meta_entry(context, sizeof(meta_implements_constant), META_IMPLEMENTS);
    size_t s_sym, s_iface;

    s_sym = add_string_to_pool(context, symbol, (char*)symbol->token_value.string);
    s_iface = add_string_to_pool(context, interface_symbol, (char*)interface_symbol->token_value.string);

    meta_implements_constant *mentry = (meta_implements_constant*)(context->binary.const_pool + entry);
    mentry->symbol = s_sym;
    mentry->interface_symbol = s_iface;
}

/* Interface Member Metadata */
void rxasmememb(Assembler_Context *context, Assembler_Token *owner, Assembler_Token *kind, Assembler_Token *member, Assembler_Token *type, Assembler_Token *args) {
    size_t entry = add_meta_entry(context, sizeof(meta_member_constant), META_MEMBER);
    size_t s_owner, s_kind, s_member, s_type, s_args;

    s_owner = add_string_to_pool(context, owner, (char*)owner->token_value.string);
    s_kind = add_string_to_pool(context, kind, (char*)kind->token_value.string);
    s_member = add_string_to_pool(context, member, (char*)member->token_value.string);
    s_type = add_string_to_pool(context, type, (char*)type->token_value.string);
    s_args = add_string_to_pool(context, args, (char*)args->token_value.string);

    meta_member_constant *mentry = (meta_member_constant*)(context->binary.const_pool + entry);
    mentry->owner = s_owner;
    mentry->kind = s_kind;
    mentry->member = s_member;
    mentry->type = s_type;
    mentry->args = s_args;
}

/* Inline Metadata */
void rxasmeil(Assembler_Context *context, Assembler_Token *symbol, Assembler_Token *option, Assembler_Token *payload) {
    size_t entry;
    size_t s_sym;
    size_t s_payload;
    meta_inline_constant *mentry;
    const char *option_text;

    option_text = (const char *)option->token_value.string;
    if (strcmp(option_text, ".task1") == 0 ||
        strcmp(option_text, ".task2") == 0 ||
        strcmp(option_text, ".task3") == 0) {
        meta_task_target_constant *target;
        size_t binding;

        if (payload->token_type != HEX) {
            rxaserat(context, payload, "Task-target metadata requires an 80-byte hex binding placeholder");
            return;
        }
        if (strlen((const char *)payload->token_value.string) !=
            2u + RX_GRAPH_TASK_BINDING_SIZE * 2u) {
            rxaserat(context, payload, "Task-target metadata requires an 80-byte hex binding placeholder");
            return;
        }
        binding = add_binary_to_pool(
                context, (char *)payload->token_value.string);
        entry = add_meta_entry(
                context, sizeof(meta_task_target_constant), META_TASK_TARGET);
        s_sym = add_string_to_pool(
                context, symbol, (char *)symbol->token_value.string);
        target = (meta_task_target_constant *)(context->binary.const_pool + entry);
        target->symbol = s_sym;
        target->binding = binding;
        target->kind = (uint32_t)(option_text[5] - '0');
        return;
    }

    if (strcmp(option_text, ".inline") != 0) {
        rxaserat(context, option, "Expecting .inline or .task1/.task2/.task3 metadata option");
        return;
    }
    if (payload->token_type != STRING) {
        rxaserat(context, payload, "Inline metadata requires a string payload");
        return;
    }

    entry = add_meta_entry(context, sizeof(meta_inline_constant), META_INLINE);
    s_sym = add_string_to_pool(context, symbol, (char*)symbol->token_value.string);
    s_payload = add_string_to_pool(context, payload, (char*)payload->token_value.string);

    mentry = (meta_inline_constant*)(context->binary.const_pool + entry);
    mentry->symbol = s_sym;
    mentry->payload = s_payload;
}
