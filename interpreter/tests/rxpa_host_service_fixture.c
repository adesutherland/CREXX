/* A real dynamic plugin with either the frozen old V2 prefix or new service. */
#include "crexxpa.h"
#include <string.h>
static uint32_t caps(const char *name) { (void)name; return RXPA_PROCEDURE_CAP_SESSION_AFFINE; }
static void *old_create(void) {
#ifdef OLD_MANIFEST
    static int session;
    return &session;
#else
    return NULL; /* Explicit failure on a host that only knows old factories. */
#endif
}
static void destroy(void *session) { (void)session; }
static int enter(void *session, uint32_t flags, void **previous) {
    (void)flags; *previous = NULL; return session ? 0 : -1;
}
static void leave(void *previous) { (void)previous; }
#ifdef OLD_MANIFEST
/* Do not use sizeof today's manifest: this is the exact historical layout. */
static const struct {
    size_t struct_size; uint32_t abi_version; const char *plugin_id;
    rxpa_procedure_capability_query_v2 procedure_capabilities;
    rxpa_session_create_v2 session_create;
    rxpa_session_destroy_v2 session_destroy;
    rxpa_session_enter_v2 session_enter;
    rxpa_session_leave_v2 session_leave;
} old = {sizeof(old), 2, "fixture-old", caps, old_create, destroy, enter, leave};
EXPORT const rxpa_plugin_manifest_v2 *_rxpa_query_v2(void) {
    return (const rxpa_plugin_manifest_v2 *)&old;
}
#else
static void *create(const rxpa_host_services_v1 *host) {
    return rxpa_host_has_string_view(host) ? (void *)host : NULL;
}
RXPA_PLUGIN_SESSION_WITH_HOST(old_create, destroy, enter, leave, caps, create)
#endif
LOADFUNCS
ENDLOADFUNCS
