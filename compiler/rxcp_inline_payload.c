/*
 * Private inline implementation fragment.
 * Included by rxcp_inline.c; not compiled separately.
 */

static int inline_proc_has_body(ASTNode *node) {
    ASTNode *instrs;

    if (!node) return 0;

    instrs = ast_chld(node, INSTRUCTIONS, NOP);
    return instrs && instrs->node_type != NOP;
}

static int inline_ascii_starts_with_ci(const char *text, size_t text_len, const char *prefix) {
    size_t i;
    size_t prefix_len;

    if (!text || !prefix) return 0;
    prefix_len = strlen(prefix);
    if (text_len < prefix_len) return 0;

    for (i = 0; i < prefix_len; i++) {
        char a = text[i];
        char b = prefix[i];

        if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
        if (b >= 'A' && b <= 'Z') b = (char)(b - 'A' + 'a');
        if (a != b) return 0;
    }

    return 1;
}

static int inline_assembler_has_unsupported_aliasing(ASTNode *node) {
    if (!node || node->node_type != ASSEMBLER || !node->node_string) return 0;

    return inline_ascii_starts_with_ci(node->node_string, node->node_string_length, "link") ||
           inline_ascii_starts_with_ci(node->node_string, node->node_string_length, "unlink");
}

static int inline_assembler_has_unsupported_effect(ASTNode *node) {
    ASTNode *child;

    if (!node || node->node_type != ASSEMBLER) return 0;

    /*
     * Array operands cannot be safely represented by the current inline
     * substitution model for assembler statements. The normal call path can
     * pass the array/ref shape through the callee signature, but an inlined
     * assembler operand is validated directly at the caller site and array
     * actuals are rejected there.
     *
     * Stateful scalar string helpers are intentionally not rejected here:
     * assembler operands are marked read/write by symbol validation, so
     * writable by-value formals are materialised through the same copy
     * semantics as normal calls. SCOPY resets copied string cursor state, which
     * preserves the call prologue boundary for setstrpos/substcut/dropchar
     * style helpers.
     */
    for (child = node->child; child; child = child->sibling) {
        if (inline_node_has_array_shape(child)) return 1;
        if (child->symbolNode &&
            child->symbolNode->symbol &&
            child->symbolNode->symbol->value_dims > 0) {
            return 1;
        }
    }

    return 0;
}

static int inline_proc_has_procedure_expose(ASTNode *node) {
    return node && ast_chld(node, EXPOSED, 0) != NULL;
}

static int inline_prune_candidate(ASTNode *node) {
    Symbol *symbol;

    if (!node || node->node_type != PROCEDURE) return 0;
    if (!inline_proc_has_body(node)) return 0;
    if (inline_proc_has_procedure_expose(node)) return 0;

    symbol = inline_symbol_from_proc_def(node);
    if (!symbol) return 0;
    if (!symbol->is_inlinable) return 0;
    if (symbol->is_main || symbol->is_implicit_main || symbol->exposed) return 0;

    return 1;
}

static int inline_collect_remaining_call_symbols(ASTNode *node,
                                                 Symbol ***symbols,
                                                 size_t *symbol_count) {
    ASTNode *child;

    if (!node) return 1;
    if (node->is_inline_pruned && inline_node_is_callable_def(node)) return 1;

    if ((node->node_type == FUNCTION ||
         node->node_type == FUNC_SYMBOL ||
         node->node_type == MEMBER_CALL ||
         node->node_type == FACTORY_CALL) &&
        node->symbolNode &&
        node->symbolNode->symbol) {
        if (!inline_append_symbol(symbols, symbol_count, node->symbolNode->symbol)) return 0;
    }

    child = node->child;
    while (child) {
        if (!inline_collect_remaining_call_symbols(child, symbols, symbol_count)) return 0;
        child = child->sibling;
    }

    return 1;
}

static int inline_prune_unreferenced_candidates(Context *context,
                                                ASTNode *node,
                                                Symbol **live_symbols,
                                                size_t live_count) {
    ASTNode *child;
    int changed;

    if (!node) return 0;

    changed = 0;

    if (!node->is_inline_pruned && inline_prune_candidate(node)) {
        Symbol *symbol;

        symbol = inline_symbol_from_proc_def(node);
        if (symbol && !inline_symbol_in_list(live_symbols, live_count, symbol)) {
            node->is_inline_pruned = 1;
            changed = 1;
            inline_debug_log(context, node, symbol, "DEBUG_INLINE",
                             "prune: private procedure has no remaining local call sites");
        }
    }

    if (node->is_inline_pruned && inline_node_is_callable_def(node)) return changed;

    child = node->child;
    while (child) {
        if (inline_prune_unreferenced_candidates(context, child, live_symbols, live_count)) {
            changed = 1;
        }
        child = child->sibling;
    }

    return changed;
}

void rxcp_inline_prune(Context *context, ASTNode *tree) {
    int changed;

    if (!context || !tree) return;

    do {
        Symbol **live_symbols;
        size_t live_count;

        live_symbols = NULL;
        live_count = 0;

        if (!inline_collect_remaining_call_symbols(tree, &live_symbols, &live_count)) {
            free(live_symbols);
            return;
        }
        changed = inline_prune_unreferenced_candidates(context, tree, live_symbols, live_count);
        free(live_symbols);
    } while (changed);
}

typedef struct {
    char *text;
    size_t length;
    size_t capacity;
    int ok;
} InlineMetaText;

typedef struct {
    Symbol *symbol;
    size_t id;
} InlineMetaSymbolEntry;

typedef struct {
    Scope *scope;
    size_t id;
    size_t parent_id;
} InlineMetaScopeEntry;

typedef struct {
    const char *file_name;
    size_t id;
} InlineMetaFileEntry;

typedef struct {
    const char *file_name;
    int line;
    int active_start_column;
    int active_end_column;
    char provenance;
    const char *source_start;
    size_t source_length;
    size_t file_id;
    size_t id;
} InlineMetaSourceEntry;

typedef struct {
    Symbol *symbol;
    size_t id;
} InlineMetaDependencyEntry;

typedef struct {
    Scope *root_scope;
    InlineMetaScopeEntry *scopes;
    size_t scope_count;
    InlineMetaSymbolEntry *symbols;
    size_t symbol_count;
    InlineMetaFileEntry *files;
    size_t file_count;
    InlineMetaSourceEntry *sources;
    size_t source_count;
    InlineMetaDependencyEntry *dependencies;
    size_t dependency_count;
} InlineMetaExport;

typedef struct {
    ASTNode *node;
    Symbol *symbol;
    Scope *scope;
    const char *reason;
} InlineMetaCollectFailure;

typedef struct {
    SourceNode *source_node;
    char provenance;
} InlineMetaImportedSource;

typedef struct {
    char *fqname;
    char *type;
    char *args;
    Symbol *symbol;
} InlineMetaImportedDependency;

typedef struct {
    Symbol **symbols;
    size_t symbol_count;
    Scope **scopes;
    size_t scope_count;
    char **files;
    size_t file_count;
    InlineMetaImportedSource *sources;
    size_t source_count;
    InlineMetaImportedDependency *dependencies;
    size_t dependency_count;
    ASTNode **stack;
    size_t stack_count;
    ASTNode *args_root;
    ASTNode *root;
    Scope *scope;
    int tree_section;
    int version;
    int ok;
} InlineMetaImport;

#define INLINE_META_NODE_SCOPE_DEF 4096u

static void inline_meta_text_init(InlineMetaText *text) {
    text->capacity = 128;
    text->length = 0;
    text->text = malloc(text->capacity);
    text->ok = text->text != NULL;
    if (text->text) text->text[0] = 0;
}

static int inline_meta_text_reserve(InlineMetaText *text, size_t extra) {
    char *new_text;
    size_t new_capacity;

    if (!text || !text->ok) return 0;
    if (text->length + extra + 1 <= text->capacity) return 1;

    new_capacity = text->capacity;
    while (text->length + extra + 1 > new_capacity) new_capacity *= 2;

    new_text = realloc(text->text, new_capacity);
    if (!new_text) {
        text->ok = 0;
        return 0;
    }

    text->text = new_text;
    text->capacity = new_capacity;
    return 1;
}

static int inline_meta_text_append(InlineMetaText *text, const char *fragment) {
    size_t length;

    if (!fragment) fragment = "";
    length = strlen(fragment);
    if (!inline_meta_text_reserve(text, length)) return 0;
    memcpy(text->text + text->length, fragment, length + 1);
    text->length += length;
    return 1;
}

static int inline_meta_text_appendf(InlineMetaText *text, const char *format, ...) {
    va_list args;
    size_t needed;
    char *buffer;

    if (!text || !text->ok || !format) return 0;

    va_start(args, format);
    needed = (size_t)vsnprintf(NULL, 0, format, args);
    va_end(args);

    buffer = malloc(needed + 1);
    if (!buffer) {
        text->ok = 0;
        return 0;
    }

    va_start(args, format);
    vsnprintf(buffer, needed + 1, format, args);
    va_end(args);

    if (!inline_meta_text_append(text, buffer)) {
        free(buffer);
        return 0;
    }

    free(buffer);
    return 1;
}

static char inline_meta_hex_digit(unsigned int value) {
    return (char)(value < 10 ? ('0' + value) : ('A' + (value - 10)));
}

static char *inline_meta_hex_encode(const char *text, size_t length) {
    char *encoded;
    size_t i;

    if (!text || !length) return strdup("-");

    encoded = malloc((length * 2) + 1);
    if (!encoded) return NULL;

    for (i = 0; i < length; i++) {
        unsigned char value = (unsigned char)text[i];
        encoded[i * 2] = inline_meta_hex_digit((unsigned int)(value >> 4));
        encoded[(i * 2) + 1] = inline_meta_hex_digit((unsigned int)(value & 0x0f));
    }
    encoded[length * 2] = 0;
    return encoded;
}

static int inline_meta_hex_value(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    return -1;
}

static char *inline_meta_hex_decode(const char *encoded, size_t *length_out) {
    char *decoded;
    size_t encoded_length;
    size_t i;

    if (length_out) *length_out = 0;
    if (!encoded || strcmp(encoded, "-") == 0) return strdup("");

    encoded_length = strlen(encoded);
    if (encoded_length % 2 != 0) return NULL;

    decoded = malloc((encoded_length / 2) + 1);
    if (!decoded) return NULL;

    for (i = 0; i < encoded_length; i += 2) {
        int hi = inline_meta_hex_value(encoded[i]);
        int lo = inline_meta_hex_value(encoded[i + 1]);
        if (hi < 0 || lo < 0) {
            free(decoded);
            return NULL;
        }
        decoded[i / 2] = (char)((hi << 4) | lo);
    }
    decoded[encoded_length / 2] = 0;
    if (length_out) *length_out = encoded_length / 2;
    return decoded;
}

static char *inline_meta_int_list_encode(const int *values, size_t count, int default_value) {
    char *encoded;
    size_t capacity;
    size_t length;
    size_t i;

    if (!count) return strdup("-");

    capacity = (count * 16) + 1;
    encoded = malloc(capacity);
    if (!encoded) return NULL;

    length = 0;
    encoded[0] = 0;
    for (i = 0; i < count; i++) {
        int value = values ? values[i] : default_value;
        int written;

        written = snprintf(encoded + length, capacity - length, "%s%d", i ? ":" : "", value);
        if (written < 0 || (size_t)written >= capacity - length) {
            free(encoded);
            return NULL;
        }
        length += (size_t)written;
    }

    return encoded;
}

