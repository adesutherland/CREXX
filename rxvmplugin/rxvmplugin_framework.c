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
#include "rxvmplugin_framework.h"

// Head of the factory list
static rxvmplugin_factory_entry *rxvmplugin_factories = 0;

// Function to load a dynamic plugin
int load_rxvmplugin(char* dir, char *name) {

    // Load the plugin - and run the plugin initialization function
    // Create the filename by appending ".rxvmplugin" to the file name
    char *file_name = malloc(strlen(name) + strlen(".rxvmplugin") + 1);
    sprintf(file_name, "%s.rxvmplugin", name);

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
    rxvmplugin_register_function init = (rxvmplugin_register_function)GetProcAddress(hDll, "_register_rxvm_plugin");
    if (!init) {
        FreeLibrary(hDll);
        return -2;
    }
    init(register_rxvmplugin);
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
    rxvmplugin_register_function init = (rxvmplugin_register_function)dlsym(hDll, "_register_rxvm_plugin");
    if (!init) {
        return -2;
    }
    init(register_rxvmplugin);
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
    rxvmplugin_register_function init = (rxvmplugin_register_function)dlsym(hDll, "_register_rxvm_plugin");
    if (!init) {
        dlclose(hDll);
        return -2;
    }
    init(register_rxvmplugin);
    free(full_file_name);
#endif
    return 0;
}

/* Function to register a plugin factory */
void register_rxvmplugin(char* factory_name, rxvm_plugin_factory factory) {
    rxvmplugin_factory_entry *entry = (rxvmplugin_factory_entry *)malloc(sizeof(rxvmplugin_factory_entry));
    if(entry){
        strncpy(entry->name, factory_name, 16); // Copy the name
        entry->factory = factory; // Set the factory function
        entry->plugin_info = factory(); // Get the plugin information from the factory
        entry->next = rxvmplugin_factories; // Add to the head of the list
        rxvmplugin_factories = entry; // Update the head of the list
    }
    else {
        // out of memory - exit with a panic message as per crexx standard
        fprintf(stderr, "PANIC: Out of memory in register_rxvmplugin()\n");
        exit(1);
    }

}

// Function to clear the factory list
void clear_rxvmplugin_factories(){
    rxvmplugin_factory_entry *entry = rxvmplugin_factories;
    while(entry){
        rxvmplugin_factory_entry *next = entry->next; // Save the next entry
        entry->plugin_info->free(entry->plugin_info); // Call the plugin free() function
        free(entry); // Free the entry
        entry = next; // Move to the next entry
    }
    rxvmplugin_factories = 0;
}

// Function to search for a plugin factory
rxvm_plugin_factory find_rxvmplugin_factory(char *name, rxvm_plugin_type type) {
    rxvmplugin_factory_entry *entry = rxvmplugin_factories;
    while(entry){
        if(strcmp(entry->name, name) == 0 && entry->plugin_info->type == type){
            return entry->factory;
        }
        entry = entry->next;
    }
    return 0;
}

// Function to return a plugin factory (the last registered)
rxvm_plugin_factory get_rxvmplugin_factory(rxvm_plugin_type type) {
    rxvmplugin_factory_entry *entry = rxvmplugin_factories;
    while(entry){
        if(entry->plugin_info->type == type){
            return entry->factory;
        }
        entry = entry->next;
    }
    return 0;
}

// Function to get the next plugin factory of a specific type from an entry in the list
// If the entry is NULL, the search starts from the head of the list, otherwise it starts after the entry
rxvm_plugin_factory get_next_rxvmplugin_factory(rxvmplugin_factory_entry **entry, rxvm_plugin_type type) {
    if(!entry || !*entry){
        *entry = rxvmplugin_factories;
    }
    else {
        *entry = (*entry)->next;
    }
    while(*entry){
        if((*entry)->plugin_info->type == type){
            return (*entry)->factory;
        }
        *entry = (*entry)->next;
    }
    return 0;
}
