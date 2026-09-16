/* Additional reusable rxfs operations. Included by rxfs.c; MIT licence. */
#ifdef _WIN32
#include <wchar.h>
static wchar_t *rxfs_wide(const char *s) {
    int n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, NULL, 0);
    wchar_t *p = n ? (wchar_t *)malloc((size_t)n * sizeof(*p)) : NULL;
    if (p && !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, p, n)) { free(p); p = NULL; }
    return p;
}
#else
#include <fcntl.h>
#include <sys/file.h>
#ifdef __linux__
#include <sys/syscall.h>
#endif
#endif

PROCEDURE(path_kind) {
    int result = -1;
    if (NUM_ARGS != 1) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.PATHKIND expects one path") }
#ifdef _WIN32
    wchar_t *p = rxfs_wide(GETSTRING(ARG0));
    if (p) {
        DWORD flags = GetFileAttributesW(p);
        if (flags == INVALID_FILE_ATTRIBUTES) {
            DWORD e = GetLastError();
            if (e == ERROR_FILE_NOT_FOUND || e == ERROR_PATH_NOT_FOUND) result = 0;
        } else if (!(flags & FILE_ATTRIBUTE_REPARSE_POINT)) result = (flags & FILE_ATTRIBUTE_DIRECTORY) ? 2 : 1;
        free(p);
    }
#else
    struct stat info;
    if (lstat(GETSTRING(ARG0), &info)) { if (errno == ENOENT || errno == ENOTDIR) result = 0; }
    else if (S_ISREG(info.st_mode)) result = 1;
    else if (S_ISDIR(info.st_mode)) result = 2;
#endif
    RETURNINT(result); RESETSIGNAL
}

PROCEDURE(absolute_path) {
    char output[32768];
    if (NUM_ARGS != 1) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.ABSPATH expects one path") }
    output[0] = 0;
#ifdef _WIN32
    wchar_t resolved[32768];
    wchar_t *p = rxfs_wide(GETSTRING(ARG0));
    if (p) {
        DWORD n = GetFullPathNameW(p, 32768, resolved, NULL);
        if (n && n < 32768) WideCharToMultiByte(CP_UTF8, 0, resolved, -1, output, sizeof(output), NULL, NULL);
        free(p);
    }
#else
    /* Do not realpath away a symlink before the Rexx safety walk. */
    const char *p = GETSTRING(ARG0);
    if (p[0] == '/') {
        int written = snprintf(output, sizeof(output), "%s", p);
        if (written < 0 || (size_t)written >= sizeof(output)) output[0] = 0;
    }
    else if (getcwd(output, sizeof(output))) {
        size_t n = strlen(output);
        if (n + strlen(p) + 2 < sizeof(output)) { output[n++] = '/'; strcpy(output + n, p); }
        else output[0] = 0;
    }
#endif
    RETURNSTR(output); RESETSIGNAL
}

/* All three operations refuse an existing destination. No silent overwrite. */
static int rxfs_transfer(const char *source, const char *target, int operation) {
    int result = -8;
#ifdef _WIN32
    wchar_t *a = rxfs_wide(source), *b = rxfs_wide(target);
    if (a && b) {
        DWORD flags = GetFileAttributesW(a);
        if (flags != INVALID_FILE_ATTRIBUTES && !(flags & FILE_ATTRIBUTE_REPARSE_POINT)) {
            if (operation == 0) result = CopyFileW(a, b, TRUE) ? 0 : -8;
            else if (operation == 1) result = CreateHardLinkW(b, a, NULL) ? 0 : -8;
            else result = MoveFileExW(a, b, MOVEFILE_WRITE_THROUGH) ? 0 : -8;
        }
    }
    free(a); free(b);
#else
    struct stat st;
    if (lstat(source, &st) || S_ISLNK(st.st_mode)) return -8;
    if (operation == 1) return S_ISREG(st.st_mode) && link(source, target) == 0 ? 0 : -8;
    if (operation == 2) {
#if defined(__APPLE__)
        return renamex_np(source, target, RENAME_EXCL) == 0 ? 0 : -8;
#elif defined(__linux__) && defined(SYS_renameat2)
        return syscall(SYS_renameat2, AT_FDCWD, source, AT_FDCWD, target, 1 /* RENAME_NOREPLACE */) == 0 ? 0 : -8;
#else
        /* Never substitute a racy overwriting rename on unsupported systems. */
        return -8;
#endif
    }
    if (S_ISREG(st.st_mode)) {
        int src = open(source, O_RDONLY | O_NOFOLLOW), dst = -1;
        if (src >= 0) dst = open(target, O_CREAT | O_EXCL | O_WRONLY | O_NOFOLLOW, st.st_mode & 0777);
        if (dst >= 0) {
            char buffer[65536]; ssize_t n; result = 0;
            while ((n = read(src, buffer, sizeof(buffer))) > 0) {
                ssize_t offset = 0;
                while (offset < n) {
                    ssize_t wrote = write(dst, buffer + offset, (size_t)(n - offset));
                    if (wrote <= 0) { result = -8; break; }
                    offset += wrote;
                }
                if (result) break;
            }
            if (n < 0 || fsync(dst)) result = -8;
            if (close(dst)) result = -8;
            if (result) unlink(target);
        }
        if (src >= 0) close(src);
    }
#endif
    return result;
}
PROCEDURE(copy_file) {
    if (NUM_ARGS != 2) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.COPY expects source and target") }
    RETURNINT(rxfs_transfer(GETSTRING(ARG0), GETSTRING(ARG1), 0)); RESETSIGNAL
}
PROCEDURE(hard_link) {
    if (NUM_ARGS != 2) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.HARDLINK expects source and target") }
    RETURNINT(rxfs_transfer(GETSTRING(ARG0), GETSTRING(ARG1), 1)); RESETSIGNAL
}
PROCEDURE(move_file) {
    if (NUM_ARGS != 2) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.MOVE expects source and target") }
    RETURNINT(rxfs_transfer(GETSTRING(ARG0), GETSTRING(ARG1), 2)); RESETSIGNAL
}

