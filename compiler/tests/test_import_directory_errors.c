/* Exercise the compiler's real discovery path with iterator failures. */
#undef NDEBUG
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "rxcpmain.h"
static int fault, closes, opens;
static char candidate[64];
static char *fault_first(const char *directory, char *prefix, char *type, void **handle) {
    (void)directory; (void)prefix;
    ++opens;
    *handle = NULL;
    if (fault == 1) { errno = EIO; return NULL; }
    if (fault == 4) { errno = ENOENT; return NULL; }
    *handle = &fault;
    if (fault == 3) { errno = 0; return NULL; }
    snprintf(candidate, sizeof(candidate), "provider.%s", type);
    errno = 0;
    return candidate;
}
static char *fault_next(void **handle) {
    (void)handle;
    errno = fault == 2 ? EIO : 0;
    return NULL;
}
static void fault_close(void **handle) {
    if (!*handle) return;
    ++closes;
    *handle = NULL;
    if (!errno && fault == 3) errno = EBADF;
}
#define dirfstfl fault_first
#define dirnxtfl fault_next
#define dirclose fault_close
#include "../rxcpfunc.c"

int main(void) {
    int f;
    for (f = 0; f <= 4; ++f) {
        Context *context = cntx_f();
        importable_file **files;
        fault = f; closes = opens = 0;
        context->ast = ast_f(context, PROGRAM_FILE, NULL);
        context->location = "mock-root";
        context->file_name = "consumer.crexx";
        context->initial_source_extension = strdup("crexx");
        files = rxfl_lst(context);
        if (f >= 1 && f <= 3) {
            int previous_opens = opens;
            assert(!files);
            assert(context->import_discovery_error == (f == 3 ? EBADF : EIO));
            assert(error_in_node(context->ast));
            assert(!rxfl_lst(context) && opens == previous_opens);
            /* A new parse must be allowed to rebuild discovery after a
             * transient iterator failure in the previous source buffer. */
            fault = 0;
            cntx_buf(context, strdup(""), 0);
            assert(!context->import_discovery_error);
            files = rxfl_lst(context);
            assert(files);
            rxfl_fre(files);
        } else {
            assert(files && !context->import_discovery_error);
            assert(!error_in_node(context->ast));
            rxfl_fre(files);
        }
        if (f == 2 || f == 3) assert(closes == opens);
        fre_cntx(context);
    }
    puts("PASS compiler propagates open, enumeration and close failures; missing optional roots stay empty");
    return 0;
}
