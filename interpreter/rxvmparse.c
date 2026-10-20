/* cREXX License (MIT). One frozen-PARSE engine for the opcode and cursor.
 * Packed targets, including kind 6 word boundaries, retain their existing
 * semantics. Authored Classic template section policy belongs to consumers. */
#include "rxvmparse.h"
#include "rxsignal.h"
#include "rxinteger.h"
#ifndef NUTF8
#include "utf.h"
#endif
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum parse_phase { P_READY, P_FIELD, P_INPUT, P_DONE, P_FAILED, P_CLOSED };
enum parse_stage { S_SCAN, S_PRE, S_BEGIN, S_CONTROL, S_TRAILING, S_DONE };

typedef struct parse_span {
    size_t start, bytes, chars;
} parse_span;

typedef struct parse_item {
    unsigned char kind, flags;
    size_t literal, bytes, chars, slot;
    uint64_t movement;
    unsigned int operand;
} parse_item;

typedef struct parse_resolved {
    unsigned char kind;
    const char *literal;
    size_t bytes, chars;
    uint64_t movement;
} parse_resolved;

struct rxvm_parse_cursor {
    char *source;
    unsigned char *plan;
    size_t source_bytes, source_chars, *offsets;
    parse_item *items;
    parse_span *captures;
    size_t count, results, dynamics;
    int classic_numeric;
    enum parse_phase phase;
    enum parse_stage stage;
    size_t event_item, event_slot;
    parse_span field;

    size_t index, pre, cursor, capture_start, pending_item;
    int pending, pre_started, pre_relative;
    size_t relative_anchor;
    unsigned char shared_kind;
    parse_resolved shared;
    char *shared_owned;

    /* Only one occurrence is requested at a time. Its text is moved to the
     * shared literal anchor when necessary, then freed after that last use. */
    size_t input_index;
    int input_ready;
    parse_resolved input;
    char *input_owned;
};

static uint64_t read_le(const unsigned char *bytes, size_t width) {
    uint64_t result = 0;
    size_t i;
    for (i = 0; i < width; ++i) result |= (uint64_t)bytes[i] << (i * 8u);
    return result;
}

static int text_chars(const char *text, size_t length, size_t *chars) {
#ifndef NUTF8
    if (utf8nvalid_count(text, length, chars)) return RXVM_PARSE_UNICODE;
#else
    (void)text;
    *chars = length;
#endif
    return RXVM_PARSE_DONE;
}

static unsigned char effective_kind(const parse_item *item) {
    if (item->kind == 7u) return 2u;
    return item->kind >= 8u ? (unsigned char)(item->kind - 5u) : item->kind;
}

static void release_input(rxvm_parse_cursor *p) {
    free(p->input_owned);
    p->input_owned = NULL;
    p->input_ready = 0;
}

static void release_buffers(rxvm_parse_cursor *p) {
    free(p->source); p->source = NULL;
    free(p->plan); p->plan = NULL;
    free(p->offsets); p->offsets = NULL;
    free(p->items); p->items = NULL;
    free(p->captures); p->captures = NULL;
    release_input(p);
    free(p->shared_owned); p->shared_owned = NULL;
}

int rxvm_parse_fail(rxvm_parse_cursor *p, int result) {
    if (result >= 0) result = RXVM_PARSE_INVALID;
    if (p && p->phase != P_DONE && p->phase != P_FAILED && p->phase != P_CLOSED)
        p->phase = P_FAILED;
    return result;
}

static int item_failure(rxvm_parse_cursor *p, size_t index, int result) {
    const parse_item *item = &p->items[index];
    p->event_item = index + 1u;
    p->event_slot = item->kind >= 7u ? item->operand + 1u : item->slot;
    return rxvm_parse_fail(p, result);
}

/* Validate all records, including unused literals/operands, before any event.
 * Completion here is the packed machine's target/control completion rule. */
