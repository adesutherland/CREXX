/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//
// Decimal Plugin Framework Implementation
//
// Created by Adrian Sutherland on 20/09/2024.
//
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#define RXVMPLUGIN_LOCK() AcquireSRWLockExclusive(&rxvmplugin_catalogue_lock)
#define RXVMPLUGIN_UNLOCK() ReleaseSRWLockExclusive(&rxvmplugin_catalogue_lock)
#else
#include <dlfcn.h> // Linux/OSX
#include <pthread.h>
#include <unistd.h>
#define RXVMPLUGIN_LOCK() ((void)pthread_mutex_lock(&rxvmplugin_catalogue_lock))
#define RXVMPLUGIN_UNLOCK() ((void)pthread_mutex_unlock(&rxvmplugin_catalogue_lock))
#endif
#include "rxvmplugin_framework.h"
#include "platform.h"
#include "rxbin.h"
#include "rxvmvars.h"

struct rxvmplugin_library {
    void *handle;
    size_t descriptor_references;
    unsigned long generation;
};

typedef struct rxvmplugin_pending_registration rxvmplugin_pending_registration;
struct rxvmplugin_pending_registration {
    char name[16];
    rxvm_plugin_factory factory;
    rxvmplugin_pending_registration *next;
};

typedef struct rxvmplugin_load_transaction {
    rxvmplugin_pending_registration *head;
    rxvmplugin_pending_registration *tail;
    int failed;
} rxvmplugin_load_transaction;

#if defined(_MSC_VER)
#define RXVMPLUGIN_THREAD_LOCAL __declspec(thread)
#else
#define RXVMPLUGIN_THREAD_LOCAL __thread
#endif

#ifdef _WIN32
static SRWLOCK rxvmplugin_catalogue_lock = SRWLOCK_INIT;
#else
static pthread_mutex_t rxvmplugin_catalogue_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

/* Published descriptors are immutable until clear detaches them. */
static rxvmplugin_factory_entry *rxvmplugin_factories = 0;
static rxvmplugin_factory_entry *rxvmplugin_retired_factories = 0;
static unsigned long rxvmplugin_next_generation = 1ul;
static size_t rxvmplugin_live_instances = 0u;

/*
 * The installed dynamic callback has no userdata parameter.  Adapt it to an
 * explicit per-thread load transaction; the library handle is attached only
 * during atomic publication and is never ambient registration state.
 */
static RXVMPLUGIN_THREAD_LOCAL rxvmplugin_load_transaction
        *rxvmplugin_current_load_transaction;

static void *rxvmplugin_memory_alloc(size_t size) {
    return rxvm_memory_alloc_bytes(rxvm_memory_current_worker(), size);
}

static char *rxvmplugin_memory_strdup(const char *text) {
    size_t length;
    char *copy;
    if (!text) text = "";
    length = strlen(text);
    copy = rxvmplugin_memory_alloc(length + 1u);
    if (copy) memcpy(copy, text, length + 1u);
    return copy;
}

static void rxvmplugin_memory_free(void *pointer) {
    rxvm_memory_worker *previous;
    if (!pointer) return;
    previous = rxvm_memory_enter(rxvm_memory_owner(pointer));
    (void)rxvm_memory_release(pointer);
    rxvm_memory_leave(previous);
}

static void *rxvmplugin_reserve_string(value *string, size_t size) {
    if (!string) return 0;
    prep_string_buffer(string, size);
    if (!string->string_value) return 0;
    rxvm_value_set_string_length_known(string, 0u);
    string->string_value[0] = 0;
    string_cache_reset(string);
    return string->string_value;
}

static void rxvmplugin_release_value_storage(value *value_storage) {
    clear_value(value_storage);
}

static int rxvmplugin_configure_instance(rxvm_plugin *plugin) {
    if (!plugin || !plugin->free ||
        plugin->type <= RXVM_PLUGIN_UNDEFINED ||
        plugin->type >= RXVM_PLUGIN_MAX) {
        return -1;
    }
    if (plugin->type == RXVM_PLUGIN_DECIMAL) {
        decplugin *decimal = (decplugin *)plugin;
        decimal->number_to_simple_format = number_to_simple_format;
        decimal->format_number_components = RexxDecimalFormat;
        decimal->reserve_decimal = rxvm_value_reserve_decimal;
        decimal->reserve_string = rxvmplugin_reserve_string;
        decimal->release_value_storage = rxvmplugin_release_value_storage;
        decimal->num_context = NULL;
    }
    return 0;
}

