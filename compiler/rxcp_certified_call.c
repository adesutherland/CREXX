/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 *
 * Compiler-owned constant evaluation for individually certified Level B
 * callables.  The eligibility universe is deterministic, non-I/O, non-random
 * core Level B, but no callable is trusted by category alone.  Every registry
 * entry pins the fully-qualified API, typed callable summary and normalized
 * body fingerprint, then admits only evaluator-specific non-signalling cases.
 */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxcpmain.h"
#include "rxcp_certified_call.h"
#include "rxcp_constant.h"
#include "rxvmvars.h"
#include "rxvalue.h"

#define RXCP_CERT_POLICY_DETERMINISTIC 1u
#define RXCP_CERT_POLICY_NO_IO         2u
#define RXCP_CERT_POLICY_NO_RANDOM     4u
#define RXCP_CERT_POLICY_NO_AMBIENT    8u
#define RXCP_CERT_POLICY_OWNED_RESULT 16u
#define RXCP_CERT_POLICY_SUCCESS_ONLY 32u
#define RXCP_CERT_REQUIRED_POLICY     63u

/* Constant evaluation is an optimization, not a way for a source literal to
 * demand unbounded compiler memory.  Wider valid results retain the ordinary
 * Level B call and are constructed at runtime. */
#define RXCP_CERT_MAX_RESULT_CODEPOINTS ((size_t)1048576u)

typedef struct RxcpCertifiedBodyFingerprint {
    uint64_t first;
    uint64_t second;
    size_t nodes;
} RxcpCertifiedBodyFingerprint;

struct RxcpCertifiedCallDescriptor;

typedef int (*RxcpCertifiedEvaluator)(ASTNode *call,
                                      RxcpCertifiedCallResult *result);

typedef struct RxcpCertifiedCallDescriptor {
    const char *fq_name;
    unsigned int semantic_revision;
    unsigned int policy;
    ValueType result_type;
    unsigned int result_flags;
    size_t formal_count;
    const ValueType *formal_types;
    const unsigned int *formal_flags;
    size_t structural_nodes;
    size_t assignments;
    size_t branches;
    size_t calls;
    size_t inline_temporaries;
    uint64_t body_hash_first;
    uint64_t body_hash_second;
    size_t body_nodes;
    RxcpCertifiedEvaluator evaluator;
} RxcpCertifiedCallDescriptor;

static int certified_eval_upper(ASTNode *call,
                                RxcpCertifiedCallResult *result);
static int certified_eval_lower(ASTNode *call,
                                RxcpCertifiedCallResult *result);
static int certified_eval_length(ASTNode *call,
                                 RxcpCertifiedCallResult *result);
static int certified_eval_left(ASTNode *call,
                               RxcpCertifiedCallResult *result);
static int certified_eval_right(ASTNode *call,
                                RxcpCertifiedCallResult *result);
static int certified_eval_substr(ASTNode *call,
                                 RxcpCertifiedCallResult *result);
static int certified_eval_word(ASTNode *call,
                               RxcpCertifiedCallResult *result);

static const ValueType upper_formal_types[] = { TP_STRING };
static const unsigned int upper_formal_flags[] = { 400u };
static const ValueType width_formal_types[] = {
    TP_STRING, TP_INTEGER, TP_STRING
};
static const unsigned int width_formal_flags[] = {
    416u, 400u, 406u
};
static const ValueType substr_formal_types[] = {
    TP_STRING, TP_INTEGER, TP_INTEGER, TP_STRING
};
static const unsigned int substr_formal_flags[] = {
    416u, 400u, 406u, 406u
};
static const ValueType word_formal_types[] = { TP_STRING, TP_INTEGER };
static const unsigned int word_formal_flags[] = { 416u, 400u };

/* Fingerprints are filled from the normalized, source-location-independent
 * callable template.  Zero is deliberately never accepted; a source/body
 * drift therefore disables only that certificate until it is re-proved. */
