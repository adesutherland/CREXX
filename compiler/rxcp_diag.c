/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * Structured compiler diagnostics.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "platform.h"
#include "rxcp_diag.h"

#ifdef _WIN32
#define RXCP_DIAG_PATH_SEPARATOR '\\'
#else
#define RXCP_DIAG_PATH_SEPARATOR '/'
#endif

typedef struct RxcpMessageEntry {
    char *code;
    char *template_text;
} RxcpMessageEntry;

typedef struct RxcpMessageCatalog {
    char *locale;
    RxcpMessageEntry *entries;
    size_t entry_count;
    struct RxcpMessageCatalog *next;
} RxcpMessageCatalog;

static char *rxcp_diag_mode_override = 0;
static char *rxcp_diag_locale_override = 0;
static RxcpMessageCatalog *rxcp_diag_catalogs = 0;

static char *rxcp_diag_strdup(const char *value) {
    char *copy;
    size_t length;

    if (!value) value = "";
    length = strlen(value);
    copy = malloc(length + 1);
    if (!copy) return 0;
    memcpy(copy, value, length + 1);
    return copy;
}

static int rxcp_diag_streq_ci(const char *left, const char *right) {
    unsigned char lch;
    unsigned char rch;

    if (!left) left = "";
    if (!right) right = "";
    while (*left || *right) {
        lch = (unsigned char)*left;
        rch = (unsigned char)*right;
        if (tolower(lch) != tolower(rch)) return 0;
        if (*left) left++;
        if (*right) right++;
    }
    return 1;
}

static const char *rxcp_diag_normalize_mode(const char *mode) {
    if (!mode || !mode[0]) return 0;
    if (rxcp_diag_streq_ci(mode, "raw") ||
        rxcp_diag_streq_ci(mode, "machine") ||
        rxcp_diag_streq_ci(mode, "code")) {
        return "raw";
    }
    if (rxcp_diag_streq_ci(mode, "localized") ||
        rxcp_diag_streq_ci(mode, "localised") ||
        rxcp_diag_streq_ci(mode, "text")) {
        return "localized";
    }
    return 0;
}

static char *rxcp_diag_trim(char *text) {
    char *end;

    if (!text) return text;
    while (*text && isspace((unsigned char)*text)) text++;
    end = text + strlen(text);
    while (end > text && isspace((unsigned char)end[-1])) end--;
    *end = 0;
    return text;
}

static char *rxcp_diag_normalize_locale(const char *locale) {
    char *copy;
    char *cursor;
    char *tail;
    size_t length;

    if (!locale || !locale[0]) return rxcp_diag_strdup("en_GB");
    if (rxcp_diag_streq_ci(locale, "C") || rxcp_diag_streq_ci(locale, "POSIX")) {
        return rxcp_diag_strdup("en_GB");
    }

    copy = rxcp_diag_strdup(locale);
    if (!copy) return 0;
    tail = strchr(copy, '.');
    if (tail) *tail = 0;
    tail = strchr(copy, '@');
    if (tail) *tail = 0;
    for (cursor = copy; *cursor; cursor++) {
        if (*cursor == '-') *cursor = '_';
    }

    length = strlen(copy);
    if (length == 2) {
        copy[0] = (char)tolower((unsigned char)copy[0]);
        copy[1] = (char)tolower((unsigned char)copy[1]);
    } else if (length >= 5 && copy[2] == '_') {
        copy[0] = (char)tolower((unsigned char)copy[0]);
        copy[1] = (char)tolower((unsigned char)copy[1]);
        copy[3] = (char)toupper((unsigned char)copy[3]);
        copy[4] = (char)toupper((unsigned char)copy[4]);
    }

    if (!copy[0]) {
        free(copy);
        return rxcp_diag_strdup("en_GB");
    }
    return copy;
}

static int rxcp_diag_append_text(char **buffer, size_t *length, size_t *capacity,
                                 const char *text, size_t text_length) {
    char *next;
    size_t required;
    size_t next_capacity;

    if (!text) text = "";
    required = *length + text_length + 1;
    if (required > *capacity) {
        next_capacity = *capacity ? *capacity : 64;
        while (next_capacity < required) next_capacity *= 2;
        next = realloc(*buffer, next_capacity);
        if (!next) return -1;
        *buffer = next;
        *capacity = next_capacity;
    }
    if (text_length) memcpy(*buffer + *length, text, text_length);
    *length += text_length;
    (*buffer)[*length] = 0;
    return 0;
}

