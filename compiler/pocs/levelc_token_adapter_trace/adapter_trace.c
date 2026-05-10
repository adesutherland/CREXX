#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKENS 256
#define MAX_TEXT 64
#define MAX_GROUPS 32

typedef enum RawKind {
    RAW_WORD,
    RAW_NUMBER,
    RAW_STRING,
    RAW_SEMI,
    RAW_COLON,
    RAW_EQUAL,
    RAW_COMMA,
    RAW_LPAREN,
    RAW_RPAREN,
    RAW_PLUS,
    RAW_MINUS,
    RAW_BACKSLASH,
    RAW_DOT,
    RAW_EOL,
    RAW_EOS
} RawKind;

typedef struct RawToken {
    RawKind kind;
    char text[MAX_TEXT];
    char upper[MAX_TEXT];
    int blank_before;
} RawToken;

typedef enum GroupKind {
    GROUP_DO,
    GROUP_SELECT
} GroupKind;

typedef enum ParseMode {
    PARSE_NONE,
    PARSE_HEADER,
    PARSE_VALUE_EXPR,
    PARSE_TEMPLATE
} ParseMode;

typedef enum AddressMode {
    ADDRESS_NONE,
    ADDRESS_TAIL
} AddressMode;

typedef struct Adapter {
    int clause_start;
    int if_condition;
    int when_condition;
    int do_specification;
    int operand_left;
    int after_end;
    int pending_else;
    ParseMode parse_mode;
    AddressMode address_mode;
    GroupKind group_stack[MAX_GROUPS];
    int group_count;
} Adapter;

typedef struct Fixture {
    const char *name;
    const char *source;
} Fixture;

static const char *raw_kind_name(RawKind kind) {
    switch (kind) {
        case RAW_WORD: return "WORD";
        case RAW_NUMBER: return "NUMBER";
        case RAW_STRING: return "STRING";
        case RAW_SEMI: return "SEMI";
        case RAW_COLON: return "COLON";
        case RAW_EQUAL: return "EQUAL";
        case RAW_COMMA: return "COMMA";
        case RAW_LPAREN: return "LPAREN";
        case RAW_RPAREN: return "RPAREN";
        case RAW_PLUS: return "PLUS";
        case RAW_MINUS: return "MINUS";
        case RAW_BACKSLASH: return "BACKSLASH";
        case RAW_DOT: return "DOT";
        case RAW_EOL: return "EOL";
        case RAW_EOS: return "EOS";
    }
    return "UNKNOWN";
}

static int is_word_start(int ch) {
    return isalpha((unsigned char)ch) || ch == '_' || ch == '!' || ch == '?';
}

static int is_word_part(int ch) {
    return isalnum((unsigned char)ch) || ch == '_' || ch == '!' || ch == '?' || ch == '.';
}

static void uppercase_copy(char *dst, const char *src) {
    size_t i;
    for (i = 0; src[i] && i + 1 < MAX_TEXT; i++) {
        dst[i] = (char)toupper((unsigned char)src[i]);
    }
    dst[i] = '\0';
}

static RawToken make_token(RawKind kind, const char *start, size_t len, int blank_before) {
    RawToken token;
    size_t copy_len = len < MAX_TEXT - 1 ? len : MAX_TEXT - 1;
    token.kind = kind;
    memcpy(token.text, start, copy_len);
    token.text[copy_len] = '\0';
    uppercase_copy(token.upper, token.text);
    token.blank_before = blank_before;
    return token;
}

static void push_token(RawToken *tokens, int *count, RawToken token) {
    if (*count >= MAX_TOKENS) {
        fprintf(stderr, "too many tokens\n");
        exit(2);
    }
    tokens[(*count)++] = token;
}

