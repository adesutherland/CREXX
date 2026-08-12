/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxvmprogram.h"

#include "rxvmintp.h"

#include <limits.h>
#include <stdlib.h>

#if defined(_WIN32)
#include <windows.h>
typedef CRITICAL_SECTION rxvm_program_mutex;
static int rxvm_program_mutex_init(rxvm_program_mutex *mutex) {
    InitializeCriticalSection(mutex);
    return 1;
}
static void rxvm_program_mutex_destroy(rxvm_program_mutex *mutex) {
    DeleteCriticalSection(mutex);
}
static void rxvm_program_mutex_lock(rxvm_program_mutex *mutex) {
    EnterCriticalSection(mutex);
}
static void rxvm_program_mutex_unlock(rxvm_program_mutex *mutex) {
    LeaveCriticalSection(mutex);
}
#else
#include <pthread.h>
typedef pthread_mutex_t rxvm_program_mutex;
static int rxvm_program_mutex_init(rxvm_program_mutex *mutex) {
    return pthread_mutex_init(mutex, 0) == 0;
}
static void rxvm_program_mutex_destroy(rxvm_program_mutex *mutex) {
    if (pthread_mutex_destroy(mutex) != 0) abort();
}
static void rxvm_program_mutex_lock(rxvm_program_mutex *mutex) {
    if (pthread_mutex_lock(mutex) != 0) abort();
}
static void rxvm_program_mutex_unlock(rxvm_program_mutex *mutex) {
    if (pthread_mutex_unlock(mutex) != 0) abort();
}
#endif

typedef struct rxvm_program_catalogue rxvm_program_catalogue;

typedef struct rxvm_program_image {
    module_file *file;
    size_t generation_references;
} rxvm_program_image;

struct rxvm_program_generation {
    rxvm_program_catalogue *catalogue;
    rxvm_program_image **images;
    size_t image_count;
    size_t references; /* One runtime catalogue reference plus VM pins. */
    uint64_t id;
    struct rxvm_program_generation *next;
};

struct rxvm_program_catalogue {
    rxvm_runtime *runtime;
    rxvm_program_generation *generations;
    uint64_t next_generation_id;
    rxvm_program_mutex mutex;
};

static void rxvm_program_image_release(rxvm_program_image *image) {
    if (!image || !image->generation_references) abort();
    image->generation_references--;
    if (!image->generation_references) {
        free_module(image->file);
        image->file = 0;
        free(image);
    }
}

static void rxvm_program_generation_destroy(
        rxvm_program_generation *generation) {
    size_t i;

    if (!generation) return;
    for (i = 0u; i < generation->image_count; i++) {
        rxvm_program_image_release(generation->images[i]);
    }
    free(generation->images);
    free(generation);
}

static void rxvm_program_catalogue_destroy(void *state) {
    rxvm_program_catalogue *catalogue = (rxvm_program_catalogue *)state;
    rxvm_program_generation *generation;

    if (!catalogue) return;
    generation = catalogue->generations;
    while (generation) {
        rxvm_program_generation *next = generation->next;

        /* Every worker must release its generation pin before its runtime. */
        if (generation->references != 1u) abort();
        rxvm_program_generation_destroy(generation);
        generation = next;
    }
    rxvm_program_mutex_destroy(&catalogue->mutex);
    catalogue->runtime = 0;
    free(catalogue);
}

static rxvm_program_catalogue *rxvm_program_catalogue_create(
        rxvm_runtime *runtime) {
    rxvm_program_catalogue *catalogue;

    catalogue = (rxvm_program_catalogue *)calloc(1u, sizeof(*catalogue));
    if (!catalogue) return 0;
    if (!rxvm_program_mutex_init(&catalogue->mutex)) {
        free(catalogue);
        return 0;
    }
    catalogue->runtime = runtime;
    catalogue->next_generation_id = 1u;
    return catalogue;
}

static rxvm_program_catalogue *rxvm_program_catalogue_for_runtime(
        rxvm_runtime *runtime) {
    rxvm_program_catalogue *catalogue;
    rxvm_program_catalogue *candidate;

    catalogue = (rxvm_program_catalogue *)
            rxvm_runtime_program_state(runtime);
    if (catalogue) return catalogue;
    candidate = rxvm_program_catalogue_create(runtime);
    if (!candidate) return 0;
    if (rxvm_runtime_install_program_state(
            runtime, candidate, rxvm_program_catalogue_destroy)) {
        return candidate;
    }
    rxvm_program_catalogue_destroy(candidate);
    return (rxvm_program_catalogue *)rxvm_runtime_program_state(runtime);
}

