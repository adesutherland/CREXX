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

/* CREXX
 * RUNTIME Variable Support
 */

#ifndef CREXX_RXVMVARS_H
#define CREXX_RXVMVARS_H

#ifndef NUTF8
#include "utf.h"
#endif
#include "../binutils/include/rxflags.h"
#include "rxvmref.h"

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <string.h>

/* Forward declarations */
static void extract_double_decimal(numeric_context* num_context, value *coefficient, value *exponent, double value);
static void extract_integer_decimal(numeric_context* num_context, value *coefficient, value *exponent, rxinteger value);
static void RexxDecimalFormat(numeric_context* num_context, value *coefficient_value, value *exponent_value, value *formatted_output_value);
RX_MOSTLYINLINE void clear_value_contents(value* v);
RX_MOSTLYINLINE void reset_value_storage_for_reuse(value* v);
RX_MOSTLYINLINE void destroy_value_storage(value* v);
RX_MOSTLYINLINE void clear_value(value* v);
RX_MOSTLYINLINE void release_value_reference_lifetime(value* v);
RX_INLINE void move_value(value *dest, value *source);
RX_MOSTLYINLINE void maybe_trim_attribute_storage(value *v);

RX_INLINE void clear_vm_private_flags(value *v) {
    v->status.all_type_flags &= ~RXFLAG_VM_PRIVATE_MASK;
}

RX_INLINE void set_vm_private_flags(value *v, uint32_t flags) {
    v->status.all_type_flags = (v->status.all_type_flags & ~RXFLAG_VM_PRIVATE_MASK) |
                               (flags & RXFLAG_VM_PRIVATE_MASK);
}

RX_INLINE void copy_vm_private_flags(value *dest, const value *source) {
    set_vm_private_flags(dest, source->status.all_type_flags);
}

RX_INLINE int value_is_uninitialized_object(const value *v) {
    return v &&
           (v->status.all_type_flags & RXFLAG_VM_OBJECT_UNINITIALIZED) != 0;
}

RX_INLINE void mark_value_uninitialized_object(value *v) {
    if (!v) return;
    set_vm_private_flags(v, v->status.all_type_flags | RXFLAG_VM_OBJECT_UNINITIALIZED);
}

RX_INLINE void clear_value_uninitialized_object(value *v) {
    if (!v) return;
    set_vm_private_flags(v, v->status.all_type_flags & ~RXFLAG_VM_OBJECT_UNINITIALIZED);
}

#ifndef NUTF8
RX_INLINE int has_utf8_valid_count(const value *v) {
    return (v->status.all_type_flags & (RXFLAG_VM_UTF8_VALID | RXFLAG_VM_UTF8_COUNT_VALID)) ==
           (RXFLAG_VM_UTF8_VALID | RXFLAG_VM_UTF8_COUNT_VALID);
}

RX_INLINE int has_utf8_valid_count_or_empty(const value *v) {
    return v->string_length == 0 || has_utf8_valid_count(v);
}

RX_INLINE void mark_utf8_valid_count(value *v) {
    set_vm_private_flags(v, RXFLAG_VM_UTF8_VALID | RXFLAG_VM_UTF8_COUNT_VALID);
}

RX_INLINE void mark_ascii_string_valid_count(value *v) {
    v->string_chars = v->string_length;
    v->string_char_pos = 0;
    mark_utf8_valid_count(v);
}

RX_INLINE void refresh_utf8_flags(value *v) {
    size_t chars = 0;
    if (!utf8nvalid_count(v->string_value, v->string_length, &chars)) {
        v->string_chars = chars;
        mark_utf8_valid_count(v);
    } else {
        v->string_chars = utf8nlen(v->string_value, v->string_length);
        clear_vm_private_flags(v);
    }
}

RX_INLINE void set_utf8_known_concat_flags(value *dest, int left_known, int right_known) {
    if (left_known && right_known) mark_utf8_valid_count(dest);
    else clear_vm_private_flags(dest);
}

RX_INLINE int validate_utf8_bytes(const void *bytes, size_t length, size_t *chars) {
    size_t local_chars = 0;
    const void *source = bytes ? bytes : "";

    if (!bytes && length != 0) return -1;
    if (utf8nvalid_count(source, length, &local_chars)) return -1;
    if (chars) *chars = local_chars;
    return 0;
}

RX_INLINE int is_valid_unicode_scalar(rxinteger codepoint) {
    return codepoint >= 0 && codepoint <= 0x10ffff &&
           !(codepoint >= 0xd800 && codepoint <= 0xdfff);
}
#endif

/* Clears the binary payload and runs native cleanup if the payload owns native resources. */
RX_INLINE void clear_binary_payload(value *v) {
    if (!v) return;
    if (v->native_payload_ops && v->native_payload_ops->finalize) {
        v->native_payload_ops->finalize(v);
    }
    if (v->binary_value) free(v->binary_value);
    v->binary_value = 0;
    v->binary_length = 0;
    v->binary_pos = 0;
    v->binary_buffer_length = 0;
    v->native_payload_ops = 0;
    v->native_payload_flags = 0;
}

/* Zeros a register value */
RX_INLINE void value_zero(value *v) {
    size_t i;

    if (v->reference_payload) rxvm_reference_value_release_payload(v);

    if (v->num_attributes) {
        for (i = 0; i < v->num_attributes; i++) {
            value *attribute = v->unlinked_attributes ? v->unlinked_attributes[i] :
                               (v->attributes ? v->attributes[i] : 0);
            if (attribute) reset_value_storage_for_reuse(attribute);
            if (v->attributes && v->unlinked_attributes) {
                v->attributes[i] = v->unlinked_attributes[i];
            }
        }
    }

    if (v->native_payload_ops) {
        clear_binary_payload(v);
    }
    v->status.all_type_flags = 0;
    v->int_value = 0;
    v->float_value = 0;
    v->object_type_name = 0;
    v->object_type_name_length = 0;
    v->string_length = 0; // Lazy Free String - just zero the used length
    v->string_pos = 0;
#ifndef NUTF8
    v->string_chars = 0;
    v->string_char_pos = 0;
#endif
    v->num_attributes = 0;

    /* Lazy Free Decimal - just zero the used length */
    v->decimal_value_length = 0;

    /* Lazy Free binary - just zero the used length */
    v->binary_length = 0;
    v->binary_pos = 0;
}

/* Setup a new value structure */
RX_INLINE void value_init(value *v) {
    v->string_value = v->small_string_buffer;
    v->string_buffer_length = sizeof(v->small_string_buffer);
    v->attributes = 0;
    v->unlinked_attributes = 0;
    v->attribute_buffers = 0;
    v->num_attribute_buffers = 0;
    v->max_num_attributes = 0;
    v->num_attributes = 0;
    v->binary_value = 0;
    v->binary_pos = 0;
    v->binary_buffer_length = 0;
    v->native_payload_ops = 0;
    v->native_payload_flags = 0;
    v->reference_identity = 0;
    v->reference_payload = 0;
    v->decimal_value = 0;
    v->decimal_value_length = 0;
    v->decimal_buffer_length = 0;
    value_zero(v);
}

/* Value Factory mallocs and inits */
RX_INLINE value* value_f() {
    value* this;
    this = malloc(sizeof(value));
    value_init(this);
    return this;
}

/*
 * Returns required buffer size - the smallest power of two that's greater or
 * equal to a given value. A minimum size is enforced.
 */
RX_INLINE size_t power_of_two_size(size_t value) {
    if (value == 0) return 0; // Handle zero input
    size_t new_size = value > 8 ? value : 8; // Enforce a minimum size of 8
    new_size--;
    new_size |= new_size >> 1;
    new_size |= new_size >> 2;
    new_size |= new_size >> 4;
    new_size |= new_size >> 8;
    new_size |= new_size >> 16;
#if __SIZEOF_SIZE_T__ == 8
    new_size |= new_size >> 32;
#endif
    new_size++;
    return new_size;
}

/* Sets up the required number of attributes */
RX_INLINE void set_num_attributes(value* v, size_t num) {
    size_t i;
    value *a;

    if (num <= v->num_attributes) {
        /* Reducing invalidates removed child storage but keeps it reusable. */
        for (i = num; i < v->num_attributes; i++) {
            v->attributes[i] = v->unlinked_attributes[i];
            reset_value_storage_for_reuse(v->attributes[i]);
        }
        v->num_attributes = num;
        maybe_trim_attribute_storage(v);
        return;
    }

    if (num <= v->max_num_attributes) {
        /* Just need to reset the recycled attributes */
        for (i = v->num_attributes; i < num; i++) {
            v->attributes[i] = v->unlinked_attributes[i]; /* Ensure Attribute is unlinked */
            clear_value_contents(v->attributes[i]);
        }
        v->num_attributes = num;
        return;
    }

    /* Increasing the number of attributes, we need to allocate more space */

    /* We first need to recycle any unused attributes */
    for (i = v->num_attributes; i < v->max_num_attributes; i++) {
        v->attributes[i] = v->unlinked_attributes[i]; /* Ensure Attribute is unlinked */
        clear_value_contents(v->attributes[i]);
    }

    /* Calculate the new maximum number of attributes using bit-twiddling */
    size_t new_max = power_of_two_size(num);

    /* Now we need to make the pointer arrays big enough */
    if (v->attributes) v->attributes = realloc(v->attributes, sizeof(value*) * new_max);
    else v->attributes = malloc(sizeof(value*) * new_max);

    if (v->unlinked_attributes) v->unlinked_attributes = realloc(v->unlinked_attributes, sizeof(value*) * new_max);
    else v->unlinked_attributes = malloc(sizeof(value*) * new_max);

    /* We create a buffer for the new attributes separate to the existing buffers. */
    size_t old_capacity = power_of_two_size(v->num_attribute_buffers);
    v->num_attribute_buffers++;
    size_t new_capacity = power_of_two_size(v->num_attribute_buffers);

    // Reallocate only when the required capacity has changed
    if (new_capacity > old_capacity) {
        if (v->attribute_buffers) {
            v->attribute_buffers = realloc(v->attribute_buffers, sizeof(value*) * new_capacity);
        } else {
            v->attribute_buffers = malloc(sizeof(value*) * new_capacity);
        }
    }

    /* Create a new buffer */
    v->attribute_buffers[v->num_attribute_buffers - 1] =
            malloc(sizeof(value) * (new_max - v->max_num_attributes));

    /* Initiate the new attributes */
    a = v->attribute_buffers[v->num_attribute_buffers - 1];
    for (i = v->max_num_attributes; i < new_max; i++, a++) {
        value_init(a);
        v->attributes[i] = v->unlinked_attributes[i] = a;
    }

    /* Set the new number of attributes */
    v->num_attributes = num;
    v->max_num_attributes = new_max;
}