static int *inline_meta_int_list_decode(const char *encoded, size_t count, int default_value) {
    int *values;
    const char *cursor;
    char *end;
    size_t i;

    if (!count) return NULL;

    values = malloc(sizeof(int) * count);
    if (!values) return NULL;

    if (!encoded || strcmp(encoded, "-") == 0) {
        for (i = 0; i < count; i++) values[i] = default_value;
        return values;
    }

    cursor = encoded;
    for (i = 0; i < count; i++) {
        long value;

        value = strtol(cursor, &end, 10);
        if (end == cursor || (i + 1 < count && *end != ':') || (i + 1 == count && *end != 0)) {
            free(values);
            return NULL;
        }

        values[i] = (int)value;
        cursor = end + 1;
    }

    return values;
}

static char *inline_meta_decode_optional_hex(const char *encoded) {
    char *decoded;
    size_t length;

    decoded = inline_meta_hex_decode(encoded, &length);
    if (!decoded) return NULL;
    if (!length) {
        free(decoded);
        return NULL;
    }

    return decoded;
}

static int inline_meta_set_symbol_shape(Symbol *symbol,
                                        ValueType type,
                                        size_t dims,
                                        int *dim_base,
                                        int *dim_elements,
                                        const char *class_name) {
    size_t i;

    if (!symbol) return 0;

    symbol->type = type;
    symbol->value_dims = dims;

    if (symbol->dim_base) free(symbol->dim_base);
    if (symbol->dim_elements) free(symbol->dim_elements);
    symbol->dim_base = NULL;
    symbol->dim_elements = NULL;

    if (dims > 0) {
        symbol->dim_base = malloc(sizeof(int) * dims);
        symbol->dim_elements = malloc(sizeof(int) * dims);
        if (!symbol->dim_base || !symbol->dim_elements) {
            if (symbol->dim_base) free(symbol->dim_base);
            if (symbol->dim_elements) free(symbol->dim_elements);
            symbol->dim_base = NULL;
            symbol->dim_elements = NULL;
            return 0;
        }
        for (i = 0; i < dims; i++) {
            symbol->dim_base[i] = dim_base ? dim_base[i] : 1;
            symbol->dim_elements[i] = dim_elements ? dim_elements[i] : 0;
        }
    }

    if (symbol->value_class) {
        free(symbol->value_class);
        symbol->value_class = NULL;
    }
    if (class_name && *class_name) {
        symbol->value_class = strdup(class_name);
        if (!symbol->value_class) return 0;
    }

    return 1;
}

static unsigned int inline_meta_symbol_flags(Symbol *symbol) {
    unsigned int flags;

    flags = 0;
    if (!symbol) return flags;
    if (symbol->is_arg) flags |= 1u;
    if (symbol->is_ref_arg) flags |= 2u;
    if (symbol->is_opt_arg) flags |= 4u;
    if (symbol->is_const_arg) flags |= 8u;
    if (symbol->has_vargs) flags |= 16u;
    if (symbol->exposed) flags |= 32u;
    if (symbol->is_this) flags |= 64u;
    if (symbol->is_factory) flags |= 128u;
    if (symbol->needs_default_initiation) flags |= 256u;
    return flags;
}

static unsigned int inline_meta_node_flags(ASTNode *node) {
    unsigned int flags;

    flags = 0;
    if (!node) return flags;
    if (node->is_ref_arg) flags |= 1u;
    if (node->is_opt_arg) flags |= 2u;
    if (node->is_const_arg) flags |= 4u;
    if (node->is_varg) flags |= 8u;
    if (node->is_compiler_added) flags |= 16u;
    if (node->is_implicit_main) flags |= 32u;
    if (node->is_interface_default_method) flags |= 64u;
    if (node->force_local_scope) flags |= 128u;
    if (node->inherit_parent_scope) flags |= 256u;
    if (node->inherit_parent_reg_scope) flags |= 512u;
    if (node->suppress_shadow_warnings) flags |= 1024u;
    if (node->skip_exit_dispatch) flags |= 2048u;
    if (node->scope && node->scope->defining_node == node) flags |= INLINE_META_NODE_SCOPE_DEF;
    return flags;
}

static void inline_meta_apply_symbol_flags(Symbol *symbol, unsigned int flags) {
    if (!symbol) return;
    symbol->is_arg = (flags & 1u) != 0;
    symbol->is_ref_arg = (flags & 2u) != 0;
    symbol->is_opt_arg = (flags & 4u) != 0;
    symbol->is_const_arg = (flags & 8u) != 0;
    symbol->has_vargs = (flags & 16u) != 0;
    symbol->exposed = (flags & 32u) != 0;
    symbol->is_this = (flags & 64u) != 0;
    symbol->is_factory = (flags & 128u) != 0;
    symbol->needs_default_initiation = (flags & 256u) != 0;
}

static void inline_meta_apply_node_flags(ASTNode *node, unsigned int flags) {
    if (!node) return;
    node->is_ref_arg = (flags & 1u) != 0;
    node->is_opt_arg = (flags & 2u) != 0;
    node->is_const_arg = (flags & 4u) != 0;
    node->is_varg = (flags & 8u) != 0;
    node->is_compiler_added = (flags & 16u) != 0;
    node->is_implicit_main = (flags & 32u) != 0;
    node->is_interface_default_method = (flags & 64u) != 0;
    node->force_local_scope = (flags & 128u) != 0;
    node->inherit_parent_scope = (flags & 256u) != 0;
    node->inherit_parent_reg_scope = (flags & 512u) != 0;
    node->suppress_shadow_warnings = (flags & 1024u) != 0;
    node->skip_exit_dispatch = (flags & 2048u) != 0;
}

static int inline_meta_symbol_is_exportable(Symbol *symbol) {
    if (!symbol) return 1;
    return symbol->symbol_type == VARIABLE_SYMBOL || symbol->symbol_type == CONSTANT_SYMBOL;
}

static ASTNode *inline_meta_direct_dependency_proc(Symbol *symbol) {
    SymbolNode *defsn;

    if (!symbol || symbol->symbol_type != FUNCTION_SYMBOL || !symbol->exposed) return NULL;
    if (sym_nond(symbol) <= 0) return NULL;

    defsn = sym_trnd(symbol, 0);
    if (!defsn || !defsn->node || defsn->node->node_type != PROCEDURE) return NULL;

    return defsn->node;
}

static int inline_meta_function_uses_direct_dependency(ASTNode *node) {
    if (!node || node->node_type != FUNCTION || !node->symbolNode || !node->symbolNode->symbol) return 0;
    return inline_meta_direct_dependency_proc(node->symbolNode->symbol) != NULL;
}

static int inline_meta_node_is_exportable(ASTNode *node) {
    if (!node) return 1;
    if (node->association &&
        !(node->node_type == INSTRUCTIONS &&
          node->is_compiler_added &&
          inline_node_is_callable_def(node->association))) {
        return 0;
    }

    switch (node->node_type) {
        case ARGS:
        case ARG:
        case INSTRUCTIONS:
        case IF:
        case RETURN:
        case SAY:
        case DEFINE:
        case ASSIGN:
        case DO:
        case REPEAT:
        case TO:
        case BY:
        case FOR:
        case WHILE:
        case UNTIL:
        case CLASS:
        case VAR_TARGET:
        case VAR_SYMBOL:
        case VAR_REFERENCE:
        case VARG:
        case VARG_REFERENCE:
        case CONST_SYMBOL:
        case INTEGER:
        case FLOAT:
        case DECIMAL:
        case STRING:
        case BINARY:
        case CONSTANT:
        case RANGE:
        case NOVAL:
        case VOID:
        case LITERAL:
        case OP_ADD:
        case OP_MINUS:
        case OP_MULT:
        case OP_DIV:
        case OP_IDIV:
        case OP_MOD:
        case OP_POWER:
        case OP_NEG:
        case OP_PLUS:
        case OP_CONCAT:
        case OP_SCONCAT:
        case OP_AND:
        case OP_OR:
        case OP_NOT:
        case OP_BIT_AND:
        case OP_BIT_OR:
        case OP_BIT_XOR:
        case OP_BIT_NOT:
        case OP_BIT_SHL:
        case OP_BIT_SHR:
        case OP_FLAG_HAS:
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
        case OP_TYPE_IS:
        case OP_TYPE_CAST:
        case OP_TYPEOF:
        case OP_ARGS:
        case OP_ARG_VALUE:
        case OP_ARG_EXISTS:
        case OP_ARG_IX_EXISTS:
        case CALL:
            return 1;
        case FUNCTION:
            return inline_meta_function_uses_direct_dependency(node);
        case ASSEMBLER:
            return !inline_assembler_has_unsupported_aliasing(node) &&
                   !inline_assembler_has_unsupported_effect(node);
        default:
            return 0;
    }
}

static int inline_meta_find_scope_id(InlineMetaExport *meta, Scope *scope, size_t *id_out) {
    size_t i;

    if (id_out) *id_out = 0;
    if (!meta || !scope) return 0;

    for (i = 0; i < meta->scope_count; i++) {
        if (meta->scopes[i].scope == scope) {
            if (id_out) *id_out = meta->scopes[i].id;
            return 1;
        }
    }

    return 0;
}

static int inline_meta_find_file_id(InlineMetaExport *meta, const char *file_name, size_t *id_out) {
    size_t i;

    if (id_out) *id_out = (size_t)-1;
    if (!meta || !file_name) return 0;

    for (i = 0; i < meta->file_count; i++) {
        if (meta->files[i].file_name && strcmp(meta->files[i].file_name, file_name) == 0) {
            if (id_out) *id_out = meta->files[i].id;
            return 1;
        }
    }

    return 0;
}

static int inline_meta_collect_file(InlineMetaExport *meta, const char *file_name, size_t *id_out) {
    InlineMetaFileEntry *new_files;

    if (id_out) *id_out = (size_t)-1;
    if (!file_name || !*file_name) return 1;
    if (inline_meta_find_file_id(meta, file_name, id_out)) return 1;

    new_files = realloc(meta->files, sizeof(InlineMetaFileEntry) * (meta->file_count + 1));
    if (!new_files) return 0;

    meta->files = new_files;
    meta->files[meta->file_count].file_name = file_name;
    meta->files[meta->file_count].id = meta->file_count;
    if (id_out) *id_out = meta->file_count;
    meta->file_count++;
    return 1;
}

static int inline_meta_pointer_in_range(const char *ptr, const char *range_start, const char *range_end) {
    return ptr && range_start && range_end && ptr >= range_start && ptr <= range_end;
}

static int inline_meta_context_range(Context *context, const char *ptr, const char **range_start, const char **range_end) {
    if (!context || !context->buff_start || !context->buff_end || !ptr) return 0;
    if (ptr < context->buff_start || ptr > context->buff_end) return 0;
    if (range_start) *range_start = context->buff_start;
    if (range_end) *range_end = context->buff_end;
    return 1;
}

static int inline_meta_owned_source_range(SourceNode *source_node,
                                          const char *ptr,
                                          const char **range_start,
                                          const char **range_end) {
    size_t length;

    if (!source_node || !source_node->owned_source_text || !ptr) return 0;
    length = strlen(source_node->owned_source_text);
    if (!inline_meta_pointer_in_range(ptr, source_node->owned_source_text, source_node->owned_source_text + length)) return 0;
    if (range_start) *range_start = source_node->owned_source_text;
    if (range_end) *range_end = source_node->owned_source_text + length;
    return 1;
}

