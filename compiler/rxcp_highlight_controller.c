#ifdef ENABLE_PARSER_MODE

#ifndef restrict
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#elif defined(__GNUC__) || defined(__clang__)
#define restrict __restrict
#elif defined(_MSC_VER)
#define restrict __restrict
#else
#define restrict
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#define rxcp_getcwd _getcwd
#else
#include <unistd.h>
#define rxcp_getcwd getcwd
#endif

/* Rename conflicting cREXX AST node types to avoid windows.h collision */
#define ERROR RXCP_ERROR
#define WARNING RXCP_WARNING
#define FLOAT RXCP_FLOAT
#define INTEGER RXCP_INTEGER
#define DECIMAL RXCP_DECIMAL
#define PATTERN RXCP_PATTERN
#define VOID RXCP_VOID

#include "rxcpmain.h"
#include "rxcp_highlight_controller.h"
#include "rxcp_source_tree.h"
#include "rxcp_val.h"
#include "rxcpbgmr.h"
#include "rxcp_exit.h"
#include "rxvml.h"
#include "utf.h"
#include "platform.h"

#undef ERROR
#undef WARNING
#undef FLOAT
#undef INTEGER
#undef DECIMAL
#undef PATTERN
#undef VOID

#include "dslsyntax_common.h"
#include "dslsyntax_parser.h"
#include "serialization.h"
#include "dslsyntax_log.h"

typedef struct HighlightTokenCursor {
    Context *context;
    Token *token;
    const char *last_source_ptr;
    size_t last_source_pos;
    struct HighlightSemanticTokenOwner *semantic_tokens;
    size_t semantic_token_count;
    size_t semantic_token_capacity;
    size_t semantic_token_index;
} HighlightTokenCursor;

typedef struct HighlightSemanticTokenOwner {
    Token *token;
    SourceNode *source_node;
    size_t depth;
} HighlightSemanticTokenOwner;

typedef struct HighlightWatchedPath {
    char *path;
    time_t mtime;
    long size;
    int exists;
} HighlightWatchedPath;

typedef struct HighlightRetainedState {
    Context *root;
    char *document_id;
    char *search_path_key;
    char *exit_module_name;
    HighlightWatchedPath *directories;
    size_t directory_count;
    HighlightWatchedPath *files;
    size_t file_count;
    size_t cached_import_file_count;
    unsigned long generation;
    unsigned long invalidation_count;
    unsigned long exit_warm_count;
    unsigned long import_inventory_warm_count;
    int exits_disabled;
    int exit_registry_ready;
} HighlightRetainedState;

typedef struct HighlightDocumentInfo {
    char *document_id;
    char *document_name;
    char *document_dir;
    char *current_working_directory;
    char *exe_path;
    char *exit_module_name;
    char *search_path_key;
    int disable_exits;
} HighlightDocumentInfo;

static HighlightRetainedState g_highlight_state;

static size_t highlight_count_registered_exits(Context *root) {
    ExitEntry *entry;
    size_t count;

    if (!root) return 0;

    count = 0;
    entry = (ExitEntry *)root->exit_registry;
    while (entry) {
        count++;
        entry = entry->next;
    }
    return count;
}

static int retain_context_buffer(Context *context, char *buffer) {
    char **new_buffers;

    if (!context || !buffer) return 0;

    new_buffers = realloc(context->extra_buffers, sizeof(char *) * (context->extra_buffers_count + 1));
    if (!new_buffers) {
        return 0;
    }

    context->extra_buffers = new_buffers;
    context->extra_buffers[context->extra_buffers_count++] = buffer;
    return 1;
}

static char *retain_context_string(Context *context, const char *value) {
    char *copy;

    if (!context || !value) return 0;

    copy = strdup(value);
    if (!copy) return 0;
    if (!retain_context_buffer(context, copy)) {
        free(copy);
        return 0;
    }
    return copy;
}

static char *highlight_getcwd(void) {
    char *buffer;

    buffer = malloc(MAXFILEPATH);
    if (!buffer) return 0;
    if (!rxcp_getcwd(buffer, MAXFILEPATH)) {
        free(buffer);
        return 0;
    }
    return buffer;
}

static void free_highlight_watched_paths(HighlightWatchedPath *paths, size_t count) {
    size_t i;

    if (!paths) return;
    for (i = 0; i < count; i++) {
        if (paths[i].path) free(paths[i].path);
    }
    free(paths);
}

static void highlight_clear_retained_state(int count_as_invalidation) {
    if (count_as_invalidation && g_highlight_state.root) {
        g_highlight_state.invalidation_count++;
    }

    if (g_highlight_state.root) {
        fre_cntx(g_highlight_state.root);
        g_highlight_state.root = 0;
    }

    if (g_highlight_state.document_id) free(g_highlight_state.document_id);
    if (g_highlight_state.search_path_key) free(g_highlight_state.search_path_key);
    if (g_highlight_state.exit_module_name) free(g_highlight_state.exit_module_name);

    free_highlight_watched_paths(g_highlight_state.directories, g_highlight_state.directory_count);
    free_highlight_watched_paths(g_highlight_state.files, g_highlight_state.file_count);

    g_highlight_state.document_id = 0;
    g_highlight_state.search_path_key = 0;
    g_highlight_state.exit_module_name = 0;
    g_highlight_state.directories = 0;
    g_highlight_state.directory_count = 0;
    g_highlight_state.files = 0;
    g_highlight_state.file_count = 0;
    g_highlight_state.cached_import_file_count = 0;
    g_highlight_state.exits_disabled = 0;
    g_highlight_state.exit_registry_ready = 0;
}

void rxcp_highlight_controller_reset_cache(void) {
    highlight_clear_retained_state(0);
    memset(&g_highlight_state, 0, sizeof(g_highlight_state));
}

void rxcp_highlight_controller_get_cache_stats(RXCPHighlightCacheStats *stats) {
    if (!stats) return;

    stats->generation = g_highlight_state.generation;
    stats->invalidation_count = g_highlight_state.invalidation_count;
    stats->exit_warm_count = g_highlight_state.exit_warm_count;
    stats->import_inventory_warm_count = g_highlight_state.import_inventory_warm_count;
    stats->cached_import_file_count = g_highlight_state.cached_import_file_count;
    stats->cached_import_function_count = g_highlight_state.root ? g_highlight_state.root->importable_function_count : 0;
    stats->cached_exit_primary_count = highlight_count_registered_exits(g_highlight_state.root);
    stats->watched_directory_count = g_highlight_state.directory_count;
    stats->watched_file_count = g_highlight_state.file_count;
    stats->exits_disabled = g_highlight_state.exits_disabled;
}