static const RxcpCertifiedCallDescriptor certified_calls[] = {
    {
        "rxfnsb.upper", 1u, RXCP_CERT_REQUIRED_POLICY,
        TP_STRING, 4u, 1u, upper_formal_types, upper_formal_flags,
        9u, 1u, 0u, 0u, 0u,
        UINT64_C(0x7ab468cb7fc1817e), UINT64_C(0xa514f6547af39a74), 15u,
        certified_eval_upper
    },
    {
        "rxfnsb.lower", 1u, RXCP_CERT_REQUIRED_POLICY,
        TP_STRING, 4u, 1u, upper_formal_types, upper_formal_flags,
        9u, 1u, 0u, 0u, 0u,
        UINT64_C(0x4471a24a7a248be2), UINT64_C(0x88bb43caefc392a4), 15u,
        certified_eval_lower
    },
    {
        "rxfnsb.length", 1u, RXCP_CERT_REQUIRED_POLICY,
        TP_INTEGER, 4u, 1u, upper_formal_types, upper_formal_flags,
        9u, 1u, 0u, 0u, 0u,
        UINT64_C(0xce2472324779bc1a), UINT64_C(0xaa9218c243fe69a0), 15u,
        certified_eval_length
    },
    {
        "rxfnsb.left", 1u, RXCP_CERT_REQUIRED_POLICY,
        TP_STRING, 6u, 3u, width_formal_types, width_formal_flags,
        92u, 9u, 5u, 0u, 0u,
        UINT64_C(0x2f0c7df6db15347f), UINT64_C(0x95139df800c61ca6), 104u,
        certified_eval_left
    },
    {
        "rxfnsb.right", 1u, RXCP_CERT_REQUIRED_POLICY,
        TP_STRING, 6u, 3u, width_formal_types, width_formal_flags,
        97u, 10u, 5u, 0u, 0u,
        UINT64_C(0xa267e62ba9a841a7), UINT64_C(0x41d29b5a5a3aca46), 109u,
        certified_eval_right
    },
    {
        "rxfnsb.substr", 1u, RXCP_CERT_REQUIRED_POLICY,
        TP_STRING, 6u, 4u, substr_formal_types, substr_formal_flags,
        164u, 16u, 12u, 0u, 0u,
        UINT64_C(0x877a0257c5b9fb92), UINT64_C(0x4e769c5ff069bfb5), 179u,
        certified_eval_substr
    },
    {
        "rxfnsb.word", 1u, RXCP_CERT_REQUIRED_POLICY,
        TP_STRING, 6u, 2u, word_formal_types, word_formal_flags,
        101u, 11u, 8u, 0u, 0u,
        UINT64_C(0x71cf535af3073734), UINT64_C(0xdf4a05a1202f5127), 110u,
        certified_eval_word
    }
};

static size_t certified_call_count(void) {
    return sizeof(certified_calls) / sizeof(certified_calls[0]);
}

static Context *certified_root_context(Context *context) {
    return context && context->master_context ? context->master_context : context;
}

static void certified_debug(Context *context,
                            ASTNode *call,
                            const RxcpCertifiedCallDescriptor *descriptor,
                            const char *reason,
                            const RxcpCertifiedBodyFingerprint *fingerprint) {
    Context *root;

    root = certified_root_context(context);
    if (!root || root->debug_mode < 1 || !descriptor || !reason) return;
    fprintf(stderr, "DEBUG_CERTIFIED_CALL %s", descriptor->fq_name);
    if (call && call->file_name) {
        fprintf(stderr, " @ %s", call->file_name);
        if (call->line > 0) fprintf(stderr, ":%d", call->line);
        if (call->column > 0) fprintf(stderr, ":%d", call->column);
    }
    fprintf(stderr, " - %s", reason);
    if (fingerprint) {
        fprintf(stderr,
                ": hash=%016llx/%016llx nodes=%lu",
                (unsigned long long)fingerprint->first,
                (unsigned long long)fingerprint->second,
                (unsigned long)fingerprint->nodes);
    }
    fputc('\n', stderr);
}

