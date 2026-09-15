/* Exercise the real plugin's I/O-failure cleanup without crashing the test host.
 * Only fclose(NULL) is intercepted: ordinary file operations remain real. */
#include <stdio.h>
#include <stdlib.h>

static unsigned null_closes;
static int checked_fclose(FILE *stream) {
    if (!stream) {
        ++null_closes;
        return EOF;
    }
    return fclose(stream);
}

#define BUILD_DLL
#define fclose checked_fclose
#include "keyaccess.c"
#undef fclose

static rxinteger get_integer(rxpa_attribute_value value) {
    return *(rxinteger *)value;
}

static void set_integer(rxpa_attribute_value value, rxinteger number) {
    *(rxinteger *)value = number;
}

static void set_string(rxpa_attribute_value value, const char *text) {
    (void)value;
    (void)text;
}

int main(void) {
    struct FileHandle *handle = calloc(1, sizeof(*handle));
    rxinteger argument, result = 123, signal = 123;
    rxpa_attribute_value args[] = { &argument };
    int failed = 0;
    if (!handle) return 2;
    handle->dataFile = tmpfile();
    handle->indexFile = tmpfile();
    /* A regular file cannot be a parent directory. This forces replacement
     * failure even for privileged test users, on both POSIX and Windows. */
    FILE *blocker = fopen("compact_failure_parent", "wb");
    if (!blocker || !handle->dataFile || !handle->indexFile) return 2;
    fclose(blocker);
    handle->dataPath = strdup("compact_failure_parent/data");
    handle->indexPath = strdup("compact_failure_parent/index");
    if (!handle->dataPath || !handle->indexPath) return 2;
    argument = (rxinteger)(intptr_t)handle;
    _rxpa_context->getint = get_integer;
    _rxpa_context->setint = set_integer;
    _rxpa_context->setstring = set_string;

    compact_database(1, args, &result, &signal);
    if (result != KA_ERROR_IO || handle->dataFile || handle->indexFile) {
        fprintf(stderr, "Expected replacement failure with closed streams\n");
        failed = 1;
    }
    closefile(1, args, &result, &signal);
    if (result != KA_SUCCESS || null_closes) {
        fprintf(stderr, "Cleanup returned %lld and attempted %u null closes\n",
                (long long)result, null_closes);
        failed = 1;
    }
    remove("compact_failure_parent");
    return failed;
}