static int decode_plan(rxvm_parse_cursor *p, size_t length) {
    size_t offset, header, i, stored = 0, completed = 0, pending_slot = 0;
    int pending = 0;
    unsigned char version;
    if (length < 8u || p->plan[0] != 0x50u) return RXVM_PARSE_INVALID;
    version = p->plan[1];
    header = (size_t)read_le(p->plan + 2u, 2u);
    if (version == 1u) {
        if (header != 8u) return RXVM_PARSE_INVALID;
    } else if (version == 2u) {
        if (header != 12u || length < 12u || (p->plan[10] & ~1u) || p->plan[11])
            return RXVM_PARSE_INVALID;
        p->dynamics = (size_t)read_le(p->plan + 8u, 2u);
        p->classic_numeric = (p->plan[10] & 1u) != 0;
    } else return RXVM_PARSE_INVALID;
    p->count = (size_t)read_le(p->plan + 4u, 2u);
    p->results = (size_t)read_le(p->plan + 6u, 2u);
    if (!p->count) return RXVM_PARSE_INVALID;
    if (p->count > SIZE_MAX / sizeof(*p->items) ||
        p->results > SIZE_MAX / sizeof(*p->captures)) return RXVM_PARSE_RANGE;
    p->items = (parse_item *)calloc(p->count, sizeof(*p->items));
    if (!p->items) return RXVM_PARSE_NOMEM;
    if (p->results) {
        p->captures = (parse_span *)calloc(p->results, sizeof(*p->captures));
        if (!p->captures) return RXVM_PARSE_NOMEM;
    }
    offset = header;
    for (i = 0; i < p->count; ++i) {
        parse_item *item = &p->items[i];
        if (offset > length || length - offset < 2u) return RXVM_PARSE_INVALID;
        item->kind = p->plan[offset++]; item->flags = p->plan[offset++];
        switch (item->kind) {
        case 1:
            if (item->flags & ~1u) return RXVM_PARSE_INVALID;
            if (pending && pending_slot) completed = pending_slot;
            pending = 1;
            pending_slot = 0;
            if (item->flags) item->slot = pending_slot = ++stored;
            break;
        case 2: {
            size_t chars;
            int result;
            if (item->flags || length - offset < 8u) return RXVM_PARSE_INVALID;
            item->bytes = (size_t)read_le(p->plan + offset, 4u);
            item->chars = (size_t)read_le(p->plan + offset + 4u, 4u);
            offset += 8u;
            if (item->bytes > length - offset) return RXVM_PARSE_INVALID;
            item->literal = offset;
            result = text_chars((const char *)p->plan + offset, item->bytes, &chars);
            if (result) return result;
            if (chars != item->chars) return RXVM_PARSE_INVALID;
            offset += item->bytes;
            break;
        }
        case 3: case 4: case 5:
            if ((item->flags & ~1u) || length - offset < 8u) return RXVM_PARSE_INVALID;
            item->movement = read_le(p->plan + offset, 8u);
            if (item->movement > (uint64_t)RXINTEGER_MAX) return RXVM_PARSE_INVALID;
            offset += 8u;
            break;
        case 6:
            if (item->flags) return RXVM_PARSE_INVALID;
            break;
        case 7: case 8: case 9: case 10:
            if (version != 2u || (item->flags & ~1u) || length - offset < 2u)
                return RXVM_PARSE_INVALID;
            item->operand = (unsigned int)read_le(p->plan + offset, 2u);
            offset += 2u;
            if (item->flags) {
                if (item->operand >= completed) return RXVM_PARSE_INVALID;
            } else if (item->operand >= p->dynamics) return RXVM_PARSE_INVALID;
            break;
        default:
            return RXVM_PARSE_INVALID;
        }
        if (item->kind != 1u && pending) {
            if (pending_slot) completed = pending_slot;
            pending = 0;
        }
    }
    return offset == length && stored == p->results ? RXVM_PARSE_DONE : RXVM_PARSE_INVALID;
}