static int highlight_capture_path_state(HighlightWatchedPath *entry, const char *path) {
    struct stat st;

    if (!entry || !path) return 0;

    memset(entry, 0, sizeof(*entry));
    entry->path = strdup(path);
    if (!entry->path) return 0;

    if (stat(path, &st) == 0) {
        entry->exists = 1;
        entry->mtime = st.st_mtime;
        entry->size = (long)st.st_size;
    }
    return 1;
}

static int highlight_add_watched_path(HighlightWatchedPath **paths,
                                      size_t *count,
                                      const char *path) {
    HighlightWatchedPath *new_paths;
    HighlightWatchedPath snapshot;
    size_t i;

    if (!paths || !count || !path || !path[0]) return 0;

    for (i = 0; i < *count; i++) {
        if ((*paths)[i].path && strcmp((*paths)[i].path, path) == 0) return 1;
    }

    if (!highlight_capture_path_state(&snapshot, path)) return 0;

    new_paths = realloc(*paths, sizeof(HighlightWatchedPath) * (*count + 1));
    if (!new_paths) {
        if (snapshot.path) free(snapshot.path);
        return 0;
    }

    *paths = new_paths;
    (*paths)[*count] = snapshot;
    (*count)++;
    return 1;
}

static int highlight_path_changed(HighlightWatchedPath *entry) {
    struct stat st;

    if (!entry || !entry->path) return 0;

    if (stat(entry->path, &st) != 0) {
        return entry->exists;
    }

    if (!entry->exists) return 1;
    if (entry->mtime != st.st_mtime) return 1;
    if (entry->size != (long)st.st_size) return 1;
    return 0;
}

static int highlight_paths_stale(void) {
    size_t i;

    for (i = 0; i < g_highlight_state.directory_count; i++) {
        if (highlight_path_changed(&g_highlight_state.directories[i])) return 1;
    }

    for (i = 0; i < g_highlight_state.file_count; i++) {
        if (highlight_path_changed(&g_highlight_state.files[i])) return 1;
    }

    return 0;
}

static char *highlight_join_path(const char *dir, const char *name) {
    size_t dir_len;
    size_t name_len;
    size_t total_len;
    char *full_path;
    int needs_sep;

    if (!name) return 0;
    if (!dir || !dir[0]) return strdup(name);

    dir_len = strlen(dir);
    name_len = strlen(name);
    needs_sep = dir_len > 0 && dir[dir_len - 1] != '/' && dir[dir_len - 1] != '\\';
    total_len = dir_len + (needs_sep ? 1 : 0) + name_len + 1;

    full_path = malloc(total_len);
    if (!full_path) return 0;

    memcpy(full_path, dir, dir_len);
    if (needs_sep) full_path[dir_len++] = '/';
    memcpy(full_path + dir_len, name, name_len);
    full_path[dir_len + name_len] = 0;
    return full_path;
}

static char *highlight_module_filename(const char *module_name) {
    if (!module_name || !module_name[0]) return 0;
    if (has_any_extension(module_name)) return strdup(module_name);
    return mprintf("%s.rxbin", module_name);
}

static char *highlight_resolve_module_path(Context *context, const char *module_name) {
    char *module_file;
    char *candidate;
    size_t i;
    struct stat st;

    if (!context) return 0;

    module_file = highlight_module_filename(module_name);
    if (!module_file) return 0;

    if (context->location) {
        candidate = highlight_join_path(context->location, module_file);
        if (candidate && stat(candidate, &st) == 0) {
            free(module_file);
            return candidate;
        }
        if (candidate) free(candidate);
    }

    if (context->import_locations) {
        for (i = 0; context->import_locations[i]; i++) {
            candidate = highlight_join_path(context->import_locations[i], module_file);
            if (candidate && stat(candidate, &st) == 0) {
                free(module_file);
                return candidate;
            }
            if (candidate) free(candidate);
        }
    }

    candidate = highlight_join_path(0, module_file);
    if (candidate && stat(candidate, &st) == 0) {
        free(module_file);
        return candidate;
    }
    if (candidate) free(candidate);

    free(module_file);
    return 0;
}

static size_t highlight_count_importable_files(importable_file **file_list) {
    size_t count;

    count = 0;
    if (!file_list) return 0;
    while (file_list[count]) count++;
    return count;
}

static void highlight_record_root_snapshots(Context *root, const HighlightDocumentInfo *info) {
    importable_file **file_list;
    size_t i;
    char *module_path;
    char *full_path;

    free_highlight_watched_paths(g_highlight_state.directories, g_highlight_state.directory_count);
    free_highlight_watched_paths(g_highlight_state.files, g_highlight_state.file_count);
    g_highlight_state.directories = 0;
    g_highlight_state.directory_count = 0;
    g_highlight_state.files = 0;
    g_highlight_state.file_count = 0;

    if (!root) return;

    highlight_add_watched_path(&g_highlight_state.directories, &g_highlight_state.directory_count, root->location);
    if (root->import_locations) {
        for (i = 0; root->import_locations[i]; i++) {
            highlight_add_watched_path(&g_highlight_state.directories,
                                       &g_highlight_state.directory_count,
                                       root->import_locations[i]);
        }
    }

    file_list = root->importable_file_list;
    while (file_list && *file_list) {
        full_path = highlight_join_path((*file_list)->location, (*file_list)->name);
        if (full_path) {
            highlight_add_watched_path(&g_highlight_state.files, &g_highlight_state.file_count, full_path);
            free(full_path);
        }
        file_list++;
    }

    module_path = highlight_resolve_module_path(root, "library");
    if (module_path) {
        highlight_add_watched_path(&g_highlight_state.files, &g_highlight_state.file_count, module_path);
        free(module_path);
    }

    module_path = highlight_resolve_module_path(root, info ? info->exit_module_name : 0);
    if (module_path) {
        highlight_add_watched_path(&g_highlight_state.files, &g_highlight_state.file_count, module_path);
        free(module_path);
    }

    g_highlight_state.cached_import_file_count = highlight_count_importable_files(root->importable_file_list);
}

static void highlight_free_document_info(HighlightDocumentInfo *info) {
    if (!info) return;
    if (info->document_id) free(info->document_id);
    if (info->document_name) free(info->document_name);
    if (info->document_dir) free(info->document_dir);
    if (info->current_working_directory) free(info->current_working_directory);
    if (info->exe_path) free(info->exe_path);
    if (info->exit_module_name) free(info->exit_module_name);
    if (info->search_path_key) free(info->search_path_key);
    memset(info, 0, sizeof(*info));
}

