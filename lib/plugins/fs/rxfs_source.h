/* Exact, bounded regular-resource acquisition. MIT licence.
 * Search/base policy belongs to the language client, which supplies a resolved
 * locator. This reader performs no directory, suffix or executable search. */
#include <stdint.h>

typedef struct rxfs_source {
    rxfs_session *owner;
    size_t refs, length;
    unsigned char *bytes;
    int status, error;
} rxfs_source;
static void rxfs_source_copy(void *, void *);
static void rxfs_source_finalize(void *);
static const rxpa_native_payload_ops rxfs_source_ops = {
    "rxfs.source", rxfs_source_copy, rxfs_source_finalize
};
static rxfs_source *rxfs_source_resource(void *value) {
    size_t length; const rxpa_native_payload_ops *ops;
    rxfs_source *source = NULL;
    void *data = GETNATIVEPAYLOAD(value, &length, &ops, NULL);
    if (data && length == sizeof(source) && ops == &rxfs_source_ops)
        memcpy(&source, data, sizeof(source));
    return source;
}
static void rxfs_source_release(rxfs_source *source) {
    if (source && !--source->refs) {
        free(source->bytes); rxfs_destroy(source->owner); free(source);
    }
}
static void rxfs_source_copy(void *destination, void *original) {
    rxfs_source *source = rxfs_source_resource(original);
    if (source) {
        ++source->refs;
        if (SETNATIVEPAYLOAD(destination, &source, sizeof(source), &rxfs_source_ops, 0))
            rxfs_source_release(source);
    }
}
static void rxfs_source_finalize(void *value) { rxfs_source_release(rxfs_source_resource(value)); }

PROCEDURE(context_cwd) {
    if (NUM_ARGS || !rxfs_current ||
        !rxpa_host_has_resource_view(rxfs_current->host)) {
        RETURNSIGNAL(SIGNAL_FAILURE, "RXFS.CONTEXTCWD requires an execution resource view")
    }
    if (rxfs_current->host->resource_view(RETURN)) {
        RETURNSIGNAL(SIGNAL_FAILURE, "RXFS.CONTEXTCWD snapshot failed")
    }
    RESETSIGNAL
}
PROCEDURE(host_windows) {
    if (NUM_ARGS) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.WINDOWS expects no arguments") }
#ifdef _WIN32
    RETURNINT(1);
#else
    RETURNINT(0);
#endif
    RESETSIGNAL
}