/*
 * Returns required buffer size - the smallest power of two that's greater or
 * equal to a given value
 */
RX_INLINE size_t buffer_size(size_t value) {
    size_t i;
    if (value <= SMALLEST_STRING_BUFFER_LENGTH)
        return SMALLEST_STRING_BUFFER_LENGTH;

    return power_of_two_size(value);
}

RX_INLINE int reserve_binary_buffer(value *v, size_t length) {
    if (v->native_payload_ops) clear_binary_payload(v);
    if (length > v->binary_buffer_length) {
        size_t new_size = buffer_size(length);
        void *new_buffer;

        if (v->binary_value) new_buffer = realloc(v->binary_value, new_size);
        else new_buffer = malloc(new_size);

        if (!new_buffer) return -1;

        v->binary_value = new_buffer;
        v->binary_buffer_length = new_size;
    }
    return 0;
}

RX_INLINE int prep_binary_buffer(value *v, size_t length) {
    if (reserve_binary_buffer(v, length) != 0) return -1;
    v->binary_length = length;
    return 0;
}

RX_INLINE int set_binary(value *v, const void *data, size_t length) {
    if (v->reference_payload) rxvm_reference_value_release_payload(v);
    if (prep_binary_buffer(v, length) != 0) return -1;
    if (length && data) memcpy(v->binary_value, data, length);
    else if (length) memset(v->binary_value, 0, length);
    v->binary_pos = 0;
    clear_vm_private_flags(v);
    return 0;
}

RX_INLINE int set_buffer_binary(value *v, char *buffer, size_t length, size_t buffer_length) {
    if (v->reference_payload) rxvm_reference_value_release_payload(v);
    if (v->native_payload_ops) clear_binary_payload(v);
    if (v->binary_value) free(v->binary_value);
    v->binary_value = buffer;
    v->binary_length = length;
    v->binary_pos = 0;
    v->binary_buffer_length = buffer_length;
    clear_vm_private_flags(v);
    return 0;
}

RX_INLINE int append_binary(value *v, const void *data, size_t length) {
    size_t start = v->binary_length;
    if (prep_binary_buffer(v, start + length) != 0) return -1;
    if (length && data) memcpy(v->binary_value + start, data, length);
    else if (length) memset(v->binary_value + start, 0, length);
    if (v->binary_pos > v->binary_length) v->binary_pos = v->binary_length;
    clear_vm_private_flags(v);
    return 0;
}

RX_INLINE int append_binary_value(value *dest, value *source) {
    size_t source_length = source->binary_length;
    if (dest == source) {
        if (prep_binary_buffer(dest, source_length * 2) != 0) return -1;
        if (source_length) memcpy(dest->binary_value + source_length, dest->binary_value, source_length);
        if (dest->binary_pos > dest->binary_length) dest->binary_pos = dest->binary_length;
        clear_vm_private_flags(dest);
        return 0;
    }
    return append_binary(dest, source->binary_value, source_length);
}

RX_INLINE int concat_binary(value *dest, value *left, value *right) {
    size_t left_length = left->binary_length;
    size_t right_length = right->binary_length;
    size_t total_length = left_length + right_length;

    if (total_length == 0) return set_binary(dest, 0, 0);

    if (dest == left || dest == right) {
        size_t buffer_length = buffer_size(total_length);
        char *buffer = malloc(buffer_length);
        if (!buffer) return -1;
        if (left_length) memcpy(buffer, left->binary_value, left_length);
        if (right_length) memcpy(buffer + left_length, right->binary_value, right_length);
        set_buffer_binary(dest, buffer, total_length, buffer_length);
        return 0;
    }

    if (prep_binary_buffer(dest, total_length) != 0) return -1;
    if (left_length) memcpy(dest->binary_value, left->binary_value, left_length);
    if (right_length) memcpy(dest->binary_value + left_length, right->binary_value, right_length);
    dest->binary_pos = 0;
    clear_vm_private_flags(dest);
    return 0;
}

RX_INLINE int slice_binary(value *dest, value *source, size_t offset, size_t length) {
    size_t actual_length;

    if (offset >= source->binary_length) actual_length = 0;
    else {
        actual_length = source->binary_length - offset;
        if (length < actual_length) actual_length = length;
    }

    if (dest == source) {
        if (actual_length) memmove(dest->binary_value, source->binary_value + offset, actual_length);
        dest->binary_length = actual_length;
        dest->binary_pos = 0;
        clear_vm_private_flags(dest);
        return 0;
    }

    if (prep_binary_buffer(dest, actual_length) != 0) return -1;
    if (actual_length) memcpy(dest->binary_value, source->binary_value + offset, actual_length);
    dest->binary_pos = 0;
    return 0;
}

RX_INLINE void prep_string_buffer(value *v, size_t length) {
    v->string_length = length;
    if (v->string_length > v->string_buffer_length) {
        if (v->string_value != v->small_string_buffer) free(v->string_value);
        v->string_buffer_length = buffer_size(v->string_length);
        v->string_value = malloc(v->string_buffer_length);
    }
}

RX_INLINE void extend_string_buffer(value *v, size_t length) {
    v->string_length = length;
    if (v->string_length > v->string_buffer_length) {
        v->string_buffer_length = buffer_size(v->string_length);
        if (v->string_value == v->small_string_buffer) {
            v->string_value = malloc(v->string_buffer_length);
            memcpy(v->string_value, v->small_string_buffer, sizeof(v->small_string_buffer));
        }
        else {
            v->string_value = realloc(v->string_value, v->string_buffer_length);
        }
    }
}

RX_INLINE void null_terminate_string_buffer(value *v) {

    if (v->string_length + 1 > v->string_buffer_length) {
        /* Make room for the null */
        extend_string_buffer(v, v->string_length + 1);

        /* extend_string_buffer() increments string_length so put it back */
        v->string_length--;
    }

    /* Add the null */
    v->string_value[v->string_length] = 0;
}

/* Releases reference identities/payload retains for storage whose lifetime ended.
 * Reusable buffers and ordinary value contents are intentionally left intact.
 */
RX_MOSTLYINLINE void release_value_reference_lifetime(value* v) {
    size_t i;

    if (!v) return;
    if (v->reference_identity) rxvm_reference_identity_release(v);
    if (v->reference_payload) rxvm_reference_value_release_payload(v);

    if (v->unlinked_attributes) {
        for (i = 0; i < v->max_num_attributes; i++) {
            release_value_reference_lifetime(v->unlinked_attributes[i]);
        }
    }
}

/* Clears a value's contents while preserving its own storage reference identity. */
RX_MOSTLYINLINE void clear_value_contents(value* v) {
    int i;

    /* Clear attribute values */
    if (v->unlinked_attributes) {
        for (i = 0; i < v->max_num_attributes; i++) {
            if (v->unlinked_attributes[i]) {
                destroy_value_storage(v->unlinked_attributes[i]);
            }
        }
        free(v->unlinked_attributes);
        v->unlinked_attributes = 0;
    }

    /* Free attribute buffer */
    if (v->attribute_buffers) {
        for (i = 0; i < v->num_attribute_buffers; i++) {
            if (v->attribute_buffers[i]) free(v->attribute_buffers[i]);
        }
        free(v->attribute_buffers);
        v->attribute_buffers = 0;
        v->num_attribute_buffers = 0;
    }

    /* Free pointer arrays */
    if (v->attributes) {
        free(v->attributes);
        v->attributes = 0;
    }
    v->max_num_attributes = 0;
    v->num_attributes = 0;

    /* Free strings */
    if (v->string_value != v->small_string_buffer) {
        free(v->string_value);
        v->string_value = v->small_string_buffer;
        v->string_buffer_length = sizeof(v->small_string_buffer);
    }
    v->string_length = 0;
    v->string_pos = 0;
#ifndef NUTF8
    v->string_chars = 0;
    v->string_char_pos = 0;
#endif

    /* Free decimal */
    if (v->decimal_value) free(v->decimal_value);
    v->decimal_value = 0;
    v->decimal_value_length = 0;
    v->decimal_buffer_length = 0;

    /* Free binary */
    clear_binary_payload(v);

    value_zero(v);
}

/* Resets a storage location for later reuse without freeing reusable buffers. */
RX_MOSTLYINLINE void reset_value_storage_for_reuse(value* v) {
    if (!v) return;
    if (v->reference_identity) rxvm_reference_identity_release(v);
    value_zero(v);
}

/* Destroys a storage location, invalidating references to that location. */
RX_MOSTLYINLINE void destroy_value_storage(value* v) {
    if (v && v->reference_identity) rxvm_reference_identity_release(v);
    clear_value_contents(v);
}

/* Backward-compatible storage teardown helper. */
RX_MOSTLYINLINE void clear_value(value* v) {
    destroy_value_storage(v);
}

RX_INLINE void reverse_attribute_pointers(value **items, size_t start, size_t count) {
    size_t left;
    size_t right;

    if (!items || count < 2) return;

    left = start;
    right = start + count - 1;
    while (left < right) {
        value *tmp = items[left];
        items[left] = items[right];
        items[right] = tmp;
        left++;
        right--;
    }
}

RX_INLINE void rotate_attribute_pointer_blocks(value **items, size_t start, size_t left_count, size_t right_count) {
    if (!items || left_count == 0 || right_count == 0) return;

    /* Swap adjacent blocks [left][right] into [right][left] without a temp buffer. */
    reverse_attribute_pointers(items, start, left_count);
    reverse_attribute_pointers(items, start + left_count, right_count);
    reverse_attribute_pointers(items, start, left_count + right_count);
}

RX_INLINE void insert_attributes(value *v, size_t index, size_t count) {
    size_t old_num;
    size_t i;

    if (!v || count == 0) return;

    old_num = v->num_attributes;
    set_num_attributes(v, old_num + count);

    if (index < old_num) {
        rotate_attribute_pointer_blocks(v->attributes, index, old_num - index, count);
        rotate_attribute_pointer_blocks(v->unlinked_attributes, index, old_num - index, count);
    }

    for (i = index; i < index + count; i++) {
        v->attributes[i] = v->unlinked_attributes[i];
        reset_value_storage_for_reuse(v->attributes[i]);
    }
}

