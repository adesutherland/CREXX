/* REXX Assembler Header */
#ifndef CREXX_RXSA_H
#define CREXX_RXSA_H

#include <stddef.h>
#include <stdio.h>

#include "platform.h"

typedef struct Token Token;
typedef struct Error Error;
typedef struct bin_space bin_space;
struct avl_tree_node;

/* Keyhole Optimiser */
/* We are aiming for 20 instructions in the keyhole */
#define OPTIMISER_TARGET_MAX_QUEUE_SIZE 20
/* We add 40 slots for any instruction growth caused by rules */
#define OPTIMISER_QUEUE_EXTRA_BUFFER_SIZE 40

enum queue_item_type {EMPTY, ASM_LABEL, OP_CODE, SRC_FILE, SRC_LINE, FUNC_META, REG_META, CONST_META, CLEAR_META};

/* Keyhole Queue Item  */
typedef struct instruction_queue {
    enum queue_item_type instrType;
    Token *instrToken;
    Token *operand1Token;
    Token *operand2Token;
    Token *operand3Token;
    Token *operand4Token;
    Token *operand5Token;
} instruction_queue;

/* cREXX Instruction Coding */
#pragma pack(push,4)
typedef struct instruction_coding {
    int opcode;
    int no_ops;
} instruction_coding;
#pragma pack(pop)

/* Single cREXX Binary Code Entry */
#pragma pack(push,4)
typedef union bin_code {
    instruction_coding instruction;
    void* impl_address;
    double fconst;
    rxinteger iconst;
    char cconst;
    size_t index;
} bin_code;
#pragma pack(pop)

/* cREXX Binary Program */
#pragma pack(push,4)
struct bin_space {
    int globals;
    size_t inst_size;
    size_t const_size;
    struct module *module;
    bin_code *binary;
    unsigned char *const_pool;
};
#pragma pack(pop)

enum const_pool_type {
    STRING_CONST, PROC_CONST, EXPOSE_REG_CONST, EXPOSE_PROC_CONST,
    META_SRC, META_FILE, META_FUNC, META_REG, META_CONST, META_CLEAR
};

/* cREXX chameleon entry in the constant pool
 * A poor C users abstract class!
 * */
typedef struct chameleon_constant {
    size_t size_in_pool; /* including any padding for alignment */
    enum const_pool_type type;
} chameleon_constant;

/* cREXX String entry in the constant pool */
typedef struct string_constant {
    chameleon_constant base;
    size_t string_len;
#ifndef NUTF8
    size_t string_chars;
#endif
    char string[1]; /* Must be last member */
} string_constant;

typedef struct stack_frame stack_frame;

/* cREXX Procedure entry in the constant pool */
typedef struct proc_constant {
    chameleon_constant base;
    int next;
    int locals;
    bin_space *binarySpace;
    stack_frame **frame_free_list;
    stack_frame *frame_free_list_head;
    size_t start;
    size_t exposed;
    char name[1]; /* Must be last member */
} proc_constant;

/* cREXX Exposed Register entry in the constant pool */
typedef struct expose_reg_constant {
    chameleon_constant base;
    int next;
    int global_reg;
    char index[1]; /* Must be last member */
} expose_reg_constant;

/* cREXX Exposed Procedure entry in the constant pool */
typedef struct expose_proc_constant {
    chameleon_constant base;
    int next;
    size_t procedure;
    unsigned char imported : 1;
    char index[1]; /* Must be last member */
} expose_proc_constant;

/* cREXX Generic meta entry to hold prev/next offsets */
typedef struct meta_entry {
    chameleon_constant base;
    int prev;
    int next;
    size_t address;
} meta_entry;

/* cREXX Meta Source entry in the constant pool */
typedef struct meta_src_constant {
    meta_entry base;
    size_t line;
    size_t column;
    size_t source;
} meta_src_constant;

/* cREXX Meta File entry in the constant pool */
typedef struct meta_file_constant {
    meta_entry base;
    size_t file;
} meta_file_constant;

typedef struct meta_func_constant {
    meta_entry base;
    size_t symbol;
    size_t option;
    size_t type;
    size_t func;
    size_t args;
    size_t inliner;
} meta_func_constant;

typedef struct meta_reg_constant {
    meta_entry base;
    size_t symbol;
    size_t option;
    size_t type;
    size_t reg;
} meta_reg_constant;

typedef struct meta_const_constant {
    meta_entry base;
    size_t symbol;
    size_t option;
    size_t type;
    size_t constant;
} meta_const_constant;

typedef struct meta_clear_constant {
    meta_entry base;
    size_t symbol;
} meta_clear_constant;

typedef struct Assembler_Context {
    char* file_name;
    char *top, *cursor, *marker, *ctxmarker, *linestart;
    int optimise;
    int line;
    int token_counter;
    Token* token_head;
    Token* token_tail;
    Error* error_tail;
    int severity;
    bin_space binary;
    size_t inst_buffer_size;
    size_t const_buffer_size;
    int current_locals;
    int proc_head; /* int because we need -1 */
    int proc_tail;
    int expose_head;
    int expose_tail;
    int meta_head;
    int meta_tail;
    struct avl_tree_node *string_constants_tree;
    struct avl_tree_node *proc_constants_tree;
    struct avl_tree_node *label_constants_tree;
    struct avl_tree_node *extern_constants_tree;
    char *extern_regs;
    instruction_queue *optimiser_queue;
    size_t optimiser_queue_items;
    int optimiser_counter;
} Assembler_Context;

/* Token Functions */
struct Token {
    int token_type;
    int token_number;
    int token_subtype;
    Token *token_next;
    Token *token_prev;
    size_t line, column, length;
    char* token_source;
    char optimised;
    union {
        rxinteger integer;
        unsigned char string[1];
        unsigned char character;
        double real;
        void *pointer;
    } token_value;
};

Token* token_f(Assembler_Context* context, int type);
void free_tok(Assembler_Context* context);
void prnt_tok(Token* token);
const char* tk_tp_nm(int type); /* Get Token Type Name */
/* Change an ID token's ID
 * MUST be a ID Token
 * Returns a new token which is a copy of the input token but with the new_id */
Token* token_id(Assembler_Context* context, Token *from_token, char* new_id);

/* Scanner */
int scan(Assembler_Context* s, char *buff_end);

/* Interface the Lemon parser */
void *ParseAlloc();
void Parse();
void ParseFree();
void ParseTrace(FILE *stream, char *zPrefix);

/* Error Functions */
#define MAX_ERROR_LENGTH 250
struct Error {
    int line;
    int column;
    int severity;
    char message[MAX_ERROR_LENGTH];
    Error *prev_error;
    Error *next_error;
};
Error* error_f(Assembler_Context* context, int line, int column,
               int severity, char *message);
void prnt_err(Assembler_Context* context);
void free_err(Assembler_Context* context);
void err_aftr(Assembler_Context* context, Token* after_token, char* message);
void err_at(Assembler_Context* context, Token* after_token, char* message);

#endif //CREXX_RXSA_H