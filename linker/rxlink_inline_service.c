/* cREXX License (MIT). Frozen native selection, one B/L transformation. */
#include "rxlink_inline_service.h"
#include "rxdadism.h"
#include "rxinline_recipe.h"
#include "rxsha256.h"
#include "rxvml.h"
#include "rxvml_inline_private.h"
#include "rxvmref.h"
#include "platform.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

#ifndef LL_RXAS_CATALOG_FINGERPRINT
#error Native and B/L inline adapters require the same catalogue fingerprint
#endif

#define INLINE_MAX_SOURCE (64u * 1024u * 1024u)
#define INLINE_MAX_MODULES 100000u
#define INLINE_MAX_ROWS 1000000u
#define INLINE_MAX_WORK 1000000u

typedef struct inline_work {
    size_t remaining;
    int limited;
} inline_work;

static int inline_take(inline_work *work, size_t amount) {
    if (amount > work->remaining) { work->limited = 1; return 0; }
    work->remaining -= amount;
    return 1;
}

typedef struct inline_module {
    module_file *source;
    module_file *original;
    module_file *candidate;
    module_file rebased;
    int owns_rebased;
    char *input;
    char *output;
    char owner[96];
    int changed;
    size_t source_bytes;
    size_t candidate_bytes;
} inline_module;

static int inline_error(const char *message) {
    fprintf(stderr, "RXLINK_INLINE_SERVICE: %s\n", message);
    return 0;
}

static char *inline_path(const char *directory, const char *name) {
    size_t a = strlen(directory), b = strlen(name);
    char *result;
    if (a > SIZE_MAX - b - 2u) return NULL;
    result = (char *)malloc(a + b + 2u);
    if (result) snprintf(result, a + b + 2u, "%s/%s", directory, name);
    return result;
}

static char *inline_directory(void) {
#ifdef _WIN32
    wchar_t base[MAX_PATH], name[MAX_PATH];
    char *result;
    int length;
    DWORD size = GetTempPathW(MAX_PATH, base);
    if (!size || size >= MAX_PATH || !GetTempFileNameW(base, L"rxi", 0, name) ||
        !DeleteFileW(name) || !CreateDirectoryW(name, NULL)) return NULL;
    length = WideCharToMultiByte(CP_UTF8, 0, name, -1, NULL, 0, NULL, NULL);
    result = length ? (char *)malloc((size_t)length) : NULL;
    if (result) WideCharToMultiByte(CP_UTF8, 0, name, -1, result, length, NULL, NULL);
    else RemoveDirectoryW(name);
    return result;
#else
    const char *base = getenv("TMPDIR");
    char *result;
    size_t length;
    if (!base || !*base) base = "/tmp";
    length = strlen(base);
    if (length > SIZE_MAX - 24u) return NULL;
    result = (char *)malloc(length + 24u);
    if (result) {
        snprintf(result, length + 24u, "%s/rxlink-inline-XXXXXX", base);
        if (!mkdtemp(result)) { free(result); result = NULL; }
    }
    return result;
#endif
}

static int inline_equal(const char *left, const char *right, inline_work *work) {
    if (!left || !right) return left == right;
    while (*left && *right) {
        unsigned char a = (unsigned char)*left++, b = (unsigned char)*right++;
        if (!inline_take(work, 1u)) return 0;
        if (a >= 'A' && a <= 'Z') a = (unsigned char)(a + 'a' - 'A');
        if (b >= 'A' && b <= 'Z') b = (unsigned char)(b + 'a' - 'A');
        if (a != b) return 0;
    }
    return *left == *right;
}

static const chameleon_constant *inline_constant(const module_file *module,
                                                size_t offset) {
    const chameleon_constant *entry;
    if (!module || offset > module->header.constant_size ||
        module->header.constant_size - offset < sizeof(*entry)) return NULL;
    entry = (const chameleon_constant *)(module->constant + offset);
    if (entry->size_in_pool < sizeof(*entry) ||
        entry->size_in_pool > module->header.constant_size - offset) return NULL;
    return entry;
}

static const proc_constant *inline_proc(const module_file *module,
                                        size_t ordinal, size_t *offset, inline_work *work) {
    int current = module->header.proc_head;
    size_t n = 0u;
    while (current >= 0 && n <= INLINE_MAX_ROWS) {
        const chameleon_constant *entry = inline_constant(module, (size_t)current);
        const proc_constant *proc;
        if (!inline_take(work, 1u) || !entry || entry->type != PROC_CONST ||
            entry->size_in_pool < sizeof(proc_constant)) return NULL;
        proc = (const proc_constant *)entry;
        if (n++ == ordinal) { if (offset) *offset = (size_t)current; return proc; }
        current = proc->next;
    }
    return NULL;
}

static const proc_constant *inline_find_proc(const module_file *module,
        const char *name, size_t *ordinal, size_t *offset, inline_work *work) {
    size_t n;
    for (n = 0u; n <= INLINE_MAX_ROWS; ++n) {
        const proc_constant *proc = inline_proc(module, n, offset, work);
        if (!proc) return NULL;
        if (inline_equal(proc->name, name, work)) { if (ordinal) *ordinal = n; return proc; }
    }
    return NULL;
}

static const expose_proc_constant *inline_exposure(const module_file *module,
                                                  const proc_constant *proc) {
    const chameleon_constant *entry;
    if (!proc->exposed) return NULL;
    entry = inline_constant(module, proc->exposed);
    return entry && entry->type == EXPOSE_PROC_CONST
        ? (const expose_proc_constant *)entry : NULL;
}

static int inline_old_member(inline_module *modules, size_t count,
        const module_file *source, uint32_t member, size_t *provider, inline_work *work) {
    size_t i;
    int found = 0;
    if (!source->semantic_graph) return 0;
    for (i = 0u; i < count; ++i) {
        const module_file *candidate = modules[i].source;
        if (!inline_take(work, 1u)) return 0;
        if (candidate->semantic_graph != source->semantic_graph ||
            candidate->semantic_module_index != member) continue;
        if (found) return inline_error("existing inline provider identity is ambiguous");
        *provider = i; found = 1;
    }
    if (!found) return inline_error("existing inline provider is not selected");
    return 1;
}

static const string_constant *inline_binary(const module_file *module, size_t offset) {
    const chameleon_constant *entry = inline_constant(module, offset);
    const string_constant *binary = (const string_constant *)entry;
    return entry && entry->type == BINARY_CONST &&
           entry->size_in_pool >= offsetof(string_constant, string) &&
           binary->string_len <= entry->size_in_pool - offsetof(string_constant, string)
        ? binary : NULL;
}

static int inline_target_from(inline_module *modules, size_t count, size_t caller,
        const module_file *source, int old_image, size_t offset, size_t *provider,
        size_t *ordinal, size_t *target_offset, inline_work *work) {
    const chameleon_constant *entry = inline_constant(source, offset);
    const proc_constant *proc;
    const expose_proc_constant *exposed;
    size_t n, p;
    int found = 0;
    if (!entry) return 0;
    if (entry->type == BINARY_CONST) {
        const string_constant *binary = inline_binary(source, offset);
        rxinl_qualified_proc_view qualified;
        size_t member = 0u;
        if (!binary || !rxinl_qualified_proc_decode(binary->string, binary->string_len, &qualified) ||
            !qualified.sealed) return 0;
        if (old_image) {
            if (!inline_old_member(modules, count, source, qualified.image_module, &member, work)) return 0;
        } else {
            member = qualified.image_module;
            if (member >= count) return 0;
        }
        source = old_image ? modules[member].source : modules[member].original;
        proc = inline_proc(source,
                           qualified.provider_procedure, target_offset, work);
        if (!proc || proc->start >= source->header.instruction_size) return 0;
        *provider = member; *ordinal = qualified.provider_procedure;
        return 1;
    }
    if (entry->type != PROC_CONST || entry->size_in_pool < sizeof(proc_constant)) return 0;
    proc = (const proc_constant *)entry;
    if (proc->start != SIZE_MAX) {
        if (!inline_find_proc(source, proc->name, ordinal, target_offset, work)) return 0;
        *provider = caller;
        return 1;
    }
    exposed = inline_exposure(source, proc);
    if (!exposed || !exposed->imported) return 0;
    for (n = 0u; n < count; ++n) {
        for (p = 0u; p <= INLINE_MAX_ROWS; ++p) {
            size_t candidate_offset;
            const module_file *owner = old_image ? modules[n].source : modules[n].original;
            const proc_constant *candidate = inline_proc(owner, p, &candidate_offset, work);
            const expose_proc_constant *candidate_exposure;
            if (!candidate) break;
            if (candidate->start == SIZE_MAX) continue;
            candidate_exposure = inline_exposure(owner, candidate);
            if (!candidate_exposure || candidate_exposure->imported ||
                !inline_equal(candidate_exposure->index, exposed->index, work)) continue;
            if (found) return 0;
            *provider = n; *ordinal = p; *target_offset = candidate_offset; found = 1;
        }
    }
    return found;
}

static int inline_target(inline_module *modules, size_t count, size_t caller,
        size_t offset, size_t *provider, size_t *ordinal, size_t *target_offset, inline_work *work) {
    return inline_target_from(modules, count, caller, modules[caller].original, 0,
                              offset, provider, ordinal, target_offset, work);
}

/* These views are never executed or published. Preserve old pool/metadata and
 * graph-operand identities while appending only newly encoded inline leaves.
 * The selected namespace is established before snapshotting or discovering
 * any call binding. Ordinary binaries with equal bytes remain untouched. */
