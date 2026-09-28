/* Fault the actual worker-owned and standalone factories before value_init. */
#undef NDEBUG
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "platform.h"
#include "rxbin.h"
#include "rxvalue.h"
#include "rxvmmemory.h"
static jmp_buf failure;
static int fail_alloc;
static const char *reported;
static void *fault_malloc(size_t bytes) { return fail_alloc ? NULL : malloc(bytes); }
static void *fault_values(rxvm_memory_worker *worker, size_t count) {
    (void)worker;
    return fail_alloc ? NULL : calloc(count, sizeof(value));
}
static void fault_panic(const char *operation) { reported = operation; longjmp(failure, 1); }
#undef RX_PANIC_OOM
#define RX_PANIC_OOM(operation, bytes, detail) fault_panic(operation)
#define malloc fault_malloc
#define rxvm_memory_alloc_values fault_values
#include "rxvmvars.h"
#undef malloc
#undef rxvm_memory_alloc_values
int main(void) {
    int worker;
    for (worker = 0; worker <= 1; worker++) {
        value *v;
        fail_alloc = 1; reported = NULL;
        if (!setjmp(failure)) {
            if (worker) value_f_in(NULL); else value_f();
            assert(!"value factory failed to report exhaustion");
        }
        assert(reported && strstr(reported, worker ? "worker value" : "standalone value"));
        fail_alloc = 0;
        v = worker ? value_f_in(NULL) : value_f();
        assert(v && !v->string_value && !v->attributes && !v->num_attributes);
        free(v);
    }
    puts("PASS worker and standalone value allocation failure and recovery");
    return 0;
}