rxvm_parse_cursor *rxvm_parse_create(const char *source, size_t source_length,
                                    const unsigned char *plan, size_t plan_length,
                                    int *status) {
    rxvm_parse_cursor *p;
    size_t i, byte;
    int result;
    if (status) *status = RXVM_PARSE_INVALID;
    if ((!source && source_length) || !plan || !plan_length) return NULL;
    if (source_length >= (size_t)PTRDIFF_MAX || plan_length > (size_t)PTRDIFF_MAX ||
        source_length == SIZE_MAX || source_length > (uint64_t)RXINTEGER_MAX ||
        plan_length > (uint64_t)RXINTEGER_MAX) {
        if (status) *status = RXVM_PARSE_RANGE;
        return NULL;
    }
    p = (rxvm_parse_cursor *)calloc(1, sizeof(*p));
    if (!p) { if (status) *status = RXVM_PARSE_NOMEM; return NULL; }
    p->source_bytes = source_length;
    result = text_chars(source ? source : "", source_length, &p->source_chars);
    if (result) goto fail;
    if (p->source_chars == SIZE_MAX) { result = RXVM_PARSE_RANGE; goto fail; }
    p->source = (char *)malloc(source_length + 1u);
    p->plan = (unsigned char *)malloc(plan_length);
    if (!p->source || !p->plan) { result = RXVM_PARSE_NOMEM; goto fail; }
    if (source_length) memcpy(p->source, source, source_length);
    p->source[source_length] = 0;
    memcpy(p->plan, plan, plan_length);
    result = decode_plan(p, plan_length);
    if (result) goto fail;
    /* ASCII/byte builds need no index. Unicode seeks otherwise use one owned
     * offset index, avoiding a full prefix scan for each blank/word character. */
    if (p->source_chars != source_length) {
        if (p->source_chars + 1u > (size_t)PTRDIFF_MAX / sizeof(*p->offsets)) {
            result = RXVM_PARSE_RANGE; goto fail;
        }
        p->offsets = (size_t *)malloc((p->source_chars + 1u) * sizeof(*p->offsets));
        if (!p->offsets) { result = RXVM_PARSE_NOMEM; goto fail; }
        byte = 0;
        for (i = 0; i < p->source_chars; ++i) {
            p->offsets[i] = byte;
#ifndef NUTF8
            byte += utf8codepointcalcsize(p->source + byte);
#endif
        }
        p->offsets[p->source_chars] = source_length;
    }
    p->phase = P_READY; p->stage = S_SCAN; p->cursor = 1u;
    if (status) *status = RXVM_PARSE_DONE;
    return p;
fail:
    if (status) *status = result;
    rxvm_parse_destroy(p);
    return NULL;
}

static size_t byte_offset(const rxvm_parse_cursor *p, size_t character) {
    if (character >= p->source_chars) return p->source_bytes;
    return p->offsets ? p->offsets[character] : character;
}

static parse_span span(const rxvm_parse_cursor *p, size_t first, size_t chars) {
    parse_span result;
    size_t start = first ? first - 1u : 0u;
    if (start > p->source_chars) start = p->source_chars;
    if (chars > p->source_chars - start) chars = p->source_chars - start;
    result.start = byte_offset(p, start);
    result.bytes = byte_offset(p, start + chars) - result.start;
    result.chars = chars;
    return result;
}

static int blank(const rxvm_parse_cursor *p, size_t position) {
    return position <= p->source_chars && p->source[byte_offset(p, position - 1u)] == ' ';
}

static size_t find_literal(const rxvm_parse_cursor *p, size_t start,
                           const parse_resolved *literal) {
    size_t offset, low, high;
    const char *candidate, *last;
    if (!literal->bytes || !start || start > p->source_chars) return 0;
    offset = byte_offset(p, start - 1u);
    if (literal->bytes > p->source_bytes - offset) return 0;
    candidate = p->source + offset;
    last = p->source + p->source_bytes - literal->bytes;
    while (candidate <= last) {
        candidate = (const char *)memchr(candidate, (unsigned char)literal->literal[0],
                                         (size_t)(last - candidate) + 1u);
        if (!candidate) return 0;
        if (!memcmp(candidate, literal->literal, literal->bytes)) {
            offset = (size_t)(candidate - p->source);
            if (!p->offsets) return offset + 1u;
            low = 0; high = p->source_chars;
            while (low < high) {
                size_t middle = low + (high - low) / 2u;
                if (p->offsets[middle] < offset) low = middle + 1u;
                else high = middle;
            }
            return low + 1u;
        }
        ++candidate;
    }
    return 0;
}

