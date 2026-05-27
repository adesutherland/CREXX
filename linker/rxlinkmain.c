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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "rxbin.h"

typedef struct string_list {
    char **items;
    size_t count;
    size_t capacity;
} string_list;

typedef struct link_module_info {
    module_file *module;
    char *input_path;
    size_t input_index;
    size_t member_index;
    char *selector_name;
    string_list exports;
    string_list imports;
    string_list defined_interfaces;
    string_list implemented_interfaces;
    string_list referenced_interfaces;
    string_list unresolved_imports;
    int has_main;
    int selected;
    int queued;
    int omitted;
} link_module_info;

typedef struct module_list {
    link_module_info *items;
    size_t count;
    size_t capacity;
} module_list;

typedef struct link_config {
    string_list inputs;
    string_list roots;
    string_list includes;
    string_list omits;
    char *output_path;
    char *map_path;
    char *location;
    int strip_source_metadata;
    int strip_inline_metadata;
    int debug_mode;
} link_config;

typedef struct const_map_entry {
    size_t old_offset;
    size_t new_offset;
} const_map_entry;

typedef struct rxlink_output_module {
    module_file *module;
    const_map_entry *maps;
    size_t map_count;
    size_t map_capacity;
} rxlink_output_module;

typedef struct rxlink_output_list {
    rxlink_output_module *items;
    size_t count;
    size_t capacity;
} rxlink_output_list;

typedef struct leaf_dedupe_entry {
    enum const_pool_type type;
    size_t size_in_pool;
    size_t output_offset;
} leaf_dedupe_entry;

typedef struct rxlink_build_context {
    rxbin_byte_buffer shared_pool;
    leaf_dedupe_entry *leaf_entries;
    size_t leaf_count;
    size_t leaf_capacity;
    int strip_source_metadata;
    int strip_inline_metadata;
} rxlink_build_context;

static void string_list_init(string_list *list) {
    list->items = 0;
    list->count = 0;
    list->capacity = 0;
}

static void string_list_free(string_list *list) {
    size_t i;
    if (!list) return;
    for (i = 0; i < list->count; i++) {
        free(list->items[i]);
    }
    free(list->items);
    list->items = 0;
    list->count = 0;
    list->capacity = 0;
}

static int string_list_contains(const string_list *list, const char *value) {
    size_t i;
    for (i = 0; i < list->count; i++) {
        if (strcmp(list->items[i], value) == 0) return 1;
    }
    return 0;
}

static int string_list_append(string_list *list, const char *value) {
    char **new_items;
    size_t new_capacity;
    char *copy;

    copy = strdup(value);
    if (!copy) return 0;

    if (list->count == list->capacity) {
        new_capacity = list->capacity ? list->capacity * 2 : 8;
        new_items = realloc(list->items, sizeof(char *) * new_capacity);
        if (!new_items) {
            free(copy);
            return 0;
        }
        list->items = new_items;
        list->capacity = new_capacity;
    }

    list->items[list->count++] = copy;
    return 1;
}

static int string_list_append_unique(string_list *list, const char *value) {
    if (string_list_contains(list, value)) return 1;
    return string_list_append(list, value);
}

static int module_list_append(module_list *list, link_module_info *item) {
    link_module_info *new_items;
    size_t new_capacity;

    if (list->count == list->capacity) {
        new_capacity = list->capacity ? list->capacity * 2 : 16;
        new_items = realloc(list->items, sizeof(link_module_info) * new_capacity);
        if (!new_items) return 0;
        list->items = new_items;
        list->capacity = new_capacity;
    }

    list->items[list->count++] = *item;
    return 1;
}

static void module_list_free(module_list *list) {
    size_t i;

    if (!list) return;
    for (i = 0; i < list->count; i++) {
        free_module(list->items[i].module);
        free(list->items[i].input_path);
        free(list->items[i].selector_name);
        string_list_free(&list->items[i].exports);
        string_list_free(&list->items[i].imports);
        string_list_free(&list->items[i].defined_interfaces);
        string_list_free(&list->items[i].implemented_interfaces);
        string_list_free(&list->items[i].referenced_interfaces);
        string_list_free(&list->items[i].unresolved_imports);
    }
    free(list->items);
    list->items = 0;
    list->count = 0;
    list->capacity = 0;
}

static int output_list_append(rxlink_output_list *list, rxlink_output_module *item) {
    rxlink_output_module *new_items;
    size_t new_capacity;

    if (list->count == list->capacity) {
        new_capacity = list->capacity ? list->capacity * 2 : 8;
        new_items = realloc(list->items, sizeof(rxlink_output_module) * new_capacity);
        if (!new_items) return 0;
        list->items = new_items;
        list->capacity = new_capacity;
    }

    list->items[list->count++] = *item;
    return 1;
}

static void output_list_free(rxlink_output_list *list) {
    size_t i;

    if (!list) return;
    for (i = 0; i < list->count; i++) {
        if (list->items[i].module) free_module(list->items[i].module);
        free(list->items[i].maps);
    }
    free(list->items);
    list->items = 0;
    list->count = 0;
    list->capacity = 0;
}

static void build_context_init(rxlink_build_context *context) {
    rxbin_byte_buffer_init(&context->shared_pool);
    context->leaf_entries = 0;
    context->leaf_count = 0;
    context->leaf_capacity = 0;
    context->strip_source_metadata = 0;
    context->strip_inline_metadata = 1;
}

static void build_context_free(rxlink_build_context *context) {
    rxbin_byte_buffer_free(&context->shared_pool);
    free(context->leaf_entries);
    context->leaf_entries = 0;
    context->leaf_count = 0;
    context->leaf_capacity = 0;
}

static int keyword_equals(const char *left, const char *right) {
    while (*left && *right) {
        if (toupper((unsigned char)*left) != toupper((unsigned char)*right)) return 0;
        left++;
        right++;
    }
    return *left == '\0' && *right == '\0';
}

