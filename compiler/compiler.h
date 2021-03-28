
typedef struct ASTNode ASTNode;
typedef struct Token Token;

typedef struct Assembler_Context {
    char *top, *cursor, *marker, *ctxmarker, *linestart;
    int line;
    int token_counter;
    Token* token_head;
    Token* token_tail;
    ASTNode* ast;
} Scanner;

struct Token {
    int token_type;
    int token_number;
    int token_subtype;
    Token *token_next;
    Token *token_prev;
    size_t line, column, length;
    char* token_source;
};

struct ASTNode {
    int node_type;
    ASTNode *parent, *child, *sibling;
    Token *token;
};

/* Token Functions */
Token* token_f(Assembler_Context* context, int type);
void free_tok(Assembler_Context* context);
void prnt_tok(Token* token);
const char* token_type_name(int type); /* Get Token Type Name */

/* AST Functions */
ASTNode* ast_f(Token *token);
ASTNode* ast_ft(int type);
void prnt_ast(ASTNode* node);
void pdot_ast(ASTNode* node, int parent, int *counter);
ASTNode* add_ast(ASTNode* parent, ASTNode* child); /* Returns child for chaining */
void free_ast(ASTNode* node);