static int equals_at(const rxvm_parse_cursor *p, size_t start,
                      const parse_resolved *literal) {
    size_t offset;
    if (!start || start - 1u > p->source_chars ||
        literal->chars > p->source_chars - (start - 1u)) return 0;
    offset = byte_offset(p, start - 1u);
    return literal->bytes <= p->source_bytes - offset &&
           (!literal->bytes || !memcmp(p->source + offset, literal->literal, literal->bytes));
}

/* Compare the int64 movement with the bounded room before narrowing to size_t.
 * In particular a huge legal movement saturates correctly on 31-bit hosts. */
static size_t move_cursor(const rxvm_parse_cursor *p, size_t cursor,
                           const parse_resolved *item) {
    size_t room;
    if (item->kind == 3u) {
        if (!item->movement) return 1u;
        return item->movement > p->source_chars + 1u ? p->source_chars + 1u : (size_t)item->movement;
    }
    if (item->kind == 4u) {
        room = p->source_chars + 1u - cursor;
        return item->movement > room ? p->source_chars + 1u : cursor + (size_t)item->movement;
    }
    return item->movement >= cursor ? 1u : cursor - (size_t)item->movement;
}

static int position_value(const char *text, size_t length, uint64_t *movement) {
    char *copy, *end;
    rxinteger value;
    int invalid;
    /* The shared native integer parser needs a terminator. A counted operand
     * cannot silently accept a valid prefix followed by an embedded NUL. */
    if (memchr(text, 0, length)) return RXVM_PARSE_CONVERSION;
    copy = (char *)malloc(length + 1u);
    if (!copy) return RXVM_PARSE_NOMEM;
    if (length) memcpy(copy, text, length);
    copy[length] = 0;
    invalid = rxinteger_parse(copy, &end, &value);
    if (!invalid) {
        while (*end && isspace((unsigned char)*end)) ++end;
        invalid = *end != 0 || value < 0;
    }
    free(copy);
    if (invalid) return RXVM_PARSE_CONVERSION;
    *movement = (uint64_t)value;
    return RXVM_PARSE_DONE;
}

static int resolve(rxvm_parse_cursor *p, size_t index, parse_resolved *result) {
    const parse_item *item = &p->items[index];
    parse_span captured;
    int status;
    memset(result, 0, sizeof(*result));
    result->kind = effective_kind(item);
    result->movement = item->movement;
    if (item->kind == 2u) {
        result->literal = (const char *)p->plan + item->literal;
        result->bytes = item->bytes; result->chars = item->chars;
    }
    if (item->kind < 7u) return RXVM_PARSE_DONE;
    if (item->flags) {
        captured = p->captures[item->operand];
        if (item->kind == 7u) {
            result->literal = p->source + captured.start;
            result->bytes = captured.bytes; result->chars = captured.chars;
            return RXVM_PARSE_DONE;
        }
        status = position_value(p->source + captured.start, captured.bytes, &result->movement);
        return status ? item_failure(p, index, status) : RXVM_PARSE_DONE;
    }
    if (p->input_ready && p->input_index == index) {
        *result = p->input;
        return RXVM_PARSE_DONE;
    }
    release_input(p);
    p->input_index = index;
    p->event_item = index + 1u;
    p->event_slot = item->operand + 1u;
    p->phase = P_INPUT;
    return RXVM_PARSE_INPUT;
}

