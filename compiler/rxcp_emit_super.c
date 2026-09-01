/*
 * Compiler-owned final emission combiner for the NR-09 Class 2 mappings.
 *
 * This is deliberately narrower than the RXAS keyhole optimiser. It only sees
 * rxc-generated text, never accepts authored RXAS, and only combines exact
 * instruction templates whose ordered effects are represented by a canonical
 * large instruction. Source steps are barriers. Metadata and trace events may
 * remain between component instructions; wide forms retain their referenced
 * intermediate registers where required.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "rxcp_emit.h"
#include "rxcp_util.h"

#define SUPER_MAX_OPERANDS 8
#define SUPER_TOKEN_SIZE 128

typedef struct super_line {
    char *text;
    char mnemonic[48];
    char operand[SUPER_MAX_OPERANDS][SUPER_TOKEN_SIZE];
    int operand_count;
    int instruction;
    int removed;
    int block;
} super_line;

static char *super_copy(const char *start, size_t length) {
    char *result = malloc(length + 1);
    if (!result) return 0;
    memcpy(result, start, length);
    result[length] = 0;
    return result;
}

static void super_trim(char *text) {
    char *start;
    size_t length;
    if (!text) return;
    start = text;
    while (*start && isspace((unsigned char)*start)) start++;
    length = strlen(start);
    while (length && isspace((unsigned char)start[length - 1])) length--;
    if (start != text) memmove(text, start, length);
    text[length] = 0;
}

static int super_transparent_line(const char *text) {
    const char *p = text;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == 0 || *p == '\r' || *p == '\n') return 1;
    if (strncmp(p, ".traceevent ", 12) == 0) return 1;
    if (strncmp(p, ".meta ", 6) == 0) return 1;
    return 0;
}

static void super_parse_instruction(super_line *line) {
    const char *p;
    const char *name_start;
    const char *operand_start;
    size_t name_length;
    char *operands;
    char *cursor;

    if (!line || !line->text) return;
    p = line->text;
    if (p[0] != ' ' || p[1] != ' ' || p[2] != ' ' ||
        !islower((unsigned char)p[3])) return;
    p += 3;
    name_start = p;
    while (*p && !isspace((unsigned char)*p)) p++;
    name_length = (size_t)(p - name_start);
    if (!name_length || name_length >= sizeof(line->mnemonic)) return;
    memcpy(line->mnemonic, name_start, name_length);
    line->mnemonic[name_length] = 0;
    line->instruction = 1;
    while (*p == ' ' || *p == '\t') p++;
    operand_start = p;
    while (*p && *p != '\r' && *p != '\n') p++;
    operands = super_copy(operand_start, (size_t)(p - operand_start));
    if (!operands) return;
    super_trim(operands);
    if (!operands[0]) {
        free(operands);
        return;
    }
    cursor = operands;
    while (cursor && *cursor && line->operand_count < SUPER_MAX_OPERANDS) {
        char *comma = strchr(cursor, ',');
        size_t token_length = comma ? (size_t)(comma - cursor) : strlen(cursor);
        if (token_length >= SUPER_TOKEN_SIZE) token_length = SUPER_TOKEN_SIZE - 1;
        memcpy(line->operand[line->operand_count], cursor, token_length);
        line->operand[line->operand_count][token_length] = 0;
        super_trim(line->operand[line->operand_count]);
        line->operand_count++;
        cursor = comma ? comma + 1 : 0;
    }
    free(operands);
}

static super_line *super_split_lines(const char *text, size_t *count_out) {
    super_line *lines;
    size_t count;
    size_t capacity;
    const char *start;
    int block;

    if (count_out) *count_out = 0;
    if (!text) return 0;
    capacity = 128;
    count = 0;
    block = 0;
    lines = calloc(capacity, sizeof(super_line));
    if (!lines) return 0;
    start = text;
    while (*start) {
        const char *newline = strchr(start, '\n');
        size_t length = newline ? (size_t)(newline - start + 1) : strlen(start);
        if (count == capacity) {
            super_line *grown;
            capacity *= 2;
            grown = realloc(lines, capacity * sizeof(super_line));
            if (!grown) {
                size_t i;
                for (i = 0; i < count; i++) free(lines[i].text);
                free(lines);
                return 0;
            }
            lines = grown;
            memset(lines + count, 0, (capacity - count) * sizeof(super_line));
        }
        lines[count].text = super_copy(start, length);
        if (!lines[count].text) break;
        super_parse_instruction(&lines[count]);
        if (lines[count].instruction) {
            lines[count].block = block;
        } else if (!super_transparent_line(lines[count].text)) {
            block++;
        }
        count++;
        start += length;
    }
    if (count_out) *count_out = count;
    return lines;
}

static void super_free_lines(super_line *lines, size_t count) {
    size_t i;
    if (!lines) return;
    for (i = 0; i < count; i++) free(lines[i].text);
    free(lines);
}

static int super_is(const super_line *line, const char *mnemonic, int operands) {
    return line && line->instruction && !line->removed &&
           strcmp(line->mnemonic, mnemonic) == 0 &&
           line->operand_count == operands;
}

static int super_equal(const super_line *left, int left_operand,
                       const super_line *right, int right_operand) {
    return strcmp(left->operand[left_operand], right->operand[right_operand]) == 0;
}

static int super_integer(const char *text) {
    char *end;
    if (!text || !text[0]) return 0;
    (void)strtol(text, &end, 10);
    return *end == 0;
}

static int super_register(const char *text) {
    const char *p;
    if (!text || !isalpha((unsigned char)text[0])) return 0;
    p = text + 1;
    if (!isdigit((unsigned char)*p)) return 0;
    while (isdigit((unsigned char)*p)) p++;
    return *p == 0;
}

static int super_same_block(super_line **item, int count) {
    int i;
    for (i = 1; i < count; i++) {
        if (!item[i] || item[i]->block != item[0]->block) return 0;
    }
    return 1;
}

static void super_replace(super_line *line, char *replacement) {
    if (!line || !replacement) return;
    free(line->text);
    line->text = replacement;
}

static char *super_replace_once(const char *text,
                                const char *old_text,
                                const char *new_text) {
    const char *match;
    size_t prefix;
    size_t suffix;
    char *result;
    if (!text || !old_text || !old_text[0] || !new_text) return 0;
    match = strstr(text, old_text);
    if (!match) return 0;
    prefix = (size_t)(match - text);
    suffix = strlen(match + strlen(old_text));
    result = malloc(prefix + strlen(new_text) + suffix + 1);
    if (!result) return 0;
    memcpy(result, text, prefix);
    memcpy(result + prefix, new_text, strlen(new_text));
    memcpy(result + prefix + strlen(new_text),
           match + strlen(old_text), suffix + 1);
    return result;
}

static void super_retarget_trace(super_line *lines,
                                 size_t first,
                                 size_t last,
                                 const char *old_register,
                                 const char *new_register) {
    char *old_marker;
    char *new_marker;
    size_t i;
    if (!lines || !old_register || !new_register ||
        !super_register(old_register) || !super_register(new_register)) return;
    old_marker = mprintf("\"%c\" %s ", old_register[0], old_register + 1);
    new_marker = mprintf("\"%c\" %s ", new_register[0], new_register + 1);
    for (i = first; i <= last; i++) {
        char *updated;
        if (!strstr(lines[i].text, ".traceevent ")) continue;
        updated = super_replace_once(lines[i].text, old_marker, new_marker);
        if (updated) {
            free(lines[i].text);
            lines[i].text = updated;
        }
    }
    free(old_marker);
    free(new_marker);
}

static int super_trace_references(super_line *lines,
                                  size_t first,
                                  size_t last,
                                  const char *reg) {
    char *marker;
    size_t i;
    int found;
    if (!lines || !reg || !super_register(reg) || first > last) return 0;
    marker = mprintf("\"%c\" %s ", reg[0], reg + 1);
    found = 0;
    for (i = first; i <= last; i++) {
        if (strstr(lines[i].text, ".traceevent ") &&
            strstr(lines[i].text, marker)) {
            found = 1;
            break;
        }
    }
    free(marker);
    return found;
}

static void super_remove_after(super_line **item, int count) {
    int i;
    for (i = 1; i < count; i++) item[i]->removed = 1;
}

static int super_combine_five(super_line *lines, super_line **item) {
    super_line *a = item[0];
    super_line *b = item[1];
    super_line *c = item[2];
    super_line *d = item[3];
    super_line *e = item[4];
    if (!super_same_block(item, 5)) return 0;

    if (super_is(a, "icopy", 2) &&
        super_is(b, "itof", 1) && super_equal(a, 0, b, 0) &&
        super_is(c, "fmult", 3) && super_equal(a, 0, c, 0) &&
        super_equal(c, 0, c, 1) && !super_register(c->operand[2]) &&
        super_is(d, "icopy", 2) &&
        super_is(e, "itof", 1) && super_equal(d, 0, e, 0)) {
        super_replace(a, mprintf("   itof %s,%s\n",
                                 a->operand[0], a->operand[1]));
        super_replace(c, mprintf("   fmulticopy %s,%s,%s,%s\n",
                                 c->operand[0], c->operand[2],
                                 d->operand[0], d->operand[1]));
        b->removed = 1;
        d->removed = 1;
        return 5;
    }
    return 0;
}

static int super_combine_four(super_line *lines, super_line **item) {
    super_line *a = item[0];
    super_line *b = item[1];
    super_line *c = item[2];
    super_line *d = item[3];
    if (!super_same_block(item, 4)) return 0;

    if (super_is(a, "load", 2) && super_integer(a->operand[1]) &&
        super_is(b, "icopy", 2) && super_equal(a, 0, b, 1) &&
        super_is(c, "unlink", 1) && super_equal(b, 0, c, 0) &&
        super_is(d, "unlink", 1)) {
        super_replace(a, mprintf("   iloadsetunlinkn %s,%s,%s,%s\n",
                                 a->operand[0], b->operand[0],
                                 a->operand[1], d->operand[0]));
        super_retarget_trace(lines, (size_t)(b - lines + 1),
                             (size_t)(c - lines), b->operand[0], a->operand[0]);
        super_remove_after(item, 4);
        return 4;
    }

    if (super_is(a, "linkattr1", 3) && super_integer(a->operand[2]) &&
        super_is(b, "setattrs", 2) && super_equal(a, 0, b, 0) &&
        super_integer(b->operand[1]) &&
        super_is(c, "iadd", 3) && super_integer(c->operand[2]) &&
        super_is(d, "linkattr1", 3) && super_equal(a, 0, d, 1) &&
        super_equal(c, 0, d, 2) &&
        !super_trace_references(lines, (size_t)(c - lines + 1),
                                (size_t)(d - lines - 1), c->operand[0])) {
        super_replace(a, mprintf("   linksetattrslinkadd %s,%s,%s,%s,%s,%s,%s\n",
                                 a->operand[0], a->operand[1], a->operand[2],
                                 b->operand[1], d->operand[0],
                                 c->operand[1], c->operand[2]));
        super_remove_after(item, 4);
        return 4;
    }
    return 0;
}

static int super_combine_three(super_line *lines, super_line **item) {
    super_line *a = item[0];
    super_line *b = item[1];
    super_line *c = item[2];
    if (!super_same_block(item, 3)) return 0;

    if (super_is(a, "icopy", 2) && super_is(b, "unlink", 1) &&
        super_equal(a, 0, b, 0) && super_is(c, "unlink", 1)) {
        super_replace(a, mprintf("   isetunlinkn %s,%s,%s\n",
                                 a->operand[0], a->operand[1], c->operand[0]));
        super_retarget_trace(lines, (size_t)(a - lines + 1),
                             (size_t)(b - lines), a->operand[0], a->operand[1]);
        super_remove_after(item, 3);
        return 3;
    }

    if (super_is(a, "setattrs", 2) && super_integer(a->operand[1]) &&
        super_is(b, "iadd", 3) && super_integer(b->operand[2]) &&
        super_is(c, "linkattr1", 3) && super_equal(a, 0, c, 1) &&
        super_equal(b, 0, c, 2) &&
        !super_trace_references(lines, (size_t)(b - lines + 1),
                                (size_t)(c - lines - 1), b->operand[0])) {
        super_replace(a, mprintf("   setlinkattr1 %s,%s,%s,%s,%s\n",
                                 c->operand[0], a->operand[0], a->operand[1],
                                 b->operand[1], b->operand[2]));
        super_remove_after(item, 3);
        return 3;
    }

    if (super_is(a, "linkattr1", 3) && super_integer(a->operand[2]) &&
        super_is(b, "icopy", 2) && super_equal(a, 0, b, 0) &&
        super_is(c, "unlink", 1) && super_equal(a, 0, c, 0) &&
        !super_trace_references(lines, (size_t)(a - lines + 1),
                                (size_t)(b - lines - 1), a->operand[0])) {
        super_replace(a, mprintf("   isetattr1 %s,%s,%s\n",
                                 a->operand[1], a->operand[2], b->operand[1]));
        super_retarget_trace(lines, (size_t)(b - lines + 1),
                             (size_t)(c - lines), a->operand[0], b->operand[1]);
        super_remove_after(item, 3);
        return 3;
    }

    if (super_is(a, "setattrs", 2) && super_integer(a->operand[1]) &&
        super_is(b, "linkattr1", 3) && super_register(b->operand[2]) &&
        super_equal(a, 0, b, 1) &&
        super_is(c, "load", 2) && super_integer(c->operand[1])) {
        super_replace(a, mprintf("   setlinkiload %s,%s,%s,%s,%s,%s\n",
                                 b->operand[0], a->operand[0], a->operand[1],
                                 b->operand[2], c->operand[0], c->operand[1]));
        super_remove_after(item, 3);
        return 3;
    }
    return 0;
}

static int super_combine_two(super_line *lines, super_line **item) {
    super_line *a = item[0];
    super_line *b = item[1];
    if (!super_same_block(item, 2)) return 0;

    if (super_is(a, "linkattr1", 3) && super_integer(a->operand[2]) &&
        super_is(b, "setlinkattr1", 5) && super_equal(a, 0, b, 1) &&
        super_integer(b->operand[2]) && super_register(b->operand[3]) &&
        super_integer(b->operand[4])) {
        super_replace(a, mprintf("   linksetattrslinkadd %s,%s,%s,%s,%s,%s,%s\n",
                                 a->operand[0], a->operand[1], a->operand[2],
                                 b->operand[2], b->operand[0],
                                 b->operand[3], b->operand[4]));
        b->removed = 1;
        return 2;
    }

    if (super_is(a, "setlinkattr1", 4) &&
        super_integer(a->operand[2]) && super_register(a->operand[3]) &&
        super_is(b, "load", 2) && super_integer(b->operand[1])) {
        super_replace(a, mprintf("   setlinkiload %s,%s,%s,%s,%s,%s\n",
                                 a->operand[0], a->operand[1], a->operand[2],
                                 a->operand[3], b->operand[0], b->operand[1]));
        b->removed = 1;
        return 2;
    }

    if (super_is(a, "icopy", 2) && super_is(b, "unlink", 1)) {
        if (super_equal(a, 0, b, 0)) {
            super_replace(a, mprintf("   isetunlink %s,%s\n",
                                     a->operand[0], a->operand[1]));
            super_retarget_trace(lines, (size_t)(a - lines + 1),
                                 (size_t)(b - lines), a->operand[0], a->operand[1]);
            b->removed = 1;
            return 2;
        }
        if (super_equal(a, 1, b, 0)) {
            super_replace(a, mprintf("   igetunlink %s,%s\n",
                                     a->operand[0], a->operand[1]));
            b->removed = 1;
            return 2;
        }
    }

    if (super_is(a, "unlink", 1) && super_is(b, "unlink", 1)) {
        super_replace(a, mprintf("   unlinkn %s,%s\n",
                                 a->operand[0], b->operand[0]));
        b->removed = 1;
        return 2;
    }

    if (super_is(a, "unlink", 1) && super_is(b, "br", 1)) {
        super_replace(a, mprintf("   unlinkbr %s,%s\n",
                                 a->operand[0], b->operand[0]));
        b->removed = 1;
        return 2;
    }

    if (super_is(a, "minattrs", 3) && super_register(a->operand[1]) &&
        super_integer(a->operand[2]) &&
        super_is(b, "linkattr1", 3) && super_equal(a, 0, b, 1) &&
        super_equal(a, 1, b, 2)) {
        super_replace(a, mprintf("   minlinkattr1 %s,%s,%s,%s\n",
                                 b->operand[0], a->operand[0],
                                 a->operand[1], a->operand[2]));
        b->removed = 1;
        return 2;
    }

    if (super_is(a, "minattrs", 2) && super_integer(a->operand[1]) &&
        super_is(b, "linkattr1", 3) && super_equal(a, 0, b, 1) &&
        strcmp(a->operand[1], b->operand[2]) == 0) {
        super_replace(a, mprintf("   minlinkattr1 %s,%s,%s\n",
                                 b->operand[0], a->operand[0], a->operand[1]));
        b->removed = 1;
        return 2;
    }

    if (super_is(a, "setattrs", 2) && super_integer(a->operand[1]) &&
        super_is(b, "linkattr1", 3) && super_register(b->operand[2]) &&
        super_equal(a, 0, b, 1)) {
        super_replace(a, mprintf("   setlinkattr1 %s,%s,%s,%s\n",
                                 b->operand[0], a->operand[0],
                                 a->operand[1], b->operand[2]));
        b->removed = 1;
        return 2;
    }

    if (super_is(a, "fdiv", 3) && super_equal(a, 0, a, 2) &&
        super_is(b, "fsub", 3) && super_equal(a, 0, b, 1) &&
        !super_register(b->operand[2])) {
        super_replace(a, mprintf("   fdivsub %s,%s,%s,%s\n",
                                 b->operand[0], a->operand[1],
                                 a->operand[0], b->operand[2]));
        b->removed = 1;
        return 2;
    }

    if (super_is(a, "fmult", 3) && super_equal(a, 0, a, 1) &&
        !super_register(a->operand[2]) && super_is(b, "icopy", 2)) {
        super_replace(a, mprintf("   fmulticopy %s,%s,%s,%s\n",
                                 a->operand[0], a->operand[2],
                                 b->operand[0], b->operand[1]));
        b->removed = 1;
        return 2;
    }

    return 0;
}

char *rxcp_combine_superinstructions(const char *text) {
    super_line *lines;
    super_line **instruction;
    size_t line_count;
    size_t instruction_count;
    size_t i;
    size_t position;
    size_t output_length;
    char *result;
    char *write;

    if (!text) return 0;
    lines = super_split_lines(text, &line_count);
    if (!lines) return super_copy(text, strlen(text));
    instruction = malloc(line_count * sizeof(super_line*));
    if (!instruction) {
        super_free_lines(lines, line_count);
        return super_copy(text, strlen(text));
    }
    instruction_count = 0;
    for (i = 0; i < line_count; i++) {
        if (lines[i].instruction) instruction[instruction_count++] = &lines[i];
    }

    position = 0;
    while (position < instruction_count) {
        int consumed = 0;
        if (position + 5 <= instruction_count)
            consumed = super_combine_five(lines, instruction + position);
        if (!consumed && position + 4 <= instruction_count)
            consumed = super_combine_four(lines, instruction + position);
        if (!consumed && position + 3 <= instruction_count)
            consumed = super_combine_three(lines, instruction + position);
        if (!consumed && position + 2 <= instruction_count)
            consumed = super_combine_two(lines, instruction + position);
        position += consumed ? (size_t)consumed : 1;
    }

    output_length = 0;
    for (i = 0; i < line_count; i++) {
        if (!lines[i].removed) output_length += strlen(lines[i].text);
    }
    result = malloc(output_length + 1);
    if (!result) {
        free(instruction);
        super_free_lines(lines, line_count);
        return super_copy(text, strlen(text));
    }
    write = result;
    for (i = 0; i < line_count; i++) {
        size_t length;
        if (lines[i].removed) continue;
        length = strlen(lines[i].text);
        memcpy(write, lines[i].text, length);
        write += length;
    }
    *write = 0;
    free(instruction);
    super_free_lines(lines, line_count);
    return result;
}
