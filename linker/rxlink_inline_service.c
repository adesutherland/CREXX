/* cREXX License (MIT). Frozen native selection, one B/L transformation. */
#include "rxlink_inline_service.h"
#include "rxdadism.h"
#include "rxinline_recipe.h"
#include "rxsha256.h"
#include "rxvml.h"
#include "platform.h"

#include <errno.h>
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
    module_file *original;
    module_file *candidate;
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

static int inline_target(inline_module *modules, size_t count, size_t caller,
        size_t offset, size_t *provider, size_t *ordinal, size_t *target_offset, inline_work *work) {
    const chameleon_constant *entry = inline_constant(modules[caller].original, offset);
    const proc_constant *proc;
    const expose_proc_constant *exposed;
    size_t n, p;
    int found = 0;
    if (!entry) return 0;
    if (entry->type == BINARY_CONST) {
        const string_constant *binary = (const string_constant *)entry;
        rxinl_qualified_proc_view qualified;
        if (!rxinl_qualified_proc_decode(binary->string, binary->string_len, &qualified) ||
            !qualified.sealed || qualified.image_module >= count) return 0;
        proc = inline_proc(modules[qualified.image_module].original,
                           qualified.provider_procedure, target_offset, work);
        if (!proc || proc->start == SIZE_MAX) return 0;
        *provider = qualified.image_module; *ordinal = qualified.provider_procedure;
        return 1;
    }
    if (entry->type != PROC_CONST) return 0;
    proc = (const proc_constant *)entry;
    if (proc->start != SIZE_MAX) {
        if (!inline_find_proc(modules[caller].original, proc->name, ordinal, target_offset, work)) return 0;
        *provider = caller;
        return 1;
    }
    exposed = inline_exposure(modules[caller].original, proc);
    if (!exposed || !exposed->imported) return 0;
    for (n = 0u; n < count; ++n) {
        for (p = 0u; p <= INLINE_MAX_ROWS; ++p) {
            size_t candidate_offset;
            const proc_constant *candidate = inline_proc(modules[n].original, p, &candidate_offset, work);
            const expose_proc_constant *candidate_exposure;
            if (!candidate) break;
            if (candidate->start == SIZE_MAX) continue;
            candidate_exposure = inline_exposure(modules[n].original, candidate);
            if (!candidate_exposure || candidate_exposure->imported ||
                !inline_equal(candidate_exposure->index, exposed->index, work)) continue;
            if (found) return 0;
            *provider = n; *ordinal = p; *target_offset = candidate_offset; found = 1;
        }
    }
    return found;
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

static int inline_invoke(inline_module *modules, size_t count,
        const rxlink_inline_options *options, const char *manifest,
        const char *response, const char *transaction) {
    rxvml_context *ctx = rxvml_create(options->location, 0u);
    rxvml_value *argument = NULL, *result = NULL;
    const char *arguments[6];
    const char *stage = "create VM context";
    size_t i;
    int status = 0, rc = -1;
    if (!ctx) goto done;
    stage = "load service module";
    if (rxvml_load_module_file(ctx, options->service) <= 0) goto done;
    stage = "load runtime module";
    for (i = 0u; i < options->runtime_count; ++i)
        if (rxvml_load_module_file(ctx, options->runtime[i]) <= 0) goto done;
    arguments[0] = "--selection"; arguments[1] = manifest;
    arguments[2] = "--selection-result"; arguments[3] = response;
    arguments[4] = "--policy"; arguments[5] = options->policy;
    stage = "invoke service main";
    if (rxvml_run(ctx, 6, arguments, &rc) != 0) goto done;
    if (rc != 0) {
        fprintf(stderr, "RXLINK_INLINE_SERVICE: service main returned %d\n", rc);
        goto done;
    }
    stage = "prepare response reader argument";
    argument = rxvml_value_new(ctx);
    if (!argument || rxvml_set_str(argument, response, strlen(response)) != 0) goto done;
    stage = "invoke response reader";
    if (rxvml_call_procedure_descriptor(ctx,
            "rxsig1|llrxinline.selectionresult|.string[*]|path=.string",
            1u, &argument, &result) != 0 || !result) goto done;
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
    if (ctx) rxvml_destroy(ctx);
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
        char name[64];
        modules[i].original = selected[i];
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
    if (!inline_invoke(modules, count, options, manifest, response, transaction)) goto done;
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
