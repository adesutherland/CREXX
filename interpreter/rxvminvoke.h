/* cREXX License (MIT). Private checked ordinary-call bridge. */
#ifndef CREXX_RXVMINVOKE_H
#define CREXX_RXVMINVOKE_H
#include <stddef.h>
#include <stdint.h>

struct rxvm_context;
struct proc_runtime;
struct module;
struct stack_frame;
struct value;
typedef struct rxvm_invocation rxvm_invocation;

/* Stack-owned and visible only during one ordinary native call. Submission is
 * restricted to the designated rxinvoke.invoke body, not a nested callback. */
typedef struct rxvm_invocation_boundary {
    struct rxvm_invocation_boundary *previous;
    struct rxvm_context *owner_context;
    struct proc_runtime *procedure;
    struct stack_frame *parent;
    rxvm_invocation *request;
    uint64_t generation;
    int64_t origin_module, origin_address;
} rxvm_invocation_boundary;

typedef enum rxvm_invoke_binding_kind {
    RXVM_INVOKE_BIND_CLASS, RXVM_INVOKE_BIND_MEMBER, RXVM_INVOKE_BIND_FUNCTION
} rxvm_invoke_binding_kind;
typedef struct rxvm_invoke_binding {
    rxvm_invoke_binding_kind kind;
    const char *symbol, *member_kind, *member, *type, *args;
} rxvm_invoke_binding;
extern const rxvm_invoke_binding rxvm_invoke_bindings[];
extern const size_t rxvm_invoke_binding_count;
void rxvm_invoke_init(void);
/* Loader-owned exact metadata/provider queries; no alternative policy here. */
const char *rxvm_invoke_metadata_string(struct module *, size_t);
int rxvm_invoke_provider_matches(struct module *, const char *, const char *);
int rxvm_invoke_is_task(struct module *, const char *);

struct proc_runtime *rxvm_invoke_target(rxvm_invocation *);
size_t rxvm_invoke_count(rxvm_invocation *);
struct value **rxvm_invoke_arguments(rxvm_invocation *);
struct value *rxvm_invoke_return(rxvm_invocation *);
void rxvm_invoke_origin(rxvm_invocation *, int64_t *, int64_t *);
void rxvm_invoke_complete(rxvm_invocation *, int returned_value);
void rxvm_invoke_fail(rxvm_invocation *, const struct value *event);
void rxvm_invoke_abandon(rxvm_invocation *);
void rxvm_invoke_unpin(rxvm_invocation *);
void rxvm_invoke_failure_event(struct value *, int, int64_t, int64_t, const struct value *);
#endif
