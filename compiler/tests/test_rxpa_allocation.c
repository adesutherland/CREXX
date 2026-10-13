/* Exercise real compiler RXPA callbacks under allocation failure. */
#undef NDEBUG
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
static jmp_buf failure;
static int fail_allocation;
static const char *reported;
static size_t requested;
static void *fault_malloc(size_t size) {
    return fail_allocation ? NULL : malloc(size);
}
static void fault_panic(const char *operation, size_t bytes) {
    reported = operation;
    requested = bytes;
    longjmp(failure, 1);
}
#undef RX_PANIC_OOM
#define RX_PANIC_OOM(operation, bytes, detail) fault_panic(operation, bytes)
#define malloc fault_malloc
#include "../rxcpfunc.c"
#undef malloc

static void add_entry(int entry) {
    switch (entry) {
    case 0: rxpa_addclass("test", "b", ".object"); break;
    case 1: rxpa_addinterface("test", "b", ".object"); break;
    case 2: rxpa_addimplements("test", "interface"); break;
    case 3: rxpa_addmember("test", "attribute", "value", ".int", ""); break;
    default: rxpa_addfunc(NULL, "test", "b", ".int", ""); break;
    }
}
int main(void) {
    Context context;
    int scope, entry;
    memset(&context, 0, sizeof(context));
    for (scope = 0; scope < 2; ++scope) {
        plugin_being_loaded_context = scope ? &context : NULL;
        for (entry = 0; entry < 5; ++entry) {
            reported = NULL;
            fail_allocation = 1;
            if (!setjmp(failure)) {
                add_entry(entry);
                assert(!"RXPA callback accepted allocation failure");
            }
            assert(reported && strstr(reported, entry == 4 ? "function" : "metadata"));
            assert(requested == (entry == 4 ? sizeof(struct static_linked_function) :
                                             sizeof(struct static_linked_metadata)));
            assert(!static_linked_metadata && !static_linked_functions);
            assert(!plugin_being_loaded_metadata && !plugin_being_loaded_functions);
        }
        fail_allocation = 0;
        add_entry(0);
        add_entry(4);
        if (scope) {
            assert(plugin_being_loaded_metadata && plugin_being_loaded_functions);
            free_rxpa_metadata_list(&plugin_being_loaded_metadata);
            free(plugin_being_loaded_functions);
            plugin_being_loaded_functions = NULL;
        } else {
            assert(static_linked_metadata && static_linked_functions);
            free_rxpa_metadata_list(&static_linked_metadata);
            free(static_linked_functions);
            static_linked_functions = NULL;
        }
    }
    plugin_being_loaded_context = NULL;
    puts("PASS RXPA metadata/function exhaustion and healthy registration");
    return 0;
}
