#include "base58id.h"
#include <string.h>

/* ----- CSPRNG (Windows / macOS-BSD / Linux / generic) ----- */
#if defined(_WIN32)
#include <windows.h>
#ifndef RtlGenRandom
extern BOOLEAN NTAPI SystemFunction036(PVOID RandomBuffer, ULONG RandomBufferLength);
#define RtlGenRandom SystemFunction036
#endif
// static int csprng(void *buf, size_t len) { return RtlGenRandom(buf, (ULONG)len) ? 1 : 0; }  // defined somewhere else

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <stdlib.h>
  static int csprng(void *buf, size_t len) { arc4random_buf(buf, len); return 1; }

#elif defined(__linux__)
  #include <sys/random.h>
  #include <unistd.h>
  #include <fcntl.h>
  static int csprng(void *buf, size_t len) {
    ssize_t n = getrandom(buf, len, 0);
    if (n == (ssize_t)len) return 1;
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return 0;
    size_t got = 0;
    while (got < len) {
      ssize_t r = read(fd, (unsigned char*)buf + got, len - got);
      if (r <= 0) { close(fd); return 0; }
      got += (size_t)r;
    }
    close(fd);
    return 1;
  }
#else
  #include <unistd.h>
  #include <fcntl.h>
  static int csprng(void *buf, size_t len) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return 0;
    size_t got = 0;
    while (got < len) {
      ssize_t r = read(fd, (unsigned char*)buf + got, len - got);
      if (r <= 0) { close(fd); return 0; }
      got += (size_t)r;
    }
    close(fd);
    return 1;
  }
#endif

/* Bitcoin Base58 alphabet (no 0,O,I,l) */
static const char B58[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

/* Encode big-endian bytes to Base58 (no checksum). out_cap must be large enough.
   Returns length (excluding NUL) or 0 on failure. */
static size_t b58_encode(const uint8_t *bytes, size_t len, char *out, size_t out_cap) {
    if (!bytes || !out || out_cap == 0) return 0;

    /* Count leading zeros for later */
    size_t zeros = 0;
    while (zeros < len && bytes[zeros] == 0) zeros++;

    /* Allocate a temp base58 buffer (max growth ~ log256/log58 ≈ 1.365) */
    size_t size = (size_t)((len - zeros) * 138 / 100) + 2;
    if (size + zeros + 1 > out_cap) return 0; /* not enough space */
    unsigned char tmp[512];
    if (size > sizeof(tmp)) return 0;

    size_t out_len = 0;
    memset(tmp, 0, size);

    /* Convert big-endian byte array to Base58 digits (in tmp) */
    for (size_t i = zeros; i < len; ++i) {
        unsigned int carry = bytes[i];
        size_t j = 0;
        for (ssize_t k = (ssize_t)size - 1; k >= 0; --k) {
            carry += 256U * tmp[k];
            tmp[k] = (unsigned char)(carry % 58U);
            carry /= 58U;
            j++;
        }
        (void)j;
    }

    /* Skip leading zeros in base58 result */
    size_t pos = 0;
    while (pos < size && tmp[pos] == 0) pos++;

    /* Leading zero bytes become '1's */
    size_t idx = 0;
    for (size_t i = 0; i < zeros; ++i) out[idx++] = '1';

    /* Translate digits */
    while (pos < size) out[idx++] = B58[tmp[pos++]];

    out[idx] = '\0';
    return idx;
}

int base58id_generate_n(char *out, size_t out_cap, size_t nbytes) {
    if (!out || nbytes == 0 || nbytes > 64) return 0;
    uint8_t buf[64];
    if (!csprng(buf, nbytes)) return 0;
    return b58_encode(buf, nbytes, out, out_cap) > 0;
}

int base58id_generate(char *out, size_t out_cap) {
    return base58id_generate_n(out, out_cap, BASE58ID_DEFAULT_BYTES);
}
