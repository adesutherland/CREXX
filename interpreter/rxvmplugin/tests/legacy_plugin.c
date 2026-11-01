/* Frozen pre-ABI-3 registration and base layout. Deliberately do not include
 * the current plugin header: decimal rejection must precede extension access. */
#include <stdlib.h>

#ifndef LEGACY_PLUGIN_TYPE
#define LEGACY_PLUGIN_TYPE 1
#endif
#if defined(_WIN32)
#define LEGACY_EXPORT __declspec(dllexport)
#else
#define LEGACY_EXPORT __attribute__((visibility("default")))
#endif

typedef struct legacy_plugin legacy_plugin;
struct legacy_plugin {
    int type;
    char *name, *version, *description;
    int signal_number;
    char *signal_string;
    void *private_context;
    void (*free)(legacy_plugin *);
    int (*set_option)(legacy_plugin *, char *, char *);
    int (*get_option)(legacy_plugin *, char *, char **);
};
typedef legacy_plugin *(*legacy_factory)(void);
typedef void (*legacy_register)(char *, legacy_factory);

static void destroy_plugin(legacy_plugin *plugin) { free(plugin); }
static legacy_plugin *make_plugin(void) {
    legacy_plugin *plugin = (legacy_plugin *)calloc(1u, sizeof(*plugin));
    if (!plugin) return NULL;
    plugin->type = LEGACY_PLUGIN_TYPE;
    plugin->name = "old-provider";
    plugin->version = "2";
    plugin->description = "frozen generic registration fixture";
    plugin->free = destroy_plugin;
    return plugin;
}

LEGACY_EXPORT void _register_rxvm_plugin(legacy_register register_factory);
LEGACY_EXPORT void _register_rxvm_plugin(legacy_register register_factory) {
    register_factory(LEGACY_PLUGIN_TYPE == 1 ? "old-decimal" : "old-unicode", make_plugin);
}
