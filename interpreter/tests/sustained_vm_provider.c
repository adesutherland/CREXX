/* Test-only reuse of the native object/session ownership fixture. */
#include "../../tests/rxpa/rxpa_objects.c"

#ifndef DECL_ONLY
/* These are existing fixture counters, not VM/provider-private telemetry. */
int sustained_vm_provider_sessions(rxinteger *created, rxinteger *destroyed) {
    *created = read_counter(&sessions_created);
    *destroyed = read_counter(&sessions_destroyed);
    return current == NULL;
}
#endif
