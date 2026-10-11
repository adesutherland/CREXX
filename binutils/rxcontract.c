/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#include "rxcontract.h"
#include "rxsignature.h"
#include "rxfloatformat.h"
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef struct { const char *start, *p; rx_contract_error *error; } Reader;
static int fail(rx_contract_error *e, const char *field, const char *message, size_t offset) {
    if (e) { e->field = field; e->message = message; e->offset = offset; }
    return 0;
}
static char *copy(const char *s, size_t n) {
    char *v = malloc(n + 1);
    if (v) { memcpy(v, s, n); v[n] = 0; }
    return v;
}
static void blanks(Reader *r) { while (isspace((unsigned char)*r->p)) ++r->p; }
static int byte(unsigned char c) {
    return isalnum(c) || c >= 128 || c == '_' || c == '!' || c == '?' ||
           c == '$' || c == '@' || c == '#';
}
static int name(const char *p) {
    if (!p || !*p || isdigit((unsigned char)*p)) return 0;
    for (; *p; ++p) if (!byte((unsigned char)*p) && *p != '.') return 0;
    return 1;
}
static int word(Reader *r, const char *s) {
    size_t i;
    for (i = 0; s[i]; ++i)
        if (!r->p[i] || tolower((unsigned char)r->p[i]) != s[i]) return 0;
    if (!isspace((unsigned char)r->p[i])) return 0;
    r->p += i;
    blanks(r);
    return 1;
}
static int is_void(const rx_contract_type *t) {
    const char *s = t->name;
    return s && strlen(s) == 5 && s[0] == '.' && tolower((unsigned char)s[1]) == 'v' &&
        tolower((unsigned char)s[2]) == 'o' && tolower((unsigned char)s[3]) == 'i' &&
        tolower((unsigned char)s[4]) == 'd';
}
void rx_contract_type_free(rx_contract_type *t) {
    size_t i;
    if (!t) return;
    free(t->name);
    if (t->referent) { rx_contract_type_free(t->referent); free(t->referent); }
    for (i = 0; i < t->dimensions; ++i) { free(t->ranges[i].lower); free(t->ranges[i].upper); }
    free(t->ranges);
    memset(t, 0, sizeof(*t));
}
static int bound(Reader *r, char **value) {
    const char *start;
    blanks(r);
    if (*r->p == '*') { ++r->p; blanks(r); return 1; }
    start = r->p;
    if (*r->p == '-') ++r->p;
    if (!isdigit((unsigned char)*r->p)) return 0;
    while (isdigit((unsigned char)*r->p)) ++r->p;
    *value = copy(start, (size_t)(r->p - start));
    blanks(r);
    return *value != NULL;
}
static int type(Reader *r, rx_contract_type *t, unsigned depth) {
    const char *start;
    int stem;
    if (depth > 64) return 0;
    blanks(r);
    if (word(r, "reference")) {
        t->referent = calloc(1, sizeof(*t->referent));
        return t->referent && type(r, t->referent, depth + 1) && !is_void(t->referent);
    }
    if (*r->p != '.') return 0;
    start = r->p++;
    if (!byte((unsigned char)*r->p) || isdigit((unsigned char)*r->p)) return 0;
    while (byte((unsigned char)*r->p)) ++r->p;
    while (r->p[0] == '.' && r->p[1] == '.' && byte((unsigned char)r->p[2]) &&
           !isdigit((unsigned char)r->p[2])) {
        r->p += 2;
        while (byte((unsigned char)*r->p)) ++r->p;
    }
    t->name = copy(start, (size_t)(r->p - start));
    if (!t->name) return 0;
    stem = *r->p == '.';
    blanks(r);
    if (*r->p == '[' || stem) {
        int more;
        if (is_void(t)) return 0;
        if (!stem) ++r->p;
        do {
            rx_contract_range *ranges, *range;
            ranges = realloc(t->ranges, (t->dimensions + 1) * sizeof(*ranges));
            if (!ranges) return 0;
            t->ranges = ranges;
            range = &ranges[t->dimensions++];
            memset(range, 0, sizeof(*range));
            if (stem) ++r->p;
            blanks(r);
            if ((!stem && (*r->p == ']' || *r->p == ',')) ||
                (stem && (!*r->p || *r->p == '.'))) {
                /* Empty dimensions have default lower and open upper bounds. */
            } else {
                if (!bound(r, &range->upper)) return 0;
                if (!stem && word(r, "to")) {
                    range->lower = range->upper;
                    range->upper = NULL;
                    if (!bound(r, &range->upper)) return 0;
                }
            }
            blanks(r);
            more = stem ? *r->p == '.' : *r->p == ',';
            if (more && !stem) ++r->p;
        } while (more);
        if (!stem) { if (*r->p != ']') return 0; ++r->p; }
    }
    blanks(r);
    return 1;
}
int rx_contract_parse_type(const char *text, size_t length, rx_contract_type *out,
                           rx_contract_error *error) {
    Reader r;
    char *owned;
    int ok;
    memset(out, 0, sizeof(*out));
    if (!text || memchr(text, 0, length)) return fail(error, "type", "invalid type span", 0);
    owned = copy(text, length);
    if (!owned) return fail(error, "type", "out of memory", 0);
    r.start = r.p = owned; r.error = error;
    ok = type(&r, out, 0) && !*r.p;
    if (!ok) fail(error, "type", "invalid metadata type", (size_t)(r.p - r.start));
    free(owned);
    if (!ok) rx_contract_type_free(out);
    return ok;
}
static int default_value(const char *text, rx_contract_arg *arg) {
    const char *s = text, *end;
    char *parsed_end;
    size_t n;
    while (isspace((unsigned char)*s)) ++s;
    n = strlen(s);
    while (n && isspace((unsigned char)s[n - 1])) --n;
    end = s + n;
    if (!n) return 0;
    if (*s == '\'' || *s == '"') {
        const char *p = s + 1;
        while (p < end) {
            if (*p == *s) {
                ++p;
                if (p == end) break;
                if (*p != *s) return 0;
            }
            ++p;
        }
        if (p != end || n < 2 || end[-1] != *s) return 0;
        arg->default_kind = RX_CONTRACT_STRING_DEFAULT;
        arg->type.name = copy(".string", 7);
    } else {
        const char *number = s;
        double special_value;
        int special = rx_double_from_special_literal(s, n, &special_value);
        if (special < 0) return 0;
        if (special) {
            arg->default_kind = RX_CONTRACT_FLOAT_DEFAULT;
            arg->type.name = copy(".float", 6);
            arg->default_literal = copy(s, n);
            return arg->type.name && arg->default_literal;
        }
        if (*number == '+' || *number == '-') ++number;
        if (!isdigit((unsigned char)*number) &&
            !(*number == '.' && isdigit((unsigned char)number[1]))) return 0;
        errno = 0;
        strtoll(s, &parsed_end, n > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X') ? 16 : 10);
        while (parsed_end < end && isspace((unsigned char)*parsed_end)) ++parsed_end;
        if (parsed_end == end && errno != ERANGE) {
            arg->default_kind = RX_CONTRACT_INTEGER_DEFAULT;
            arg->type.name = copy(".int", 4);
        } else {
            errno = 0;
            strtod(s, &parsed_end);
            while (parsed_end < end && isspace((unsigned char)*parsed_end)) ++parsed_end;
            if (parsed_end != end || errno == ERANGE) return 0;
            arg->default_kind = RX_CONTRACT_FLOAT_DEFAULT;
            arg->type.name = copy(".float", 6);
        }
    }
    arg->default_literal = copy(s, n);
    return arg->type.name && arg->default_literal;
}
void rx_contract_callable_free(rx_contract_callable *c) {
    size_t i;
    if (!c) return;
    free(c->name); rx_contract_type_free(&c->result);
    for (i = 0; i < c->arg_count; ++i) {
        free(c->args[i].name); free(c->args[i].default_literal);
        rx_contract_type_free(&c->args[i].type);
    }
    free(c->args); memset(c, 0, sizeof(*c));
}
int rx_contract_parse_callable(rx_contract_callable_kind kind, const char *call_name,
    const char *result, const char *args, int task, rx_contract_callable *out, rx_contract_error *error) {
    rx_callable_signature signature;
    size_t i;
    memset(out, 0, sizeof(*out));
    if (kind < RX_CONTRACT_PROCEDURE || kind > RX_CONTRACT_FACTORY ||
        (!name(call_name) && !(kind == RX_CONTRACT_FACTORY && call_name && !strcmp(call_name, "*"))))
        return fail(error, "name", "invalid callable name", 0);
    if (!result || !*result) result = ".void";
    if (!rx_contract_parse_type(result, strlen(result), &out->result, error)) {
        if (error) error->field = "return";
        return 0;
    }
    while (args && isspace((unsigned char)*args)) ++args;
    if (!rx_sig_init_from_parts(&signature, call_name, result, args)) {
        rx_contract_callable_free(out);
        return fail(error, "arguments", "invalid argument fields", 0);
    }
    out->kind = kind; out->task = (unsigned char)(task != 0);
    out->name = copy(signature.name, strlen(signature.name));
    out->args = calloc(signature.arg_count, sizeof(*out->args));
    out->arg_count = out->args ? signature.arg_count : 0;
    if (!out->name || (signature.arg_count && !out->args)) goto bad;
    for (i = 0; i < signature.arg_count; ++i) {
        rx_callable_arg *in = &signature.args[i];
        rx_contract_arg *a = &out->args[i];
        a->name = copy(in->name, strlen(in->name));
        a->by_reference = in->is_ref; a->optional = in->is_optional; a->vararg = in->is_vararg;
        if ((!in->is_vararg && !name(in->name)) || !a->name) goto bad;
        if (!rx_contract_parse_type(in->type, strlen(in->type), &a->type, NULL) &&
            !default_value(in->type, a)) goto bad;
        if (is_void(&a->type)) goto bad;
        if (a->default_kind != RX_CONTRACT_NO_DEFAULT) a->optional = 1;
        if (a->vararg && (a->optional || i + 1 != signature.arg_count)) goto bad;
    }
    rx_sig_free(&signature);
    return 1;
bad:
    rx_sig_free(&signature); rx_contract_callable_free(out);
    return fail(error, "arguments", "invalid argument contract", 0);
}
void rx_contract_attribute_free(rx_contract_attribute *a) {
    if (!a) return;
    free(a->name); free(a->view); rx_contract_type_free(&a->type);
    memset(a, 0, sizeof(*a));
}
int rx_contract_parse_attribute(const char *attribute_name, const char *text, int reg,
    const char *view, rx_contract_attribute *out, rx_contract_error *error) {
    memset(out, 0, sizeof(*out));
    if (!name(attribute_name) || reg < -1 || (view && !name(view)))
        return fail(error, "attribute", "invalid attribute name or mapping", 0);
    if (!text || !rx_contract_parse_type(text, strlen(text), &out->type, error)) return 0;
    if (is_void(&out->type)) { rx_contract_attribute_free(out); return fail(error, "type", "void attribute", 0); }
    out->name = copy(attribute_name, strlen(attribute_name));
    out->view = view ? copy(view, strlen(view)) : NULL;
    out->register_index = reg;
    if (!out->name || (view && !out->view)) { rx_contract_attribute_free(out); return fail(error, "attribute", "out of memory", 0); }
    return 1;
}
void rx_contract_class_header_free(rx_contract_class_header *h) {
    if (!h) return;
    free(h->namespace_name); free(h->name); memset(h, 0, sizeof(*h));
}
int rx_contract_parse_class_header(const char *ns, const char *class_name, int is_interface,
    rx_contract_class_header *out, rx_contract_error *error) {
    memset(out, 0, sizeof(*out));
    if (!ns || (*ns && !name(ns)) || !name(class_name)) return fail(error, "class", "invalid class header", 0);
    out->namespace_name = copy(ns, strlen(ns)); out->name = copy(class_name, strlen(class_name));
    out->is_interface = (unsigned char)(is_interface != 0);
    if (!out->namespace_name || !out->name) { rx_contract_class_header_free(out); return fail(error, "class", "out of memory", 0); }
    return 1;
}
