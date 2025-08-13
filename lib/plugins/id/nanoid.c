#include "nanoid.h"
#include <stdint.h>
#include <string.h>

/* --------- csprng_nano (Windows / macOS-BSD / Linux / generic) --------- */
#if defined(_WIN32)
#include <windows.h>
#ifndef RtlGenRandom
extern BOOLEAN NTAPI SystemFunction036(PVOID RandomBuffer, ULONG RandomBufferLength);
#define RtlGenRandom SystemFunction036
#endif
static int csprng_nano(void *buf, size_t len) { return RtlGenRandom(buf, (ULONG)len) ? 1 : 0; }   // defined somewhere else

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <stdlib.h> /* arc4random_buf */
  static int csprng_nano(void *buf, size_t len) { arc4random_buf(buf, len); return 1; }

#elif defined(__linux__)
  #include <sys/random.h>
  #include <unistd.h>
  #include <fcntl.h>
  static int csprng_nano(void *buf, size_t len) {
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
  static int csprng_nano(void *buf, size_t len) {
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

/* URL-safe alphabet (per NanoID): 64 chars */
static const char ALPHABET_URLSAFE[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
static const size_t ALPHABET_URLSAFE_LEN = 64;

/* Fill out[0..len-1] with chars from alphabet using unbiased mask method */
static int fill_with_alphabet(char *out, size_t len, const char *alphabet, size_t alphabet_len) {
    if (!out || !alphabet || alphabet_len < 2 || alphabet_len > 255 || len == 0) return 0;

    /* Find mask = 2^k - 1 >= alphabet_len-1 (smallest mask covering the set) */
    uint32_t mask = 1;
    while (mask < (alphabet_len - 1)) mask = (mask << 1) | 1;  /* 1,3,7,15,31,63,127,255 */

    size_t i = 0;
    /* Batch-sized random buffer to reduce syscalls */
    unsigned char rnd[64];

    while (i < len) {
        /* pull fresh randomness */
        if (!csprng_nano(rnd, sizeof rnd)) return 0;

        for (size_t r = 0; r < sizeof rnd && i < len; ++r) {
            uint32_t idx = rnd[r] & mask;
            if (idx < alphabet_len) {
                out[i++] = alphabet[idx];
            }
            /* else: reject and continue */
        }
    }

    out[len] = '\0';
    return 1;
}

int nanoid_generate(char out[NANOID_DEFAULT_SIZE + 1]) {
    return fill_with_alphabet(out, NANOID_DEFAULT_SIZE, ALPHABET_URLSAFE, ALPHABET_URLSAFE_LEN);
}

int nanoid_generate_custom(char *out, size_t len, const char *alphabet, size_t alphabet_len) {
    return fill_with_alphabet(out, len, alphabet, alphabet_len);
}