static int inline_rebase_operand(inline_module *item, size_t operand,
        const void *bytes, size_t length, size_t *used_bytes, inline_work *work) {
    unsigned char *pool;
    string_constant *entry;
    size_t size, offset, code_bytes;
    if (!item->owns_rebased) {
        if (item->source->header.instruction_size > INLINE_MAX_SOURCE / sizeof(bin_code))
            return inline_error("selection rebase byte limit");
        code_bytes = item->source->header.instruction_size * sizeof(bin_code);
        if (code_bytes > INLINE_MAX_SOURCE - *used_bytes ||
            item->source->header.constant_size > INLINE_MAX_SOURCE - *used_bytes - code_bytes)
            return inline_error("selection rebase byte limit");
        if (!inline_take(work, item->source->header.instruction_size +
                                item->source->header.constant_size / 8u + 1u)) return 0;
        item->rebased = *item->source;
        item->rebased.instructions = malloc(code_bytes);
        item->rebased.constant = malloc(item->source->header.constant_size);
        item->rebased.shared_constant_pool = NULL;
        item->owns_rebased = 1;
        if (!item->rebased.instructions || !item->rebased.constant) return 0;
        memcpy(item->rebased.instructions, item->source->instructions, code_bytes);
        memcpy(item->rebased.constant, item->source->constant, item->source->header.constant_size);
        *used_bytes += code_bytes + item->source->header.constant_size;
        item->original = &item->rebased;
    }
    if (length > INLINE_MAX_SOURCE - offsetof(string_constant, string) - 7u)
        return inline_error("selection rebase byte limit");
    size = (offsetof(string_constant, string) + length + 7u) & ~(size_t)7u;
    if (size > INLINE_MAX_SOURCE - *used_bytes)
        return inline_error("selection rebase byte limit");
    if (!inline_take(work, size / 8u + 1u)) return 0;
    offset = item->rebased.header.constant_size;
    pool = (unsigned char *)realloc(item->rebased.constant, offset + size);
    if (!pool) return 0;
    item->rebased.constant = pool;
    entry = (string_constant *)(pool + offset);
    memset(entry, 0, size);
    entry->base.type = BINARY_CONST; entry->base.size_in_pool = size;
    entry->string_len = length; memcpy(entry->string, bytes, length);
    item->rebased.header.constant_size += size;
    item->rebased.header.constant_stored_size = item->rebased.header.constant_size;
    ((bin_code *)item->rebased.instructions)[operand].index = offset;
    *used_bytes += size;
    return 1;
}

static int inline_rebase_inputs(inline_module *modules, size_t count, inline_work *work) {
    size_t m, used_bytes = 0u;
    for (m = 0u; m < count; ++m) {
        const module_file *source = modules[m].source;
        const bin_code *code = (const bin_code *)source->instructions;
        size_t at = 0u;
        while (at < source->header.instruction_size) {
            int opcode = code[at].instruction.opcode;
            OpFormat format = rxbin_opcode_format(opcode);
            size_t width = rxop_format_operand_count(format), j;
            unsigned int recipe_operand = rxop_recipe_operand(opcode);
            if (!inline_take(work, width + 1u) || width >= source->header.instruction_size - at)
                return 0;
            if (recipe_operand) {
                const string_constant *binary;
                rxinl_recipe_view recipe;
                RxOpCallContract call = rxop_call_contract(opcode);
                size_t provider, actual_provider, ordinal, target_offset;
                unsigned char *bytes;
                int ok;
                if (recipe_operand > width || !call.target_operand || call.target_operand > width)
                    goto invalid;
                binary = inline_binary(source, code[at + recipe_operand].index);
                if (!binary || !rxinl_recipe_decode(binary->string, binary->string_len, &recipe) ||
                    !(recipe.flags & RXINL_RECIPE_SEALED) ||
                    !inline_old_member(modules, count, source, recipe.image_module, &provider, work) ||
                    !inline_target_from(modules, count, m, source, 1,
                        code[at + call.target_operand].index, &actual_provider, &ordinal, &target_offset, work) ||
                    provider != actual_provider || ordinal != recipe.provider_procedure) goto invalid;
                bytes = (unsigned char *)malloc(binary->string_len);
                if (!bytes) return 0;
                memcpy(bytes, binary->string, binary->string_len);
                ok = rxinl_recipe_seal(bytes, binary->string_len, (uint32_t)provider, (uint32_t)ordinal) &&
                     inline_rebase_operand(&modules[m], at + recipe_operand, bytes, binary->string_len, &used_bytes, work);
                free(bytes);
                if (!ok) return 0;
            }
            for (j = 0u; j < width; ++j) {
                const string_constant *binary;
                unsigned char *bytes = NULL;
                size_t length = 0u, provider, ordinal, target_offset;
                char spelling[64];
                int ok;
                if (rxop_operand_relocation(opcode, (unsigned int)j) == RXOP_RELOC_MODULE) {
                    rxinl_module_ref_view ref;
                    binary = inline_binary(source, code[at + j + 1u].index);
                    if (!binary || !rxinl_module_ref_decode(binary->string, binary->string_len, &ref) ||
                        j + 1u >= width || code[at + j + 2u].iconst < 0) goto invalid;
                    if (ref.self_relative) {
                        if (ref.sealed && ref.image_module != source->semantic_module_index) goto invalid;
                        if (!inline_old_member(modules, count, source, source->semantic_module_index, &provider, work)) goto invalid;
                    } else if (!ref.sealed ||
                               !inline_old_member(modules, count, source, ref.image_module, &provider, work)) goto invalid;
                    if ((uint64_t)code[at + j + 2u].iconst >= (uint64_t)modules[provider].source->header.globals) goto invalid;
                    snprintf(spelling, sizeof(spelling), "@%lu", (unsigned long)provider);
                    ok = rxinl_module_ref_from_text(ref.self_relative ? "self" : spelling, &bytes, &length) &&
                         rxinl_module_ref_seal(bytes, length, (uint32_t)provider);
                } else if (rxop_format_operand_type(format, j) == OP_FUNC) {
                    const chameleon_constant *entry = inline_constant(source, code[at + j + 1u].index);
                    if (!entry) goto invalid;
                    if (entry->type != BINARY_CONST) continue;
                    if (!inline_target_from(modules, count, m, source, 1,
                            code[at + j + 1u].index, &provider, &ordinal, &target_offset, work)) goto invalid;
                    snprintf(spelling, sizeof(spelling), "@%lu::%lu", (unsigned long)provider, (unsigned long)ordinal);
                    ok = rxinl_qualified_proc_from_text(spelling, &bytes, &length) &&
                         rxinl_qualified_proc_seal(bytes, length);
                } else continue;
                if (ok) ok = inline_rebase_operand(&modules[m], at + j + 1u, bytes, length, &used_bytes, work);
                free(bytes);
                if (!ok) return 0;
            }
            at += width + 1u;
        }
    }
    return 1;
invalid:
    return inline_error("existing inline identity is unsealed or mismatched before rebase");
}

static int inline_json_string(FILE *file, const char *text) {
    const unsigned char *p = (const unsigned char *)text;
    if (fputc('"', file) == EOF) return 0;
    for (; *p; ++p) {
        if (*p == '"' || *p == '\\') { if (fputc('\\', file) == EOF) return 0; }
        if (*p < 32u) { if (fprintf(file, "\\u%04x", (unsigned)*p) < 0) return 0; }
        else if (fputc(*p, file) == EOF) return 0;
    }
    return fputc('"', file) != EOF;
}

static int inline_snapshot(inline_module *item, size_t ordinal) {
    FILE *file = fopen(item->input, "wb");
    bin_space space;
    rx_sha256_context hash;
    unsigned char digest[RX_SHA256_DIGEST_SIZE];
    size_t n;
    long length;
    if (!file) return 0;
    memset(&space, 0, sizeof(space));
    space.globals = item->original->header.globals;
    space.inst_size = item->original->header.instruction_size;
    space.const_size = item->original->header.constant_size;
    space.binary = (bin_code *)item->original->instructions;
    space.const_pool = item->original->constant;
    disassemble(&space, item->original, file, 0);
    length = ftell(file);
    if (ferror(file) || length < 0 || (unsigned long)length > INLINE_MAX_SOURCE) { fclose(file); return 0; }
    if (fclose(file) != 0) return 0;
    item->source_bytes = (size_t)length;
    rx_sha256_init(&hash);
    rx_sha256_update(&hash, item->original->name, strlen(item->original->name));
    rx_sha256_update(&hash, item->original->instructions,
                     item->original->header.instruction_size * sizeof(bin_code));
    rx_sha256_update(&hash, item->original->constant, item->original->header.constant_size);
    rx_sha256_final(&hash, digest);
    snprintf(item->owner, sizeof(item->owner), "module.%lu.", (unsigned long)ordinal);
    n = strlen(item->owner);
    for (ordinal = 0u; ordinal < sizeof(digest); ++ordinal)
        snprintf(item->owner + n + ordinal * 2u, sizeof(item->owner) - n - ordinal * 2u,
                 "%02x", (unsigned)digest[ordinal]);
    return 1;
}

