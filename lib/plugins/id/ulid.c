#include "ulid.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>

/* ---------- RNG: Windows / macOS-BSD / Linux ---------- */
#if defined(_WIN32)
#include <windows.h>
#ifndef RtlGenRandom
extern BOOLEAN NTAPI SystemFunction036(PVOID RandomBuffer, ULONG RandomBufferLength);
#define RtlGenRandom SystemFunction036
#endif
static int  csprng_ulid(void *buf, size_t len) { return RtlGenRandom(buf, (ULONG)len) ? 1 : 0; }
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <stdlib.h> /* arc4random_buf */
  static int  csprng_ulid(void *buf, size_t len) { arc4random_buf(buf, len); return 1; }
#elif defined(__linux__)
  #include <sys/random.h>
  #include <unistd.h>
  #include <fcntl.h>
  static int  csprng_ulid(void *buf, size_t len) {
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
  static int  csprng_ulid(void *buf, size_t len) {
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

/* ---------- time (ms since Unix epoch) ---------- */
static uint64_t now_ms_ulid(void) {
#if defined(_WIN32)
    FILETIME ft;
    static void (WINAPI *GetPrecise)(LPFILETIME) = NULL;
    static int resolved = 0;
    if (!resolved) {
        HMODULE h = GetModuleHandleA("kernel32.dll");
        if (h) GetPrecise = (void (WINAPI*)(LPFILETIME))GetProcAddress(h, "GetSystemTimePreciseAsFileTime");
        resolved = 1;
    }
    if (GetPrecise) GetPrecise(&ft);
    else GetSystemTimeAsFileTime(&ft);
    uint64_t t100 = ((uint64_t)ft.dwHighDateTime << 32) | (uint64_t)ft.dwLowDateTime; /* 100ns since 1601 */
    const uint64_t EPOCH_DIFF_100NS = 116444736000000000ULL;
    uint64_t ns = (t100 - EPOCH_DIFF_100NS) * 100ULL;
    return ns / 1000000ULL;
#else
    struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000ULL);
#endif
}

/* ---------- ULID state for monotonic behavior ---------- */
static uint64_t last_ms = 0;
static uint8_t  last_rand[10] = {0};  /* 80-bit randomness, big-endian */

/* Increment the 80-bit big-endian random field by 1. Return 0 if it overflowed. */
static int inc_80_be(uint8_t r[10]) {
    for (int i = 9; i >= 0; --i) {
        if (++r[i] != 0) return 1;
    }
    return 0; /* overflow (all became zero) */
}

/* out[16] = 48-bit ms timestamp (big-endian) + 80-bit random (big-endian) */
int ulid_generate(uint8_t out[16]) {
    if (!out) return 0;

    uint64_t ms = now_ms_ulid();

    if (ms != last_ms) {
        if (! csprng_ulid(last_rand, sizeof last_rand)) return 0;
    } else {
        /* same millisecond: monotonic increment of random field; if overflow, wait next ms and reseed */
        if (!inc_80_be(last_rand)) {
            do { ms = now_ms_ulid(); } while (ms == last_ms);
            if (! csprng_ulid(last_rand, sizeof last_rand)) return 0;
        }
    }
    last_ms = ms;

    /* write timestamp (48-bit, big-endian) */
    out[0] = (uint8_t)((ms >> 40) & 0xFF);
    out[1] = (uint8_t)((ms >> 32) & 0xFF);
    out[2] = (uint8_t)((ms >> 24) & 0xFF);
    out[3] = (uint8_t)((ms >> 16) & 0xFF);
    out[4] = (uint8_t)((ms >>  8) & 0xFF);
    out[5] = (uint8_t)( ms        & 0xFF);

    /* write randomness (80-bit, big-endian) */
    memcpy(out + 6, last_rand, 10);
    return 1;
}

/* Crockford Base32 (no I, L, O, U) */
static const char CROCK32[32] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";

/* Read 5 bits (big-endian) from 10-byte randomness. bitpos=0 is MSB of byte 0. */
static uint8_t read5(const uint8_t *buf, int bitpos) {
    uint32_t v = 0;
    for (int j = 0; j < 5; ++j) {
        int p = bitpos + j;
        uint8_t byte = buf[p >> 3];
        int bit = 7 - (p & 7);
        v = (v << 1) | ((byte >> bit) & 1);
    }
    return (uint8_t)(v & 31);
}

/* Encode binary ULID to 26-char Crockford Base32 string */
void ulid_to_string(const uint8_t ulid[16], char out[27]) {
    /* time: 48 bits -> 10 chars (LSB groups, but we fill from end) */
    uint64_t ts =
            ((uint64_t)ulid[0] << 40) |
            ((uint64_t)ulid[1] << 32) |
            ((uint64_t)ulid[2] << 24) |
            ((uint64_t)ulid[3] << 16) |
            ((uint64_t)ulid[4] <<  8) |
            ((uint64_t)ulid[5]);

    for (int i = 9; i >= 0; --i) {
        out[i] = CROCK32[ts & 31ULL];
        ts >>= 5;
    }

    /* randomness: 80 bits -> 16 chars, read 5-bit groups big-endian from ulid[6..15] */
    for (int k = 0; k < 16; ++k) {
        out[10 + k] = CROCK32[ read5(ulid + 6, k * 5) ];
    }

    out[26] = '\0';
}
