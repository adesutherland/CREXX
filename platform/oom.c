/* Bounded, allocation-free exhaustion diagnostics. */
#include "platform.h"
#include "text_codec.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#include <io.h>
#elif defined(__unix__) || defined(__APPLE__) || defined(CREXX_MAINFRAME_ELF)
#include <unistd.h>
#endif
#if defined(CREXX_NATIVE_RAW_IO)
void crexx_native_panic(const char *, size_t);
#endif

/* Exhaustion reporting cannot call stdio, strerror, locale or heap-backed
 * memory introspection. Keep the complete diagnostic in a bounded stack buffer
 * and use the platform's unbuffered output service. Failure is best effort and
 * never recurses into allocation or diagnostics. */
typedef struct oom_message { char bytes[2048]; size_t used; } oom_message;
static void oom_text(oom_message *message, const char *text) {
    if (!text) return;
    crexx_utf8_state state = {0, 0, 0};
    while (*text && message->used + 1 < sizeof(message->bytes)) {
        uint32_t scalar;
        unsigned char bytes[4];
        int status = crexx_utf8_feed(&state, (unsigned char)*text++, &scalar);
        int length;
        if (!status) continue;
        if (status < 0) {
            /* Panic reporting remains useful even for an invalid native path.
             * Normal text streams never use this emergency replacement rule. */
            state.remaining = 0;
            scalar = '?';
        }
        length = crexx_utf8_emit(scalar, bytes);
        if (message->used + (size_t)length >= sizeof(message->bytes)) break;
        memcpy(message->bytes + message->used, bytes, (size_t)length);
        message->used += (size_t)length;
    }
}
static void oom_number(oom_message *message, unsigned long long value) {
    char digits[32];
    size_t count = 0;
    do { digits[count++] = (char)('0' + value % 10u); value /= 10u; } while (value);
    while (count && message->used + 1 < sizeof(message->bytes))
        message->bytes[message->used++] = digits[--count];
}
static void oom_write(const char *bytes, size_t length) {
    while (length) {
        ptrdiff_t written;
#if defined(CREXX_NATIVE_RAW_IO)
        /* The platform adapter encodes UTF8 to the native emergency service. */
        crexx_native_panic(bytes, length);
        return;
#elif defined(_WIN32)
        written = _write(2, bytes, (unsigned int)length);
#elif defined(__unix__) || defined(__APPLE__) || defined(CREXX_MAINFRAME_ELF)
        written = write(2, bytes, length);
#else
        /* Legacy hosts must provide the same allocation-free primitive. */
        extern ptrdiff_t crexx_host_write_stderr(const char *, size_t);
        written = crexx_host_write_stderr(bytes, length);
#endif
        if (written < 0 && errno == EINTR) continue;
        if (written <= 0 || (size_t)written > length) break;
        bytes += written;
        length -= (size_t)written;
    }
}
void rx_report_out_of_memory(const char *operation, size_t requested_bytes,
                             const char *detail, const char *source_file,
                             int source_line, const char *function_name) {
    int saved_errno = errno;
    oom_message message;
    message.used = 0;
    oom_text(&message, "PANIC: Out of memory\n  allocation: ");
    oom_text(&message, operation);
    oom_text(&message, "\n  requested bytes: ");
    if (requested_bytes == RX_OOM_UNKNOWN_SIZE) oom_text(&message, "unknown");
    else oom_number(&message, (unsigned long long)requested_bytes);
    if (detail && *detail) { oom_text(&message, "\n  detail: "); oom_text(&message, detail); }
    oom_text(&message, "\n  source: "); oom_text(&message, source_file);
    oom_text(&message, ":"); oom_number(&message, source_line < 0 ? 0u : (unsigned)source_line);
    oom_text(&message, " ("); oom_text(&message, function_name); oom_text(&message, ")\n  errno: ");
    if (saved_errno < 0) oom_text(&message, "-");
    oom_number(&message, saved_errno < 0 ? 0u - (unsigned)saved_errno : (unsigned)saved_errno);
    oom_text(&message, "\n");
    /* A truncated detail still terminates the bounded diagnostic. */
    if (!message.used || message.bytes[message.used - 1] != '\n')
        message.bytes[message.used++] = '\n';
    oom_write(message.bytes, message.used);
    errno = saved_errno;
}

void rx_panic_out_of_memory(const char *operation, size_t requested_bytes,
                            const char *detail, const char *source_file,
                            int source_line, const char *function_name) {
    rx_report_out_of_memory(operation, requested_bytes, detail,
                            source_file, source_line, function_name);
    exit(-1);
}
