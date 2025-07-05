//
// Created by Adrian Sutherland on 20/09/2024.
//

#ifndef CREXX_RXVMPLUGIN_FRAMEWORK_H
#define CREXX_RXVMPLUGIN_FRAMEWORK_H
#include "rxvmplugin.h"

// Structure to hold the rxvm plugins
typedef struct rxvmplugin_factory_entry rxvmplugin_factory_entry;
struct rxvmplugin_factory_entry {
    char name[16]; // Plugin Name
    rxvm_plugin *plugin_info; // Plugin Information
    rxvm_plugin_factory factory; // Plugin Factory
    rxvmplugin_factory_entry *next; // Next plugin
};

// Function to load a dynamic rxvm plugin
int load_rxvmplugin(char* dir, char *name);

// Function to clear the factory list
void clear_rxvmplugin_factories();

// Function to search for a rxvm plugin
rxvm_plugin* find_rxvmplugin(char *name, rxvm_plugin_type type);

// Function to return a rxvm plugin (the last registered)
rxvm_plugin* get_rxvmplugin(rxvm_plugin_type type);

// Function to get the next plugin of a specific type from an entry in the list
rxvm_plugin* get_next_rxvmplugin(rxvmplugin_factory_entry **entry, rxvm_plugin_type type);

// typedef for dynamic plugin register function
typedef void (*rxvmplugin_register_function)(register_rxvmplugin_factory register_func);

// Helper function for decimal plugins to reformat a number string (should be valid, errors not checked)
// to remove the exponent, to make the number in simple format
void number_to_simple_format(const char *input, char *output);

#endif //CREXX_RXVMPLUGIN_FRAMEWORK_H