static const RxcpCertifiedCallDescriptor *certified_descriptor(ASTNode *call) {
    Symbol *symbol;
    char *fq_name;
    size_t i;
    const RxcpCertifiedCallDescriptor *result;

    if (!call || call->node_type != FUNCTION || !call->symbolNode ||
        !(symbol = call->symbolNode->symbol) ||
        symbol->symbol_type != FUNCTION_SYMBOL) return 0;

    fq_name = sym_frnm(symbol);
    if (!fq_name) return 0;
    result = 0;
    for (i = 0; i < certified_call_count(); i++) {
        if (strcmp(fq_name, certified_calls[i].fq_name) == 0) {
            result = &certified_calls[i];
            break;
        }
    }
    free(fq_name);
    return result;
}

int rxcp_certified_call_candidate(ASTNode *call) {
    return certified_descriptor(call) != 0;
}

static void certified_hash_byte(RxcpCertifiedBodyFingerprint *hash,
                                unsigned int byte) {
    hash->first ^= (uint64_t)(byte & 0xffu);
    hash->first *= UINT64_C(1099511628211);
    hash->second ^= (uint64_t)(byte & 0xffu) + UINT64_C(0x9e3779b97f4a7c15) +
                    (hash->second << 6) + (hash->second >> 2);
    hash->second *= UINT64_C(0xbf58476d1ce4e5b9);
}

static void certified_hash_uint64(RxcpCertifiedBodyFingerprint *hash,
                                  uint64_t value) {
    unsigned int i;
    for (i = 0; i < 8u; i++) {
        certified_hash_byte(hash, (unsigned int)(value & UINT64_C(0xff)));
        value >>= 8;
    }
}

static void certified_hash_bytes(RxcpCertifiedBodyFingerprint *hash,
                                 const char *bytes,
                                 size_t length) {
    size_t i;

    certified_hash_uint64(hash, (uint64_t)length);
    for (i = 0; i < length; i++) {
        certified_hash_byte(hash, (unsigned int)(unsigned char)bytes[i]);
    }
}

static void certified_hash_dims(RxcpCertifiedBodyFingerprint *hash,
                                size_t dims,
                                const int *base,
                                const int *elements) {
    size_t i;

    certified_hash_uint64(hash, (uint64_t)dims);
    for (i = 0; i < dims; i++) {
        certified_hash_uint64(hash, (uint64_t)(int64_t)(base ? base[i] : 0));
        certified_hash_uint64(hash,
                              (uint64_t)(int64_t)(elements ? elements[i] : 0));
    }
}

static void certified_hash_node(RxcpCertifiedBodyFingerprint *hash,
                                const ASTNode *node) {
    const ASTNode *child;
    size_t child_count;
    uint64_t float_bits;

    if (!node) {
        certified_hash_byte(hash, 0u);
        return;
    }

    hash->nodes++;
    certified_hash_byte(hash, 0xa1u);
    certified_hash_uint64(hash, (uint64_t)node->node_type);
    certified_hash_uint64(hash, (uint64_t)node->value_type);
    certified_hash_uint64(hash, (uint64_t)node->target_type);
    certified_hash_dims(hash, node->value_dims,
                        node->value_dim_base, node->value_dim_elements);
    certified_hash_dims(hash, node->target_dims,
                        node->target_dim_base, node->target_dim_elements);
    certified_hash_bytes(hash,
                         node->node_string ? node->node_string : "",
                         node->node_string ? node->node_string_length : 0u);
    certified_hash_bytes(hash,
                         node->decimal_value ? node->decimal_value : "",
                         node->decimal_value ? strlen(node->decimal_value) : 0u);

    if (node->node_type == INTEGER || node->node_type == CONSTANT ||
        node->value_type == TP_INTEGER || node->value_type == TP_BOOLEAN) {
        certified_hash_uint64(hash, (uint64_t)(int64_t)node->int_value);
    } else {
        certified_hash_uint64(hash, UINT64_C(0));
    }
    float_bits = UINT64_C(0);
    if (node->node_type == FLOAT || node->value_type == TP_FLOAT) {
        memcpy(&float_bits, &node->float_value,
               sizeof(node->float_value) < sizeof(float_bits) ?
               sizeof(node->float_value) : sizeof(float_bits));
    }
    certified_hash_uint64(hash, float_bits);

    certified_hash_uint64(hash, (uint64_t)(unsigned int)node->is_ref_arg);
    certified_hash_uint64(hash, (uint64_t)(unsigned int)node->is_opt_arg);
    certified_hash_uint64(hash, (uint64_t)(unsigned int)node->is_varg);
    certified_hash_uint64(hash, (uint64_t)(unsigned int)node->is_const_arg);
    certified_hash_uint64(hash,
                          node->symbolNode ?
                          (uint64_t)node->symbolNode->readUsage : UINT64_C(0));
    certified_hash_uint64(hash,
                          node->symbolNode ?
                          (uint64_t)node->symbolNode->writeUsage : UINT64_C(0));
    certified_hash_uint64(hash,
                          node->association ?
                          (uint64_t)node->association->node_type : UINT64_C(0));

    child_count = 0;
    for (child = node->child; child; child = child->sibling) child_count++;
    certified_hash_uint64(hash, (uint64_t)child_count);
    for (child = node->child; child; child = child->sibling) {
        certified_hash_node(hash, child);
    }
    certified_hash_byte(hash, 0x1au);
}

