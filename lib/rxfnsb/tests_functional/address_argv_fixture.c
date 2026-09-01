/* Harmless direct-process fixture for the Level B ADDRESS argv contract. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>

static int emit_argument(int index, const wchar_t *argument) {
    int byte_count;
    char *utf8;

    byte_count = WideCharToMultiByte(CP_UTF8, 0, argument, -1, NULL, 0, NULL, NULL);
    if (byte_count <= 0) return 1;
    utf8 = (char *)malloc((size_t)byte_count);
    if (!utf8) return 1;
    if (WideCharToMultiByte(CP_UTF8, 0, argument, -1, utf8, byte_count, NULL, NULL) <= 0) {
        free(utf8);
        return 1;
    }
    printf("ARG%d=", index);
    if (byte_count > 1) fwrite(utf8, 1, (size_t)byte_count - 1, stdout);
    putchar('\n');
    free(utf8);
    return 0;
}

int wmain(int argc, wchar_t **argv) {
    int i;
    for (i = 1; i < argc; i++) {
        if (emit_argument(i, argv[i]) != 0) return 2;
    }
    return 0;
}
#else
int main(int argc, char **argv) {
    int i;
    for (i = 1; i < argc; i++) {
        printf("ARG%d=", i);
        fwrite(argv[i], 1, strlen(argv[i]), stdout);
        putchar('\n');
    }
    return 0;
}
#endif