/* VM-owned guards use existing RXPA native values. Copies share one owner;
 * explicit close closes all aliases, and the last finalizer closes the OS
 * handle if the caller omitted close. Guards are never transferable handles. */
typedef struct rxfs_session {
    const rxpa_host_services_v1 *host;
    size_t refs;
} rxfs_session;
typedef struct rxfs_guard {
    rxfs_session *owner;
    size_t refs;
    int status;
#ifdef _WIN32
    HANDLE handle;
#else
    int handle;
#endif
} rxfs_guard;
#ifdef _MSC_VER
#define RXFS_LOCAL __declspec(thread)
#else
#define RXFS_LOCAL __thread
#endif
static RXFS_LOCAL rxfs_session *rxfs_current;
static void *rxfs_create(const rxpa_host_services_v1 *host) {
    rxfs_session *session = (rxfs_session *)calloc(1, sizeof(*session));
    if (session) { session->host = host; session->refs = 1; }
    return session;
}
static void *rxfs_old_create(void) { return rxfs_create(NULL); }
static void rxfs_destroy(void *opaque) {
    rxfs_session *session = (rxfs_session *)opaque;
    if (session && !--session->refs) free(session);
}
static int rxfs_enter(void *session, uint32_t caps, void **previous) {
    (void)caps; *previous = rxfs_current; rxfs_current = (rxfs_session *)session; return 0;
}
static void rxfs_leave(void *previous) { rxfs_current = (rxfs_session *)previous; }
static uint32_t rxfs_caps(const char *name) {
    return name && strstr(name, "rxfs.fileguard") ? RXPA_PROCEDURE_CAP_SESSION_AFFINE : RXPA_PROCEDURE_CAP_PROCESS_REENTRANT;
}
RXPA_PLUGIN_SESSION_WITH_HOST(rxfs_old_create, rxfs_destroy, rxfs_enter, rxfs_leave, rxfs_caps, rxfs_create)
static void rxfs_guard_copy(void *destination, void *source);
static void rxfs_guard_finalize(void *value);
static const rxpa_native_payload_ops rxfs_guard_ops = {"rxfs.fileguard", rxfs_guard_copy, rxfs_guard_finalize};
static rxfs_guard *rxfs_guard_resource(void *value) {
    size_t length; const rxpa_native_payload_ops *ops; rxfs_guard *guard = NULL;
    void *data = GETNATIVEPAYLOAD(value, &length, &ops, NULL);
    if (data && length == sizeof(guard) && ops == &rxfs_guard_ops) memcpy(&guard, data, length);
    return guard;
}
static int rxfs_guard_is_open(rxfs_guard *guard) {
#ifdef _WIN32
    return guard && guard->handle != INVALID_HANDLE_VALUE;
#else
    return guard && guard->handle >= 0;
#endif
}
static int rxfs_guard_release_handle(rxfs_guard *guard) {
    int result = 0;
    if (!rxfs_guard_is_open(guard)) return 0;
#ifdef _WIN32
    if (!CloseHandle(guard->handle)) result = -8;
    guard->handle = INVALID_HANDLE_VALUE;
#else
    if (close(guard->handle)) result = -8;
    guard->handle = -1;
#endif
    return result;
}
static void rxfs_guard_release(rxfs_guard *guard) {
    if (guard && !--guard->refs) {
        rxfs_guard_release_handle(guard); rxfs_destroy(guard->owner); free(guard);
    }
}
static void rxfs_guard_copy(void *destination, void *source) {
    rxfs_guard *guard = rxfs_guard_resource(source);
    if (guard) {
        ++guard->refs;
        if (SETNATIVEPAYLOAD(destination, &guard, sizeof(guard), &rxfs_guard_ops, 0)) rxfs_guard_release(guard);
    }
}
static void rxfs_guard_finalize(void *value) { rxfs_guard_release(rxfs_guard_resource(value)); }
PROCEDURE(make_guard) {
    rxfs_guard *guard;
    const char *mode;
    int lease;
    if (NUM_ARGS != 2) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.FILEGUARD expects a path and explicit mode") }
    mode = GETSTRING(ARG1); lease = !strcmp(mode, "lease");
    if (!lease && strcmp(mode, "exclusive")) {
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXFS.FILEGUARD expects a path and exclusive or lease mode")
    }
    if (!rxfs_current || !rxpa_host_has_object_set_type(rxfs_current->host)) {
        RETURNSIGNAL(SIGNAL_FAILURE, "RXFS.FILEGUARD requires native object host services")
    }
    guard = (rxfs_guard *)calloc(1, sizeof(*guard));
    if (!guard) { RETURNSIGNAL(SIGNAL_FAILURE, "RXFS.FILEGUARD allocation failed") }
    guard->owner = rxfs_current; ++rxfs_current->refs; guard->refs = 1;
#ifdef _WIN32
    {
        wchar_t *path = rxfs_wide(GETSTRING(ARG0));
        guard->handle = INVALID_HANDLE_VALUE;
        if (path) {
            DWORD attributes = GetFileAttributesW(path);
            if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
                guard->handle = CreateFileW(path, GENERIC_READ | GENERIC_WRITE | (lease ? DELETE : 0),
                    lease ? FILE_SHARE_DELETE : 0, NULL, lease ? OPEN_EXISTING : OPEN_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
                if (guard->handle != INVALID_HANDLE_VALUE) {
                    BY_HANDLE_FILE_INFORMATION info;
                    if (!GetFileInformationByHandle(guard->handle, &info) ||
                        (info.dwFileAttributes & (FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_DIRECTORY))) {
                        CloseHandle(guard->handle); guard->handle = INVALID_HANDLE_VALUE;
                    }
                }
            }
            free(path);
        }
    }
#else
    guard->handle = open(GETSTRING(ARG0), O_RDWR | O_NOFOLLOW | (lease ? 0 : O_CREAT), 0600);
    if (guard->handle >= 0) {
        struct stat info;
        if (fstat(guard->handle, &info) || !S_ISREG(info.st_mode) || flock(guard->handle, LOCK_EX | LOCK_NB)) {
            close(guard->handle); guard->handle = -1;
        }
    }
#endif
    guard->status = rxfs_guard_is_open(guard) ? 0 : -8;
    if (SETNATIVEPAYLOAD(RETURN, &guard, sizeof(guard), &rxfs_guard_ops, 0)) {
        rxfs_guard_release(guard); RETURNSIGNAL(SIGNAL_FAILURE, "RXFS.FILEGUARD publication failed")
    }
    SETNUMATTRS(RETURN, 0);
    if (SETOBJECTTYPE(rxfs_current->host, RETURN, "rxfs.fileguard")) {
        RETURNSIGNAL(SIGNAL_FAILURE, "RXFS.FILEGUARD type publication failed")
    }
    RESETSIGNAL
}
METHODPROCEDURE(guard_held) {
    rxfs_guard *guard = rxfs_guard_resource(ARG0);
    if (!guard || guard->owner != rxfs_current) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Invalid RXFS.FILEGUARD owner") }
    RETURNINT(rxfs_guard_is_open(guard)); RESETSIGNAL
}
METHODPROCEDURE(guard_status) {
    rxfs_guard *guard = rxfs_guard_resource(ARG0);
    if (!guard || guard->owner != rxfs_current) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Invalid RXFS.FILEGUARD owner") }
    RETURNINT(guard->status); RESETSIGNAL
}
METHODPROCEDURE(guard_close) {
    rxfs_guard *guard = rxfs_guard_resource(ARG0);
    if (!guard || guard->owner != rxfs_current) { RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "Invalid RXFS.FILEGUARD owner") }
    guard->status = rxfs_guard_release_handle(guard);
    RETURNINT(guard->status); RESETSIGNAL
}
