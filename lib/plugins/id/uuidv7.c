#include "uuidv7.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>

#if defined(_WIN32)
#include <windows.h>
#ifndef RtlGenRandom
extern BOOLEAN NTAPI SystemFunction036(PVOID, ULONG);
#define RtlGenRandom SystemFunction036
#endif
static int uuidv7_csprng(void *buf, size_t len) {
    return RtlGenRandom(buf, (ULONG)len) ? 1 : 0;
}
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <stdlib.h>
static int uuidv7_csprng(void *buf, size_t len) {
    arc4random_buf(buf, len);
    return 1;
}
#elif defined(__linux__)
#include <sys/random.h>
#include <unistd.h>
#include <fcntl.h>
static int uuidv7_csprng(void *buf, size_t len) {
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
#include <fcntl.h>
#include <unistd.h>
static int uuidv7_csprng(void *buf, size_t len) {
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

static uint64_t uuidv7_now_ms(void) {
#if defined(_WIN32)
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    uint64_t t100 = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    const uint64_t EPOCH_DIFF_100NS = 116444736000000000ULL;
    uint64_t ns = (t100 - EPOCH_DIFF_100NS) * 100ULL;
    return ns / 1000000ULL;
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000ULL);
#endif
}

static uint64_t g_last_ms = 0;
static uint16_t g_last_ra = 0;
static uint64_t g_last_rb = 0;

static void random_12bits(uint16_t *out) {
    uint16_t x = 0; uuidv7_csprng(&x, sizeof x);
    *out = (uint16_t)(x & 0x0FFFu);
}
static void random_62bits(uint64_t *out) {
    uint64_t x = 0; uuidv7_csprng(&x, sizeof x);
    *out = x & ((1ULL << 62) - 1ULL);
}

int uuidv7_generate(uint8_t uuid[16]) {
    if (!uuid) return 0;
    uint64_t ms = uuidv7_now_ms();
    uint16_t ra; uint64_t rb;

    if (ms != g_last_ms) {
        random_12bits(&ra);
        random_62bits(&rb);
    } else {
        ra = g_last_ra;
        rb = g_last_rb + 1ULL;
        if (rb >= (1ULL << 62)) { rb = 0; ra = (uint16_t)((ra + 1) & 0x0FFFu); }
        if (rb == 0 && ra == 0) {
            do { ms = uuidv7_now_ms(); } while (ms == g_last_ms);
            random_12bits(&ra);
            random_62bits(&rb);
        }
    }
    g_last_ms = ms; g_last_ra = ra; g_last_rb = rb;

    uuid[0] = (uint8_t)((ms >> 40) & 0xFF);
    uuid[1] = (uint8_t)((ms >> 32) & 0xFF);
    uuid[2] = (uint8_t)((ms >> 24) & 0xFF);
    uuid[3] = (uint8_t)((ms >> 16) & 0xFF);
    uuid[4] = (uint8_t)((ms >>  8) & 0xFF);
    uuid[5] = (uint8_t)( ms        & 0xFF);

    uuid[6] = (uint8_t)(0x70 | ((ra >> 8) & 0x0F));
    uuid[7] = (uint8_t)(ra & 0xFF);

    uint8_t rbb[8];
    for (int i = 0; i < 8; ++i) rbb[7 - i] = (uint8_t)(rb >> (i * 8));

    uuid[8]  = (uint8_t)((rbb[0] & 0x3F) | 0x80);
    uuid[9]  = rbb[1];
    uuid[10] = rbb[2];
    uuid[11] = rbb[3];
    uuid[12] = rbb[4];
    uuid[13] = rbb[5];
    uuid[14] = rbb[6];
    uuid[15] = rbb[7];

    return 1;
}

void uuidv_to_string(const uint8_t u[16], char out[37]) {
    static const char *hex = "0123456789abcdef";
    int p = 0;
    for (int i = 0; i < 16; ++i) {
        out[p++] = hex[u[i] >> 4];
        out[p++] = hex[u[i] & 0x0F];
        if (i==3 || i==5 || i==7 || i==9) out[p++] = '-';
    }
    out[p] = '\0';
}
