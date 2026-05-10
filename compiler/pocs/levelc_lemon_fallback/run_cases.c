#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fallback_poc_shared.h"

#ifndef POC_HEADER
#error "Define POC_HEADER to the generated Lemon header"
#endif

#ifndef PARSE_PREFIX
#error "Define PARSE_PREFIX to the generated parser prefix"
#endif

#define POC_CAT2(a, b) a##b
#define POC_CAT(a, b) POC_CAT2(a, b)
#define POC_PARSE(name) POC_CAT(PARSE_PREFIX, name)

#include POC_HEADER

void *POC_PARSE(Alloc)(void *(*mallocProc)(size_t));
void POC_PARSE(Free)(void *parser, void (*freeProc)(void *));
void POC_PARSE()(void *parser, int token_code, PocToken token, PocContext *ctx);

typedef struct PocCase {
    const char *name;
    const char *input;
    int expect_accept;
} PocCase;

void poc_event(PocContext *ctx, const char *name) {
    ctx->events++;
    printf("    event %s\n", name);
}

void poc_event2(PocContext *ctx, const char *name, const char *a, const char *b) {
    ctx->events++;
    printf("    event %s(%s,%s)\n", name, a, b);
}

void poc_syntax_error(PocContext *ctx, PocToken token) {
    ctx->syntax_errors++;
    printf("    syntax error at %s\n", token.text ? token.text : "<eos>");
}

void poc_parse_failure(PocContext *ctx) {
    ctx->parse_failures++;
    printf("    parse failure\n");
}

void poc_parse_accept(PocContext *ctx) {
    ctx->accepted = 1;
    printf("    parse accept\n");
}

static int token_code(const char *word) {
    if (strcmp(word, "IF") == 0) return IF;
    if (strcmp(word, "THEN") == 0) return THEN;
    if (strcmp(word, "SAY") == 0) return SAY;
    if (strcmp(word, "DO") == 0) return DO;
    if (strcmp(word, "=") == 0) return EQUAL;
    if (strcmp(word, ";") == 0) return EOC;
    return ID;
}

static const char *stable_text(const char *word) {
    if (strcmp(word, "IF") == 0) return "IF";
    if (strcmp(word, "THEN") == 0) return "THEN";
    if (strcmp(word, "SAY") == 0) return "SAY";
    if (strcmp(word, "DO") == 0) return "DO";
    if (strcmp(word, "=") == 0) return "=";
    if (strcmp(word, ";") == 0) return ";";
    if (strcmp(word, "A") == 0) return "A";
    return "ID";
}

static void send_token(void *parser, int code, const char *text, PocContext *ctx) {
    PocToken token = { text };
    POC_PARSE()(parser, code, token, ctx);
}

static int run_case(const PocCase *poc_case) {
    PocContext ctx = { poc_case->name, 0, 0, 0, 0 };
    void *parser = POC_PARSE(Alloc)(malloc);
    const char *cursor = poc_case->input;
    char word[64];

    printf("case %s: %s\n", poc_case->name, poc_case->input);
    while (*cursor) {
        size_t len = 0;
        while (*cursor == ' ' || *cursor == '\t' || *cursor == '\r' || *cursor == '\n') {
            cursor++;
        }
        if (*cursor == '\0') break;
        while (cursor[len] && cursor[len] != ' ' && cursor[len] != '\t' &&
               cursor[len] != '\r' && cursor[len] != '\n') {
            if (len + 1 >= sizeof(word)) {
                fprintf(stderr, "token too long in case %s\n", poc_case->name);
                exit(2);
            }
            word[len] = cursor[len];
            len++;
        }
        word[len] = '\0';
        cursor += len;
        send_token(parser, token_code(word), stable_text(word), &ctx);
    }
    send_token(parser, EOS, "<eos>", &ctx);
    send_token(parser, 0, "<end>", &ctx);
    POC_PARSE(Free)(parser, free);

    int clean_accept = ctx.accepted && ctx.syntax_errors == 0 && ctx.parse_failures == 0;
    printf("    result accepted=%d syntax_errors=%d parse_failures=%d events=%d clean_accept=%d expected=%d\n",
           ctx.accepted, ctx.syntax_errors, ctx.parse_failures, ctx.events,
           clean_accept, poc_case->expect_accept);
    return clean_accept == poc_case->expect_accept ? 0 : 1;
}

int main(void) {
    const PocCase cases[] = {
        {
            "keyword_expression_fallback",
            "SAY IF ;",
            1,
        },
        {
            "shifted_if_cannot_backtrack",
            "IF = A ;",
            0,
        },
        {
            "shifted_say_cannot_backtrack",
            "SAY = A ;",
            0,
        },
        {
            "normal_if_keyword_expr",
            "IF A THEN SAY IF ;",
            1,
        },
        {
            "then_as_condition_variable",
            "IF THEN THEN SAY A ;",
            EXPECT_THEN_CONDITION_ACCEPT,
        },
        {
            "do_keyword_expression_fallback",
            "SAY DO ;",
            1,
        },
    };
    int failures = 0;
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        failures += run_case(&cases[i]);
    }

    if (failures) {
        printf("POC FAILURES: %d\n", failures);
        return 1;
    }
    printf("POC OK\n");
    return 0;
}
