/* The actual AST factory must stop before dereferencing a failed allocation. */
#undef NDEBUG
#include <assert.h>
#include <setjmp.h>
#include "platform.h"
#include "rxcpmain.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
static jmp_buf failure;
static int fail_allocation;
static size_t requested;
static const char *reported;
static void *fault_malloc(size_t size) {
    return fail_allocation ? NULL : malloc(size);
}
static void fault_panic(const char *operation, size_t size) {
    reported = operation; requested = size;
    longjmp(failure, 1);
}
#undef RX_PANIC_OOM
#define RX_PANIC_OOM(operation, bytes, detail) fault_panic(operation, bytes)
#define malloc fault_malloc
#include "../rxcp_ast_core.c"
#undef malloc
int main(void) {
    Context context;
    ASTNode *node;
    memset(&context, 0, sizeof(context));
    context.file_name = "allocation.crexx";
    fail_allocation = 1;
    if (!setjmp(failure)) {
        ast_ft(&context, ERROR);
        assert(!"AST factory failed to report exhaustion");
    }
    assert(reported && !strcmp(reported, "malloc compiler AST node"));
    assert(requested == sizeof(ASTNode) && !context.free_list);
    fail_allocation = 0;
    node = ast_ft(&context, ERROR);
    assert(node && node->context == &context && node->node_type == ERROR);
    assert(context.free_list == node);
    free_ast(&context);
    assert(!context.free_list);
    puts("PASS AST allocation failure and healthy factory");
    return 0;
}
