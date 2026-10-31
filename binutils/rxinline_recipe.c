#include "rxinline_recipe.h"

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t read_u32(const unsigned char *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8u |
           (uint32_t)p[2] << 16u | (uint32_t)p[3] << 24u;
}

static void write_u32(unsigned char *p, uint32_t value) {
    p[0] = (unsigned char)value;
    p[1] = (unsigned char)(value >> 8u);
    p[2] = (unsigned char)(value >> 16u);
    p[3] = (unsigned char)(value >> 24u);
}

static void skip_space(const char **at) {
    while (**at == ' ' || **at == '\t') ++*at;
}

static int expect(const char **at, const char *literal) {
    size_t length;
    skip_space(at);
    length = strlen(literal);
    if (strncmp(*at, literal, length) != 0) return 0;
    *at += length;
    skip_space(at);
    return 1;
}

static int decimal(const char **at, uint32_t *result) {
    uint32_t value = 0u;
    int any = 0;
    skip_space(at);
    while (**at >= '0' && **at <= '9') {
        unsigned int digit = (unsigned int)(**at - '0');
        if (value > (UINT32_MAX - digit) / 10u) return 0;
        value = value * 10u + digit;
        ++*at;
        any = 1;
    }
    skip_space(at);
    if (!any) return 0;
    *result = value;
    return 1;
}

static int reg(const char **at, uint32_t *result) {
    if (!expect(at, "r")) return 0;
    return decimal(at, result) && *result != UINT32_MAX;
}

static int parse_regs(const char **at, uint32_t *items, uint32_t *count) {
    *count = 0u;
    if (!expect(at, "(")) return 0;
    if (expect(at, ")")) return 1;
    for (;;) {
        if (*count == RXINL_RECIPE_MAX_ITEMS || !reg(at, &items[*count])) return 0;
        ++*count;
        if (expect(at, ")")) return 1;
        if (!expect(at, ",")) return 0;
    }
}

static int parse_proxies(const char **at, uint32_t *items, uint32_t *count) {
    *count = 0u;
    if (!expect(at, "(")) return 0;
    if (expect(at, ")")) return 1;
    for (;;) {
        if (*count == RXINL_RECIPE_MAX_ITEMS ||
            !reg(at, &items[*count * 2u]) ||
            !expect(at, ":") ||
            !decimal(at, &items[*count * 2u + 1u])) return 0;
        ++*count;
        if (expect(at, ")")) return 1;
        if (!expect(at, ",")) return 0;
    }
}

static int parse_swap(const char **at, uint32_t *first, uint32_t *second) {
    return expect(at, "swap(") && reg(at, first) && expect(at, ",") &&
           reg(at, second) && expect(at, ")");
}

static int parse_settp(const char **at, uint32_t *target, uint32_t *flags) {
    return expect(at, "settp(") && reg(at, target) && expect(at, ",") &&
           decimal(at, flags) && expect(at, ")");
}

static int append_format(char *buffer, size_t capacity, size_t *used,
                         const char *format, ...) {
    int written;
    va_list args;
    if (*used >= capacity) return 0;
    va_start(args, format);
    written = vsnprintf(buffer + *used, capacity - *used, format, args);
    va_end(args);
    if (written < 0 || (size_t)written >= capacity - *used) return 0;
    *used += (size_t)written;
    return 1;
}

