//
// CREXX Plugin Architecture Library
//
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h> // Linux/OSX
#endif
#include "rxpa.h"

// Typedef the initfuncs_type the initializer function - void f(initctxptr context);
typedef void (*initfuncs_type)(initctxptr);

// Function to load a plugin dynamically
// - ctx is the context structure containing pointers to plugins helper functions
// - file_name is the full file name of the plugin
// Returns 0 on success
//               -1 Failed to load plugin
//               -2 Failed to call _initfuncs
int load_plugin(initctxptr ctx, char* file_name)
{
// Windows Version
#ifdef _WIN32
    // Load the DLL
    HMODULE hDll = LoadLibrary(TEXT(file_name));
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

// OSX Version
#elif __APPLE__
    // Load the dylib
    void* hDll = dlopen(file_name, RTLD_LAZY);
    if (!hDll) {
        return -1;
    }

    // Get the plugin initializer address and call it
    initfuncs_type init = (initfuncs_type)dlsym(hDll, "_initfuncs");
    if (!init) {
        printf("Failed to find function '_initfuncs'\n");
        return -2;
    }
    init(ctx);

// Linux Version
#else
    // Load the so
    void* hDll = dlopen(file_name, RTLD_LAZY);
    if (!hDll) {
        printf("Failed to load so\n");
        return -1;
    }

    // Get the plugin initializer address and call it
    initfuncs_type init = (initfuncs_type)dlsym(hDll, "_initfuncs");
    if (!init) {
        printf("Failed to find function '_initfuncs'\n");
        dlclose(hDll);
        return -1;
    }
    init(ctx);
#endif

    return 0;
}