static int inline_manifest(FILE *file, inline_module *modules, size_t count,
                           const char *transaction, inline_work *work) {
    size_t m, p, rows = 0u;
    int comma = 0;
    if (fputs("{\"version\":\"llinline.selection.v1\",\"transaction\":", file) == EOF ||
        !inline_json_string(file, transaction) ||
        fputs(",\"catalog\":", file) == EOF ||
        !inline_json_string(file, LL_RXAS_CATALOG_FINGERPRINT) ||
        fputs(",\"modules\":[", file) == EOF) return 0;
    for (m = 0u; m < count; ++m) {
        if (fprintf(file, "%s{\"ordinal\":%lu,\"owner\":", m ? "," : "", (unsigned long)m) < 0 ||
            !inline_json_string(file, modules[m].owner) || fputs(",\"name\":", file) == EOF ||
            !inline_json_string(file, modules[m].original->name) || fputs(",\"input\":", file) == EOF ||
            !inline_json_string(file, modules[m].input) || fputs(",\"output\":", file) == EOF ||
            !inline_json_string(file, modules[m].output) || fputc('}', file) == EOF) return 0;
    }
    if (fputs("],\"procedures\":[", file) == EOF) return 0;
    for (m = 0u; m < count; ++m) for (p = 0u; p <= INLINE_MAX_ROWS; ++p) {
        size_t offset;
        const proc_constant *proc = inline_proc(modules[m].original, p, &offset, work);
        const expose_proc_constant *exposed;
        if (!proc) break;
        if (proc->start == SIZE_MAX) continue;
        if (++rows > INLINE_MAX_ROWS) return 0;
        exposed = inline_exposure(modules[m].original, proc);
        if (fprintf(file, "%s{\"owner\":", comma ? "," : "") < 0 ||
            !inline_json_string(file, modules[m].owner) ||
            fprintf(file, ",\"constant\":%lu,\"ordinal\":%lu,\"entry\":%lu,\"symbol\":",
                    (unsigned long)offset, (unsigned long)p, (unsigned long)proc->start) < 0 ||
            !inline_json_string(file, proc->name) || fputs(",\"export\":", file) == EOF ||
            !inline_json_string(file, exposed ? exposed->index : "") ||
            fprintf(file, ",\"locals\":%d}", proc->locals) < 0) return 0;
        comma = 1;
    }
    if (fputs("],\"bindings\":[", file) == EOF) return 0;
    comma = 0;
    for (m = 0u; m < count; ++m) for (p = 0u; p <= INLINE_MAX_ROWS; ++p) {
        size_t offset, at, end, q, instruction = 0u;
        const proc_constant *proc = inline_proc(modules[m].original, p, &offset, work);
        bin_code *code = (bin_code *)modules[m].original->instructions;
        if (!proc) break;
        if (proc->start == SIZE_MAX) continue;
        end = modules[m].original->header.instruction_size;
        for (q = 0u; q <= INLINE_MAX_ROWS; ++q) {
            const proc_constant *next = inline_proc(modules[m].original, q, NULL, work);
            if (!next) break;
            if (next->start > proc->start && next->start < end) end = next->start;
        }
        for (at = proc->start; at < end; ++instruction) {
            int opcode = code[at].instruction.opcode;
            RxOpCallContract call = rxop_call_contract(opcode);
            size_t width = rxop_format_operand_count(rxbin_opcode_format(opcode));
            size_t provider, ordinal, target;
            if (!inline_take(work, width + 1u) || width > end - at - 1u) return 0;
            if (call.kind == RXOP_CALL_DIRECT && call.target_operand > 0 &&
                inline_target(modules, count, m, code[at + call.target_operand].index,
                              &provider, &ordinal, &target, work)) {
                if (++rows > INLINE_MAX_ROWS ||
                    fprintf(file, "%s{\"caller\":", comma ? "," : "") < 0 ||
                    !inline_json_string(file, modules[m].owner) ||
                    fprintf(file, ",\"procedure\":%lu,\"instruction_ordinal\":%lu,\"provider\":",
                            (unsigned long)offset, (unsigned long)instruction) < 0 ||
                    !inline_json_string(file, modules[provider].owner) ||
                    fprintf(file, ",\"target\":%lu}", (unsigned long)target) < 0) return 0;
                comma = 1;
            }
            at += width + 1u;
        }
    }
    return !work->limited && fputs("]}", file) != EOF && !ferror(file);
}

static int inline_attribute_equal(rxvml_context *ctx, rxvml_value *array,
                                   size_t index, const char *expected) {
    rxvml_value *value = rxvml_get_attribute(ctx, array, index);
    const char *text = NULL;
    size_t length = 0u;
    int ok = value && rxvml_to_str(ctx, value, &text, &length) == 0 &&
             length == strlen(expected) && (!length || !memcmp(text, expected, length));
    rxvml_value_free(value);
    return ok;
}

/* Capabilities refer to this request node, not to a stack frame. Tokens can be
 * copied, but teardown marks the node dead before any enrolled owner expires.
 * A dead node retains no VM values and is freed by its last token finalizer. */
typedef struct inline_owner {
    rxvml_value *reference;
    rxvml_value *ir;
    rxinteger revision;
} inline_owner;
typedef struct inline_authority_row {
    size_t phase, caller, provider;
    rxinteger operation, procedure, typeoperation;
} inline_authority_row;
typedef struct inline_load_range { size_t first, last; } inline_load_range;
typedef struct inline_owned_entry {
    const char *member, *descriptor;
    int method;
    rxvml_inline_procedure *pin;
} inline_owned_entry;
typedef struct inline_authority {
    size_t references;
    int live;
    rxvml_context *ctx;
    rxvml_value *session;
    inline_owner *owners;
    size_t count;
    inline_authority_row *rows;
    size_t row_count;
    size_t row_capacity;
    const inline_load_range *ranges;
    size_t range_count;
    inline_owned_entry entries[32];
    size_t entry_count;
    struct inline_procedure_row **procedures;
    size_t *procedure_counts;
    inline_work *work;
} inline_authority;
typedef struct inline_capability {
    inline_authority *request;
    size_t row;
} inline_capability;

static void inline_token_copy(void *destination, void *source);
static void inline_token_finalize(void *item);
static const rxvm_native_payload_ops inline_token_ops = {
    "rxlink.inline.owner.v1", inline_token_copy, inline_token_finalize
};

static int inline_token_read(rxvml_value *value, inline_capability *token) {
    const rxvm_native_payload_ops *ops = NULL;
    size_t length = 0u;
    void *bytes = rxvml_get_native_payload(value, &length, &ops, NULL);
    if (!bytes || ops != &inline_token_ops || length != sizeof(*token)) return 0;
    memcpy(token, bytes, sizeof(*token));
    return token->request != NULL;
}
static void inline_authority_release(inline_authority *request) {
    if (request && !--request->references) free(request);
}
static int inline_token_write(rxvml_value *value, inline_authority *request,
                              size_t row) {
    inline_capability token;
    token.request = request; token.row = row;
    ++request->references;
    if (rxvml_set_native_payload(value, &token, sizeof(token), &inline_token_ops, 0)) {
        inline_authority_release(request); return 0;
    }
    return 1;
}
static void inline_token_copy(void *destination, void *source) {
    inline_capability token;
    if (inline_token_read(source, &token))
        (void)inline_token_write(destination, token.request, token.row);
}
static void inline_token_finalize(void *item) {
    inline_capability token;
    if (inline_token_read(item, &token)) inline_authority_release(token.request);
}

static int inline_owned_call(inline_authority *request, const char *member,
        const char *descriptor, int method, rxvml_value *receiver,
        size_t count, rxvml_value **arguments, rxvml_value **result) {
    size_t i;
    inline_owned_entry *entry = NULL;
    for (i = 0u; i < request->entry_count; ++i) {
        if (request->entries[i].method == method &&
            !strcmp(request->entries[i].member, member) &&
            !strcmp(request->entries[i].descriptor, descriptor)) {
            entry = &request->entries[i]; break;
        }
    }
    if (!entry) {
        rxvml_inline_procedure *selected = NULL;
        if (request->entry_count >= sizeof(request->entries) / sizeof(request->entries[0])) return 0;
        for (i = 0u; i < request->range_count; ++i) {
            rxvml_inline_procedure *candidate = NULL;
            int status = rxvml_inline_resolve_owned(request->ctx,
                    request->ranges[i].first, request->ranges[i].last,
                    member, descriptor, method, &candidate);
            if (status < 0 || (status == 0 && selected)) {
                rxvml_inline_procedure_free(candidate);
                rxvml_inline_procedure_free(selected); return 0;
            }
            if (status == 0) selected = candidate;
        }
        if (!selected) return 0;
        entry = &request->entries[request->entry_count++];
        entry->member = member; entry->descriptor = descriptor;
        entry->method = method; entry->pin = selected;
    }
    return rxvml_inline_call_owned(request->ctx, entry->pin, receiver,
            count, arguments, result) == 0;
}
static int inline_session_call(inline_authority *request, const char *descriptor,
        size_t count, rxvml_value **arguments, rxvml_value **result) {
    return inline_owned_call(request, "llrxinlinestage", descriptor, 1,
            request->session, count, arguments, result);
}
static rxvml_value *inline_number(rxvml_context *ctx, size_t number) {
    rxvml_value *result;
    if (number > (size_t)LLONG_MAX) return NULL;
    result = rxvml_value_new(ctx);
    if (result) rxvml_set_int(result, (rxinteger)number);
    return result;
}
static int inline_session_number(inline_authority *request,
        const char *descriptor, size_t module_index, const char *symbol,
        size_t ordinal, size_t phase, int operation, rxinteger *number) {
    rxvml_value *arguments[4] = {NULL, NULL, NULL, NULL}, *result = NULL;
    size_t count = operation ? 4u : 3u, i;
    int ok = 0;
    arguments[0] = inline_number(request->ctx, module_index + 1u);
    arguments[1] = rxvml_value_new(request->ctx);
    if (!arguments[0] || !arguments[1] ||
        rxvml_set_str(arguments[1], symbol, strlen(symbol))) goto done;
    if (operation) arguments[2] = inline_number(request->ctx, ordinal);
    arguments[count - 1u] = inline_number(request->ctx, phase);
    if (!arguments[count - 1u] || (operation && !arguments[2]) ||
        !inline_session_call(request, descriptor, count, arguments, &result) ||
        !result || rxvml_to_int(request->ctx, result, number)) goto done;
    ok = *number > 0;
done:
    for (i = 0u; i < count; ++i) rxvml_value_free(arguments[i]);
    rxvml_value_free(result); return ok;
}

