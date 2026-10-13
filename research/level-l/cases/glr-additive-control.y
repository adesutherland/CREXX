/* Independent Bison GLR derivation-count control; no tree enumeration. */
%glr-parser
%expect 1
%define api.value.type {unsigned long}
%code {
#include <stdio.h>
static const unsigned char *source;
static int at;
static unsigned long total;
static unsigned long sum(YYSTYPE left, YYSTYPE right);
int yylex(void);
void yyerror(const char *message);
}
%start root
%%
root: S { total = $1; };
S: S S %merge <sum> { $$ = $1 * $2; }
 | 'a' %merge <sum> { $$ = 1; };
%%
static unsigned long sum(YYSTYPE left, YYSTYPE right) { return left + right; }
int yylex(void) { return source[at] ? source[at++] : 0; }
void yyerror(const char *message) { (void)message; }
int main(int argc, char **argv) {
  if (argc != 2) return 2;
  source = (const unsigned char *)argv[1];
  int result = yyparse();
  printf("input=%s accepted=%d derivations=%lu\n", argv[1], result == 0, result ? 0 : total);
  return 0;
}