int rxvm_parse_supply(rxvm_parse_cursor *p, const char *text, size_t length) {
    const parse_item *item;
    size_t chars;
    int status;
    if (!p || p->phase != P_INPUT) return rxvm_parse_fail(p, RXVM_PARSE_INVALID);
    if ((!text && length) || length >= (size_t)PTRDIFF_MAX ||
        length == SIZE_MAX || length > (uint64_t)RXINTEGER_MAX)
        return rxvm_parse_fail(p, !text && length ? RXVM_PARSE_INVALID : RXVM_PARSE_RANGE);
    if (!text) text = "";
    status = text_chars(text, length, &chars);
    if (status) return rxvm_parse_fail(p, status);
    item = &p->items[p->input_index];
    memset(&p->input, 0, sizeof(p->input));
    p->input.kind = effective_kind(item);
    if (item->kind == 7u) {
        p->input_owned = (char *)malloc(length + 1u);
        if (!p->input_owned) return rxvm_parse_fail(p, RXVM_PARSE_NOMEM);
        if (length) memcpy(p->input_owned, text, length);
        p->input_owned[length] = 0;
        p->input.literal = p->input_owned;
        p->input.bytes = length; p->input.chars = chars;
    } else {
        status = position_value(text, length, &p->input.movement);
        if (status) return rxvm_parse_fail(p, status);
    }
    p->input_ready = 1;
    p->phase = P_READY;
    p->event_item = p->event_slot = 0;
    return RXVM_PARSE_DONE;
}

static size_t store_implicit(rxvm_parse_cursor *p, parse_span *field) {
    size_t first = p->capture_start, after;
    while (first <= p->source_chars && blank(p, first)) ++first;
    if (first > p->source_chars) {
        *field = span(p, first, 0);
        return p->source_chars + 1u;
    }
    after = first;
    while (after <= p->source_chars && !blank(p, after)) ++after;
    *field = span(p, first, after - first);
    if (after <= p->source_chars) ++after;
    return after;
}

static parse_span store_word(const rxvm_parse_cursor *p, int raw) {
    size_t first = p->capture_start, word_start = first, after;
    if (first > p->source_chars) return span(p, first, 0);
    while (word_start <= p->source_chars && blank(p, word_start)) ++word_start;
    if (word_start > p->source_chars) return span(p, first, p->source_chars - first + 1u);
    after = word_start;
    while (after <= p->source_chars && !blank(p, after)) ++after;
    if (!raw) return span(p, word_start, after - word_start);
    while (after <= p->source_chars && blank(p, after)) ++after;
    return span(p, first, after - first);
}

static size_t store_literal(rxvm_parse_cursor *p, const parse_resolved *item,
                             parse_span *field) {
    size_t position;
    if (!item->bytes) {
        *field = span(p, p->capture_start, 0);
        return p->cursor;
    }
    position = find_literal(p, p->cursor, item);
    if (!position) {
        *field = span(p, p->capture_start, p->capture_start > p->source_chars
                       ? 0u : p->source_chars - p->capture_start + 1u);
        return p->source_chars + 1u;
    }
    p->relative_anchor = position;
    *field = span(p, p->capture_start, position - p->capture_start);
    return position + item->chars;
}

static size_t store_numeric(rxvm_parse_cursor *p, const parse_resolved *item,
                             parse_span *field) {
    size_t cursor = p->cursor, position;
    int anchored = (item->kind == 4u || item->kind == 5u) && p->relative_anchor;
    if (anchored) {
        cursor = p->relative_anchor;
        p->relative_anchor = 0;
    }
    position = move_cursor(p, cursor, item);
    if (position <= p->capture_start) {
        *field = store_word(p, item->kind == 5u);
        if (item->kind != 5u || anchored) position = p->capture_start;
    } else *field = span(p, p->capture_start, position - p->capture_start);
    return position;
}

static int field_event(rxvm_parse_cursor *p, parse_span field) {
    size_t slot = p->items[p->pending_item].slot;
    if (slot) p->captures[slot - 1u] = field;
    p->field = field;
    p->event_item = p->pending_item + 1u;
    p->event_slot = slot;
    p->pending = 0; p->capture_start = 0;
    p->phase = P_FIELD;
    return RXVM_PARSE_FIELD;
}