int rxcp_diag_set_mode(const char *mode) {
    const char *normalized;
    char *copy;

    normalized = rxcp_diag_normalize_mode(mode);
    if (!normalized) return -1;
    copy = rxcp_diag_strdup(normalized);
    if (!copy) return -1;
    if (rxcp_diag_mode_override) free(rxcp_diag_mode_override);
    rxcp_diag_mode_override = copy;
    return 0;
}

int rxcp_diag_set_locale(const char *locale) {
    char *copy;

    copy = rxcp_diag_normalize_locale(locale);
    if (!copy) return -1;
    if (rxcp_diag_locale_override) free(rxcp_diag_locale_override);
    rxcp_diag_locale_override = copy;
    return 0;
}

const char *rxcp_diag_mode(void) {
    const char *mode;
    const char *normalized;

    if (rxcp_diag_mode_override) return rxcp_diag_mode_override;
    mode = getenv("CREXX_DIAGNOSTICS");
    normalized = rxcp_diag_normalize_mode(mode);
    return normalized ? normalized : "localized";
}

char *rxcp_diag_effective_locale(void) {
    const char *locale;

    if (rxcp_diag_locale_override) return rxcp_diag_strdup(rxcp_diag_locale_override);
    locale = getenv("CREXX_DIAGNOSTIC_LOCALE");
    if (!locale || !locale[0]) locale = getenv("LC_ALL");
#ifndef _WIN32
    if (!locale || !locale[0]) locale = getenv("LC_MESSAGES");
#endif
    if (!locale || !locale[0]) locale = getenv("LANG");
    return rxcp_diag_normalize_locale(locale);
}

static char *rxcp_diag_escape(const char *value) {
    char *escaped;
    char *out;
    size_t length;
    size_t escaped_length;
    size_t i;
    unsigned char ch;
    char hex[5];

    if (!value) value = "";
    length = strlen(value);
    escaped_length = 0;
    for (i = 0; i < length; i++) {
        ch = (unsigned char)value[i];
        if (ch == '\\' || ch == '"') escaped_length += 2;
        else if (ch == '\n' || ch == '\r' || ch == '\t') escaped_length += 2;
        else if (ch < 32) escaped_length += 4;
        else escaped_length++;
    }

    escaped = malloc(escaped_length + 1);
    if (!escaped) return rxcp_diag_strdup("");

    out = escaped;
    for (i = 0; i < length; i++) {
        ch = (unsigned char)value[i];
        if (ch == '\\' || ch == '"') {
            *out++ = '\\';
            *out++ = (char)ch;
        } else if (ch == '\n') {
            *out++ = '\\';
            *out++ = 'n';
        } else if (ch == '\r') {
            *out++ = '\\';
            *out++ = 'r';
        } else if (ch == '\t') {
            *out++ = '\\';
            *out++ = 't';
        } else if (ch < 32) {
            snprintf(hex, sizeof(hex), "\\x%02X", ch);
            memcpy(out, hex, 4);
            out += 4;
        } else {
            *out++ = (char)ch;
        }
    }
    *out = 0;
    return escaped;
}

RxcpDiagnostic *rxcp_diag_create(const char *code) {
    RxcpDiagnostic *diag;

    diag = calloc(1, sizeof(RxcpDiagnostic));
    if (!diag) return 0;
    diag->code = rxcp_diag_strdup(code && code[0] ? code : "UNKNOWN_DIAGNOSTIC");
    if (!diag->code) {
        free(diag);
        return 0;
    }
    return diag;
}

RxcpDiagnostic *rxcp_diag_clone(const RxcpDiagnostic *diag) {
    RxcpDiagnostic *copy;
    size_t i;

    if (!diag) return 0;
    copy = rxcp_diag_create(diag->code);
    if (!copy) return 0;
    for (i = 0; i < diag->param_count; i++) {
        if (rxcp_diag_add_param(copy, diag->params[i].name, diag->params[i].value) != 0) {
            rxcp_diag_free(copy);
            return 0;
        }
    }
    return copy;
}

void rxcp_diag_free(RxcpDiagnostic *diag) {
    size_t i;

    if (!diag) return;
    if (diag->code) free(diag->code);
    for (i = 0; i < diag->param_count; i++) {
        if (diag->params[i].name) free(diag->params[i].name);
        if (diag->params[i].value) free(diag->params[i].value);
    }
    if (diag->params) free(diag->params);
    free(diag);
}