/* Read the actual module and IR through the fixed trusted owning accessors.
 * Comparing only a live wrapper cell or caller-supplied identity is unsafe. */
static int inline_owner_read(inline_authority *request, rxvml_value *reference,
        rxinteger *revision, rxvml_value **ir) {
    value *owner;
    rxvml_value *result = NULL;
    rxinteger valid = 0;
    if (!reference || !(owner = rxvm_reference_cell_target(
                ((value *)reference)->reference_payload))) return 0;
    if (!inline_owned_call(request, "lltarget",
            "rxsig1|lltarget.module.valid|.int|", 1, owner, 0u, NULL, &result) ||
        !result || rxvml_to_int(request->ctx, result, &valid) || valid != 1) {
        rxvml_value_free(result); return 0;
    }
    rxvml_value_free(result); result = NULL;
    if (!inline_owned_call(request, "lltarget",
            "rxsig1|lltarget.module.identity|.int|", 1, owner, 0u, NULL, &result) ||
        !result || rxvml_to_int(request->ctx, result, revision)) {
        rxvml_value_free(result); return 0;
    }
    rxvml_value_free(result);
    if (*revision <= 0 || !inline_owned_call(request, "lltarget",
            "rxsig1|lltarget.module.ir|reference .llir..module|", 1, owner, 0u, NULL, ir) ||
        !*ir || !rxvm_reference_cell_target(((value *)*ir)->reference_payload)) {
        rxvml_value_free(*ir); *ir = NULL; return 0;
    }
    return 1;
}
static int inline_owner_matches(inline_authority *request, size_t index,
                                 rxvml_value *actual) {
    inline_owner *owner = &request->owners[index];
    rxvml_value *ir = NULL;
    rxinteger revision = 0;
    int ok;
    if (!actual || !owner->reference || ((value *)actual)->reference_payload !=
            ((value *)owner->reference)->reference_payload) return 0;
    if (!inline_owner_read(request, actual, &revision, &ir)) return 0;
    ok = revision == owner->revision && ((value *)ir)->reference_payload ==
            ((value *)owner->ir)->reference_payload;
    rxvml_value_free(ir); return ok;
}
static int inline_row_matches(inline_authority *request,
        const inline_authority_row *row, rxvml_value *const *arguments) {
    rxinteger operation, procedure, typeoperation;
    if (rxvml_to_int(request->ctx, arguments[1], &operation) ||
        rxvml_to_int(request->ctx, arguments[3], &procedure) ||
        rxvml_to_int(request->ctx, arguments[4], &typeoperation) ||
        operation != row->operation || procedure != row->procedure ||
        typeoperation != row->typeoperation) return 0;
    return inline_owner_matches(request, row->phase * request->count + row->caller,
                arguments[0]) && inline_owner_matches(request,
                row->phase * request->count + row->provider, arguments[2]);
}
static int inline_authority_dispatch(rxvml_context *ctx, void *userdata,
        enum rxvml_inline_operation operation, size_t argc,
        rxvml_value *const *arguments, rxvml_value *result) {
    inline_authority *request = userdata;
    size_t i;
    inline_capability token;
    if (!request || !request->live || request->ctx != ctx ||
        !inline_take(request->work, 1u)) return 0;
    if (operation == RXVML_INLINE_TOKEN && argc == 5u) {
        for (i = 0u; i < request->row_count; ++i) {
            if (!inline_take(request->work, 1u)) return 0;
            if (inline_row_matches(request, &request->rows[i], arguments))
                return inline_token_write(result, request, i);
        }
    } else if (operation == RXVML_INLINE_CHECK && argc == 6u &&
        inline_token_read(arguments[0], &token) && token.request == request &&
        token.row < request->row_count &&
        inline_row_matches(request, &request->rows[token.row], arguments + 1u)) {
        rxvml_set_int(result, 1); return 1;
    }
    return 0;
}

static int inline_session_owners(inline_authority *request, size_t phase) {
    size_t i;
    for (i = 0u; i < request->count; ++i) {
        rxvml_value *arguments[2] = {inline_number(request->ctx, i + 1u),
                                    inline_number(request->ctx, phase)};
        inline_owner *owner = &request->owners[phase * request->count + i];
        int ok = arguments[0] && arguments[1] && inline_session_call(request,
                "rxsig1|llrxinlinestage.session.owner|reference .lltarget..module|index=.int,phase=.int",
                2u, arguments, &owner->reference) && inline_owner_read(request,
                    owner->reference, &owner->revision, &owner->ir);
        rxvml_value_free(arguments[0]); rxvml_value_free(arguments[1]);
        if (!ok) return 0;
    }
    return 1;
}
static int inline_session_addproof(inline_authority *request,
                                   inline_authority_row row) {
    rxvml_value *arguments[7] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
    rxvml_value *result = NULL;
    inline_authority_row *grown;
    rxinteger accepted = 0;
    size_t i, index = request->row_count;
    int ok = 0;
    if (index >= INLINE_MAX_ROWS) return 0;
    if (index == request->row_capacity) {
        size_t capacity = request->row_capacity ? request->row_capacity * 2u : 32u;
        if (capacity > INLINE_MAX_ROWS) capacity = INLINE_MAX_ROWS;
        grown = realloc(request->rows, capacity * sizeof(*grown));
        if (!grown) return 0;
        request->rows = grown; request->row_capacity = capacity;
    }
    request->rows[index] = row; ++request->row_count;
    arguments[0] = inline_number(request->ctx, row.caller + 1u);
    arguments[1] = inline_number(request->ctx, (size_t)row.operation);
    arguments[2] = inline_number(request->ctx, row.provider + 1u);
    arguments[3] = inline_number(request->ctx, (size_t)row.procedure);
    arguments[4] = inline_number(request->ctx, (size_t)row.typeoperation);
    arguments[5] = inline_number(request->ctx, row.phase);
    arguments[6] = rxvml_value_new(request->ctx);
    for (i = 0u; i < 7u; ++i) if (!arguments[i]) goto done;
    if (!inline_token_write(arguments[6], request, index) ||
        !inline_session_call(request,
            "rxsig1|llrxinlinestage.session.addproof|.int|caller=.int,operation=.int,provider=.int,functionid=.int,typeoperation=.int,phase=.int,capability=.object",
            7u, arguments, &result) || !result ||
        rxvml_to_int(request->ctx, result, &accepted) || accepted != 1) goto done;
    ok = 1;
done:
    for (i = 0u; i < 7u; ++i) rxvml_value_free(arguments[i]);
    rxvml_value_free(result); return ok;
}

typedef struct inline_procedure_row {
    const proc_constant *procedure;
    size_t constant, ordinal, end;
} inline_procedure_row;
typedef struct inline_procedure_order { size_t start, index; } inline_procedure_order;
static int inline_procedure_compare(const void *left, const void *right) {
    const inline_procedure_order *a = left, *b = right;
    return a->start < b->start ? -1 : a->start > b->start;
}
/* Decode the PROC chain once. Sorted entry points supply exact body bounds;
 * neither source-order enumeration nor each binding rescans that chain. */
static inline_procedure_row *inline_procedure_index(const module_file *module,
        size_t *count, inline_work *work) {
    inline_procedure_row *rows = NULL;
    inline_procedure_order *order = NULL;
    size_t capacity = 0u, n = 0u, executable = 0u, i, log = 1u;
    int at = module->header.proc_head;
    *count = 0u;
    while (at >= 0) {
        const chameleon_constant *entry = inline_constant(module, (size_t)at);
        const proc_constant *procedure;
        if (!inline_take(work, 1u) || n >= INLINE_MAX_ROWS || !entry ||
            entry->type != PROC_CONST || entry->size_in_pool < sizeof(*procedure)) goto fail;
        procedure = (const proc_constant *)entry;
        if (n == capacity) {
            size_t next = capacity ? capacity * 2u : 32u;
            inline_procedure_row *grown;
            if (next > INLINE_MAX_ROWS) next = INLINE_MAX_ROWS;
            if (!inline_take(work, n)) goto fail;
            grown = realloc(rows, next * sizeof(*grown));
            if (!grown) goto fail;
            rows = grown; capacity = next;
        }
        rows[n].procedure = procedure; rows[n].constant = (size_t)at;
        rows[n].ordinal = n; rows[n].end = module->header.instruction_size;
        if (procedure->start != SIZE_MAX) ++executable;
        ++n; at = procedure->next;
    }
    order = calloc(executable ? executable : 1u, sizeof(*order));
    if (!order) goto fail;
    executable = 0u;
    for (i = 0u; i < n; ++i) if (rows[i].procedure->start != SIZE_MAX) {
        if (rows[i].procedure->start >= module->header.instruction_size) goto fail;
        order[executable].start = rows[i].procedure->start;
        order[executable++].index = i;
    }
    for (i = executable; i > 1u; i >>= 1u) ++log;
    if (executable > SIZE_MAX / log || !inline_take(work, executable * log)) goto fail;
    qsort(order, executable, sizeof(*order), inline_procedure_compare);
    if (!inline_take(work, executable)) goto fail;
    for (i = 0u; i < executable;) {
        size_t next = i + 1u, j, end;
        while (next < executable && order[next].start == order[i].start) ++next;
        end = next < executable ? order[next].start : module->header.instruction_size;
        for (j = i; j < next; ++j) rows[order[j].index].end = end;
        i = next;
    }
    free(order); *count = n; return rows;
fail:
    free(order); free(rows); return NULL;
}

