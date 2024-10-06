//
// Created by Adrian Sutherland on 16/09/2024.
//
// ICU Decimal Test 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decplugin_framework.h"

#ifdef DYNAMIC
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h> // Linux/OSX
#endif

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
    initfuncs_type init = (initfuncs_type)GetProcAddress(hDll, "_register_dec_plugin");
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
    initfuncs_type init = (initfuncs_type)dlsym(hDll, "_register_dec_plugin");
    if (!init) {
        dlclose(hDll);
        return -2;
    }
    init(register_decplugin);
    free(full_file_name);
#endif
    return 0;
}
#endif

int main(int argc, char *argv[]) {
    value a, b, result;

// If DYNAMIC is defined, load the plugin
#ifdef DYNAMIC
    // Load the plugin
    printf("Loading Plugin\n");
    if (load_plugin(".", "rxdec_mc_decimal_dyn") != 0) {
        printf("Unable to load the decimal plugin\n");
        return 1;
    }
#endif

    if (argc < 4) {
        printf("Please supply the number of digits, and two numbers to add.\n");
        return 1;
    }

    decplugin_factory plugin_factory = find_decplugin_factory("DEC_PLUGIN");
    if (!plugin_factory) {
        printf("Decimal plugin \"decnumber\" not found\n");
        return 1;
    }
    decplugin *plugin = plugin_factory();

    // Set the number of digits in the decimal context
    plugin->setDigits(plugin, atoi(argv[1]));

    /* Make a string buffer to hold the result */
    char* string = malloc(plugin->getRequiredStringSize(plugin));

    a.decimal_value = NULL;
    b.decimal_value = NULL;
    result.decimal_value = NULL;

    // Convert the arguments to decimal numbers
    plugin->decFloatFromString(plugin, &a, argv[2]);
    plugin->decFloatFromString(plugin, &b, argv[3]);

    // Add the numbers and print the result
    plugin->decFloatAdd(plugin, &result, &a, &b);
    plugin->decFloatToString(plugin, &result, string);
    printf("ADD:%s:\n", string);

    // Subtract the numbers and print the result
    plugin->decFloatSub(plugin, &result, &a, &b);
    plugin->decFloatToString(plugin, &result, string);
    printf("SUB:%s:\n", string);

    // Multiply the numbers and print the result
    plugin->decFloatMul(plugin, &result, &a, &b);
    plugin->decFloatToString(plugin, &result, string);
    printf("MUL:%s:\n", string);

    // Divide the numbers and print the result
    plugin->decFloatDiv(plugin, &result, &a, &b);
    plugin->decFloatToString(plugin, &result, string);
    printf("DIV:%s:\n", string);

    // Free the memory
    if (a.decimal_value) {
        free(a.decimal_value);
    }
    if (b.decimal_value) {
        free(b.decimal_value);
    }
    if (result.decimal_value) {
        free(result.decimal_value);
    }
    free(string);

    plugin->free(plugin);
    return 0;
} // main

