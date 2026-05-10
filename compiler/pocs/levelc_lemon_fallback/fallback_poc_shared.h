#ifndef LEVELC_LEMON_FALLBACK_POC_SHARED_H
#define LEVELC_LEMON_FALLBACK_POC_SHARED_H

typedef struct PocToken {
    const char *text;
} PocToken;

typedef struct PocContext {
    const char *case_name;
    int accepted;
    int syntax_errors;
    int parse_failures;
    int events;
} PocContext;

void poc_event(PocContext *ctx, const char *name);
void poc_event2(PocContext *ctx, const char *name, const char *a, const char *b);
void poc_syntax_error(PocContext *ctx, PocToken token);
void poc_parse_failure(PocContext *ctx);
void poc_parse_accept(PocContext *ctx);

#endif