static RxcpCertifiedBodyFingerprint certified_body_fingerprint(ASTNode *body) {
    RxcpCertifiedBodyFingerprint result;

    result.first = UINT64_C(14695981039346656037);
    result.second = UINT64_C(0x243f6a8885a308d3);
    result.nodes = 0;
    certified_hash_node(&result, body);
    return result;
}

static int certified_summary_matches(const RxcpCertifiedCallDescriptor *descriptor,
                                     const InlineCallableSummary *summary) {
    size_t i;

    if (!descriptor || !summary ||
        summary->schema_version != RXCP_INLINE_CALLABLE_SUMMARY_SCHEMA ||
        descriptor->policy != RXCP_CERT_REQUIRED_POLICY ||
        summary->formal_count != descriptor->formal_count ||
        (summary->formal_count && !summary->formals) ||
        summary->result_type != descriptor->result_type ||
        summary->result_dims != 0u ||
        summary->result_flags != descriptor->result_flags ||
        summary->control_flags != 0u ||
        summary->context_flags != (RXCP_INLINE_CONTEXT_SOURCE_IDENTITY |
                                   RXCP_INLINE_CONTEXT_TRACE_IDENTITY |
                                   RXCP_INLINE_CONTEXT_NUMERIC) ||
        summary->structural_nodes != descriptor->structural_nodes ||
        summary->assignments != descriptor->assignments ||
        summary->branches != descriptor->branches ||
        summary->calls != descriptor->calls ||
        summary->inline_temp_definitions != descriptor->inline_temporaries) {
        return 0;
    }

    for (i = 0; i < summary->formal_count; i++) {
        if (summary->formals[i].type != descriptor->formal_types[i] ||
            summary->formals[i].dims != 0u ||
            summary->formals[i].flags != descriptor->formal_flags[i]) return 0;
    }
    return 1;
}

static int certified_provider_matches(Context *context,
                                      ASTNode *call,
                                      const RxcpCertifiedCallDescriptor *descriptor) {
    Symbol *symbol;
    RxcpCertifiedBodyFingerprint fingerprint;

    if (!call || !call->symbolNode || !(symbol = call->symbolNode->symbol) ||
        !symbol->is_inlinable || !symbol->ast_template ||
        !certified_summary_matches(descriptor, symbol->inline_summary)) {
        certified_debug(context, call, descriptor,
                        "reject: missing or contradictory callable proof", 0);
        return 0;
    }

    fingerprint = certified_body_fingerprint(symbol->ast_template);
    if (!descriptor->body_hash_first || !descriptor->body_hash_second ||
        !descriptor->body_nodes ||
        fingerprint.first != descriptor->body_hash_first ||
        fingerprint.second != descriptor->body_hash_second ||
        fingerprint.nodes != descriptor->body_nodes) {
        certified_debug(context, call, descriptor,
                        "reject: callable body fingerprint mismatch", &fingerprint);
        return 0;
    }
    return 1;
}

