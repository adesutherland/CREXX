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
    //if (!dir) dir = ".";

// Windows Version
#ifdef _WIN32
    // Create a full file name buffer and append the directory and file name
    char* full_file_name;
    if (dir) {
        full_file_name = malloc(strlen(dir) + strlen(file_name) + 2);
        sprintf(full_file_name, "%s\\%s", dir, file_name);
    }
    else {
        full_file_name = malloc(strlen(file_name) + 1);
        sprintf(full_file_name, "%s", file_name);
    }
    // Load the DLL
    SetDllDirectory("."); // todo - check this!
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
    char* full_file_name;
    if (dir) {
        full_file_name = malloc(strlen(dir) + strlen(file_name) + 2);
        sprintf(full_file_name, "%s/%s", dir, file_name);
    }
    else {
        full_file_name = malloc(strlen(file_name) + 1);
        sprintf(full_file_name, "%s", file_name);
    }
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
    char* full_file_name;
    if (dir) {
        full_file_name = malloc(strlen(dir) + strlen(file_name) + 2);
        sprintf(full_file_name, "%s/%s", dir, file_name);
    }
    else {
        full_file_name = malloc(strlen(file_name) + 1);
        sprintf(full_file_name, "%s", file_name);
    }
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

        // Add framework-provided functions to the plugin
        if (entry->plugin_info->type == RXVM_PLUGIN_DECIMAL) {
            decplugin *plugin = (decplugin *)entry->plugin_info;
            plugin->number_to_simple_format = number_to_simple_format;
        }
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
rxvm_plugin* find_rxvmplugin(char *name, rxvm_plugin_type type) {
    rxvmplugin_factory_entry *entry = rxvmplugin_factories;
    while(entry){
        if(strcmp(entry->name, name) == 0 && entry->plugin_info->type == type){
            return entry->plugin_info;
        }
        entry = entry->next;
    }
    return 0;
}

// Function to return a plugin factory (the last registered)
rxvm_plugin* get_rxvmplugin(rxvm_plugin_type type) {
    rxvmplugin_factory_entry *entry = rxvmplugin_factories;
    while(entry){
        if(entry->plugin_info->type == type){
            return entry->plugin_info;
        }
        entry = entry->next;
    }
    return 0;
}

// Function to get the next plugin factory of a specific type from an entry in the list
// If the entry is NULL, the search starts from the head of the list, otherwise it starts after the entry
rxvm_plugin* get_next_rxvmplugin(rxvmplugin_factory_entry **entry, rxvm_plugin_type type) {
    if(!*entry) {
        *entry = rxvmplugin_factories;
    }
    else {
        *entry = (*entry)->next;
    }
    while(*entry){
        if((*entry)->plugin_info->type == type){
            return (*entry)->plugin_info;
        }
        *entry = (*entry)->next;
    }
    return 0;
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
        mantissa = (char *)malloc(mantLen + 1);
        if (!mantissa) {
            perror("PANIC: Failed to allocate memory for deExponify");
            exit(EXIT_FAILURE);
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
        for (char *m = dotPos; *m; m++) {
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
        if (mantissa != mantissa_buffer) free(mantissa);
    }

    // If newDotPos <= 0, output "0." followed by (-newDotPos-1) zeros, then digits
    if (newDotPos <= 0) {
        // put "0."
        *o++ = '0';
        if (needDecimal) {
            *o++ = '.';
        }
        // put leading zeros: (-newDotPos) = leadingZeros
        for (int i = 0; i < leadingZeros; i++) {
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
        for (int i = 0; i < trailingZeros; i++) {
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
    if (mantissa != mantissa_buffer) free(mantissa);

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
    for (const char *c = check; *c; c++) {
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