static void begin_pre(rxvm_parse_cursor *p) {
    if (p->shared_kind == 3u) p->cursor = move_cursor(p, p->cursor, &p->shared);
    else if (p->shared_kind == 8u) {
        if (p->shared.chars >= p->cursor) p->cursor = p->source_chars + 1u;
        else {
            size_t position = p->cursor - p->shared.chars;
            p->cursor = equals_at(p, position, &p->shared) ? position : p->source_chars + 1u;
        }
    }
    p->shared_kind = 0;
    free(p->shared_owned); p->shared_owned = NULL;
    p->pre_relative = 0;
    p->pre_started = 1;
}

static int apply_pre(rxvm_parse_cursor *p) {
    parse_resolved item;
    size_t position;
    unsigned char following, next;
    int status, anchor;
    if (!p->pre_started) begin_pre(p);
    while (p->pre < p->index) {
        const parse_item *raw = &p->items[p->pre];
        /* This flag belongs only to static numeric overlay normalization. */
        if (raw->kind >= 3u && raw->kind <= 5u && raw->flags) {
            ++p->pre;
            continue;
        }
        status = resolve(p, p->pre, &item);
        if (status) return status;
        following = p->pre + 1u < p->index ? effective_kind(&p->items[p->pre + 1u]) : 0u;
        next = p->index + 1u < p->count ? effective_kind(&p->items[p->index + 1u]) : 0u;
        if (item.kind == 2u) {
            if (following == 4u || following == 5u) p->pre_relative = 1;
            position = item.bytes ? find_literal(p, p->cursor, &item) : p->cursor;
            if (item.bytes && !position) p->cursor = p->source_chars + 1u;
            else {
                anchor = following == 4u || following == 5u;
                if (!anchor && (!item.bytes || item.literal[0] != ' ') && next == 4u) anchor = 1;
                p->relative_anchor = anchor ? position : 0u;
                p->cursor = anchor ? position : position + item.chars;
            }
        } else if (item.kind == 6u) {
            while (p->cursor <= p->source_chars && blank(p, p->cursor)) ++p->cursor;
        } else p->cursor = move_cursor(p, p->cursor, &item);
        release_input(p);
        ++p->pre;
    }
    if (p->pre_relative) p->relative_anchor = 0;
    p->pre_started = 0;
    return RXVM_PARSE_DONE;
}

int rxvm_parse_next(rxvm_parse_cursor *p) {
    parse_resolved item;
    parse_span field;
    int status;
    if (!p) return RXVM_PARSE_INVALID;
    if (p->phase == P_DONE) return RXVM_PARSE_DONE;
    if (p->phase != P_READY && p->phase != P_FIELD) return rxvm_parse_fail(p, RXVM_PARSE_INVALID);
    p->phase = P_READY;
    p->event_item = p->event_slot = 0;
    for (;;) {
        switch (p->stage) {
        case S_SCAN:
            if (p->index == p->count) {
                if (p->pending) {
                    field = span(p, p->capture_start, p->capture_start > p->source_chars
                                     ? 0u : p->source_chars - p->capture_start + 1u);
                    p->stage = S_DONE;
                    return field_event(p, field);
                }
                p->stage = S_TRAILING;
            } else if (p->items[p->index].kind == 1u) {
                p->stage = S_PRE;
                if (p->pending) {
                    p->cursor = store_implicit(p, &field);
                    return field_event(p, field);
                }
            } else if (p->pending) p->stage = S_CONTROL;
            else ++p->index; /* Pre-target controls resolve at their actual use. */
            break;
        case S_PRE:
            status = apply_pre(p);
            if (status) return status;
            p->stage = S_BEGIN;
            break;
        case S_BEGIN:
            p->pending_item = p->index;
            p->capture_start = p->cursor;
            p->pending = 1;
            p->pre = ++p->index;
            p->stage = S_SCAN;
            break;
        case S_CONTROL:
            status = resolve(p, p->index, &item);
            if (status) return status;
            if (item.kind == 2u) {
                p->cursor = store_literal(p, &item, &field);
                /* Kind lookahead is structural only. Never resolve a later
                 * dynamic position just to recognize this anchor shape. */
                if (p->index + 2u < p->count && p->items[p->index + 1u].kind == 1u &&
                    effective_kind(&p->items[p->index + 2u]) == 4u) {
                    p->shared = item; p->shared_kind = 8u;
                    p->shared_owned = p->input_owned; p->input_owned = NULL;
                }
            } else if (item.kind >= 3u && item.kind <= 5u) {
                p->cursor = store_numeric(p, &item, &field);
                p->relative_anchor = 0;
                if (item.kind == 3u && p->index + 1u < p->count &&
                    p->items[p->index + 1u].kind == 1u) {
                    p->shared = item; p->shared_kind = 3u;
                }
            } else p->cursor = store_implicit(p, &field);
            release_input(p);
            p->pre = ++p->index;
            p->stage = S_SCAN;
            return field_event(p, field);
        case S_TRAILING:
            /* Even a targetless/trailing dynamic item must be checked. There
             * is no later target whose pre-control pass could request it. */
            while (p->pre < p->count) {
                status = resolve(p, p->pre, &item);
                if (status) return status;
                release_input(p);
                ++p->pre;
            }
            p->stage = S_DONE;
            break;
        case S_DONE:
            p->phase = P_DONE;
            return RXVM_PARSE_DONE;
        }
    }
}

