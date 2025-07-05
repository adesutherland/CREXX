/* REXX Assembler Header */
#ifndef CREXX_RXSA_H
#define CREXX_RXSA_H

#include <stddef.h>
#include <stdio.h>
#include "platform.h"
#include "rxbin.h"

typedef struct Assembler_Token Assembler_Token;
typedef struct Assembler_Error Assembler_Error;
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
    Assembler_Token *instrToken;
    Assembler_Token *operand1Token;
    Assembler_Token *operand2Token;
    Assembler_Token *operand3Token;
    Assembler_Token *operand4Token;
    Assembler_Token *operand5Token;
} instruction_queue;

typedef struct Assembler_Context {
    char *file_name;
    char *output_file_name;
    char *location;
    char *buff;
    char *buff_end;
    void *parser;
    char *top, *cursor, *marker, *ctxmarker, *linestart;
    int optimise;
    int line;
    int token_counter;
    Assembler_Token* token_head;
    Assembler_Token* token_tail;
    Assembler_Error* error_tail;
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
    struct avl_tree_node *decimal_constants_tree;
    struct avl_tree_node *binary_constants_tree;
    struct avl_tree_node *proc_constants_tree;
    struct avl_tree_node *label_constants_tree;
    struct avl_tree_node *extern_constants_tree;
    char *extern_regs;
    instruction_queue *optimiser_queue;
    size_t optimiser_queue_items;
    int optimiser_counter;
    FILE *traceFile;
    int debug_mode;
} Assembler_Context;

/* Assembler_Token Functions */
struct Assembler_Token {
    int token_type;
    int token_number;
    int token_subtype;
    Assembler_Token *token_next;
    Assembler_Token *token_prev;
    size_t line, column, length;
    char* token_source;
    char optimised;
    union {
        rxinteger integer;
        unsigned char string[1]; // Stores the string, binary (as hex) or decimal (as a string)
        unsigned char character;
        double real;
        void *pointer;
    } token_value;
};

/* Token Factory */
Assembler_Token* rxast_f(Assembler_Context* context, int type);

/* Free Token */
void rxasf_t(Assembler_Context* context);

/* Print Token */
void rxasp_t(Assembler_Token* token);

/* Get Assembler_Token Type Name */
const char* rxas_tpn(int type);

/* Change an ID token's ID
 * MUST be a ID Assembler_Token
 * Returns a new token which is a copy of the input token but with the new_id */
Assembler_Token* rxas_tid(Assembler_Context* context, Assembler_Token *from_token, char* new_id);

/* Scanner */
int rx_scan(Assembler_Context* s, char *buff_end);

/* Interface the Lemon parser */
void *RxasmAlloc();
void Rxasm();
void RxasmFree();
void RxasmTrace(FILE *stream, char *zPrefix);

/* Error Functions */
#define MAX_ERROR_LENGTH 250
struct Assembler_Error {
    int line;
    int column;
    int severity;
    char message[MAX_ERROR_LENGTH];
    Assembler_Error *prev_error;
    Assembler_Error *next_error;
};

/* Error Factory */
Assembler_Error* rxaserrf(Assembler_Context* context, int line, int column,
                          int severity, char *message);
/* Print Error */
void rxasperr(Assembler_Context* context);

/* Free Error */
void rxasfrer(Assembler_Context* context);

/* Add Error after a token */
void rxaseaft(Assembler_Context* context, Assembler_Token* after_token, char* message);

/* Add Error at a token */
void rxaserat(Assembler_Context* context, Assembler_Token* after_token, char* message);

/* Main Assembler Function
 * Returns: 0 on success, -1 on an error
 * scanner should be clear with the following set:
 *   scanner->debug_mode
 *   scanner->traceFile
 *   scanner->optimise
 *   scanner->file_name
 *   scanner->output_file_name
 *   scanner->location
 */
int rxasmble(Assembler_Context *scanner);

/* Init Assembler Context from Buffer
 * Returns: 0 on success, -1 on an error
 * scanner should be clear with the following set:
 *   scanner->debug_mode
 *   scanner->traceFile
 *   scanner->optimise
 *   scanner->file_name (nominal)
 *   scanner->output_file_name
 *   scanner->location
 *   scanner->buff
 *   scanner->buff_end
 */
int rxasinbf(Assembler_Context *scanner);

/* Init Assembler Context from File
 * Returns: 0 on success, -1 on an error
 * scanner should be clear with the following set:
 *   scanner->debug_mode
 *   scanner->traceFile
 *   scanner->optimise
 *   scanner->file_name (nominal)
 *   scanner->output_file_name
 *   scanner->location
 *
 *  file_name_includes_type_extension should be set to true if the filename in the Assmebler_Context
 *  already has the ".rxas" extention (otherwise it is added)
*/
int rxasinfl(Assembler_Context *scanner, int file_name_includes_type_extension);

/* Parse & Process Assembler Function
 * Returns: scanner->severity
 * scanner has to have been initiated
 */
int rxaspars(Assembler_Context *scanner);

/* Output rxbin Function
 * Returns: 0 on success, otherwise an error
 * scanner has to have been initiated and parsed and scanner->output_file_name set
 */
int rxasoutf(Assembler_Context *scanner);

/* Clear and Free Assembler Context */
void rxasclrc(Assembler_Context *scanner);

/* Convert FLOAT tokens to a DECIMAL tokens as defined by the instruction types */
void promote_floats_to_decimals(Assembler_Token *instrToken,
                                Assembler_Token *operand1Token, Assembler_Token *operand2Token, Assembler_Token *operand3Token);

#endif //CREXX_RXSA_H