static int rxvm_program_context_is_owner(const rxvm_context *context) {
    rxvm_worker_state state;

    if (!context || !rxvm_worker_is_current_thread_owner(&context->worker)) {
        return 0;
    }
    state = rxvm_worker_get_state(&context->worker);
    return state == RXVM_WORKER_IDLE || state == RXVM_WORKER_RUNNING;
}

static void rxvm_program_generation_unpin(
        rxvm_program_generation *generation) {
    rxvm_program_catalogue *catalogue;

    if (!generation) return;
    catalogue = generation->catalogue;
    rxvm_program_mutex_lock(&catalogue->mutex);
    if (generation->references <= 1u) abort();
    generation->references--;
    if (generation->references == 1u &&
        catalogue->generations != generation) {
        rxvm_program_generation **link = &catalogue->generations;
        while (*link && *link != generation) link = &(*link)->next;
        if (*link != generation) abort();
        *link = generation->next;
        generation->next = 0;
        rxvm_program_generation_destroy(generation);
    }
    rxvm_program_mutex_unlock(&catalogue->mutex);
}

static void rxvm_program_generation_pin(
        rxvm_program_generation *generation) {
    rxvm_program_catalogue *catalogue = generation->catalogue;

    rxvm_program_mutex_lock(&catalogue->mutex);
    if (!generation->references || generation->references == SIZE_MAX) abort();
    generation->references++;
    rxvm_program_mutex_unlock(&catalogue->mutex);
}

static void rxvm_program_discard_unpublished(
        rxvm_program_generation *generation) {
    size_t i;

    if (!generation) return;
    for (i = 0u; i < generation->image_count; i++) {
        rxvm_program_image *image = generation->images[i];
        /* A zero count identifies a wrapper not yet owning its module file. */
        if (image && !image->generation_references) free(image);
    }
    free(generation->images);
    free(generation);
}

rxvm_program_result rxvm_program_generation_seal(
        rxvm_context *context,
        const rxvm_program_generation **generation_out) {
    rxvm_program_generation *current;
    rxvm_program_generation *generation;
    rxvm_program_catalogue *catalogue;
    size_t prefix_count;
    size_t i;

    if (generation_out) *generation_out = 0;
    if (!context || !context->worker.runtime || !generation_out) {
        return RXVM_PROGRAM_INVALID;
    }
    if (!rxvm_program_context_is_owner(context)) {
        return RXVM_PROGRAM_WRONG_THREAD;
    }
    current = context->program_generation;
    prefix_count = current ? current->image_count : 0u;
    if (!context->num_modules || context->num_modules < prefix_count) {
        return RXVM_PROGRAM_INCOMPATIBLE;
    }
    if (current && current->catalogue->runtime != context->worker.runtime) {
        return RXVM_PROGRAM_WRONG_RUNTIME;
    }
    for (i = 0u; i < prefix_count; i++) {
        if (context->modules[i]->file != current->images[i]->file) {
            return RXVM_PROGRAM_INCOMPATIBLE;
        }
    }
    for (i = prefix_count; i < context->num_modules; i++) {
        module *mod = context->modules[i];
        if (!mod || !mod->file) return RXVM_PROGRAM_INVALID;
        if (mod->native || mod->file->native) {
            return RXVM_PROGRAM_NATIVE_EXCLUDED;
        }
    }
    if (current && prefix_count == context->num_modules) {
        *generation_out = current;
        return RXVM_PROGRAM_OK;
    }

    catalogue = rxvm_program_catalogue_for_runtime(context->worker.runtime);
    if (!catalogue) return RXVM_PROGRAM_OUT_OF_MEMORY;
    generation = (rxvm_program_generation *)calloc(1u, sizeof(*generation));
    if (!generation) return RXVM_PROGRAM_OUT_OF_MEMORY;
    generation->images = (rxvm_program_image **)calloc(
            context->num_modules, sizeof(*generation->images));
    if (!generation->images) {
        free(generation);
        return RXVM_PROGRAM_OUT_OF_MEMORY;
    }
    generation->catalogue = catalogue;
    generation->image_count = context->num_modules;

    for (i = 0u; i < prefix_count; i++) {
        generation->images[i] = current->images[i];
    }
    for (i = prefix_count; i < generation->image_count; i++) {
        rxvm_program_image *image =
                (rxvm_program_image *)calloc(1u, sizeof(*image));
        if (!image) {
            rxvm_program_discard_unpublished(generation);
            return RXVM_PROGRAM_OUT_OF_MEMORY;
        }
        image->file = context->modules[i]->file;
        generation->images[i] = image;
    }

    /* The complete descriptor is immutable before this publication lock. */
    rxvm_program_mutex_lock(&catalogue->mutex);
    if (!catalogue->next_generation_id) abort();
    generation->id = catalogue->next_generation_id++;
    generation->references = 2u; /* Runtime catalogue plus this context. */
    for (i = 0u; i < generation->image_count; i++) {
        if (generation->images[i]->generation_references == SIZE_MAX) abort();
        generation->images[i]->generation_references++;
    }
    generation->next = catalogue->generations;
    catalogue->generations = generation;
    rxvm_program_mutex_unlock(&catalogue->mutex);

    context->program_generation = generation;
    rxvm_program_generation_unpin(current);
    *generation_out = generation;
    return RXVM_PROGRAM_OK;
}