static int inline_meta_source_range(ASTNode *node,
                                    const char *source_start,
                                    const char **range_start,
                                    const char **range_end) {
    if (!node || !source_start) return 0;
    if (inline_meta_owned_source_range(node->source_node, source_start, range_start, range_end)) return 1;
    if (inline_meta_context_range(node->context, source_start, range_start, range_end)) return 1;
    if (node->source_node && inline_meta_context_range(node->source_node->context, source_start, range_start, range_end)) return 1;
    return 0;
}

static int inline_meta_line_bounds(const char *ptr,
                                   const char *range_start,
                                   const char *range_end,
                                   const char **line_start_out,
                                   const char **line_end_out) {
    const char *line_start;
    const char *line_end;

    if (!ptr || !range_start || !range_end || ptr < range_start || ptr > range_end) return 0;
    if (ptr == range_end && ptr > range_start) ptr--;

    line_start = ptr;
    while (line_start > range_start &&
           line_start[-1] != '\n' &&
           line_start[-1] != '\r') {
        line_start--;
    }

    line_end = ptr;
    while (line_end < range_end &&
           *line_end != '\n' &&
           *line_end != '\r' &&
           *line_end != 0) {
        line_end++;
    }

    if (line_start_out) *line_start_out = line_start;
    if (line_end_out) *line_end_out = line_end;
    return 1;
}

static int inline_meta_node_source_data(ASTNode *node,
                                        const char **file_name_out,
                                        int *line_out,
                                        int *active_start_column_out,
                                        int *active_end_column_out,
                                        char *provenance_out,
                                        const char **source_start_out,
                                        size_t *source_length_out) {
    const char *source_start;
    const char *source_end;
    const char *range_start;
    const char *range_end;
    const char *line_start;
    const char *line_end;
    const char *active_end;
    const char *file_name;
    int line;
    int column;
    int active_start_column;
    int active_end_column;
    char provenance;

    if (file_name_out) *file_name_out = NULL;
    if (line_out) *line_out = -1;
    if (active_start_column_out) *active_start_column_out = -1;
    if (active_end_column_out) *active_end_column_out = -1;
    if (provenance_out) *provenance_out = AST_SOURCE_NONE;
    if (source_start_out) *source_start_out = NULL;
    if (source_length_out) *source_length_out = 0;
    if (!node) return 0;

    file_name = node->file_name;
    line = node->line;
    column = node->column;
    provenance = node->source_provenance;
    source_start = node->source_start;
    source_end = node->source_end;

    if (node->source_node) {
        if (!file_name) file_name = node->source_node->file_name;
        if (line < 0) line = node->source_node->line;
        if (column < 0) column = node->source_node->column;
        if (!source_start) source_start = node->source_node->source_start;
        if (!source_end) source_end = node->source_node->source_end;
    }

    if (!source_start || !source_end || source_end < source_start) return 0;

    range_start = 0;
    range_end = 0;
    line_start = source_start;
    line_end = source_end + 1;
    active_start_column = column >= 0 ? column + 1 : 1;
    active_end_column = active_start_column + (int)(source_end - source_start) + 1;
    if (inline_meta_source_range(node, source_start, &range_start, &range_end) &&
        inline_meta_line_bounds(source_start, range_start, range_end, &line_start, &line_end)) {
        active_end = source_end;
        if (active_end >= line_end && line_end > line_start) active_end = line_end - 1;
        active_start_column = (int)(source_start - line_start) + 1;
        active_end_column = (int)(active_end - line_start) + 2;
        if (active_end_column > (int)(line_end - line_start) + 1) {
            active_end_column = (int)(line_end - line_start) + 1;
        }
    }

    if (line_end < line_start) return 0;
    if ((size_t)(line_end - line_start) > INLINE_META_MAX_SOURCE_SPAN) return 0;
    if (memchr(line_start, 0, (size_t)(line_end - line_start))) return 0;

    if (file_name_out) *file_name_out = file_name;
    if (line_out) *line_out = line;
    if (active_start_column_out) *active_start_column_out = active_start_column;
    if (active_end_column_out) *active_end_column_out = active_end_column;
    if (provenance_out) *provenance_out = provenance;
    if (source_start_out) *source_start_out = line_start;
    if (source_length_out) *source_length_out = (size_t)(line_end - line_start);
    return 1;
}

static int inline_meta_find_source_id(InlineMetaExport *meta, ASTNode *node, size_t *id_out) {
    const char *file_name;
    const char *source_start;
    size_t source_length;
    int line;
    int active_start_column;
    int active_end_column;
    char provenance;
    size_t i;

    if (id_out) *id_out = (size_t)-1;
    if (!inline_meta_node_source_data(node,
                                      &file_name,
                                      &line,
                                      &active_start_column,
                                      &active_end_column,
                                      &provenance,
                                      &source_start,
                                      &source_length)) {
        return 0;
    }

    for (i = 0; i < meta->source_count; i++) {
        InlineMetaSourceEntry *source = &meta->sources[i];
        if (source->line == line &&
            source->active_start_column == active_start_column &&
            source->active_end_column == active_end_column &&
            source->provenance == provenance &&
            source->source_length == source_length &&
            ((source->file_name == file_name) ||
             (source->file_name && file_name && strcmp(source->file_name, file_name) == 0)) &&
            memcmp(source->source_start, source_start, source_length) == 0) {
            if (id_out) *id_out = source->id;
            return 1;
        }
    }

    return 0;
}

static int inline_meta_collect_source(InlineMetaExport *meta, ASTNode *node) {
    InlineMetaSourceEntry *new_sources;
    const char *file_name;
    const char *source_start;
    size_t source_length;
    size_t file_id;
    int line;
    int active_start_column;
    int active_end_column;
    char provenance;

    if (!meta || !node) return 0;
    if (inline_meta_find_source_id(meta, node, NULL)) return 1;
    if (!inline_meta_node_source_data(node,
                                      &file_name,
                                      &line,
                                      &active_start_column,
                                      &active_end_column,
                                      &provenance,
                                      &source_start,
                                      &source_length)) {
        return 1;
    }

    file_id = (size_t)-1;
    if (file_name && *file_name && !inline_meta_collect_file(meta, file_name, &file_id)) return 0;

    new_sources = realloc(meta->sources, sizeof(InlineMetaSourceEntry) * (meta->source_count + 1));
    if (!new_sources) return 0;

    meta->sources = new_sources;
    meta->sources[meta->source_count].file_name = file_name;
    meta->sources[meta->source_count].line = line;
    meta->sources[meta->source_count].active_start_column = active_start_column;
    meta->sources[meta->source_count].active_end_column = active_end_column;
    meta->sources[meta->source_count].provenance = provenance;
    meta->sources[meta->source_count].source_start = source_start;
    meta->sources[meta->source_count].source_length = source_length;
    meta->sources[meta->source_count].file_id = file_id;
    meta->sources[meta->source_count].id = meta->source_count;
    meta->source_count++;
    return 1;
}

static int inline_meta_scope_is_exportable(InlineMetaExport *meta, Scope *scope) {
    Scope *cursor;

    if (!meta || !scope) return 0;
    if (scope == meta->root_scope) return 1;
    if (meta->root_scope &&
        meta->root_scope->defining_node &&
        (meta->root_scope->defining_node->node_type == METHOD ||
         meta->root_scope->defining_node->node_type == FACTORY) &&
        scope == meta->root_scope->parent &&
        scope->type == SCOPE_CLASS) {
        return 1;
    }
    if (scope->type != SCOPE_LOCAL) return 0;

    cursor = scope->parent;
    while (cursor) {
        if (cursor == meta->root_scope) return 1;
        if (cursor->type != SCOPE_LOCAL) return 0;
        cursor = cursor->parent;
    }

    return 0;
}

static int inline_meta_collect_scope(InlineMetaExport *meta, Scope *scope) {
    InlineMetaScopeEntry *new_scopes;
    size_t parent_id;

    if (!scope) return 1;
    if (!meta || !meta->root_scope) return 0;
    if (!inline_meta_scope_is_exportable(meta, scope)) return 0;
    if (inline_meta_find_scope_id(meta, scope, NULL)) return 1;

    parent_id = (size_t)-1;
    if (scope != meta->root_scope) {
        if (meta->root_scope &&
            meta->root_scope->defining_node &&
            (meta->root_scope->defining_node->node_type == METHOD ||
             meta->root_scope->defining_node->node_type == FACTORY) &&
            scope == meta->root_scope->parent &&
            scope->type == SCOPE_CLASS) {
            parent_id = (size_t)-1;
        } else {
            if (!inline_meta_collect_scope(meta, scope->parent)) return 0;
            if (!inline_meta_find_scope_id(meta, scope->parent, &parent_id)) return 0;
        }
    }

    new_scopes = realloc(meta->scopes, sizeof(InlineMetaScopeEntry) * (meta->scope_count + 1));
    if (!new_scopes) return 0;

    meta->scopes = new_scopes;
    meta->scopes[meta->scope_count].scope = scope;
    meta->scopes[meta->scope_count].id = meta->scope_count;
    meta->scopes[meta->scope_count].parent_id = parent_id;
    meta->scope_count++;
    return 1;
}

static int inline_meta_find_symbol_id(InlineMetaExport *meta, Symbol *symbol, size_t *id_out) {
    size_t i;

    if (id_out) *id_out = (size_t)-1;
    if (!meta || !symbol) return 0;

    for (i = 0; i < meta->symbol_count; i++) {
        if (meta->symbols[i].symbol == symbol) {
            if (id_out) *id_out = meta->symbols[i].id;
            return 1;
        }
    }

    return 0;
}

static int inline_meta_find_dependency_id(InlineMetaExport *meta, Symbol *symbol, size_t *id_out) {
    size_t i;

    if (id_out) *id_out = (size_t)-1;
    if (!meta || !symbol) return 0;

    for (i = 0; i < meta->dependency_count; i++) {
        if (meta->dependencies[i].symbol == symbol) {
            if (id_out) *id_out = meta->dependencies[i].id;
            return 1;
        }
    }

    return 0;
}

static int inline_meta_collect_symbol(InlineMetaExport *meta, Symbol *symbol) {
    InlineMetaSymbolEntry *new_symbols;

    if (!symbol) return 1;
    if (!inline_meta_symbol_is_exportable(symbol)) return 0;
    if (!inline_meta_collect_scope(meta, symbol->scope)) return 0;
    if (inline_meta_find_symbol_id(meta, symbol, NULL)) return 1;

    new_symbols = realloc(meta->symbols, sizeof(InlineMetaSymbolEntry) * (meta->symbol_count + 1));
    if (!new_symbols) return 0;

    meta->symbols = new_symbols;
    meta->symbols[meta->symbol_count].symbol = symbol;
    meta->symbols[meta->symbol_count].id = meta->symbol_count;
    meta->symbol_count++;
    return 1;
}

static int inline_meta_collect_dependency(InlineMetaExport *meta, ASTNode *node) {
    InlineMetaDependencyEntry *new_dependencies;
    Symbol *symbol;
    ASTNode *dependency_proc;

    if (!meta || !node || node->node_type != FUNCTION || !node->symbolNode) return 0;

    symbol = node->symbolNode->symbol;
    dependency_proc = inline_meta_direct_dependency_proc(symbol);
    if (!dependency_proc) return 0;
    if (meta->root_scope && dependency_proc == meta->root_scope->defining_node) return 0;
    if (inline_meta_find_dependency_id(meta, symbol, NULL)) return 1;

    new_dependencies = realloc(meta->dependencies,
                               sizeof(InlineMetaDependencyEntry) * (meta->dependency_count + 1));
    if (!new_dependencies) return 0;

    meta->dependencies = new_dependencies;
    meta->dependencies[meta->dependency_count].symbol = symbol;
    meta->dependencies[meta->dependency_count].id = meta->dependency_count;
    meta->dependency_count++;
    return 1;
}

