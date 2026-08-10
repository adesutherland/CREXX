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
// CREXX Plugin Architecture Library
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h> // Linux/OSX
#include <pthread.h>
#endif
#include "rxpa.h"

typedef void (*initfuncs_type)(rxpa_initctxptr);

#ifdef _WIN32
static SRWLOCK rxpa_loader_lock = SRWLOCK_INIT;
#define RXPA_LOADER_LOCK() AcquireSRWLockExclusive(&rxpa_loader_lock)
#define RXPA_LOADER_UNLOCK() ReleaseSRWLockExclusive(&rxpa_loader_lock)
#else
static pthread_mutex_t rxpa_loader_lock = PTHREAD_MUTEX_INITIALIZER;
#define RXPA_LOADER_LOCK() ((void)pthread_mutex_lock(&rxpa_loader_lock))
#define RXPA_LOADER_UNLOCK() ((void)pthread_mutex_unlock(&rxpa_loader_lock))
#endif

static size_t rxpa_live_handles;

static void rxpa_os_close(void *handle) {
    if (!handle) return;
#ifdef _WIN32
    FreeLibrary((HMODULE)handle);
#else
    dlclose(handle);
#endif
}

static void *rxpa_os_symbol(void *handle, const char *name) {
#ifdef _WIN32
    return (void *)GetProcAddress((HMODULE)handle, name);
#else
    return dlsym(handle, name);
#endif
}

static uint32_t rxpa_query_capabilities(void *handle) {
    rxpa_plugin_query_v1 query;
    const rxpa_plugin_manifest_v1 *manifest;
    size_t minimum_size = offsetof(rxpa_plugin_manifest_v1, capabilities) +
                          sizeof(((rxpa_plugin_manifest_v1 *)0)->capabilities);

    query = (rxpa_plugin_query_v1)rxpa_os_symbol(
            handle, RXPA_PLUGIN_QUERY_SYMBOL_V1);
    if (!query) return 0u;
    manifest = query();
    if (!manifest || manifest->struct_size < minimum_size ||
        manifest->abi_version != RXPA_PLUGIN_MANIFEST_ABI_V1 ||
        (manifest->capabilities & ~RXPA_PLUGIN_CAP_KNOWN_V1) != 0u) {
        return 0u;
    }
    return manifest->capabilities;
}

int rxpa_open_plugin(char *dir, char *file_name, rxpa_loaded_plugin *plugin) {
    char *full_file_name;
    int free_full_file_name = 0;
    void *handle = NULL;
    initfuncs_type initializer;

    if (!file_name || !plugin) return -1;
    memset(plugin, 0, sizeof(*plugin));

    if (!dir) {
        full_file_name = file_name;
    } else {
        full_file_name = malloc(strlen(dir) + strlen(file_name) + 2u);
        if (!full_file_name) return -1;
        sprintf(full_file_name, "%s/%s", dir, file_name);
        free_full_file_name = 1;
    }

#ifdef _WIN32
    {
        char *load_file_name = full_file_name;
        char *absolute_file_name = NULL;
        DWORD absolute_file_name_size = GetFullPathNameA(
                full_file_name, 0, NULL, NULL);
        if (absolute_file_name_size > 0) {
            absolute_file_name = malloc((size_t)absolute_file_name_size + 1u);
            if (absolute_file_name &&
                GetFullPathNameA(full_file_name, absolute_file_name_size + 1u,
                                 absolute_file_name, NULL) > 0) {
                load_file_name = absolute_file_name;
            }
        }
        handle = (void *)LoadLibraryA(load_file_name);
        free(absolute_file_name);
    }
#else
    if (full_file_name[0] != '/' && full_file_name[0] != '.') {
        char *relative_path = malloc(strlen(full_file_name) + 3u);
        if (!relative_path) {
            if (free_full_file_name) free(full_file_name);
            return -1;
        }
        sprintf(relative_path, "./%s", full_file_name);
        if (free_full_file_name) free(full_file_name);
        full_file_name = relative_path;
        free_full_file_name = 1;
    }
    handle = dlopen(full_file_name, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Failed to load plugin %s: %s\n",
                full_file_name, dlerror());
    }
#endif

    if (!handle) {
        if (free_full_file_name) free(full_file_name);
        return -1;
    }

    initializer = (initfuncs_type)rxpa_os_symbol(handle, "_initfuncs");
    if (!initializer) {
        rxpa_os_close(handle);
        if (free_full_file_name) free(full_file_name);
        return -2;
    }

    plugin->handle = handle;
    plugin->initializer = initializer;
    RXPA_LOADER_LOCK();
    plugin->capabilities = rxpa_query_capabilities(handle);
    rxpa_live_handles++;
    RXPA_LOADER_UNLOCK();

    if (free_full_file_name) free(full_file_name);
    return 0;
}

int rxpa_initialize_plugin(rxpa_loaded_plugin *plugin, rxpa_initctxptr ctx) {
    if (!plugin || !plugin->handle || !plugin->initializer || !ctx) return -2;

    /* Existing plugins copy the helper table into DSO-static storage. */
    RXPA_LOADER_LOCK();
    plugin->initializer(ctx);
    RXPA_LOADER_UNLOCK();
    return 0;
}

void rxpa_close_plugin(rxpa_loaded_plugin *plugin) {
    void *handle;
    if (!plugin || !plugin->handle) return;
    handle = plugin->handle;
    plugin->handle = NULL;
    plugin->initializer = NULL;
    plugin->capabilities = 0u;

    RXPA_LOADER_LOCK();
    if (!rxpa_live_handles) abort();
    rxpa_live_handles--;
    RXPA_LOADER_UNLOCK();
    rxpa_os_close(handle);
}

size_t rxpa_live_plugin_handle_count(void) {
    size_t count;
    RXPA_LOADER_LOCK();
    count = rxpa_live_handles;
    RXPA_LOADER_UNLOCK();
    return count;
}

// Function to load a plugin dynamically
// - ctx is the context structure containing pointers to plugins helper functions
// - file_name is the name of the plugin
// - dir is the directory where the plugin is located
// dir and file_name are appended to create the full file name
//
// Returns 0 on success
//               -1 Failed to load plugin
//               -2 Failed to call _initfuncs
int load_plugin(rxpa_initctxptr ctx, char* dir, char* file_name)
{
    rxpa_loaded_plugin plugin;
    int rc = rxpa_open_plugin(dir, file_name, &plugin);
    if (rc != 0) return rc;
    rc = rxpa_initialize_plugin(&plugin, ctx);
    if (rc != 0) rxpa_close_plugin(&plugin);
    /* Preserve the legacy success contract: the handle remains resident. */
    return rc;
}
