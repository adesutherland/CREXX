/* cREXX License (MIT). Core ownership uses the runtime's exact signatures. */
#include "../rxcore_bindings.h"
#include "rxsignature.h"
#include <stdio.h>

#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "Core binding check failed at line %d: %s\n", __LINE__, #test); \
    return 1; } } while (0)

int main(void) {
    CHECK(rx_core_binding_classify("rxparse", "rxparse.cursor.next", "b", ".int", "") == RX_CORE_BINDING_MANDATORY);
    CHECK(rx_core_binding_classify("rxinvoke", "rxinvoke.catalogue.\xc2\xa7" "factory", "b",
        ".rxinvoke..catalogue", "names=.string[*],results=.string[*],args=.string[*],providers=.string[*]") == RX_CORE_BINDING_MANDATORY);
    CHECK(rx_core_binding_classify("rxinvoke", "rxinvoke.prepare", " b ",
        ".rxinvoke..invocation", "binding = .rxinvoke..binding, cells = .rxinvoke..scalar[ ], presence = .int[*], extent = .int, bytebudget = .int") == RX_CORE_BINDING_MANDATORY);
    CHECK(rx_core_binding_classify("rxparse", "rxparse.cursor.next", "b", ".string", "") == RX_CORE_BINDING_CONFLICT);
    CHECK(rx_core_binding_classify("rxparse", "rxparse.cursor.supply", "b", ".void", "renamed=.string") == RX_CORE_BINDING_CONFLICT);
    CHECK(rx_core_binding_classify("rxparse", "rxparse.cursor.supply", "b", ".void", "expose text=.string") == RX_CORE_BINDING_CONFLICT);
    CHECK(rx_core_binding_classify("rxparse", "rxparse.cursor.supply", "b", ".void", "?text=.string") == RX_CORE_BINDING_CONFLICT);
    CHECK(rx_core_binding_classify("rxparse", "rxparse.cursor.next", "g", ".int", "") == RX_CORE_BINDING_CONFLICT);
    CHECK(rx_core_binding_classify("other", "rxparse.cursor.next", "b", ".int", "") == RX_CORE_BINDING_CONFLICT);
    CHECK(rx_core_binding_classify("rxparse", "other.call", "b", ".int", "") == RX_CORE_BINDING_CONFLICT);
    CHECK(rx_core_binding_classify("rxinvoke", "rxparse.cursor.next", "b", ".int", "") == RX_CORE_BINDING_CONFLICT);
    CHECK(rx_core_binding_classify("other", "other.call", "b", ".int", "") == RX_CORE_BINDING_EXTERNAL);
    CHECK(rx_sig_provider_text_equal(" cells = .rxinvoke..scalar[ ], presence=.int[] ",
                                     "cells=.rxinvoke..scalar[*],presence=.int[*]") == 1);
    CHECK(rx_sig_provider_text_equal("text=.string", "renamed=.string") == 0);
    CHECK(rx_sig_provider_text_equal("expose text=.string", "text=.string") == 0);
    CHECK(rx_sig_provider_text_equal("cells=.int[2]", "cells=.int[*]") == 0);
    puts("PASS: mandatory core binding identities and exact signatures");
    return 0;
}
