#include "rxinline_recipe.h"

#include <stdio.h>
#include <stdlib.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "qualified reference check failed at line %d: %s\n", \
            __LINE__, #condition); return 1; \
} } while (0)

int main(void) {
    unsigned char *module_bytes = NULL;
    unsigned char *qualified_bytes = NULL;
    size_t module_length = 0u;
    size_t qualified_length = 0u;
    rxinl_module_ref_view module_ref;
    rxinl_qualified_proc_view qualified;
    CHECK(rxinl_module_ref_from_text("self", &module_bytes,
                                     &module_length));
    CHECK(rxinl_module_ref_decode(module_bytes, module_length, &module_ref) &&
          module_ref.self_relative && !module_ref.sealed);
    CHECK(rxinl_module_ref_seal(module_bytes, module_length, 3u));
    CHECK(rxinl_module_ref_decode(module_bytes, module_length, &module_ref) &&
          module_ref.self_relative && module_ref.image_module == 3u);
    free(module_bytes);
    CHECK(rxinl_module_ref_from_text("@2", &module_bytes,
                                     &module_length));
    CHECK(rxinl_module_ref_seal(module_bytes, module_length, 2u));
    CHECK(rxinl_module_ref_decode(module_bytes, module_length, &module_ref) &&
          !module_ref.self_relative && module_ref.image_module == 2u);
    free(module_bytes);

    CHECK(rxinl_qualified_proc_from_text("@2::3", &qualified_bytes,
                                         &qualified_length));
    CHECK(rxinl_qualified_proc_decode(qualified_bytes, qualified_length,
                                      &qualified) && !qualified.sealed);
    CHECK(rxinl_qualified_proc_seal(qualified_bytes, qualified_length));
    CHECK(rxinl_qualified_proc_decode(qualified_bytes, qualified_length,
                                      &qualified) && qualified.sealed &&
          qualified.image_module == 2u &&
          qualified.provider_procedure == 3u);
    CHECK(!rxinl_qualified_proc_decode(qualified_bytes,
                                       qualified_length - 1u, &qualified));
    free(qualified_bytes);
    return 0;
}
