/* Independent Bison GLR SET OF TREES oracle for short inputs only.
 * Additive %merge is not a trustworthy distinct-tree count (retained separately).
 */
%glr-parser
%expect 1
%define api.value.type {struct Trees *}
%code requires { struct Trees; }
%code {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct Trees { size_t n; char **tree; struct Trees *next; };
static const unsigned char *source;
static int at;
static struct Trees *total, *allocated;
static struct Trees *sum(YYSTYPE left, YYSTYPE right);
static struct Trees *leaf(void);
static struct Trees *product(struct Trees *left, struct Trees *right);
int yylex(void);
void yyerror(const char *message);
}
%start root
%%
root: S { total = $1; };
S: S S %merge <sum> { $$ = product($1, $2); }
 | 'a' %merge <sum> { $$ = leaf(); };
%%
static struct Trees *create(void) {
  struct Trees *r = calloc(1, sizeof *r);
  if (!r) abort();
  r->next = allocated; allocated = r; return r;
}
static void add(struct Trees *r, const char *tree) {
  for (size_t i = 0; i < r->n; ++i) if (!strcmp(r->tree[i], tree)) return;
  r->tree = realloc(r->tree, (r->n + 1) * sizeof *r->tree);
  if (!r->tree) abort();
  r->tree[r->n++] = strdup(tree);
}
static struct Trees *leaf(void) { struct Trees *r = create(); add(r, "a"); return r; }
static struct Trees *sum(YYSTYPE left, YYSTYPE right) {
  struct Trees *r = create();
  for (size_t i = 0; i < left->n; ++i) add(r, left->tree[i]);
  for (size_t i = 0; i < right->n; ++i) add(r, right->tree[i]);
  return r;
}
static struct Trees *product(struct Trees *left, struct Trees *right) {
  struct Trees *r = create();
  for (size_t i = 0; i < left->n; ++i) for (size_t j = 0; j < right->n; ++j) {
    size_t bytes = strlen(left->tree[i]) + strlen(right->tree[j]) + 3;
    char *s = malloc(bytes); if (!s) abort();
    snprintf(s, bytes, "(%s%s)", left->tree[i], right->tree[j]);
    add(r, s); free(s);
  }
  return r;
}
int yylex(void) { return source[at] ? source[at++] : 0; }
void yyerror(const char *message) { (void)message; }
int main(int argc, char **argv) {
  if (argc != 2) return 2;
  source = (const unsigned char *)argv[1];
  int result = yyparse();
  printf("input=%s accepted=%d derivations=%zu\n", argv[1], result == 0, result ? (size_t)0 : total->n);
  while (allocated) {
    struct Trees *next = allocated->next;
    for (size_t i = 0; i < allocated->n; ++i) free(allocated->tree[i]);
    free(allocated->tree); free(allocated); allocated = next;
  }
  return 0;
}