int rxinl_recipe_decode(const void *source, size_t length,
                        rxinl_recipe_view *view) {
    const unsigned char *p = (const unsigned char *)source;
    uint32_t argument_items;
    size_t item_count;
    uint32_t i;
    if (!view || !p || length < RXINL_RECIPE_HEADER_SIZE ||
        memcmp(p, "INL1", 4u) != 0) return 0;
    memset(view, 0, sizeof(*view));
    view->bytes = p;
    view->length = length;
    view->flags = read_u32(p + 4u);
    view->image_module = read_u32(p + 8u);
    view->provider_procedure = read_u32(p + 12u);
    view->count_source = read_u32(p + 16u);
    view->a0_register = read_u32(p + 20u);
    view->argument_base = read_u32(p + 24u);
    view->argument_count = read_u32(p + 28u);
    view->proxy_count = read_u32(p + 32u);
    view->owned_count = read_u32(p + 36u);
    view->status_register = read_u32(p + 40u);
    view->status_flags = read_u32(p + 44u);
    view->swap_first = read_u32(p + 48u);
    view->swap_second = read_u32(p + 52u);
    view->clear_register = read_u32(p + 56u);
    if ((view->flags & ~RXINL_RECIPE_KNOWN_FLAGS) ||
        (view->flags & RXINL_RECIPE_SEALED &&
         (view->image_module == RXINL_RECIPE_UNSEALED ||
          view->provider_procedure == RXINL_RECIPE_UNSEALED)) ||
        (!(view->flags & RXINL_RECIPE_SEALED) &&
         (view->image_module != RXINL_RECIPE_UNSEALED ||
          view->provider_procedure != RXINL_RECIPE_UNSEALED)) ||
        view->a0_register == UINT32_MAX ||
        view->argument_count > RXINL_RECIPE_MAX_ITEMS ||
        view->proxy_count > RXINL_RECIPE_MAX_ITEMS ||
        view->owned_count > RXINL_RECIPE_MAX_ITEMS ||
        read_u32(p + 60u)) return 0;
    if (!(view->flags & RXINL_COUNT_REGISTER) &&
        view->count_source > RXINL_RECIPE_MAX_ITEMS) return 0;
    if ((view->flags & RXINL_COUNT_REGISTER) &&
        view->count_source == UINT32_MAX) return 0;
    if (!(view->flags & RXINL_COUNT_REGISTER) &&
        !(view->flags & RXINL_ARGS_WINDOW) &&
        view->count_source != view->argument_count) return 0;
    if (!(view->flags & RXINL_ARGS_WINDOW) && view->argument_base != 0u) return 0;
    if (view->flags & RXINL_ARGS_WINDOW && view->argument_count != 0u) return 0;
    if ((view->flags & RXINL_ARGS_WINDOW) &&
        view->argument_base == UINT32_MAX) return 0;
    if (!(view->flags & RXINL_PREP_SETTP) &&
        (view->status_register || view->status_flags)) return 0;
    if (!(view->flags & RXINL_PREP_SWAP) &&
        (view->swap_first || view->swap_second)) return 0;
    if (!(view->flags & RXINL_PREP_CLEAR) && view->clear_register) return 0;
    if ((view->flags & RXINL_PREP_CLEAR) &&
        view->clear_register == UINT32_MAX) return 0;
    if ((view->flags & RXINL_PREP_SETTP) &&
        view->status_register == UINT32_MAX) return 0;
    if ((view->flags & RXINL_PREP_SWAP) &&
        (view->swap_first == UINT32_MAX ||
         view->swap_second == UINT32_MAX)) return 0;
    if ((view->flags & RXINL_PREP_CLEAR) &&
        (view->flags & (RXINL_PREP_SETTP | RXINL_PREP_SWAP))) return 0;
    argument_items = (view->flags & RXINL_ARGS_WINDOW) ? 0u : view->argument_count;
    item_count = (size_t)argument_items + (size_t)view->proxy_count * 2u +
                 (size_t)view->owned_count;
    if (item_count > (SIZE_MAX - RXINL_RECIPE_HEADER_SIZE) / 4u ||
        length != RXINL_RECIPE_HEADER_SIZE + item_count * 4u) return 0;
    for (i = 0u; i < view->proxy_count; ++i) {
        uint32_t j;
        /* A fixed formal may be mentioned behind an a0/presence guard even
         * when the caller supplied fewer actuals. Its proxy then keeps its
         * fresh owned backing cell instead of borrowing a caller pointer. */
        if (rxinl_recipe_proxy_register(view, i) == UINT32_MAX) return 0;
        for (j = 0u; j < i; ++j)
            if (rxinl_recipe_proxy_register(view, j) ==
                rxinl_recipe_proxy_register(view, i)) return 0;
        if (rxinl_recipe_proxy_register(view, i) == view->a0_register) return 0;
    }
    for (i = 0u; i < view->owned_count; ++i) {
        uint32_t j;
        uint32_t reg_index = rxinl_recipe_owned_register(view, i);
        if (reg_index == UINT32_MAX) return 0;
        for (j = 0u; j < i; ++j)
            if (rxinl_recipe_owned_register(view, j) == reg_index) return 0;
    }
    return 1;
}

