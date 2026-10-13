/*
 * cREXX License (MIT)
 * Copyright (c) 2026 the cREXX contributors
 */

#ifndef CREXX_RXVMLINKINDEX_H
#define CREXX_RXVMLINKINDEX_H

#include "rxvmintp.h"

/* Private, optional lookup acceleration for one compatibility-registry rebuild.
 * Metadata and procedure pointers are borrowed from the explicitly named context.
 * No index is retained by the context, module, execution image or current thread.
 */
typedef struct rxvm_link_index rxvm_link_index;

typedef struct rxvm_link_member_cursor {
    rxvm_context *context;
    const rxvm_link_index *index;
    const char *owner;
    size_t owner_length;
    size_t module_index;
    int meta_index;
    size_t member_index;
} rxvm_link_member_cursor;

/* Failure releases every partial allocation; callers use the NULL-index path. */
rxvm_link_index *rxvm_link_index_build(rxvm_context *context);
void rxvm_link_index_free(rxvm_link_index *index);

/* NULL or foreign-context indexes always use the original ordered metadata walk. */
proc_runtime *rxvm_link_index_resolve_procedure(rxvm_context *context,
                                               const rxvm_link_index *index,
                                               const char *name,
                                               size_t name_length);
meta_func_constant *rxvm_link_index_find_function(rxvm_context *context,
                                                 const rxvm_link_index *index,
                                                 proc_runtime *procedure,
                                                 module **function_module);
int rxvm_link_index_class_implements(rxvm_context *context,
                                     const rxvm_link_index *index,
                                     const char *class_name,
                                     size_t class_name_length,
                                     const char *interface_name,
                                     size_t interface_name_length);

void rxvm_link_member_cursor_init(rxvm_link_member_cursor *cursor,
                                  rxvm_context *context,
                                  const rxvm_link_index *index,
                                  const char *owner,
                                  size_t owner_length);
meta_entry *rxvm_link_member_cursor_next(rxvm_link_member_cursor *cursor,
                                        module **member_module);

#endif
