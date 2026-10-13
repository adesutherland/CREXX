#include <stdlib.h>
#include <unistd.h>

/* Compile-time wrapper applied only to rxvmintp.c in the diagnostic build. */
void *review_calloc(size_t count, size_t size) {
    if (count == 1 && size == 112) {
        static const char message[] = "REVIEW: index allocation fault\n";
        write(2, message, sizeof(message) - 1);
        return NULL;
    }
    return calloc(count, size);
}
