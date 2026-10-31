#include "rxinline_recipe.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "inline recipe check failed at line %d: %s\n", \
            __LINE__, #condition); return 1; \
} } while (0)

static void put_u32(unsigned char *p, uint32_t n) {
    p[0] = (unsigned char)n;
    p[1] = (unsigned char)(n >> 8u);
    p[2] = (unsigned char)(n >> 16u);
    p[3] = (unsigned char)(n >> 24u);
}

int main(void) {
    const char *source =
        "count=r7;args=list(r2,r5);a0=r20;proxies=(r21:1,r22:2);"
        "owned=(r20,r21,r22,r23);prep=settp(r4,5)+swap(r4,r6)";
    unsigned char *bytes = NULL;
    unsigned char *again = NULL;
    unsigned char *module_bytes = NULL;
    unsigned char *qualified_bytes = NULL;
    size_t length = 0u, again_length = 0u, module_length = 0u;
    size_t qualified_length = 0u;
    char error[160];
    char *printed;
    rxinl_recipe_view recipe;
    rxinl_module_ref_view module_ref;
    rxinl_qualified_proc_view qualified;
    unsigned char saved;

    CHECK(rxinl_recipe_from_text(source, &bytes, &length,
                                 error, sizeof(error)));
    CHECK(rxinl_recipe_decode(bytes, length, &recipe));
    CHECK(recipe.argument_count == 2u && recipe.proxy_count == 2u &&
          recipe.owned_count == 4u);
    printed = rxinl_recipe_to_text(&recipe);
    CHECK(printed && strcmp(printed, source) == 0);
    CHECK(rxinl_recipe_from_text(printed, &again, &again_length,
                                 error, sizeof(error)));
    CHECK(again_length == length && memcmp(again, bytes, length) == 0);
    free(printed);
    free(again);
    CHECK(rxinl_recipe_seal(bytes, length, 2u, 3u));
    CHECK(rxinl_recipe_decode(bytes, length, &recipe) &&
          recipe.image_module == 2u && recipe.provider_procedure == 3u);
    CHECK(!rxinl_recipe_decode(bytes, length - 1u, &recipe));
    saved = bytes[3]; bytes[3] = '2';
    CHECK(!rxinl_recipe_decode(bytes, length, &recipe)); bytes[3] = saved;
    saved = bytes[4]; bytes[4] |= 0x80u;
    CHECK(!rxinl_recipe_decode(bytes, length, &recipe)); bytes[4] = saved;
    put_u32(bytes + 16u, UINT32_MAX); /* register selector is reserved */
    CHECK(!rxinl_recipe_decode(bytes, length, &recipe));
    put_u32(bytes + 16u, 7u);
    put_u32(bytes + 28u, RXINL_RECIPE_MAX_ITEMS + 1u);
    CHECK(!rxinl_recipe_decode(bytes, length, &recipe));
    put_u32(bytes + 28u, 2u);
    put_u32(bytes + 68u, 2u); /* duplicate list source is legal */
    CHECK(rxinl_recipe_decode(bytes, length, &recipe));
    put_u32(bytes + 80u, 21u); /* duplicate proxy destination */
    CHECK(!rxinl_recipe_decode(bytes, length, &recipe));
    put_u32(bytes + 80u, 22u);
    put_u32(bytes + 8u, UINT32_MAX); /* sealed ordinal must be concrete */
    CHECK(!rxinl_recipe_decode(bytes, length, &recipe));
    free(bytes);

    CHECK(!rxinl_recipe_from_text(
        "count=1;args=list(r2);a0=r20;proxies=(r21:1,r21:1);"
        "owned=(r20,r21);prep=none", &bytes, &length,
        error, sizeof(error)));
    CHECK(!rxinl_recipe_from_text(
        "count=1;args=list(r2);a0=r20;proxies=();"
        "owned=(r20,r20);prep=none", &bytes, &length,
        error, sizeof(error)));
    CHECK(!rxinl_recipe_from_text(
        "count=4294967296;args=list();a0=r20;proxies=();"
        "owned=(r20);prep=none", &bytes, &length,
        error, sizeof(error)));

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