/* Direct tuples come only from the existing original-image-aware native
 * resolver. The manifest is not consulted as authority. All source IDs are
 * obtained from the host-created trusted session and its owned rewrite map. */
static int inline_session_direct(inline_authority *request,
        inline_module *modules, size_t phase, inline_work *work) {
    inline_procedure_row **indices;
    size_t *counts;
    size_t m, p;
    int ok = 0;
    if (!request->procedures) {
        request->procedures = calloc(request->count, sizeof(*request->procedures));
        request->procedure_counts = calloc(request->count, sizeof(*request->procedure_counts));
        if (!request->procedures || !request->procedure_counts) return 0;
        for (m = 0u; m < request->count; ++m) {
            request->procedures[m] = inline_procedure_index(modules[m].original,
                    &request->procedure_counts[m], work);
            if (!request->procedures[m]) return 0;
        }
    }
    indices = request->procedures; counts = request->procedure_counts;
    for (m = 0u; m < request->count; ++m) for (p = 0u; p < counts[m]; ++p) {
        const proc_constant *proc = indices[m][p].procedure;
        bin_code *code = (bin_code *)modules[m].original->instructions;
        size_t at, end = indices[m][p].end, instruction = 0u;
        if (proc->start == SIZE_MAX) continue;
        for (at = proc->start; at < end; ++instruction) {
            int opcode = code[at].instruction.opcode;
            RxOpCallContract call = rxop_call_contract(opcode);
            size_t width = rxop_format_operand_count(rxbin_opcode_format(opcode));
            size_t provider, ordinal, constant;
            if (!inline_take(work, width + 1u) || width > end - at - 1u) goto done;
            if (call.kind == RXOP_CALL_DIRECT && call.target_operand > 0 &&
                inline_target(modules, request->count, m,
                    code[at + call.target_operand].index, &provider, &ordinal,
                    &constant, work)) {
                const proc_constant *target;
                inline_authority_row row;
                if (provider >= request->count || ordinal >= counts[provider]) goto done;
                target = indices[provider][ordinal].procedure;
                memset(&row, 0, sizeof(row));
                row.phase = phase; row.caller = m; row.provider = provider;
                if (!inline_take(work, 3u) ||
                    !inline_session_number(request,
                        "rxsig1|llrxinlinestage.session.operation|.int|index=.int,symbol=.string,ordinal=.int,phase=.int",
                        m, proc->name, instruction, phase, 1, &row.operation) ||
                    !inline_session_number(request,
                        "rxsig1|llrxinlinestage.session.function|.int|index=.int,symbol=.string,phase=.int",
                        provider, target->name, 0u, phase, 0, &row.procedure) ||
                    !inline_session_addproof(request, row)) goto done;
            }
            at += width + 1u;
        }
    }
    ok = !work->limited;
done:
    return ok;
}

static void inline_session_close(inline_authority *request) {
    size_t i;
    if (!request) return;
    request->live = 0;
    (void)rxvml_inline_set_callback(request->ctx, NULL, NULL);
    for (i = 0u; request->owners && i < request->count * 2u; ++i) {
        rxvml_value_free(request->owners[i].reference);
        rxvml_value_free(request->owners[i].ir);
    }
    free(request->owners); request->owners = NULL;
    free(request->rows); request->rows = NULL; request->row_count = 0u;
    if (request->procedures) for (i = 0u; i < request->count; ++i) free(request->procedures[i]);
    free(request->procedures); free(request->procedure_counts);
    request->procedures = NULL; request->procedure_counts = NULL;
    rxvml_value_free(request->session); request->session = NULL;
    for (i = 0u; i < request->entry_count; ++i)
        rxvml_inline_procedure_free(request->entries[i].pin);
    request->ctx = NULL;
    inline_authority_release(request);
}
static inline_authority *inline_session_open(rxvml_context *ctx,
        inline_module *modules, size_t count, inline_work *work,
        const inline_load_range *ranges, size_t range_count) {
    inline_authority *request = calloc(1u, sizeof(*request));
    rxvml_value *names = NULL, *sources = NULL, *name = NULL, *source = NULL;
    rxvml_value *reader_args[2];
    rxvml_value *result = NULL, *allowance = NULL;
    size_t i;
    int ok = 0;
    if (!request) return NULL;
    request->references = 1u; request->ctx = ctx; request->count = count;
    request->ranges = ranges; request->range_count = range_count;
    request->work = work;
    request->owners = calloc(count * 2u, sizeof(*request->owners));
    names = rxvml_array_new(ctx, count);
    sources = rxvml_array_new(ctx, count);
    if (!request->owners || !names || !sources) goto done;
    for (i = 0u; i < count; ++i) {
        FILE *file;
        char *bytes;
        size_t size = modules[i].source_bytes;
        if (!inline_take(work, 1u)) goto done;
        bytes = malloc(size ? size : 1u);
        file = fopen(modules[i].input, "rb");
        if (!bytes || !file) { free(bytes); if (file) fclose(file); goto done; }
        if (fread(bytes, 1u, size, file) != size || fgetc(file) != EOF || ferror(file)) {
            free(bytes); fclose(file); goto done;
        }
        fclose(file);
        name = rxvml_value_new(ctx); source = rxvml_value_new(ctx);
        if (!name || !source ||
            rxvml_set_str(name, modules[i].original->name, strlen(modules[i].original->name)) ||
            rxvml_set_native_payload(source, bytes, size, NULL, 0)) {
            free(bytes); goto done;
        }
        free(bytes);
        if (rxvml_array_set(ctx, names, i + 1u, name) ||
            rxvml_array_set(ctx, sources, i + 1u, source)) goto done;
        rxvml_value_free(name); name = NULL;
        rxvml_value_free(source); source = NULL;
    }
    reader_args[0] = names; reader_args[1] = sources;
    if (!inline_owned_call(request, "llrxinlinestage",
            "rxsig1|llrxinlinestage.read|.object|names=.string[*],sources=.binary[*]",
            0, NULL, 2u, reader_args, &request->session) || !request->session) goto done;
    allowance = inline_number(ctx, 4000000u);
    if (!allowance || !inline_session_call(request,
            "rxsig1|llrxinlinestage.session.canonicalize|.void|work=.int",
            1u, &allowance, &result)) goto done;
    rxvml_value_free(result); result = NULL;
    if (!inline_session_owners(request, 0u) ||
        rxvml_inline_set_callback(ctx, inline_authority_dispatch, request)) goto done;
    request->live = 1;
    if (!inline_session_direct(request, modules, 0u, work) ||
        !inline_session_call(request,
            "rxsig1|llrxinlinestage.session.normalize|.void|", 0u, NULL, &result)) goto done;
    rxvml_value_free(result); result = NULL;
    if (!inline_session_owners(request, 1u) ||
        !inline_session_direct(request, modules, 1u, work)) goto done;
    ok = 1;
done:
    rxvml_value_free(result); rxvml_value_free(allowance);
    rxvml_value_free(source); rxvml_value_free(name);
    rxvml_value_free(sources); rxvml_value_free(names);
    if (!ok) { inline_session_close(request); return NULL; }
    return request;
}