static int scan_source(const char *source, RawToken *tokens) {
    int count = 0;
    int blank_before = 0;
    size_t i = 0;

    while (source[i]) {
        unsigned char ch = (unsigned char)source[i];
        if (ch == ' ' || ch == '\t' || ch == '\r') {
            blank_before = 1;
            i++;
            continue;
        }
        if (ch == '\n') {
            push_token(tokens, &count, make_token(RAW_EOL, source + i, 1, blank_before));
            blank_before = 0;
            i++;
            continue;
        }
        if (is_word_start(ch)) {
            size_t start = i;
            while (source[i] && is_word_part((unsigned char)source[i])) i++;
            push_token(tokens, &count, make_token(RAW_WORD, source + start, i - start, blank_before));
            blank_before = 0;
            continue;
        }
        if (isdigit(ch)) {
            size_t start = i;
            while (isdigit((unsigned char)source[i])) i++;
            push_token(tokens, &count, make_token(RAW_NUMBER, source + start, i - start, blank_before));
            blank_before = 0;
            continue;
        }
        if (ch == '\'' || ch == '"') {
            unsigned char quote = ch;
            size_t start = i++;
            while (source[i] && (unsigned char)source[i] != quote) i++;
            if (source[i] == quote) i++;
            push_token(tokens, &count, make_token(RAW_STRING, source + start, i - start, blank_before));
            blank_before = 0;
            continue;
        }

        switch (ch) {
            case ';': push_token(tokens, &count, make_token(RAW_SEMI, source + i, 1, blank_before)); break;
            case ':': push_token(tokens, &count, make_token(RAW_COLON, source + i, 1, blank_before)); break;
            case '=': push_token(tokens, &count, make_token(RAW_EQUAL, source + i, 1, blank_before)); break;
            case ',': push_token(tokens, &count, make_token(RAW_COMMA, source + i, 1, blank_before)); break;
            case '(': push_token(tokens, &count, make_token(RAW_LPAREN, source + i, 1, blank_before)); break;
            case ')': push_token(tokens, &count, make_token(RAW_RPAREN, source + i, 1, blank_before)); break;
            case '+': push_token(tokens, &count, make_token(RAW_PLUS, source + i, 1, blank_before)); break;
            case '-': push_token(tokens, &count, make_token(RAW_MINUS, source + i, 1, blank_before)); break;
            case '\\': push_token(tokens, &count, make_token(RAW_BACKSLASH, source + i, 1, blank_before)); break;
            case '.': push_token(tokens, &count, make_token(RAW_DOT, source + i, 1, blank_before)); break;
            default:
                fprintf(stderr, "unsupported character '%c'\n", ch);
                exit(2);
        }
        blank_before = 0;
        i++;
    }

    push_token(tokens, &count, make_token(RAW_EOS, "<eos>", 5, blank_before));
    return count;
}

static int text_is(const RawToken *token, const char *text) {
    return strcmp(token->upper, text) == 0;
}

static int is_clause_keyword(const RawToken *token) {
    static const char *keywords[] = {
        "ADDRESS", "ARG", "CALL", "DO", "DROP", "EXIT", "IF", "INTERPRET",
        "ITERATE", "LEAVE", "NOP", "NUMERIC", "OPTIONS", "PARSE",
        "PROCEDURE", "PULL", "PUSH", "QUEUE", "RETURN", "SAY", "SELECT",
        "SIGNAL", "TRACE"
    };
    size_t i;
    for (i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (text_is(token, keywords[i])) return 1;
    }
    return 0;
}

static int is_do_modifier(const RawToken *token) {
    return text_is(token, "TO") || text_is(token, "BY") || text_is(token, "FOR") ||
           text_is(token, "WHILE") || text_is(token, "UNTIL") || text_is(token, "FOREVER");
}

static int is_parse_word(const RawToken *token) {
    return text_is(token, "UPPER") || text_is(token, "ARG") || text_is(token, "PULL") ||
           text_is(token, "SOURCE") || text_is(token, "LINEIN") || text_is(token, "VERSION") ||
           text_is(token, "VALUE") || text_is(token, "VAR") || text_is(token, "WITH");
}

static int is_address_word(const RawToken *token) {
    return text_is(token, "VALUE") || text_is(token, "WITH") || text_is(token, "INPUT") ||
           text_is(token, "OUTPUT") || text_is(token, "ERROR") || text_is(token, "STREAM") ||
           text_is(token, "STEM") || text_is(token, "NORMAL") || text_is(token, "APPEND") ||
           text_is(token, "REPLACE");
}

static int in_select(const Adapter *adapter) {
    return adapter->group_count > 0 && adapter->group_stack[adapter->group_count - 1] == GROUP_SELECT;
}

static void push_group(Adapter *adapter, GroupKind kind) {
    if (adapter->group_count >= MAX_GROUPS) return;
    adapter->group_stack[adapter->group_count++] = kind;
}

static void pop_group(Adapter *adapter) {
    if (adapter->group_count > 0) adapter->group_count--;
}