static int inline_meta_collect(ASTNode *node, InlineMetaExport *meta) {
    ASTNode *child;

    if (!node) return 1;
    if (!inline_meta_node_is_exportable(node)) return 0;
    if (!inline_meta_collect_scope(meta, node->scope)) return 0;
    if (inline_meta_function_uses_direct_dependency(node)) {
        if (!inline_meta_collect_dependency(meta, node)) return 0;
    } else if (node->symbolNode && !inline_meta_collect_symbol(meta, node->symbolNode->symbol)) {
        return 0;
    }
    if (!inline_meta_collect_source(meta, node)) return 0;

    child = node->child;
    while (child) {
        if (!inline_meta_collect(child, meta)) return 0;
        child = child->sibling;
    }

    return 1;
}

static int inline_meta_find_collect_failure(ASTNode *node,
                                            InlineMetaExport *meta,
                                            InlineMetaCollectFailure *failure) {
    ASTNode *child;

    if (!node || !meta || !failure) return 0;

    if (node->association &&
        !(node->node_type == INSTRUCTIONS &&
          node->is_compiler_added &&
          inline_node_is_callable_def(node->association))) {
        failure->node = node;
        failure->reason = "unsupported association in inline metadata";
        return 1;
    }
    if (node->node_type == ASSEMBLER &&
        inline_assembler_has_unsupported_aliasing(node)) {
        failure->node = node;
        failure->reason = "assembler aliasing instruction";
        return 1;
    }
    if (node->node_type == ASSEMBLER &&
        inline_assembler_has_unsupported_effect(node)) {
        failure->node = node;
        failure->reason = "assembler stateful instruction";
        return 1;
    }
    if (node->node_type == FUNCTION &&
        node->symbolNode &&
        node->symbolNode->symbol &&
        !inline_meta_function_uses_direct_dependency(node)) {
        failure->node = node;
        failure->symbol = node->symbolNode->symbol;
        failure->reason = "unsupported residual callable dependency in inline metadata";
        return 1;
    }
    if (!inline_meta_node_is_exportable(node)) {
        failure->node = node;
        failure->reason = "unsupported AST node in inline metadata";
        return 1;
    }
    if (node->scope && !inline_meta_scope_is_exportable(meta, node->scope)) {
        failure->node = node;
        failure->scope = node->scope;
        failure->reason = "unsupported scope in inline metadata";
        return 1;
    }
    if (node->symbolNode &&
        node->symbolNode->symbol &&
        !inline_meta_function_uses_direct_dependency(node) &&
        !inline_meta_symbol_is_exportable(node->symbolNode->symbol)) {
        failure->node = node;
        failure->symbol = node->symbolNode->symbol;
        failure->reason = "unsupported symbol in inline metadata";
        return 1;
    }

    child = node->child;
    while (child) {
        if (inline_meta_find_collect_failure(child, meta, failure)) return 1;
        child = child->sibling;
    }

    return 0;
}

static void inline_meta_debug_collect_failure(Context *context,
                                              ASTNode *callable,
                                              Symbol *symbol,
                                              InlineMetaExport *meta,
                                              ASTNode *args,
                                              ASTNode *instrs) {
    InlineMetaCollectFailure failure;
    ASTNode *node;
    const char *node_type;

    memset(&failure, 0, sizeof(failure));
    if (inline_meta_find_collect_failure(args, meta, &failure) ||
        inline_meta_find_collect_failure(instrs, meta, &failure)) {
        node = failure.node ? failure.node : callable;
        node_type = node ? ast_ndtp(node->node_type) : "UNKNOWN";
        if (failure.symbol && failure.symbol->name) {
            inline_export_debug_reject(context,
                                       node,
                                       symbol,
                                       "%s: %s symbol=%s",
                                       failure.reason,
                                       node_type,
                                       failure.symbol->name);
        } else {
            inline_export_debug_reject(context,
                                       node,
                                       symbol,
                                       "%s: %s",
                                       failure.reason,
                                       node_type);
        }
        return;
    }

    inline_export_debug_reject(context,
                               callable,
                               symbol,
                               "inline metadata collection failed");
}

static int inline_meta_emit_scopes(InlineMetaText *text, InlineMetaExport *meta) {
    size_t i;

    for (i = 0; i < meta->scope_count; i++) {
        Scope *scope = meta->scopes[i].scope;
        long parent_id = meta->scopes[i].parent_id == (size_t)-1 ? -1L : (long)meta->scopes[i].parent_id;

        if (!inline_meta_text_appendf(text,
                                      ";q,%zu,%ld,%d",
                                      meta->scopes[i].id,
                                      parent_id,
                                      scope ? (int)scope->type : (int)SCOPE_LOCAL)) {
            return 0;
        }
    }

    return 1;
}

static int inline_meta_emit_files(InlineMetaText *text, InlineMetaExport *meta) {
    size_t i;

    for (i = 0; i < meta->file_count; i++) {
        char *file_hex = inline_meta_hex_encode(meta->files[i].file_name,
                                                meta->files[i].file_name ? strlen(meta->files[i].file_name) : 0);
        if (!file_hex) return 0;
        if (!inline_meta_text_appendf(text, ";f,%zu,%s", meta->files[i].id, file_hex)) {
            free(file_hex);
            return 0;
        }
        free(file_hex);
    }

    return 1;
}

static int inline_meta_emit_sources(InlineMetaText *text, InlineMetaExport *meta) {
    size_t i;

    for (i = 0; i < meta->source_count; i++) {
        InlineMetaSourceEntry *source = &meta->sources[i];
        long file_id = source->file_id == (size_t)-1 ? -1L : (long)source->file_id;
        char *source_hex = inline_meta_hex_encode(source->source_start, source->source_length);
        if (!source_hex) return 0;
        if (!inline_meta_text_appendf(text,
                                      ";u,%zu,%ld,%d,%d,%d,%d,%s",
                                      source->id,
                                      file_id,
                                      source->line,
                                      source->active_start_column,
                                      source->active_end_column,
                                      (int)source->provenance,
                                      source_hex)) {
            free(source_hex);
            return 0;
        }
        free(source_hex);
    }

    return 1;
}

static int inline_meta_emit_symbols(InlineMetaText *text, InlineMetaExport *meta) {
    size_t i;

    for (i = 0; i < meta->symbol_count; i++) {
        Symbol *symbol = meta->symbols[i].symbol;
        size_t scope_id;
        char *name_hex = inline_meta_hex_encode(symbol->name, strlen(symbol->name));
        char *base_text = inline_meta_int_list_encode(symbol->dim_base, symbol->value_dims, 1);
        char *elements_text = inline_meta_int_list_encode(symbol->dim_elements, symbol->value_dims, 0);
        char *class_hex = inline_meta_hex_encode(symbol->value_class,
                                                 symbol->value_class ? strlen(symbol->value_class) : 0);
        if (!name_hex || !base_text || !elements_text || !class_hex) {
            if (name_hex) free(name_hex);
            if (base_text) free(base_text);
            if (elements_text) free(elements_text);
            if (class_hex) free(class_hex);
            return 0;
        }
        if (!inline_meta_find_scope_id(meta, symbol->scope, &scope_id)) {
            free(name_hex);
            free(base_text);
            free(elements_text);
            free(class_hex);
            return 0;
        }

        if (!inline_meta_text_appendf(text,
                                      ";s,%zu,%zu,%s,%d,%d,%zu,%s,%s,%s,%u,%c,%d",
                                      meta->symbols[i].id,
                                      scope_id,
                                      name_hex,
                                      (int)symbol->symbol_type,
                                      (int)symbol->type,
                                      symbol->value_dims,
                                      base_text,
                                      elements_text,
                                      class_hex,
                                      inline_meta_symbol_flags(symbol),
                                      symbol->register_type ? symbol->register_type : '-',
                                      inline_class_attribute_register_num(symbol))) {
            free(name_hex);
            free(base_text);
            free(elements_text);
            free(class_hex);
            return 0;
        }
        free(name_hex);
        free(base_text);
        free(elements_text);
        free(class_hex);
    }

    return 1;
}

static int inline_meta_emit_dependencies(InlineMetaText *text, InlineMetaExport *meta) {
    size_t i;

    for (i = 0; i < meta->dependency_count; i++) {
        Symbol *symbol = meta->dependencies[i].symbol;
        ASTNode *proc = inline_meta_direct_dependency_proc(symbol);
        ASTNode *args = proc ? ast_chld(proc, ARGS, 0) : NULL;
        char *fqname = symbol ? sym_frnm(symbol) : NULL;
        char *type = proc ? callable_effective_return_type(proc) : NULL;
        char *arg_text = meta_narg(args);
        char *fqname_hex;
        char *type_hex;
        char *args_hex;

        if (!fqname || !type || !arg_text) {
            if (fqname) free(fqname);
            if (type) free(type);
            if (arg_text) free(arg_text);
            return 0;
        }

        fqname_hex = inline_meta_hex_encode(fqname, strlen(fqname));
        type_hex = inline_meta_hex_encode(type, strlen(type));
        args_hex = inline_meta_hex_encode(arg_text, strlen(arg_text));
        free(fqname);
        free(type);
        free(arg_text);
        if (!fqname_hex || !type_hex || !args_hex) {
            if (fqname_hex) free(fqname_hex);
            if (type_hex) free(type_hex);
            if (args_hex) free(args_hex);
            return 0;
        }

        if (!inline_meta_text_appendf(text,
                                      ";d,%zu,%s,%s,%s",
                                      meta->dependencies[i].id,
                                      fqname_hex,
                                      type_hex,
                                      args_hex)) {
            free(fqname_hex);
            free(type_hex);
            free(args_hex);
            return 0;
        }

        free(fqname_hex);
        free(type_hex);
        free(args_hex);
    }

    return 1;
}

