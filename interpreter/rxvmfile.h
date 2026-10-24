/* Private FOPEN implementation: establish inheritance policy at creation,
 * before another worker can launch a child. Standard streams bypass this. */
#ifndef CREXX_RXVMFILE_H
#define CREXX_RXVMFILE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "platform.h"
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <sys/stat.h>
#elif defined(__APPLE__)
#include <fcntl.h>
#include <unistd.h>
#endif

static FILE *rxvm_private_fopen(const char *path, const char *mode) {
#if defined(CREXX_MAINFRAME_ELF)
    /* Native profiles have no child-process inheritance and libc may reject
     * Linux's e modifier. Use the logical-name/text boundary exactly once. */
    return platform_fopen(path, mode);
#elif defined(__APPLE__)
    /* Darwin fopen does not implement the 'e' modifier. Preserve its C mode
     * parsing and initial append position, but open atomically close-on-exec. */
    int flags, fd, saved_errno;
    const char *p = mode;
    FILE *stream;
    switch (*p++) {
        case 'r': flags = O_RDONLY; break;
        case 'w': flags = O_WRONLY | O_CREAT | O_TRUNC; break;
        case 'a': flags = O_WRONLY | O_CREAT | O_APPEND; break;
        default: errno = EINVAL; return NULL;
    }
    if (*p == 'b') p++;
    if (*p == '+') {
        flags = (flags & ~O_ACCMODE) | O_RDWR;
        p++;
        if (*p == 'b') p++;
    }
    if (*p == 'x') flags |= O_EXCL;
    fd = open(path, flags | O_CLOEXEC, 0666);
    if (fd < 0) return NULL;
    stream = fdopen(fd, mode);
    if (!stream) {
        saved_errno = errno;
        close(fd);
        errno = saved_errno;
    } else if (flags & O_APPEND) {
        (void)fseek(stream, 0, SEEK_END);
    }
    return stream;
#else
#ifdef _WIN32
    /* Some CRTs reject fopen's standard x mode. Own exclusivity and inheritance
     * at descriptor creation; fdopen takes ownership only after it succeeds. */
    if (strchr(mode, 'x')) {
        const char *stream_mode;
        int flags = _O_CREAT | _O_EXCL | _O_NOINHERIT;
        int fd = -1, error, saved_errno;
        FILE *stream;
        if (!strcmp(mode, "wx")) stream_mode = "w";
        else if (!strcmp(mode, "wbx")) stream_mode = "wb";
        else if (!strcmp(mode, "w+x")) stream_mode = "w+";
        else if (!strcmp(mode, "wb+x")) stream_mode = "wb+";
        else if (!strcmp(mode, "w+bx")) stream_mode = "w+b";
        else { errno = EINVAL; return NULL; }
        flags |= strchr(stream_mode, '+') ? _O_RDWR : _O_WRONLY;
        /* Omitted b retains the CRT's default translation policy. */
        if (strchr(stream_mode, 'b')) flags |= _O_BINARY;
        error = _sopen_s(&fd, path, flags, _SH_DENYNO, _S_IREAD | _S_IWRITE);
        if (error) { errno = error; return NULL; }
        stream = _fdopen(fd, stream_mode);
        if (!stream) {
            saved_errno = errno;
            _close(fd);
            errno = saved_errno;
        }
        return stream;
    }
#endif
    /* Put N/e before any ',ccs=...' encoding suffix. libc/CRT retains all
     * mode validation and text/binary/append/encoding behavior. */
    size_t length = strlen(mode);
#ifdef _WIN32
    size_t prefix = strcspn(mode, ",");
#else
    size_t prefix = 1u;
#endif
    char *private_mode;
    FILE *stream;
    int saved_errno;
    if (!length) { errno = EINVAL; return NULL; }
    private_mode = (char *)malloc(length + 2u);
    if (!private_mode) { errno = ENOMEM; return NULL; }
    memcpy(private_mode, mode, prefix);
#ifdef _WIN32
    private_mode[prefix] = 'N';
#else
    private_mode[prefix] = 'e';
#endif
    memcpy(private_mode + prefix + 1u, mode + prefix, length - prefix + 1u);
    stream = fopen(path, private_mode);
    saved_errno = errno;
    free(private_mode);
    errno = saved_errno;
    return stream;
#endif
}
#endif
