/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXVMPROGRAM_H
#define CREXX_RXVMPROGRAM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rxvm_program_generation rxvm_program_generation;
struct rxvm_context;
struct module_file;

typedef enum rxvm_program_result {
    RXVM_PROGRAM_OK = 0,
    RXVM_PROGRAM_INVALID = 1,
    RXVM_PROGRAM_WRONG_THREAD = 2,
    RXVM_PROGRAM_WRONG_RUNTIME = 3,
    RXVM_PROGRAM_INCOMPATIBLE = 4,
    RXVM_PROGRAM_NATIVE_EXCLUDED = 5,
    RXVM_PROGRAM_OUT_OF_MEMORY = 6
} rxvm_program_result;

/*
 * Publish the context's complete bytecode module prefix as a new immutable
 * generation.  Existing generation-owned images are retained; newly loaded
 * bytecode images are adopted only after the complete generation is ready.
 */
rxvm_program_result rxvm_program_generation_seal(
        struct rxvm_context *context,
        const rxvm_program_generation **generation_out);

/*
 * Attach an empty context, or advance a compatible context to an append-only
 * derived generation.  Every mutable runtime structure is materialized in the
 * attaching worker's allocator.
 */
rxvm_program_result rxvm_program_generation_attach(
        struct rxvm_context *context,
        const rxvm_program_generation *generation);

const rxvm_program_generation *rxvm_program_generation_current(
        const struct rxvm_context *context);
uint64_t rxvm_program_generation_id(
        const rxvm_program_generation *generation);
size_t rxvm_program_generation_module_count(
        const rxvm_program_generation *generation);
size_t rxvm_program_generation_instruction_bytes(
        const rxvm_program_generation *generation);
size_t rxvm_program_generation_constant_bytes(
        const rxvm_program_generation *generation);

/* Context teardown hook; generation storage remains runtime-owned. */
void rxvm_program_generation_release_context(struct rxvm_context *context);
int rxvm_program_generation_owns_module(
        const struct rxvm_context *context,
        size_t module_index);

/* Loader hook used to create a worker overlay over one immutable image. */
size_t rxvm_materialize_module_overlay(
        struct rxvm_context *context,
        struct module_file *file_module_section);

#ifdef __cplusplus
}
#endif

#endif