static int highlight_build_document_info(CodeBuffer *cb, HighlightDocumentInfo *info) {
    const char *doc_id;

    if (!info) return 0;
    memset(info, 0, sizeof(*info));

    doc_id = (cb && cb->unique_document_id && cb->unique_document_id[0]) ? cb->unique_document_id : "dsl_buffer.rexx";
    info->document_id = strdup(doc_id);
    info->document_name = strdup(filename(doc_id));
    info->document_dir = file_dir(doc_id);
    info->current_working_directory = highlight_getcwd();
    info->exe_path = exepath();
    info->exit_module_name = strdup(getenv("RXCP_EXIT_MODULE") ? getenv("RXCP_EXIT_MODULE") : "rxcexits");
    info->disable_exits = getenv("RXCP_DISABLE_EXIT") != 0;

    if (!info->document_dir) info->document_dir = strdup(".");
    if (!info->current_working_directory) info->current_working_directory = strdup(".");
    if (!info->exe_path) info->exe_path = strdup(".");

    if (!info->document_id || !info->document_name || !info->document_dir ||
        !info->current_working_directory || !info->exe_path || !info->exit_module_name) {
        highlight_free_document_info(info);
        return 0;
    }

    info->search_path_key = mprintf("%s|%s|%s",
                                    info->document_dir,
                                    info->current_working_directory,
                                    info->exe_path);
    if (!info->search_path_key) {
        highlight_free_document_info(info);
        return 0;
    }

    return 1;
}

static int highlight_configure_root_context(Context *root, const HighlightDocumentInfo *info) {
    char **import_locations;
    char *cwd;
    char *exe_path;
    size_t count;

    if (!root || !info) return 0;

    root->master_context = root;
    root->debug_mode = 0;
    root->stop_after_parse = 1;
    root->optimise = 0;
    root->level = LEVELB;
    root->disable_exits = info->disable_exits;
    root->location = retain_context_string(root, info->document_dir);
    root->file_name = retain_context_string(root, info->document_name);

    count = 0;
    cwd = retain_context_string(root, info->current_working_directory);
    exe_path = retain_context_string(root, info->exe_path);
    if (cwd) count++;
    if (exe_path && (!cwd || strcmp(exe_path, cwd) != 0)) count++;

    import_locations = malloc(sizeof(char *) * (count + 1));
    if (!import_locations) return 0;

    count = 0;
    if (cwd) import_locations[count++] = cwd;
    if (exe_path && (!cwd || strcmp(exe_path, cwd) != 0)) import_locations[count++] = exe_path;
    import_locations[count] = 0;
    root->import_locations = import_locations;

    if (root->file_name) {
        root->loading_files = malloc(sizeof(char *));
        if (root->loading_files) {
            root->loading_files[0] = strdup(root->file_name);
            if (root->loading_files[0]) {
                root->loading_files_count = 1;
            } else {
                free(root->loading_files);
                root->loading_files = 0;
            }
        }
    }

    return 1;
}

static int highlight_cache_needs_reset(const HighlightDocumentInfo *info) {
    if (!g_highlight_state.root) return 1;
    if (!info) return 1;
    if (!g_highlight_state.document_id || strcmp(g_highlight_state.document_id, info->document_id) != 0) return 1;
    if (!g_highlight_state.search_path_key || strcmp(g_highlight_state.search_path_key, info->search_path_key) != 0) return 1;
    if (!g_highlight_state.exit_module_name ||
        strcmp(g_highlight_state.exit_module_name, info->exit_module_name) != 0) return 1;
    if (g_highlight_state.exits_disabled != info->disable_exits) return 1;
    if (highlight_paths_stale()) return 1;
    return 0;
}

static Context *highlight_prepare_root_cache(CodeBuffer *cb) {
    HighlightDocumentInfo info;
    Context *root;

    if (!highlight_build_document_info(cb, &info)) return 0;

    if (highlight_cache_needs_reset(&info)) {
        highlight_clear_retained_state(g_highlight_state.root != 0);

        root = cntx_f();
        if (!root || !highlight_configure_root_context(root, &info)) {
            if (root) fre_cntx(root);
            highlight_free_document_info(&info);
            return 0;
        }

        g_highlight_state.root = root;
        g_highlight_state.document_id = strdup(info.document_id);
        g_highlight_state.search_path_key = strdup(info.search_path_key);
        g_highlight_state.exit_module_name = strdup(info.exit_module_name);
        g_highlight_state.exits_disabled = info.disable_exits;
        g_highlight_state.exit_registry_ready = 0;
        g_highlight_state.generation++;
    }

    root = g_highlight_state.root;

    if (root && !root->importable_file_list) {
        root->importable_file_list = rxfl_lst(root);
        g_highlight_state.import_inventory_warm_count++;
    }

    if (root && !root->disable_exits && !g_highlight_state.exit_registry_ready) {
        rxcp_init_exits(root);
        g_highlight_state.exit_registry_ready = 1;
        g_highlight_state.exit_warm_count++;
    }

    highlight_record_root_snapshots(root, &info);
    highlight_free_document_info(&info);
    return root;
}

static void highlight_release_parse_exit_objects(Context *context, ASTNode *node) {
    rxvml_context *bridge;

    if (!context || !node || !context->master_context || !context->master_context->rxvml_bridge) return;

    bridge = (rxvml_context *)context->master_context->rxvml_bridge;
    while (node) {
        if (node->exit_obj_reg >= 0) {
            rxvml_reg_free(bridge, node->exit_obj_reg);
            node->exit_obj_reg = -1;
        }
        if (node->child) highlight_release_parse_exit_objects(context, node->child);
        node = node->sibling;
    }
}

static void configure_parser_import_locations(Context *context) {
    char *exe_path;
    char *combined_locations;
    char **import_locations;
    size_t length;
    size_t i;
    size_t index;

    if (!context || context->import_locations) return;

    exe_path = exepath();
    if (!exe_path) return;

    length = strlen(exe_path) + 4;
    combined_locations = malloc(length);
    if (!combined_locations) {
        free(exe_path);
        return;
    }
    snprintf(combined_locations, length, ".;%s", exe_path);
    free(exe_path);

    import_locations = malloc(sizeof(char *) * 3);
    if (!import_locations) {
        free(combined_locations);
        return;
    }

    index = 0;
    import_locations[index++] = combined_locations;
    for (i = 0; combined_locations[i]; i++) {
        if (combined_locations[i] == ';') {
            combined_locations[i] = 0;
            import_locations[index++] = combined_locations + i + 1;
        }
    }
    import_locations[index] = 0;

    if (!retain_context_buffer(context, combined_locations)) {
        free(import_locations);
        free(combined_locations);
        return;
    }

    context->import_locations = import_locations;
}

