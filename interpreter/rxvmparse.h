/* cREXX License (MIT). Private shared frozen-PARSE execution kernel.
 * This header is not an installed SDK or source-language interface. */
#ifndef CREXX_RXVMPARSE_H
#define CREXX_RXVMPARSE_H

#include <stddef.h>

typedef struct rxvm_parse_cursor rxvm_parse_cursor;

enum rxvm_parse_result {
    RXVM_PARSE_DONE = 0,
    RXVM_PARSE_FIELD = 1,
    RXVM_PARSE_INPUT = 2,
    RXVM_PARSE_INVALID = -1,
    RXVM_PARSE_UNICODE = -2,
    RXVM_PARSE_RANGE = -3,
    RXVM_PARSE_NOMEM = -4,
    RXVM_PARSE_CONVERSION = -5
};

typedef struct rxvm_parse_diagnostic {
    int signal;
    const char *message;
} rxvm_parse_diagnostic;

/* Own both byte snapshots; validate the entire input before returning Ready.
 * NULL is valid only for a zero-length source. Failure releases all storage,
 * returns NULL and places a negative rxvm_parse_result in status. */
rxvm_parse_cursor *rxvm_parse_create(const char *source, size_t source_length,
                                    const unsigned char *plan, size_t plan_length,
                                    int *status);

/* Events are exactly the public cursor events. Failed calls return a negative
 * result and preserve diagnostic item/slot IDs. Input requires one supply. */
int rxvm_parse_next(rxvm_parse_cursor *cursor);
int rxvm_parse_supply(rxvm_parse_cursor *cursor, const char *text, size_t length);
size_t rxvm_parse_item(const rxvm_parse_cursor *cursor);
size_t rxvm_parse_slot(const rxvm_parse_cursor *cursor);

/* Field only. This borrowed view expires at next/supply/close/destroy. A native
 * binding must copy it into the caller's result during this call. Drop fields
 * have slot zero and need no text materialization when they are ignored. */
int rxvm_parse_text(rxvm_parse_cursor *cursor, const char **text, size_t *length);

/* Driver publication/host failures use the same terminal transition without
 * losing the current event IDs. Terminal Done/Failed/Closed stay terminal. */
int rxvm_parse_fail(rxvm_parse_cursor *cursor, int result);

/* Close is idempotent and releases execution buffers. Destroy also frees the
 * cursor header; the RXPA resource's last-owner hook owns that final call. */
void rxvm_parse_close(rxvm_parse_cursor *cursor);
void rxvm_parse_destroy(rxvm_parse_cursor *cursor);
int rxvm_parse_classic_numeric(const rxvm_parse_cursor *cursor);
/* Whole-opcode vector sizing; these facts come from the validated header. */
size_t rxvm_parse_result_count(const rxvm_parse_cursor *cursor);
size_t rxvm_parse_external_count(const rxvm_parse_cursor *cursor);

/* One error-policy/message owner for the frozen opcode and RXPA binding.
 * This returns data only; each driver raises its own bound VM event. */
rxvm_parse_diagnostic rxvm_parse_diagnose(int result, int classic_numeric);

#endif
