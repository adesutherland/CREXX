/* cREXX License (MIT). Private core/native-compiler declaration bridge. */
#ifndef CREXX_RXVMPARSE_BINDINGS_H
#define CREXX_RXVMPARSE_BINDINGS_H
#include <stddef.h>

enum rxvm_parse_binding_kind {
    RXVM_PARSE_BIND_CLASS,
    RXVM_PARSE_BIND_MEMBER,
    RXVM_PARSE_BIND_FUNCTION
};
typedef struct rxvm_parse_binding {
    enum rxvm_parse_binding_kind kind;
    const char *symbol;
    const char *member_kind;
    const char *member;
    const char *type;
    const char *args;
} rxvm_parse_binding;

/* Compiled DECL_ONLY from the same seven definitions as the runtime binding.
 * Binutils owns the shared declaration objects. Immutable records are imported
 * per compiler request and queried by the linker, never registered in global
 * compiler callback lists. This is not an installed API. */
extern const rxvm_parse_binding rxvm_parse_cursor_bindings[];
extern const size_t rxvm_parse_cursor_binding_count;
extern const char rxvm_parse_cursor_provider_id[];
void rxvm_parse_cursor_init(void);
#endif
