#include "rxinline_recipe.h"

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
