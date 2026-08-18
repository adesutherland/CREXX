/* Focused raw-copy lifecycle proof for the DECIMAL-01 decQuad candidate. */

#include <stdio.h>
#include <string.h>

#include "platform.h"
#include "rxbin.h"
#include "rxvmplugin_framework.h"
#include "rxvmvars.h"

static int copy_decimal(value *target, const value *source) {
    size_t length = rxvm_value_decimal_length(source);
    void *storage = rxvm_value_reserve_decimal(target, length);
    if (!storage) return 1;
    memcpy(storage, source->decimal_value, length);
    return 0;
}

int main(void) {
    numeric_context context = {
        18, 0, NUMERIC_FORM_SCIENTIFIC, CASE_LOWER, NUMERIC_STANDARD_COMMON
    };
    decplugin *plugin;
    value source;
    value copy;
    value addend;
    char output[80];

    value_init(&source);
    value_init(&copy);
    value_init(&addend);
    if (load_rxvmplugin(".", "rxvm_decquad_decimal") != 0) {
        fprintf(stderr, "FAIL: candidate load\n");
        return 1;
    }
    plugin = (decplugin *)get_rxvmplugin(RXVM_PLUGIN_DECIMAL);
    if (!plugin) {
        fprintf(stderr, "FAIL: candidate registration\n");
        return 1;
    }
    plugin->num_context = &context;
    plugin->syncNumericContext(plugin);
    plugin->decimalFromString(plugin, &source, "12345.6789");
    if (plugin->base.signal_number || copy_decimal(&copy, &source)) {
        fprintf(stderr, "FAIL: source or raw copy\n");
        return 1;
    }
    if (rxvm_value_decimal_length(&source) != 16u ||
        rxvm_value_decimal_length(&copy) != 16u) {
        fprintf(stderr, "FAIL: decQuad sidecar is not 16 bytes\n");
        return 1;
    }

    clear_value(&source);
    plugin->decimalToString(plugin, &copy, output);
    if (strcmp(output, "12345.6789") != 0) {
        fprintf(stderr, "FAIL: copied value after source clear: %s\n", output);
        return 1;
    }

    plugin->decimalFromString(plugin, &addend, "0.3211");
    plugin->decimalAdd(plugin, &copy, &copy, &addend);
    plugin->decimalToString(plugin, &copy, output);
    if (plugin->base.signal_number || strcmp(output, "12346") != 0) {
        fprintf(stderr, "FAIL: in-place arithmetic after raw copy: %s\n",
                output);
        return 1;
    }

    clear_value(&addend);
    clear_value(&copy);
    plugin->base.free((rxvm_plugin *)plugin);
    puts("PASS: decQuad fixed-34 lifecycle");
    return 0;
}