static CB_NodeType map_c_token_to_cb_type(int token_type) {
    switch (token_type) {
        case TK_UNKNOWN:
        case TK_BADCOMMENT: return LEXER_UNKNOWN;
        case TK_EOC:
        case TK_EOL:
        case TK_EOS: return LEXER_STATEMENT_SEPARATOR;
        case TK_MINUSMINUS: return LEXER_COMMENT;
        case TK_VAR_SYMBOL:
        case TK_CLASS_STEM:
        case TK_STEM:
        case TK_STEMVAR:
        case TK_STEMSTRING:
        case TK_STEMNOVAL:
        case TK_STEMINT: return LEXER_IDENTIFIER;
        case TK_CLASS_TYPE: return LEXER_TYPE_IDENTIFIER;
        case TK_LABEL:
        case TK_MULT_LABEL: return LEXER_FUNCTION_IDENTIFIER;
        case TK_IMPORT:
        case TK_NAMESPACE:
        case TK_OPTIONS: return LEXER_PREPROCESSOR;
        case TK_EXIT_PRIMARY:
        case TK_EXIT_TOKEN: return LEXER_KEYWORD;
        case TK_STRING: return LEXER_STRING_LITERAL;
        case TK_DECIMAL:
        case TK_INTEGER:
        case TK_FLOAT: return LEXER_NUMBER_LITERAL;
        case TK_CONCAT: return LEXER_OPERATOR;
        case TK_PLUS:
        case TK_MINUS:
        case TK_HIGH_PRIORITY_MINUS:
        case TK_MULT:
        case TK_DIV:
        case TK_MOD:
        case TK_IDIV:
        case TK_POWER_L:
        case TK_POWER_R: return LEXER_OPERATOR_ARITHMETIC;
        case TK_EQUAL: return LEXER_OPERATOR_ASSIGN;
        case TK_NEQ:
        case TK_GT:
        case TK_LT:
        case TK_GTE:
        case TK_LTE:
        case TK_S_EQ:
        case TK_S_NEQ:
        case TK_S_GT:
        case TK_S_LT:
        case TK_S_GTE:
        case TK_S_LTE:
        case TK_AND:
        case TK_OR:
        case TK_NOT: return LEXER_OPERATOR_LOGICAL;
        case TK_OPEN_BRACKET:
        case TK_OPEN_SBRACKET: return LEXER_LH_EXPR;
        case TK_CLOSE_BRACKET:
        case TK_CLOSE_SBRACKET: return LEXER_RH_EXPR;
        case TK_COMMA:
        case TK_DOT: return LEXER_SEPARATOR;
        default:
            return LEXER_KEYWORD;
    }
}

static int token_span_utf8(Context *context, Token *token, size_t *pos, size_t *len) {
    size_t byte_offset;

    if (!context || !token || !pos || !len) return 0;
    if (!token->token_string || token->length <= 0) return 0;
    if (token->token_string < context->buff_start) return 0;

    byte_offset = (size_t)(token->token_string - context->buff_start);
    *pos = utf8nlen(context->buff_start, byte_offset);
    *len = utf8nlen(token->token_string, token->length);
    return *len > 0;
}

static int source_node_span_utf8(Context *context, SourceNode *node, size_t *pos, size_t *len) {
    size_t start_offset;
    size_t byte_length;

    if (!context || !node || !pos || !len) return 0;
    if (node->source_start && node->source_end &&
        node->source_start >= context->buff_start &&
        node->source_end >= node->source_start) {
        start_offset = (size_t)(node->source_start - context->buff_start);
        byte_length = (size_t)(node->source_end - node->source_start) + 1;
        *pos = utf8nlen(context->buff_start, start_offset);
        *len = utf8nlen(node->source_start, byte_length);
        return *len > 0;
    }

    if (node->token_start && node->token_end &&
        node->token_start->token_string && node->token_end->token_string &&
        node->token_start->token_string >= context->buff_start &&
        node->token_end->token_string >= node->token_start->token_string) {
        start_offset = (size_t)(node->token_start->token_string - context->buff_start);
        byte_length = (size_t)(node->token_end->token_string - node->token_start->token_string) +
                      (size_t)node->token_end->length;
        *pos = utf8nlen(context->buff_start, start_offset);
        *len = utf8nlen(node->token_start->token_string, byte_length);
        return *len > 0;
    }

    if (token_span_utf8(context, node->token, pos, len)) return 1;
    if (node->parent) return source_node_span_utf8(context, node->parent, pos, len);
    return 0;
}

static int source_diagnostic_span_utf8(Context *context, SourceDiagnostic *diag, size_t *pos, size_t *len) {
    size_t start_offset;
    size_t byte_length;

    if (!context || !diag || !pos || !len) return 0;
    if (diag->source_start && diag->source_end &&
        diag->source_start >= context->buff_start &&
        diag->source_end >= diag->source_start) {
        start_offset = (size_t)(diag->source_start - context->buff_start);
        byte_length = (size_t)(diag->source_end - diag->source_start) + 1;
        *pos = utf8nlen(context->buff_start, start_offset);
        *len = utf8nlen(diag->source_start, byte_length);
        return *len > 0;
    }

    if (diag->owner) return source_node_span_utf8(context, diag->owner, pos, len);
    return 0;
}

static int ast_node_span_utf8(Context *context, ASTNode *node, size_t *pos, size_t *len) {
    size_t start_offset;
    size_t byte_length;

    if (!context || !node || !pos || !len) return 0;
    if (node->source_start && node->source_end &&
        node->source_start >= context->buff_start &&
        node->source_end >= node->source_start) {
        start_offset = (size_t)(node->source_start - context->buff_start);
        byte_length = (size_t)(node->source_end - node->source_start) + 1;
        *pos = utf8nlen(context->buff_start, start_offset);
        *len = utf8nlen(node->source_start, byte_length);
        return *len > 0;
    }

    if (node->token_start && node->token_end &&
        node->token_start->token_string && node->token_end->token_string &&
        node->token_start->token_string >= context->buff_start &&
        node->token_end->token_string >= node->token_start->token_string) {
        start_offset = (size_t)(node->token_start->token_string - context->buff_start);
        byte_length = (size_t)(node->token_end->token_string - node->token_start->token_string) +
                      (size_t)node->token_end->length;
        *pos = utf8nlen(context->buff_start, start_offset);
        *len = utf8nlen(node->token_start->token_string, byte_length);
        return *len > 0;
    }

    if (token_span_utf8(context, node->token, pos, len)) return 1;
    if (node->parent) return ast_node_span_utf8(context, node->parent, pos, len);
    return 0;
}

static int highlight_has_source_errors(Context *context) {
    SourceDiagnostic *diag;

    if (!context) return 0;

    diag = context->source_diagnostics_list;
    while (diag) {
        if (diag->severity == SOURCE_DIAG_ERROR) return 1;
        diag = diag->next_in_context;
    }
    return 0;
}

static int highlight_semantic_rank(SourceNode *node) {
    SourceSemanticInfo *semantics;

    if (!node) return 0;
    semantics = node->semantics;
    if (!semantics) return node->token ? 1 : 0;

    if (semantics->identifier_id > 0) return 4;
    if (semantics->identifier_kind != SOURCE_SEMANTIC_NONE) return 3;
    if (semantics->symbol_type != UNKNOWN_SYMBOL) return 2;
    return node->token ? 1 : 0;
}

