/* cREXX License (MIT). Private mandatory VM-core declaration catalogue. */
#ifndef CREXX_RXCORE_BINDINGS_H
#define CREXX_RXCORE_BINDINGS_H

enum rx_core_binding_class {
    RX_CORE_BINDING_ERROR = -2,
    RX_CORE_BINDING_CONFLICT = -1,
    RX_CORE_BINDING_EXTERNAL = 0,
    RX_CORE_BINDING_MANDATORY = 1
};

/* Classify an exact native id/callable/signature. A claimed core provider or
 * core callable with a conflicting contract must never become an external
 * archive dependency or be silently removed from the requirements file. */
enum rx_core_binding_class rx_core_binding_classify(
        const char *provider, const char *symbol, const char *options,
        const char *type, const char *args);

#endif
