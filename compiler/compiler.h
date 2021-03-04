/* cREXX Phase 0 (PoC) Compiler */
/* (c) Adrian Sutherland 2021   */

/* functions to interface the lemon parser */
void *ParseAlloc();
void Parse();
void ParseFree();
void ParseTrace(FILE *stream, char *zPrefix);

typedef struct ASTNode ASTNode;
typedef struct Token Token;

/* Scanner */
typedef struct Scanner {
    char *top, *cursor, *marker, *ctxmarker, *linestart;
    int line;
    int token_counter;
    Token* token_head;
    Token* token_tail;
    ASTNode* ast;
    ASTNode* last_node;
} Scanner;

int scan(Scanner* s, char *buff_end);

typedef enum NodeType {
    ABS_POS=1, ADDRESS, ARG, ASSIGN, BY, CALL, CONST_SYMBOL,
    DO, ENVIRONMENT, ERROR, FOR, FUNCTION, IF, INSTRUCTIONS, ITERATE, LABEL, LEAVE,
    NUMBER, OP_ADD, OP_AND, OP_COMPARE, OP_CONCAT, OP_MULT, OP_OR, OP_POWER, OP_PREFIX,
    OP_SCONCAT, OPTIONS, PARSE, PATTERN, PROCEDURE, PROGRAM_FILE, PULL, REL_POS, REPEAT,
    RETURN, REXX, SAY, SIGN, STRING, TARGET, TEMPLATES, TO, TOKEN, UPPER, VAR_SYMBOL
} NodeType;


struct Token {
    int token_type;
    int token_number;
    int token_subtype;
    Token *token_next;
    Token *token_prev;
    size_t line, column, length;
    char* token_string;
};

struct ASTNode {
    NodeType node_type;
    Scanner* context;
    ASTNode *parent, *child, *sibling;
    Token *token;
    char *node_string;
    size_t length;
};

/* Token Functions */
Token* token_f(Scanner* context, int type);
void free_tok(Scanner* context);
void prnt_tok(Token* token);
const char* token_type_name(int type); /* Get Token Type Name */

/* AST Function */
ASTNode* ast_f(Scanner* context, NodeType type, Token *token);
/* ASTNode Factory - With node type*/
ASTNode *ast_ft(Scanner* context, NodeType type);
/* ASTNode Factory - With node type and string value */
ASTNode *ast_ftt(Scanner* context, NodeType type, char *string);
/* ASTNode Factory - Error Node */
ASTNode *ast_error(Scanner* context, char *error_string, Token *token);
const char *ast_nodetype(NodeType type);
void prnt_ast(ASTNode* node);
void pdot_ast(ASTNode* node, int parent, int *counter);
ASTNode* add_ast(ASTNode* parent, ASTNode* child); /* Returns child for chaining */
void free_ast(ASTNode* node);
