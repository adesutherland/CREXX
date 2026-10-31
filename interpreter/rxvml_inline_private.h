/* cREXX License (MIT). Private linker-service authority dispatch.
 *
 * This header is not installed and does not expose a general native provider.
 * Only the adapter which owns a live RXVML request may install this callback.
 * The two fixed native procedures below return unproved without that owner.
 * Argument values are borrowed for the callback; retaining an ordinary value
 * or reference does not make it immutable or extend its target lifetime.
 */
#ifndef CREXX_RXVML_INLINE_PRIVATE_H
#define CREXX_RXVML_INLINE_PRIVATE_H

#include "rxvml.h"

enum rxvml_inline_operation {
    RXVML_INLINE_TOKEN = 1,
    RXVML_INLINE_CHECK = 2
};

/* Return exactly 1 after publishing a proved result into RESULT. Every other
 * return is unproved and the staged result is discarded. Arguments and RESULT
 * are distinct even when the VM's eventual result aliases an input. */
typedef int (*rxvml_inline_callback)(rxvml_context *ctx, void *userdata,
        enum rxvml_inline_operation operation, size_t argc,
        rxvml_value *const *arguments, rxvml_value *result);

/* The callback/userdata remain borrowed until reset. Reset before destroying
 * request-owned session storage. Replacing an active callback is refused. */
int rxvml_inline_set_callback(rxvml_context *ctx,
        rxvml_inline_callback callback, void *userdata);

#endif