static int highlight_semantic_owner_compare(const void *lhs, const void *rhs) {
    const HighlightSemanticTokenOwner *left = (const HighlightSemanticTokenOwner *)lhs;
    const HighlightSemanticTokenOwner *right = (const HighlightSemanticTokenOwner *)rhs;

    if (left->token->token_number < right->token->token_number) return -1;
    if (left->token->token_number > right->token->token_number) return 1;
    if (left->depth < right->depth) return -1;
    if (left->depth > right->depth) return 1;
    return 0;
}

static void highlight_collect_semantic_tokens(HighlightTokenCursor *cursor,
                                              SourceNode *node,
                                              size_t depth) {
    HighlightSemanticTokenOwner *new_tokens;
    size_t new_capacity;
    size_t i;
    int new_rank;
    int old_rank;

    while (node) {
        if (node->token) {
            for (i = 0; i < cursor->semantic_token_count; i++) {
                if (cursor->semantic_tokens[i].token == node->token) {
                    new_rank = highlight_semantic_rank(node);
                    old_rank = highlight_semantic_rank(cursor->semantic_tokens[i].source_node);
                    if (new_rank > old_rank ||
                        (new_rank == old_rank && depth >= cursor->semantic_tokens[i].depth)) {
                        cursor->semantic_tokens[i].source_node = node;
                        cursor->semantic_tokens[i].depth = depth;
                    }
                    break;
                }
            }

            if (i == cursor->semantic_token_count) {
                if (cursor->semantic_token_count == cursor->semantic_token_capacity) {
                    new_capacity = cursor->semantic_token_capacity ? cursor->semantic_token_capacity * 2 : 64;
                    new_tokens = realloc(cursor->semantic_tokens,
                                         sizeof(HighlightSemanticTokenOwner) * new_capacity);
                    if (!new_tokens) return;
                    cursor->semantic_tokens = new_tokens;
                    cursor->semantic_token_capacity = new_capacity;
                }

                cursor->semantic_tokens[cursor->semantic_token_count].token = node->token;
                cursor->semantic_tokens[cursor->semantic_token_count].source_node = node;
                cursor->semantic_tokens[cursor->semantic_token_count].depth = depth;
                cursor->semantic_token_count++;
            }
        }

        if (node->child) highlight_collect_semantic_tokens(cursor, node->child, depth + 1);
        node = node->sibling;
    }
}

static void highlight_prepare_semantic_tokens(HighlightTokenCursor *cursor, SourceNode *root) {
    if (!cursor) return;

    cursor->semantic_token_count = 0;
    cursor->semantic_token_capacity = 0;
    cursor->semantic_token_index = 0;
    cursor->semantic_tokens = 0;

    if (!root) return;

    highlight_collect_semantic_tokens(cursor, root, 0);
    if (cursor->semantic_token_count > 1) {
        qsort(cursor->semantic_tokens,
              cursor->semantic_token_count,
              sizeof(HighlightSemanticTokenOwner),
              highlight_semantic_owner_compare);
    }
}

static void highlight_free_semantic_tokens(HighlightTokenCursor *cursor) {
    if (!cursor) return;
    if (cursor->semantic_tokens) free(cursor->semantic_tokens);
    cursor->semantic_tokens = 0;
    cursor->semantic_token_count = 0;
    cursor->semantic_token_capacity = 0;
    cursor->semantic_token_index = 0;
}

static SourceNode *highlight_source_node_for_token(HighlightTokenCursor *cursor, Token *token) {
    size_t index;

    if (!cursor || !token || !cursor->semantic_tokens) return 0;

    index = cursor->semantic_token_index;
    if (index < cursor->semantic_token_count &&
        cursor->semantic_tokens[index].token->token_number > token->token_number) {
        index = 0;
    }
    while (index < cursor->semantic_token_count &&
           cursor->semantic_tokens[index].token->token_number < token->token_number) {
        index++;
    }
    cursor->semantic_token_index = index;

    while (index < cursor->semantic_token_count &&
           cursor->semantic_tokens[index].token->token_number == token->token_number) {
        if (cursor->semantic_tokens[index].token == token) {
            cursor->semantic_token_index = index;
            return cursor->semantic_tokens[index].source_node;
        }
        index++;
    }

    return 0;
}

static CB_NodeType highlight_identifier_type_from_source(SourceNode *node, int fallback_type) {
    SourceSemanticInfo *semantics;

    if (!node) return (CB_NodeType)fallback_type;
    semantics = node->semantics;
    if (!semantics) return (CB_NodeType)fallback_type;

    switch (semantics->identifier_kind) {
        case SOURCE_SEMANTIC_FUNCTION: return LEXER_FUNCTION_IDENTIFIER;
        case SOURCE_SEMANTIC_TYPE: return LEXER_TYPE_IDENTIFIER;
        case SOURCE_SEMANTIC_CONSTANT: return LEXER_CONSTANT_IDENTIFIER;
        default: return (CB_NodeType)fallback_type;
    }
}

static CB_Node highlight_cb_node_for_token(HighlightTokenCursor *cursor,
                                           Token *token,
                                           size_t pos,
                                           size_t len) {
    CB_Node node;
    SourceNode *source_node;

    node = cb_create_node(map_c_token_to_cb_type(token->token_type), pos, len);
    source_node = highlight_source_node_for_token(cursor, token);

    switch (token->token_type) {
        case TK_UNKNOWN:
        case TK_VAR_SYMBOL:
        case TK_STEM:
        case TK_STEMSTRING:
        case TK_CLASS_TYPE:
        case TK_LABEL:
        case TK_MULT_LABEL:
            node.type = highlight_identifier_type_from_source(source_node, node.type);
            if (source_node && source_node->semantics) node.identifier_id = source_node->semantics->identifier_id;
            break;
        case TK_STEMVAR:
            if (source_node && source_node->semantics) node.identifier_id = source_node->semantics->identifier_id;
            break;
        default:
            break;
    }

    return node;
}

static int source_container_type(SourceNode *node, CB_NodeType *type) {
    if (!node || !type) return 0;

    switch (node->node_type) {
        case CLASS_DEF:
            *type = PARSE_TREE_STRUCTURE;
            return 1;
        case PROCEDURE:
        case METHOD:
        case FACTORY:
            *type = PARSE_TREE_FUNCTION;
            return 1;
        case INSTRUCTIONS:
        case DO:
        case SELECT:
        case WHEN:
        case OTHERWISE:
            *type = PARSE_TREE_CODEBLOCK;
            return 1;
        case IF:
        case ASSIGN:
        case CALL:
        case RETURN:
        case EXIT:
        case ADDRESS:
        case IMPLICIT_CMD:
        case SAY:
        case PULL:
        case PARSE:
            *type = PARSE_TREE_STATEMENT;
            return 1;
        case BLOCK_EXPR:
        case OP_ADD:
        case OP_MINUS:
        case OP_MULT:
        case OP_DIV:
        case OP_IDIV:
        case OP_MOD:
        case OP_POWER:
        case OP_CONCAT:
        case OP_SCONCAT:
        case OP_AND:
        case OP_OR:
        case OP_COMPARE_EQUAL:
        case OP_COMPARE_NEQ:
        case OP_COMPARE_GT:
        case OP_COMPARE_LT:
        case OP_COMPARE_GTE:
        case OP_COMPARE_LTE:
        case OP_COMPARE_S_EQ:
        case OP_COMPARE_S_NEQ:
        case OP_COMPARE_S_GT:
        case OP_COMPARE_S_LT:
        case OP_COMPARE_S_GTE:
        case OP_COMPARE_S_LTE:
            *type = PARSE_TREE_EXPR;
            return 1;
        default:
            return 0;
    }
}