int rxinl_recipe_seal(void *source, size_t length, uint32_t image_module,
                       uint32_t provider_procedure) {
    unsigned char *p = (unsigned char *)source;
    rxinl_recipe_view view;
    if (!p || !rxinl_recipe_decode(p, length, &view) ||
        image_module == UINT32_MAX || provider_procedure == UINT32_MAX) return 0;
    write_u32(p + 4u, view.flags | RXINL_RECIPE_SEALED);
    write_u32(p + 8u, image_module);
    write_u32(p + 12u, provider_procedure);
    return rxinl_recipe_decode(p, length, &view);
}

char *rxinl_recipe_to_text(const rxinl_recipe_view *view) {
    size_t capacity, used = 0u;
    uint32_t i;
    char *buffer;
    rxinl_recipe_view checked;
    if (!view || !rxinl_recipe_decode(view->bytes, view->length, &checked)) return NULL;
    capacity = 512u + (size_t)RXINL_RECIPE_MAX_ITEMS * 4u * 24u;
    buffer = (char *)malloc(capacity);
    if (!buffer) return NULL;
#define ADD(...) do { if (!append_format(buffer, capacity, &used, __VA_ARGS__)) goto failed; } while (0)
    if (checked.flags & RXINL_COUNT_REGISTER)
        ADD("count=r%u", checked.count_source);
    else ADD("count=%u", checked.count_source);
    if (checked.flags & RXINL_ARGS_WINDOW)
        ADD(";args=window(r%u)", checked.argument_base);
    else {
        ADD(";args=list(");
        for (i = 0u; i < checked.argument_count; ++i)
            ADD("%sr%u", i ? "," : "", rxinl_recipe_argument(&checked, i));
        ADD(")");
    }
    ADD(";a0=r%u;proxies=(", checked.a0_register);
    for (i = 0u; i < checked.proxy_count; ++i)
        ADD("%sr%u:%u", i ? "," : "",
            rxinl_recipe_proxy_register(&checked, i),
            rxinl_recipe_proxy_argument(&checked, i));
    ADD(");owned=(");
    for (i = 0u; i < checked.owned_count; ++i)
        ADD("%sr%u", i ? "," : "", rxinl_recipe_owned_register(&checked, i));
    ADD(");prep=");
    if (!(checked.flags & (RXINL_PREP_SETTP | RXINL_PREP_SWAP | RXINL_PREP_CLEAR))) ADD("none");
    if (checked.flags & RXINL_PREP_CLEAR)
        ADD("clear(r%u)", checked.clear_register);
    if (checked.flags & RXINL_PREP_SETTP)
        ADD("settp(r%u,%u)", checked.status_register, checked.status_flags);
    if (checked.flags & RXINL_PREP_SWAP)
        ADD("%sswap(r%u,r%u)", checked.flags & RXINL_PREP_SETTP ? "+" : "",
            checked.swap_first, checked.swap_second);
    if (checked.flags & RXINL_RESULT_IGNORED) ADD(";result=ignored");
#undef ADD
    return buffer;
failed:
#undef ADD
    free(buffer);
    return NULL;
}

uint32_t rxinl_recipe_argument(const rxinl_recipe_view *view, uint32_t index) {
    if (!view || view->flags & RXINL_ARGS_WINDOW || index >= view->argument_count)
        return UINT32_MAX;
    return read_u32(view->bytes + RXINL_RECIPE_HEADER_SIZE + (size_t)index * 4u);
}

