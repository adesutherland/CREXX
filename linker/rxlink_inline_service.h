/* cREXX License (MIT). Frozen selection adapter to the shared B/L service. */
#ifndef CREXX_RXLINK_INLINE_SERVICE_H
#define CREXX_RXLINK_INLINE_SERVICE_H

#include <stddef.h>
#include "rxbin.h"

typedef struct rxlink_inline_options {
    const char *service;
    const char *policy;
    const char *location;
    char *const *runtime;
    size_t runtime_count;
    const char *const *selectors;
} rxlink_inline_options;

/* Returns a fully validated ordered candidate array, or leaves it NULL.
 * NULL candidate entries mean retain that original selected module. Caller
 * publishes the whole array together and marks its final image seals. */
int rxlink_inline_service(module_file *const *selected, size_t count,
                         const rxlink_inline_options *options,
                         module_file ***candidates_out);

/* Internal validation seam shared with the focused metadata controls. */
int rxlink_inline_metadata_equal(const module_file *original,
                                const module_file *candidate);

#endif
