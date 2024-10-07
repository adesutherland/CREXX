//
// Created by Adrian Sutherland on 16/09/2024.
//
// ICU Decimal Test 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decplugin_framework.h"

int main(int argc, char *argv[]) {
    value a, b, result;

// If DYNAMIC is defined, load the plugin
#ifdef DYNAMIC
    // Load the plugin
    printf("Loading Dynamic Plugin\n");
    if (load_plugin(".", "rxdec_mc_decimal_dyn") != 0) {
        printf("Unable to load the decimal plugin\n");
        return 1;
    }
#else
#ifdef MANUAL_PLUGIN_LINK
    printf("Manually Initialising Plugin\n");
    CALL_PLUGIN_INITIALIZER(decnumber);
#endif
// Otherwise the linker "magic" will take care of initialising the plugin
#endif // DYNAMIC

    if (argc < 4) {
        printf("Please supply the number of digits, and two numbers to add.\n");
        return 1;
    }

    decplugin_factory plugin_factory = find_decplugin_factory("decnumber");
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