PROCEDURE(locator_equal) {
    const char *left, *right;
    size_t left_length, right_length;
    int equal;
    if (NUM_ARGS != 2 || !rxfs_current || !rxpa_host_has_string_view(rxfs_current->host) ||
        rxfs_current->host->string_view(ARG0, &left, &left_length) ||
        rxfs_current->host->string_view(ARG1, &right, &right_length) ||
        memchr(left, 0, left_length) || memchr(right, 0, right_length)) {
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.LOCATOREQUAL expects two NUL-free locators")
    }
#ifdef _WIN32
    {
        wchar_t *a = rxfs_wide(GETSTRING(ARG0)), *b = rxfs_wide(GETSTRING(ARG1));
        size_t i;
        if (!a || !b) { free(a); free(b); RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Invalid UTF8 locator") }
        for (i = 0; a[i]; ++i) if (a[i] == L'/') a[i] = L'\\';
        for (i = 0; b[i]; ++i) if (b[i] == L'/') b[i] = L'\\';
        equal = CompareStringOrdinal(a, -1, b, -1, TRUE) == CSTR_EQUAL;
        free(a); free(b);
    }
#else
    equal = left_length == right_length && !memcmp(left, right, left_length);
#endif
    RETURNINT(equal); RESETSIGNAL
}

PROCEDURE(make_source) {
    rxfs_source *source;
    const char *path;
    size_t path_length, budget, capacity = 0;
    rxinteger allowance;
    unsigned char chunk[32768];
#ifdef _WIN32
    HANDLE input = INVALID_HANDLE_VALUE;
#else
    int input = -1;
#endif
    if (NUM_ARGS != 2 || !rxfs_current ||
        !rxpa_host_has_string_view(rxfs_current->host) ||
        !rxpa_host_has_object_set_type(rxfs_current->host)) {
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.SOURCE requires a path, byte budget and host services")
    }
    allowance = GETINT(ARG1);
    if (allowance < 0 || (uint64_t)allowance > (uint64_t)SIZE_MAX ||
        rxfs_current->host->string_view(ARG0, &path, &path_length) ||
        !path_length || memchr(path, 0, path_length)) {
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.SOURCE requires a nonempty NUL-free path and representable byte budget")
    }
    /* GETSTRING provides the terminator only after length-bearing validation. */
    path = GETSTRING(ARG0); budget = (size_t)allowance;
    source = (rxfs_source *)calloc(1, sizeof(*source));
    if (!source) { RETURNSIGNAL(SIGNAL_FAILURE, "RXFS.SOURCE allocation failed") }
    source->owner = rxfs_current; ++rxfs_current->refs; source->refs = 1;
    source->status = -1;
#ifdef _WIN32
    {
        wchar_t *wide = rxfs_wide(path);
        if (!wide) { source->error = ERROR_NO_UNICODE_TRANSLATION; goto publish_source; }
        input = CreateFileW(wide, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        free(wide);
        if (input == INVALID_HANDLE_VALUE) {
            source->error = (int)GetLastError();
            if (source->error == ERROR_FILE_NOT_FOUND || source->error == ERROR_PATH_NOT_FOUND)
                source->status = 0;
            goto publish_source;
        }
        {
            BY_HANDLE_FILE_INFORMATION info;
            DWORD type = GetFileType(input);
            if (type != FILE_TYPE_DISK) {
                source->error = type == FILE_TYPE_UNKNOWN ? (int)GetLastError() : ERROR_INVALID_DATA;
                if (!source->error) source->error = ERROR_INVALID_DATA;
                goto close_source;
            }
            if (!GetFileInformationByHandle(input, &info)) {
                source->error = (int)GetLastError(); goto close_source;
            }
            if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                source->error = ERROR_DIRECTORY; goto close_source;
            }
            if (((uint64_t)info.nFileSizeHigh << 32 | info.nFileSizeLow) > budget) {
                source->status = -2; goto close_source;
            }
        }
    }
#else
    input = open(path, O_RDONLY | O_NONBLOCK);
    if (input < 0) {
        source->error = errno;
        if (errno == ENOENT || errno == ENOTDIR) source->status = 0;
        goto publish_source;
    }
    {
        struct stat info;
        if (fstat(input, &info)) { source->error = errno; goto close_source; }
        if (!S_ISREG(info.st_mode)) { source->error = EINVAL; goto close_source; }
        if (info.st_size < 0 || (uint64_t)info.st_size > budget) {
            source->status = -2; goto close_source;
        }
    }
#endif
    for (;;) {
        size_t wanted = budget - source->length, got;
        if (wanted > sizeof(chunk)) wanted = sizeof(chunk);
        /* One fixed-size probe distinguishes an exact-bound EOF from overflow. */
        if (!wanted) wanted = 1;
#ifdef _WIN32
        {
            DWORD count;
            if (!ReadFile(input, chunk, (DWORD)wanted, &count, NULL)) {
                source->error = (int)GetLastError(); goto close_source;
            }
            got = count;
        }
#else
        {
            ssize_t count;
            do { count = read(input, chunk, wanted); } while (count < 0 && errno == EINTR);
            if (count < 0) { source->error = errno; goto close_source; }
            got = (size_t)count;
        }
#endif
        if (!got) { source->status = 1; break; }
        if (got > budget - source->length) { source->status = -2; break; }
        if (source->length + got > capacity) {
            unsigned char *resized;
            capacity = source->length + got;
            resized = (unsigned char *)realloc(source->bytes, capacity);
            if (!resized) {
#ifdef _WIN32
                source->error = ERROR_NOT_ENOUGH_MEMORY;
#else
                source->error = ENOMEM;
#endif
                goto close_source;
            }
            source->bytes = resized;
        }
        memcpy(source->bytes + source->length, chunk, got); source->length += got;
    }
close_source:
#ifdef _WIN32
    if (!CloseHandle(input) && source->status == 1) { source->status = -1; source->error = (int)GetLastError(); }
#else
    if (close(input) && source->status == 1) { source->status = -1; source->error = errno; }
#endif
publish_source:
    if (source->status != 1) { free(source->bytes); source->bytes = NULL; source->length = 0; }
    if (SETNATIVEPAYLOAD(RETURN, &source, sizeof(source), &rxfs_source_ops, 0)) {
        rxfs_source_release(source); RETURNSIGNAL(SIGNAL_FAILURE, "RXFS.SOURCE publication failed")
    }
    SETNUMATTRS(RETURN, 0);
    if (SETOBJECTTYPE(rxfs_current->host, RETURN, "rxfs.source")) {
        RETURNSIGNAL(SIGNAL_FAILURE, "RXFS.SOURCE type publication failed")
    }
    RESETSIGNAL
}
METHODPROCEDURE(source_status) {
    rxfs_source *source = rxfs_source_resource(ARG0);
    if (!source || source->owner != rxfs_current) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Invalid RXFS.SOURCE owner") }
    RETURNINT(source->status); RESETSIGNAL
}
METHODPROCEDURE(source_error) {
    rxfs_source *source = rxfs_source_resource(ARG0);
    if (!source || source->owner != rxfs_current) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Invalid RXFS.SOURCE owner") }
    RETURNINT(source->error); RESETSIGNAL
}
METHODPROCEDURE(source_bytes) {
    rxfs_source *source = rxfs_source_resource(ARG0);
    if (!source || source->owner != rxfs_current || source->status != 1) {
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.SOURCE bytes require a found resource")
    }
    if (SETNATIVEPAYLOAD(RETURN, source->bytes, source->length, NULL, 0)) {
        RETURNSIGNAL(SIGNAL_FAILURE, "RXFS.SOURCE byte copy failed")
    }
    RESETSIGNAL
}