static void emit_synthetic(const char *token, const char *role, const char *note) {
    printf("    synthetic -> %-18s role=%-15s note=%s\n", token, role, note);
}

static void emit_raw(const RawToken *raw, const char *token, const char *role, const char *note) {
    printf("    %-6s %-12s blank=%d -> %-18s role=%-15s note=%s\n",
           raw_kind_name(raw->kind), raw->text, raw->blank_before, token, role, note);
}

static void maybe_concat(Adapter *adapter, const RawToken *raw, int is_keyword) {
    if (adapter->operand_left && !adapter->clause_start && !is_keyword &&
        raw->kind != RAW_EOS && raw->kind != RAW_SEMI && raw->kind != RAW_EOL &&
        raw->kind != RAW_RPAREN && raw->kind != RAW_COMMA) {
        emit_synthetic(raw->blank_before ? "TK_BLANK_CONCAT" : "TK_CONCAT",
                       "implicit_concat",
                       raw->blank_before ? "blank before operand" : "abutted operand");
        adapter->operand_left = 0;
    }
}

static void reset_clause(Adapter *adapter) {
    adapter->clause_start = 1;
    adapter->do_specification = 0;
    adapter->parse_mode = PARSE_NONE;
    adapter->address_mode = ADDRESS_NONE;
    adapter->operand_left = 0;
    adapter->after_end = 0;
}

static void emit_eoc(Adapter *adapter, const RawToken *raw, const char *note) {
    emit_raw(raw, "TK_EOC", "eoc", note);
    reset_clause(adapter);
}