static int inline_invoke(inline_module *modules, size_t count,
        const rxlink_inline_options *options, const char *manifest,
        const char *response, const char *transaction, inline_work *work) {
    rxvml_context *ctx = rxvml_create(options->location, 0u);
    rxvml_value *argument = NULL, *result = NULL;
    rxvml_value *arguments[5] = {NULL, NULL, NULL, NULL, NULL};
    inline_load_range *ranges = NULL;
    inline_authority *authority = NULL;
    const char *stage = "create VM context";
    size_t i, range_count = 0u;
    int status = 0, loaded;
    rxinteger rc = -1;
    if (!ctx) goto done;
    ranges = calloc(options->runtime_count + 1u, sizeof(*ranges));
    if (!ranges) goto done;
    stage = "load service module";
    ranges[0].first = rxvml_inline_module_count(ctx) + 1u;
    loaded = rxvml_load_module_file(ctx, options->service);
    if (loaded <= 0) goto done;
    ranges[0].last = (size_t)loaded;
    if (ranges[0].first <= ranges[0].last) range_count = 1u;
    stage = "load runtime module";
    for (i = 0u; i < options->runtime_count; ++i) {
        ranges[range_count].first = rxvml_inline_module_count(ctx) + 1u;
        loaded = rxvml_load_module_file(ctx, options->runtime[i]);
        if (loaded <= 0) goto done;
        ranges[range_count].last = (size_t)loaded;
        if (ranges[range_count].first <= ranges[range_count].last) ++range_count;
    }
    stage = "seal exact service runtime closure";
    if (rxvml_inline_seal_context(ctx)) goto done;
    stage = "read and enroll exact selected owners";
    authority = inline_session_open(ctx, modules, count, work, ranges,
                                    range_count);
    if (!authority) goto done;
    arguments[0] = authority->session;
    for (i = 1u; i < 5u; ++i) {
        arguments[i] = rxvml_value_new(ctx);
        if (!arguments[i]) goto done;
    }
    if (rxvml_set_str(arguments[1], manifest, strlen(manifest)) ||
        rxvml_set_str(arguments[2], response, strlen(response)) ||
        rxvml_set_str(arguments[3], options->policy, strlen(options->policy))) goto done;
    rxvml_set_int(arguments[4], 4000000);
    stage = "invoke exact owned service entry";
    if (!inline_owned_call(authority, "llrxinline",
            "rxsig1|llrxinline.selectionexecute|.int|state=.llrxinlinestage..session,manifestpath=.string,resultpath=.string,policy=.string,workbudget=.int",
            0, NULL, 5u, arguments, &result) || !result ||
        rxvml_to_int(ctx, result, &rc)) goto done;
    rxvml_value_free(result); result = NULL;
    if (rc != 0) {
        fprintf(stderr, "RXLINK_INLINE_SERVICE: service entry returned %lld\n", (long long)rc);
        goto done;
    }
    stage = "prepare response reader argument";
    argument = rxvml_value_new(ctx);
    if (!argument || rxvml_set_str(argument, response, strlen(response)) != 0) goto done;
    stage = "invoke response reader";
    if (!inline_owned_call(authority, "llrxinline",
            "rxsig1|llrxinline.selectionresult|.string[*]|path=.string",
            0, NULL, 1u, &argument, &result) || !result) goto done;
    stage = "validate response attribute count";
    if (rxvml_num_attributes(result) != count * 4u + 3u) {
        fprintf(stderr, "RXLINK_INLINE_SERVICE: response has %lu attributes; expected %lu\n",
                (unsigned long)rxvml_num_attributes(result), (unsigned long)(count * 4u + 3u));
        goto done;
    }
    stage = "validate response header";
    if (!inline_attribute_equal(ctx, result, 1u, "llinline.selection.result.v1") ||
        !inline_attribute_equal(ctx, result, 2u, transaction) ||
        !inline_attribute_equal(ctx, result, 3u, LL_RXAS_CATALOG_FINGERPRINT)) goto done;
    stage = "validate response module row";
    for (i = 0u; i < count; ++i) {
        char ordinal[32];
        size_t base = i * 4u + 4u;
        snprintf(ordinal, sizeof(ordinal), "%lu", (unsigned long)i);
        if (!inline_attribute_equal(ctx, result, base, modules[i].owner) ||
            !inline_attribute_equal(ctx, result, base + 1u, ordinal) ||
            !inline_attribute_equal(ctx, result, base + 3u, modules[i].output)) goto done;
        if (inline_attribute_equal(ctx, result, base + 2u, "true")) modules[i].changed = 1;
        else if (!inline_attribute_equal(ctx, result, base + 2u, "false")) goto done;
    }
    status = 1;
done:
    if (!status) {
        const char *error = NULL;
        if (ctx) rxvml_last_error(ctx, &error);
        fprintf(stderr, "RXLINK_INLINE_SERVICE: B/L request failed during %s: %s\n",
                stage, error ? error : "request rejected");
    }
    rxvml_value_free(result); rxvml_value_free(argument);
    for (i = 1u; i < 5u; ++i) rxvml_value_free(arguments[i]);
    inline_session_close(authority);
    if (ctx) rxvml_destroy(ctx);
    free(ranges);
    return status;
}

static module_file *inline_assemble(inline_module *item) {
    Assembler_Context scanner;
    module_file *result = NULL;
    FILE *file = fopen(item->output, "rb");
    long length;
    if (!file) return NULL;
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) < 0 ||
        (unsigned long)length > INLINE_MAX_SOURCE || fseek(file, 0, SEEK_SET) != 0) { fclose(file); return NULL; }
    fclose(file);
    item->candidate_bytes = (size_t)length;
    memset(&scanner, 0, sizeof(scanner));
    scanner.file_name = item->original->name;
    scanner.input_file_name = item->output;
    scanner.optimise = 0;
    if (rxasinfl(&scanner, 1) != 0) return NULL;
    if (rxaspars(&scanner) != 0) { rxasperr(&scanner); rxasclrc(&scanner); return NULL; }
    result = (module_file *)calloc(1u, sizeof(*result));
    if (!result) { rxasclrc(&scanner); return NULL; }
    init_module(result); result->fromfile = 1u;
    result->name = strdup(item->original->name); result->description = strdup(item->original->description);
    result->header = item->original->header;
    result->header.name_size = strlen(item->original->name) + 1u;
    result->header.description_size = strlen(item->original->description) + 1u;
    result->header.instruction_size = scanner.binary.inst_size;
    result->header.constant_size = scanner.binary.const_size;
    result->header.constant_stored_size = 0u;
    result->header.globals = scanner.binary.globals;
    result->header.proc_head = scanner.proc_head; result->header.expose_head = scanner.expose_head;
    result->header.meta_head = scanner.meta_head;
    result->instructions = scanner.binary.binary; scanner.binary.binary = NULL;
    result->constant = scanner.binary.const_pool; scanner.binary.const_pool = NULL;
    rxasclrc(&scanner);
    if (!result->name || !result->description) { free_module(result); return NULL; }
    return result;
}

static int inline_identity(inline_module *item, inline_work *work) {
    size_t p, original_exposures = 0u, candidate_exposures = 0u;
    int offset;
    if (item->candidate->header.globals != item->original->header.globals) return 0;
    for (p = 0u; p <= INLINE_MAX_ROWS; ++p) {
        const proc_constant *original = inline_proc(item->original, p, NULL, work);
        const proc_constant *candidate;
        const expose_proc_constant *left, *right;
        if (!original) break;
        candidate = inline_find_proc(item->candidate, original->name, NULL, NULL, work);
        if (!candidate || (original->start == SIZE_MAX) != (candidate->start == SIZE_MAX)) return 0;
        left = inline_exposure(item->original, original); right = inline_exposure(item->candidate, candidate);
        if ((!left) != (!right) || (left && (left->imported != right->imported || !inline_equal(left->index, right->index, work)))) return 0;
    }
    if (inline_proc(item->candidate, p, NULL, work) != NULL) return 0;
    offset = item->original->header.expose_head;
    while (offset >= 0) {
        const chameleon_constant *entry = inline_constant(item->original, (size_t)offset);
        if (!inline_take(work, 1u) || !entry || ++original_exposures > INLINE_MAX_ROWS) return 0;
        if (entry->type == EXPOSE_PROC_CONST) offset = ((const expose_proc_constant *)entry)->next;
        else if (entry->type == EXPOSE_REG_CONST) {
            const expose_reg_constant *reg = (const expose_reg_constant *)entry;
            int other = item->candidate->header.expose_head, found = 0;
            size_t visited = 0u;
            while (other >= 0) {
                const chameleon_constant *candidate = inline_constant(item->candidate, (size_t)other);
                if (!inline_take(work, 1u) || !candidate || ++visited > INLINE_MAX_ROWS) return 0;
                if (candidate->type == EXPOSE_PROC_CONST) other = ((const expose_proc_constant *)candidate)->next;
                else if (candidate->type == EXPOSE_REG_CONST) {
                    const expose_reg_constant *value = (const expose_reg_constant *)candidate;
                    if (value->global_reg == reg->global_reg && inline_equal(value->index, reg->index, work)) found = 1;
                    other = value->next;
                } else return 0;
            }
            if (!found) return 0;
            offset = reg->next;
        } else return 0;
    }
    offset = item->candidate->header.expose_head;
    while (offset >= 0) {
        const chameleon_constant *entry = inline_constant(item->candidate, (size_t)offset);
        if (!inline_take(work, 1u) || !entry || ++candidate_exposures > INLINE_MAX_ROWS) return 0;
        if (entry->type == EXPOSE_PROC_CONST) offset = ((const expose_proc_constant *)entry)->next;
        else if (entry->type == EXPOSE_REG_CONST) offset = ((const expose_reg_constant *)entry)->next;
        else return 0;
    }
    return !work->limited && original_exposures == candidate_exposures;
}

/* Compare semantic references, never pool offsets or host-structure padding. */
static int inline_same_reference(const module_file *a, size_t ai,
                                  const module_file *b, size_t bi, inline_work *work) {
    const chameleon_constant *left, *right;
    if (ai == SIZE_MAX || bi == SIZE_MAX) return ai == bi;
    left = inline_constant(a, ai); right = inline_constant(b, bi);
    if (!left || !right || left->type != right->type) return 0;
    if (left->type == STRING_CONST || left->type == BINARY_CONST || left->type == DECIMAL_CONST) {
        const string_constant *x = (const string_constant *)left, *y = (const string_constant *)right;
        return x->string_len == y->string_len && inline_take(work, x->string_len + 1u) &&
               !memcmp(x->string, y->string, x->string_len);
    }
    if (left->type == FLOAT_CONST)
        return !memcmp(&((const float_constant *)left)->double_value,
                       &((const float_constant *)right)->double_value, sizeof(double));
    if (left->type == PROC_CONST) {
        const proc_constant *x = (const proc_constant *)left, *y = (const proc_constant *)right;
        const expose_proc_constant *xe = inline_exposure(a, x), *ye = inline_exposure(b, y);
        return inline_equal(x->name, y->name, work) && (x->start == SIZE_MAX) == (y->start == SIZE_MAX) &&
               ((!xe && !ye) || (xe && ye && xe->imported == ye->imported && inline_equal(xe->index, ye->index, work)));
    }
    return 0;
}