static void emit_projected_token(CB_ParseTree *tb,
                                 HighlightTokenCursor *cursor,
                                 Token *token,
                                 size_t pos,
                                 size_t len) {
    CB_Node node;
    int has_embedded_separator;

    if (!tb || !token || len == 0) return;
    has_embedded_separator = token->token_string && token->token_string[0] == '.';
    node = highlight_cb_node_for_token(cursor, token, pos, len);

    switch (token->token_type) {
        case TK_STEMVAR:
            if (has_embedded_separator && len > 1) {
                cb_add_child_node(tb, cb_create_node(LEXER_SEPARATOR, pos, 1));
                node.pos = pos + 1;
                node.length = len - 1;
                node.type = highlight_identifier_type_from_source(highlight_source_node_for_token(cursor, token),
                                                                  LEXER_IDENTIFIER);
                cb_add_child_node(tb, node);
                return;
            }
            node.type = highlight_identifier_type_from_source(highlight_source_node_for_token(cursor, token),
                                                              LEXER_IDENTIFIER);
            cb_add_child_node(tb, node);
            return;
        case TK_STEMINT:
            if (has_embedded_separator && len > 1) {
                cb_add_child_node(tb, cb_create_node(LEXER_SEPARATOR, pos, 1));
                cb_add_child_node(tb, cb_create_node(LEXER_NUMBER_LITERAL, pos + 1, len - 1));
                return;
            }
            cb_add_child_node(tb, cb_create_node(LEXER_NUMBER_LITERAL, pos, len));
            return;
        case TK_STEMSTRING:
            if (has_embedded_separator && len > 1) {
                cb_add_child_node(tb, cb_create_node(LEXER_SEPARATOR, pos, 1));
                node.pos = pos + 1;
                node.length = len - 1;
                node.type = highlight_identifier_type_from_source(highlight_source_node_for_token(cursor, token),
                                                                  LEXER_IDENTIFIER);
                cb_add_child_node(tb, node);
                return;
            }
            node.type = highlight_identifier_type_from_source(highlight_source_node_for_token(cursor, token),
                                                              LEXER_IDENTIFIER);
            cb_add_child_node(tb, node);
            return;
        case TK_STEMNOVAL:
            cb_add_child_node(tb, cb_create_node(LEXER_SEPARATOR, pos, len));
            return;
    }

    cb_add_child_node(tb, node);
}

static void emit_tokens_until(CB_ParseTree *tb, HighlightTokenCursor *cursor, size_t limit_pos) {
    size_t pos;
    size_t len;

    while (cursor && cursor->token) {
        if (!token_span_utf8(cursor->context, cursor->token, &pos, &len)) {
            cursor->token = cursor->token->token_next;
            continue;
        }
        if (pos >= limit_pos) break;
        if (pos + len > limit_pos) break;

        emit_projected_token(tb, cursor, cursor->token, pos, len);
        cursor->token = cursor->token->token_next;
    }
}

static void emit_source_projection(CB_ParseTree *tb,
                                   Context *context,
                                   SourceNode *node,
                                   HighlightTokenCursor *cursor) {
    SourceNode *child;
    size_t child_pos;
    size_t child_len;
    CB_NodeType child_type;
    CB_Node child_node;

    child = node;
    while (child) {
        if (child->node_type != RXCP_ERROR && child->node_type != RXCP_WARNING &&
            source_container_type(child, &child_type) &&
            source_node_span_utf8(context, child, &child_pos, &child_len)) {
            emit_tokens_until(tb, cursor, child_pos);
            child_node = cb_create_node(child_type, child_pos, child_len);
            cb_add_child_node(tb, child_node);
            cb_set_current_parent_to_last_node(tb);
            emit_source_projection(tb, context, child->child, cursor);
            emit_tokens_until(tb, cursor, child_pos + child_len);
            cb_set_current_parent_to_grandparent(tb);
        } else if (child->child) {
            emit_source_projection(tb, context, child->child, cursor);
        }
        child = child->sibling;
    }
}

static char *format_source_diagnostic_message(SourceDiagnostic *diag) {
    char *message;

    if (!diag) return strdup("Syntax Error");
    if (!diag->is_internal) {
        return strdup(diag->message ? diag->message : "Syntax Error");
    }

    if (diag->severity == SOURCE_DIAG_WARNING) {
        message = mprintf("Internal generated-code warning: %s",
                          diag->message ? diag->message : "Warning");
    } else {
        message = mprintf("Internal generated-code error: %s",
                          diag->message ? diag->message : "Syntax Error");
    }
    return message;
}

static char *format_ast_diagnostic_message(ASTNode *diag) {
    char *message;

    if (!diag) return strdup("Syntax Error");
    if (!diag->is_internal_diagnostic) {
        return strdup(diag->node_string ? diag->node_string : "Syntax Error");
    }

    if (diag->node_type == RXCP_WARNING) {
        message = mprintf("Internal generated-code warning: %s",
                          diag->node_string ? diag->node_string : "Warning");
    } else {
        message = mprintf("Internal generated-code error: %s",
                          diag->node_string ? diag->node_string : "Syntax Error");
    }
    return message;
}

typedef struct HighlightDiagnosticSelection {
    SourceDiagnostic *source_diag;
    ASTNode *ast_diag;
    CB_Severity severity;
    size_t pos;
    size_t len;
} HighlightDiagnosticSelection;

static int highlight_spans_overlap(size_t left_pos,
                                   size_t left_len,
                                   size_t right_pos,
                                   size_t right_len) {
    size_t left_end;
    size_t right_end;

    if (left_len == 0 || right_len == 0) return 0;

    left_end = left_pos + left_len;
    right_end = right_pos + right_len;
    return left_pos < right_end && right_pos < left_end;
}

static int highlight_should_replace_diagnostic_match(const HighlightDiagnosticSelection *current,
                                                     CB_Severity severity,
                                                     size_t pos,
                                                     size_t len) {
    if (!current || (!current->source_diag && !current->ast_diag)) return 1;
    if (severity > current->severity) return 1;
    if (severity < current->severity) return 0;
    if (len < current->len) return 1;
    if (len > current->len) return 0;
    return pos < current->pos;
}