static char *trim_whitespace(char *text) {
    char *end;

    while (*text && isspace((unsigned char)*text)) text++;
    if (!*text) return text;

    end = text + strlen(text) - 1;
    while (end >= text && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
    return text;
}

static char *unquote_in_place(char *text) {
    size_t len;

    text = trim_whitespace(text);
    len = strlen(text);
    if (len >= 2 && ((text[0] == '"' && text[len - 1] == '"') || (text[0] == '\'' && text[len - 1] == '\''))) {
        text[len - 1] = '\0';
        text++;
    }
    return text;
}

static void init_link_config(link_config *config) {
    string_list_init(&config->inputs);
    string_list_init(&config->roots);
    string_list_init(&config->includes);
    string_list_init(&config->omits);
    config->output_path = 0;
    config->map_path = 0;
    config->location = 0;
    config->strip_source_metadata = 0;
    config->strip_inline_metadata = 1;
    config->debug_mode = 0;
}

static void free_link_config(link_config *config) {
    string_list_free(&config->inputs);
    string_list_free(&config->roots);
    string_list_free(&config->includes);
    string_list_free(&config->omits);
    free(config->output_path);
    free(config->map_path);
}

static int set_single_path(char **target, const char *value) {
    char *copy = strdup(value);
    if (!copy) return 0;
    if (*target) free(*target);
    *target = copy;
    return 1;
}

static int parse_control_file(link_config *config, const char *path) {
    FILE *fp;
    char line[4096];
    size_t line_number = 0;

    fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "ERROR: opening control file %s\n", path);
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        char *cursor;
        char *keyword;
        char *value;

        line_number++;
        cursor = trim_whitespace(line);
        if (!*cursor || *cursor == '*' || *cursor == '#') continue;

        keyword = cursor;
        while (*cursor && !isspace((unsigned char)*cursor)) cursor++;
        if (*cursor) {
            *cursor = '\0';
            cursor++;
        }

        value = unquote_in_place(cursor);
        if (!*value) {
            fprintf(stderr, "ERROR: control file %s line %lu missing value for %s\n",
                    path, (unsigned long)line_number, keyword);
            fclose(fp);
            return 0;
        }

        if (keyword_equals(keyword, "INPUT")) {
            if (!string_list_append(&config->inputs, value)) goto oom;
        } else if (keyword_equals(keyword, "ROOT")) {
            if (!string_list_append(&config->roots, value)) goto oom;
        } else if (keyword_equals(keyword, "INCLUDE")) {
            if (!string_list_append(&config->includes, value)) goto oom;
        } else if (keyword_equals(keyword, "OMIT")) {
            if (!string_list_append(&config->omits, value)) goto oom;
        } else if (keyword_equals(keyword, "OUTPUT")) {
            if (!set_single_path(&config->output_path, value)) goto oom;
        } else if (keyword_equals(keyword, "MAP")) {
            if (!set_single_path(&config->map_path, value)) goto oom;
        } else if (keyword_equals(keyword, "STRIP")) {
            if (keyword_equals(value, "SOURCE")) {
                config->strip_source_metadata = 1;
            } else if (keyword_equals(value, "INLINE")) {
                config->strip_inline_metadata = 1;
            } else {
                fprintf(stderr, "ERROR: unknown STRIP mode %s on line %lu\n",
                        value, (unsigned long)line_number);
                fclose(fp);
                return 0;
            }
        } else if (keyword_equals(keyword, "PRESERVE")) {
            if (keyword_equals(value, "INLINE")) {
                config->strip_inline_metadata = 0;
            } else {
                fprintf(stderr, "ERROR: unknown PRESERVE mode %s on line %lu\n",
                        value, (unsigned long)line_number);
                fclose(fp);
                return 0;
            }
        } else {
            fprintf(stderr, "ERROR: unknown control directive %s on line %lu\n",
                    keyword, (unsigned long)line_number);
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return 1;

oom:
    fprintf(stderr, "PANIC: Out of memory while reading control file %s\n", path);
    fclose(fp);
    return 0;
}

static int module_selector_matches(const link_module_info *module, const char *selector) {
    const char *separator = strstr(selector, "::");

    if (separator) {
        size_t input_len = (size_t)(separator - selector);
        const char *member = separator + 2;
        const char *module_basename = filename(module->module->name);

        if (module->input_path[input_len] != '\0') return 0;
        if (strncmp(selector, module->input_path, input_len) != 0) return 0;
        if (!*member) return 0;

        return strcmp(member, module->module->name) == 0 ||
               strcmp(member, module_basename) == 0 ||
               strcmp(member, module->selector_name) == 0;
    }

    return strcmp(selector, module->module->name) == 0 ||
           strcmp(selector, filename(module->module->name)) == 0 ||
           strcmp(selector, module->selector_name) == 0;
}

static int load_module_metadata(link_module_info *info) {
    int ix;
    module_file *module = info->module;
    size_t code_index;

    ix = module->header.proc_head;
    while (ix != -1) {
        proc_constant *proc = (proc_constant *)((unsigned char *)module->constant + (size_t)ix);
        if (proc->base.type != PROC_CONST) return 0;
        if (strcmp(proc->name, "main") == 0) info->has_main = 1;
        ix = proc->next;
    }

    ix = module->header.expose_head;
    while (ix != -1) {
        chameleon_constant *entry = (chameleon_constant *)((unsigned char *)module->constant + (size_t)ix);
        if (entry->type == EXPOSE_PROC_CONST) {
            expose_proc_constant *exposed = (expose_proc_constant *)entry;
            if (exposed->imported) {
                if (!string_list_append_unique(&info->imports, exposed->index)) return 0;
            } else {
                if (!string_list_append_unique(&info->exports, exposed->index)) return 0;
            }
            ix = exposed->next;
        } else if (entry->type == EXPOSE_REG_CONST) {
            ix = ((expose_reg_constant *)entry)->next;
        } else {
            return 0;
        }
    }

    ix = module->header.meta_head;
    while (ix != -1) {
        meta_entry *entry = (meta_entry *)((unsigned char *)module->constant + (size_t)ix);
        switch (entry->base.type) {
            case META_INTERFACE: {
                meta_interface_constant *iface = (meta_interface_constant *)entry;
                string_constant *symbol = (string_constant *)((unsigned char *)module->constant + iface->symbol);
                if (!string_list_append_unique(&info->defined_interfaces, symbol->string)) return 0;
                break;
            }
            case META_IMPLEMENTS: {
                meta_implements_constant *impl = (meta_implements_constant *)entry;
                string_constant *iface = (string_constant *)((unsigned char *)module->constant + impl->interface_symbol);
                if (!string_list_append_unique(&info->implemented_interfaces, iface->string)) return 0;
                break;
            }
            default:
                break;
        }
        ix = entry->next;
    }

    code_index = 0;
    while (code_index < module->header.instruction_size) {
        OperandType types[3];
        int operand_count;
        int operand_index;
        int opcode;

        opcode = ((bin_code *)module->instructions)[code_index].instruction.opcode;
        operand_count = rxbin_get_operand_types(rxbin_opcode_format(opcode), types);
        if (opcode == OP_SRCFPROC_REG_STRING_REG) {
            for (operand_index = 0; operand_index < operand_count; operand_index++) {
                if (types[operand_index] == OP_STRING) {
                    size_t selector_offset;
                    string_constant *selector;
                    const char *separator;
                    size_t interface_length;
                    char *interface_name;
                    int ok;

                    selector_offset = ((bin_code *)module->instructions)[code_index + (size_t)operand_index + 1].index;
                    if (selector_offset >= module->header.constant_size) return 0;
                    selector = (string_constant *)((unsigned char *)module->constant + selector_offset);
                    if (selector->base.type != STRING_CONST) return 0;

                    separator = strstr(selector->string, "..");
                    interface_length = separator ? (size_t)(separator - selector->string) : strlen(selector->string);
                    if (!interface_length) continue;

                    interface_name = malloc(interface_length + 1);
                    if (!interface_name) return 0;
                    memcpy(interface_name, selector->string, interface_length);
                    interface_name[interface_length] = '\0';
                    ok = string_list_append_unique(&info->referenced_interfaces, interface_name);
                    free(interface_name);
                    if (!ok) return 0;
                }
            }
        }
        code_index += (size_t)operand_count + 1;
    }

    return 1;
}

static int load_input_modules(module_list *modules, const link_config *config) {
    size_t input_index;

    for (input_index = 0; input_index < config->inputs.count; input_index++) {
        const char *input_path = config->inputs.items[input_index];
        const char *type_bin = has_any_extension(input_path) ? "" : "rxbin";
        FILE *fp = openfile((char *)input_path, (char *)type_bin, config->location, "rb");
        int rc = 0;
        size_t member_index = 0;

        if (!fp) {
            fprintf(stderr, "ERROR: opening input %s\n", input_path);
            return 0;
        }

        while (rc == 0) {
            module_file *module = 0;
            link_module_info info;

            rc = read_module(&module, fp);
            if (rc == 0) {
                memset(&info, 0, sizeof(info));
                string_list_init(&info.exports);
                string_list_init(&info.imports);
                string_list_init(&info.defined_interfaces);
                string_list_init(&info.implemented_interfaces);
                string_list_init(&info.referenced_interfaces);
                string_list_init(&info.unresolved_imports);
                info.module = module;
                info.input_path = strdup(input_path);
                info.input_index = input_index;
                info.member_index = member_index++;
                info.selector_name = strip_rightmost_extension_if(filename(module->name), "rxas");
                if (!info.input_path || !info.selector_name || !load_module_metadata(&info) ||
                    !module_list_append(modules, &info)) {
                    free_module(module);
                    free(info.input_path);
                    free(info.selector_name);
                    string_list_free(&info.exports);
                    string_list_free(&info.imports);
                    string_list_free(&info.defined_interfaces);
                    string_list_free(&info.implemented_interfaces);
                    string_list_free(&info.referenced_interfaces);
                    string_list_free(&info.unresolved_imports);
                    fclose(fp);
                    return 0;
                }
            } else if (rc != 1) {
                fprintf(stderr, "ERROR: reading input %s\n", input_path);
                fclose(fp);
                return 0;
            }
        }

        fclose(fp);
    }

    return 1;
}

static int add_to_queue(size_t **queue_ref, size_t *queue_count, size_t *queue_capacity, size_t index) {
    size_t *new_queue;
    size_t new_capacity;

    if (*queue_count == *queue_capacity) {
        new_capacity = *queue_capacity ? *queue_capacity * 2 : 16;
        new_queue = realloc(*queue_ref, sizeof(size_t) * new_capacity);
        if (!new_queue) return 0;
        *queue_ref = new_queue;
        *queue_capacity = new_capacity;
    }
    (*queue_ref)[(*queue_count)++] = index;
    return 1;
}

static int select_module_by_index(module_list *modules, size_t **queue_ref, size_t *queue_count,
                                  size_t *queue_capacity, size_t index) {
    if (modules->items[index].queued) return 1;
    modules->items[index].queued = 1;
    modules->items[index].selected = 1;
    return add_to_queue(queue_ref, queue_count, queue_capacity, index);
}

static int selected_export_conflicts(const module_list *modules, const char *export_name, size_t *first_index,
                                     size_t *second_index) {
    size_t i;
    int found = 0;

    for (i = 0; i < modules->count; i++) {
        if (!modules->items[i].selected || modules->items[i].omitted) continue;
        if (!string_list_contains(&modules->items[i].exports, export_name)) continue;
        if (!found) {
            *first_index = i;
            found = 1;
        } else {
            *second_index = i;
            return 1;
        }
    }

    return 0;
}

static int select_modules(module_list *modules, const link_config *config) {
    size_t i;
    size_t *queue = 0;
    size_t queue_count = 0;
    size_t queue_capacity = 0;
    size_t queue_pos = 0;
    int roots_applied = 0;

    for (i = 0; i < config->omits.count; i++) {
        size_t j;
        int matched = 0;
        for (j = 0; j < modules->count; j++) {
            if (module_selector_matches(&modules->items[j], config->omits.items[i])) {
                modules->items[j].omitted = 1;
                matched = 1;
            }
        }
        if (!matched) {
            fprintf(stderr, "ERROR: OMIT selector %s matched no modules\n", config->omits.items[i]);
            free(queue);
            return 0;
        }
    }

    for (i = 0; i < config->includes.count; i++) {
        size_t j;
        int matched = 0;
        for (j = 0; j < modules->count; j++) {
            if (module_selector_matches(&modules->items[j], config->includes.items[i])) {
                if (modules->items[j].omitted) {
                    fprintf(stderr, "ERROR: module %s is both included and omitted\n", config->includes.items[i]);
                    free(queue);
                    return 0;
                }
                if (!select_module_by_index(modules, &queue, &queue_count, &queue_capacity, j)) {
                    free(queue);
                    return 0;
                }
                matched = 1;
                roots_applied = 1;
            }
        }
        if (!matched) {
            fprintf(stderr, "ERROR: INCLUDE selector %s matched no modules\n", config->includes.items[i]);
            free(queue);
            return 0;
        }
    }

    for (i = 0; i < config->roots.count; i++) {
        size_t j;
        int matched = 0;
        for (j = 0; j < modules->count; j++) {
            if (module_selector_matches(&modules->items[j], config->roots.items[i])) {
                if (modules->items[j].omitted) {
                    fprintf(stderr, "ERROR: root module %s is omitted\n", config->roots.items[i]);
                    free(queue);
                    return 0;
                }
                if (!select_module_by_index(modules, &queue, &queue_count, &queue_capacity, j)) {
                    free(queue);
                    return 0;
                }
                matched = 1;
                roots_applied = 1;
            }
        }
        if (!matched) {
            fprintf(stderr, "ERROR: ROOT selector %s matched no modules\n", config->roots.items[i]);
            free(queue);
            return 0;
        }
    }

    if (!roots_applied) {
        int found_main = 0;
        for (i = 0; i < modules->count; i++) {
            if (!modules->items[i].omitted && modules->items[i].has_main) {
                if (!select_module_by_index(modules, &queue, &queue_count, &queue_capacity, i)) {
                    free(queue);
                    return 0;
                }
                found_main = 1;
            }
        }

        if (!found_main) {
            for (i = 0; i < modules->count; i++) {
                if (!modules->items[i].omitted && modules->items[i].input_index == 0) {
                    if (!select_module_by_index(modules, &queue, &queue_count, &queue_capacity, i)) {
                        free(queue);
                        return 0;
                    }
                }
            }
        }
    }

    if (!queue_count) {
        fprintf(stderr, "ERROR: no root modules selected\n");
        free(queue);
        return 0;
    }

    while (queue_pos < queue_count) {
        link_module_info *module = &modules->items[queue[queue_pos++]];
        size_t import_index;
        size_t iface_index;

        for (import_index = 0; import_index < module->imports.count; import_index++) {
            size_t provider_index = 0;
            size_t provider_count = 0;
            size_t candidate;

            for (candidate = 0; candidate < modules->count; candidate++) {
                if (modules->items[candidate].omitted) continue;
                if (string_list_contains(&modules->items[candidate].exports, module->imports.items[import_index])) {
                    provider_index = candidate;
                    provider_count++;
                }
            }

            if (provider_count == 1) {
                if (!select_module_by_index(modules, &queue, &queue_count, &queue_capacity, provider_index)) {
                    free(queue);
                    return 0;
                }
            } else if (provider_count > 1) {
                fprintf(stderr, "ERROR: Duplicate providers for import %s\n", module->imports.items[import_index]);
                free(queue);
                return 0;
            } else {
                if (!string_list_append_unique(&module->unresolved_imports, module->imports.items[import_index])) {
                    free(queue);
                    return 0;
                }
            }
        }

        for (iface_index = 0; iface_index < module->implemented_interfaces.count; iface_index++) {
            size_t candidate;
            for (candidate = 0; candidate < modules->count; candidate++) {
                if (modules->items[candidate].omitted) continue;
                if (string_list_contains(&modules->items[candidate].defined_interfaces,
                                         module->implemented_interfaces.items[iface_index])) {
                    if (!select_module_by_index(modules, &queue, &queue_count, &queue_capacity, candidate)) {
                        free(queue);
                        return 0;
                    }
                }
            }
        }

        for (iface_index = 0; iface_index < module->referenced_interfaces.count; iface_index++) {
            size_t candidate;
            for (candidate = 0; candidate < modules->count; candidate++) {
                if (modules->items[candidate].omitted) continue;
                if (string_list_contains(&modules->items[candidate].defined_interfaces,
                                         module->referenced_interfaces.items[iface_index]) ||
                    string_list_contains(&modules->items[candidate].implemented_interfaces,
                                         module->referenced_interfaces.items[iface_index])) {
                    if (!select_module_by_index(modules, &queue, &queue_count, &queue_capacity, candidate)) {
                        free(queue);
                        return 0;
                    }
                }
            }
        }

        for (iface_index = 0; iface_index < module->defined_interfaces.count; iface_index++) {
            size_t candidate;
            for (candidate = 0; candidate < modules->count; candidate++) {
                if (modules->items[candidate].omitted) continue;
                if (string_list_contains(&modules->items[candidate].implemented_interfaces,
                                         module->defined_interfaces.items[iface_index])) {
                    if (!select_module_by_index(modules, &queue, &queue_count, &queue_capacity, candidate)) {
                        free(queue);
                        return 0;
                    }
                }
            }
        }
    }

    for (i = 0; i < modules->count; i++) {
        size_t export_index;
        if (!modules->items[i].selected || modules->items[i].omitted) continue;
        for (export_index = 0; export_index < modules->items[i].exports.count; export_index++) {
            size_t first_index = 0;
            size_t second_index = 0;
            if (selected_export_conflicts(modules, modules->items[i].exports.items[export_index],
                                          &first_index, &second_index) &&
                first_index != i) {
                fprintf(stderr, "ERROR: Duplicate exported symbol %s in %s and %s\n",
                        modules->items[i].exports.items[export_index],
                        modules->items[first_index].selector_name,
                        modules->items[second_index].selector_name);
                free(queue);
                return 0;
            }
        }
    }

    free(queue);
    return 1;
}

static int reserve_pool_entry(rxlink_build_context *context, size_t size_in_pool, enum const_pool_type type,
                              size_t *offset_out) {
    size_t offset = context->shared_pool.size;

    if (!rxbin_byte_buffer_reserve(&context->shared_pool, size_in_pool)) return 0;
    memset(context->shared_pool.data + context->shared_pool.size, 0, size_in_pool);
    context->shared_pool.size += size_in_pool;
    ((chameleon_constant *)(context->shared_pool.data + offset))->size_in_pool = size_in_pool;
    ((chameleon_constant *)(context->shared_pool.data + offset))->type = type;
    *offset_out = offset;
    return 1;
}

static const_map_entry *find_const_map(rxlink_output_module *module, size_t old_offset) {
    size_t i;
    for (i = 0; i < module->map_count; i++) {
        if (module->maps[i].old_offset == old_offset) return &module->maps[i];
    }
    return 0;
}

static int add_const_map(rxlink_output_module *module, size_t old_offset, size_t new_offset) {
    const_map_entry *new_entries;
    size_t new_capacity;

    if (find_const_map(module, old_offset)) return 1;

    if (module->map_count == module->map_capacity) {
        new_capacity = module->map_capacity ? module->map_capacity * 2 : 32;
        new_entries = realloc(module->maps, sizeof(const_map_entry) * new_capacity);
        if (!new_entries) return 0;
        module->maps = new_entries;
        module->map_capacity = new_capacity;
    }

    module->maps[module->map_count].old_offset = old_offset;
    module->maps[module->map_count].new_offset = new_offset;
    module->map_count++;
    return 1;
}

static size_t dedupe_leaf_constant(rxlink_build_context *context, const chameleon_constant *entry, int *ok) {
    size_t i;
    size_t offset;
    leaf_dedupe_entry *new_entries;
    size_t new_capacity;

    for (i = 0; i < context->leaf_count; i++) {
        if (context->leaf_entries[i].type == entry->type &&
            context->leaf_entries[i].size_in_pool == entry->size_in_pool &&
            memcmp(context->shared_pool.data + context->leaf_entries[i].output_offset,
                   entry, entry->size_in_pool) == 0) {
            return context->leaf_entries[i].output_offset;
        }
    }

    if (!reserve_pool_entry(context, entry->size_in_pool, entry->type, &offset)) {
        *ok = 0;
        return 0;
    }

    memcpy(context->shared_pool.data + offset, entry, entry->size_in_pool);

    if (context->leaf_count == context->leaf_capacity) {
        new_capacity = context->leaf_capacity ? context->leaf_capacity * 2 : 32;
        new_entries = realloc(context->leaf_entries, sizeof(leaf_dedupe_entry) * new_capacity);
        if (!new_entries) {
            *ok = 0;
            return 0;
        }
        context->leaf_entries = new_entries;
        context->leaf_capacity = new_capacity;
    }

    context->leaf_entries[context->leaf_count].type = entry->type;
    context->leaf_entries[context->leaf_count].size_in_pool = entry->size_in_pool;
    context->leaf_entries[context->leaf_count].output_offset = offset;
    context->leaf_count++;
    return offset;
}

static size_t link_constant_offset(rxlink_build_context *context, rxlink_output_module *output_module,
                                   module_file *input_module, size_t old_offset, int *ok);

static int is_meta_constant_type(enum const_pool_type type) {
    switch (type) {
        case META_SRC:
        case META_FILE:
        case META_SOURCE_STEP:
        case META_TRACE_EVENT:
        case META_FUNC:
        case META_REG:
        case META_CONST:
        case META_CLEAR:
        case META_CLASS:
        case META_ATTR:
        case META_INTERFACE:
        case META_IMPLEMENTS:
        case META_MEMBER:
        case META_INLINE:
            return 1;
        default:
            return 0;
    }
}

static int should_strip_meta_constant(const rxlink_build_context *context, enum const_pool_type type) {
    if (context->strip_source_metadata && (type == META_SRC || type == META_FILE || type == META_SOURCE_STEP)) return 1;
    if (context->strip_inline_metadata && type == META_INLINE) return 1;
    return 0;
}

static int rewrite_meta_constant(rxlink_build_context *context, rxlink_output_module *output_module,
                                 module_file *input_module, chameleon_constant *entry,
                                 size_t new_offset, int prev_offset, int next_offset, int *ok) {
    switch (entry->type) {
        case META_SRC: {
            meta_src_constant *source = (meta_src_constant *)entry;
            size_t source_offset = link_constant_offset(context, output_module, input_module, source->source, ok);
            meta_src_constant *meta = (meta_src_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->source = source_offset;
            return *ok;
        }
        case META_FILE: {
            meta_file_constant *source = (meta_file_constant *)entry;
            size_t file_offset = link_constant_offset(context, output_module, input_module, source->file, ok);
            meta_file_constant *meta = (meta_file_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->file = file_offset;
            return *ok;
        }
        case META_SOURCE_STEP: {
            meta_source_step_constant *source = (meta_source_step_constant *)entry;
            size_t file_offset = link_constant_offset(context, output_module, input_module, source->file, ok);
            size_t source_line_offset = link_constant_offset(context, output_module, input_module, source->source_line, ok);
            meta_source_step_constant *meta = (meta_source_step_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->file = file_offset;
            meta->source_line = source_line_offset;
            return *ok;
        }
        case META_TRACE_EVENT: {
            meta_trace_event_constant *source = (meta_trace_event_constant *)entry;
            meta_trace_event_constant *meta = (meta_trace_event_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            if (source->value_source == RXBIN_TRACE_VALUE_CONSTANT &&
                source->value_ref != RXBIN_TRACE_REF_NONE) {
                meta->value_ref = link_constant_offset(context, output_module, input_module, source->value_ref, ok);
            }
            if (source->symbol != RXBIN_TRACE_REF_NONE) {
                meta->symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            }
            if (source->resolved_name != RXBIN_TRACE_REF_NONE) {
                meta->resolved_name = link_constant_offset(context, output_module, input_module, source->resolved_name, ok);
            }
            return *ok;
        }
        case META_FUNC: {
            meta_func_constant *source = (meta_func_constant *)entry;
            size_t symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            size_t option = link_constant_offset(context, output_module, input_module, source->option, ok);
            size_t type = link_constant_offset(context, output_module, input_module, source->type, ok);
            size_t func = link_constant_offset(context, output_module, input_module, source->func, ok);
            size_t args = link_constant_offset(context, output_module, input_module, source->args, ok);
            meta_func_constant *meta = (meta_func_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            meta->option = option;
            meta->type = type;
            meta->func = func;
            meta->args = args;
            return *ok;
        }
        case META_REG: {
            meta_reg_constant *source = (meta_reg_constant *)entry;
            size_t symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            size_t option = link_constant_offset(context, output_module, input_module, source->option, ok);
            size_t type = link_constant_offset(context, output_module, input_module, source->type, ok);
            meta_reg_constant *meta = (meta_reg_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            meta->option = option;
            meta->type = type;
            return *ok;
        }
        case META_CONST: {
            meta_const_constant *source = (meta_const_constant *)entry;
            size_t symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            size_t option = link_constant_offset(context, output_module, input_module, source->option, ok);
            size_t type = link_constant_offset(context, output_module, input_module, source->type, ok);
            size_t constant = link_constant_offset(context, output_module, input_module, source->constant, ok);
            meta_const_constant *meta = (meta_const_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            meta->option = option;
            meta->type = type;
            meta->constant = constant;
            return *ok;
        }
        case META_CLEAR: {
            meta_clear_constant *source = (meta_clear_constant *)entry;
            size_t symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            meta_clear_constant *meta = (meta_clear_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            return *ok;
        }
        case META_CLASS: {
            meta_class_constant *source = (meta_class_constant *)entry;
            size_t symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            size_t option = link_constant_offset(context, output_module, input_module, source->option, ok);
            size_t type = link_constant_offset(context, output_module, input_module, source->type, ok);
            meta_class_constant *meta = (meta_class_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            meta->option = option;
            meta->type = type;
            return *ok;
        }
        case META_ATTR: {
            meta_attr_constant *source = (meta_attr_constant *)entry;
            size_t symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            size_t option = link_constant_offset(context, output_module, input_module, source->option, ok);
            size_t type = link_constant_offset(context, output_module, input_module, source->type, ok);
            meta_attr_constant *meta = (meta_attr_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            meta->option = option;
            meta->type = type;
            return *ok;
        }
        case META_INTERFACE: {
            meta_interface_constant *source = (meta_interface_constant *)entry;
            size_t symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            size_t option = link_constant_offset(context, output_module, input_module, source->option, ok);
            size_t type = link_constant_offset(context, output_module, input_module, source->type, ok);
            meta_interface_constant *meta = (meta_interface_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            meta->option = option;
            meta->type = type;
            return *ok;
        }
        case META_IMPLEMENTS: {
            meta_implements_constant *source = (meta_implements_constant *)entry;
            size_t symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            size_t interface_symbol = link_constant_offset(context, output_module, input_module,
                                                           source->interface_symbol, ok);
            meta_implements_constant *meta = (meta_implements_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            meta->interface_symbol = interface_symbol;
            return *ok;
        }
        case META_MEMBER: {
            meta_member_constant *source = (meta_member_constant *)entry;
            size_t owner = link_constant_offset(context, output_module, input_module, source->owner, ok);
            size_t kind = link_constant_offset(context, output_module, input_module, source->kind, ok);
            size_t member = link_constant_offset(context, output_module, input_module, source->member, ok);
            size_t type = link_constant_offset(context, output_module, input_module, source->type, ok);
            size_t args = link_constant_offset(context, output_module, input_module, source->args, ok);
            meta_member_constant *meta = (meta_member_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->owner = owner;
            meta->kind = kind;
            meta->member = member;
            meta->type = type;
            meta->args = args;
            return *ok;
        }
        case META_INLINE: {
            meta_inline_constant *source = (meta_inline_constant *)entry;
            size_t symbol = link_constant_offset(context, output_module, input_module, source->symbol, ok);
            size_t payload = link_constant_offset(context, output_module, input_module, source->payload, ok);
            meta_inline_constant *meta = (meta_inline_constant *)(context->shared_pool.data + new_offset);
            meta->base.prev = prev_offset;
            meta->base.next = next_offset;
            meta->symbol = symbol;
            meta->payload = payload;
            return *ok;
        }
        default:
            *ok = 0;
            return 0;
    }
}

static int link_meta_chain(rxlink_build_context *context, rxlink_output_module *output_module,
                           module_file *input_module, int old_head, int *ok) {
    int old_offset = old_head;
    int new_head = -1;
    int previous_new = -1;

    while (old_offset != -1) {
        chameleon_constant *entry;
        meta_entry *old_meta;
        size_t new_offset;

        if ((size_t)old_offset >= input_module->header.constant_size) {
            *ok = 0;
            return -1;
        }

        entry = (chameleon_constant *)((unsigned char *)input_module->constant + (size_t)old_offset);
        if (!is_meta_constant_type(entry->type)) {
            *ok = 0;
            return -1;
        }

        old_meta = (meta_entry *)entry;
        if (should_strip_meta_constant(context, entry->type)) {
            old_offset = old_meta->next;
            continue;
        }

        if (!reserve_pool_entry(context, entry->size_in_pool, entry->type, &new_offset)) {
            *ok = 0;
            return -1;
        }
        if (!add_const_map(output_module, (size_t)old_offset, new_offset)) {
            *ok = 0;
            return -1;
        }

        memcpy(context->shared_pool.data + new_offset, entry, entry->size_in_pool);
        if (!rewrite_meta_constant(context, output_module, input_module, entry, new_offset, previous_new, -1, ok)) {
            return -1;
        }

        if (previous_new != -1) {
            ((meta_entry *)(context->shared_pool.data + (size_t)previous_new))->next = (int)new_offset;
        } else {
            new_head = (int)new_offset;
        }
        previous_new = (int)new_offset;
        old_offset = old_meta->next;
    }

    return new_head;
}

static int link_constant_offset_int(rxlink_build_context *context, rxlink_output_module *output_module,
                                    module_file *input_module, int old_offset, int *ok) {
    size_t new_offset;

    if (old_offset == -1) return -1;
    new_offset = link_constant_offset(context, output_module, input_module, (size_t)old_offset, ok);
    if (!*ok || new_offset > (size_t)INT_MAX) {
        *ok = 0;
        return -1;
    }
    return (int)new_offset;
}

static size_t link_constant_offset(rxlink_build_context *context, rxlink_output_module *output_module,
                                   module_file *input_module, size_t old_offset, int *ok) {
    chameleon_constant *entry;
    const_map_entry *existing;
    size_t new_offset;

    if (old_offset == SIZE_MAX) return SIZE_MAX;
    if (old_offset >= input_module->header.constant_size) {
        *ok = 0;
        return 0;
    }

    existing = find_const_map(output_module, old_offset);
    if (existing) return existing->new_offset;

    entry = (chameleon_constant *)((unsigned char *)input_module->constant + old_offset);
    switch (entry->type) {
        case STRING_CONST:
        case BINARY_CONST:
        case DECIMAL_CONST:
        case FLOAT_CONST:
            new_offset = dedupe_leaf_constant(context, entry, ok);
            if (!*ok) return 0;
            if (!add_const_map(output_module, old_offset, new_offset)) {
                *ok = 0;
                return 0;
            }
            return new_offset;
        default:
            break;
    }

    if (!reserve_pool_entry(context, entry->size_in_pool, entry->type, &new_offset)) {
        *ok = 0;
        return 0;
    }
    if (!add_const_map(output_module, old_offset, new_offset)) {
        *ok = 0;
        return 0;
    }

    memcpy(context->shared_pool.data + new_offset, entry, entry->size_in_pool);

    switch (entry->type) {
        case PROC_CONST: {
            proc_constant *source = (proc_constant *)entry;
            int next = link_constant_offset_int(context, output_module, input_module, source->next, ok);
            size_t exposed = link_constant_offset(context, output_module, input_module, source->exposed, ok);
            proc_constant *proc = (proc_constant *)(context->shared_pool.data + new_offset);
            proc->next = next;
            proc->exposed = exposed;
            break;
        }
        case EXPOSE_REG_CONST: {
            expose_reg_constant *source = (expose_reg_constant *)entry;
            int next = link_constant_offset_int(context, output_module, input_module, source->next, ok);
            expose_reg_constant *reg = (expose_reg_constant *)(context->shared_pool.data + new_offset);
            reg->next = next;
            break;
        }
        case EXPOSE_PROC_CONST: {
            expose_proc_constant *source = (expose_proc_constant *)entry;
            int next = link_constant_offset_int(context, output_module, input_module, source->next, ok);
            size_t procedure = link_constant_offset(context, output_module, input_module, source->procedure, ok);
            expose_proc_constant *proc = (expose_proc_constant *)(context->shared_pool.data + new_offset);
            proc->next = next;
            proc->procedure = procedure;
            break;
        }
        case META_SRC:
        case META_FILE:
        case META_SOURCE_STEP:
        case META_TRACE_EVENT:
        case META_FUNC:
        case META_REG:
        case META_CONST:
        case META_CLEAR:
        case META_CLASS:
        case META_ATTR:
        case META_INTERFACE:
        case META_IMPLEMENTS:
        case META_MEMBER:
        case META_INLINE: {
            meta_entry *meta = (meta_entry *)entry;
            int prev = link_constant_offset_int(context, output_module, input_module, meta->prev, ok);
            int next = link_constant_offset_int(context, output_module, input_module, meta->next, ok);
            if (!rewrite_meta_constant(context, output_module, input_module, entry, new_offset, prev, next, ok)) {
                return 0;
            }
            break;
        }
        default:
            *ok = 0;
            return 0;
    }

    return new_offset;
}

static int rewrite_module_code(rxlink_build_context *context, rxlink_output_module *output_module,
                               const link_module_info *input_info) {
    module_file *input = input_info->module;
    bin_code *input_code;
    bin_code *output_code;
    size_t index;

    output_module->module = malloc(sizeof(module_file));
    if (!output_module->module) return 0;
    init_module(output_module->module);
    output_module->module->fromfile = 1;
    output_module->module->header.record_type = RXBIN_RECORD_MODULE_SHARED;
    output_module->module->name = strdup(input->name ? input->name : "");
    output_module->module->description = strdup(input->description ? input->description : "");
    if (!output_module->module->name || !output_module->module->description) return 0;
    output_module->module->header.name_size = strlen(output_module->module->name) + 1;
    output_module->module->header.description_size = strlen(output_module->module->description) + 1;
    output_module->module->header.instruction_size = input->header.instruction_size;
    output_module->module->header.constant_size = 0;
    output_module->module->header.constant_stored_size = 0;
    output_module->module->header.globals = input->header.globals;
    output_module->module->header.proc_head = -1;
    output_module->module->header.expose_head = -1;
    output_module->module->header.meta_head = -1;
    output_module->module->instructions = 0;
    output_module->module->constant = 0;
    output_module->module->shared_constant_pool = 0;

    if (input->header.instruction_size) {
        output_module->module->instructions = malloc(sizeof(bin_code) * input->header.instruction_size);
        if (!output_module->module->instructions) return 0;
        memcpy(output_module->module->instructions, input->instructions, sizeof(bin_code) * input->header.instruction_size);
    }

    {
        int ok = 1;
        output_module->module->header.proc_head =
                link_constant_offset_int(context, output_module, input, input->header.proc_head, &ok);
        output_module->module->header.expose_head =
                link_constant_offset_int(context, output_module, input, input->header.expose_head, &ok);
        output_module->module->header.meta_head =
                link_meta_chain(context, output_module, input, input->header.meta_head, &ok);
        if (!ok) return 0;
    }

    input_code = (bin_code *)input->instructions;
    output_code = (bin_code *)output_module->module->instructions;
    index = 0;
    while (index < input->header.instruction_size) {
        OperandType types[3];
        int operand_count;
        int operand_index;
        int ok = 1;

        operand_count = rxbin_get_operand_types(rxbin_opcode_format(input_code[index].instruction.opcode), types);
        for (operand_index = 0; operand_index < operand_count; operand_index++) {
            bin_code *operand = &output_code[index + (size_t)operand_index + 1];
            switch (types[operand_index]) {
                case OP_FUNC:
                case OP_FLOAT:
                case OP_STRING:
                case OP_DECIMAL:
                case OP_BINARY:
                    operand->index = link_constant_offset(context, output_module, input,
                                                          input_code[index + (size_t)operand_index + 1].index, &ok);
                    break;
                default:
                    break;
            }
            if (!ok) return 0;
        }
        index += (size_t)operand_count + 1;
    }

    return 1;
}

static int build_linked_modules(rxlink_build_context *context, module_list *modules, rxlink_output_list *outputs) {
    size_t i;

    for (i = 0; i < modules->count; i++) {
        rxlink_output_module output_module;
        memset(&output_module, 0, sizeof(output_module));
        if (!modules->items[i].selected || modules->items[i].omitted) continue;
        if (!rewrite_module_code(context, &output_module, &modules->items[i]) ||
            !output_list_append(outputs, &output_module)) {
            if (output_module.module) free_module(output_module.module);
            free(output_module.maps);
            return 0;
        }
    }

    return 1;
}

static int write_map_file(const module_list *modules, const link_config *config) {
    FILE *fp;
    size_t i;

    if (!config->map_path) return 1;

    fp = openfile(config->map_path, "", config->location, "w");
    if (!fp) {
        fprintf(stderr, "ERROR: opening map output %s\n", config->map_path);
        return 0;
    }

    fprintf(fp, "cREXX Link Map\n");
    fprintf(fp, "Selected Modules:\n");
    for (i = 0; i < modules->count; i++) {
        if (!modules->items[i].selected || modules->items[i].omitted) continue;
        fprintf(fp, "  %s from %s\n", modules->items[i].selector_name, modules->items[i].input_path);
        if (modules->items[i].unresolved_imports.count) {
            size_t j;
            fprintf(fp, "    unresolved:\n");
            for (j = 0; j < modules->items[i].unresolved_imports.count; j++) {
                fprintf(fp, "      %s\n", modules->items[i].unresolved_imports.items[j]);
            }
        }
    }

    fclose(fp);
    return 1;
}

static int write_linked_image(const link_config *config, rxlink_build_context *context, rxlink_output_list *outputs) {
    FILE *fp;
    module_file shared_pool_record;
    size_t i;
    const char *type_bin = has_any_extension(config->output_path) ? "" : "rxbin";

    fp = openfile(config->output_path, (char *)type_bin, config->location, "wb");
    if (!fp) {
        fprintf(stderr, "ERROR: opening output %s\n", config->output_path);
        return 0;
    }

    init_module(&shared_pool_record);
    shared_pool_record.header.record_type = RXBIN_RECORD_POOL_SHARED;
    shared_pool_record.header.name_size = 0;
    shared_pool_record.header.description_size = 0;
    shared_pool_record.header.instruction_size = 0;
    shared_pool_record.header.constant_size = context->shared_pool.size;
    shared_pool_record.header.globals = 0;
    shared_pool_record.header.proc_head = -1;
    shared_pool_record.header.expose_head = -1;
    shared_pool_record.header.meta_head = -1;
    shared_pool_record.name = "";
    shared_pool_record.description = "";
    shared_pool_record.instructions = 0;
    shared_pool_record.constant = context->shared_pool.data;

    if (write_module(&shared_pool_record, fp) != 0) {
        fclose(fp);
        return 0;
    }

    for (i = 0; i < outputs->count; i++) {
        if (write_module(outputs->items[i].module, fp) != 0) {
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return 1;
}

static void print_help(void) {
    printf("cREXX Linker\n");
    printf("Usage: rxlink [options] input_file [input_file ...]\n");
    printf("Options:\n");
    printf("  -o output_file  Linked output file\n");
    printf("  -c control_file Control file with INPUT/ROOT/INCLUDE/OMIT/OUTPUT/MAP/STRIP\n");
    printf("  -r root_member  Root module selector (may be repeated)\n");
    printf("  -m map_file     Write a simple link map\n");
    printf("  -l location     Working location for input/output resolution\n");
    printf("  -s              Strip source/file metadata from linked output\n");
    printf("  -i              Preserve inline-body metadata in linked output\n");
    printf("  -d              Debug mode\n");
    printf("  -h              Help\n");
}

int main(int argc, char *argv[]) {
    link_config config;
    module_list modules;
    rxlink_build_context build_context;
    rxlink_output_list outputs;
    char *control_path = 0;
    int argi;

    init_link_config(&config);
    memset(&modules, 0, sizeof(modules));
    build_context_init(&build_context);
    memset(&outputs, 0, sizeof(outputs));

    for (argi = 1; argi < argc && argv[argi][0] == '-'; argi++) {
        if (strlen(argv[argi]) != 2) {
            fprintf(stderr, "ERROR: invalid argument %s\n", argv[argi]);
            goto fail;
        }
        switch (toupper((unsigned char)argv[argi][1])) {
            case 'O':
                if (++argi >= argc || !set_single_path(&config.output_path, argv[argi])) goto fail;
                break;
            case 'C':
                if (++argi >= argc) goto fail;
                control_path = argv[argi];
                break;
            case 'R':
                if (++argi >= argc || !string_list_append(&config.roots, argv[argi])) goto fail;
                break;
            case 'M':
                if (++argi >= argc || !set_single_path(&config.map_path, argv[argi])) goto fail;
                break;
            case 'S':
                config.strip_source_metadata = 1;
                break;
            case 'I':
                config.strip_inline_metadata = 0;
                break;
            case 'L':
                if (++argi >= argc) goto fail;
                config.location = argv[argi];
                break;
            case 'D':
                config.debug_mode = 1;
                break;
            case 'H':
            case '?':
                print_help();
                free_link_config(&config);
                build_context_free(&build_context);
                return 0;
            default:
                fprintf(stderr, "ERROR: invalid option %s\n", argv[argi]);
                goto fail;
        }
    }

    if (control_path && !parse_control_file(&config, control_path)) goto fail;
    build_context.strip_source_metadata = config.strip_source_metadata;
    build_context.strip_inline_metadata = config.strip_inline_metadata;

    while (argi < argc) {
        if (!string_list_append(&config.inputs, argv[argi++])) goto fail;
    }

    if (!config.inputs.count) {
        fprintf(stderr, "ERROR: no input files\n");
        goto fail;
    }

    if (!config.output_path) {
        fprintf(stderr, "ERROR: no output file specified\n");
        goto fail;
    }

    if (!load_input_modules(&modules, &config)) goto fail;
    if (!select_modules(&modules, &config)) goto fail;
    if (!build_linked_modules(&build_context, &modules, &outputs)) goto fail;
    if (!outputs.count) {
        fprintf(stderr, "ERROR: no modules selected for output\n");
        goto fail;
    }
    if (!write_linked_image(&config, &build_context, &outputs)) goto fail;
    if (!write_map_file(&modules, &config)) goto fail;

    output_list_free(&outputs);
    build_context_free(&build_context);
    module_list_free(&modules);
    free_link_config(&config);
    return 0;

fail:
    output_list_free(&outputs);
    build_context_free(&build_context);
    module_list_free(&modules);
    free_link_config(&config);
    return 1;
}
