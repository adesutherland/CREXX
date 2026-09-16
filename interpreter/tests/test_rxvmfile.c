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
static FILE *checked_open(const char *path, const char *mode) {
    FILE *fp = rxvm_private_fopen(path, mode);
    CHECK(fp != NULL, mode);
    if (fp) {
#ifdef _WIN32
        DWORD flags;
        CHECK(GetHandleInformation((HANDLE)_get_osfhandle(_fileno(fp)), &flags)
              && !(flags & HANDLE_FLAG_INHERIT), "private from creation");
#else
        CHECK((fcntl(fileno(fp), F_GETFD) & FD_CLOEXEC) != 0, "private from creation");
#endif
    }
    return fp;
}
int main(void) {
    char path[1024];
    FILE *fp;
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
    fp = checked_open(path, "ab");
    if (!fp) return 2;
    CHECK(ftell(fp) == 3, "initial append position");
    CHECK(fwrite("two", 1, 3, fp) == 3, "append"); fclose(fp);
    fp = checked_open(path, "rb");
    if (!fp) return 2;
    CHECK(fseek(fp, 0, SEEK_END) == 0 && ftell(fp) == 6, "append retains bytes");
    fclose(fp);
    errno = 0;
    fp = rxvm_private_fopen(path, "wbx");
    CHECK(fp == NULL && errno == EEXIST, "exclusive open preserves existing file");
    if (fp) fclose(fp);
    CHECK(remove(path) == 0, "file cleanup");
    fp = rxvm_private_fopen(path, "rb");
    CHECK(fp == NULL && errno == ENOENT, "failed open preserves errno");
    if (fp) fclose(fp);
    printf("FOPEN inheritance: %d failures\n", failures);
    return failures ? 1 : 0;
}
