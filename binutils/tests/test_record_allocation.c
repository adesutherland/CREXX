/* Fail real record-pool allocation/growth while reading a valid container.
   Verify detail survives the caller, partial tables are freed, and retry works. */
#undef NDEBUG
#include "rxbin.h"
#include <assert.h>
static size_t pool_bytes, failure_bytes;
static int fail_pool;
static void *owned_pool, *owned_records;
static void *fault_calloc(size_t count, size_t size) {
    void *result;
    if (fail_pool && size == pool_bytes) return NULL;
    result = calloc(count, size);
    if (failure_bytes && size == pool_bytes) owned_pool = result;
    return result;
}
static void *fault_realloc(void *old, size_t size) {
    void *result;
    if (failure_bytes && size == failure_bytes) return NULL;
    result = realloc(old, size);
    if (failure_bytes && result) owned_records = result;
    return result;
}
static void fault_free(void *pointer) {
    if (pointer == owned_pool) owned_pool = NULL;
    if (pointer == owned_records) owned_records = NULL;
    free(pointer);
}
#define calloc fault_calloc
#define realloc fault_realloc
#define free fault_free
#include "../rxbin007.c"
#undef calloc
#undef realloc
#undef free

int main(void) {
    module_file fixture, *loaded;
    float_constant constants[40];
    FILE *file = tmpfile();
    unsigned i, test;
    assert(file);
    init_module(&fixture);
    fixture.name = "allocation";
    fixture.description = "";
    fixture.header.proc_head = fixture.header.meta_head = fixture.header.expose_head = -1;
    fixture.constant = (unsigned char *)constants;
    fixture.header.constant_size = sizeof(constants);
    for (i = 0; i < 40u; i++) {
        constants[i].base.type = FLOAT_CONST;
        constants[i].base.size_in_pool = sizeof(constants[i]);
        constants[i].double_value = (double)i;
    }
    assert(write_module(&fixture, file) == 0);
    pool_bytes = sizeof(rxbin007_pool_read);
    for (test = 0; test < 3u; test++) {
        rewind(file);
        fail_pool = test == 0u;
        failure_bytes = (test == 2u ? 64u : 32u) * sizeof(rxbin007_record_view);
        loaded = NULL;
        assert(read_module(&loaded, file) == -1 && !loaded);
        assert(rxbin_last_error() && strstr(rxbin_last_error(), "out of memory"));
        assert(strstr(rxbin_last_error(), test ? "record table" : "record pools"));
        assert(!owned_pool && !owned_records);
        fail_pool = 0; failure_bytes = 0;
        rewind(file);
        assert(read_module(&loaded, file) == 0 && loaded);
        assert(loaded->header.constant_size == sizeof(constants));
        free_module(loaded);
        assert(read_module(&loaded, file) == 1 && !loaded);
    }
    assert(fclose(file) == 0);
    puts("PASS record allocation detail, partial cleanup and reader reuse");
    return 0;
}
