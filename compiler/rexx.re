#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "rexxgrmr.h"
#include "compiler.h"

#define   YYCTYPE     char
#define   YYCURSOR    s->cursor
#define   YYMARKER    s->marker
#define   YYCTXMARKER s->ctxmarker

/* functions to interface the lemon parser */
void *ParseAlloc();
void Parse();
void ParseFree();
void ParseTrace(FILE *stream, char *zPrefix);

int scan(Scanner* s, char *buff_end) {
    int depth;

    regular:
    if (s->cursor >= buff_end) {
        return TK_EOF;
    }
    s->top = s->cursor;

/*!re2c
  re2c:yyfill:enable = 0;

  whitespace = [ \t\v\f]+;
  digit = [0-9];
  letter = [a-zA-Z];
  hex = [a-fA-F0-9];
  int_des = [uUlL]*;
  all = [\000-\377];
  eof = [\000];
  any = all\eof;
  symchr = letter|digit|[.!?_];
  const	= (digit|[.])symchr*([eE][+-]?digit+)?;
  simple = (symchr\(digit|[.]))(symchr\[.])*;
  stem = simple [.];
  symbol = symchr*;
  sqstr = ['] ((any\['\n\r])|(['][']))* ['];
  dqstr = ["] ((any\["\n\r])|(["]["]))* ["];
  str = sqstr|dqstr;
  ob = [ \t]*;
  not = [\\~];
*/

/*!re2c
  "/*" {
    depth = 1;
    goto comment;
  }
  "|" ob "|" { return(TK_CONCAT); }
  "+" { return(TK_PLUS); }
  "-" { return(TK_MINUS); }
  "*" { return(TK_MULT); }
  "/" { return(TK_DIV); }
  "%" { return(TK_IDIV); }
  "/" ob "/" { return(TK_REMAIN); }
  "*" ob "*" { return(TK_POWER); }
  "=" { return(TK_EQUAL); }
  not ob "=" | "<" ob ">" | ">" ob "<" { return(TK_NOT_EQUAL); }
  ">" { return(TK_GT); }
  "<" { return(TK_LT); }
  ">" ob "=" | not ob "<" { return(TK_GE); }
  "<" ob "=" | not ob ">" { return(TK_LE); }
  "=" ob "=" { return(TK_EQUAL_EQUAL); }
  not ob "=" ob "=" { return(TK_NOT_EQUAL_EQUAL); }
  ">" ob ">" { return(TK_GT_STRICT); }
  "<" ob "<" { return(TK_LT_STRICT); }
  ">" ob ">" ob "=" | not ob "<" ob "<" { return(TK_GE_STRICT); }
  "<" ob "<" ob "=" | not ob ">" ob ">" { return(TK_LE_STRICT); }
  "&" { return(TK_AND); }
  "|" { return(TK_OR); }
  "&" ob "&" { return(TK_XOR); }
  not { return(TK_NOT); }
  "," { return(TK_COMMA); }
  "." { return(TK_STOP); }
  "(" { return(TK_BOPEN); }
  ")" { return(TK_BCLOSE); }
  ";" { return(TK_EOC); }
  'ADDRESS' { return(TK_ADDRESS); }
  'ARG' { return(TK_ARG); }
  'CALL' { return(TK_CALL); }
  'DO' { return(TK_DO); }
  'DROP' { return(TK_DROP); }
  'ELSE' { return(TK_ELSE); }
  'END' { return(TK_END); }
  'EXTERNAL' { return(TK_EXTERNAL); }
  'EXIT' { return(TK_EXIT); }
  'IF' { return(TK_IF); }
  'INTERPRET' { return(TK_INTERPRET); }
  'ITERATE' { return(TK_ITERATE); }
  'LEAVE' { return(TK_LEAVE); }
  'NOP' { return(TK_NOP); }
  'NUMERIC' { return(TK_NUMERIC); }
  'OPTIONS' { return(TK_OPTIONS); }
  'OTHERWISE' { return(TK_OTHERWISE); }
  'PARSE' { return(TK_PARSE); }
  'PROCEDURE' { return(TK_PROCEDURE); }
  'PULL' { return(TK_PULL); }
  'PUSH' { return(TK_PUSH); }
  'QUEUE' { return(TK_QUEUE); }
  'RETURN' { return(TK_RETURN); }
  'SAY' { return(TK_SAY); }
  'SELECT' { return(TK_SELECT); }
  'SIGNAL' { return(TK_SIGNAL); }
  'THEN' { return(TK_THEN); }
  'TRACE' { return(TK_TRACE); }
  'WHEN' { return(TK_WHEN); }
  'OFF' { return(TK_OFF); }
  'ON' { return(TK_ON); }
  'BY' { return(TK_BY); }
  'DIGITS' { return(TK_DIGITS); }
  'ENGINEERING' { return(TK_ENGINEERING); }
  'ERROR' { return(TK_ERROR); }
  'EXPOSE' { return(TK_EXPOSE); }
  'FAILURE' { return(TK_FAILURE); }
  'FOR' { return(TK_FOR); }
  'FOREVER' { return(TK_FOREVER); }
  'FORM' { return(TK_FORM); }
  'FUZZ' { return(TK_FUZZ); }
  'HALT' { return(TK_HALT); }
  'LINEIN' { return(TK_LINEIN); }
  'NAME' { return(TK_NAME); }
  'NOVALUE' { return(TK_NOVALUE); }
  'SCIENTIFIC' { return(TK_SCIENTIFIC); }
  'SOURCE' { return(TK_SOURCE); }
  'SYNTAX' { return(TK_SYNTAX); }
  'TO' { return(TK_TO); }
  'UNTIL' { return(TK_UNTIL); }
  'UPPER' { return(TK_UPPER); }
  'VALUE' { return(TK_VALUE); }
  'VAR' { return(TK_VAR); }
  'VERSION' { return(TK_VERSION); }
  'WHILE' { return(TK_WHILE); }
  'WITH' { return(TK_WITH); }
  const { return(TK_CONST); }
  simple { return(TK_SYMBOL); }
  stem { return(TK_SYMBOL_STEM); }
  symbol ob ":" { return(TK_LABEL); }
  symbol { return(TK_SYMBOL_COMPOUND); }
  str { return(TK_STRING); }
  str [bB] / (all\symchr) { return(TK_STRING); }
  str [xX] / (all\symchr) { return(TK_STRING); }
  eof { return(TK_EOF); }
  whitespace { goto regular; }
  "\r\n" {
     s->line++;
     s->linestart = s->cursor+2;
     return(TK_EOL);
  }
  "\n" {
     s->line++;
     s->linestart = s->cursor+1;
     return(TK_EOL);
  }

  any { printf("unexpected character: %c\n", *s->cursor); return(-1); }
*/

    comment:
/*!re2c
  "*/" {
    if(--depth == 0) goto regular;
    else goto comment;
  }
  "\n" {
    s->line++;
    s->linestart = s->cursor+1;
    goto comment;
  }
  "\r\n" {
    s->line++;
    s->linestart = s->cursor+2;
    goto comment;
  }
  "/*" {
    ++depth;
    goto comment;
  }
  eof { printf("EOF before comment closed (comment depth %d): %c\n", depth); return(-1); }
  any { goto comment; }
