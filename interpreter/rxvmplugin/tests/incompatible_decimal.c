/* A valid registration followed by an incompatible one must publish neither.
 * The first factory must not run before the complete transaction is checked. */
#include <stdlib.h>
#include "rxvmplugin.h"

#if defined(_WIN32)
#define FIXTURE_EXPORT __declspec(dllexport)
#else
#define FIXTURE_EXPORT __attribute__((visibility("default")))
#endif

static rxvm_plugin *must_not_run(void) { abort(); return NULL; }

FIXTURE_EXPORT int _register_rxvm_decimal_plugin_v3(register_rxvm_decimal_factory_v3 register_factory);
FIXTURE_EXPORT int _register_rxvm_decimal_plugin_v3(register_rxvm_decimal_factory_v3 register_factory) {
    rxvm_decimal_registration_v3 registration = {
        sizeof(rxvm_decimal_registration_v3), RXVM_DECIMAL_PLUGIN_ABI_V3,
        RXVM_PLUGIN_DECIMAL, RXVM_DECIMAL_SYNC_CONTRACT_V1, "unpublished", must_not_run
    };
    if (register_factory(&registration) != 0) return -1;
    registration.abi_version = 2u;
    return register_factory(&registration);
}

/* A malformed versioned declaration must not fall back to this symbol. */
FIXTURE_EXPORT void _register_rxvm_plugin(register_rxvmplugin_factory register_factory);
FIXTURE_EXPORT void _register_rxvm_plugin(register_rxvmplugin_factory register_factory) {
    (void)register_factory;
    abort();
}