static void emit_word(Adapter *adapter, const RawToken *tokens, int *index) {
    const RawToken *raw = &tokens[*index];
    const RawToken *next = &tokens[*index + 1];

    if (adapter->clause_start && next->kind == RAW_COLON) {
        emit_raw(raw, "TK_LABEL", "label", "label lookahead");
        emit_raw(next, "TK_COLON", "punctuation", "consumed by label");
        emit_synthetic("TK_EOC", "synthetic_eoc", "after label");
        *index += 1;
        reset_clause(adapter);
        return;
    }

    if ((adapter->clause_start || adapter->do_specification) && next->kind == RAW_EQUAL) {
        maybe_concat(adapter, raw, 0);
        emit_raw(raw, "TK_VAR_SYMBOL", "identifier", adapter->do_specification ? "do control assignment lhs" : "assignment lhs");
        adapter->clause_start = 0;
        adapter->operand_left = 0;
        return;
    }

    if ((adapter->if_condition || adapter->when_condition) && text_is(raw, "THEN")) {
        int was_if_condition = adapter->if_condition;
        emit_synthetic("TK_EOC", "synthetic_eoc", "before THEN");
        emit_raw(raw, "TK_THEN", "keyword", adapter->if_condition ? "terminates IF condition" : "terminates WHEN condition");
        emit_synthetic("TK_EOC", "synthetic_eoc", "after THEN");
        adapter->if_condition = 0;
        adapter->when_condition = 0;
        if (was_if_condition) adapter->pending_else++;
        adapter->clause_start = 1;
        adapter->operand_left = 0;
        return;
    }

    if (text_is(raw, "ELSE") && adapter->pending_else > 0) {
        emit_raw(raw, "TK_ELSE", "keyword", "nearest pending IF else");
        emit_synthetic("TK_EOC", "synthetic_eoc", "after ELSE");
        adapter->pending_else--;
        adapter->clause_start = 1;
        adapter->operand_left = 0;
        return;
    }

    if (adapter->after_end) {
        emit_raw(raw, "TK_VAR_SYMBOL", "end_name", "optional END control name");
        adapter->after_end = 0;
        adapter->clause_start = 0;
        adapter->operand_left = 0;
        return;
    }

    if (adapter->parse_mode == PARSE_TEMPLATE) {
        if (raw->kind == RAW_WORD) {
            emit_raw(raw, "TK_VAR_SYMBOL", "parse_target", "parse template target");
            adapter->operand_left = 0;
            adapter->clause_start = 0;
            return;
        }
    }

    if (adapter->parse_mode == PARSE_VALUE_EXPR && text_is(raw, "WITH")) {
        emit_raw(raw, "TK_WITH", "keyword", "PARSE VALUE expression/template boundary");
        adapter->parse_mode = PARSE_TEMPLATE;
        adapter->operand_left = 0;
        adapter->clause_start = 0;
        return;
    }

    if (adapter->parse_mode == PARSE_HEADER && is_parse_word(raw)) {
        char token[MAX_TEXT + 4];
        snprintf(token, sizeof(token), "TK_%s", raw->upper);
        emit_raw(raw, token, "keyword", "PARSE subkeyword");
        adapter->parse_mode = text_is(raw, "VALUE") ? PARSE_VALUE_EXPR : PARSE_TEMPLATE;
        adapter->clause_start = 0;
        adapter->operand_left = 0;
        return;
    }

    if (adapter->address_mode == ADDRESS_TAIL && is_address_word(raw)) {
        char token[MAX_TEXT + 4];
        snprintf(token, sizeof(token), "TK_%s", raw->upper);
        emit_raw(raw, token, "keyword", "ADDRESS tail keyword");
        adapter->clause_start = 0;
        adapter->operand_left = 0;
        return;
    }

    if (adapter->do_specification && is_do_modifier(raw)) {
        char token[MAX_TEXT + 4];
        snprintf(token, sizeof(token), "TK_%s", raw->upper);
        emit_raw(raw, token, "keyword", "DO specification keyword");
        adapter->clause_start = 0;
        adapter->operand_left = 0;
        return;
    }

    if (adapter->clause_start && in_select(adapter) && text_is(raw, "WHEN")) {
        emit_raw(raw, "TK_WHEN", "keyword", "SELECT body WHEN");
        adapter->when_condition = 1;
        adapter->clause_start = 0;
        adapter->operand_left = 0;
        return;
    }

    if (adapter->clause_start && in_select(adapter) && text_is(raw, "OTHERWISE")) {
        emit_raw(raw, "TK_OTHERWISE", "keyword", "SELECT body OTHERWISE");
        emit_synthetic("TK_EOC", "synthetic_eoc", "after OTHERWISE");
        adapter->clause_start = 1;
        adapter->operand_left = 0;
        return;
    }

    if (adapter->clause_start &&
        (text_is(raw, "THEN") || text_is(raw, "ELSE") ||
         text_is(raw, "WHEN") || text_is(raw, "OTHERWISE"))) {
        char token[MAX_TEXT + 4];
        snprintf(token, sizeof(token), "TK_%s", raw->upper);
        emit_raw(raw, token, "keyword", "unexpected structural keyword");
        if (text_is(raw, "THEN") || text_is(raw, "ELSE") || text_is(raw, "OTHERWISE")) {
            emit_synthetic("TK_EOC", "synthetic_eoc", "after structural keyword");
        }
        adapter->clause_start = 1;
        adapter->operand_left = 0;
        return;
    }

    if (adapter->clause_start && text_is(raw, "END")) {
        emit_raw(raw, "TK_END", "keyword", "group terminator");
        pop_group(adapter);
        adapter->after_end = 1;
        adapter->clause_start = 0;
        adapter->operand_left = 0;
        return;
    }

    if (adapter->clause_start && is_clause_keyword(raw)) {
        char token[MAX_TEXT + 4];
        snprintf(token, sizeof(token), "TK_%s", raw->upper);
        emit_raw(raw, token, "keyword", "clause-leading instruction");
        if (text_is(raw, "IF")) adapter->if_condition = 1;
        if (text_is(raw, "DO")) {
            adapter->do_specification = 1;
            push_group(adapter, GROUP_DO);
        }
        if (text_is(raw, "SELECT")) push_group(adapter, GROUP_SELECT);
        if (text_is(raw, "PARSE")) adapter->parse_mode = PARSE_HEADER;
        if (text_is(raw, "ADDRESS")) adapter->address_mode = ADDRESS_TAIL;
        adapter->clause_start = 0;
        adapter->operand_left = 0;
        return;
    }

    maybe_concat(adapter, raw, 0);
    emit_raw(raw, "TK_VAR_SYMBOL", "identifier", "contextual non-keyword");
    adapter->clause_start = 0;
    adapter->operand_left = 1;
}