static int certified_constant_string(ASTNode *node,
                                     unsigned char **bytes,
                                     size_t *length) {
    unsigned char *decoded;
    size_t decoded_length;

    if (!node || node->node_type != CONSTANT || node->value_type != TP_STRING ||
        (node->node_string_length && !node->node_string)) return 0;
    decoded = rxcp_constant_string_decode(node->node_string ? node->node_string : "",
                                          node->node_string_length,
                                          &decoded_length);
    if (!decoded) return 0;
    if (bytes) *bytes = decoded;
    else free(decoded);
    if (length) *length = decoded_length;
    return 1;
}

static int certified_constant_integer(ASTNode *node, rxinteger *result) {
    if (!node || node->node_type != CONSTANT || node->value_type != TP_INTEGER) return 0;
    if (result) *result = node->int_value;
    return 1;
}

static int certified_copy_value_string(const value *source,
                                       RxcpCertifiedCallResult *result) {
    char *buffer;
    size_t encoded_length;

    buffer = rxcp_constant_string_encode(
            (const unsigned char *)(source->string_value ? source->string_value : ""),
            source->string_length,
            &encoded_length);
    if (!buffer) return 0;
    result->type = TP_STRING;
    result->string_value = buffer;
    result->string_length = encoded_length;
    return 1;
}

static int certified_eval_case(ASTNode *call,
                               RxcpCertifiedCallResult *result,
                               int uppercase) {
    ASTNode *actual;
    unsigned char *bytes;
    size_t length;
    value source;
    value output;
    int ok;

    actual = call ? call->child : 0;
    if (!actual || actual->sibling ||
        !certified_constant_string(actual, &bytes, &length)) return 0;

    value_init(&source);
    value_init(&output);
    ok = 0;
    if (set_string_validated(&source, (const char *)bytes, length) == 0) {
        set_value_string(&output, &source);
#ifdef NUTF8
        {
            size_t i;
            for (i = 0; i < output.string_length; i++) {
                output.string_value[i] = (char)(uppercase ?
                        toupper((unsigned char)output.string_value[i]) :
                        tolower((unsigned char)output.string_value[i]));
            }
        }
#else
        {
            char *current;
            char *next;
            char *end;
            utf8_int32_t codepoint;
            utf8_int32_t mapped;

            current = output.string_value;
            end = current + output.string_length;
            while (current < end) {
                next = utf8codepoint(current, &codepoint);
                mapped = uppercase ? utf8uprcodepoint(codepoint) :
                                     utf8lwrcodepoint(codepoint);
                if (mapped != codepoint) {
                    utf8catcodepoint(current, mapped, (size_t)(next - current));
                }
                current = next;
            }
        }
#endif
        ok = certified_copy_value_string(&output, result);
    }
    free(bytes);
    clear_value(&output);
    clear_value(&source);
    return ok;
}

static int certified_eval_upper(ASTNode *call,
                                RxcpCertifiedCallResult *result) {
    return certified_eval_case(call, result, 1);
}

static int certified_eval_lower(ASTNode *call,
                                RxcpCertifiedCallResult *result) {
    return certified_eval_case(call, result, 0);
}

static int certified_eval_length(ASTNode *call,
                                 RxcpCertifiedCallResult *result) {
    ASTNode *actual;
    unsigned char *bytes;
    size_t byte_length;
    size_t char_count;
    value source;
    int ok;

    actual = call ? call->child : 0;
    if (!actual || actual->sibling ||
        !certified_constant_string(actual, &bytes, &byte_length)) return 0;

    value_init(&source);
    ok = 0;
    if (set_string_validated(&source, (const char *)bytes, byte_length) == 0) {
#ifdef NUTF8
        char_count = source.string_length;
#else
        char_count = source.string_chars;
#endif
        if (char_count <= (size_t)RXINTEGER_MAX) {
            result->type = TP_INTEGER;
            result->int_value = (rxinteger)char_count;
            ok = 1;
        }
    }
    free(bytes);
    clear_value(&source);
    return ok;
}