static int inline_meta_emit_node(InlineMetaText *text, InlineMetaExport *meta, ASTNode *node) {
    ASTNode *child;
    size_t symbol_id;
    size_t dependency_id;
    size_t scope_id;
    size_t source_id;
    unsigned int symbol_read_usage;
    unsigned int symbol_write_usage;
    char *node_hex;
    char *decimal_hex;
    char *value_base_text;
    char *value_elements_text;
    char *target_base_text;
    char *target_elements_text;
    char *value_class_hex;
    char *target_class_hex;
    char int_buffer[64];

    if (!node) return 1;

    symbol_id = (size_t)-1;
    dependency_id = (size_t)-1;
    symbol_read_usage = 0;
    symbol_write_usage = 0;
    if (inline_meta_function_uses_direct_dependency(node)) {
        if (!inline_meta_find_dependency_id(meta, node->symbolNode->symbol, &dependency_id)) return 0;
    } else if (node->symbolNode && node->symbolNode->symbol) {
        if (!inline_meta_find_symbol_id(meta, node->symbolNode->symbol, &symbol_id)) return 0;
        symbol_read_usage = node->symbolNode->readUsage;
        symbol_write_usage = node->symbolNode->writeUsage;
    }
    if (node->scope) {
        if (!inline_meta_find_scope_id(meta, node->scope, &scope_id)) return 0;
    } else {
        scope_id = 0;
    }
    if (!inline_meta_find_source_id(meta, node, &source_id)) source_id = (size_t)-1;

    node_hex = inline_meta_hex_encode(node->node_string, node->node_string_length);
    decimal_hex = inline_meta_hex_encode(node->decimal_value, node->decimal_value ? strlen(node->decimal_value) : 0);
    value_base_text = inline_meta_int_list_encode(node->value_dim_base, node->value_dims, 1);
    value_elements_text = inline_meta_int_list_encode(node->value_dim_elements, node->value_dims, 0);
    target_base_text = inline_meta_int_list_encode(node->target_dim_base, node->target_dims, 1);
    target_elements_text = inline_meta_int_list_encode(node->target_dim_elements, node->target_dims, 0);
    value_class_hex = inline_meta_hex_encode(node->value_class, node->value_class ? strlen(node->value_class) : 0);
    target_class_hex = inline_meta_hex_encode(node->target_class, node->target_class ? strlen(node->target_class) : 0);
    if (!node_hex || !decimal_hex || !value_base_text || !value_elements_text ||
        !target_base_text || !target_elements_text || !value_class_hex || !target_class_hex) {
        if (node_hex) free(node_hex);
        if (decimal_hex) free(decimal_hex);
        if (value_base_text) free(value_base_text);
        if (value_elements_text) free(value_elements_text);
        if (target_base_text) free(target_base_text);
        if (target_elements_text) free(target_elements_text);
        if (value_class_hex) free(value_class_hex);
        if (target_class_hex) free(target_class_hex);
        return 0;
    }

#ifdef __32BIT__
    snprintf(int_buffer, sizeof(int_buffer), "%ld", node->int_value);
#else
    snprintf(int_buffer, sizeof(int_buffer), "%lld", (long long)node->int_value);
#endif

    if (!inline_meta_text_appendf(text,
                                  ";>,%zu,%ld,%d,%d,%d,%zu,%zu,%s,%s,%s,%s,%s,%s,%u,%ld,%ld,%u,%u,%s,%d,%.17g,%s,%s",
                                  scope_id,
                                  source_id == (size_t)-1 ? -1L : (long)source_id,
                                  (int)node->node_type,
                                  (int)node->value_type,
                                  (int)node->target_type,
                                  node->value_dims,
                                  node->target_dims,
                                  value_base_text,
                                  value_elements_text,
                                  target_base_text,
                                  target_elements_text,
                                  value_class_hex,
                                  target_class_hex,
                                  inline_meta_node_flags(node),
                                  symbol_id == (size_t)-1 ? -1L : (long)symbol_id,
                                  dependency_id == (size_t)-1 ? -1L : (long)dependency_id,
                                  symbol_read_usage,
                                  symbol_write_usage,
                                  int_buffer,
                                  node->bool_value,
                                  node->float_value,
                                  node_hex,
                                  decimal_hex)) {
        free(node_hex);
        free(decimal_hex);
        free(value_base_text);
        free(value_elements_text);
        free(target_base_text);
        free(target_elements_text);
        free(value_class_hex);
        free(target_class_hex);
        return 0;
    }

    free(node_hex);
    free(decimal_hex);
    free(value_base_text);
    free(value_elements_text);
    free(target_base_text);
    free(target_elements_text);
    free(value_class_hex);
    free(target_class_hex);

    child = node->child;
    while (child) {
        if (!inline_meta_emit_node(text, meta, child)) return 0;
        child = child->sibling;
    }

    return inline_meta_text_append(text, ";<");
}

char *rxcp_inline_export_payload(Context *context, ASTNode *callable) {
    Symbol *symbol;
    InlineEligibility eligibility;
    InlineMetaExport meta;
    InlineMetaText text;

    if (!callable || !inline_node_is_callable_def(callable)) return strdup("");
    if (callable->is_inline_pruned) {
        inline_export_debug_reject(context, callable, NULL, "callable already pruned");
        return strdup("");
    }

    symbol = inline_symbol_from_proc_def(callable);
    if (inline_proc_has_procedure_expose(callable)) {
        inline_export_debug_reject(context, callable, symbol, "procedure-level expose");
        return strdup("");
    }

    if (!symbol || !symbol->exposed) {
        inline_export_debug_reject(context, callable, symbol, "callable is not namespace-exposed");
        return strdup("");
    }
    if (symbol->is_main || symbol->is_implicit_main) {
        inline_export_debug_reject(context, callable, symbol, "main is not inline-exportable");
        return strdup("");
    }

    if (inline_analyse_callable_eligibility(context, callable, symbol, 1, 1, &eligibility) != INLINE_ELIGIBILITY_OK) {
        inline_export_debug_eligibility_reject(context, callable, symbol, &eligibility);
        return strdup("");
    }

    memset(&meta, 0, sizeof(meta));
    meta.root_scope = callable->scope;
    if (!inline_meta_collect_scope(&meta, callable->scope)) {
        inline_export_debug_reject(context, callable, symbol, "failed to collect callable scope");
        free(meta.scopes);
        return strdup("");
    }
    if (!inline_meta_collect(eligibility.args, &meta) ||
        !inline_meta_collect(eligibility.instrs, &meta)) {
        inline_meta_debug_collect_failure(context, callable, symbol, &meta, eligibility.args, eligibility.instrs);
        free(meta.scopes);
        free(meta.symbols);
        free(meta.files);
        free(meta.sources);
        free(meta.dependencies);
        return strdup("");
    }

    inline_meta_text_init(&text);
    if (!text.ok ||
        !inline_meta_text_append(&text, "I5") ||
        !inline_meta_emit_files(&text, &meta) ||
        !inline_meta_emit_sources(&text, &meta) ||
        !inline_meta_emit_scopes(&text, &meta) ||
        !inline_meta_emit_symbols(&text, &meta) ||
        !inline_meta_emit_dependencies(&text, &meta) ||
        !inline_meta_text_append(&text, ";a") ||
        !inline_meta_emit_node(&text, &meta, eligibility.args) ||
        !inline_meta_text_append(&text, ";b") ||
        !inline_meta_emit_node(&text, &meta, eligibility.instrs)) {
        inline_export_debug_reject(context, callable, symbol, "failed to emit inline metadata");
        free(meta.scopes);
        free(meta.symbols);
        free(meta.files);
        free(meta.sources);
        free(meta.dependencies);
        if (text.text) free(text.text);
        return strdup("");
    }

    free(meta.scopes);
    free(meta.symbols);
    free(meta.files);
    free(meta.sources);
    free(meta.dependencies);
    return text.text;
}

int rxcp_inline_payload_is_supported(const char *payload) {
    return payload && payload[0] == 'I' && (payload[1] == '4' || payload[1] == '5') &&
           (payload[2] == 0 || payload[2] == ';');
}

static ASTNode *inline_meta_find_first_procedure(ASTNode *node) {
    ASTNode *child;
    ASTNode *found;

    if (!node) return NULL;
    if (node->node_type == PROCEDURE) return node;

    child = node->child;
    while (child) {
        found = inline_meta_find_first_procedure(child);
        if (found) return found;
        child = child->sibling;
    }

    return NULL;
}

static char *inline_meta_next_field(char **cursor) {
    char *field;
    char *comma;

    if (!cursor || !*cursor) return NULL;
    field = *cursor;
    comma = strchr(field, ',');
    if (comma) {
        *comma = 0;
        *cursor = comma + 1;
    } else {
        *cursor = NULL;
    }
    return field;
}

static int inline_meta_ensure_symbol_slot(InlineMetaImport *meta, size_t id) {
    Symbol **new_symbols;
    size_t old_count;
    size_t i;

    if (!meta) return 0;
    if (id < meta->symbol_count) return 1;

    old_count = meta->symbol_count;
    new_symbols = realloc(meta->symbols, sizeof(Symbol *) * (id + 1));
    if (!new_symbols) return 0;

    meta->symbols = new_symbols;
    meta->symbol_count = id + 1;
    for (i = old_count; i < meta->symbol_count; i++) meta->symbols[i] = NULL;
    return 1;
}

static int inline_meta_ensure_scope_slot(InlineMetaImport *meta, size_t id) {
    Scope **new_scopes;
    size_t old_count;
    size_t i;

    if (!meta) return 0;
    if (id < meta->scope_count) return 1;

    old_count = meta->scope_count;
    new_scopes = realloc(meta->scopes, sizeof(Scope *) * (id + 1));
    if (!new_scopes) return 0;

    meta->scopes = new_scopes;
    meta->scope_count = id + 1;
    for (i = old_count; i < meta->scope_count; i++) meta->scopes[i] = NULL;
    return 1;
}

static int inline_meta_ensure_file_slot(InlineMetaImport *meta, size_t id) {
    char **new_files;
    size_t old_count;
    size_t i;

    if (!meta) return 0;
    if (id < meta->file_count) return 1;

    old_count = meta->file_count;
    new_files = realloc(meta->files, sizeof(char *) * (id + 1));
    if (!new_files) return 0;

    meta->files = new_files;
    meta->file_count = id + 1;
    for (i = old_count; i < meta->file_count; i++) meta->files[i] = NULL;
    return 1;
}

static int inline_meta_ensure_source_slot(InlineMetaImport *meta, size_t id) {
    InlineMetaImportedSource *new_sources;
    size_t old_count;
    size_t i;

    if (!meta) return 0;
    if (id < meta->source_count) return 1;

    old_count = meta->source_count;
    new_sources = realloc(meta->sources, sizeof(InlineMetaImportedSource) * (id + 1));
    if (!new_sources) return 0;

    meta->sources = new_sources;
    meta->source_count = id + 1;
    for (i = old_count; i < meta->source_count; i++) {
        meta->sources[i].source_node = NULL;
        meta->sources[i].provenance = AST_SOURCE_NONE;
    }
    return 1;
}

static int inline_meta_ensure_dependency_slot(InlineMetaImport *meta, size_t id) {
    InlineMetaImportedDependency *new_dependencies;
    size_t old_count;
    size_t i;

    if (!meta) return 0;
    if (id < meta->dependency_count) return 1;

    old_count = meta->dependency_count;
    new_dependencies = realloc(meta->dependencies, sizeof(InlineMetaImportedDependency) * (id + 1));
    if (!new_dependencies) return 0;

    meta->dependencies = new_dependencies;
    meta->dependency_count = id + 1;
    for (i = old_count; i < meta->dependency_count; i++) {
        meta->dependencies[i].fqname = NULL;
        meta->dependencies[i].type = NULL;
        meta->dependencies[i].args = NULL;
        meta->dependencies[i].symbol = NULL;
    }
    return 1;
}

static Scope *inline_meta_scope_by_id(InlineMetaImport *meta, size_t id) {
    if (!meta) return NULL;
    if (id >= meta->scope_count) return NULL;
    return meta->scopes[id];
}

static int inline_meta_import_file(Context *context, InlineMetaImport *meta, char *record) {
    char *cursor;
    char *id_field;
    char *name_field;
    char *name;
    size_t id;
    size_t name_length;

    (void)context;
    if (!meta || !record) return 0;

    cursor = record + 2;
    id_field = inline_meta_next_field(&cursor);
    name_field = inline_meta_next_field(&cursor);
    if (!id_field || !name_field) return 0;

    id = (size_t)strtoul(id_field, NULL, 10);
    if (!inline_meta_ensure_file_slot(meta, id)) return 0;

    name = inline_meta_hex_decode(name_field, &name_length);
    if (!name) return 0;

    if (meta->files[id]) free(meta->files[id]);
    meta->files[id] = name;
    return 1;
}