*/
}

int main(int argc, char *argv[]) {

    FILE *fp, *traceFile;
    char *buff, *buff_end;
    size_t bytes;
    int token_type;
    Token *token;
    Scanner scanner;
    void *parser;

    /* Open input file */
    if (argc==2)   fp = fopen(argv[1], "r");
    else fp = fopen("test.rexx", "r");
    if(fp == NULL) {
        fprintf(stderr, "Can't open input file\n");
        exit(-1);
    }

    /* Open trace file */
    traceFile = fopen("trace.out", "w");
    if(traceFile == NULL) {
        fprintf(stderr, "Can't open trace file\n");
        exit(-1);
    }

    /* Get file size */
    fseek(fp, 0, SEEK_END);
    bytes = ftell(fp);
    rewind(fp);

    /* Allocate buffer and read */
    buff = (char*) malloc(bytes * sizeof(char));
    bytes = fread(buff, 1, bytes, fp);
    if (!bytes) {
        fprintf(stderr, "Error reading input file\n");
        exit(-1);
    }

    /* Initialize scanner */
    scanner.top = buff;
    scanner.cursor = buff;
    scanner.linestart = buff;
    scanner.line = 0;
    scanner.token_head = 0;
    scanner.token_tail = 0;
    scanner.token_counter = 0;
    scanner.ast = 0;

    /* Pointer to the end of the buffer */
    buff_end = (char*) (((char*)buff) + bytes);

    /* Create parser and set up tracing */
    parser = ParseAlloc(malloc);
#ifndef NDEBUG
    ParseTrace(traceFile, "parser >> ");
#endif
    while((token_type = scan(&scanner, buff_end))) {
        // Skip Scanner Errors
        if (token_type < 0) continue;

        // Setup and parse token
        token = token_f(&scanner, token_type);
        Parse(parser, token_type, token, &scanner);

        // Execute Parse for the last time
        if(token_type == TK_EOF) {
            Parse(parser, 0, NULL, &scanner);
            break;
        }
    }

    /* Deallocate parser */
    ParseFree(parser, free);

    if (scanner.ast) {
        prnt_ast(scanner.ast);
        printf("\n");
    }

    if (scanner.ast) {
        int counter = 0;
        printf("digraph REXX {\n");
        pdot_ast(scanner.ast, -1, &counter);
        printf("\n}\n");
    }

    /* Deallocate AST */
    if (scanner.ast) free_ast(scanner.ast);

    /* Deallocate Tokens */
    free_tok(&scanner);

    /* Close files and deallocate */
    fclose(fp);
    fclose(traceFile);
    free(buff);
    return(0);
}