rxvm_program_result rxvm_program_generation_attach(
        rxvm_context *context,
        const rxvm_program_generation *generation_const) {
    rxvm_program_generation *generation =
            (rxvm_program_generation *)generation_const;
    rxvm_program_generation *current;
    size_t prefix_count;
    size_t i;

    if (!context || !generation || !context->worker.runtime) {
        return RXVM_PROGRAM_INVALID;
    }
    if (!rxvm_program_context_is_owner(context)) {
        return RXVM_PROGRAM_WRONG_THREAD;
    }
    if (generation->catalogue->runtime != context->worker.runtime) {
        return RXVM_PROGRAM_WRONG_RUNTIME;
    }
    current = context->program_generation;
    if (current == generation) return RXVM_PROGRAM_OK;
    prefix_count = current ? current->image_count : 0u;
    if ((!current && context->num_modules != 0u) ||
        (current && context->num_modules != prefix_count) ||
        generation->image_count < prefix_count) {
        return RXVM_PROGRAM_INCOMPATIBLE;
    }
    for (i = 0u; i < prefix_count; i++) {
        if (current->images[i] != generation->images[i] ||
            context->modules[i]->file != current->images[i]->file) {
            return RXVM_PROGRAM_INCOMPATIBLE;
        }
    }

    for (i = prefix_count; i < generation->image_count; i++) {
        rxvm_materialize_module_overlay(
                context, generation->images[i]->file);
    }
    rxvm_program_generation_pin(generation);
    context->program_generation = generation;
    rxvm_program_generation_unpin(current);
    return RXVM_PROGRAM_OK;
}

const rxvm_program_generation *rxvm_program_generation_current(
        const rxvm_context *context) {
    return context ? context->program_generation : 0;
}

uint64_t rxvm_program_generation_id(
        const rxvm_program_generation *generation) {
    return generation ? generation->id : 0u;
}

size_t rxvm_program_generation_module_count(
        const rxvm_program_generation *generation) {
    return generation ? generation->image_count : 0u;
}

static size_t rxvm_program_saturating_add(size_t total, size_t addition) {
    return addition > SIZE_MAX - total ? SIZE_MAX : total + addition;
}

size_t rxvm_program_generation_instruction_bytes(
        const rxvm_program_generation *generation) {
    size_t total = 0u;
    size_t i;

    if (!generation) return 0u;
    for (i = 0u; i < generation->image_count; i++) {
        const module_file *file = generation->images[i]->file;
        size_t j;

        for (j = 0u; j < i; j++) {
            if (generation->images[j]->file->instructions ==
                file->instructions) break;
        }
        if (j != i) continue;
        if (file->header.instruction_size > SIZE_MAX / sizeof(bin_code)) {
            return SIZE_MAX;
        }
        total = rxvm_program_saturating_add(
                total, file->header.instruction_size * sizeof(bin_code));
    }
    return total;
}

size_t rxvm_program_generation_constant_bytes(
        const rxvm_program_generation *generation) {
    size_t total = 0u;
    size_t i;

    if (!generation) return 0u;
    for (i = 0u; i < generation->image_count; i++) {
        const module_file *file = generation->images[i]->file;
        const void *identity = file->shared_constant_pool
                ? (const void *)file->shared_constant_pool
                : file->constant;
        size_t bytes = file->shared_constant_pool
                ? file->shared_constant_pool->size
                : file->header.constant_size;
        size_t j;

        for (j = 0u; j < i; j++) {
            const module_file *prior = generation->images[j]->file;
            const void *prior_identity = prior->shared_constant_pool
                    ? (const void *)prior->shared_constant_pool
                    : prior->constant;
            if (prior_identity == identity) break;
        }
        if (j == i) total = rxvm_program_saturating_add(total, bytes);
    }
    return total;
}

void rxvm_program_generation_release_context(rxvm_context *context) {
    rxvm_program_generation *generation;

    if (!context) return;
    generation = context->program_generation;
    context->program_generation = 0;
    rxvm_program_generation_unpin(generation);
}

int rxvm_program_generation_owns_module(
        const rxvm_context *context,
        size_t module_index) {
    const rxvm_program_generation *generation;

    if (!context) return 0;
    generation = context->program_generation;
    return generation && module_index < generation->image_count &&
           context->modules[module_index]->file ==
               generation->images[module_index]->file;
}
