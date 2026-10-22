/* Deterministic child for Group 2's shared ADDRESS/executable CALL boundary. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#else
#include <signal.h>
#include <unistd.h>
#endif

int main(int argc, char **argv) {
#ifdef _WIN32
    (void)_setmode(_fileno(stdin), _O_BINARY);
    (void)_setmode(_fileno(stdout), _O_BINARY);
    (void)_setmode(_fileno(stderr), _O_BINARY);
#endif
    if (argc == 1) {
        puts("EMPTY-ARGV");
        return 0;
    }
    if (argc == 3 && strcmp(argv[1], "--status") == 0)
        return atoi(argv[2]);
    if (argc == 2 && strcmp(argv[1], "--wait") == 0) {
#ifdef _WIN32
        printf("%lu\n", (unsigned long)GetCurrentProcessId());
#else
        printf("%ld\n", (long)getpid());
#endif
        fflush(stdout);
        /* A broken test cannot leave this fixture running indefinitely. */
        for (int attempt = 0; attempt < 300; ++attempt) {
#ifdef _WIN32
            Sleep(100);
#else
            usleep(100000);
#endif
        }
        return 42;
    }
    if (argc == 3 && strcmp(argv[1], "--expect-dead") == 0) {
        char *end = NULL;
        long pid = strtol(argv[2], &end, 10);
        if (!end || *end || pid <= 0 || pid > INT_MAX) return 41;
#ifdef _WIN32
        HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE,
                                     (DWORD)pid);
        DWORD status;
        if (!process) return GetLastError() == ERROR_INVALID_PARAMETER ? 0 : 41;
        if (!GetExitCodeProcess(process, &status)) { CloseHandle(process); return 41; }
        CloseHandle(process);
        return status == STILL_ACTIVE ? 40 : 0;
#else
        if (kill((pid_t)pid, 0) == 0) return 40;
        return errno == ESRCH ? 0 : 41;
#endif
    }
    if (argc == 2 && strcmp(argv[1], "--streams") == 0) {
        puts("CHILD-STDOUT");
        fputs("CHILD-STDERR\n", stderr);
        return 7;
    }
    if (argc == 2 && strcmp(argv[1], "--echo") == 0) {
        unsigned char bytes[4096];
        static const unsigned char error[] = {'E', 0, 255, '\n'};
        size_t count;
        while ((count = fread(bytes, 1, sizeof bytes, stdin)) != 0)
            if (fwrite(bytes, 1, count, stdout) != count) return 32;
        if (ferror(stdin) || fwrite(error, 1, sizeof error, stderr) != sizeof error)
            return 33;
        return 7;
    }
    return 31;
}
