//
// CREXX Plugin Architecture Library
//
// Plugin PoC
// This has the functions that allow a library of functions to be loaded and executed.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h> // Linux/OSX
#endif

#include "rxpa.h"
#include "crexxpa.h"

// Private Structure holding the name and pointer to the function, and a pointer
// to the next function in the list.
typedef struct funcdef {
    char* name;
    libfunc func;
    struct funcdef* next;
} funcdef;

// Static / Global Variable for the Start of the Function List
static funcdef* funcs = 0;

// Function to add a function to the list of functions
void addfunc(char* name, libfunc func)
{
    // Add the function to the start of the list
    funcdef* f = (funcdef*)malloc( sizeof( funcdef));
    f->name = name;
    f->func = func;
    f->next = funcs;
    funcs = f;
}

// Function to find a function in the list of functions
libfunc findfunc( char* name)
{
    // Find the function in the list
    funcdef* f = funcs;
    while( f)
    {
        if( strcmp( f->name, name) == 0)
            return f->func;
        f = f->next;
    }
    return 0;
}

// Initialization context structure
static struct initctx ctx = { addfunc, findfunc};

// Function pointer type for '_initfuncs'
typedef void (*initfuncs_type)(initctxptr);

// Function to load a plugin dynamically
int load_plugin(char* plugin_name)
{
// Windows Version
#ifdef _WIN32
    // prefix the plugin name with 'rx' and suffix with '.dll'
    char dllname[256];
    strcpy( dllname, "rx");
    strcat( dllname, plugin_name);
    strcat( dllname, ".dll");

    // Load the DLL
    HMODULE hDll = LoadLibrary(TEXT(dllname));
    if (!hDll) {
        printf("Failed to load DLL\n");
        return -1;
    }

    // Get the plugin initializer address and call it
    initfuncs_type init = (initfuncs_type)GetProcAddress(hDll, "_initfuncs");
    if (!init) {
        printf("Failed to find function '_initfuncs'\n");
        FreeLibrary(hDll);
        return -1;
    }
    init(&ctx);

// OSX Version
#elif __APPLE__
    // prefix the plugin name with 'rx' and suffix with '.dylib'
    char dylibname[256];
    strcpy( dylibname, "rx");
    strcat( dylibname, plugin_name);
    strcat( dylibname, ".dylib");

    // Load the dylib
    void* hDll = dlopen(dylibname, RTLD_LAZY);
    if (!hDll) {
        printf("Failed to load dylib\n");
        return -1;
    }

    // Get the plugin initializer address and call it
    initfuncs_type init = (initfuncs_type)dlsym(hDll, "_initfuncs");
    if (!init) {
        printf("Failed to find function '_initfuncs'\n");
        dlclose(hDll);
        return -1;
    }
    init(&ctx);

// Linux Version
#else
    // prefix the plugin name with 'rx' and suffix with '.so'
    char soname[256];
    strcpy( soname, "rx");
    strcat( soname, plugin_name);
    strcat( soname, ".so");

    // Load the so
    void* hDll = dlopen(soname, RTLD_LAZY);
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
    init(&ctx);
#endif

    return 0;
}

