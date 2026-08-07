/* PERF3-13 EF-0 deterministic child-process I/O fixture. */
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#define CAPTURE_LINE_COUNT 80
#define CAPTURE_BODY_LENGTH 300

static int emit_line(FILE *stream, char prefix, int index, char fill) {
    int i;

    if (fprintf(stream, "%c%03d:", prefix, index) < 0) return -1;
    for (i = 0; i < CAPTURE_BODY_LENGTH; i++) {
        if (fputc(fill, stream) == EOF) return -1;
    }
    if (fputc('\n', stream) == EOF) return -1;
    return fflush(stream) == 0 ? 0 : -1;
}

static int copy_input(FILE *output) {
    char buffer[1024];
    size_t length;

    while ((length = fread(buffer, 1, sizeof(buffer), stdin)) != 0) {
        if (fwrite(buffer, 1, length, output) != length) return -1;
        if (fflush(output) != 0) return -1;
    }
    return ferror(stdin) ? -1 : 0;
}

static int duplex_input(void) {
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), stdin)) {
        if (fputs("OUT:", stdout) == EOF || fputs(buffer, stdout) == EOF ||
            fflush(stdout) != 0) return -1;
        if (fputs("ERR:", stderr) == EOF || fputs(buffer, stderr) == EOF ||
            fflush(stderr) != 0) return -1;
    }
    return ferror(stdin) ? -1 : 0;
}

int main(int argc, char **argv) {
    int i;

#ifdef _WIN32
    (void)_setmode(_fileno(stdin), _O_BINARY);
    (void)_setmode(_fileno(stdout), _O_BINARY);
    (void)_setmode(_fileno(stderr), _O_BINARY);
#endif

    if (argc < 2 || strcmp(argv[1], "empty") == 0) return 0;
    if (strcmp(argv[1], "capture") == 0) {
        for (i = 0; i < CAPTURE_LINE_COUNT; i++) {
            if (emit_line(stdout, 'O', i, 'o') != 0 ||
                emit_line(stderr, 'E', i, 'e') != 0) return 2;
        }
        return 0;
    }
    if (strcmp(argv[1], "echo") == 0) return copy_input(stdout) == 0 ? 0 : 3;
    if (strcmp(argv[1], "duplex") == 0) return duplex_input() == 0 ? 0 : 4;
    if (strcmp(argv[1], "fail") == 0) {
        fputs("partial-out\n", stdout);
        fputs("partial-error\n", stderr);
        fflush(stdout);
        fflush(stderr);
        return 7;
    }
    if (strcmp(argv[1], "early") == 0) return 0;
    return 64;
}