static void rxvmplugin_close_library(void *handle) {
    if (!handle) return;
#ifdef _WIN32
    FreeLibrary((HMODULE)handle);
#else
    dlclose(handle);
#endif
}

static rxvmplugin_factory_entry *rxvmplugin_build_descriptor(
        const char *factory_name,
        rxvm_plugin_factory factory,
        rxvmplugin_library *library) {
    rxvmplugin_factory_entry *entry;
    rxvm_plugin *compatibility_instance;

    if (!factory_name || !factory) return NULL;
    compatibility_instance = factory();
    if (rxvmplugin_configure_instance(compatibility_instance) != 0) {
        if (compatibility_instance && compatibility_instance->free) {
            compatibility_instance->free(compatibility_instance);
        }
        return NULL;
    }

    entry = (rxvmplugin_factory_entry *)calloc(1, sizeof(*entry));
    if (!entry) {
        compatibility_instance->free(compatibility_instance);
        return NULL;
    }
    strncpy(entry->name, factory_name, sizeof(entry->name) - 1u);
    entry->name[sizeof(entry->name) - 1u] = '\0';
    entry->plugin_info = compatibility_instance;
    entry->factory = factory;
    entry->handle = library ? library->handle : NULL;
    entry->type = compatibility_instance->type;
    entry->library = library;
    return entry;
}

static void rxvmplugin_destroy_unpublished_descriptor(
        rxvmplugin_factory_entry *entry) {
    if (!entry) return;
    if (entry->plugin_info) entry->plugin_info->free(entry->plugin_info);
    free(entry);
}