static int certified_eval_width(ASTNode *call,
                                RxcpCertifiedCallResult *result,
                                int keep_right) {
    ASTNode *source_node;
    ASTNode *width_node;
    ASTNode *pad_node;
    unsigned char *source_bytes;
    unsigned char *pad_bytes;
    unsigned char *combined;
    size_t source_byte_length;
    size_t pad_byte_length;
    size_t source_chars;
    size_t width_chars;
    size_t padding_chars;
    size_t result_byte_length;
    size_t source_offset;
    size_t i;
    rxinteger requested_width;
    value source;
    value pad;
    value output;
    int ok;

    if (!call) return 0;
    source_bytes = 0;
    pad_bytes = 0;
    combined = 0;
    source_node = call->child;
    width_node = source_node ? source_node->sibling : 0;
    pad_node = width_node ? width_node->sibling : 0;
    if (!source_node || !width_node || !pad_node || pad_node->sibling ||
        !certified_constant_integer(width_node, &requested_width) ||
        requested_width < 0 ||
        !certified_constant_string(source_node, &source_bytes,
                                   &source_byte_length)) return 0;

    value_init(&source);
    value_init(&pad);
    value_init(&output);
    ok = 0;

    /* LEFT/RIGHT validate PAD after WIDTH and before their width-zero return. */
    if (pad_node->node_type == NOVAL) {
        if (set_string_validated(&pad, " ", 1u) != 0) goto done;
        pad_bytes = (unsigned char *)malloc(1u);
        if (!pad_bytes) goto done;
        pad_bytes[0] = (unsigned char)' ';
        pad_byte_length = 1u;
    } else {
        if (!certified_constant_string(pad_node, &pad_bytes, &pad_byte_length) ||
            set_string_validated(&pad, (const char *)pad_bytes,
                                 pad_byte_length) != 0) goto done;
    }
#ifdef NUTF8
    if (pad.string_length != 1u) goto done;
#else
    if (pad.string_chars != 1u) goto done;
#endif

    if ((uint64_t)requested_width >
        (uint64_t)RXCP_CERT_MAX_RESULT_CODEPOINTS ||
        set_string_validated(&source, (const char *)source_bytes,
                             source_byte_length) != 0) goto done;
#ifdef NUTF8
    source_chars = source.string_length;
#else
    source_chars = source.string_chars;
#endif
    width_chars = (size_t)requested_width;

    if (width_chars <= source_chars) {
        source_offset = keep_right ? source_chars - width_chars : 0u;
        string_set_byte_pos(&source, source_offset);
        string_slice_from_cursor(&output, &source, width_chars);
        ok = certified_copy_value_string(&output, result);
        goto done;
    }

    padding_chars = width_chars - source_chars;
    if (padding_chars > (SIZE_MAX - source_byte_length) / pad_byte_length) goto done;
    result_byte_length = source_byte_length + padding_chars * pad_byte_length;
    combined = (unsigned char *)malloc(result_byte_length ? result_byte_length : 1u);
    if (!combined) goto done;

    if (keep_right) {
        for (i = 0; i < padding_chars; i++) {
            memcpy(combined + i * pad_byte_length, pad_bytes, pad_byte_length);
        }
        memcpy(combined + padding_chars * pad_byte_length,
               source_bytes, source_byte_length);
    } else {
        memcpy(combined, source_bytes, source_byte_length);
        for (i = 0; i < padding_chars; i++) {
            memcpy(combined + source_byte_length + i * pad_byte_length,
                   pad_bytes, pad_byte_length);
        }
    }
    if (set_string_validated(&output, (const char *)combined,
                             result_byte_length) != 0) goto done;
    ok = certified_copy_value_string(&output, result);

done:
    free(combined);
    free(pad_bytes);
    free(source_bytes);
    clear_value(&output);
    clear_value(&pad);
    clear_value(&source);
    return ok;
}