RX_INLINE void delete_attributes(value *v, size_t index, size_t count) {
    size_t old_num;
    size_t new_num;
    size_t tail_count;
    size_t i;

    if (!v || count == 0) return;

    old_num = v->num_attributes;
    if (index > old_num) return;
    tail_count = old_num - index;
    if (count > tail_count) return;
    tail_count -= count;

    if (tail_count > 0) {
        rotate_attribute_pointer_blocks(v->attributes, index, count, tail_count);
        rotate_attribute_pointer_blocks(v->unlinked_attributes, index, count, tail_count);
    }

    new_num = old_num - count;
    for (i = new_num; i < old_num; i++) {
        v->attributes[i] = v->unlinked_attributes[i];
        reset_value_storage_for_reuse(v->attributes[i]);
    }
    v->num_attributes = new_num;
    maybe_trim_attribute_storage(v);
}

/* Int Flag */
/*
RX_INLINE void set_type_int(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_int = 1;
}

RX_INLINE void add_type_int(value *v) {
    v->status.type_int = 1;
}

RX_INLINE unsigned int get_type_int(value *v) {
    return v->status.type_int;
}

// Float Flag
RX_INLINE void set_type_float(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_float = 1;
}

RX_INLINE void add_type_float(value *v) {
    v->status.type_float = 1;
}

RX_INLINE unsigned int get_type_float(value *v) {
    return v->status.type_float;
}

// Decimal Flag
RX_INLINE void set_type_decimal(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_decimal = 1;
}

RX_INLINE void add_type_decimal(value *v) {
    v->status.type_decimal = 1;
}

RX_INLINE unsigned int get_type_decimal(value *v) {
    return v->status.type_decimal;
}

// String Flag
RX_INLINE void set_type_string(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_string = 1;
}

RX_INLINE void add_type_string(value *v) {
    v->status.type_string = 1;
}

RX_INLINE unsigned int get_type_string(value *v) {
    return v->status.type_string;
}

// Object Flag
RX_INLINE void set_type_object(value *v) {
    v->status.all_type_flags = 0;
    v->status.type_object = 1;
}

RX_INLINE void add_type_object(value *v) {
    v->status.type_object = 1;
}

RX_INLINE unsigned int get_type_object(value *v) {
    return v->status.type_object;
}

// Unset Flag
RX_INLINE void unset_type(value *v) {
    v->status.all_type_flags = 0;
}
*/

RX_INLINE void set_int(value *v, rxinteger value) {
    if (v->reference_payload) rxvm_reference_value_release_payload(v);
    if (v->native_payload_ops) clear_binary_payload(v);
    v->int_value = value;
}
RX_INLINE void set_float(value *v, double value) {
    if (v->reference_payload) rxvm_reference_value_release_payload(v);
    if (v->native_payload_ops) clear_binary_payload(v);
    v->float_value = value;
}

RX_INLINE void set_string(value *v, char *value, size_t length) {
    if (v->reference_payload) rxvm_reference_value_release_payload(v);
    if (v->native_payload_ops) clear_binary_payload(v);
    prep_string_buffer(v,length);
    memcpy(v->string_value, value, v->string_length);
    v->string_pos = 0;
#ifndef NUTF8
    v->string_char_pos = 0;
    refresh_utf8_flags(v);
#else
    clear_vm_private_flags(v);
#endif
}

/* set value string from null string value */
RX_INLINE void set_null_string(value *v, const char *from) {
    if (v->reference_payload) rxvm_reference_value_release_payload(v);
    if (v->native_payload_ops) clear_binary_payload(v);
    if (v->string_value == from) return;
    prep_string_buffer(v, strlen(from));
    memcpy(v->string_value, from, v->string_length);
    v->string_pos = 0;
#ifndef NUTF8
    v->string_char_pos = 0;
    refresh_utf8_flags(v);
#else
    clear_vm_private_flags(v);
#endif
}

RX_INLINE int set_string_validated(value *v, const char *from, size_t length) {
    if (!from && length != 0) return -1;
#ifndef NUTF8
    size_t chars = 0;
    if (validate_utf8_bytes(from, length, &chars) != 0) return -1;
#endif
    set_string(v, (char *)(from ? from : ""), length);
#ifndef NUTF8
    v->string_chars = chars;
    v->string_char_pos = 0;
    mark_utf8_valid_count(v);
#endif
    return 0;
}

RX_INLINE int set_null_string_validated(value *v, const char *from) {
    const char *text = from ? from : "";
    return set_string_validated(v, text, strlen(text));
}

RX_INLINE void set_const_string(value *v, string_constant *from) {
    if (v->reference_payload) rxvm_reference_value_release_payload(v);
    if (v->native_payload_ops) clear_binary_payload(v);
    prep_string_buffer(v,from->string_len);
    memcpy(v->string_value, from->string, v->string_length);
    v->string_pos = 0;
#ifndef NUTF8
    v->string_char_pos = 0;
    v->string_chars = from->string_chars;
    mark_utf8_valid_count(v);
#else
    clear_vm_private_flags(v);
#endif
}

RX_INLINE void set_value_string(value *v, value *from) {
    if (v == from) return;
    if (v->reference_payload) rxvm_reference_value_release_payload(v);
    if (v->native_payload_ops) clear_binary_payload(v);
    prep_string_buffer(v, from->string_length);
    memcpy(v->string_value, from->string_value, v->string_length);
    v->string_pos = 0;
#ifndef NUTF8
    v->string_char_pos = 0;
    v->string_chars = from->string_chars;
    copy_vm_private_flags(v, from);
#else
    clear_vm_private_flags(v);
#endif
}

RX_INLINE void set_buffer_string(
        value *v,
        char *buffer,
        size_t length,
        size_t buffer_length
#ifndef NUTF8
        , size_t string_chars
#endif
) {
    if (v->reference_payload) rxvm_reference_value_release_payload(v);
    if (v->native_payload_ops) clear_binary_payload(v);
    if (v->string_value != v->small_string_buffer) free(v->string_value);
    v->string_value = buffer;
    v->string_length = length;
    v->string_buffer_length = buffer_length;
    v->string_pos = 0;
#ifndef NUTF8
    v->string_char_pos = 0;
    v->string_chars = string_chars;
#endif
    clear_vm_private_flags(v);
}

RX_INLINE int set_native_payload(value *v,
                                 const void *payload,
                                 size_t length,
                                 const rxvm_native_payload_ops *ops,
                                 unsigned int flags) {
    if (!v) return -1;

    if (v->reference_payload) rxvm_reference_value_release_payload(v);
    clear_binary_payload(v);
    if (length) {
        if (set_binary(v, payload, length) != 0) return -1;
    }
    v->native_payload_ops = ops;
    v->native_payload_flags = flags;
    return 0;
}

RX_INLINE void* get_native_payload(value *v,
                                   size_t *out_length,
                                   const rxvm_native_payload_ops **out_ops,
                                   unsigned int *out_flags) {
    if (out_length) *out_length = v ? v->binary_length : 0;
    if (out_ops) *out_ops = v ? v->native_payload_ops : 0;
    if (out_flags) *out_flags = v ? v->native_payload_flags : 0;
    return v ? v->binary_value : 0;
}

/* Copy a value */
RX_MOSTLYINLINE void copy_value(value *dest, value *source) {
    size_t i;

    if (dest == source) return;

    if (dest->reference_payload || source->reference_payload) {
        rxvm_reference_value_copy_payload(dest, source);
    }

    if (dest->native_payload_ops || source->native_payload_ops) {
        clear_binary_payload(dest);
    }

    dest->status.all_type_flags = source->status.all_type_flags;
    dest->int_value = source->int_value;
    dest->float_value = source->float_value;
    dest->object_type_name = source->object_type_name;
    dest->object_type_name_length = source->object_type_name_length;

    /* Copy Decimal Value */
    if (source->decimal_value_length) {
        dest->decimal_value_length = source->decimal_value_length;
        dest->decimal_buffer_length = dest->decimal_value_length;
        if (dest->decimal_value) dest->decimal_value = realloc(dest->decimal_value, dest->decimal_value_length);
        else dest->decimal_value = malloc(dest->decimal_value_length);
        memcpy(dest->decimal_value, source->decimal_value, dest->decimal_value_length);
    }
    else {
        dest->decimal_value_length = 0;
    }

    /* Copy Strings */
    if (source->string_length) {
        /* Copy String Data */
        prep_string_buffer(dest, source->string_length);
        dest->string_pos = source->string_pos;
#ifndef NUTF8
        dest->string_chars = source->string_chars;
        dest->string_char_pos = source->string_char_pos;
#endif
        memcpy(dest->string_value, source->string_value, dest->string_length);
    }
    else {
        dest->string_length = 0;
        dest->string_pos = 0;
#ifndef NUTF8
        dest->string_chars = 0;
        dest->string_char_pos = 0;;
#endif
    }
    copy_vm_private_flags(dest, source);

    /* Copy Binary */
    if (source->native_payload_ops && source->native_payload_ops->copy) {
        source->native_payload_ops->copy(dest, source);
    }
    else if (source->binary_length) {
        if (prep_binary_buffer(dest, source->binary_length) != 0) abort();
        memcpy(dest->binary_value, source->binary_value, dest->binary_length);
        dest->binary_pos = source->binary_pos;
        dest->native_payload_ops = source->native_payload_ops;
        dest->native_payload_flags = source->native_payload_flags;
    }
    else {
        dest->binary_length = 0;
        dest->binary_pos = 0;
        dest->native_payload_ops = 0;
        dest->native_payload_flags = 0;
    }

    /* Copy Attributes */
    set_num_attributes(dest,source->num_attributes);
    for (i = 0; i < dest->num_attributes; i++)
        copy_value(dest->attributes[i], source->attributes[i]);
}

