//
// Created by Adrian Sutherland on 20/09/2024.
//

#ifndef CREXX_DECPLUGIN_FRAMEWORK_H
#define CREXX_DECPLUGIN_FRAMEWORK_H
#include "decplugin.h"

// Structure to hold the decimal plugin factories
typedef struct decplugin_factory_entry decplugin_factory_entry;
struct decplugin_factory_entry {
    char name[16]; // Name of the factory
    decplugin_factory factory; // Factory function
    decplugin_factory_entry *next; // Next factory
};

// Function to load a dynamic decimal plugin
int load_plugin(char* dir, char *name);

// Function to clear the factory list
void clear_decplugin_factories();

// Function to search for a decimal plugin factory
decplugin_factory find_decplugin_factory(char *name);

// Function to return a decimal plugin factory (the last registered)
decplugin_factory get_decplugin_factory();

// typedef for dynamic plugin register function
typedef void (*decplugin_register_function)(register_decplugin_factory register_func);

#endif //CREXX_DECPLUGIN_FRAMEWORK_H
