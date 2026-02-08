#ifndef CREXX_RXVM_H
#define CREXX_RXVM_H

#include <stddef.h>

/* Forward declarations for opaque handles */
struct rxvm_context;
struct module;

/* --- Phase 1: Loading --- */
/* Loads a module from a .rxbin file. Returns module handle or NULL on error. */
struct module* rxvm_load(struct rxvm_context* ctx, char* filename);

/* --- Phase 2: Linking --- */
/* Resolves imports/exports across all loaded modules in the context. */
/* Returns 0 on success, non-zero on error. */
int rxvm_link(struct rxvm_context* ctx);

/* --- Phase 3: Preparation (Threading) --- */
/* Performs Opcode-to-Pointer transformation for threaded execution. */
/* This operation is destructive but idempotent (guarded by state flags). */
/* Returns 0 on success, non-zero on error. */
int rxvm_prepare(struct rxvm_context* ctx);

/* --- Phase 4: Execution --- */
/* Executes a specific procedure by name (usually "main") within the context. */
/* Returns the return code from the procedure. */
int rxvm_call(struct rxvm_context* ctx, char* proc_name, int argc, char** argv);

/* --- Backward Compatibility --- */
/* Wrapper that performs Link -> Prepare -> Call("main"). */
int rxvm_run(struct rxvm_context* ctx, int argc, char** argv);

#endif /* CREXX_RXVM_H */