uint32_t rxinl_recipe_proxy_register(const rxinl_recipe_view *view, uint32_t index) {
    size_t base;
    if (!view || index >= view->proxy_count) return UINT32_MAX;
    base = RXINL_RECIPE_HEADER_SIZE +
           (size_t)((view->flags & RXINL_ARGS_WINDOW) ? 0u : view->argument_count) * 4u;
    return read_u32(view->bytes + base + (size_t)index * 8u);
}

uint32_t rxinl_recipe_proxy_argument(const rxinl_recipe_view *view, uint32_t index) {
    size_t base;
    if (!view || index >= view->proxy_count) return UINT32_MAX;
    base = RXINL_RECIPE_HEADER_SIZE +
           (size_t)((view->flags & RXINL_ARGS_WINDOW) ? 0u : view->argument_count) * 4u;
    return read_u32(view->bytes + base + (size_t)index * 8u + 4u);
}

uint32_t rxinl_recipe_owned_register(const rxinl_recipe_view *view, uint32_t index) {
    size_t base;
    if (!view || index >= view->owned_count) return UINT32_MAX;
    base = RXINL_RECIPE_HEADER_SIZE +
           (size_t)(((view->flags & RXINL_ARGS_WINDOW) ? 0u : view->argument_count) +
                    view->proxy_count * 2u) * 4u;
    return read_u32(view->bytes + base + (size_t)index * 4u);
}

int rxinl_recipe_from_text(const char *source, unsigned char **bytes,
                           size_t *length, char *error, size_t error_size) {
    const char *at = source;
    uint32_t *arguments = NULL;
    uint32_t *proxies = NULL;
    uint32_t *owned = NULL;
    uint32_t flags = 0u, count_source = 0u, a0 = 0u, argument_base = 0u;
    uint32_t argument_count = 0u, proxy_count = 0u, owned_count = 0u;
    uint32_t status_register = 0u, status_flags = 0u;
    uint32_t swap_first = 0u, swap_second = 0u;
    uint32_t clear_register = 0u;
    size_t item_count, total_size, position;
    unsigned char *out = NULL;
    uint32_t i;
    int good = 0;

    if (bytes) *bytes = NULL;
    if (length) *length = 0u;
    if (error && error_size) error[0] = 0;
    if (!source || !bytes || !length) return 0;
    arguments = (uint32_t *)malloc(RXINL_RECIPE_MAX_ITEMS * sizeof(uint32_t));
    proxies = (uint32_t *)malloc(RXINL_RECIPE_MAX_ITEMS * 2u * sizeof(uint32_t));
    owned = (uint32_t *)malloc(RXINL_RECIPE_MAX_ITEMS * sizeof(uint32_t));
    if (!arguments || !proxies || !owned) goto done;

    if (!expect(&at, "count=")) goto done;
    if (*at == 'r') {
        flags |= RXINL_COUNT_REGISTER;
        if (!reg(&at, &count_source)) goto done;
    } else if (!decimal(&at, &count_source)) goto done;
    if (!expect(&at, ";args=")) goto done;
    if (expect(&at, "window")) {
        flags |= RXINL_ARGS_WINDOW;
        if (!expect(&at, "(") || !reg(&at, &argument_base) || !expect(&at, ")")) goto done;
    } else if (expect(&at, "list")) {
        if (!parse_regs(&at, arguments, &argument_count)) goto done;
    } else goto done;
    if (!expect(&at, ";a0=") || !reg(&at, &a0) ||
        !expect(&at, ";proxies=") || !parse_proxies(&at, proxies, &proxy_count) ||
        !expect(&at, ";owned=") || !parse_regs(&at, owned, &owned_count) ||
        !expect(&at, ";prep=")) goto done;
    if (!expect(&at, "none")) {
        if (*at == 'c' && at[1] == 'l') {
            flags |= RXINL_PREP_CLEAR;
            if (!expect(&at, "clear(") || !reg(&at, &clear_register) ||
                !expect(&at, ")")) goto done;
        } else if (*at == 's' && at[1] == 'w') {
            flags |= RXINL_PREP_SWAP;
            if (!parse_swap(&at, &swap_first, &swap_second)) goto done;
        } else {
            flags |= RXINL_PREP_SETTP;
            if (!parse_settp(&at, &status_register, &status_flags)) goto done;
            if (expect(&at, "+")) {
                flags |= RXINL_PREP_SWAP;
                if (!parse_swap(&at, &swap_first, &swap_second)) goto done;
            }
        }
    }
    if (expect(&at, ";result=ignored")) flags |= RXINL_RESULT_IGNORED;
    skip_space(&at);
    if (*at || (!(flags & RXINL_COUNT_REGISTER) &&
                !(flags & RXINL_ARGS_WINDOW) && argument_count != count_source)) goto done;
    item_count = (size_t)argument_count + (size_t)proxy_count * 2u + owned_count;
    total_size = RXINL_RECIPE_HEADER_SIZE + item_count * 4u;
    out = (unsigned char *)calloc(total_size, 1u);
    if (!out) goto done;
    memcpy(out, "INL1", 4u);
    write_u32(out + 4u, flags);
    write_u32(out + 8u, RXINL_RECIPE_UNSEALED);
    write_u32(out + 12u, RXINL_RECIPE_UNSEALED);
    write_u32(out + 16u, count_source);
    write_u32(out + 20u, a0);
    write_u32(out + 24u, argument_base);
    write_u32(out + 28u, argument_count);
    write_u32(out + 32u, proxy_count);
    write_u32(out + 36u, owned_count);
    write_u32(out + 40u, status_register);
    write_u32(out + 44u, status_flags);
    write_u32(out + 48u, swap_first);
    write_u32(out + 52u, swap_second);
    write_u32(out + 56u, clear_register);
    position = RXINL_RECIPE_HEADER_SIZE;
    for (i = 0u; i < argument_count; i++, position += 4u)
        write_u32(out + position, arguments[i]);
    for (i = 0u; i < proxy_count * 2u; i++, position += 4u)
        write_u32(out + position, proxies[i]);
    for (i = 0u; i < owned_count; i++, position += 4u)
        write_u32(out + position, owned[i]);
    {
        rxinl_recipe_view view;
        if (!rxinl_recipe_decode(out, total_size, &view)) goto done;
    }
    *bytes = out;
    *length = total_size;
    out = NULL;
    good = 1;
done:
    if (!good && error && error_size)
        snprintf(error, error_size,
                 "invalid inline recipe; expected count, args, a0, proxies, owned, prep in that order");
    free(out);
    free(arguments);
    free(proxies);
    free(owned);
    return good;
}

