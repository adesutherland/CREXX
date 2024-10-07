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
#else
#include <dlfcn.h> // Linux/OSX
#endif
#include "decplugin_framework.h"

// Head of the factory list
static decplugin_factory_entry *decplugin_factories = 0;

// Function to load a dynamic decimal plugin
int load_plugin(char* dir, char *name) {

    // Load the plugin - and run the plugin initialization function
    // Create the filename by appending ".decplugin" to the file name
    char *file_name = malloc(strlen(name) + strlen(".decplugin") + 1);
    sprintf(file_name, "%s.decplugin", name);

    // Provide a default current directory
    if (!dir) dir = ".";

// Windows Version
#ifdef _WIN32
    // Create a full file name buffer and append the directory and file name
    char* full_file_name = malloc(strlen(dir) + strlen(file_name) + 2);
    sprintf(full_file_name, "%s\\%s", dir, file_name);
    // Load the DLL
    SetDllDirectory(".");
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
        return -1;
    }

    // Get the plugin initializer address and call it
    decplugin_register_function init = (decplugin_register_function)GetProcAddress(hDll, "_register_dec_plugin");
    if (!init) {
        FreeLibrary(hDll);
        return -2;
    }
    init(register_decplugin);
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
    decplugin_register_function init = (decplugin_register_function)dlsym(hDll, "_register_dec_plugin");
    if (!init) {
        return -2;
    }
    init(register_decplugin);
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
    decplugin_register_function init = (decplugin_register_function)dlsym(hDll, "_register_dec_plugin");
    if (!init) {
        dlclose(hDll);
        return -2;
    }
    init(register_decplugin);
    free(full_file_name);
#endif
    return 0;
}

/* Function to register a decimal plugin factory */
void register_decplugin(char* factory_name, decplugin_factory factory) {
    decplugin_factory_entry *entry = (decplugin_factory_entry *)malloc(sizeof(decplugin_factory_entry));
    if(entry){
        strncpy(entry->name, factory_name, 16);
        entry->factory = factory;
        entry->next = decplugin_factories;
        decplugin_factories = entry;
    }
    else {
        // out of memory - exit with a panic message as per crexx standard
        fprintf(stderr, "PANIC: Out of memory in register_decplugin()\n");
        exit(1);
    }

}

// Function to clear the factory list
void clear_decplugin_factories(){
    decplugin_factory_entry *entry = decplugin_factories;
    while(entry){
        decplugin_factory_entry *next = entry->next;
        free(entry);
        entry = next;
    }
    decplugin_factories = 0;
}

// Function to search for a decimal plugin factory
decplugin_factory find_decplugin_factory(char *name) {
    decplugin_factory_entry *entry = decplugin_factories;
    while(entry){
        if(strcmp(entry->name, name) == 0){
            return entry->factory;
        }
        entry = entry->next;
    }
    return 0;
}

// Function to return a decimal plugin factory (the last registered)
decplugin_factory get_decplugin_factory() {
    return decplugin_factories->factory;
}