static const proc_constant *inline_address_owner(const module_file *module, size_t address, inline_work *work) {
    const proc_constant *best = NULL;
    size_t p;
    for (p = 0u; p <= INLINE_MAX_ROWS; ++p) {
        const proc_constant *proc = inline_proc(module, p, NULL, work);
        if (!proc) break;
        if (proc->start != SIZE_MAX && proc->start <= address && (!best || proc->start > best->start)) best = proc;
    }
    return best;
}

static int inline_same_register(const module_file *a, const meta_entry *x, size_t ar,
                                const module_file *b, const meta_entry *y, size_t br, inline_work *work) {
    const proc_constant *ap = inline_address_owner(a, x->address, work), *bp = inline_address_owner(b, y->address, work);
    unsigned int ab = 0u, bb = 0u;
    if (!ap || !bp) return !ap && !bp && ar == br;
    if (!inline_equal(ap->name, bp->name, work)) return 0;
    if (ar >= (size_t)ap->locals + (size_t)a->header.globals) { ab = 2u; ar -= (size_t)ap->locals + (size_t)a->header.globals; }
    else if (ar >= (size_t)ap->locals) { ab = 1u; ar -= (size_t)ap->locals; }
    if (br >= (size_t)bp->locals + (size_t)b->header.globals) { bb = 2u; br -= (size_t)bp->locals + (size_t)b->header.globals; }
    else if (br >= (size_t)bp->locals) { bb = 1u; br -= (size_t)bp->locals; }
    return ab == bb && ar == br;
}

static int inline_observation(enum const_pool_type type) {
    return type == META_REG || type == META_CONST || type == META_CLEAR ||
           type == META_SOURCE_STEP || type == META_TRACE_EVENT;
}

static int inline_same_metadata(const module_file *a, const meta_entry *left,
                                const module_file *b, const meta_entry *right, inline_work *work) {
    if (left->base.type != right->base.type) return 0;
#define ROW(TYPE) const TYPE *x = (const TYPE *)left, *y = (const TYPE *)right
#define REF(FIELD) inline_same_reference(a, x->FIELD, b, y->FIELD, work)
    switch (left->base.type) {
        case META_FUNC: { ROW(meta_func_constant); return REF(symbol) && REF(option) && REF(type) && REF(func) && REF(args); }
        case META_REG: { ROW(meta_reg_constant); return REF(symbol) && REF(option) && REF(type) && inline_same_register(a, left, x->reg, b, right, y->reg, work); }
        case META_CONST: { ROW(meta_const_constant); return REF(symbol) && REF(option) && REF(type) && REF(constant); }
        case META_CLEAR: { ROW(meta_clear_constant); return REF(symbol); }
        case META_CLASS: { ROW(meta_class_constant); return REF(symbol) && REF(option) && REF(type); }
        case META_ATTR: { ROW(meta_attr_constant); return REF(symbol) && REF(option) && REF(type) && x->reg == y->reg; }
        case META_INTERFACE: { ROW(meta_interface_constant); return REF(symbol) && REF(option) && REF(type); }
        case META_IMPLEMENTS: { ROW(meta_implements_constant); return REF(symbol) && REF(interface_symbol); }
        case META_MEMBER: { ROW(meta_member_constant); return REF(owner) && REF(kind) && REF(member) && REF(type) && REF(args); }
        case META_INLINE: { ROW(meta_inline_constant); return REF(symbol) && REF(payload); }
        case META_PROVIDER: { ROW(meta_provider_constant); return REF(symbol) && REF(provider) && x->flags == y->flags; }
        case META_INITIALIZER: { ROW(meta_initializer_constant); return REF(symbol) && REF(function); }
        case META_AUTOLOAD: { ROW(meta_autoload_constant); return REF(symbol) && REF(artifact); }
        case META_TASK_TARGET: {
            ROW(meta_task_target_constant);
            /* The final graph owner regenerates the address-dependent seal.
             * Its typed callable identity/kind and original opaque request
             * must survive this source transformation unchanged. */
            return REF(symbol) && x->kind == y->kind && REF(binding);
        }
        case META_SOURCE_STEP: {
            ROW(meta_source_step_constant);
            return REF(file) && REF(source_line) && x->step_id == y->step_id && x->clause_id == y->clause_id &&
                   x->line == y->line && x->active_start_column == y->active_start_column &&
                   x->active_end_column == y->active_end_column && x->flags == y->flags;
        }
        case META_TRACE_EVENT: {
            ROW(meta_trace_event_constant);
            return x->kind == y->kind && x->value_source == y->value_source && x->value_type == y->value_type &&
                   x->register_type == y->register_type && x->mode_mask == y->mode_mask && x->flags == y->flags &&
                   (x->value_source == RXBIN_TRACE_VALUE_CONSTANT ? REF(value_ref) : x->value_ref == y->value_ref) &&
                   REF(symbol) && REF(resolved_name) && x->source_step_id == y->source_step_id && x->clause_id == y->clause_id;
        }
        default: return 0;
    }
#undef REF
#undef ROW
}

static const meta_entry *inline_meta(const module_file *module, int offset) {
    const chameleon_constant *entry;
    if (offset < 0) return NULL;
    entry = inline_constant(module, (size_t)offset);
    return entry && entry->size_in_pool >= sizeof(meta_entry) ? (const meta_entry *)entry : NULL;
}

static int inline_metadata_identity(const module_file *original_module, const module_file *candidate_module, inline_work *work) {
    int original = original_module->header.meta_head, candidate = candidate_module->header.meta_head;
    size_t rows = 0u;
    /* Non-observation contracts, including initializer/dependency order,
     * are exact ordered chains. Extra inline source observations are allowed. */
    while (original >= 0 || candidate >= 0) {
        const meta_entry *x = inline_meta(original_module, original), *y = inline_meta(candidate_module, candidate);
        if (!inline_take(work, 1u) || ++rows > INLINE_MAX_ROWS) return 0;
        if (x && inline_observation(x->base.type)) { original = x->next; continue; }
        if (y && inline_observation(y->base.type)) { candidate = y->next; continue; }
        if (!x || !y || !inline_same_metadata(original_module, x, candidate_module, y, work)) return 0;
        original = x->next; candidate = y->next;
    }
    original = original_module->header.meta_head; candidate = candidate_module->header.meta_head; rows = 0u;
    while (original >= 0) {
        const meta_entry *x = inline_meta(original_module, original);
        int found = 0;
        if (!inline_take(work, 1u) || !x || ++rows > INLINE_MAX_ROWS) return 0;
        original = x->next;
        if (!inline_observation(x->base.type)) continue;
        while (candidate >= 0) {
            const meta_entry *y = inline_meta(candidate_module, candidate);
            if (!inline_take(work, 1u) || !y || ++rows > INLINE_MAX_ROWS) return 0;
            candidate = y->next;
            if (inline_same_metadata(original_module, x, candidate_module, y, work)) { found = 1; break; }
        }
        if (!found) return 0;
    }
    return 1;
}

int rxlink_inline_metadata_equal(const module_file *original,
                                const module_file *candidate) {
    inline_work state = { INLINE_MAX_WORK, 0 };
    inline_work *work = &state;
    if (!original || !candidate) return 0;
    return inline_metadata_identity(original, candidate, work);
}

static int inline_seal_qproc(inline_module *modules, size_t count,
                             string_constant *binary, size_t *provider,
                             size_t *procedure, inline_work *work) {
    rxinl_qualified_proc_view ref;
    const proc_constant *original, *candidate;
    unsigned char *bytes = NULL;
    size_t length = 0u, ordinal;
    char spelling[96];
    if (!rxinl_qualified_proc_decode(binary->string, binary->string_len, &ref) ||
        ref.image_module >= count) return 0;
    if (ref.sealed) {
        module_file *final = modules[ref.image_module].changed ? modules[ref.image_module].candidate : modules[ref.image_module].original;
        candidate = inline_proc(final, ref.provider_procedure, NULL, work);
        if (!candidate || candidate->start == SIZE_MAX ||
            !inline_find_proc(modules[ref.image_module].original, candidate->name, NULL, NULL, work)) return 0;
        *provider = ref.image_module; *procedure = ref.provider_procedure;
        return 1;
    }
    original = inline_proc(modules[ref.image_module].original, ref.provider_procedure, NULL, work);
    if (!original || original->start == SIZE_MAX) return 0;
    candidate = inline_find_proc(modules[ref.image_module].changed ? modules[ref.image_module].candidate : modules[ref.image_module].original,
                                 original->name, &ordinal, NULL, work);
    if (!candidate || candidate->start == SIZE_MAX || ordinal > UINT32_MAX) return 0;
    snprintf(spelling, sizeof(spelling), "@%u::%lu", (unsigned)ref.image_module, (unsigned long)ordinal);
    if (!rxinl_qualified_proc_from_text(spelling, &bytes, &length) || length != binary->string_len ||
        !rxinl_qualified_proc_seal(bytes, length)) { free(bytes); return 0; }
    memcpy(binary->string, bytes, length); free(bytes);
    *provider = ref.image_module; *procedure = ordinal;
    return 1;
}