static int certified_eval_left(ASTNode *call,
                               RxcpCertifiedCallResult *result) {
    return certified_eval_width(call, result, 0);
}

static int certified_eval_right(ASTNode *call,
                                RxcpCertifiedCallResult *result) {
    return certified_eval_width(call, result, 1);
}

static int certified_eval_substr(ASTNode *call,
                                 RxcpCertifiedCallResult *result) {
    ASTNode *source_node;
    ASTNode *start_node;
    ASTNode *length_node;
    ASTNode *pad_node;
    unsigned char *source_bytes;
    unsigned char *pad_bytes;
    size_t source_length;
    size_t pad_length;
    rxinteger start;
    rxinteger requested;
    size_t start_offset;
    size_t char_count;
    size_t requested_count;
    value source;
    value pad;
    value output;
    int ok;

    if (!call) return 0;
    source_bytes = 0;
    pad_bytes = 0;
    source_node = call->child;
    start_node = source_node ? source_node->sibling : 0;
    length_node = start_node ? start_node->sibling : 0;
    pad_node = length_node ? length_node->sibling : 0;
    if (!source_node || !start_node || !length_node || !pad_node ||
        pad_node->sibling) return 0;
    if (!certified_constant_string(source_node, &source_bytes, &source_length)) {
        return 0;
    }
    if (!certified_constant_integer(start_node, &start) ||
        !certified_constant_integer(length_node, &requested) ||
        start < 1 || requested <= 0) {
        free(source_bytes);
        return 0;
    }

    value_init(&source);
    value_init(&pad);
    value_init(&output);
    ok = 0;

    if (pad_node->node_type == NOVAL) {
        if (set_string_validated(&pad, " ", 1u) != 0) goto done;
    } else {
        if (!certified_constant_string(pad_node, &pad_bytes, &pad_length) ||
            set_string_validated(&pad, (const char *)pad_bytes, pad_length) != 0) goto done;
    }
#ifdef NUTF8
    if (pad.string_length != 1u) goto done;
#else
    if (pad.string_chars != 1u) goto done;
#endif

    if (set_string_validated(&source, (const char *)source_bytes, source_length) != 0) goto done;
#ifdef NUTF8
    char_count = source.string_length;
#else
    char_count = source.string_chars;
#endif
    if ((uint64_t)(start - 1) > (uint64_t)char_count) goto done;
    start_offset = (size_t)(start - 1);
    if ((uint64_t)requested > (uint64_t)(char_count - start_offset)) goto done;
    requested_count = (size_t)requested;

    string_set_byte_pos(&source, start_offset);
    string_slice_from_cursor(&output, &source, requested_count);
    ok = certified_copy_value_string(&output, result);

done:
    free(pad_bytes);
    free(source_bytes);
    clear_value(&output);
    clear_value(&pad);
    clear_value(&source);
    return ok;
}

/* This is the exact codepoint set used by the VM's FNDBLNK/FNDNBLNK
 * instructions.  The WORD certificate is additionally pinned to the exact
 * Level B body and a semantic revision, so a deliberate change to either
 * contract disables or revises this evaluator instead of silently widening
 * it from host-locale isspace(). */
static int certified_word_is_blank(utf8_int32_t codepoint) {
    return codepoint == 0x0009 || codepoint == 0x000a ||
           codepoint == 0x000b || codepoint == 0x000c ||
           codepoint == 0x000d || codepoint == 0x0020 ||
           codepoint == 0x0085 || codepoint == 0x00a0 ||
           codepoint == 0x1680 ||
           (codepoint >= 0x2000 && codepoint <= 0x200a) ||
           codepoint == 0x2028 || codepoint == 0x2029 ||
           codepoint == 0x202f || codepoint == 0x205f ||
           codepoint == 0x3000;
}