int rxcp_diag_add_param(RxcpDiagnostic *diag, const char *name, const char *value) {
    RxcpDiagnosticParam *params;
    char *name_copy;
    char *value_copy;
    size_t next_count;

    if (!diag || !name || !name[0]) return -1;

    name_copy = rxcp_diag_strdup(name);
    value_copy = rxcp_diag_strdup(value);
    if (!name_copy || !value_copy) {
        if (name_copy) free(name_copy);
        if (value_copy) free(value_copy);
        return -1;
    }

    next_count = diag->param_count + 1;
    params = realloc(diag->params, sizeof(RxcpDiagnosticParam) * next_count);
    if (!params) {
        free(name_copy);
        free(value_copy);
        return -1;
    }

    diag->params = params;
    diag->params[diag->param_count].name = name_copy;
    diag->params[diag->param_count].value = value_copy;
    diag->param_count = next_count;
    return 0;
}

int rxcp_diag_equal(const RxcpDiagnostic *left, const RxcpDiagnostic *right) {
    size_t i;

    if (left == right) return 1;
    if (!left || !right) return 0;
    if (strcmp(left->code ? left->code : "", right->code ? right->code : "") != 0) return 0;
    if (left->param_count != right->param_count) return 0;
    for (i = 0; i < left->param_count; i++) {
        if (strcmp(left->params[i].name ? left->params[i].name : "",
                   right->params[i].name ? right->params[i].name : "") != 0) return 0;
        if (strcmp(left->params[i].value ? left->params[i].value : "",
                   right->params[i].value ? right->params[i].value : "") != 0) return 0;
    }
    return 1;
}

static int rxcp_diag_catalog_add_entry(RxcpMessageCatalog *catalog,
                                       const char *code,
                                       const char *template_text) {
    RxcpMessageEntry *entries;
    char *code_copy;
    char *template_copy;
    size_t next_count;

    if (!catalog || !code || !code[0] || !template_text) return -1;
    code_copy = rxcp_diag_strdup(code);
    template_copy = rxcp_diag_strdup(template_text);
    if (!code_copy || !template_copy) {
        if (code_copy) free(code_copy);
        if (template_copy) free(template_copy);
        return -1;
    }

    next_count = catalog->entry_count + 1;
    entries = realloc(catalog->entries, sizeof(RxcpMessageEntry) * next_count);
    if (!entries) {
        free(code_copy);
        free(template_copy);
        return -1;
    }

    catalog->entries = entries;
    catalog->entries[catalog->entry_count].code = code_copy;
    catalog->entries[catalog->entry_count].template_text = template_copy;
    catalog->entry_count = next_count;
    return 0;
}

static int rxcp_diag_catalog_load_file(RxcpMessageCatalog *catalog, const char *path) {
    FILE *file;
    char line[8192];

    file = fopen(path, "rb");
    if (!file) return 0;

    while (fgets(line, sizeof(line), file)) {
        char *cursor;
        char *equals;
        char *code;
        char *template_text;

        cursor = line + strlen(line);
        while (cursor > line && (cursor[-1] == '\n' || cursor[-1] == '\r')) {
            *--cursor = 0;
        }

        cursor = rxcp_diag_trim(line);
        if (!cursor[0] || cursor[0] == '#') continue;
        equals = strchr(cursor, '=');
        if (!equals) continue;
        *equals = 0;
        code = rxcp_diag_trim(cursor);
        template_text = rxcp_diag_trim(equals + 1);
        if (code[0] && template_text[0]) {
            (void)rxcp_diag_catalog_add_entry(catalog, code, template_text);
        }
    }

    fclose(file);
    return 1;
}

static int rxcp_diag_catalog_try_dir(RxcpMessageCatalog *catalog,
                                     const char *directory,
                                     const char *locale) {
    char path[MAXFILEPATH * 2];
    size_t length;

    if (!catalog || !directory || !directory[0] || !locale || !locale[0]) return 0;
    length = strlen(directory);
    if (length > 0 && (directory[length - 1] == '/' || directory[length - 1] == '\\')) {
        snprintf(path, sizeof(path), "%sdiagnostics.%s.msg", directory, locale);
    } else {
        snprintf(path, sizeof(path), "%s%cdiagnostics.%s.msg",
                 directory, RXCP_DIAG_PATH_SEPARATOR, locale);
    }
    return rxcp_diag_catalog_load_file(catalog, path);
}