static rxvmplugin_factory_entry *rxvmplugin_find_exact_locked(
        const char *name,
        rxvm_plugin_factory factory) {
    rxvmplugin_factory_entry *entry = rxvmplugin_factories;
    while (entry) {
        if (entry->factory == factory && strcmp(entry->name, name) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

static void rxvmplugin_destroy_descriptor(rxvmplugin_factory_entry *entry) {
    rxvmplugin_library *library;
    void *handle_to_close = NULL;

    if (!entry) return;
    if (entry->plugin_info) entry->plugin_info->free(entry->plugin_info);
    library = entry->library;
    if (library) {
        RXVMPLUGIN_LOCK();
        if (!library->descriptor_references) abort();
        library->descriptor_references--;
        if (!library->descriptor_references) {
            handle_to_close = library->handle;
            free(library);
        }
        RXVMPLUGIN_UNLOCK();
    }
    free(entry);
    rxvmplugin_close_library(handle_to_close);
}

static void rxvmplugin_release_descriptor_instance(
        rxvmplugin_factory_entry *descriptor) {
    rxvmplugin_factory_entry **cursor;
    int destroy = 0;

    if (!descriptor) return;
    RXVMPLUGIN_LOCK();
    if (!descriptor->instance_references || !rxvmplugin_live_instances) abort();
    descriptor->instance_references--;
    rxvmplugin_live_instances--;
    if (descriptor->retired && !descriptor->instance_references) {
        cursor = &rxvmplugin_retired_factories;
        while (*cursor && *cursor != descriptor) {
            cursor = &(*cursor)->retired_next;
        }
        if (*cursor == descriptor) {
            *cursor = descriptor->retired_next;
            destroy = 1;
        }
    }
    RXVMPLUGIN_UNLOCK();
    if (destroy) rxvmplugin_destroy_descriptor(descriptor);
}

static void rxvmplugin_pending_free(rxvmplugin_pending_registration *entry) {
    while (entry) {
        rxvmplugin_pending_registration *next = entry->next;
        free(entry);
        entry = next;
    }
}

static void rxvmplugin_transaction_register(
        char *factory_name,
        rxvm_plugin_factory factory) {
    rxvmplugin_load_transaction *transaction =
            rxvmplugin_current_load_transaction;
    rxvmplugin_pending_registration *pending;

    if (!transaction || !factory_name || !factory) {
        if (transaction) transaction->failed = 1;
        return;
    }
    pending = (rxvmplugin_pending_registration *)calloc(1, sizeof(*pending));
    if (!pending) {
        transaction->failed = 1;
        return;
    }
    strncpy(pending->name, factory_name, sizeof(pending->name) - 1u);
    pending->name[sizeof(pending->name) - 1u] = '\0';
    pending->factory = factory;
    if (transaction->tail) transaction->tail->next = pending;
    else transaction->head = pending;
    transaction->tail = pending;
}

static int rxvmplugin_commit_dynamic_transaction(
        rxvmplugin_load_transaction *transaction,
        void *handle) {
    rxvmplugin_library *library;
    rxvmplugin_pending_registration *pending;
    rxvmplugin_factory_entry *prepared_head = NULL;
    rxvmplugin_factory_entry *prepared_tail = NULL;
    rxvmplugin_factory_entry *discarded = NULL;
    size_t published = 0u;

    if (!transaction || transaction->failed || !transaction->head) return -1;
    library = (rxvmplugin_library *)calloc(1, sizeof(*library));
    if (!library) return -1;
    library->handle = handle;

    pending = transaction->head;
    while (pending) {
        rxvmplugin_factory_entry *entry = rxvmplugin_build_descriptor(
                pending->name, pending->factory, library);
        if (!entry) {
            while (prepared_head) {
                rxvmplugin_factory_entry *next = prepared_head->next;
                rxvmplugin_destroy_unpublished_descriptor(prepared_head);
                prepared_head = next;
            }
            free(library);
            return -1;
        }
        if (prepared_tail) prepared_tail->next = entry;
        else prepared_head = entry;
        prepared_tail = entry;
        pending = pending->next;
    }

    RXVMPLUGIN_LOCK();
    library->generation = rxvmplugin_next_generation++;
    while (prepared_head) {
        rxvmplugin_factory_entry *entry = prepared_head;
        prepared_head = prepared_head->next;
        entry->next = NULL;
        if (rxvmplugin_find_exact_locked(entry->name, entry->factory)) {
            entry->next = discarded;
            discarded = entry;
            continue;
        }
        entry->generation = library->generation;
        entry->next = rxvmplugin_factories;
        rxvmplugin_factories = entry;
        library->descriptor_references++;
        published++;
    }
    RXVMPLUGIN_UNLOCK();

    while (discarded) {
        rxvmplugin_factory_entry *next = discarded->next;
        rxvmplugin_destroy_unpublished_descriptor(discarded);
        discarded = next;
    }
    if (!published) {
        free(library);
        return 1;
    }
    return 0;
}

/* Function to load a dynamic plugin */
int load_rxvmplugin(char* dir, char *name) {
    int rc = 0;
    int commit_result;
    char *file_name;
    char *full_file_name = NULL;
    char *exe_dir = NULL;
    char *dir_copy = NULL;
    char *token;
    char *next_token;
    char *combined_dir = NULL;
    void *loaded_handle = NULL;
    rxvmplugin_register_function init = NULL;
    rxvmplugin_load_transaction transaction;

    if (!name || rxvmplugin_current_load_transaction) return -3;
    memset(&transaction, 0, sizeof(transaction));

    /* Create the filename by appending ".rxvmplugin" to the file name */
    file_name = rxvmplugin_memory_alloc(
        strlen(name) + strlen(".rxvmplugin") + 1u);
    if (!file_name) {
        RX_REPORT_OOM("malloc rxvmplugin file name",
                      strlen(name) + strlen(".rxvmplugin") + 1, name);
        return -1;
    }
    sprintf(file_name, "%s.rxvmplugin", name);

    exe_dir = exepath();
    if (dir) {
        combined_dir = rxvmplugin_memory_alloc(
            strlen(dir) + strlen(exe_dir) + 2u);
        if (!combined_dir) {
            RX_REPORT_OOM("malloc rxvmplugin search path",
                          strlen(dir) + strlen(exe_dir) + 2, name);
            free(exe_dir);
            rxvmplugin_memory_free(file_name);
            return -1;
        }
        sprintf(combined_dir, "%s;%s", dir, exe_dir);
    } else {
        combined_dir = rxvmplugin_memory_strdup(exe_dir);
        if (!combined_dir) {
            RX_REPORT_OOM("strdup rxvmplugin search path", strlen(exe_dir) + 1, name);
            free(exe_dir);
            rxvmplugin_memory_free(file_name);
            return -1;
        }
    }
    free(exe_dir);

    dir_copy = rxvmplugin_memory_strdup(combined_dir);
    if (!dir_copy) {
        RX_REPORT_OOM("strdup rxvmplugin directory iterator",
                      strlen(combined_dir) + 1, name);
        rxvmplugin_memory_free(combined_dir);
        rxvmplugin_memory_free(file_name);
        return -1;
    }
    token = dir_copy;
    while (token) {
        next_token = strchr(token, ';');
        if (next_token) *next_token = 0;

        rxvmplugin_memory_free(full_file_name);
        full_file_name = rxvmplugin_memory_alloc(
            strlen(token) + strlen(file_name) + 2u);
        if (!full_file_name) {
            RX_REPORT_OOM("malloc rxvmplugin full path",
                          strlen(token) + strlen(file_name) + 2, name);
            rxvmplugin_memory_free(dir_copy);
            rxvmplugin_memory_free(combined_dir);
            rxvmplugin_memory_free(file_name);
            return -1;
        }
        if (full_file_name) {
#ifdef _WIN32
            sprintf(full_file_name, "%s\\%s", token, file_name);
#else
            sprintf(full_file_name, "%s/%s", token, file_name);
#endif
        }

        if (full_file_name) {
#ifdef _WIN32
            DWORD dwAttrib = GetFileAttributes(full_file_name);
            if (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) break;
#else
            if (access(full_file_name, F_OK) == 0) break;
#endif
        }

        token = next_token ? next_token + 1 : 0;
    }
    rxvmplugin_memory_free(dir_copy);
    rxvmplugin_memory_free(combined_dir);

    if (!token) {
        /* Not found in any directory, try one last time with bare filename */
        rxvmplugin_memory_free(full_file_name);
        full_file_name = rxvmplugin_memory_strdup(file_name);
        if (!full_file_name) {
            RX_REPORT_OOM("strdup rxvmplugin fallback path", strlen(file_name) + 1, name);
        }
    }

    if (!full_file_name) {
        rxvmplugin_memory_free(file_name);
        return -1;
    }

#ifdef _WIN32
    loaded_handle = (void *)LoadLibraryA(full_file_name);
    if (loaded_handle) {
        init = (rxvmplugin_register_function)GetProcAddress(
                (HMODULE)loaded_handle, "_register_rxvm_plugin");
    }
#else
    loaded_handle = dlopen(full_file_name, RTLD_LAZY);
    if (loaded_handle) {
        init = (rxvmplugin_register_function)dlsym(
                loaded_handle, "_register_rxvm_plugin");
    }
#endif

    if (!loaded_handle) {
        rc = -1;
    } else if (!init) {
        rxvmplugin_close_library(loaded_handle);
        rc = -2;
    } else {
        rxvmplugin_current_load_transaction = &transaction;
        init(rxvmplugin_transaction_register);
        rxvmplugin_current_load_transaction = NULL;
        commit_result = rxvmplugin_commit_dynamic_transaction(
                &transaction, loaded_handle);
        if (commit_result != 0) {
            rxvmplugin_close_library(loaded_handle);
            rc = commit_result < 0 ? -3 : 0;
        }
    }

    rxvmplugin_pending_free(transaction.head);
    rxvmplugin_memory_free(full_file_name);
    rxvmplugin_memory_free(file_name);
    return rc;
}

/* Function to register a plugin factory */
void register_rxvmplugin(char* factory_name, rxvm_plugin_factory factory) {
    rxvmplugin_factory_entry *entry;
    int duplicate;

    entry = rxvmplugin_build_descriptor(factory_name, factory, NULL);
    if (!entry) {
        fprintf(stderr, "Unable to register RXVM plugin factory %s\n",
                factory_name ? factory_name : "(null)");
        return;
    }

    RXVMPLUGIN_LOCK();
    duplicate = rxvmplugin_find_exact_locked(entry->name, entry->factory) != NULL;
    if (!duplicate) {
        entry->generation = rxvmplugin_next_generation++;
        entry->next = rxvmplugin_factories;
        rxvmplugin_factories = entry;
    }
    RXVMPLUGIN_UNLOCK();
    if (duplicate) rxvmplugin_destroy_unpublished_descriptor(entry);
}

/* Detach the active catalogue.  Live VM instances retain their descriptor and
 * dynamic-library generation until the final context releases it. */
void clear_rxvmplugin_factories(void) {
    rxvmplugin_factory_entry *entry;
    rxvmplugin_factory_entry *destroy_head = NULL;

    RXVMPLUGIN_LOCK();
    entry = rxvmplugin_factories;
    rxvmplugin_factories = NULL;
    while (entry) {
        rxvmplugin_factory_entry *next = entry->next;
        entry->next = NULL;
        entry->retired = 1u;
        if (entry->instance_references) {
            entry->retired_next = rxvmplugin_retired_factories;
            rxvmplugin_retired_factories = entry;
        } else {
            entry->retired_next = destroy_head;
            destroy_head = entry;
        }
        entry = next;
    }
    RXVMPLUGIN_UNLOCK();

    while (destroy_head) {
        rxvmplugin_factory_entry *next = destroy_head->retired_next;
        rxvmplugin_destroy_descriptor(destroy_head);
        destroy_head = next;
    }
}

/* Legacy compatibility lookup.  Returned instances remain process-owned and
 * are not used by VM execution. */
rxvm_plugin* find_rxvmplugin(char *name, rxvm_plugin_type type) {
    rxvmplugin_factory_entry *entry;
    rxvm_plugin *result = NULL;

    RXVMPLUGIN_LOCK();
    entry = rxvmplugin_factories;
    while (entry) {
        if (strcmp(entry->name, name) == 0 && entry->type == type) {
            result = entry->plugin_info;
            break;
        }
        entry = entry->next;
    }
    RXVMPLUGIN_UNLOCK();
    return result;
}

rxvm_plugin* get_rxvmplugin(rxvm_plugin_type type) {
    rxvmplugin_factory_entry *entry;
    rxvm_plugin *result = NULL;

    RXVMPLUGIN_LOCK();
    entry = rxvmplugin_factories;
    while (entry) {
        if (entry->type == type) {
            result = entry->plugin_info;
            break;
        }
        entry = entry->next;
    }
    RXVMPLUGIN_UNLOCK();
    return result;
}

rxvm_plugin* get_next_rxvmplugin(rxvmplugin_factory_entry **entry, rxvm_plugin_type type) {
    rxvm_plugin *result = NULL;

    if (!entry) return NULL;
    RXVMPLUGIN_LOCK();
    if (!*entry) {
        *entry = rxvmplugin_factories;
    } else {
        *entry = (*entry)->next;
    }
    while (*entry) {
        if ((*entry)->type == type) {
            result = (*entry)->plugin_info;
            break;
        }
        *entry = (*entry)->next;
    }
    RXVMPLUGIN_UNLOCK();
    return result;
}

void rxvmplugin_instance_set_init(rxvmplugin_instance_set *set) {
    if (set) memset(set, 0, sizeof(*set));
}

int rxvmplugin_instance_set_prepare(rxvmplugin_instance_set *set,
                                    rxvm_plugin_type type) {
    rxvmplugin_factory_entry *descriptor;
    rxvm_plugin *plugin;

    if (!set || type <= RXVM_PLUGIN_UNDEFINED || type >= RXVM_PLUGIN_MAX) {
        return -1;
    }
    if (set->entries[type].plugin) return 0;

    RXVMPLUGIN_LOCK();
    descriptor = rxvmplugin_factories;
    while (descriptor && descriptor->type != type) descriptor = descriptor->next;
    if (descriptor) {
        descriptor->instance_references++;
        rxvmplugin_live_instances++;
    }
    RXVMPLUGIN_UNLOCK();
    if (!descriptor) return -1;

    plugin = descriptor->factory();
    if (rxvmplugin_configure_instance(plugin) != 0 || plugin->type != type) {
        if (plugin && plugin->free) plugin->free(plugin);
        rxvmplugin_release_descriptor_instance(descriptor);
        return -1;
    }
    set->entries[type].plugin = plugin;
    set->entries[type].descriptor = descriptor;
    return 0;
}

rxvm_plugin *rxvmplugin_instance_set_get(rxvmplugin_instance_set *set,
                                         rxvm_plugin_type type) {
    if (!set || type <= RXVM_PLUGIN_UNDEFINED || type >= RXVM_PLUGIN_MAX) {
        return NULL;
    }
    return set->entries[type].plugin;
}

void rxvmplugin_instance_set_destroy(rxvmplugin_instance_set *set) {
    int type;

    if (!set) return;
    for (type = RXVM_PLUGIN_UNDEFINED + 1; type < RXVM_PLUGIN_MAX; type++) {
        rxvm_plugin *plugin = set->entries[type].plugin;
        rxvmplugin_factory_entry *descriptor =
                set->entries[type].descriptor;
        set->entries[type].plugin = NULL;
        set->entries[type].descriptor = NULL;
        if (plugin) plugin->free(plugin);
        if (descriptor) rxvmplugin_release_descriptor_instance(descriptor);
    }
}

size_t rxvmplugin_catalogue_count(void) {
    rxvmplugin_factory_entry *entry;
    size_t count = 0u;

    RXVMPLUGIN_LOCK();
    entry = rxvmplugin_factories;
    while (entry) {
        count++;
        entry = entry->next;
    }
    RXVMPLUGIN_UNLOCK();
    return count;
}

size_t rxvmplugin_live_instance_count(void) {
    size_t count;
    RXVMPLUGIN_LOCK();
    count = rxvmplugin_live_instances;
    RXVMPLUGIN_UNLOCK();
    return count;
}

#define MANTISSA_BUFFER_LEN 100
/* Helper function for decimal plugins to reformat a number string (should be valid, errors not checked)
 * to remove the exponent, to make the number in simple format */
void number_to_simple_format(const char *input, char *output) {
    const char *p = input;
    char mantissa_buffer[MANTISSA_BUFFER_LEN]; // If a longer buffer is needed, it will be dynamically allocated

    // Optional sign
    int sign = 0;
    if (*p == '-') {
        sign = 1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    // Extract the mantissa and exponent parts
    // Normalised scientific format assumed: e.g. "1.234E+3" or "1E-2".
    // Find 'E' or 'e'
    const char *ePos = strchr(p, 'E');
    if (!ePos) ePos = strchr(p, 'e');

    // If no exponent found, just copy input (it should be normalised anyway)
    // but as per instructions, normal form has exponent. If not, just return copy.
    int hasExponent = (ePos != NULL);

    char *mantissa = NULL;
    int exponentValue = 0;

    if (!hasExponent) {
        // No exponent - just return a copy with sign if needed
        strcpy(output, input);
        return;
    }

    // Extract exponent
    exponentValue = atoi(ePos + 1); // from after 'E' NOLINT

    // Extract mantissa (digits and decimal point)
    size_t mantLen = (size_t)(ePos - p);
    if (mantLen >= MANTISSA_BUFFER_LEN) {
        mantissa = (char *)rxvmplugin_memory_alloc(mantLen + 1u);
        if (!mantissa) {
            RX_PANIC_OOM("malloc rxvmplugin deExponify mantissa", mantLen + 1, p);
        }
    } else {
        mantissa = mantissa_buffer;
    }
    strncpy(mantissa, p, mantLen);
    mantissa[mantLen] = '\0';

    // Mantissa is something like "1.234"
    // Remove decimal point but remember its position
    char *dotPos = strchr(mantissa, '.');
    int dotIndex;
    int mantissaLenNoDot;
    if (dotPos) {
        dotIndex = (int)(dotPos - mantissa);
        // Remove the dot by shifting chars left
        char *m;
        for (m = dotPos; *m; m++) {
            *m = *(m+1);
        }
        mantissaLenNoDot = (int)strlen(mantissa);
    } else {
        // No dot means all are integer digits
        dotIndex = (int)strlen(mantissa);
        mantissaLenNoDot = (int)strlen(mantissa);
    }

    // Now, mantissa is pure digits, and we know the original decimal point
    // was right after dotIndex digits from the left.

    // We need to shift the decimal point according to exponentValue.
    // New decimal point position = dotIndex + exponentValue
    int newDotPos = dotIndex + exponentValue;

    // If newDotPos <= 0, we need leading zeros:
    // e.g. 1.234E-3 => newDotPos = 1-3=-2 => "0.00...1234"
    // If newDotPos >= length of digits, need trailing zeros
    // e.g. 1.234E+3 => newDotPos = 1+3=4 => "1234"
    // or if newDotPos is in the middle, just place the decimal point there.

    // Determine final length
    // Worst case: if very negative exponent, need many leading zeros
    int digitsCount = mantissaLenNoDot;
    int leadingZeros = 0;
    int trailingZeros = 0;

    if (newDotPos <= 0) {
        // Need (-newDotPos) leading zeros before first digit
        leadingZeros = -newDotPos;
    } else if (newDotPos >= digitsCount) {
        // Need (newDotPos - digitsCount) trailing zeros
        trailingZeros = newDotPos - digitsCount;
    }

    // Now determine if we need a decimal point at all
    // If newDotPos <= 0, decimal point will be after "0."
    // If newDotPos > 0 and newDotPos < digitsCount, decimal point inside digits
    // If newDotPos >= digitsCount, no decimal point needed
    int needDecimal = (newDotPos < digitsCount && newDotPos > 0) || (newDotPos <= 0 && digitsCount > 0);

    // Calculate output length
    //int totalLength = sign + leadingZeros + digitsCount + trailingZeros + (needDecimal ? 1 : 0);
    if (digitsCount == 0) {
        // If no digits, just produce something minimal, like "0"
        //totalLength = sign + 1;
        needDecimal = 0; // no decimal point if no digits
    }

    char *o = output;

    // Add sign if needed
    if (sign) {
        *o++ = '-';
    }

    if (digitsCount == 0) {
        // Just "0"
        *o++ = '0';
        *o = '\0';
        if (mantissa != mantissa_buffer) rxvmplugin_memory_free(mantissa);
    }

    // If newDotPos <= 0, output "0." followed by (-newDotPos-1) zeros, then digits
    if (newDotPos <= 0) {
        // put "0."
        *o++ = '0';
        if (needDecimal) {
            *o++ = '.';
        }
        // put leading zeros: (-newDotPos) = leadingZeros
        int i;
        for (i = 0; i < leadingZeros; i++) {
            *o++ = '0';
        }
        // then digits
        memcpy(o, mantissa, (size_t)digitsCount);
        o += digitsCount;
    } else if (newDotPos >= digitsCount) {
        // all digits first
        memcpy(o, mantissa, (size_t)digitsCount);
        o += digitsCount;
        // trailing zeros
        int i;
        for (i = 0; i < trailingZeros; i++) {
            *o++ = '0';
        }
        // no decimal point needed at the end
    } else {
        // decimal point in the middle of the digits
        // first newDotPos digits
        memcpy(o, mantissa, (size_t)newDotPos);
        o += newDotPos;
        if (needDecimal) {
            *o++ = '.';
        }
        // rest of the digits
        memcpy(o, mantissa + newDotPos, (size_t)(digitsCount - newDotPos));
        o += (digitsCount - newDotPos);
    }

    *o = '\0';
    if (mantissa != mantissa_buffer) rxvmplugin_memory_free(mantissa);

    // Remove trailing zeros after the decimal point (and the decimal point if it's the last character)
    // Find decimal point
    char *dot = strchr(output, '.');
    if (dot) {
        // Trim trailing zeros after decimal point
        char *end = output + strlen(output) - 1;
        while (end > dot && *end == '0') {
            *end = '\0';
            end--;
        }

        // If the decimal point is now the last character, remove it
        if (*end == '.') *end = '\0';
    }

    // If all digits are zeros, replace with just "0" (or "-0" if sign was set)
    const char *check = output + (sign ? 1 : 0);
    int allZero = 1;
    const char *c;
    for (c = check; *c; c++) {
        if (*c != '0') {
            allZero = 0;
            break;
        }
    }
    if (allZero) {
        // Replace it with just "0" (or "-0" if sign was set)
        if (sign) {
            strcpy(output+1, "0");
        } else {
            strcpy(output, "0");
        }
    }
}