size_t rxvm_parse_item(const rxvm_parse_cursor *p) {
    return p && (p->phase == P_FIELD || p->phase == P_INPUT || p->phase == P_FAILED) ? p->event_item : 0u;
}

size_t rxvm_parse_slot(const rxvm_parse_cursor *p) {
    return p && (p->phase == P_FIELD || p->phase == P_INPUT || p->phase == P_FAILED) ? p->event_slot : 0u;
}

int rxvm_parse_text(rxvm_parse_cursor *p, const char **text, size_t *length) {
    if (!p || p->phase != P_FIELD || !text || !length) return rxvm_parse_fail(p, RXVM_PARSE_INVALID);
    *text = p->source + p->field.start;
    *length = p->field.bytes;
    return RXVM_PARSE_DONE;
}

void rxvm_parse_close(rxvm_parse_cursor *p) {
    if (!p) return;
    release_buffers(p);
    p->phase = P_CLOSED;
    p->event_item = p->event_slot = 0;
    memset(&p->field, 0, sizeof(p->field));
}

void rxvm_parse_destroy(rxvm_parse_cursor *p) {
    if (!p) return;
    rxvm_parse_close(p);
    free(p);
}

int rxvm_parse_classic_numeric(const rxvm_parse_cursor *p) {
    return p && p->classic_numeric;
}

size_t rxvm_parse_result_count(const rxvm_parse_cursor *p) { return p ? p->results : 0u; }
size_t rxvm_parse_external_count(const rxvm_parse_cursor *p) { return p ? p->dynamics : 0u; }

rxvm_parse_diagnostic rxvm_parse_diagnose(int result, int classic_numeric) {
    rxvm_parse_diagnostic diagnostic;
    switch (result) {
    case RXVM_PARSE_UNICODE:
        diagnostic.signal = RXSIGNAL_UNICODE_ERROR;
        diagnostic.message = "Invalid Unicode text in compiled PARSE"; break;
    case RXVM_PARSE_RANGE:
        diagnostic.signal = RXSIGNAL_OUT_OF_RANGE;
        diagnostic.message = "Compiled PARSE storage extent is not representable"; break;
    case RXVM_PARSE_NOMEM:
        diagnostic.signal = RXSIGNAL_FAILURE;
        diagnostic.message = "Cannot allocate compiled PARSE state"; break;
    case RXVM_PARSE_CONVERSION:
        diagnostic.signal = classic_numeric ? RXSIGNAL_CLASSIC_SYNTAX : RXSIGNAL_CONVERSION_ERROR;
        diagnostic.message = classic_numeric
            ? "RXC-LC-26.4: Positional parameter of parsing template must be a whole number"
            : "Dynamic PARSE position is not a nonnegative native integer";
        break;
    default:
        diagnostic.signal = RXSIGNAL_INVALID_ARGUMENTS;
        diagnostic.message = "Invalid compact compiled PARSE descriptor"; break;
    }
    return diagnostic;
}
