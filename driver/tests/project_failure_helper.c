/* Deterministic fake compiler/assembler: return success, then arrange one
 * specific worker publication failure. No retry or filesystem timing race. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#define make_dir(p) _mkdir(p)
#else
#include <sys/stat.h>
#define make_dir(p) mkdir(p, 0700)
#endif
int main(int argc, char **argv) {
    const char *output = NULL;
    char directory[4096], path[sizeof(directory) + sizeof("/action.sha256.tmp")], *slash;
    FILE *fp;
    int i;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--project-dependencies")) return 0;
        if (!strcmp(argv[i], "-o") && i + 1 < argc) output = argv[i + 1];
    }
    if (!output) return 2;
    if (strstr(output, "missing_output")) return 0;
    i = snprintf(path, sizeof(path), "%s.rxbin", output);
    if (i < 0 || (size_t)i >= sizeof(path)) return 2;
    fp = fopen(path, "wb");
    if (!fp) return 3;
    fputs("fixture", fp); fclose(fp);
    i = snprintf(directory, sizeof(directory), "%s", output);
    if (i < 0 || (size_t)i >= sizeof(directory)) return 2;
    slash = strrchr(directory, '/');
    if (!slash) return 4;
    *slash = '\0';
    i = snprintf(path, sizeof(path), "%s/action.sha256%s", directory,
                 strstr(output, "stamp_write") ? ".tmp" : "");
    if (i < 0 || (size_t)i >= sizeof(path)) return 2;
    return make_dir(path) == 0 ? 0 : 5;
}
