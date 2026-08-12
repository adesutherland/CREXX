/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//
// Created by Adrian Sutherland on 20/09/2024.
//

#ifndef CREXX_RXVMPLUGIN_FRAMEWORK_H
#define CREXX_RXVMPLUGIN_FRAMEWORK_H
#include "rxvmplugin.h"

/*
 * Immutable catalogue descriptor.  The first five fields retain the legacy
 * framework layout; plugin_info is the compatibility-adapter instance used by
 * existing compiler and standalone plugin consumers.  VM execution creates a
 * private instance from factory instead of borrowing plugin_info.
 */
typedef struct rxvmplugin_factory_entry rxvmplugin_factory_entry;
typedef struct rxvmplugin_library rxvmplugin_library;
struct rxvmplugin_factory_entry {
    char name[16]; // Plugin Name
    rxvm_plugin *plugin_info; // Legacy compatibility-adapter instance
    rxvm_plugin_factory factory; // Plugin Factory
    void *handle; // Legacy raw view of the dynamic-library handle
    rxvmplugin_factory_entry *next; // Next plugin
    rxvm_plugin_type type;
    unsigned long generation;
    size_t instance_references;
    unsigned char retired;
    rxvmplugin_library *library;
    rxvmplugin_factory_entry *retired_next;
};

typedef struct rxvmplugin_instance {
    rxvm_plugin *plugin;
    rxvmplugin_factory_entry *descriptor;
} rxvmplugin_instance;

typedef struct rxvmplugin_instance_set {
    rxvmplugin_instance entries[RXVM_PLUGIN_MAX];
} rxvmplugin_instance_set;

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

/* Worker-VM-owned provider instances. */
void rxvmplugin_instance_set_init(rxvmplugin_instance_set *set);
int rxvmplugin_instance_set_prepare(rxvmplugin_instance_set *set,
                                    rxvm_plugin_type type);
rxvm_plugin *rxvmplugin_instance_set_get(rxvmplugin_instance_set *set,
                                         rxvm_plugin_type type);
void rxvmplugin_instance_set_destroy(rxvmplugin_instance_set *set);

/* Focused lifecycle diagnostics used by the ownership regression test. */
size_t rxvmplugin_catalogue_count(void);
size_t rxvmplugin_live_instance_count(void);

// typedef for dynamic plugin register function
typedef void (*rxvmplugin_register_function)(register_rxvmplugin_factory register_func);

// Helper function for decimal plugins to reformat a number string (should be valid, errors not checked)
// to remove the exponent, to make the number in simple format
void number_to_simple_format(const char *input, char *output);

#endif //CREXX_RXVMPLUGIN_FRAMEWORK_H
