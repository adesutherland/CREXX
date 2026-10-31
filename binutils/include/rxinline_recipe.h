/* Checked, portable operational recipe for logical RXAS inline activations. */
#ifndef CREXX_RXINLINE_RECIPE_H
#define CREXX_RXINLINE_RECIPE_H

#include <stddef.h>
#include <stdint.h>

#define RXINL_RECIPE_HEADER_SIZE 64u
#define RXINL_RECIPE_MAX_ITEMS 4096u
#define RXINL_RECIPE_UNSEALED UINT32_MAX

enum rxinl_recipe_flags {
    RXINL_COUNT_REGISTER = 1u << 0,
    RXINL_ARGS_WINDOW = 1u << 1,
    RXINL_PREP_SWAP = 1u << 2,
    RXINL_PREP_SETTP = 1u << 3,
    RXINL_RECIPE_SEALED = 1u << 4,
    RXINL_PREP_CLEAR = 1u << 5,
    RXINL_RESULT_IGNORED = 1u << 6,
    RXINL_RECIPE_KNOWN_FLAGS = RXINL_COUNT_REGISTER | RXINL_ARGS_WINDOW |
                               RXINL_PREP_SWAP | RXINL_PREP_SETTP |
                               RXINL_RECIPE_SEALED | RXINL_PREP_CLEAR |
                               RXINL_RESULT_IGNORED
};

typedef struct rxinl_recipe_view {
    const unsigned char *bytes;
    size_t length;
    uint32_t flags;
    uint32_t image_module;
    uint32_t provider_procedure;
    uint32_t count_source;
    uint32_t a0_register;
    uint32_t argument_base;
    uint32_t argument_count;
    uint32_t proxy_count;
    uint32_t owned_count;
    uint32_t status_register;
    uint32_t status_flags;
    uint32_t swap_first;
    uint32_t swap_second;
    uint32_t clear_register;
} rxinl_recipe_view;

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

/* The RXAS parser calls this once. The result is an operational binary
 * constant, never an optimizer proof. All multi-byte fields are LE u32. */
int rxinl_recipe_from_text(const char *source, unsigned char **bytes,
                           size_t *length, char *error, size_t error_size);
int rxinl_recipe_decode(const void *bytes, size_t length,
                        rxinl_recipe_view *view);
int rxinl_recipe_seal(void *bytes, size_t length, uint32_t image_module,
                       uint32_t provider_procedure);
/* Caller owns the formatted source text. */
char *rxinl_recipe_to_text(const rxinl_recipe_view *view);
uint32_t rxinl_recipe_argument(const rxinl_recipe_view *view, uint32_t index);
uint32_t rxinl_recipe_proxy_register(const rxinl_recipe_view *view, uint32_t index);
uint32_t rxinl_recipe_proxy_argument(const rxinl_recipe_view *view, uint32_t index);
uint32_t rxinl_recipe_owned_register(const rxinl_recipe_view *view, uint32_t index);
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
