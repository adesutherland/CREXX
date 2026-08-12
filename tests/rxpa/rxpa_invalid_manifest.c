/* RXPA fixture: an unknown capability bit must fail closed to legacy mode. */

#include "crexxpa.h"

static const rxpa_plugin_manifest_v1 invalid_manifest = {
    sizeof(rxpa_plugin_manifest_v1),
    RXPA_PLUGIN_MANIFEST_ABI_V1,
    0x80000000u,
    "rxpa-invalid-manifest"
};

RXPA_EXTERN_C EXPORT const rxpa_plugin_manifest_v1 *_rxpa_query_v1(void) {
    return &invalid_manifest;
}

PROCEDURE(invalid_manifest_probe)
{
    SETINT(RETURN, 1);
    RESETSIGNAL
}

LOADFUNCS
ADDPROC(invalid_manifest_probe, "rxpatests.invalid_manifest_probe",
        "b", ".int", "");
ENDLOADFUNCS