static void adapt_tokens(const RawToken *tokens) {
    Adapter adapter;
    int i;
    memset(&adapter, 0, sizeof(adapter));
    adapter.clause_start = 1;

    for (i = 0; tokens[i].kind != RAW_EOS; i++) {
        const RawToken *raw = &tokens[i];
        switch (raw->kind) {
            case RAW_WORD:
                emit_word(&adapter, tokens, &i);
                break;
            case RAW_NUMBER:
                maybe_concat(&adapter, raw, 0);
                emit_raw(raw, "TK_NUMBER", "literal", "number operand");
                adapter.clause_start = 0;
                adapter.operand_left = 1;
                break;
            case RAW_STRING:
                maybe_concat(&adapter, raw, 0);
                emit_raw(raw, "TK_STRING", "literal", "string operand");
                adapter.clause_start = 0;
                adapter.operand_left = 1;
                break;
            case RAW_DOT:
                emit_raw(raw, "TK_DOT", adapter.parse_mode == PARSE_TEMPLATE ? "parse_drop" : "punctuation",
                         adapter.parse_mode == PARSE_TEMPLATE ? "parse template placeholder" : "dot");
                adapter.operand_left = 0;
                adapter.clause_start = 0;
                break;
            case RAW_EQUAL:
                emit_raw(raw, "TK_EQUAL", "operator", "assignment or comparison");
                adapter.operand_left = 0;
                adapter.clause_start = 0;
                break;
            case RAW_PLUS:
            case RAW_MINUS:
            case RAW_BACKSLASH:
                emit_raw(raw, raw->kind == RAW_PLUS ? "TK_PLUS" : raw->kind == RAW_MINUS ? "TK_MINUS" : "TK_NOT",
                         "operator", "operator token");
                adapter.operand_left = 0;
                adapter.clause_start = 0;
                break;
            case RAW_LPAREN:
                maybe_concat(&adapter, raw, 0);
                emit_raw(raw, "TK_OPEN", "punctuation", "left parenthesis");
                adapter.operand_left = 0;
                adapter.clause_start = 0;
                break;
            case RAW_RPAREN:
                emit_raw(raw, "TK_CLOSE", "punctuation", "right parenthesis");
                adapter.operand_left = 1;
                adapter.clause_start = 0;
                break;
            case RAW_COMMA:
                emit_raw(raw, "TK_COMMA", "punctuation", "comma");
                adapter.operand_left = 0;
                adapter.clause_start = 0;
                break;
            case RAW_SEMI:
            case RAW_EOL:
                emit_eoc(&adapter, raw, raw->kind == RAW_EOL ? "EOL supplies EOC" : "explicit semicolon");
                break;
            case RAW_COLON:
                emit_raw(raw, "TK_COLON", "punctuation", "colon outside label");
                adapter.operand_left = 0;
                adapter.clause_start = 0;
                break;
            case RAW_EOS:
                break;
        }
    }
    emit_raw(&tokens[i], "TK_EOS", "eos", "end of source");
}

static void dump_raw_tokens(const RawToken *tokens) {
    int i;
    printf("  scanner:\n");
    for (i = 0; tokens[i].kind != RAW_EOS; i++) {
        printf("    %-6s %-12s upper=%-12s blank=%d\n",
               raw_kind_name(tokens[i].kind), tokens[i].text, tokens[i].upper, tokens[i].blank_before);
    }
    printf("    %-6s %-12s upper=%-12s blank=%d\n",
           raw_kind_name(tokens[i].kind), tokens[i].text, tokens[i].upper, tokens[i].blank_before);
}

static void run_fixture(const Fixture *fixture) {
    RawToken tokens[MAX_TOKENS];
    scan_source(fixture->source, tokens);
    printf("fixture %s\n", fixture->name);
    printf("  source: %s\n", fixture->source);
    dump_raw_tokens(tokens);
    printf("  adapter:\n");
    adapt_tokens(tokens);
    printf("\n");
}

int main(void) {
    const Fixture fixtures[] = {
        { "keyword_assignment_if", "if = then" },
        { "keyword_assignment_say", "say = if" },
        { "say_then_identifier", "say then" },
        { "simple_if", "if a then say b" },
        { "invalid_then_condition_shape", "if then then say b" },
        { "nested_if_else", "if a then if b then say c else say d" },
        { "select_when_otherwise", "select; when a then say b; otherwise say c; end" },
        { "do_modifiers", "do i = 1 to 10 by 2 while ready; say i; end i" },
        { "parse_value_template", "parse value a b with x . y" },
        { "address_with", "address value env with output stem out." },
        { "label", "label: say label" },
    };
    size_t i;
    for (i = 0; i < sizeof(fixtures) / sizeof(fixtures[0]); i++) {
        run_fixture(&fixtures[i]);
    }
    return 0;
}
