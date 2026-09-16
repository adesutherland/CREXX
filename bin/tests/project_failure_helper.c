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
    char path[4096], directory[4096], *slash;
    FILE *fp;
    int i;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--project-dependencies")) return 0;
        if (!strcmp(argv[i], "-o") && i + 1 < argc) output = argv[i + 1];
    }
    if (!output) return 2;
    if (strstr(output, "missing_output")) return 0;
    snprintf(path, sizeof(path), "%s.rxbin", output);
    fp = fopen(path, "wb");
    if (!fp) return 3;
    fputs("fixture", fp); fclose(fp);
    snprintf(directory, sizeof(directory), "%s", output);
    slash = strrchr(directory, '/');
    if (!slash) return 4;
    *slash = '\0';
    snprintf(path, sizeof(path), "%s/action.sha256%s", directory,
             strstr(output, "stamp_write") ? ".tmp" : "");
    return make_dir(path) == 0 ? 0 : 5;
}