static void rxcp_diag_catalog_try_env_path(RxcpMessageCatalog *catalog, const char *locale) {
    const char *path_list;
    char *copy;
    char *cursor;
    char *next;

    path_list = getenv("CREXX_MESSAGE_PATH");
    if (!path_list || !path_list[0]) return;
    copy = rxcp_diag_strdup(path_list);
    if (!copy) return;

    cursor = copy;
    while (cursor) {
        next = strchr(cursor, ';');
        if (next) *next++ = 0;
        cursor = rxcp_diag_trim(cursor);
        if (cursor[0]) (void)rxcp_diag_catalog_try_dir(catalog, cursor, locale);
        cursor = next;
    }
    free(copy);
}

static void rxcp_diag_catalog_load(RxcpMessageCatalog *catalog) {
    char *exe_dir;
    char directory[MAXFILEPATH * 2];

    if (!catalog || !catalog->locale) return;
    rxcp_diag_catalog_try_env_path(catalog, catalog->locale);

    exe_dir = exepath();
    if (!exe_dir) return;
    if (exe_dir[0]) {
        snprintf(directory, sizeof(directory), "%s%c%s",
                 exe_dir, RXCP_DIAG_PATH_SEPARATOR, "messages");
        (void)rxcp_diag_catalog_try_dir(catalog, directory, catalog->locale);

        snprintf(directory, sizeof(directory), "%s%c..%cshare%ccrexx%c%s",
                 exe_dir, RXCP_DIAG_PATH_SEPARATOR, RXCP_DIAG_PATH_SEPARATOR,
                 RXCP_DIAG_PATH_SEPARATOR, RXCP_DIAG_PATH_SEPARATOR, "messages");
        (void)rxcp_diag_catalog_try_dir(catalog, directory, catalog->locale);
    }
    free(exe_dir);
}

static RxcpMessageCatalog *rxcp_diag_catalog_for_locale(const char *locale) {
    RxcpMessageCatalog *catalog;

    for (catalog = rxcp_diag_catalogs; catalog; catalog = catalog->next) {
        if (strcmp(catalog->locale ? catalog->locale : "", locale ? locale : "") == 0) {
            return catalog;
        }
    }

    catalog = calloc(1, sizeof(RxcpMessageCatalog));
    if (!catalog) return 0;
    catalog->locale = rxcp_diag_strdup(locale && locale[0] ? locale : "en_GB");
    if (!catalog->locale) {
        free(catalog);
        return 0;
    }
    rxcp_diag_catalog_load(catalog);
    catalog->next = rxcp_diag_catalogs;
    rxcp_diag_catalogs = catalog;
    return catalog;
}

static const char *rxcp_diag_catalog_lookup(const char *locale, const char *code) {
    RxcpMessageCatalog *catalog;
    size_t i;

    if (!locale || !locale[0] || !code || !code[0]) return 0;
    catalog = rxcp_diag_catalog_for_locale(locale);
    if (!catalog) return 0;
    for (i = 0; i < catalog->entry_count; i++) {
        if (strcmp(catalog->entries[i].code ? catalog->entries[i].code : "", code) == 0) {
            return catalog->entries[i].template_text;
        }
    }
    return 0;
}

static const char *rxcp_diag_find_template(const char *code) {
    char *locale;
    const char *template_text;

    locale = rxcp_diag_effective_locale();
    if (!locale) return 0;
    template_text = rxcp_diag_catalog_lookup(locale, code);
    if (!template_text && strcmp(locale, "en_GB") != 0) {
        template_text = rxcp_diag_catalog_lookup("en_GB", code);
    }
    free(locale);
    return template_text;
}

static const char *rxcp_diag_param_value(const RxcpDiagnostic *diag,
                                         const char *name,
                                         size_t name_length) {
    size_t i;

    if (!diag || !name) return 0;
    for (i = 0; i < diag->param_count; i++) {
        const char *param_name = diag->params[i].name ? diag->params[i].name : "";
        if (strlen(param_name) == name_length && strncmp(param_name, name, name_length) == 0) {
            return diag->params[i].value ? diag->params[i].value : "";
        }
    }
    return 0;
}

