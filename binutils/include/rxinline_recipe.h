/* Checked, portable exact module and procedure references for RXAS/RXBIN. */
#ifndef CREXX_RXINLINE_RECIPE_H
#define CREXX_RXINLINE_RECIPE_H

#include <stddef.h>
#include <stdint.h>

typedef struct rxinl_module_ref_view {
    uint32_t image_module;
    const char *selector;
    uint32_t selector_length;
    int sealed;
    int self_relative;
} rxinl_module_ref_view;

typedef struct rxinl_qualified_proc_view {
    uint32_t image_module;
    uint32_t provider_procedure;
    int sealed;
} rxinl_qualified_proc_view;

/* Constructors allocate portable binary constants owned by the caller.
 * All multi-byte wire fields are little-endian u32. Sealing binds the exact
 * image member selected by the caller; it grants no optimizer proof. */
int rxinl_module_ref_from_text(const char *selector, unsigned char **bytes,
                               size_t *length);
int rxinl_module_ref_decode(const void *bytes, size_t length,
                            rxinl_module_ref_view *view);
int rxinl_module_ref_seal(void *bytes, size_t length, uint32_t image_module);
int rxinl_qualified_proc_from_text(const char *source, unsigned char **bytes,
                                   size_t *length);
int rxinl_qualified_proc_decode(const void *bytes, size_t length,
                                rxinl_qualified_proc_view *view);
int rxinl_qualified_proc_seal(void *bytes, size_t length);

#endif
