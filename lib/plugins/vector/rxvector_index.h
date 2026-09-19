/* MIT. Generic immutable RXVIDX/1 float32 matrix, independent of application policy. */
#ifndef CREXX_RXVECTOR_INDEX_H
#define CREXX_RXVECTOR_INDEX_H
#include <stddef.h>
#include "rxvector_kernel.h"
typedef struct rxvector_text { const char *data; size_t length; } rxvector_text;
typedef struct rxvector_index {
    unsigned char *wire;
    size_t bytes, rows, dimensions, metadata_offset, metadata_length, matrix_offset;
    size_t *label_offsets;
} rxvector_index;
int rxvector_index_open(const void *, size_t, rxvector_index **, const char **);
int rxvector_index_make(const void *, size_t, size_t, const rxvector_text *, size_t,
                       rxvector_text, rxvector_index **, const char **);
void rxvector_index_free(rxvector_index *);
rxvector_text rxvector_index_label(const rxvector_index *, size_t);
rxvector_status rxvector_index_search(const rxvector_index *, const void *, size_t,
                                     size_t, rxvector_hit *);
#endif
