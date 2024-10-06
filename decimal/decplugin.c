//
// Decimal Plugin Framework Implementation
//
// Created by Adrian Sutherland on 20/09/2024.
//
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "decplugin_framework.h"

// Head of the factory list
static decplugin_factory_entry *decplugin_factories = 0;

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