static int inline_seal(inline_module *modules, size_t count, size_t owner,
                       const rxlink_inline_options *options, inline_work *work) {
    module_file *module = modules[owner].candidate;
    bin_code *code = (bin_code *)module->instructions;
    size_t at = 0u;
    while (at < module->header.instruction_size) {
        int opcode = code[at].instruction.opcode;
        size_t width = rxop_format_operand_count(rxbin_opcode_format(opcode)), j;
        size_t provider = owner, procedure = 0u;
        unsigned int recipe_ordinal = rxop_recipe_operand(opcode);
        if (!inline_take(work, width + 1u) || width > module->header.instruction_size - at - 1u) return 0;
        for (j = 0u; j < width; ++j) {
            const chameleon_constant *entry;
            if (rxop_format_operand_type(rxbin_opcode_format(opcode), j) != OP_FUNC) continue;
            entry = inline_constant(module, code[at + j + 1u].index);
            if (!entry) return 0;
            if (entry->type == BINARY_CONST) {
                if (!inline_seal_qproc(modules, count, (string_constant *)entry, &provider, &procedure, work)) return 0;
            } else if (entry->type == PROC_CONST) {
                const proc_constant *proc = (const proc_constant *)entry;
                const proc_constant *original = inline_find_proc(modules[owner].original, proc->name, NULL, NULL, work);
                if (!original) return 0;
                if (proc->start != SIZE_MAX) {
                    if (!inline_find_proc(modules[owner].changed ? module : modules[owner].original,
                                          proc->name, &procedure, NULL, work)) return 0;
                    provider = owner;
                } else if (recipe_ordinal) {
                    size_t original_offset, old_ordinal, target_offset;
                    if (!inline_find_proc(modules[owner].original, proc->name, NULL, &original_offset, work) ||
                        !inline_target(modules, count, owner, original_offset, &provider, &old_ordinal, &target_offset, work)) return 0;
                    original = inline_proc(modules[provider].original, old_ordinal, NULL, work);
                    if (!original || !inline_find_proc(modules[provider].changed ? modules[provider].candidate : modules[provider].original,
                                                       original->name, &procedure, NULL, work)) return 0;
                }
            } else return 0;
        }
        if (recipe_ordinal) {
            string_constant *recipe = (string_constant *)inline_constant(module, code[at + recipe_ordinal].index);
            if (!recipe || recipe->base.type != BINARY_CONST || provider >= count || procedure > UINT32_MAX ||
                !rxinl_recipe_seal(recipe->string, recipe->string_len, (uint32_t)provider, (uint32_t)procedure)) return 0;
        }
        for (j = 0u; j < width; ++j) if (rxop_operand_relocation(opcode, j) == RXOP_RELOC_MODULE) {
            string_constant *binary = (string_constant *)inline_constant(module, code[at + j + 1u].index);
            rxinl_module_ref_view ref;
            uint32_t target;
            if (!binary || binary->base.type != BINARY_CONST ||
                !rxinl_module_ref_decode(binary->string, binary->string_len, &ref)) return 0;
            target = UINT32_MAX;
            if (ref.self_relative) target = (uint32_t)owner;
            else if (ref.selector_length > 1u && ref.selector[0] == '@') {
                uint32_t k;
                target = 0u;
                for (k = 1u; k < ref.selector_length; ++k)
                    target = target * 10u + (uint32_t)(ref.selector[k] - '0');
            } else {
                size_t k;
                for (k = 0u; k < count; ++k) {
                    const char *name = modules[k].original->name;
                    const char *selector = options->selectors ? options->selectors[k] : NULL;
                    size_t name_length = strlen(name), selector_length = selector ? strlen(selector) : 0u;
                    int match;
                    if (!inline_take(work, 1u + name_length + selector_length)) return 0;
                    match = name_length == ref.selector_length &&
                                !memcmp(name, ref.selector, ref.selector_length);
                    if (selector && selector_length == ref.selector_length &&
                        !memcmp(selector, ref.selector, ref.selector_length)) match = 1;
                    if (match) { if (target != UINT32_MAX) return 0; target = (uint32_t)k; }
                }
            }
            if (target >= count || code[at + j + 2u].index >= (size_t)modules[target].original->header.globals ||
                !rxinl_module_ref_seal(binary->string, binary->string_len, target)) return 0;
        }
        at += width + 1u;
    }
    return at == module->header.instruction_size;
}

int rxlink_inline_service(module_file *const *selected, size_t count,
        const rxlink_inline_options *options, module_file ***candidates_out) {
    inline_module *modules = NULL;
    module_file **candidates = NULL;
    char *directory = NULL, *manifest = NULL, *response = NULL;
    char transaction[96];
    FILE *file = NULL;
    size_t i, source_bytes = 0u, candidate_bytes = 0u;
    int ok = 0;
    inline_work state = { INLINE_MAX_WORK, 0 };
    inline_work *work = &state;
    if (!candidates_out) return -1;
    *candidates_out = NULL;
    if (!selected || !count || count > INLINE_MAX_MODULES || !options || !options->service ||
        !options->policy) return -1;
    if (!strcmp(options->policy, "never")) {
        *candidates_out = (module_file **)calloc(count, sizeof(**candidates_out));
        return *candidates_out ? 0 : -1;
    }
    modules = (inline_module *)calloc(count, sizeof(*modules));
    candidates = (module_file **)calloc(count, sizeof(*candidates));
    directory = inline_directory();
    if (!modules || !candidates || !directory) goto done;
    manifest = inline_path(directory, "selection.json"); response = inline_path(directory, "result.json");
    if (!manifest || !response) goto done;
    snprintf(transaction, sizeof(transaction), "rxlink.%s", strrchr(directory, '/') ? strrchr(directory, '/') + 1 : directory);
    for (i = 0u; i < count; ++i) {
        if (!selected[i]) goto done;
        modules[i].source = modules[i].original = selected[i];
    }
    if (!inline_rebase_inputs(modules, count, work)) goto done;
    for (i = 0u; i < count; ++i) {
        char name[64];
        snprintf(name, sizeof(name), "module-%lu.rxas", (unsigned long)i); modules[i].input = inline_path(directory, name);
        snprintf(name, sizeof(name), "candidate-%lu.rxas", (unsigned long)i); modules[i].output = inline_path(directory, name);
        if (!modules[i].input || !modules[i].output || !inline_snapshot(&modules[i], i)) goto done;
        if (modules[i].source_bytes > INLINE_MAX_SOURCE - source_bytes) {
            inline_error("selection source byte limit"); goto done;
        }
        source_bytes += modules[i].source_bytes;
    }
    file = fopen(manifest, "wb");
    if (!file || !inline_manifest(file, modules, count, transaction, work)) goto done;
    { long length = ftell(file); if (length < 0 || (unsigned long)length > INLINE_MAX_SOURCE) goto done; }
    if (fclose(file) != 0) { file = NULL; goto done; } file = NULL;
    if (!inline_invoke(modules, count, options, manifest, response, transaction, work)) goto done;
    for (i = 0u; i < count; ++i) {
        modules[i].candidate = inline_assemble(&modules[i]);
        if (!modules[i].candidate) { inline_error("candidate RXAS reassembly failed"); goto done; }
        if (modules[i].candidate_bytes > INLINE_MAX_SOURCE - candidate_bytes) {
            inline_error("selection candidate byte limit"); goto done;
        }
        candidate_bytes += modules[i].candidate_bytes;
        if (!inline_identity(&modules[i], work)) { if (!work->limited) inline_error("candidate module/procedure/export identity differs"); goto done; }
        if (!inline_metadata_identity(modules[i].original, modules[i].candidate, work)) { if (!work->limited) inline_error("candidate metadata contracts or original source observations differ"); goto done; }
    }
    for (i = 0u; i < count; ++i) if (!inline_seal(modules, count, i, options, work)) goto done;
    {
        module_file **validation = (module_file **)calloc(count, sizeof(*validation));
        RxGraph *graph;
        char *error = NULL;
        if (!validation) goto done;
        for (i = 0u; i < count; ++i) validation[i] = modules[i].changed ? modules[i].candidate : modules[i].original;
        graph = rx_graph_build_crexx(validation, count, &error); free(validation);
        if (!graph || !rx_graph_validate(graph, &error)) {
            fprintf(stderr, "RXLINK_INLINE_SERVICE: candidate graph: %s\n", error ? error : "invalid");
            free(error); if (graph) rx_graph_release(&graph); goto done;
        }
        free(error); rx_graph_release(&graph);
    }
    for (i = 0u; i < count; ++i) if (modules[i].changed) {
        candidates[i] = modules[i].candidate; modules[i].candidate = NULL;
    }
    *candidates_out = candidates; candidates = NULL; ok = 1;
done:
    if (file) fclose(file);
    if (modules) for (i = 0u; i < count; ++i) {
        free_module(modules[i].candidate);
        if (modules[i].owns_rebased) {
            free(modules[i].rebased.instructions);
            free(modules[i].rebased.constant);
        }
        if (modules[i].input) remove(modules[i].input);
        if (modules[i].output) remove(modules[i].output);
        free(modules[i].input); free(modules[i].output);
    }
    if (manifest) remove(manifest);
    if (response) remove(response);
    if (directory) {
#ifdef _WIN32
        _rmdir(directory);
#else
        rmdir(directory);
#endif
    }
    free(modules); free(candidates); free(directory); free(manifest); free(response);
    if (!ok) inline_error(work->limited ? "selection work limit" : "selection request failed; selected inputs and output remain unchanged");
    return ok ? 0 : -1;
}
