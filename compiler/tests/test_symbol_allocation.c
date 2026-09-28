/* Fail each allocation in the real symbol factory before it is used. */
#undef NDEBUG
#include <assert.h>
#include <setjmp.h>
#include "platform.h"
#include "rxcpmain.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
static jmp_buf failure;
static int allocation, fail_at;
static void *owned[8];
static unsigned owned_count;
static const char *reported;
static void *fault_malloc(size_t size) {
    void *p;
    if (++allocation == fail_at) return NULL;
    p = malloc(size);
    assert(owned_count < 8);
    owned[owned_count++] = p;
    return p;
}
static void fault_panic(const char *operation) { reported = operation; longjmp(failure, 1); }
#undef RX_PANIC_OOM
#define RX_PANIC_OOM(operation, bytes, detail) fault_panic(operation)
#define malloc fault_malloc
#include "../rxcpsymb.c"
#undef malloc
int main(void) {
    int test;
    for (test = 1; test <= 3; test++) {
        allocation = 0; owned_count = 0; reported = NULL;
        fail_at = test == 3 ? 0 : test;
        if (!setjmp(failure)) {
            sym_fn(NULL, "x", test == 3 ? SIZE_MAX : 1u);
            assert(!"symbol factory failed to report exhaustion");
        }
        assert(reported && strstr(reported, test == 1 ? "symbol" : "symbol name"));
        /* Production panic exits; this harness releases captured terminal
         * allocations because longjmp intentionally keeps the test alive. */
        while (owned_count) free(owned[--owned_count]);
    }
    puts("PASS symbol allocation and size overflow guards");
    return 0;
}