static int inline_meta_import_source(Context *context, InlineMetaImport *meta, char *record) {
    char *cursor;
    char *id_field;
    char *file_field;
    char *line_field;
    char *start_column_field;
    char *end_column_field;
    char *provenance_field;
    char *source_field;
    char *source_text;
    char *file_name;
    size_t id;
    size_t source_length;
    size_t start_offset;
    size_t end_offset;
    long file_id;
    int start_column;
    int end_column;
    SourceNode *source_node;

    if (!context || !meta || !record) return 0;

    cursor = record + 2;
    id_field = inline_meta_next_field(&cursor);
    file_field = inline_meta_next_field(&cursor);
    line_field = inline_meta_next_field(&cursor);
    start_column_field = inline_meta_next_field(&cursor);
    end_column_field = NULL;
    if (meta->version >= 5) end_column_field = inline_meta_next_field(&cursor);
    provenance_field = inline_meta_next_field(&cursor);
    source_field = inline_meta_next_field(&cursor);
    if (!id_field || !file_field || !line_field || !start_column_field || !provenance_field || !source_field) return 0;
    if (meta->version >= 5 && !end_column_field) return 0;

    id = (size_t)strtoul(id_field, NULL, 10);
    if (!inline_meta_ensure_source_slot(meta, id)) return 0;

    file_name = NULL;
    file_id = strtol(file_field, NULL, 10);
    if (file_id >= 0) {
        if ((size_t)file_id >= meta->file_count || !meta->files[file_id]) return 0;
        file_name = meta->files[file_id];
    }

    source_text = inline_meta_hex_decode(source_field, &source_length);
    if (!source_text) return 0;

    source_node = calloc(1, sizeof(SourceNode));
    if (!source_node) {
        free(source_text);
        return 0;
    }

    source_node->context = context;
    source_node->node_type = NOP;
    source_node->owned_file_name = file_name ? strdup(file_name) : NULL;
    source_node->file_name = source_node->owned_file_name;
    source_node->owned_source_text = source_text;
    source_node->line = atoi(line_field);
    if (meta->version >= 5) {
        start_column = atoi(start_column_field);
        end_column = atoi(end_column_field);
        if (start_column < 1) start_column = 1;
        if (end_column < start_column) end_column = start_column;
        start_offset = (size_t)(start_column - 1);
        end_offset = (size_t)(end_column - 1);
        if (start_offset > source_length) start_offset = source_length;
        if (end_offset > source_length) end_offset = source_length;
        if (end_offset < start_offset) end_offset = start_offset;
        source_node->source_start = source_text + start_offset;
        if (end_offset > start_offset) source_node->source_end = source_text + end_offset - 1;
        else source_node->source_end = source_node->source_start;
        source_node->column = start_column - 1;
    } else {
        source_node->source_start = source_text;
        source_node->source_end = source_length ? source_text + source_length - 1 : source_text;
        source_node->column = atoi(start_column_field);
    }
    source_node->free_list = context->source_free_list;
    if (source_node->free_list) source_node->node_number = source_node->free_list->node_number + 1;
    else source_node->node_number = 1;
    context->source_free_list = source_node;

    meta->sources[id].source_node = source_node;
    meta->sources[id].provenance = (char)atoi(provenance_field);
    return 1;
}

static int inline_meta_import_scope(Context *context, InlineMetaImport *meta, char *record) {
    char *cursor;
    char *id_field;
    char *parent_field;
    char *type_field;
    size_t id;
    long parent_id;
    ScopeType type;
    Scope *parent;
    Scope *scope;

    if (!context || !meta || !record) return 0;

    cursor = record + 2;
    id_field = inline_meta_next_field(&cursor);
    parent_field = inline_meta_next_field(&cursor);
    type_field = inline_meta_next_field(&cursor);
    if (!id_field || !parent_field || !type_field) return 0;

    id = (size_t)strtoul(id_field, NULL, 10);
    parent_id = strtol(parent_field, NULL, 10);
    type = (ScopeType)atoi(type_field);

    if (!inline_meta_ensure_scope_slot(meta, id)) return 0;

    if (id == 0) {
        if (type != SCOPE_PROCEDURE) return 0;
        meta->scopes[0] = meta->scope;
        return 1;
    }

    if (type == SCOPE_CLASS && parent_id < 0) {
        if (!meta->scope || !meta->scope->parent || meta->scope->parent->type != SCOPE_CLASS) return 0;
        meta->scopes[id] = meta->scope->parent;
        return 1;
    }

    if (type != SCOPE_LOCAL || parent_id < 0) return 0;
    parent = inline_meta_scope_by_id(meta, (size_t)parent_id);
    if (!parent) return 0;

    if (meta->scopes[id]) return 1;

    scope = scp_f(context, parent, NULL, NULL, type);
    if (!scope) return 0;

    meta->scopes[id] = scope;
    return 1;
}

static Symbol *inline_meta_find_or_create_symbol(Context *context,
                                                 Scope *scope,
                                                 const char *name,
                                                 ValueType type,
                                                 size_t dims,
                                                 int *dim_base,
                                                 int *dim_elements,
                                                 const char *class_name,
                                                 unsigned int flags) {
    ASTNode lookup_node;
    Symbol *symbol;

    if (!context || !scope || !name) return NULL;

    memset(&lookup_node, 0, sizeof(lookup_node));
    lookup_node.node_string = (char *)name;
    lookup_node.node_string_length = strlen(name);

    symbol = sym_lrsv(scope, &lookup_node);
    if (!symbol) symbol = sym_fn(scope, name, strlen(name));
    if (!symbol) return NULL;

    if (!inline_meta_set_symbol_shape(symbol, type, dims, dim_base, dim_elements, class_name)) return NULL;
    if (symbol->symbol_type == UNKNOWN_SYMBOL) symbol->symbol_type = VARIABLE_SYMBOL;
    if (symbol->status == SYM_STATUS_UNRESOLVED) symbol->status = SYM_STATUS_LOCAL_VAR;
    inline_meta_apply_symbol_flags(symbol, flags);

    return symbol;
}

static int inline_meta_import_symbol(Context *context, InlineMetaImport *meta, char *record) {
    char *cursor;
    char *id_field;
    char *scope_field;
    char *name_field;
    char *symbol_kind_field;
    char *type_field;
    char *dims_field;
    char *base_field;
    char *elements_field;
    char *class_field;
    char *flags_field;
    char *register_type_field;
    char *register_num_field;
    size_t id;
    size_t scope_id;
    size_t name_length;
    size_t dims;
    int *dim_base;
    int *dim_elements;
    char *class_name;
    char *name;
    Scope *scope;
    Symbol *symbol;

    cursor = record + 2;
    id_field = inline_meta_next_field(&cursor);
    scope_field = NULL;
    if (meta->version >= 2) scope_field = inline_meta_next_field(&cursor);
    name_field = inline_meta_next_field(&cursor);
    symbol_kind_field = NULL;
    if (meta->version >= 4) symbol_kind_field = inline_meta_next_field(&cursor);
    type_field = inline_meta_next_field(&cursor);
    dims_field = inline_meta_next_field(&cursor);
    base_field = NULL;
    elements_field = NULL;
    class_field = NULL;
    register_type_field = NULL;
    register_num_field = NULL;
    if (meta->version >= 3) {
        base_field = inline_meta_next_field(&cursor);
        elements_field = inline_meta_next_field(&cursor);
        class_field = inline_meta_next_field(&cursor);
    }
    flags_field = inline_meta_next_field(&cursor);
    if (meta->version >= 3) {
        register_type_field = inline_meta_next_field(&cursor);
        register_num_field = inline_meta_next_field(&cursor);
    }
    if (!id_field || !name_field || !type_field || !dims_field || !flags_field) return 0;
    if (meta->version >= 3 && (!base_field || !elements_field || !class_field)) return 0;

    id = (size_t)strtoul(id_field, NULL, 10);
    if (!inline_meta_ensure_symbol_slot(meta, id)) return 0;

    scope_id = 0;
    if (scope_field) scope_id = (size_t)strtoul(scope_field, NULL, 10);
    scope = meta->version >= 2 ? inline_meta_scope_by_id(meta, scope_id) : meta->scope;
    if (!scope) return 0;

    name = inline_meta_hex_decode(name_field, &name_length);
    if (!name) return 0;

    dims = (size_t)strtoul(dims_field, NULL, 10);
    dim_base = meta->version >= 3 ? inline_meta_int_list_decode(base_field, dims, 1) : NULL;
    dim_elements = meta->version >= 3 ? inline_meta_int_list_decode(elements_field, dims, 0) : NULL;
    class_name = meta->version >= 3 ? inline_meta_decode_optional_hex(class_field) : NULL;
    if (dims && (!dim_base || !dim_elements)) {
        free(name);
        if (dim_base) free(dim_base);
        if (dim_elements) free(dim_elements);
        if (class_name) free(class_name);
        return 0;
    }

    symbol = inline_meta_find_or_create_symbol(context,
                                               scope,
                                               name,
                                               (ValueType)atoi(type_field),
                                               dims,
                                               dim_base,
                                               dim_elements,
                                               class_name,
                                               (unsigned int)strtoul(flags_field, NULL, 10));
    free(name);
    if (dim_base) free(dim_base);
    if (dim_elements) free(dim_elements);
    if (class_name) free(class_name);
    if (!symbol) return 0;
    if (symbol_kind_field) symbol->symbol_type = (SymbolType)atoi(symbol_kind_field);

    if (inline_symbol_is_class_attribute(symbol) &&
        register_type_field && register_type_field[0] &&
        register_num_field && register_num_field[0]) {
        symbol->register_type = register_type_field[0];
        symbol->register_num = atoi(register_num_field);
    }

    meta->symbols[id] = symbol;
    return 1;
}

static Symbol *inline_meta_resolve_dependency_symbol(Context *context,
                                                     InlineMetaImport *meta,
                                                     const char *fqname) {
    Symbol *symbol;
    ASTNode *lookup;
    char *lookup_name;

    if (!context || !context->ast || !fqname || !*fqname) return NULL;

    symbol = sym_rfqn(context->ast, fqname);
    if (symbol && symbol->symbol_type == FUNCTION_SYMBOL) return symbol;

    symbol = sym_rfqv(context->ast, fqname);
    if (symbol && symbol->symbol_type == FUNCTION_SYMBOL) return symbol;

    lookup_name = strdup(fqname);
    if (!lookup_name) return NULL;

    lookup = ast_ftt(context, FUNCTION, lookup_name);
    if (!lookup) {
        free(lookup_name);
        return NULL;
    }
    lookup->free_node_string = 1;
    lookup->scope = meta ? meta->scope : NULL;

    symbol = sym_imfn(context, lookup);
    if (symbol && symbol->symbol_type == FUNCTION_SYMBOL) return symbol;

    symbol = sym_rfqn(context->ast, fqname);
    if (symbol && symbol->symbol_type == FUNCTION_SYMBOL) return symbol;

    return NULL;
}

static int inline_meta_nullable_strings_equal(const char *left, const char *right) {
    if (!left || !right) return left == right || (!left && right && !*right) || (!right && left && !*left);
    return strcmp(left, right) == 0;
}