/* Token Factory */
Token* token_f(Scanner* context, int type) {
    Token* token = malloc(sizeof(Token));
    token->token_type = type;

    /* Link it up */
    if (context->token_tail) {
        token->token_next = 0;
        token->token_prev = context->token_tail;
        context->token_tail->token_next = token;
        context->token_tail = token;
    }
    else {
        context->token_head = token;
        context->token_tail = token;
        token->token_next = 0;
        token->token_prev = 0;
    }
    token->token_number = ++(context->token_counter);
    token->token_subtype = 0; /* TODO */
    token->length = context->cursor - context->top;
    token->line = context->line;
    token->column = context->top - context->linestart + 1;
    token->token_string = context->top;

    return token;
}

void prnt_tok(Token* token) {
/*
    printf("%d.%d %s \"%.*s\"",token->line+1,token->column+1,
           token_type_name(token->token_type),token->length,token->token_string);
*/
    printf("%s \"%.*s\"",
           token_type_name(token->token_type),token->length,token->token_string);

}

void free_tok(Scanner* context) {
    Token *t = context->token_head;
    Token *n;
    while (t) {
        n = t->token_next;
        free(t);
        t = n;
    }
}

/* ASTNode Factory */
ASTNode* ast_f(Token *token) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->parent = 0;
    node->child = 0;
    node->sibling = 0;
    node->token = token;
    node->node_type = token->token_type;
    return node;
}

ASTNode* ast_ft(int type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->parent = 0;
    node->child = 0;
    node->sibling = 0;
    node->token = 0;
    node->node_type = type;
    return node;

}

void prnt_ast(ASTNode* node) {
    printf("[");
    if (node->token) prnt_tok(node->token);
    else printf("%s", token_type_name(node->node_type));
    if (node->child) {
        printf(" (");
        prnt_ast(node->child);
        printf(")");
    }
    printf("]");
    if (node->sibling) {
        printf(" ");
        prnt_ast(node->sibling);
    }
}

ASTNode* add_ast(ASTNode* parent, ASTNode* child) {
    ASTNode* s = parent->child;
    if (s) {
        while (s->sibling) s = s->sibling;
        s->sibling = child;
    }
    else parent->child = child;
    child->parent = parent;
    return child;
}

void free_ast(ASTNode* node) {
    if (node->child) free_ast(node->child);
    if (node->sibling) free_ast(node->sibling);
    free(node);
}

void print_unescaped(char* ptr, int len) {
    int i;
    if (!ptr) return;
    for (i = 0; i < len; i++, ptr++) {
        switch (*ptr) {
            case '\0': printf("\\0");  break;
            case '\a': printf("\\a");  break;
            case '\b': printf("\\b");  break;
            case '\f': printf("\\f");  break;
            case '\n': printf("\\n");  break;
            case '\r': printf("\\r");  break;
            case '\t': printf("\\t");  break;
            case '\v': printf("\\v");  break;
            case '\\': printf("\\\\"); break;
            case '\?': printf("\\\?"); break;
            case '\'': printf("\\\'"); break;
            case '\"': printf("\\\""); break;
            default:   printf("%c",     *ptr);
        }
    }
}

void pdot_ast(ASTNode* node, int parent, int *counter) {
    int me = *counter;
    if (node->token) {
        printf("n%d[label=\"%s\\n", *counter,
               token_type_name(node->token->token_type));

        print_unescaped(node->token->token_string, node->token->length);

        printf("\"]\n");
    }
    else printf("n%d[label=\"%s\"]\n", *counter,
                token_type_name(node->node_type));

    if (node->child) {
        (*counter)++;
        printf("n%d -> n%d\n", me, *counter);
        pdot_ast(node->child, me, counter);
    }
    if (node->sibling) {
        (*counter)++;
        printf("n%d -> n%d\n", parent, *counter);
        pdot_ast(node->sibling, parent, counter);
    }
}