static int highlight_select_diagnostic_for_leaf(Context *context,
                                                CB_Node *node,
                                                HighlightDiagnosticSelection *selection) {
    SourceDiagnostic *source_diag;
    ASTNode *ast_diag;
    size_t pos;
    size_t len;

    if (!selection) return 0;
    memset(selection, 0, sizeof(*selection));
    if (!context || !node || node->length == 0) return 0;

    source_diag = context->source_diagnostics_list;
    while (source_diag) {
        if (source_diagnostic_span_utf8(context, source_diag, &pos, &len) &&
            highlight_spans_overlap(node->pos, node->length, pos, len) &&
            highlight_should_replace_diagnostic_match(selection,
                                                     source_diag->severity == SOURCE_DIAG_WARNING ? CB_WARNING : CB_ERROR,
                                                     pos,
                                                     len)) {
            selection->source_diag = source_diag;
            selection->ast_diag = 0;
            selection->severity = source_diag->severity == SOURCE_DIAG_WARNING ? CB_WARNING : CB_ERROR;
            selection->pos = pos;
            selection->len = len;
        }
        source_diag = source_diag->next_in_context;
    }

    if (selection->source_diag) return 1;

    ast_diag = (ASTNode *)context->diagnostics_list;
    while (ast_diag) {
        if ((ast_diag->node_type == RXCP_ERROR || ast_diag->node_type == RXCP_WARNING) &&
            ast_node_span_utf8(context, ast_diag, &pos, &len) &&
            highlight_spans_overlap(node->pos, node->length, pos, len) &&
            highlight_should_replace_diagnostic_match(selection,
                                                     ast_diag->node_type == RXCP_WARNING ? CB_WARNING : CB_ERROR,
                                                     pos,
                                                     len)) {
            selection->source_diag = 0;
            selection->ast_diag = ast_diag;
            selection->severity = ast_diag->node_type == RXCP_WARNING ? CB_WARNING : CB_ERROR;
            selection->pos = pos;
            selection->len = len;
        }
        ast_diag = ast_diag->sibling;
    }

    return selection->ast_diag != 0;
}

static void highlight_overlay_diagnostic_on_leaf(CB_Node *node,
                                                 size_t depth,
                                                 void *user_data) {
    Context *context;
    HighlightDiagnosticSelection selection;

    (void)depth;

    context = (Context *)user_data;
    if (!context || !node || node->child) return;
    if (node->type == SYNTAX_ERROR || node->type == INTERNAL_ERROR) return;
    if (!highlight_select_diagnostic_for_leaf(context, node, &selection)) return;
    if (node->severity > selection.severity) return;
    if (node->severity == selection.severity && node->message) return;

    node->severity = selection.severity;
    if (node->message) {
        free(node->message);
        node->message = 0;
    }
    if (selection.source_diag) {
        node->message = format_source_diagnostic_message(selection.source_diag);
    } else if (selection.ast_diag) {
        node->message = format_ast_diagnostic_message(selection.ast_diag);
    }
}

static void highlight_overlay_diagnostics_on_tree(CB_ParseTree *tb, Context *context) {
    if (!tb || !context) return;
    if (!context->source_diagnostics_list && !context->diagnostics_list) return;
    cb_walk_tree_top_down(tb, highlight_overlay_diagnostic_on_leaf, context);
}

static void emit_diagnostics_from_source_state(CB_ParseTree *tb, Context *context) {
    SourceDiagnostic *diag;
    size_t pos;
    size_t len;
    CB_Node diag_node;

    diag = context ? context->source_diagnostics_list : 0;
    while (diag) {
        if (source_diagnostic_span_utf8(context, diag, &pos, &len)) {
            diag_node = cb_create_node(SYNTAX_ERROR, pos, len);
            diag_node.severity = (diag->severity == SOURCE_DIAG_WARNING) ? CB_WARNING : CB_ERROR;
            diag_node.message = format_source_diagnostic_message(diag);
            cb_set_current_parent_to_root_node(tb);
            cb_add_child_node(tb, diag_node);
        }
        diag = diag->next_in_context;
    }
}

static void emit_diagnostics_from_detached_ast(CB_ParseTree *tb, Context *context, ASTNode *diag) {
    size_t pos;
    size_t len;
    CB_Node diag_node;

    while (diag) {
        if ((diag->node_type == RXCP_ERROR || diag->node_type == RXCP_WARNING) &&
            ast_node_span_utf8(context, diag, &pos, &len)) {
            diag_node = cb_create_node(SYNTAX_ERROR, pos, len);
            diag_node.severity = (diag->node_type == RXCP_WARNING) ? CB_WARNING : CB_ERROR;
            diag_node.message = format_ast_diagnostic_message(diag);
            cb_set_current_parent_to_root_node(tb);
            cb_add_child_node(tb, diag_node);
        }
        diag = diag->sibling;
    }
}

static const char *highlight_source_ptr_for_pos(HighlightTokenCursor *cursor, size_t pos) {
    const char *ptr;
    size_t current_pos;
    utf8_int32_t codepoint;

    if (!cursor || !cursor->context || !cursor->context->buff_start) return 0;

    if (!cursor->last_source_ptr || pos < cursor->last_source_pos) {
        ptr = cursor->context->buff_start;
        current_pos = 0;
    } else {
        ptr = cursor->last_source_ptr;
        current_pos = cursor->last_source_pos;
    }

    while (ptr && *ptr && current_pos < pos) {
        ptr = utf8codepoint(ptr, &codepoint);
        current_pos++;
    }

    if (!ptr || current_pos != pos) return 0;

    cursor->last_source_ptr = ptr;
    cursor->last_source_pos = current_pos;
    return ptr;
}

/* Comments are intentionally skipped by the compiler lexer, so parser mode
 * must recover them from source gaps after projecting the authoritative tree. */