/* Move a value */
RX_INLINE void move_value(value *dest, value *source) {
    if (dest == source) return;

    /* Clear out destination - including string / attributes */
    destroy_value_storage(dest);
    value_init(dest);

    /* Copy basic values */
    dest->status.all_type_flags = source->status.all_type_flags;
    dest->int_value = source->int_value;
    dest->float_value = source->float_value;
    dest->object_type_name = source->object_type_name;
    dest->object_type_name_length = source->object_type_name_length;
    source->object_type_name = 0;
    source->object_type_name_length = 0;

    /* Move Decimal Value */
    if (source->decimal_value) {
        dest->decimal_value_length = source->decimal_value_length;
        dest->decimal_value = source->decimal_value;
        dest->decimal_buffer_length = source->decimal_buffer_length;
        source->decimal_value = 0;
        source->decimal_value_length = 0;
        source->decimal_buffer_length = 0;
    }

    /* Move String */
    if (source->string_value != source->small_string_buffer) {
        /* Move String Data */
        dest->string_value = source->string_value;
        dest->string_length = source->string_length;
        dest->string_buffer_length = source->string_buffer_length;
        dest->string_pos = source->string_pos;
#ifndef NUTF8
        dest->string_chars = source->string_chars;
        dest->string_char_pos = source->string_char_pos;
#endif
    }
    else {
        dest->string_length = source->string_length;
        dest->string_pos = source->string_pos;
#ifndef NUTF8
        dest->string_chars = source->string_chars;
        dest->string_char_pos = source->string_char_pos;
#endif
        if (dest->string_length) {
            memcpy(dest->string_value, source->string_value,
                   dest->string_length);
        }
    }

    /* Move Binary */
    if (source->binary_value) {
        dest->binary_length = source->binary_length;
        dest->binary_pos = source->binary_pos;
        dest->binary_value = source->binary_value;
        dest->binary_buffer_length = source->binary_buffer_length;
        dest->native_payload_ops = source->native_payload_ops;
        dest->native_payload_flags = source->native_payload_flags;
        source->binary_value = 0;
        source->binary_length = 0;
        source->binary_pos = 0;
        source->binary_buffer_length = 0;
        source->native_payload_ops = 0;
        source->native_payload_flags = 0;
    }
    else {
        dest->native_payload_ops = source->native_payload_ops;
        dest->native_payload_flags = source->native_payload_flags;
        source->native_payload_ops = 0;
        source->native_payload_flags = 0;
    }

    /* Move Attributes */
    dest->attributes = source->attributes;
    dest->unlinked_attributes = source->unlinked_attributes;
    dest->attribute_buffers = source->attribute_buffers;
    dest->max_num_attributes = source->max_num_attributes;
    dest->num_attributes = source->num_attributes;
    dest->num_attribute_buffers = source->num_attribute_buffers;

    if (dest->reference_identity || source->reference_identity) {
        rxvm_reference_identity_move(dest, source);
    }
    if (dest->reference_payload || source->reference_payload) {
        rxvm_reference_value_move_payload(dest, source);
    }

    /* Reset / fixup source */
    value_init(source);
}

RX_INLINE value *map_trimmed_attribute_pointer(value *old_attribute,
                                               value **old_unlinked_attributes,
                                               value *new_storage,
                                               size_t active_count) {
    size_t i;

    for (i = 0; i < active_count; i++) {
        if (old_attribute == old_unlinked_attributes[i]) return &new_storage[i];
    }
    return old_attribute;
}

RX_MOSTLYINLINE void maybe_trim_attribute_storage(value *v) {
    value **old_attributes;
    value **old_unlinked_attributes;
    value **old_attribute_buffers;
    value **new_attributes;
    value **new_unlinked_attributes;
    value **new_attribute_buffers;
    value *new_storage;
    size_t old_max;
    size_t old_buffer_count;
    size_t new_max;
    size_t i;

    if (!v || v->max_num_attributes < 64) return;
    if (v->num_attributes &&
        v->max_num_attributes <= v->num_attributes * 4) return;

    old_max = v->max_num_attributes;
    old_attributes = v->attributes;
    old_unlinked_attributes = v->unlinked_attributes;
    old_attribute_buffers = v->attribute_buffers;
    old_buffer_count = v->num_attribute_buffers;

    if (v->num_attributes == 0) {
        if (old_unlinked_attributes) {
            for (i = 0; i < old_max; i++) {
                if (old_unlinked_attributes[i]) clear_value_contents(old_unlinked_attributes[i]);
            }
        }
        if (old_attribute_buffers) {
            for (i = 0; i < old_buffer_count; i++) {
                if (old_attribute_buffers[i]) free(old_attribute_buffers[i]);
            }
            free(old_attribute_buffers);
        }
        if (old_attributes) free(old_attributes);
        if (old_unlinked_attributes) free(old_unlinked_attributes);
        v->attributes = 0;
        v->unlinked_attributes = 0;
        v->attribute_buffers = 0;
        v->num_attribute_buffers = 0;
        v->max_num_attributes = 0;
        return;
    }

    new_max = power_of_two_size(v->num_attributes);
    if (new_max >= old_max) return;

    new_attributes = malloc(sizeof(value*) * new_max);
    new_unlinked_attributes = malloc(sizeof(value*) * new_max);
    new_attribute_buffers = malloc(sizeof(value*));
    new_storage = malloc(sizeof(value) * new_max);
    if (!new_attributes || !new_unlinked_attributes || !new_attribute_buffers || !new_storage) {
        if (new_attributes) free(new_attributes);
        if (new_unlinked_attributes) free(new_unlinked_attributes);
        if (new_attribute_buffers) free(new_attribute_buffers);
        if (new_storage) free(new_storage);
        return;
    }

    for (i = 0; i < new_max; i++) {
        value_init(&new_storage[i]);
        new_unlinked_attributes[i] = &new_storage[i];
    }

    for (i = 0; i < v->num_attributes; i++) {
        move_value(&new_storage[i], old_unlinked_attributes[i]);
    }

    for (i = 0; i < v->num_attributes; i++) {
        new_attributes[i] = map_trimmed_attribute_pointer(old_attributes[i],
                                                          old_unlinked_attributes,
                                                          new_storage,
                                                          v->num_attributes);
    }
    for (i = v->num_attributes; i < new_max; i++) {
        new_attributes[i] = new_unlinked_attributes[i];
    }

    if (old_unlinked_attributes) {
        for (i = 0; i < old_max; i++) {
            if (old_unlinked_attributes[i]) clear_value_contents(old_unlinked_attributes[i]);
        }
    }
    if (old_attribute_buffers) {
        for (i = 0; i < old_buffer_count; i++) {
            if (old_attribute_buffers[i]) free(old_attribute_buffers[i]);
        }
        free(old_attribute_buffers);
    }
    if (old_attributes) free(old_attributes);
    if (old_unlinked_attributes) free(old_unlinked_attributes);

    new_attribute_buffers[0] = new_storage;
    v->attributes = new_attributes;
    v->unlinked_attributes = new_unlinked_attributes;
    v->attribute_buffers = new_attribute_buffers;
    v->num_attribute_buffers = 1;
    v->max_num_attributes = new_max;
}

/* Copy string value */
RX_INLINE void copy_string_value(value *dest, value *source) {
    if (dest == source) return;
    if (source->string_length) {
        /* Copy String Data */
        prep_string_buffer(dest, source->string_length);
        dest->string_pos = 0;
#ifndef NUTF8
        dest->string_chars = source->string_chars;
        dest->string_char_pos = 0;
#endif
        memcpy(dest->string_value, source->string_value, source->string_length);
    }
    else {
        dest->string_length = 0;
        dest->string_pos = 0;
#ifndef NUTF8
        dest->string_chars = 0;
        dest->string_char_pos = 0;
#endif
    }
    copy_vm_private_flags(dest, source);
}

/* Copy binary payload only. Public/compiler/library status flags are not copied. */
RX_INLINE void copy_binary_value(value *dest, value *source) {
    if (dest == source) return;

    if (dest->reference_payload) rxvm_reference_value_release_payload(dest);
    if (dest->native_payload_ops) clear_binary_payload(dest);

    if (source->native_payload_ops && source->native_payload_ops->copy) {
        source->native_payload_ops->copy(dest, source);
    }
    else if (source->binary_length) {
        if (prep_binary_buffer(dest, source->binary_length) != 0) abort();
        memcpy(dest->binary_value, source->binary_value, dest->binary_length);
        dest->binary_pos = source->binary_pos;
        dest->native_payload_ops = source->native_payload_ops;
        dest->native_payload_flags = source->native_payload_flags;
    }
    else {
        dest->binary_length = 0;
        dest->binary_pos = 0;
        dest->native_payload_ops = 0;
        dest->native_payload_flags = 0;
    }
}

/* Compares two strings. returns -1, 0, 1 as appropriate */
#define MIN(a,b) (((a)<(b))?(a):(b))

#ifndef NUTF8
RX_INLINE void string_set_byte_pos(value *v, size_t new_string_char_pos);

RX_INLINE void string_reset_cursor(value *v) {
    v->string_pos = 0;
    v->string_char_pos = 0;
}

RX_INLINE void string_set_lengths(value *v, size_t byte_length, size_t char_length) {
    v->string_length = byte_length;
    v->string_chars = char_length;
    string_reset_cursor(v);
}

RX_INLINE void string_set_ascii_length(value *v, size_t length) {
    string_set_lengths(v, length, length);
}

RX_INLINE size_t string_chars_remaining(const value *v) {
    if (v->string_char_pos >= v->string_chars) return 0;
    return v->string_chars - v->string_char_pos;
}

RX_INLINE void string_slice_from_cursor(value *dest, value *source, size_t char_count) {
    size_t byte_pos = source->string_pos;
    size_t actual_chars = MIN(char_count, string_chars_remaining(source));
    size_t byte_length = actual_chars;

    if (actual_chars == 0) {
        if (dest->string_buffer_length > 0) dest->string_value[0] = '\0';
        string_set_ascii_length(dest, 0);
        mark_utf8_valid_count(dest);
        return;
    }

#if ASCII_FAST_PATH
    if (source->string_chars != source->string_length)
#endif
    {
        size_t end_pos = byte_pos;
        size_t i;

        for (i = 0; i < actual_chars; ++i) {
            end_pos += utf8codepointcalcsize(source->string_value + end_pos);
        }
        byte_length = end_pos >= byte_pos ? end_pos - byte_pos : 0;
    }

    if (dest == source) {
        memmove(dest->string_value, source->string_value + byte_pos, byte_length);
    } else {
        prep_string_buffer(dest, byte_length);
        memcpy(dest->string_value, source->string_value + byte_pos, byte_length);
    }

    string_set_lengths(dest, byte_length, actual_chars);
    null_terminate_string_buffer(dest);
    if (has_utf8_valid_count(source)) mark_utf8_valid_count(dest);
    else clear_vm_private_flags(dest);
}