int rxinl_module_ref_decode(const void *source, size_t length,
                            rxinl_module_ref_view *view) {
    const unsigned char *p = (const unsigned char *)source;
    uint32_t flags, name_length;
    if (!p || !view || length < 16u || memcmp(p, "QGR1", 4u) != 0) return 0;
    flags = read_u32(p + 8u);
    name_length = read_u32(p + 12u);
    if (flags & ~3u || name_length == 0u || name_length > 4096u ||
        length != 16u + (size_t)name_length ||
        memchr(p + 16u, 0, name_length) != NULL ||
        ((flags & 1u) == 0u && read_u32(p + 4u) != UINT32_MAX) ||
        ((flags & 1u) != 0u && read_u32(p + 4u) == UINT32_MAX) ||
        ((flags & 2u) != 0u &&
         (name_length != 4u || memcmp(p + 16u, "self", 4u) != 0))) return 0;
    if (p[16u] == '@') {
        uint32_t ordinal = 0u;
        uint32_t i;
        if (name_length < 2u) return 0;
        for (i = 1u; i < name_length; ++i) {
            unsigned int digit;
            if (p[16u + i] < '0' || p[16u + i] > '9') return 0;
            digit = (unsigned int)(p[16u + i] - '0');
            if (ordinal > (UINT32_MAX - digit) / 10u) return 0;
            ordinal = ordinal * 10u + digit;
        }
        if (ordinal == UINT32_MAX ||
            ((flags & 1u) && ordinal != read_u32(p + 4u))) return 0;
    }
    view->image_module = read_u32(p + 4u);
    view->selector = (const char *)(p + 16u);
    view->selector_length = name_length;
    view->sealed = (int)(flags & 1u);
    view->self_relative = (int)((flags & 2u) != 0u);
    return 1;
}

