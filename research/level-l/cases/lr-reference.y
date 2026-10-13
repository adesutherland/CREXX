/* Independent Bison control for the classic LR(1)-but-not-LALR(1) grammar. */
%define api.value.type {int}
%code {
#include <stdio.h>
static const unsigned char *source;
static int at, selected;
int yylex(void);
void yyerror(const char *message);
}
%start S
%%
S: 'a' A 'd' { selected = 1; }
 | 'b' B 'd' { selected = 2; }
 | 'a' B 'e' { selected = 3; }
 | 'b' A 'e' { selected = 4; };
A: 'c';
B: 'c';
%%
int yylex(void) { return source[at] ? source[at++] : 0; }
void yyerror(const char *message) { (void)message; }
int main(int argc, char **argv) {
  if (argc != 2) return 2;
  source = (const unsigned char *)argv[1];
  int result = yyparse();
  printf("input=%s accepted=%d selected=%d\n", argv[1], result == 0, result ? 0 : selected);
  return 0;
}