static char *rxcp_diag_render_template(const RxcpDiagnostic *diag, const char *template_text) {
    char *rendered;
    size_t length;
    size_t capacity;
    const char *cursor;

    if (!diag || !diag->code || !template_text) return 0;
    rendered = 0;
    length = 0;
    capacity = 0;

    if (rxcp_diag_append_text(&rendered, &length, &capacity, diag->code, strlen(diag->code)) != 0 ||
        rxcp_diag_append_text(&rendered, &length, &capacity, ": ", 2) != 0) {
        if (rendered) free(rendered);
        return 0;
    }

    cursor = template_text;
    while (*cursor) {
        const char *open_brace;
        const char *close_brace;

        open_brace = strchr(cursor, '{');
        if (!open_brace) {
            if (rxcp_diag_append_text(&rendered, &length, &capacity, cursor, strlen(cursor)) != 0) {
                free(rendered);
                return 0;
            }
            break;
        }

        if (rxcp_diag_append_text(&rendered, &length, &capacity,
                                  cursor, (size_t)(open_brace - cursor)) != 0) {
            free(rendered);
            return 0;
        }

        close_brace = strchr(open_brace + 1, '}');
        if (!close_brace) {
            if (rxcp_diag_append_text(&rendered, &length, &capacity,
                                      open_brace, strlen(open_brace)) != 0) {
                free(rendered);
                return 0;
            }
            break;
        }

        {
            const char *value;
            size_t name_length;

            name_length = (size_t)(close_brace - open_brace - 1);
            value = rxcp_diag_param_value(diag, open_brace + 1, name_length);
            if (value) {
                char *escaped = rxcp_diag_escape(value);
                if (!escaped ||
                    rxcp_diag_append_text(&rendered, &length, &capacity,
                                          escaped, strlen(escaped)) != 0) {
                    if (escaped) free(escaped);
                    free(rendered);
                    return 0;
                }
                free(escaped);
            } else {
                if (rxcp_diag_append_text(&rendered, &length, &capacity,
                                          open_brace, (size_t)(close_brace - open_brace + 1)) != 0) {
                    free(rendered);
                    return 0;
                }
            }
        }
        cursor = close_brace + 1;
    }

    return rendered;
}

char *rxcp_diag_render_raw(const RxcpDiagnostic *diag, const char *fallback) {
    char *rendered;
    size_t length;
    size_t offset;
    size_t i;

    if (!diag || !diag->code) return rxcp_diag_strdup(fallback ? fallback : "UNKNOWN_DIAGNOSTIC");

    length = strlen(diag->code);
    for (i = 0; i < diag->param_count; i++) {
        char *escaped = rxcp_diag_escape(diag->params[i].value);
        length += 1 + strlen(diag->params[i].name ? diag->params[i].name : "") + 2 + strlen(escaped ? escaped : "") + 1;
        if (escaped) free(escaped);
    }

    rendered = malloc(length + 1);
    if (!rendered) return rxcp_diag_strdup(fallback ? fallback : diag->code);

    strcpy(rendered, diag->code);
    offset = strlen(rendered);
    for (i = 0; i < diag->param_count; i++) {
        char *escaped = rxcp_diag_escape(diag->params[i].value);
        const char *name = diag->params[i].name ? diag->params[i].name : "";
        rendered[offset++] = ' ';
        strcpy(rendered + offset, name);
        offset += strlen(name);
        rendered[offset++] = '=';
        rendered[offset++] = '"';
        strcpy(rendered + offset, escaped ? escaped : "");
        offset += strlen(escaped ? escaped : "");
        rendered[offset++] = '"';
        if (escaped) free(escaped);
    }
    rendered[offset] = 0;
    return rendered;
}

char *rxcp_diag_render(const RxcpDiagnostic *diag, const char *fallback) {
    const char *template_text;
    char *rendered;

    if (strcmp(rxcp_diag_mode(), "raw") == 0) {
        return rxcp_diag_render_raw(diag, fallback);
    }

    if (!diag || !diag->code) return rxcp_diag_render_raw(diag, fallback);
    template_text = rxcp_diag_find_template(diag->code);
    if (!template_text) return rxcp_diag_render_raw(diag, fallback);

    rendered = rxcp_diag_render_template(diag, template_text);
    if (!rendered) return rxcp_diag_render_raw(diag, fallback);
    return rendered;
}

char *rxcp_diag_int_string(int value) {
    char buffer[64];

    snprintf(buffer, sizeof(buffer), "%d", value);
    return rxcp_diag_strdup(buffer);
}

char *rxcp_diag_levelc_code(const char *standard_code) {
    char *result;
    size_t code_length;
    const char *prefix = "RXC-LC-";

    if (!standard_code) standard_code = "0";
    code_length = strlen(prefix) + strlen(standard_code);
    result = malloc(code_length + 1);
    if (!result) return 0;
    strcpy(result, prefix);
    strcat(result, standard_code);
    return result;
}
