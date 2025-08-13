#include "snowflake.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

/* ---------------- time in ms ---------------- */
#if defined(_WIN32)
#include <windows.h>

static uint64_t now_ms_snow(void) {
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
}
#else
#include <sys/time.h>
  static uint64_t now_ms_snow(void) {
    struct timeval tv; gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)(tv.tv_usec / 1000ULL);
  }
#endif

/* ---------------- tiny mutex ---------------- */
#if defined(_WIN32)
#include <windows.h>
static CRITICAL_SECTION g_lock;
static int g_lock_inited = 0;
static void lock_init(void){ if(!g_lock_inited){ InitializeCriticalSection(&g_lock); g_lock_inited=1; } }
static void lock_enter(void){ if(!g_lock_inited) lock_init(); EnterCriticalSection(&g_lock); }
static void lock_leave(void){ LeaveCriticalSection(&g_lock); }
#else
#include <pthread.h>
  static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
  static void lock_enter(void){ pthread_mutex_lock(&g_lock); }
  static void lock_leave(void){ pthread_mutex_unlock(&g_lock); }
#endif

/* ---------------- state ---------------- */
// static uint64_t g_last_ms = 0;   // defined somewhere else
static uint16_t g_node_id = 0xFFFF; /* means "unset / auto" */
static uint16_t g_seq     = 0;

/* quick djb2 hash */
static uint32_t djb2_hash(const unsigned char *s, size_t n) {
    uint32_t h = 5381U;
    for (size_t i = 0; i < n; ++i) h = ((h << 5) + h) ^ s[i];
    return h;
}

#if defined(_WIN32)
#include <windows.h>
static uint32_t pid_get(void) { return (uint32_t)GetCurrentProcessId(); }
static int hostname_get(char *buf, size_t sz) {
    DWORD n = (DWORD)sz;
    return GetComputerNameA(buf, &n) ? 1 : 0;
}
#else
#include <unistd.h>
  #include <limits.h>
  static uint32_t pid_get(void) { return (uint32_t)getpid(); }
  static int hostname_get(char *buf, size_t sz) {
    return gethostname(buf, sz) == 0 ? 1 : 0;
  }
#endif

/* derive node id from hostname + PID if not set explicitly.
   Also allow environment override: IDLIB_SNOWFLAKE_NODE (0..MAX_NODE) */
static uint16_t node_ensure(void) {
    if (g_node_id != 0xFFFF) return g_node_id;

    /* env override */
    const char *env = getenv("IDLIB_SNOWFLAKE_NODE");
    if (env && *env) {
        long v = strtol(env, NULL, 10);
        if (v >= 0 && v <= (long)SNOWFLAKE_MAX_NODE) {
            g_node_id = (uint16_t)v;
            return g_node_id;
        }
    }

    char host[256] = {0};
    hostname_get(host, sizeof host);
    uint32_t pid = pid_get();

    uint32_t h = djb2_hash((const unsigned char*)host, strlen(host));
    h ^= djb2_hash((const unsigned char*)&pid, sizeof(pid));

    g_node_id = (uint16_t)(h & SNOWFLAKE_MAX_NODE);
    return g_node_id;
}

int snowflake_set_node(uint16_t node_id) {
    if (node_id > SNOWFLAKE_MAX_NODE) return 0;
    g_node_id = node_id;
    return 1;
}

/* u64 -> decimal string */
static void u64_to_dec(uint64_t v, char out[21]) {
    char buf[21]; int i = 20; buf[i] = '\0';
    if (v == 0) { out[0]='0'; out[1]='\0'; return; }
    while (v && i) { buf[--i] = (char)('0' + (v % 10)); v /= 10; }
    memcpy(out, buf + i, 21 - i);
}

/* Assemble: [41 bits ms since epoch][10 bits node][12 bits seq] */
int snowflake_next_u64(uint64_t *out) {
    if (!out) return 0;

    const uint64_t node_shift = SNOWFLAKE_SEQ_BITS;
    const uint64_t time_shift = SNOWFLAKE_NODE_BITS + SNOWFLAKE_SEQ_BITS;

    lock_enter();

    uint64_t ms = now_ms_snow();
    uint64_t rel = (ms >= SNOWFLAKE_EPOCH_MS) ? (ms - SNOWFLAKE_EPOCH_MS) : 0;

    /* If clock goes backward, pin to last_ms (monotonic) */
    if (ms < g_last_ms) {
        rel = (g_last_ms >= SNOWFLAKE_EPOCH_MS) ? (g_last_ms - SNOWFLAKE_EPOCH_MS) : 0;
        ms = g_last_ms;
    }

    if (ms == g_last_ms) {
        g_seq = (uint16_t)((g_seq + 1) & SNOWFLAKE_MAX_SEQ);
        if (g_seq == 0) {
            /* sequence overflow within same ms -> wait next ms */
            do { ms = now_ms_snow(); } while (ms <= g_last_ms);
            rel = ms - SNOWFLAKE_EPOCH_MS;
        }
    } else {
        g_seq = 0;
    }
    g_last_ms = ms;

    uint64_t id = (rel << time_shift) | ((uint64_t)node_ensure() << node_shift) | (uint64_t)g_seq;

    lock_leave();

    *out = id;
    return 1;
}

int snowflake_next_str(char out[21]) {
    uint64_t v;
    if (!snowflake_next_u64(&v)) return 0;
    u64_to_dec(v, out);
    return 1;
}