static int inline_meta_dependency_signature_matches(Context *context,
                                                    const char *fqname,
                                                    const char *type,
                                                    const char *args) {
    imported_func *func;
    char *lookup_name;
    int found;

    if (!context || !fqname) return 0;

    lookup_name = strdup(fqname);
    if (!lookup_name) return 0;
    func = NULL;
    found = src_fqfu(context, 0, lookup_name, &func);
    free(lookup_name);

    if (!found || !func) return 1;
    if (func->is_variable) return 0;
    if (!inline_meta_nullable_strings_equal(func->type, type)) return 0;
    if (!inline_meta_nullable_strings_equal(func->args, args)) return 0;
    return 1;
}

static int inline_meta_import_dependency(Context *context, InlineMetaImport *meta, char *record) {
    char *cursor;
    char *id_field;
    char *fqname_field;
    char *type_field;
    char *args_field;
    size_t id;
    size_t length;
    char *fqname;
    char *type;
    char *args;
    Symbol *symbol;

    if (!context || !meta || !record) return 0;

    cursor = record + 2;
    id_field = inline_meta_next_field(&cursor);
    fqname_field = inline_meta_next_field(&cursor);
    type_field = inline_meta_next_field(&cursor);
    args_field = inline_meta_next_field(&cursor);
    if (!id_field || !fqname_field || !type_field || !args_field) return 0;

    id = (size_t)strtoul(id_field, NULL, 10);
    if (!inline_meta_ensure_dependency_slot(meta, id)) return 0;

    fqname = inline_meta_hex_decode(fqname_field, &length);
    type = inline_meta_hex_decode(type_field, &length);
    args = inline_meta_hex_decode(args_field, &length);
    if (!fqname || !type || !args) {
        if (fqname) free(fqname);
        if (type) free(type);
        if (args) free(args);
        return 0;
    }

    symbol = inline_meta_resolve_dependency_symbol(context, meta, fqname);
    if (!symbol || !inline_meta_dependency_signature_matches(context, fqname, type, args)) {
        free(fqname);
        free(type);
        free(args);
        return 0;
    }

    if (meta->dependencies[id].fqname) free(meta->dependencies[id].fqname);
    if (meta->dependencies[id].type) free(meta->dependencies[id].type);
    if (meta->dependencies[id].args) free(meta->dependencies[id].args);
    meta->dependencies[id].fqname = fqname;
    meta->dependencies[id].type = type;
    meta->dependencies[id].args = args;
    meta->dependencies[id].symbol = symbol;
    return 1;
}

static int inline_meta_push_node(InlineMetaImport *meta, ASTNode *node) {
    ASTNode **new_stack;

    new_stack = realloc(meta->stack, sizeof(ASTNode *) * (meta->stack_count + 1));
    if (!new_stack) return 0;

    meta->stack = new_stack;
    meta->stack[meta->stack_count] = node;
    meta->stack_count++;
    return 1;
}

static int inline_meta_import_node(Context *context, InlineMetaImport *meta, char *record) {
    char *cursor;
    char *scope_field;
    char *source_field;
    char *node_type_field;
    char *value_type_field;
    char *target_type_field;
    char *value_dims_field;
    char *target_dims_field;
    char *value_base_field;
    char *value_elements_field;
    char *target_base_field;
    char *target_elements_field;
    char *value_class_field;
    char *target_class_field;
    char *flags_field;
    char *symbol_field;
    char *dependency_field;
    char *symbol_read_field;
    char *symbol_write_field;
    char *int_field;
    char *bool_field;
    char *float_field;
    char *node_string_field;
    char *decimal_field;
    ASTNode *node;
    char *node_string;
    char *decimal_string;
    size_t node_string_length;
    size_t decimal_length;
    size_t scope_id;
    size_t value_dims;
    size_t target_dims;
    int *value_dim_base;
    int *value_dim_elements;
    int *target_dim_base;
    int *target_dim_elements;
    char *value_class;
    char *target_class;
    long symbol_id;
    long source_id;
    long dependency_id;
    unsigned int symbol_read_usage;
    unsigned int symbol_write_usage;
    unsigned int flags;
    Scope *node_scope;

    cursor = record + 2;
    scope_field = NULL;
    if (meta->version >= 2) scope_field = inline_meta_next_field(&cursor);
    source_field = NULL;
    if (meta->version >= 4) source_field = inline_meta_next_field(&cursor);
    node_type_field = inline_meta_next_field(&cursor);
    value_type_field = inline_meta_next_field(&cursor);
    target_type_field = inline_meta_next_field(&cursor);
    value_dims_field = inline_meta_next_field(&cursor);
    target_dims_field = inline_meta_next_field(&cursor);
    value_base_field = NULL;
    value_elements_field = NULL;
    target_base_field = NULL;
    target_elements_field = NULL;
    value_class_field = NULL;
    target_class_field = NULL;
    if (meta->version >= 3) {
        value_base_field = inline_meta_next_field(&cursor);
        value_elements_field = inline_meta_next_field(&cursor);
        target_base_field = inline_meta_next_field(&cursor);
        target_elements_field = inline_meta_next_field(&cursor);
        value_class_field = inline_meta_next_field(&cursor);
        target_class_field = inline_meta_next_field(&cursor);
    }
    flags_field = inline_meta_next_field(&cursor);
    symbol_field = inline_meta_next_field(&cursor);
    dependency_field = NULL;
    if (meta->version >= 4) dependency_field = inline_meta_next_field(&cursor);
    symbol_read_field = NULL;
    symbol_write_field = NULL;
    if (meta->version >= 3) {
        symbol_read_field = inline_meta_next_field(&cursor);
        symbol_write_field = inline_meta_next_field(&cursor);
    }
    int_field = inline_meta_next_field(&cursor);
    bool_field = inline_meta_next_field(&cursor);
    float_field = inline_meta_next_field(&cursor);
    node_string_field = inline_meta_next_field(&cursor);
    decimal_field = inline_meta_next_field(&cursor);
    if (!node_type_field || !value_type_field || !target_type_field ||
        !value_dims_field || !target_dims_field || !flags_field ||
        !symbol_field || !int_field || !bool_field || !float_field ||
        !node_string_field || !decimal_field) {
        return 0;
    }
    if (meta->version >= 4 && !dependency_field) return 0;
    if (meta->version >= 3 &&
        (!value_base_field || !value_elements_field || !target_base_field ||
         !target_elements_field || !value_class_field || !target_class_field ||
         !symbol_read_field || !symbol_write_field)) {
        return 0;
    }
    dependency_id = -1;
    if (meta->version >= 4) {
        dependency_id = strtol(dependency_field, NULL, 10);
        if (dependency_id < -1) return 0;
    }

    scope_id = 0;
    if (scope_field) scope_id = (size_t)strtoul(scope_field, NULL, 10);
    node_scope = meta->version >= 2 ? inline_meta_scope_by_id(meta, scope_id) : meta->scope;
    if (!node_scope) return 0;

    node = ast_ft(context, (NodeType)atoi(node_type_field));
    if (!node) return 0;

    value_dims = (size_t)strtoul(value_dims_field, NULL, 10);
    target_dims = (size_t)strtoul(target_dims_field, NULL, 10);
    value_dim_base = meta->version >= 3 ? inline_meta_int_list_decode(value_base_field, value_dims, 1) : NULL;
    value_dim_elements = meta->version >= 3 ? inline_meta_int_list_decode(value_elements_field, value_dims, 0) : NULL;
    target_dim_base = meta->version >= 3 ? inline_meta_int_list_decode(target_base_field, target_dims, 1) : NULL;
    target_dim_elements = meta->version >= 3 ? inline_meta_int_list_decode(target_elements_field, target_dims, 0) : NULL;
    value_class = meta->version >= 3 ? inline_meta_decode_optional_hex(value_class_field) : NULL;
    target_class = meta->version >= 3 ? inline_meta_decode_optional_hex(target_class_field) : NULL;
    if ((value_dims && (!value_dim_base || !value_dim_elements)) ||
        (target_dims && (!target_dim_base || !target_dim_elements))) {
        if (value_dim_base) free(value_dim_base);
        if (value_dim_elements) free(value_dim_elements);
        if (target_dim_base) free(target_dim_base);
        if (target_dim_elements) free(target_dim_elements);
        if (value_class) free(value_class);
        if (target_class) free(target_class);
        return 0;
    }
    ast_set_value_type(0,
                       node,
                       (ValueType)atoi(value_type_field),
                       value_dims,
                       value_dim_base,
                       value_dim_elements,
                       value_class);
    ast_set_target_type(0,
                        node,
                        (ValueType)atoi(target_type_field),
                        target_dims,
                        target_dim_base,
                        target_dim_elements,
                        target_class);
    if (value_dim_base) free(value_dim_base);
    if (value_dim_elements) free(value_dim_elements);
    if (target_dim_base) free(target_dim_base);
    if (target_dim_elements) free(target_dim_elements);
    if (value_class) free(value_class);
    if (target_class) free(target_class);
    node->int_value = (rxinteger)atoll(int_field);
    node->bool_value = atoi(bool_field);
    node->float_value = atof(float_field);
    node->scope = node_scope;
    flags = (unsigned int)strtoul(flags_field, NULL, 10);
    inline_meta_apply_node_flags(node, flags);
    if ((flags & INLINE_META_NODE_SCOPE_DEF) && node_scope != meta->scope) node_scope->defining_node = node;

    source_id = source_field ? strtol(source_field, NULL, 10) : -1;
    if (source_id >= 0) {
        SourceNode *source_node;
        if ((size_t)source_id >= meta->source_count) return 0;
        source_node = meta->sources[source_id].source_node;
        if (!source_node) return 0;
        node->file_name = source_node->file_name;
        node->line = source_node->line;
        node->column = source_node->column;
        node->source_start = source_node->source_start;
        node->source_end = source_node->source_end;
        ast_set_primary_source_node(node,
                                    source_node,
                                    (ASTSourceProvenance)meta->sources[source_id].provenance);
    }

    node_string = inline_meta_hex_decode(node_string_field, &node_string_length);
    decimal_string = inline_meta_hex_decode(decimal_field, &decimal_length);
    if (!node_string || !decimal_string) {
        if (node_string) free(node_string);
        if (decimal_string) free(decimal_string);
        return 0;
    }

    ast_sstr(node, node_string, node_string_length);
    if (decimal_length) node->decimal_value = decimal_string;
    else free(decimal_string);

    symbol_id = strtol(symbol_field, NULL, 10);
    if (dependency_id >= 0) {
        Symbol *symbol;

        if (symbol_id >= 0 || node->node_type != FUNCTION) return 0;
        if ((size_t)dependency_id >= meta->dependency_count) return 0;
        symbol = meta->dependencies[dependency_id].symbol;
        if (!symbol || symbol->symbol_type != FUNCTION_SYMBOL) return 0;
        sym_adnd(symbol, node, 1, 0);
    }
    if (symbol_id >= 0) {
        Symbol *symbol;
        if ((size_t)symbol_id >= meta->symbol_count) return 0;
        symbol = meta->symbols[symbol_id];
        if (!symbol) return 0;

        if (meta->version >= 3) {
            symbol_read_usage = (unsigned int)strtoul(symbol_read_field, NULL, 10);
            symbol_write_usage = (unsigned int)strtoul(symbol_write_field, NULL, 10);
        } else if (node->node_type == VAR_TARGET) {
            symbol_read_usage = 0;
            symbol_write_usage = 1;
        } else {
            symbol_read_usage = 1;
            symbol_write_usage = 0;
        }
        sym_adnd(symbol, node, symbol_read_usage, symbol_write_usage);
    }

    if (meta->stack_count) add_ast(meta->stack[meta->stack_count - 1], node);
    else if (meta->version >= 3) {
        if (meta->tree_section == 1) meta->args_root = node;
        else if (meta->tree_section == 2) meta->root = node;
        else return 0;
    } else {
        meta->root = node;
    }

    return inline_meta_push_node(meta, node);
}