RX_INLINE void string_truncate_chars(value *v, size_t char_count) {
    int was_valid = has_utf8_valid_count_or_empty(v);
    string_set_byte_pos(v, char_count);
    string_set_lengths(v, v->string_pos, v->string_char_pos);
    null_terminate_string_buffer(v);
    if (was_valid || v->string_length == 0) mark_utf8_valid_count(v);
    else clear_vm_private_flags(v);
}
#else
RX_INLINE void string_reset_cursor(value *v) {
    v->string_pos = 0;
}

RX_INLINE void string_set_lengths(value *v, size_t byte_length, size_t char_length) {
    (void)char_length;
    v->string_length = byte_length;
    string_reset_cursor(v);
}

RX_INLINE void string_set_ascii_length(value *v, size_t length) {
    string_set_lengths(v, length, length);
}

RX_INLINE size_t string_chars_remaining(const value *v) {
    if (v->string_pos >= v->string_length) return 0;
    return v->string_length - v->string_pos;
}

RX_INLINE void string_slice_from_cursor(value *dest, value *source, size_t char_count) {
    size_t byte_pos = source->string_pos;
    size_t byte_length = MIN(char_count, string_chars_remaining(source));

    if (byte_length == 0) {
        if (dest->string_buffer_length > 0) dest->string_value[0] = '\0';
        string_set_ascii_length(dest, 0);
        clear_vm_private_flags(dest);
        return;
    }

    if (dest == source) {
        memmove(dest->string_value, source->string_value + byte_pos, byte_length);
    } else {
        prep_string_buffer(dest, byte_length);
        memcpy(dest->string_value, source->string_value + byte_pos, byte_length);
    }

    string_set_ascii_length(dest, byte_length);
    null_terminate_string_buffer(dest);
    clear_vm_private_flags(dest);
}

RX_INLINE void string_truncate_chars(value *v, size_t char_count) {
    size_t new_length = MIN(char_count, v->string_length);
    string_set_ascii_length(v, new_length);
    null_terminate_string_buffer(v);
    clear_vm_private_flags(v);
}
#endif

RX_INLINE int string_cmp(char *value1, size_t length1, char *value2, size_t length2) {
    int ret;

    ret = memcmp(value1, value2, MIN(length1, length2));
    if (!ret) {
        if (length1 > length2) ret = 1;
        else if (length1 < length2) ret = -1;
    }
    ret = ret > 0 ? 1 : (ret < 0 ? -1 : 0);

    return ret;
}

RX_INLINE int string_cmp_value(value *v1, value *v2) {
    return string_cmp(v1->string_value, v1->string_length,
                      v2->string_value, v2->string_length);
}

RX_INLINE int string_cmp_const(value *v1, string_constant *v2) {
    return string_cmp(v1->string_value, v1->string_length,
                      v2->string, v2->string_len);
}

RX_INLINE void string_append(value *v1, value *v2) {
    size_t start = v1->string_length;
    size_t append_length = v2->string_length;
#ifndef NUTF8
    size_t append_chars = v2->string_chars;
    int left_known = has_utf8_valid_count_or_empty(v1);
    int right_known = has_utf8_valid_count_or_empty(v2);
#endif

    extend_string_buffer(v1, v1->string_length + append_length);
    memcpy(v1->string_value + start, v2->string_value, append_length);
    v1->string_pos = 0;
#ifndef NUTF8
    v1->string_char_pos = 0;
    v1->string_chars += append_chars;
    set_utf8_known_concat_flags(v1, left_known, right_known);
#else
    clear_vm_private_flags(v1);
#endif
}

RX_INLINE void string_append_chars(value *v1, char *value, size_t length) {
    size_t start = v1->string_length;
#ifndef NUTF8
    int had_valid = has_utf8_valid_count_or_empty(v1);
#endif

    extend_string_buffer(v1, v1->string_length + length);
    memcpy(v1->string_value + start, value, length);

    v1->string_pos = 0;
#ifndef NUTF8
    v1->string_char_pos = 0;
    {
        size_t chars = 0;
        if (!utf8nvalid_count(value, length, &chars)) {
            v1->string_chars += chars;
            if (had_valid) mark_utf8_valid_count(v1);
            else clear_vm_private_flags(v1);
        } else {
            v1->string_chars += utf8nlen(value, length); /* SLOW! */
            clear_vm_private_flags(v1);
        }
    }
#else
    clear_vm_private_flags(v1);
#endif
}

RX_INLINE void string_sappend(value *v1, value *v2) {
    size_t start = v1->string_length;
    size_t append_length = v2->string_length;
#ifndef NUTF8
    size_t append_chars = v2->string_chars;
    int left_known = has_utf8_valid_count_or_empty(v1);
    int right_known = has_utf8_valid_count_or_empty(v2);
#endif

    extend_string_buffer(v1, v1->string_length + append_length + 1);
    v1->string_value[start++] = ' ';
    memcpy(v1->string_value + start, v2->string_value, append_length);
    v1->string_pos = 0;
#ifndef NUTF8
    v1->string_char_pos = 0;
    v1->string_chars += append_chars + 1;
    set_utf8_known_concat_flags(v1, left_known, right_known);
#else
    clear_vm_private_flags(v1);
#endif
}

RX_INLINE void string_concat(value *v1, value *v2, value *v3) {
    size_t len = v2->string_length + v3->string_length ;
    size_t buffer_len = buffer_size(len);
    char *buffer;
#ifndef NUTF8
    size_t left_chars = v2->string_chars;
    size_t right_chars = v3->string_chars;
    int left_known = has_utf8_valid_count_or_empty(v2);
    int right_known = has_utf8_valid_count_or_empty(v3);
#endif
    if (v1 == v2 || v1 == v3) {
        /* Need to use a buffer */
        buffer = malloc(buffer_len);

        memcpy(buffer, v2->string_value, v2->string_length);
        memcpy(buffer + v2->string_length, v3->string_value, v3->string_length);
        v1->string_pos = 0;

#ifdef NUTF8
        set_buffer_string(v1, buffer, len, buffer_len);
#else
        set_buffer_string(v1, buffer, len, buffer_len, left_chars + right_chars);
        v1->string_char_pos = 0;
#endif
    }
    else {
        /* Can write into v1 directly */
        prep_string_buffer(v1, len);
        memcpy(v1->string_value, v2->string_value, v2->string_length);
        memcpy(v1->string_value + v2->string_length, v3->string_value, v3->string_length);
        v1->string_pos = 0;
#ifndef NUTF8
        v1->string_chars = left_chars + right_chars;
        v1->string_char_pos = 0;
#endif
    }
#ifndef NUTF8
    set_utf8_known_concat_flags(v1, left_known, right_known);
#else
    clear_vm_private_flags(v1);
#endif
}

RX_INLINE void string_sconcat(value *v1, value *v2, value *v3) {
    size_t len = v2->string_length + v3->string_length + 1;
    size_t buffer_len = buffer_size(len);
    char *buffer;
#ifndef NUTF8
    size_t left_chars = v2->string_chars;
    size_t right_chars = v3->string_chars;
    int left_known = has_utf8_valid_count_or_empty(v2);
    int right_known = has_utf8_valid_count_or_empty(v3);
#endif
    if (v1 == v2 || v1 == v3) {
        /* Need to use a buffer */
        buffer = malloc(buffer_len);

        memcpy(buffer, v2->string_value, v2->string_length);
        buffer[v2->string_length] = ' ';
        memcpy(buffer + v2->string_length + 1, v3->string_value, v3->string_length);
        v1->string_pos = 0;
#ifdef NUTF8
        set_buffer_string(v1, buffer, len, buffer_len);
#else
        set_buffer_string(v1, buffer, len, buffer_len, left_chars + right_chars + 1);
        v1->string_char_pos = 0;
#endif
    }
    else {
        /* Can write into v1 directly */
        prep_string_buffer(v1, len);
        memcpy(v1->string_value, v2->string_value, v2->string_length);
        v1->string_value[v2->string_length] = ' ';
        memcpy(v1->string_value + v2->string_length + 1, v3->string_value, v3->string_length);
        v1->string_pos = 0;
#ifndef NUTF8
        v1->string_chars = left_chars + right_chars + 1;
        v1->string_char_pos = 0;
#endif
    }
#ifndef NUTF8
    set_utf8_known_concat_flags(v1, left_known, right_known);
#else
    clear_vm_private_flags(v1);
#endif
}

RX_INLINE void string_concat_var_const(value *v1, value *v2, string_constant *v3) {
    size_t len = v2->string_length + v3->string_len;
    size_t buffer_len = buffer_size(len);
    char *buffer;
#ifndef NUTF8
    size_t left_chars = v2->string_chars;
    int left_known = has_utf8_valid_count_or_empty(v2);
#endif
    if (v1 == v2) {
        /* Need to use a buffer */
        buffer = malloc(buffer_len);

        memcpy(buffer, v2->string_value, v2->string_length);
        memcpy(buffer + v2->string_length, v3->string, v3->string_len);
        v1->string_pos = 0;

#ifdef NUTF8
        set_buffer_string(v1, buffer, len, buffer_len);
#else
        set_buffer_string(v1, buffer, len, buffer_len,
                          left_chars + v3->string_chars);
        v1->string_char_pos = 0;
#endif
    }
    else {
        /* Can write into v1 directly */
        prep_string_buffer(v1, len);
        memcpy(v1->string_value, v2->string_value, v2->string_length);
        memcpy(v1->string_value + v2->string_length, v3->string, v3->string_len);
        v1->string_pos = 0;
#ifndef NUTF8
        v1->string_chars = left_chars + v3->string_chars;
        v1->string_char_pos = 0;
#endif
    }
#ifndef NUTF8
    set_utf8_known_concat_flags(v1, left_known, 1);
#else
    clear_vm_private_flags(v1);
#endif
}

