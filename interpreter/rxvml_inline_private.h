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

/* Host-only pins for the exact bytecode members loaded from a trusted artifact.
 * Capture module_count()+1 before a load and its returned last module number.
 * The inclusive range can contain several linked members. A locator is its
 * basename, optionally ending in .rxas; it is never a context-wide lookup.
 * DESCRIPTOR has the complete META_FUNC name, including the class for methods.
 * Resolve returns 0 on success, 1 for an absent member/name, and -1 for invalid,
 * duplicate, signature-mismatched or non-owned definitions. METHOD is 0 or 1.
 * Call returns 0 or -1 and requires a receiver exactly when METHOD is 1.
 * These fixed entry points use the existing RXVML invocation/diagnostic owner.
 * Pins belong to the caller, on the context's owning thread. Context teardown
 * invalidates every live pin before releasing modules; free remains safe after
 * teardown, and a stale pin cannot become valid in a later context. */
typedef struct rxvml_inline_procedure rxvml_inline_procedure;
size_t rxvml_inline_module_count(rxvml_context *ctx);
/* After every explicit service/runtime load, link and freeze one helper
 * universe. Distinct concrete exports with the same resolved key are refused;
 * imported declarations are not definitions. Seal before resolve/call or
 * callback installation. Any later module/generation change invalidates it. */
int rxvml_inline_seal_context(rxvml_context *ctx);
int rxvml_inline_resolve_owned(rxvml_context *ctx,
        size_t first_module, size_t last_module, const char *member_name,
        const char *descriptor, int method, rxvml_inline_procedure **out);
int rxvml_inline_call_owned(rxvml_context *ctx,
        const rxvml_inline_procedure *pin, rxvml_value *receiver,
        size_t argc, rxvml_value **arguments, rxvml_value **result);
void rxvml_inline_procedure_free(rxvml_inline_procedure *pin);

#endif
