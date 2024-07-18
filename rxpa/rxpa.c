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
#endif
#include "rxpa.h"

// Typedef the initfuncs_type the initializer function - void f(rxpa_initctxptr context);
typedef void (*initfuncs_type)(rxpa_initctxptr);

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
// Windows Version
#ifdef _WIN32
    // Create a full file name buffer and append the directory and file name
    char* full_file_name = malloc(strlen(dir) + strlen(file_name) + 2);
    sprintf(full_file_name, "%s\\%s", dir, file_name);

    // Load the DLL
    HMODULE hDll = LoadLibrary(TEXT(full_file_name));
    if (!hDll) {
        return -1;
    }

    // Get the plugin initializer address and call it
    initfuncs_type init = (initfuncs_type)GetProcAddress(hDll, "_initfuncs");
    if (!init) {
        FreeLibrary(hDll);
        return -2;
    }
    init(ctx);
    free(full_file_name);

// OSX Version
#elif __APPLE__
    // Create a full file name buffer and append the directory and file name
    char* full_file_name = malloc(strlen(dir) + strlen(file_name) + 2);
    sprintf(full_file_name, "%s/%s", dir, file_name);

    // Load the dylib
    void* hDll = dlopen(full_file_name, RTLD_LAZY);
    if (!hDll) {
        return -1;
    }

    // Get the plugin initializer address and call it
    initfuncs_type init = (initfuncs_type)dlsym(hDll, "_initfuncs");
    if (!init) {
        return -2;
    }
    init(ctx);
    free(full_file_name);

// Linux Version
#else
    // Create a full file name buffer and append the directory and file name
    char* full_file_name = malloc(strlen(dir) + strlen(file_name) + 2);
    sprintf(full_file_name, "%s/%s", dir, file_name);

    // Load the so
    void* hDll = dlopen(full_file_name, RTLD_LAZY);
    if (!hDll) {
        return -1;
    }

    // Get the plugin initializer address and call it
    initfuncs_type init = (initfuncs_type)dlsym(hDll, "_initfuncs");
    if (!init) {
        dlclose(hDll);
        return -2;
    }
    init(ctx);
    free(full_file_name);
#endif

    return 0;
}

