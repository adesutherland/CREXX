/* The FOPEN primitive must produce a private descriptor at its first return. */
#include "rxvmfile.h"
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif
static int failures;
#define CHECK(c,m) do { if (!(c)) { fprintf(stderr, "FAIL: %s\n", m); failures++; } } while (0)
static void check_private(FILE *fp) {
    if (fp) {
#ifdef _WIN32
        DWORD flags;
        CHECK(GetHandleInformation((HANDLE)_get_osfhandle(_fileno(fp)), &flags)
              && !(flags & HANDLE_FLAG_INHERIT), "private from creation");
#else
        CHECK((fcntl(fileno(fp), F_GETFD) & FD_CLOEXEC) != 0, "private from creation");
#endif
    }
}
static FILE *checked_open(const char *path, const char *mode) {
    FILE *fp = rxvm_private_fopen(path, mode);
    CHECK(fp != NULL, mode);
    check_private(fp);
    return fp;
}

/* Each diagnostic uses a distinct absent path in our owned directory. Even the
 * nonexclusive CRT control must never overwrite the failed test's file. */
static void exclusive_diagnostics(const char *path, int wrapper_errno) {
    static const char *modes[] = {"wbx", "wbxN", "wb", "wbx"};
    char probe[1100];
    size_t i;
    fprintf(stderr, "exclusive create failed: %s wrapper errno=%d\n", path, wrapper_errno);
    for (i = 0; i < sizeof(modes) / sizeof(modes[0]); i++) {
        FILE *fp;
        int open_errno, result;
        snprintf(probe, sizeof(probe), "%s.probe-%u", path, (unsigned)i);
        errno = 0;
        fp = i == 3 ? rxvm_private_fopen(probe, modes[i]) : fopen(probe, modes[i]);
        open_errno = errno;
        fprintf(stderr, "  %s mode=%s opened=%d errno=%d\n",
                i == 3 ? "wrapper" : "CRT", modes[i], fp != NULL, open_errno);
        if (fp) {
            result = fclose(fp);
            CHECK(result == 0, "diagnostic close");
            result = remove(probe);
            CHECK(result == 0, "diagnostic cleanup");
        }
    }
}

static void check_exclusive_new(const char *path) {
    char contents[4];
    FILE *fp;
    size_t count;
    int open_errno, result, tail;
    errno = 0;
    fp = rxvm_private_fopen(path, "wbx");
    open_errno = errno;
    if (!fp) exclusive_diagnostics(path, open_errno);
    CHECK(fp != NULL, "exclusive mode creates an absent file");
    if (!fp) return;
    check_private(fp);
    count = fwrite("safe", 1, 4, fp);
    CHECK(count == 4, "exclusive file write");
    result = fclose(fp);
    CHECK(result == 0, "exclusive file close");

    fp = rxvm_private_fopen(path, "wbx");
    CHECK(fp == NULL, "exclusive mode rejects an existing file");
    if (fp) {
        result = fclose(fp);
        CHECK(result == 0, "unexpected exclusive handle close");
    }
    fp = checked_open(path, "rb");
    if (fp) {
        count = fread(contents, 1, sizeof(contents), fp);
        tail = fgetc(fp);
        CHECK(count == sizeof(contents) && !memcmp(contents, "safe", sizeof(contents))
              && tail == EOF, "exclusive rejection does not clobber bytes");
        result = fclose(fp);
        CHECK(result == 0, "exclusive verification close");
    }
    result = remove(path);
    CHECK(result == 0, "exclusive file cleanup");
}

static void check_exclusive_paths(void) {
    char directory[1024], path[1060];
    int result;
#ifdef _WIN32
    char root[MAX_PATH];
    DWORD length = GetTempPathA(sizeof(root), root);
    if (!length || length >= sizeof(root)
        || !GetTempFileNameA(root, "cxf", 0, directory)) {
        CHECK(0, "exclusive test directory reservation");
        return;
    }
    if (!DeleteFileA(directory) || !CreateDirectoryA(directory, NULL)) {
        CHECK(0, "exclusive test directory creation");
        return;
    }
#else
    strcpy(directory, "/tmp/crexx-exclusive-XXXXXX");
    if (!mkdtemp(directory)) {
        CHECK(0, "exclusive test directory creation");
        return;
    }
#endif
    snprintf(path, sizeof(path), "%s/new.bin", directory);
    check_exclusive_new(path);
    snprintf(path, sizeof(path), "%s/path with spaces.bin", directory);
    check_exclusive_new(path);
#ifdef _WIN32
    result = RemoveDirectoryA(directory) ? 0 : -1;
#else
    result = rmdir(directory);
#endif
    CHECK(result == 0, "exclusive test directory cleanup");
}
int main(void) {
    char path[1024];
    FILE *fp;
    long append_position;
    int exclusive_opened, exclusive_errno;
#ifdef _WIN32
    char root[MAX_PATH];
    GetTempPathA(sizeof(root), root);
    GetTempFileNameA(root, "701", 0, path);
#else
    int fd;
    strcpy(path, "/tmp/crexx-701-file-XXXXXX");
    fd = mkstemp(path);
    if (fd < 0) return 2;
    close(fd);
#endif
    fp = checked_open(path, "wb+");
    if (!fp) return 2;
    CHECK(fwrite("one", 1, 3, fp) == 3, "write binary");
    CHECK(fseek(fp, 0, SEEK_SET) == 0 && fgetc(fp) == 'o', "update read");
    fclose(fp);
    fp = fopen(path, "ab");
    if (!fp) return 2;
    append_position = ftell(fp);
    fclose(fp);
    fp = checked_open(path, "ab");
    if (!fp) return 2;
    CHECK(ftell(fp) == append_position, "initial append position matches host CRT");
    CHECK(fwrite("two", 1, 3, fp) == 3, "append"); fclose(fp);
    fp = checked_open(path, "rb");
    if (!fp) return 2;
    CHECK(fseek(fp, 0, SEEK_END) == 0 && ftell(fp) == 6, "append retains bytes");
    fclose(fp);
    errno = 0;
    fp = fopen(path, "wbx");
    exclusive_opened = fp != NULL;
    exclusive_errno = errno;
    if (fp) fclose(fp);
    errno = 0;
    fp = rxvm_private_fopen(path, "wbx");
    CHECK((fp != NULL) == exclusive_opened &&
          (fp != NULL || errno == exclusive_errno),
          "exclusive mode retains host CRT capability and errno");
    if (fp) fclose(fp);
    CHECK(remove(path) == 0, "file cleanup");
    fp = rxvm_private_fopen(path, "rb");
    CHECK(fp == NULL && errno == ENOENT, "failed open preserves errno");
    if (fp) fclose(fp);
    check_exclusive_paths();
    printf("FOPEN inheritance: %d failures\n", failures);
    return failures ? 1 : 0;
}
