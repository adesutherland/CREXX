/* Exercise the real call-result allocator without accepting a live-window
 * result when its reservation bookkeeping cannot grow. */
#undef NDEBUG
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "rxcpmain.h"
#include "rxcp_emit.h"

static jmp_buf failure;
static int allocation, fail_at;
static void *reservation;
static const char *reported;
static size_t requested;

static void *fault_realloc(void *previous, size_t bytes) {
    void *result;
    if (++allocation == fail_at) return NULL;
    result = realloc(previous, bytes);
    assert(result);
    reservation = result;
    return result;
}

static void fault_panic(const char *operation, size_t bytes) {
    reported = operation;
    requested = bytes;
    longjmp(failure, 1);
}

#undef RX_PANIC_OOM
#define RX_PANIC_OOM(operation, bytes, detail) fault_panic(operation, bytes)
#define realloc fault_realloc
#include "../rxcp_emit_reg.c"
#undef realloc

static void prepare(Scope *scope, ASTNode *node, dpa *available, dpa *deferred,
                    void **available_slots, void **deferred_slots) {
    int index;
    memset(scope, 0, sizeof(*scope));
    memset(node, 0, sizeof(*node));
    memset(available, 0, sizeof(*available));
    memset(deferred, 0, sizeof(*deferred));
    available->buffer_size = 8;
    available->size = 4;
    available->pointers = available_slots;
    for (index = 0; index < 4; ++index)
        available_slots[index] = (void *)(size_t)(index + 1);
    available_slots[4] = NULL;
    deferred->buffer_size = 1;
    deferred->pointers = deferred_slots;
    deferred_slots[0] = NULL;
    scope->free_registers_array = available;
    scope->deferred_registers_array = deferred;
    scope->num_registers = 6;
    node->scope = scope;
    node->additional_registers = 2;
    node->num_additional_registers = 3;
    allocation = 0;
    reservation = NULL;
    reported = NULL;
    requested = 0;
}

int main(void) {
    Scope scope;
    ASTNode node;
    dpa available, deferred;
    void *available_slots[8], *deferred_slots[1];
    int test, result;

    for (test = 1; test <= 3; ++test) {
        prepare(&scope, &node, &available, &deferred,
                available_slots, deferred_slots);
        fail_at = test;
        if (!setjmp(failure)) {
            allocate_call_result_reg(&node);
            assert(!"call-result allocator accepted reservation exhaustion");
        }
        assert(reported && !strcmp(reported, "realloc call-result register reservations"));
        assert(requested == sizeof(int) * (size_t)test);
        /* Production panic exits. The injected longjmp keeps this test alive,
         * so release the reservation retained by the terminal path. */
        free(reservation);
    }

    prepare(&scope, &node, &available, &deferred,
            available_slots, deferred_slots);
    fail_at = 0;
    result = allocate_call_result_reg(&node);
    assert(result == 1 && allocation == 3 && available.size == 3);
    assert(scope.num_registers == 6);
    assert(get_reg(&scope) == 4);
    assert(get_reg(&scope) == 3);
    assert(get_reg(&scope) == 2);
    assert(!available.size && !deferred.size);

    prepare(&scope, &node, &available, &deferred,
            available_slots, deferred_slots);
    node.additional_registers = 5;
    assert(allocate_call_result_reg(&node) == 4 && !allocation);
    assert(available.size == 3 && !deferred.size);
    puts("PASS call-result reservation exhaustion and outside-window allocation");
    return 0;
}
