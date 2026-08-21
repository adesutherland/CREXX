/* cREXX optional Level-G host-information and timing provider. */

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <sys/sysctl.h>
#include <sys/time.h>
#endif

#ifdef __linux__
#include <sys/sysinfo.h>
#endif

#include "crexxpa.h"

RXPA_PLUGIN_PROCESS_REENTRANT

PROCEDURE(uptime)
{
    if (NUM_ARGS != 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXPLATFORM.UPTIME expects no arguments")
#ifdef _WIN32
    RETURNINT((rxinteger)(GetTickCount64() / 1000u));
#elif defined(__linux__)
    {
        struct sysinfo info;
        if (sysinfo(&info) != 0)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.UPTIME is unavailable")
        RETURNINT((rxinteger)info.uptime);
    }
#elif defined(__APPLE__)
    {
        struct timeval boot;
        struct timeval now;
        size_t length = sizeof(boot);
        if (sysctlbyname("kern.boottime", &boot, &length, NULL, 0) != 0 ||
            gettimeofday(&now, NULL) != 0)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.UPTIME is unavailable")
        RETURNINT((rxinteger)(now.tv_sec - boot.tv_sec));
    }
#else
    RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.UPTIME is unavailable")
#endif
    RESETSIGNAL
}

PROCEDURE(user)
{
    if (NUM_ARGS != 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXPLATFORM.USER expects no arguments")
#ifdef _WIN32
    {
        char name[256];
        DWORD length = (DWORD)sizeof(name);
        if (!GetUserNameA(name, &length))
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.USER is unavailable")
        RETURNSTR(name);
    }
#else
    {
        struct passwd pwd;
        struct passwd *result = NULL;
        char buffer[16384];
        const char *name = NULL;
        if (getpwuid_r(getuid(), &pwd, buffer, sizeof(buffer), &result) == 0 &&
            result && result->pw_name && result->pw_name[0]) name = result->pw_name;
        if (!name || !name[0]) name = getenv("USER");
        if (!name || !name[0]) name = getenv("LOGNAME");
        if (!name || !name[0])
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.USER is unavailable")
        RETURNSTR(name);
    }
#endif
    RESETSIGNAL
}

PROCEDURE(host)
{
    char name[256];
    if (NUM_ARGS != 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXPLATFORM.HOST expects no arguments")
#ifdef _WIN32
    {
        DWORD length = (DWORD)sizeof(name);
        if (!GetComputerNameA(name, &length))
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.HOST is unavailable")
    }
#else
    if (gethostname(name, sizeof(name)) != 0)
        RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.HOST is unavailable")
    name[sizeof(name) - 1u] = '\0';
#endif
    RETURNSTR(name);
    RESETSIGNAL
}

PROCEDURE(osname)
{
    if (NUM_ARGS != 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXPLATFORM.OSNAME expects no arguments")
#ifdef _WIN32
    RETURNSTR("Windows");
#elif defined(__linux__)
    RETURNSTR("Linux");
#elif defined(__APPLE__)
    RETURNSTR("macOS");
#elif defined(__FreeBSD__)
    RETURNSTR("FreeBSD");
#elif defined(__unix__)
    RETURNSTR("Unix");
#else
    RETURNSTR("Unknown");
#endif
    RESETSIGNAL
}

PROCEDURE(sleep_ms)
{
    rxinteger milliseconds;
    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXPLATFORM.SLEEP expects one millisecond value")
    milliseconds = GETINT(ARG0);
    if (milliseconds < 0 || (uint64_t)milliseconds > UINT32_MAX)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXPLATFORM.SLEEP requires 0..4294967295 milliseconds")
#ifdef _WIN32
    Sleep((DWORD)milliseconds);
#else
    {
        struct timespec requested;
        struct timespec remaining;
        requested.tv_sec = (time_t)(milliseconds / 1000);
        requested.tv_nsec = (long)((milliseconds % 1000) * 1000000);
        while (nanosleep(&requested, &remaining) != 0) requested = remaining;
    }
#endif
    RETURNINT(0);
    RESETSIGNAL
}

LOADFUNCS
    ADDPROC(uptime, "rxplatform.uptime", "b", ".int", "");
    ADDPROC(user, "rxplatform.user", "b", ".string", "");
    ADDPROC(host, "rxplatform.host", "b", ".string", "");
    ADDPROC(osname, "rxplatform.osname", "b", ".string", "");
    ADDPROC(sleep_ms, "rxplatform.sleep", "b", ".int", "milliseconds = .int");
ENDLOADFUNCS
