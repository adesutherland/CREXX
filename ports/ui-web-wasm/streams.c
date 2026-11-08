/* Browser byte streams. Only stdin may suspend the single VM owner. */
#include <emscripten.h>
#include <wasi/api.h>
#include <stddef.h>
#include <stdint.h>

__wasi_errno_t __real___wasi_fd_read(__wasi_fd_t, const __wasi_iovec_t *, size_t, __wasi_size_t *);
__wasi_errno_t __real___wasi_fd_write(__wasi_fd_t, const __wasi_ciovec_t *, size_t, __wasi_size_t *);

EM_ASYNC_JS(int, ui_stream_read, (void *out, size_t capacity), {
    try {
        const bytes = await Module.uiRead(capacity);
        if (bytes === null) return 0;
        if (!(bytes instanceof Uint8Array) || !bytes.length || bytes.length > capacity)
            throw new Error('Invalid UI input chunk');
        HEAPU8.set(bytes, out);
        return bytes.length;
    } catch (error) {
        Module.uiStreamError = String(error);
        return -1;
    }
});

EM_JS(int, ui_stream_write, (int fd, const void *vectors, size_t count), {
    let total = 0;
    for (let i = 0; i < count; i++) total += HEAPU32[(vectors + i * 8 + 4) >> 2];
    if (total > 16777216) return -1;
    const bytes = new Uint8Array(total);
    let offset = 0;
    for (let i = 0; i < count; i++) {
        const ptr = HEAPU32[(vectors + i * 8) >> 2];
        const size = HEAPU32[(vectors + i * 8 + 4) >> 2];
        bytes.set(HEAPU8.subarray(ptr, ptr + size), offset);
        offset += size;
    }
    try { if (total) Module.uiWrite(fd, bytes); return total; }
    catch (error) { Module.uiStreamError = String(error); return -1; }
});

__wasi_errno_t __wrap___wasi_fd_read(__wasi_fd_t fd, const __wasi_iovec_t *iov,
        size_t count, __wasi_size_t *received) {
    if (fd != 0) return __real___wasi_fd_read(fd, iov, count, received);
    *received = 0;
    for (size_t i = 0; i < count; i++) {
        if (iov[i].buf_len) {
            int size = ui_stream_read(iov[i].buf, iov[i].buf_len);
            if (size < 0) return __WASI_ERRNO_IO;
            *received = (__wasi_size_t)size;
            break;
        }
    }
    return 0;
}

__wasi_errno_t __wrap___wasi_fd_write(__wasi_fd_t fd, const __wasi_ciovec_t *iov,
        size_t count, __wasi_size_t *written) {
    if (fd != 1 && fd != 2) return __real___wasi_fd_write(fd, iov, count, written);
    *written = 0;
    int size = ui_stream_write(fd, iov, count);
    if (size < 0) return __WASI_ERRNO_IO;
    *written = (__wasi_size_t)size;
    return 0;
}