static int certified_word_codepoint_at(value *source,
                                       size_t position,
                                       utf8_int32_t *codepoint) {
    if (!source || !codepoint) return 0;
#ifdef NUTF8
    if (position >= source->string_length) return 0;
    *codepoint = (utf8_int32_t)source->string_value[position];
#else
    if (position >= source->string_chars) return 0;
    string_set_byte_pos(source, position);
    utf8codepoint(source->string_value + source->string_pos, codepoint);
#endif
    return 1;
}

static int certified_eval_word(ASTNode *call,
                               RxcpCertifiedCallResult *result) {
    ASTNode *source_node;
    ASTNode *word_number_node;
    unsigned char *source_bytes;
    size_t source_byte_length;
    size_t source_chars;
    size_t scan;
    size_t word_start;
    size_t word_end;
    size_t requested_index;
    rxinteger requested_word;
    size_t current_word;
    utf8_int32_t codepoint;
    value source;
    value output;
    int ok;

    if (!call) return 0;
    source_bytes = 0;
    source_node = call->child;
    word_number_node = source_node ? source_node->sibling : 0;
    if (!source_node || !word_number_node || word_number_node->sibling ||
        !certified_constant_string(source_node, &source_bytes,
                                   &source_byte_length) ||
        !certified_constant_integer(word_number_node, &requested_word) ||
        requested_word < 1) {
        free(source_bytes);
        return 0;
    }

    value_init(&source);
    value_init(&output);
    ok = 0;
    if (set_string_validated(&source, (const char *)source_bytes,
                             source_byte_length) != 0) goto done;
#ifdef NUTF8
    source_chars = source.string_length;
#else
    source_chars = source.string_chars;
#endif

    if ((uint64_t)requested_word > (uint64_t)source_chars) {
        ok = certified_copy_value_string(&output, result);
        goto done;
    }
    requested_index = (size_t)requested_word;

    scan = 0;
    current_word = 0;
    while (scan < source_chars) {
        word_start = scan;
        while (word_start < source_chars) {
            if (!certified_word_codepoint_at(&source, word_start, &codepoint)) {
                goto done;
            }
            if (!certified_word_is_blank(codepoint)) break;
            word_start++;
        }
        if (word_start == source_chars) break;

        current_word++;
        word_end = word_start;
        while (word_end < source_chars) {
            if (!certified_word_codepoint_at(&source, word_end, &codepoint)) {
                goto done;
            }
            if (certified_word_is_blank(codepoint)) break;
            word_end++;
        }

        if (current_word == requested_index) {
            string_set_byte_pos(&source, word_start);
            string_slice_from_cursor(&output, &source, word_end - word_start);
            ok = certified_copy_value_string(&output, result);
            goto done;
        }
        scan = word_end;
    }

    /* Empty, blank-only and missing-word inputs all return the owned empty
     * string in the maintained Level B contract. */
    ok = certified_copy_value_string(&output, result);

done:
    free(source_bytes);
    clear_value(&output);
    clear_value(&source);
    return ok;
}

int rxcp_certified_call_evaluate(Context *context,
                                 ASTNode *call,
                                 RxcpCertifiedCallResult *result) {
    const RxcpCertifiedCallDescriptor *descriptor;

    if (!result) return 0;
    memset(result, 0, sizeof(*result));
    descriptor = certified_descriptor(call);
    if (!descriptor || !certified_provider_matches(context, call, descriptor)) return 0;
    if (!descriptor->evaluator(call, result)) {
        certified_debug(context, call, descriptor,
                        "retain Level B fallback: constant domain not proved", 0);
        rxcp_certified_call_result_clear(result);
        return 0;
    }
    if (result->type != descriptor->result_type) {
        certified_debug(context, call, descriptor,
                        "reject: evaluator result contradicts certificate", 0);
        rxcp_certified_call_result_clear(result);
        return 0;
    }
    certified_debug(context, call, descriptor, "fold: certified constant result", 0);
    return 1;
}

void rxcp_certified_call_result_clear(RxcpCertifiedCallResult *result) {
    if (!result) return;
    free(result->decimal_value);
    free(result->string_value);
    memset(result, 0, sizeof(*result));
}
