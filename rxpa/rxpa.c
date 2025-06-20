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
    // Create a full file name buffer and append the directory and file name
    char* full_file_name;
    int free_full_file_name = 0;

    if (!dir) {
        full_file_name  = file_name;
    } else {
        full_file_name  = malloc(strlen(dir) + strlen(file_name) + 2);
        sprintf(full_file_name, "%s/%s", dir, file_name);
        free_full_file_name = 1;
    }

// Windows Version
#ifdef _WIN32
    // Load the DLL
    // SetDllDirectory("."); // Commented out because it should not be necessary, but it can be harmful because it
                             // means the current directory is searched for DLLs BEFORE the system directories, which
                             // can lead to DLL hijacking attacks.
    HMODULE hDll = LoadLibrary(TEXT(full_file_name));
    if (!hDll) {
        DWORD errorCode = GetLastError();
        LPVOID errorMsg;
        FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                errorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&errorMsg,
                0,
                NULL
        );
        LocalFree(errorMsg);
        if (free_full_file_name) free(full_file_name);
        return -1;
    }

    // Get the plugin initializer address and call it
    initfuncs_type init = (initfuncs_type)GetProcAddress(hDll, "_initfuncs");
    if (!init) {
        FreeLibrary(hDll);
        if (free_full_file_name) free(full_file_name);
        return -2;
    }
    init(ctx);

// OSX Version
#elif __APPLE__
    // Load the dylib
    void* hDll = dlopen(full_file_name, RTLD_LAZY);
    if (!hDll) {
        if (free_full_file_name) free(full_file_name);
        return -1;
    }

    // Get the plugin initializer address and call it
    initfuncs_type init = (initfuncs_type)dlsym(hDll, "_initfuncs");
    if (!init) {
        if (free_full_file_name) free(full_file_name);
        return -2;
    }
    init(ctx);

// Linux Version
#else
    // Load the so
    // A special case for linux - if it is not an absolute path, load it as a relative path by
    // prepending "./" to the file name
    if (full_file_name[0] != '/' && full_file_name[0] != '.') {
        char* relative_path = malloc(strlen("./") + strlen(file_name) + 1);
        sprintf(relative_path, "./%s", full_file_name);
        if (free_full_file_name) free(full_file_name);
        full_file_name = relative_path;
        free_full_file_name = 1;
    }
    void* hDll = dlopen(full_file_name, RTLD_LAZY);
    if (!hDll) {
        if (free_full_file_name) free(full_file_name);
        return -1;
    }

    // Get the plugin initializer address and call it
    initfuncs_type init = (initfuncs_type)dlsym(hDll, "_initfuncs");
    if (!init) {
        dlclose(hDll);
        if (free_full_file_name) free(full_file_name);
        return -2;
    }
    init(ctx);
#endif

    if (free_full_file_name) free(full_file_name);
    return 0;
}

