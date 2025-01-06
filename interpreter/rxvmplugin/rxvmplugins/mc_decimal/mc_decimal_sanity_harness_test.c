//
// Created by Adrian Sutherland on 16/09/2024.
//

#include <stdio.h>
#include <stdlib.h>
#include "rxvmplugin_framework.h"

int main(int argc, char *argv[]) {
    value a, b, result;

// If DYNAMIC is defined, load the plugin
#ifdef DYNAMIC
    // Load the plugin
    printf("Loading Dynamic Plugin\n");
    if (load_rxvmplugin(".", "rxvm_mc_decimal") != 0) {
        printf("Unable to load the rxvmplugin plugin\n");
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

    decplugin *plugin = (decplugin*)get_rxvmplugin(RXVM_PLUGIN_DECIMAL);
    if (!plugin) {
        printf("No default rxvmplugin plugin\n");
        return 1;
    }

    // Set the number of digits in the rxvmplugin context
    plugin->setDigits(plugin, atoi(argv[1]));

    /* Make a string buffer to hold the result */
    char* string = malloc(plugin->getRequiredStringSize(plugin));

    a.decimal_value = NULL;
    b.decimal_value = NULL;
    result.decimal_value = NULL;

    // Convert the arguments to rxvmplugin numbers
    plugin->decimalFromString(plugin, &a, argv[2]);
    plugin->decimalFromString(plugin, &b, argv[3]);

    // Add the numbers and print the result
    plugin->decimalAdd(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, string);
    printf("ADD:%s:\n", string);

    // Subtract the numbers and print the result
    plugin->decimalSub(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, string);
    printf("SUB:%s:\n", string);

    // Multiply the numbers and print the result
    plugin->decimalMul(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, string);
    printf("MUL:%s:\n", string);

    // Divide the numbers and print the result
    plugin->decimalDiv(plugin, &result, &a, &b);
    plugin->decimalToString(plugin, &result, string);
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

    plugin->base.free((rxvm_plugin*)plugin);
    return 0;
} // main