RX_INLINE void string_sconcat_var_const(value *v1, value *v2, string_constant *v3) {
    size_t len = v2->string_length + v3->string_len + 1;
    size_t buffer_len = buffer_size(len);
    char *buffer;
#ifndef NUTF8
    size_t left_chars = v2->string_chars;
    int left_known = has_utf8_valid_count_or_empty(v2);
#endif
    if (v1 == v2) {
        /* Need to use a buffer */
        buffer = malloc(buffer_len);

        memcpy(buffer, v2->string_value, v2->string_length);
        buffer[v2->string_length] = ' ';
        memcpy(buffer + v2->string_length + 1, v3->string, v3->string_len);
        v1->string_pos = 0;

#ifdef NUTF8
        set_buffer_string(v1, buffer, len, buffer_len);
#else
        set_buffer_string(v1, buffer, len, buffer_len,
                          left_chars + v3->string_chars + 1);
        v1->string_char_pos = 0;
#endif
    }
    else {
        /* Can write into v1 directly */
        prep_string_buffer(v1, len);
        memcpy(v1->string_value, v2->string_value, v2->string_length);
        v1->string_value[v2->string_length] = ' ';
        memcpy(v1->string_value + v2->string_length + 1, v3->string, v3->string_len);
        v1->string_pos = 0;
#ifndef NUTF8
        v1->string_chars = left_chars + v3->string_chars + 1;
        v1->string_char_pos = 0;
#endif
    }
#ifndef NUTF8
    set_utf8_known_concat_flags(v1, left_known, 1);
#else
    clear_vm_private_flags(v1);
#endif
}

RX_INLINE void string_concat_const_var(value *v1, string_constant *v2, value *v3) {
    size_t len = v2->string_len + v3->string_length;
    size_t buffer_len = buffer_size(len);
    char *buffer;
#ifndef NUTF8
    size_t right_chars = v3->string_chars;
    int right_known = has_utf8_valid_count_or_empty(v3);
#endif
    if (v1 == v3) {
        /* Need to use a buffer */
        buffer = malloc(buffer_len);

        memcpy(buffer, v2->string, v2->string_len);
        memcpy(buffer + v2->string_len, v3->string_value, v3->string_length);
        v1->string_pos = 0;

#ifdef NUTF8
        set_buffer_string(v1, buffer, len, buffer_len);
#else
        set_buffer_string(v1, buffer, len, buffer_len,
                          v2->string_chars + right_chars);
        v1->string_char_pos = 0;
#endif
    }
    else {
        /* Can write into v1 directly */
        prep_string_buffer(v1, len);

        memcpy(v1->string_value, v2->string, v2->string_len);
        memcpy(v1->string_value + v2->string_len, v3->string_value, v3->string_length);
        v1->string_pos = 0;
#ifndef NUTF8
        v1->string_chars = v2->string_chars + right_chars;
        v1->string_char_pos = 0;
#endif
    }
#ifndef NUTF8
    set_utf8_known_concat_flags(v1, 1, right_known);
#else
    clear_vm_private_flags(v1);
#endif
}

RX_INLINE void string_sconcat_const_var(value *v1, string_constant *v2, value *v3) {
    size_t len = v2->string_len + v3->string_length + 1;
    size_t buffer_len = buffer_size(len);
    char *buffer;
#ifndef NUTF8
    size_t right_chars = v3->string_chars;
    int right_known = has_utf8_valid_count_or_empty(v3);
#endif
    if (v1 == v3) {
        /* Need to use a buffer */
        buffer = malloc(buffer_len);

        memcpy(buffer, v2->string, v2->string_len);
        buffer[v2->string_len] = ' ';
        memcpy(buffer + v2->string_len + 1, v3->string_value,
               v3->string_length);
        v1->string_pos = 0;

#ifdef NUTF8
        set_buffer_string(v1, buffer, len, buffer_len);
#else
        set_buffer_string(v1, buffer, len, buffer_len,
                          v2->string_chars + right_chars + 1);
        v1->string_char_pos = 0;
#endif
    }
    else {
        /* Can write into v1 directly */
        prep_string_buffer(v1, len);
        memcpy(v1->string_value, v2->string, v2->string_len);
        v1->string_value[v2->string_len] = ' ';
        memcpy(v1->string_value + v2->string_len + 1, v3->string_value,
               v3->string_length);
        v1->string_pos = 0;
#ifndef NUTF8
        v1->string_chars = v2->string_chars + right_chars + 1;
        v1->string_char_pos = 0;
#endif
    }
#ifndef NUTF8
    set_utf8_known_concat_flags(v1, 1, right_known);
#else
    clear_vm_private_flags(v1);
#endif
}

#ifndef NUTF8
/* This sets v's string_pos (the byte index) and v's string_char_pos
 * (the utf8 codepoint index) based on a new string_char_pos */
RX_INLINE void string_step_forward(value *v) {
    size_t step;
    size_t remaining;

    if (v->string_pos >= v->string_length) {
        v->string_pos = v->string_length;
        return;
    }

    step = utf8codepointcalcsize(v->string_value + v->string_pos);
    remaining = v->string_length - v->string_pos;
    if (step > remaining) step = remaining;
    v->string_pos += step;
}

RX_INLINE void string_step_backward(value *v) {
    size_t step;

    if (v->string_pos == 0) {
        v->string_pos = 0;
        return;
    }

    step = utf8rcodepointcalcsize(v->string_value + v->string_pos);
    if (step > v->string_pos) step = v->string_pos;
    v->string_pos -= step;
}

RX_INLINE void string_set_byte_pos(value *v, size_t new_string_char_pos) {
    assert (v->string_char_pos <= v->string_chars);

    // Boundary Check: If the requested position is beyond the last character,
    // clamp it to the end of the string.
    if (new_string_char_pos >= v->string_chars) {
        if (v->string_chars == 0) {
            v->string_pos = 0;
            v->string_char_pos = 0;
        } else {
            // Position at the very end of the string.
            // string_char_pos can be equal to string_chars, indicating a position
            // after the last character, which is useful for appending.
            // Here we will just set it to the last valid character index for seeking.
            v->string_pos = v->string_length;
            v->string_char_pos = v->string_chars;
        }
        return;
    }

    int diff = (int)new_string_char_pos - (int)v->string_char_pos;

    if (diff == 0) {
        return; // Nothing to do
    }

    // Optimised for stepping one character forward or backward
    if (diff == 1) {
        string_step_forward(v);
        v->string_char_pos++;
        return;
    }
    if (diff == -1) {
        string_step_backward(v);
        v->string_char_pos--;
        return;
    }

    // For larger jumps, determine the most efficient starting point.
    // We compare the cost of seeking from the start, the current position, or the end.
    size_t cost_from_start = new_string_char_pos;
    size_t cost_from_current = (diff > 0) ? diff : -diff;
    size_t cost_from_end = v->string_chars - new_string_char_pos;

    if (cost_from_start <= cost_from_current && cost_from_start <= cost_from_end) {
        // Seek from the beginning
        v->string_char_pos = 0;
        v->string_pos = 0;
        while (v->string_char_pos < new_string_char_pos) {
            string_step_forward(v);
            v->string_char_pos++;
        }
    } else if (cost_from_end < cost_from_current) {
        // Seek from the end (backwards)
        v->string_char_pos = v->string_chars;
        v->string_pos = v->string_length;
        while (v->string_char_pos > new_string_char_pos) {
            string_step_backward(v);
            v->string_char_pos--;
        }
    } else {
        // Seek from the current position
        if (diff > 0) { // Forward
            while (v->string_char_pos < new_string_char_pos) {
                string_step_forward(v);
                v->string_char_pos++;
            }
        } else { // Backward
            while (v->string_char_pos > new_string_char_pos) {
                string_step_backward(v);
                v->string_char_pos--;
            }
        }
    }
}
#endif

RX_INLINE void string_concat_char(value *v1, value *v2) {
    int char_size;
    char *insert_at;
#ifndef NUTF8
    int was_valid = has_utf8_valid_count_or_empty(v1);
    int scalar_valid = is_valid_unicode_scalar(v2->int_value);
#endif

    v1->string_pos = v1->string_length;
#ifdef NUTF8
    char_size = 1;
#else
    v1->string_char_pos = v1->string_chars;
    char_size = utf8codepointsize(v2->int_value);
#endif

    extend_string_buffer(v1,v1->string_length + char_size);
    insert_at = v1->string_value + v1->string_pos;

#ifdef NUTF8
    *insert_at = (unsigned char)v2->int_value;
    clear_vm_private_flags(v1);
#else
    v1->string_chars += 1;
    utf8catcodepoint(insert_at, v2->int_value, char_size);
    set_utf8_known_concat_flags(v1, was_valid, scalar_valid);
#endif
}

/* ****************************************************************************/
/* Funnctions to support operators for both the interpreter and the optimizer */
/* ****************************************************************************/

/* Calculate the string value */
RX_INLINE void int_to_string(numeric_context *cnt, value *temp, value *v) {
    if (cnt->digits >= DIGITS_STRIKE_POINT) {
        // Fast path for a large number of digits - just convert the integer to string and set exponent to 0
        prep_string_buffer(v, SMALLEST_STRING_BUFFER_LENGTH); // Large enough for an int
#ifdef __32BIT__
        v->string_length = snprintf(v->string_value,SMALLEST_STRING_BUFFER_LENGTH,"%ld",(long)v->int_value);
#else
        v->string_length = snprintf(v->string_value,SMALLEST_STRING_BUFFER_LENGTH,"%lld",(long long)v->int_value);
#endif
        v->string_pos = 0;
#ifndef NUTF8
        mark_ascii_string_valid_count(v);
#endif
        return;
    }

    extract_integer_decimal(cnt,temp, temp, v->int_value);
    RexxDecimalFormat(cnt, temp, temp, v);
}

/* Calculate the string value of v from its float value
 * cnt - numeric context is needed to ensure the format is correct
 * temp - a work buffer for the conversion
 * v - the value to convert (float value -> string value)
 */
RX_INLINE void float_to_string(numeric_context *cnt, value *temp, value *v) {
    extract_double_decimal(cnt,temp, temp, v->float_value);
    RexxDecimalFormat(cnt, temp, temp, v);
}

/* Calculate the integer value from float */
RX_INLINE void int_from_float(value *v) {
    v->int_value = floor(v->float_value);
    if (v->float_value - (double)v->int_value > 0.5) v->int_value++;
}

/* Make a malloced null terminated string from a register - needs to be free()d */
RX_INLINE char* reg2nullstring(value* reg) {
    char *buffer = malloc(reg->string_length + 1);

    /* Null terminated buffer */
    buffer[reg->string_length] = 0;
    memcpy(buffer, reg->string_value, reg->string_length);

    return buffer;
}