static Symbol *inline_meta_clone_signature_symbol(Context *context, Scope *scope, Symbol *old_symbol) {
    Symbol *symbol;

    if (!context || !scope || !old_symbol || !old_symbol->name) return NULL;

    symbol = inline_meta_find_or_create_symbol(context,
                                               scope,
                                               old_symbol->name,
                                               old_symbol->type,
                                               old_symbol->value_dims,
                                               old_symbol->dim_base,
                                               old_symbol->dim_elements,
                                               old_symbol->value_class,
                                               inline_meta_symbol_flags(old_symbol));
    if (!symbol) return NULL;

    symbol->symbol_type = old_symbol->symbol_type;
    symbol->status = old_symbol->status;
    symbol->fixed_args = old_symbol->fixed_args;
    symbol->has_vargs = old_symbol->has_vargs;
    symbol->needs_default_initiation = old_symbol->needs_default_initiation;
    symbol->register_num = UNSET_REGISTER;
    symbol->register_type = 'r';
    return symbol;
}

static ASTNode *inline_meta_clone_signature_with_symbols(Context *context, ASTNode *node, Scope *scope) {
    ASTNode *clone;
    ASTNode *child;

    if (!context || !node || !scope) return NULL;

    clone = ast_dup(context, node);
    if (!clone) return NULL;
    clone->scope = scope;
    if (node->symbolNode && node->symbolNode->symbol) {
        Symbol *symbol = inline_meta_clone_signature_symbol(context, scope, node->symbolNode->symbol);
        if (!symbol) return NULL;
        sym_adnd(symbol, clone, node->symbolNode->readUsage, node->symbolNode->writeUsage);
    }

    child = node->child;
    while (child) {
        ASTNode *child_clone = inline_meta_clone_signature_with_symbols(context, child, scope);
        if (!child_clone) return NULL;
        add_ast(clone, child_clone);
        child = child->sibling;
    }

    return clone;
}

static int inline_meta_clone_missing_scope_symbols(Scope *source, Scope *target) {
    Symbol **symbols;
    size_t i;

    if (!source || !target) return 0;

    symbols = scp_syms(source);
    if (!symbols) return 1;

    for (i = 0; symbols[i]; i++) {
        ASTNode lookup_node;
        Symbol *old_symbol;
        Symbol *new_symbol;

        old_symbol = symbols[i];
        if (!old_symbol || old_symbol->symbol_type == FUNCTION_SYMBOL || !old_symbol->name) continue;

        memset(&lookup_node, 0, sizeof(lookup_node));
        lookup_node.node_string = old_symbol->name;
        lookup_node.node_string_length = strlen(old_symbol->name);
        if (sym_lrsv(target, &lookup_node)) continue;

        new_symbol = sym_dup(target, old_symbol);
        if (!new_symbol) {
            free(symbols);
            return 0;
        }

        new_symbol->register_num = UNSET_REGISTER;
        new_symbol->register_type = 'r';
        new_symbol->meta_emitted = 0;
        new_symbol->init_emitted = 0;
        new_symbol->defines_scope = NULL;
        new_symbol->ast_template = NULL;
        new_symbol->is_inlinable = 0;
    }

    free(symbols);
    return 1;
}

static ASTNode *inline_meta_create_template_proc(Context *context,
                                                 ASTNode *proc,
                                                 Scope **scope_out,
                                                 int skip_args) {
    ASTNode *template_proc;
    ASTNode *child;
    Scope *template_scope;

    if (scope_out) *scope_out = NULL;
    if (!context || !proc || !proc->scope || !proc->symbolNode || !proc->symbolNode->symbol) return NULL;

    template_proc = ast_dup(context, proc);
    if (!template_proc) return NULL;

    template_scope = scp_f(context,
                           (proc->node_type == METHOD || proc->node_type == FACTORY) ? proc->scope->parent : NULL,
                           template_proc,
                           NULL,
                           SCOPE_PROCEDURE);
    if (!template_scope) return NULL;
    if (!template_scope->parent && !scp_track_detached(context, template_scope)) return NULL;
    if (proc->node_type == METHOD || proc->node_type == FACTORY) template_proc->parent = proc->parent;
    if (proc->scope->name) template_scope->name = strdup(proc->scope->name);
    inline_copy_numeric_context(template_scope, proc->scope);

    template_proc->scope = template_scope;
    if (!inline_meta_clone_missing_scope_symbols(proc->scope, template_scope)) return NULL;
    sym_adnd(proc->symbolNode->symbol,
             template_proc,
             proc->symbolNode->readUsage,
             proc->symbolNode->writeUsage);

    child = proc->child;
    while (child) {
        if (child->node_type != INSTRUCTIONS && child->node_type != NOP &&
            (!skip_args || child->node_type != ARGS)) {
            ASTNode *child_clone = inline_meta_clone_signature_with_symbols(context, child, template_scope);
            if (!child_clone) return NULL;
            add_ast(template_proc, child_clone);
        }
        child = child->sibling;
    }

    if (scope_out) *scope_out = template_scope;
    return template_proc;
}

static int inline_meta_finish_template(ASTNode *proc,
                                       ASTNode *template_proc,
                                       ASTNode *args_root,
                                       ASTNode *body_root) {
    if (!proc || !template_proc || !proc->symbolNode || !proc->symbolNode->symbol || !body_root) return 0;

    if (args_root) add_ast(template_proc, args_root);
    add_ast(template_proc, body_root);

    proc->symbolNode->symbol->is_inlinable = 1;
    proc->symbolNode->symbol->ast_template = template_proc;
    return 1;
}

static char *inline_meta_next_record(char **cursor) {
    char *record;
    char *separator;

    if (!cursor || !*cursor) return NULL;

    record = *cursor;
    separator = strchr(record, ';');
    if (separator) {
        *separator = '\0';
        *cursor = separator + 1;
    } else {
        *cursor = NULL;
    }

    return record;
}

static int inline_meta_attach_to_proc(Context *context, ASTNode *proc, const char *payload) {
    InlineMetaImport meta;
    ASTNode *template_proc;
    Scope *template_scope;
    char *copy;
    char *cursor;
    char *record;

    if (!context || !rxcp_inline_payload_is_supported(payload)) return 0;
    if (!proc || !proc->scope || !proc->symbolNode || !proc->symbolNode->symbol) return 0;

    memset(&meta, 0, sizeof(meta));
    meta.ok = 1;

    copy = strdup(payload);
    if (!copy) return 0;

    cursor = copy;
    record = inline_meta_next_record(&cursor);
    if (!record) {
        free(copy);
        return 0;
    }
    if (strcmp(record, "I1") == 0) meta.version = 1;
    else if (strcmp(record, "I2") == 0) meta.version = 2;
    else if (strcmp(record, "I3") == 0) meta.version = 3;
    else if (strcmp(record, "I4") == 0) meta.version = 4;
    else if (strcmp(record, "I5") == 0) meta.version = 5;
    else {
        free(copy);
        return 0;
    }

    template_scope = NULL;
    template_proc = inline_meta_create_template_proc(context, proc, &template_scope, meta.version >= 3);
    if (!template_proc || !template_scope) {
        free(copy);
        return 0;
    }

    meta.scope = template_scope;
    meta.tree_section = meta.version >= 3 ? 0 : 2;

    if (!inline_meta_ensure_scope_slot(&meta, 0)) {
        free(copy);
        return 0;
    }
    meta.scopes[0] = meta.scope;

    while ((record = inline_meta_next_record(&cursor)) != NULL) {
        if (strcmp(record, "<") == 0) {
            if (!meta.stack_count) {
                meta.ok = 0;
                break;
            }
            meta.stack_count--;
        } else if (strcmp(record, "a") == 0) {
            if (meta.version < 3 || meta.stack_count || meta.args_root) {
                meta.ok = 0;
                break;
            }
            meta.tree_section = 1;
        } else if (strcmp(record, "b") == 0) {
            if (meta.version < 3 || meta.stack_count || meta.root) {
                meta.ok = 0;
                break;
            }
            meta.tree_section = 2;
        } else if (record[0] == 'q' && record[1] == ',') {
            if (meta.version < 2 || !inline_meta_import_scope(context, &meta, record)) {
                meta.ok = 0;
                break;
            }
        } else if (record[0] == 'f' && record[1] == ',') {
            if (meta.version < 4 || !inline_meta_import_file(context, &meta, record)) {
                meta.ok = 0;
                break;
            }
        } else if (record[0] == 'u' && record[1] == ',') {
            if (meta.version < 4 || !inline_meta_import_source(context, &meta, record)) {
                meta.ok = 0;
                break;
            }
        } else if (record[0] == 's' && record[1] == ',') {
            if (!inline_meta_import_symbol(context, &meta, record)) {
                meta.ok = 0;
                break;
            }
        } else if (record[0] == 'd' && record[1] == ',') {
            if (meta.version < 4 || !inline_meta_import_dependency(context, &meta, record)) {
                meta.ok = 0;
                break;
            }
        } else if (record[0] == '>' && record[1] == ',') {
            if (!inline_meta_import_node(context, &meta, record)) {
                meta.ok = 0;
                break;
            }
        } else {
            meta.ok = 0;
            break;
        }
    }

    if (meta.stack_count != 0 || !meta.root || meta.root->node_type != INSTRUCTIONS) meta.ok = 0;
    if (meta.version >= 3 && (!meta.args_root || meta.args_root->node_type != ARGS)) meta.ok = 0;

    free(copy);
    if (meta.files) {
        size_t i;
        for (i = 0; i < meta.file_count; i++) {
            if (meta.files[i]) free(meta.files[i]);
        }
    }
    free(meta.files);
    free(meta.sources);
    if (meta.dependencies) {
        size_t i;
        for (i = 0; i < meta.dependency_count; i++) {
            if (meta.dependencies[i].fqname) free(meta.dependencies[i].fqname);
            if (meta.dependencies[i].type) free(meta.dependencies[i].type);
            if (meta.dependencies[i].args) free(meta.dependencies[i].args);
        }
    }
    free(meta.dependencies);
    free(meta.symbols);
    free(meta.scopes);
    free(meta.stack);

    if (!meta.ok) return 0;

    return inline_meta_finish_template(proc,
                                       template_proc,
                                       meta.version >= 3 ? meta.args_root : NULL,
                                       meta.root);
}

int rxcp_inline_attach_imported_body(Context *context, const char *payload) {
    ASTNode *proc;

    if (!context || !context->ast || !rxcp_inline_payload_is_supported(payload)) return 0;

    proc = inline_meta_find_first_procedure(context->ast);
    if (!proc) return 0;

    return inline_meta_attach_to_proc(context, proc, payload);
}

int rxcp_inline_attach_imported_symbol(Context *context, Symbol *symbol, const char *payload) {
    ASTNode *proc;

    if (!context || !symbol || !rxcp_inline_payload_is_supported(payload)) return 0;

    proc = sym_proc(symbol);
    if (!proc) return 0;

    return inline_meta_attach_to_proc(context, proc, payload);
}
