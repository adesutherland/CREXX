/* Simulate hosts that know only _initfuncs or the original V2 factory. */
#include "rxpa.h"
#include <stdio.h>
#include <string.h>
static rxpa_libfunc configcreate;
static unsigned metadata_calls;
/* These callbacks already exist in the legacy initializer. Only the optional
 * sized host services are absent from this simulated host. The typed provider
 * still registers declarations before a legacy call is rejected. */
static void addtype(char *name, char *option, char *type) {
    (void)name; (void)option; (void)type; ++metadata_calls;
}
static void addimplements(char *name, char *interface_name) {
    (void)name; (void)interface_name; ++metadata_calls;
}
static void addmember(char *owner, char *kind, char *member, char *type, char *args) {
    (void)owner; (void)kind; (void)member; (void)type; (void)args; ++metadata_calls;
}
static void add(rxpa_libfunc fn, char *name, char *option, char *type, char *args) {
    (void)option; (void)type; (void)args;
    if (!strcmp(name, "rxllama.configcreate")) configcreate = fn;
}
static void setint(rxpa_attribute_value value, rxinteger n) { *(rxinteger *)value = n; }
static void setstring(rxpa_attribute_value value, const char *text) { (void)value; (void)text; }
int main(int argc, char **argv) {
    rxpa_loaded_plugin plugin;
    rxpa_initctx helpers = {0};
    rxpa_host_services_v1 absent = {sizeof(absent), RXPA_HOST_SERVICES_ABI_V1, NULL};
    rxinteger result = 0, signal = 99, handle = 0;
    rxpa_attribute_value args[] = {&handle};
    if (argc != 2 || rxpa_open_plugin(NULL, argv[1], &plugin)) return 1;
    if (!plugin.has_manifest_v2 || plugin.manifest_v2.session_create() ||
        !plugin.manifest_v2.session_create_with_host ||
        plugin.manifest_v2.session_create_with_host(&absent) ||
        plugin.manifest_v2.session_create_with_host(NULL)) return 1;
    helpers.addfunc = add; helpers.setint = setint; helpers.setstring = setstring;
    helpers.addclass = addtype; helpers.addinterface = addtype;
    helpers.addimplements = addimplements; helpers.addmember = addmember;
    if (rxpa_initialize_plugin(&plugin, &helpers) || !configcreate || !metadata_calls) return 1;
    configcreate(1, args, &result, &signal);
    rxpa_close_plugin(&plugin);
    if (result != -7 || signal != SIGNAL_NONE || handle) return 1;
    puts("PASS: rxllama fails bounded on old hosts and missing text service");
    return 0;
}