/* Convert a string to an integer - returns 1 on error */
RX_INLINE int string2integer(rxinteger *out, char *string, size_t length) {
    char *buffer = malloc(length + 1);
    char *end = buffer;
    int rc = 0;
    errno = 0;

    /* Null terminated buffer */
    buffer[length] = 0;
    memcpy(buffer, string, length);

    /* Convert */
#ifdef __32BIT__
    rxinteger l = strtol(buffer, &end, 10);
#else
    rxinteger l = strtoll(buffer, &end, 10);
#endif

    /* Convert error */
    if (errno == ERANGE || end == buffer) {
        rc = 1;
        goto end_string2integer;
    }

    /* Check only trailing spaces */
    while (*end != 0) {
        if (!isspace(*end)) {
            rc = 1;
            goto end_string2integer;
        }
        end++;
    }

    /* All good */
    *out = l;

    end_string2integer:
    free(buffer);
    return rc;
}

/* Convert a string to a float - returns 1 on error */
RX_INLINE int string2float(double *out, char *string, size_t length) {
    char *buffer = malloc(length + 1);
    char *end = buffer;
    int rc = 0;
    errno = 0;

    /* Null terminated buffer */
    buffer[length] = 0;
    memcpy(buffer, string, length);

    /* Convert */
    double l = strtod(buffer, &end);

    /* Convert error */
    if (errno == ERANGE || end == buffer) {
        rc = 1;
        goto end_string2float;
    }

    /* Check only trailing spaces */
    while (*end != 0) {
        if (!isspace(*end)) {
            rc = 1;
            goto end_string2float;
        }
        end++;
    }

    /* All good */
    *out = l;

    end_string2float:
    free(buffer);
    return rc;
}

/* Convert a string to a decimal - returns 1 on error
 * Validated the string is a valid decimal number
 * and returns the decimal as a malloced string in out.
 * The out must be freed by the caller
 * Returns 0 on success, 1 on error.
 */
RX_INLINE int stringtodecimal(char **out, char *string, size_t length) {
    // Note that decimal can have a large number of digits
    // First validate the string is a valid decimal number by checking each character
    // Skip leading spaces
    *out = NULL; // Set output to NULL (used on an error)
    int i;

    if (length == 0) {
        // Empty string is not a valid decimal
        return 1; // Error
    }
    // Skip leading spaces
    for (i = 0; i < length; i++) {
        if (!isspace(string[i])) break;
    }
    int start = i;
    // Check for a sign
    if (string[i] == '+') {
        i++;
        start = i;
    }
    else if (string[i] == '-') {
        i++;
    }
    // Check for digits before the decimal point
    int has_digits = 0;
    while (i < length && isdigit(string[i])) {
        has_digits = 1;
        i++;
    }
    // Check for a decimal point
    if (i < length && string[i] == '.') {
        i++;
        // Check for digits after the decimal point
        while (i < length && isdigit(string[i])) {
            has_digits = 1;
            i++;
        }
    }
    // Check for a trailing exponent
    if (i < length && (string[i] == 'e' || string[i] == 'E')) {
        i++;
        // Check for an optional sign
        if (i < length && (string[i] == '+' || string[i] == '-')) {
            i++;
        }
        // Check for digits in the exponent
        if (i < length && isdigit(string[i])) {
            while (i < length && isdigit(string[i])) {
                i++;
            }
        }
        else {
            // No digits in exponent
            return 1; // Error
        }
    }
    int end = i; // Mark the end of the valid decimal part

    // Check for a trailing 'd' which we use as a marker for a decimal literal
    if (i < length && string[i] == 'd') {
        i++;
    }

    // Skip trailing spaces
    while (i < length && isspace(string[i])) {
        i++;
    }

    // If we reached the end of the string and we have digits, we have a valid decimal
    if (i == length && has_digits) {
        // Allocate memory for the decimal string
        *out = malloc(end - start + 1);
        if (*out == NULL) {
            // Error allocating memory - PANIC and exit with error
            fprintf(stderr, "Memory allocation error in stringtodecimal\n");
            exit(EXIT_FAILURE);
        }
        // Copy the valid decimal part to the output
        memcpy(*out, string + start, end - start);
        (*out)[end - start] = '\0'; // Null-terminate the string
        return 0; // Success
    }
    // If we reach here, the string is not a valid decimal
    return 1; // Error
}

// Static function to trim trailing zeros from a number format and including possibly the decimal point
static void trim_numeric_trailing_zeros(char *str) {
    size_t len = strlen(str);
    if (len == 0)
        return;

    // Find the decimal point
    char *dot = strchr(str, '.');
    if (!dot)
        return; // No decimal point, nothing to trim

    // Start from the end of the string
    char *end = str + len - 1;

    // Remove trailing zeros
    while (end > dot && *end == '0') {
        *end = '\0';
        end--;
    }

    // If the last character is a decimal point, remove it
    if (end == dot) {
        *end = '\0';
    }
}

// Function to extract decimal components from a double
// - coefficient (string) will be set to the coefficient string (or nan, inf, -inf)
// - exponent (integer) will be set to the exponent
static void extract_double_decimal(numeric_context* num_context, value *coefficient, value *exponent, double value) {

    size_t digits = num_context->digits;
    if (digits < DIGITS_MINIMUM) digits = DIGITS_MINIMUM;
    else if (digits > DBL_DIG) digits = DBL_DIG;

    // Set Buffer Size in Coefficient Value
    prep_string_buffer(coefficient, digits + 5); // +5 for sign, decimal point, possible rounding digit and null terminator
    exponent->int_value = 0;

    // Handle special cases
    if (isnan(value)) {
        if (num_context->casetype == CASE_UPPER)
            strcpy(coefficient->string_value, "NAN");
        else
            strcpy(coefficient->string_value, "nan");
        coefficient->string_length = 3;
        coefficient->string_pos = 0;
#ifndef NUTF8
        mark_ascii_string_valid_count(coefficient);
#endif
        return;
    }
    if (isinf(value)) {
        if (num_context->casetype == CASE_UPPER)
            strcpy(coefficient->string_value, signbit(value) ? "-INF" : "INF");
        else
            strcpy(coefficient->string_value, signbit(value) ? "-inf" : "inf");
        coefficient->string_length = signbit(value) ? 4 : 3;
        coefficient->string_pos = 0;
#ifndef NUTF8
        mark_ascii_string_valid_count(coefficient);
#endif
        return;
    }

    if (value == 0.0) {
        // Handle zero
        strcpy(coefficient->string_value, "0");
        coefficient->string_length = 1;
        coefficient->string_pos = 0;
#ifndef NUTF8
        mark_ascii_string_valid_count(coefficient);
#endif
        return;
    }

    // Determine if negative
    int is_negative = value < 0.0;
    double abs_value = fabs(value);

    // Calculate decimal exponent
    int64_t exp = (int64_t)floor(log10(abs_value));

    // Normalize the coefficient to [1.0, 10.0)
    double coeff = abs_value / pow(10.0, (double)exp);

    // Adjust if coeff is exactly 10.0 due to floating-point inaccuracies
    if (coeff >= 10.0) {
        coeff /= 10.0;
        exp += 1;
    }

    // Adjust if coeff is smaller than 1 due to floating-point inaccuracies
    if (coeff < 1.0) {
        coeff *= 10.0;
        exp -= 1;
    }

    exponent->int_value = exp;

    // Format the coefficient string with precision up to DBL_DIG-1 fractional digits
    snprintf(coefficient->string_value, digits + 5, is_negative ? "-%.*lf" : "%.*lf", (int)(digits - 1), coeff);

    // Logic to [re-]check for edge case where rounding the coefficient could change the exponent - we look at the string
    char* abs_start = is_negative ? coefficient->string_value + 1 : coefficient->string_value;

    // If the coefficient starts with "10." it means rounding has caused it to become >= 10.0 so we need to adjust
    if (strncmp(abs_start, "10.", 3) == 0) {
        // Adjust coefficient and exponent but moving the decimal point left
        abs_start[2] = abs_start[1]; // Move the '0' to replace the '.'
        abs_start[1] = '.';          // Put the decimal point after the '1'
        (exponent->int_value)++;     // Increment the exponent
    }
    else if (strncmp(abs_start, "0.", 2) == 0) {
        // If the coefficient starts with "0." it means rounding has caused it to become < 1.0 so we need to adjust
        if (abs_start[2] != '\0') { // Just in case of a malformed string
            // Adjust coefficient and exponent by moving the decimal point right
            abs_start[0] = abs_start[2]; // Move the first digit after the '.' to the front
            // Shift the rest of the string left
            memmove(abs_start + 2, abs_start + 3, strlen(abs_start + 3) + 1);
            (exponent->int_value)--;                 // Decrement the exponent
        }
    }

    trim_numeric_trailing_zeros(coefficient->string_value);
    coefficient->string_length = strlen(coefficient->string_value);
    coefficient->string_pos = 0;
#ifndef NUTF8
    mark_ascii_string_valid_count(coefficient);
#endif
}

/* Calculate the number of digits in an integer, including the sign if negative */
static size_t number_of_digits(rxinteger n) {
    if (n == 0) {
        return 1;
    }
    size_t digits = 0;

    // By using an unsigned type, we can safely represent the absolute value
    unsigned long long num;

    if (n < 0) {
        num = -(unsigned long long)n;
        digits = 1; // For the negative sign
    } else {
        num = n;
    }

    while (num > 0) {
        num /= 10;
        digits++;
    }

    return digits;
}

/*
 * Converts a numeric value represented by a coefficient and exponent into a formatted string.
 * The formatting can be either scientific or engineering, and the case of the exponent can be upper
 * or lower.
 * - the coefficient is a string representing the normalized number (e.g., "-1.2345").
 * - the exponent is an integer representing the power of ten.
 *
 * In the num_conntext:
 * - the digits parameter specifies the total number of significant digits to consider.
 * - the form parameter specifies whether to use scientific or engineering notation.
 * - the casetype parameter specifies whether the exponent should be in upper or lower case.
 *
 * The formatted output is written to the formatted_output buffer.
 *
 * The function handles special cases like zero, NaN, and infinity.
 */