int rxinl_module_ref_seal(void *source, size_t length, uint32_t image_module) {
    unsigned char *p = (unsigned char *)source;
    rxinl_module_ref_view view;
    if (!p || image_module == UINT32_MAX ||
        !rxinl_module_ref_decode(p, length, &view)) return 0;
    write_u32(p + 4u, image_module);
    write_u32(p + 8u, (uint32_t)(view.self_relative ? 2u : 0u) | 1u);
    return rxinl_module_ref_decode(p, length, &view);
}

int rxinl_module_ref_from_text(const char *selector, unsigned char **bytes,
                               size_t *length) {
    size_t selector_length;
    unsigned char *out;
    rxinl_module_ref_view view;
    if (bytes) *bytes = NULL;
    if (length) *length = 0u;
    if (!selector || !bytes || !length) return 0;
    selector_length = strlen(selector);
    if (!selector_length || selector_length > 4096u) return 0;
    if (selector[0] == '@') {
        const char *at = selector + 1;
        uint32_t ordinal;
        if (!decimal(&at, &ordinal) || *at || ordinal == UINT32_MAX) return 0;
    }
    out = (unsigned char *)calloc(16u + selector_length, 1u);
    if (!out) return 0;
    memcpy(out, "QGR1", 4u);
    write_u32(out + 4u, UINT32_MAX);
    write_u32(out + 8u, strcmp(selector, "self") == 0 ? 2u : 0u);
    write_u32(out + 12u, (uint32_t)selector_length);
    memcpy(out + 16u, selector, selector_length);
    if (!rxinl_module_ref_decode(out, 16u + selector_length, &view)) {
        free(out);
        return 0;
    }
    *bytes = out;
    *length = 16u + selector_length;
    return 1;
}

int rxinl_qualified_proc_decode(const void *source, size_t length,
                                rxinl_qualified_proc_view *view) {
    const unsigned char *p = (const unsigned char *)source;
    uint32_t flags;
    if (!p || !view || length != 16u || memcmp(p, "QPR1", 4u) != 0) return 0;
    flags = read_u32(p + 12u);
    if (flags & ~1u || read_u32(p + 4u) == UINT32_MAX ||
        read_u32(p + 8u) == UINT32_MAX) return 0;
    view->image_module = read_u32(p + 4u);
    view->provider_procedure = read_u32(p + 8u);
    view->sealed = (int)(flags & 1u);
    return 1;
}

int rxinl_qualified_proc_seal(void *source, size_t length) {
    unsigned char *p = (unsigned char *)source;
    rxinl_qualified_proc_view view;
    if (!p || !rxinl_qualified_proc_decode(p, length, &view)) return 0;
    write_u32(p + 12u, 1u);
    return rxinl_qualified_proc_decode(p, length, &view);
}

int rxinl_qualified_proc_from_text(const char *source, unsigned char **bytes,
                                   size_t *length) {
    const char *at = source;
    uint32_t module, procedure;
    unsigned char *out;
    rxinl_qualified_proc_view view;
    if (bytes) *bytes = NULL;
    if (length) *length = 0u;
    if (!source || !bytes || !length || *at++ != '@' ||
        !decimal(&at, &module) || !expect(&at, "::") ||
        !decimal(&at, &procedure) || *at ||
        module == UINT32_MAX || procedure == UINT32_MAX) return 0;
    out = (unsigned char *)calloc(16u, 1u);
    if (!out) return 0;
    memcpy(out, "QPR1", 4u);
    write_u32(out + 4u, module);
    write_u32(out + 8u, procedure);
    if (!rxinl_qualified_proc_decode(out, 16u, &view)) {
        free(out);
        return 0;
    }
    *bytes = out;
    *length = 16u;
    return 1;
}
