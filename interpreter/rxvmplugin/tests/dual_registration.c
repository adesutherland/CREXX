/* One library can atomically publish a new decimal and a legacy Unicode. */
#define RXVM_PLUGIN dualdecimal
#include <stdlib.h>
#include "rxvmplugin.h"

static void destroy_plugin(rxvm_plugin *plugin) {
    free(plugin->private_context);
    free(plugin);
}

static void sync_context(decplugin *plugin) {
    numeric_context defaults = {DEFAULT_NUMERIC_DIGITS, DEFAULT_NUMERIC_FUZZ,
        DEFAULT_NUMERIC_FORM, DEFAULT_NUMERIC_CASE, NUMERIC_STANDARD_COMMON};
    *(numeric_context *)plugin->base.private_context = plugin->num_context ? *plugin->num_context : defaults;
}

static rxvm_plugin *make_decimal(void) {
    decplugin *plugin = (decplugin *)calloc(1u, sizeof(*plugin));
    if (!plugin) return NULL;
    plugin->base.private_context = calloc(1u, sizeof(numeric_context));
    if (!plugin->base.private_context) { free(plugin); return NULL; }
    plugin->base.type = RXVM_PLUGIN_DECIMAL;
    plugin->base.free = destroy_plugin;
    plugin->syncNumericContext = sync_context;
    return (rxvm_plugin *)plugin;
}

static rxvm_plugin *make_unicode(void) {
    rxvm_plugin *plugin = (rxvm_plugin *)calloc(1u, sizeof(*plugin));
    if (plugin) { plugin->type = RXVM_PLUGIN_UNICODE; plugin->free = destroy_plugin; }
    return plugin;
}

REGISTER_DECIMAL_PLUGIN(make_decimal)
EXPORT void _register_rxvm_plugin(register_rxvmplugin_factory register_factory);
EXPORT void _register_rxvm_plugin(register_rxvmplugin_factory register_factory) {
    register_factory("dual-unicode", make_unicode);
}