static void RexxDecimalFormat(numeric_context* num_context, value *coefficient_value, value *exponent_value, value *formatted_output_value) {

    const char *coef_start;
    size_t digits_in_coef;
    int use_exponential;
    int is_engineering;
    size_t i;
    const char *scientific_format;
    const char *engineering_format;
    char *coefficient = coefficient_value->string_value;
    coefficient[coefficient_value->string_length] = 0; // Null-terminate - just in case
    rxinteger exponent = exponent_value->int_value;

    /* Prepare the output buffer */
    // Calculate the output buffer size which is based on the number of digits and the exponent size from the arguments
    size_t output_buffer_size = coefficient_value->string_length + number_of_digits(exponent) + 5; // +5 for sign, decimal point, 'e' and null terminator
    prep_string_buffer(formatted_output_value, output_buffer_size);
    formatted_output_value->string_length = 0;
    formatted_output_value->string_value[0] = 0; // Null-terminate - just in case
    formatted_output_value->string_pos = 0;
#ifndef NUTF8
    mark_ascii_string_valid_count(formatted_output_value);
#endif
    char *formatted_output = formatted_output_value->string_value;

    /* Case specific Formats */
    if (num_context->casetype == CASE_UPPER) {
        scientific_format = "%sE%+lld";
        engineering_format = "E%+lld";
    }
    else {
        scientific_format = "%se%+lld";
        engineering_format = "e%+lld";
    }

    // If exponent is 0, we can use the simple format directly
    if (exponent == 0) {
        strcpy(formatted_output, coefficient); // This also handles 0, nan, inf
        // convert case if the first character isn't a digit or '-'

        /* Detecting a number - funny logic, but we have "-inf" and so on to handle, and it is a normalised x.xxx */
        /* So, is it a one-digit number, if not check the second character for a decimal point */
        if (formatted_output[1] != 0 && formatted_output[1] != '.' ) {
            /* If not a number (e.g. nan, inf, -inf), convert case */
            if (num_context->casetype == CASE_UPPER) {
                for (i = 0; formatted_output[i] != 0; i++)
                    formatted_output[i] = (char)toupper((unsigned char)formatted_output[i]);
            }
            else {
                for (i = 0; formatted_output[i] != 0; i++)
                    formatted_output[i] = (char)tolower((unsigned char)formatted_output[i]);
            }
        }
        formatted_output_value->string_length = strlen(formatted_output_value->string_value);
        formatted_output_value->string_pos = 0;
#ifndef NUTF8
        mark_ascii_string_valid_count(formatted_output_value);
#endif
        return;
    }

    // Apply the REXX rule to decide on simple vs. exponential format
    // See ANSI REXX standard, 7.4.10, Floating() routine
    use_exponential = (exponent + 1 > (rxinteger)(num_context->digits)) || (exponent < -6);
    if (use_exponential) {
        is_engineering = (num_context->form == NUMERIC_FORM_ENGINEERING);
        if (!is_engineering) {
            // --- SCIENTIFIC NOTATION ---
            // The number has already been formatted in scientific notation by decimalExtract
            sprintf(formatted_output, scientific_format, coefficient, exponent);
            formatted_output_value->string_length = strlen(formatted_output_value->string_value);
            formatted_output_value->string_pos = 0;
#ifndef NUTF8
            mark_ascii_string_valid_count(formatted_output_value);
#endif
            return;
        }
    }

    // We will need to process the coefficient for simple format or engineering format

    // Remove the negative and decimal point from the coefficient, it has been normalised to [-]x[.xxxxx]
    // Add the negative to the string output
    if (coefficient[0] == '-') {
        coef_start = coefficient + 1;
        *formatted_output++ = '-';
    } else {
        coef_start = coefficient;
    }
    digits_in_coef = strlen(coef_start);
    if (digits_in_coef > 1) {
        // More than one digit - remove the decimal point
        digits_in_coef--;
    }

    if (!use_exponential) {
        // --- SIMPLE FORMAT ---
        if (exponent > 0) { // Note: exponent == 0 is handled above
            // Positive exponent
            if (exponent < digits_in_coef) {
                // Insert a decimal point within the coefficient
                // Copy up to the position of the decimal point
                // First digit
                *formatted_output++ = coef_start[0];
                // Remaining digits before the new decimal point
                if (coef_start[1] != 0) {
                    strncpy(formatted_output, coef_start + 2, exponent);
                    formatted_output += (exponent);
                }
                // Insert the decimal point - if we have more digits
                if (coef_start[exponent + 2] != 0) {
                    *formatted_output++ = '.';
                    // Copy the rest of the digits after the decimal point
                    strcpy(formatted_output, coef_start + exponent + 2);
                }
                else {
                    formatted_output[0] = 0; // Null-terminate
                }
            } else {
                // Append zeros to the end
                // Copy the coefficient
                // First digit
                formatted_output[0] = coef_start[0];
                // Remaining digits after the decimal point (if any)
                if (coef_start[1] != 0) {
                    strcpy(formatted_output + 1, coef_start + 2);
                }
                // Append zeros
                for (i = 0; i < exponent - digits_in_coef + 1; i++) {
                    formatted_output[i + digits_in_coef] = '0';
                }
                formatted_output[i + digits_in_coef] = 0; // Null-terminate
            }
        }
        else {
            // Negative exponent
            strcpy(formatted_output, "0.");
            for (i = 0; i < -exponent - 1; i++) {
                formatted_output[i + 2] = '0';
            }
            // Copy the coefficient after the leading zeros - first digit
            formatted_output[-exponent + 1] = coef_start[0];
            // Remaining digits after the decimal point (if any)
            for (i = 1; i < digits_in_coef; i++) {
                formatted_output[-exponent + 1 + i] = coef_start[i + 1];
            }
            formatted_output[-exponent + 1 + digits_in_coef] = 0;
        }
        formatted_output_value->string_length = strlen(formatted_output_value->string_value);
        formatted_output_value->string_pos = 0;
#ifndef NUTF8
            mark_ascii_string_valid_count(formatted_output_value);
#endif
        return;
    }

    // --- EXPONENTIAL FORMAT ---
    // ENGINEERING form: exponent must be a multiple of 3; 1..3 digits before the decimal point.
    // Adjust the exponent to a multiple of 3 using a non-negative remainde
    int rem = (int)(exponent % 3);
    int k = (rem + 3) % 3;          // how many places to shift the decimal point to the RIGHT
    rxinteger eng_exp = exponent - k;

    // Build the engineering mantissa by inserting the decimal point after (k+1) digits.
    size_t need_int = (size_t)k + 1;

    if (digits_in_coef <= need_int) {

        // Not enough digits for a fractional part: pad with zeros up to need_int
        // Copy first digit
        formatted_output[0] = coef_start[0];
        // Copy remaining digits in the coefficient (if any)
        if (coef_start[1] != 0) {
            strncpy(formatted_output + 1, coef_start + 2, digits_in_coef - 1);
        }
        // Pad with zeros
        for (i = digits_in_coef; i < need_int; ++i) {
            formatted_output[i] = '0';
        }
        // Move output pointed to the end of the integer part
        formatted_output += need_int;

    } else {

        // We have more than (k+1) digits: insert decimal point
        // Copy the first digit
        formatted_output[0] = coef_start[0];
        // Copy the next (need_int - 1) digits (skipping the old decimal point)
        memcpy(formatted_output + 1, coef_start + 2, need_int - 1);
        formatted_output[need_int] = '.';
        // Copy the rest of the digits after the decimal point
        for (i = 0; i < digits_in_coef - need_int; ++i) {
            formatted_output[need_int + 1 + i] = coef_start[need_int + 1 + i];
        }
        // Move output pointed to the end of the integer part
        formatted_output += 1 + digits_in_coef;

    }

    // Add the exponent
    sprintf(formatted_output, engineering_format, eng_exp);
    formatted_output_value->string_length = strlen(formatted_output_value->string_value);
    formatted_output_value->string_pos = 0;
#ifndef NUTF8
    mark_ascii_string_valid_count(formatted_output_value);
#endif
}

// Function to extract decimal components from an integer
// - coefficient (string) will be set to the coefficient string (or nan, inf, -inf)
// - exponent (integer) will be set to the exponent
static void extract_integer_decimal(numeric_context* num_context, value *coefficient, value *exponent, rxinteger value) {

    // Handle special case of zero
    if (value == 0) {
        strcpy(coefficient->string_value, "0");
        coefficient->string_length = 1;
        coefficient->string_pos = 0;
#ifndef NUTF8
        mark_ascii_string_valid_count(coefficient);
#endif
        exponent->int_value = 0;
        return;
    }

    prep_string_buffer(coefficient, SMALLEST_STRING_BUFFER_LENGTH); // Large enough for an int
#ifdef __32BIT__
    coefficient->string_length = snprintf(coefficient->string_value,SMALLEST_STRING_BUFFER_LENGTH,"%ld",(long)value);
#else
    coefficient->string_length = snprintf(coefficient->string_value,SMALLEST_STRING_BUFFER_LENGTH,"%lld",(long long)value);
#endif
    coefficient->string_pos = 0;

    // We are converting to coefficient, as an example, from 123456 to 1.23456 (i.e. normalised scientific notation)
    if (value > 0) {
        // Positive number logic

        // Calculate the exponent based on the number of digits in the integer
        // The exponent is the number of digits - 1
        exponent->int_value = (rxinteger)(coefficient->string_length - 1);

        // Insert the decimal point after the first digit if there are more than 1 digit
        if (coefficient->string_length > 1) {
            // Shift the string to the right to make space for the decimal point
            memmove(coefficient->string_value + 2, coefficient->string_value + 1, coefficient->string_length);
            coefficient->string_value[1] = '.';
            coefficient->string_length++;
            coefficient->string_value[coefficient->string_length] = 0; // Null-terminate
        }
    }
    else {
        // Negative number logic

        // Calculate the exponent based on the number of digits in the integer
        // The exponent is the number of digits - 2 (to account for the '-' sign)
        exponent->int_value = (rxinteger)(coefficient->string_length - 2);

        // Insert the decimal point after the first digit following the '-' sign if there are more than 2 characters
        if (coefficient->string_length > 2) {
            // Shift the string to the right to make space for the decimal point
            memmove(coefficient->string_value + 3, coefficient->string_value + 2, coefficient->string_length - 1);
            coefficient->string_value[2] = '.';
            coefficient->string_length++;
            coefficient->string_value[coefficient->string_length] = 0; // Null-terminate
        }
    }

    // Set the utf8 values
#ifndef NUTF8
    mark_ascii_string_valid_count(coefficient);
#endif
}

#endif //CREXX_RXVMVARS_H