static int highlight_comment_span(HighlightTokenCursor *cursor,
                                  size_t pos,
                                  CB_Node *node_out) {
    Context *context;
    const char *start;
    const char *ptr;
    size_t byte_length;
    size_t char_length;
    int depth;

    if (!cursor || !node_out) return 0;
    context = cursor->context;
    if (!context) return 0;

    start = highlight_source_ptr_for_pos(cursor, pos);
    if (!start || start >= context->buff_end) return 0;

    if ((size_t)(context->buff_end - start) >= 2 && start[0] == '/' && start[1] == '*') {
        depth = 1;
        ptr = start + 2;
        while (ptr < context->buff_end) {
            if ((size_t)(context->buff_end - ptr) >= 2 && ptr[0] == '/' && ptr[1] == '*') {
                depth++;
                ptr += 2;
                continue;
            }
            if ((size_t)(context->buff_end - ptr) >= 2 && ptr[0] == '*' && ptr[1] == '/') {
                depth--;
                ptr += 2;
                if (depth == 0) break;
                continue;
            }
            ptr++;
        }

        byte_length = (size_t)(ptr - start);
        char_length = utf8nlen(start, byte_length);
        if (char_length == 0) char_length = 1;
        *node_out = cb_create_node(LEXER_COMMENT, pos, char_length);
        return 1;
    }

    if (context->comments_hash && start[0] == '#') {
        ptr = start + 1;
    } else if (context->comments_dash &&
               (size_t)(context->buff_end - start) >= 2 &&
               start[0] == '-' && start[1] == '-') {
        ptr = start + 2;
    } else if (context->comments_slash &&
               (size_t)(context->buff_end - start) >= 2 &&
               start[0] == '/' && start[1] == '/') {
        ptr = start + 2;
    } else {
        return 0;
    }

    while (ptr < context->buff_end && *ptr != '\n' && *ptr != '\r') ptr++;

    byte_length = (size_t)(ptr - start);
    char_length = utf8nlen(start, byte_length);
    if (char_length == 0) char_length = 1;
    *node_out = cb_create_node(LEXER_COMMENT, pos, char_length);
    return 1;
}

static CB_Node compiler_get_token_callback(void *user_data,
                                           size_t pos,
                                           size_t length,
                                           CodeBufferCharacter* token_chars) {
    HighlightTokenCursor *cursor;
    Token *token;
    size_t token_pos;
    size_t token_len;
    CB_Node comment_node;

    cursor = (HighlightTokenCursor *)user_data;
    token = cursor ? cursor->context->token_head : 0;
    while (token) {
        if (token_span_utf8(cursor->context, token, &token_pos, &token_len) && token_pos == pos) {
            return highlight_cb_node_for_token(cursor, token, pos, token_len);
        }
        token = token->token_next;
    }

    if (highlight_comment_span(cursor, pos, &comment_node)) {
        return comment_node;
    }

    if (token_chars && length > 0 && token_chars[0].codepoints > 0) {
        switch (token_chars[0].character[0]) {
            case '.':
            case ',':
                return cb_create_node(LEXER_SEPARATOR, pos, 1);
            case '(':
            case '[':
                return cb_create_node(LEXER_LH_EXPR, pos, 1);
            case ')':
            case ']':
                return cb_create_node(LEXER_RH_EXPR, pos, 1);
            case '=':
                return cb_create_node(LEXER_OPERATOR_ASSIGN, pos, 1);
            case '+':
            case '-':
            case '*':
            case '/':
                return cb_create_node(LEXER_OPERATOR_ARITHMETIC, pos, 1);
        }
    }

    return cb_default_get_token_callback(user_data, pos, length, token_chars);
}

static void emit_flat_tokens(CB_ParseTree *tb, Context *context) {
    Token *token;
    size_t pos;
    size_t len;

    token = context->token_head;
    while (token) {
        if (token_span_utf8(context, token, &pos, &len)) {
            cb_add_child_node(tb, cb_create_node(map_c_token_to_cb_type(token->token_type), pos, len));
        }
        token = token->token_next;
    }
}

void rxc_highlight_controller_parse(CodeBuffer *cb) {
    char *source_code;
    size_t source_len;
    Context *context;
    Context *root_context;
    CB_ParseTree *tb;
    CB_Node root_node;
    HighlightTokenCursor cursor;
    HighlightDocumentInfo doc_info;
    int have_doc_info;

    if (!cb) return;

    source_code = get_code_buffer_source(cb);
    if (!source_code) return;
    source_len = strlen(source_code);
    memset(&cursor, 0, sizeof(cursor));
    have_doc_info = highlight_build_document_info(cb, &doc_info);
    root_context = highlight_prepare_root_cache(cb);

    context = cntx_f();
    context->master_context = root_context ? root_context : context;
    context->file_name = strdup(have_doc_info && doc_info.document_name ? doc_info.document_name : "dsl_buffer.rexx");
    context->debug_mode = 0;
    context->stop_after_parse = 1;
    context->optimise = 0;
    context->level = LEVELB;
    context->disable_exits = root_context ? root_context->disable_exits : (have_doc_info ? doc_info.disable_exits : 0);
    context->location = root_context ? root_context->location : (have_doc_info ? doc_info.document_dir : 0);

    cntx_buf(context, source_code, source_len);
    opt_pars(context);
    free_ast(context);
    free_tok(context);
    cntx_buf(context, source_code, source_len);
    if (!root_context) {
        configure_parser_import_locations(context);
        if (!context->disable_exits) rxcp_init_exits(context);
        if (!context->importable_file_list) context->importable_file_list = rxfl_lst(context);
    }
    rexbpars(context);

    if (!context->ast) {
        rxcp_run_fallback_diagnostics(context);
    } else {
        rxcp_prepare_source_ast(context);
        source_tree_sync_diagnostics(context);
        if (!highlight_has_source_errors(context)) {
            validate_ast(context);
            source_tree_sync_diagnostics(context);
            source_tree_sync_semantics(context);
        }
    }

    tb = cb_create_token_buffer();
    root_node = cb_create_node(PARSE_TREE_FILE, 0, source_len);
    cb_add_child_node(tb, root_node);
    cb_set_current_parent_to_root_node(tb);

    if (context->source_tree) {
        cursor.context = context;
        cursor.token = context->token_head;
        cursor.last_source_ptr = context->buff_start;
        cursor.last_source_pos = 0;
        highlight_prepare_semantic_tokens(&cursor, context->source_tree);
        emit_source_projection(tb, context, context->source_tree->child, &cursor);
        emit_tokens_until(tb, &cursor, source_len);
        emit_diagnostics_from_source_state(tb, context);
    } else {
        emit_flat_tokens(tb, context);
        emit_diagnostics_from_detached_ast(tb, context, (ASTNode *)context->diagnostics_list);
    }

    cb_order_tree(tb);
    cursor.context = context;
    cursor.token = context->token_head;
    cursor.last_source_ptr = context->buff_start;
    cursor.last_source_pos = 0;
    if (!cursor.semantic_tokens && context->source_tree) highlight_prepare_semantic_tokens(&cursor, context->source_tree);
    cb_add_missing_tokens(tb, cb, compiler_get_token_callback, &cursor);
    cb_tweak_tree_positions(tb);
    highlight_overlay_diagnostics_on_tree(tb, context);
    cb_validate_tree(tb);
    highlight_free_semantic_tokens(&cursor);

    if (context->master_context != context) {
        highlight_release_parse_exit_objects(context, context->ast);
    }
    if (context->file_name) free(context->file_name);
    fre_cntx(context);
    if (have_doc_info) highlight_free_document_info(&doc_info);
    cb->parse_tree = tb;
}

#endif
