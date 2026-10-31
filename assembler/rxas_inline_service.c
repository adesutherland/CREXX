/* cREXX License (MIT). Invoke the one B/L inline service before RXAS parsing. */
#include "rxas_inline_service.h"
#include "platform.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <direct.h>
#include "../compiler/rxcp_windows_argv.h"
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

static char *rxas_inline_join(const char *directory, const char *name) {
    size_t a = strlen(directory), b = strlen(name);
    char *path;
    if (a > (size_t)-1 - b - 2u) return NULL;
    path = (char *)malloc(a + b + 2u);
    if (path) snprintf(path, a + b + 2u, "%s/%s", directory, name);
    return path;
}

#ifdef _WIN32
static char *rxas_inline_private_dir(void) {
    wchar_t base[MAX_PATH], name[MAX_PATH];
    char *utf8;
    DWORD size = GetTempPathW(MAX_PATH, base);
    int length;
    if (!size || size >= MAX_PATH ||
        !GetTempFileNameW(base, L"rxi", 0, name) ||
        !DeleteFileW(name) || !CreateDirectoryW(name, NULL)) return NULL;
    length = WideCharToMultiByte(CP_UTF8, 0, name, -1, NULL, 0, NULL, NULL);
    utf8 = length ? (char *)malloc((size_t)length) : NULL;
    if (utf8) WideCharToMultiByte(CP_UTF8, 0, name, -1,
                                 utf8, length, NULL, NULL);
    else RemoveDirectoryW(name);
    return utf8;
}

static wchar_t *rxas_inline_wide(const char *text) {
    int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                     text, -1, NULL, 0);
    wchar_t *wide = length ? (wchar_t *)calloc((size_t)length,
                                              sizeof(wchar_t)) : NULL;
    if (wide && !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                    text, -1, wide, length)) {
        free(wide);
        wide = NULL;
    }
    return wide;
}

static int rxas_inline_execute(const char *vm, char *const *argv) {
    size_t count = 0u, i;
    int result = -1;
    wchar_t **arguments;
    wchar_t *program = rxas_inline_wide(vm);
    while (argv[count]) ++count;
    arguments = (wchar_t **)calloc(count + 1u, sizeof(*arguments));
    if (arguments && program) {
        for (i = 0u; i < count; ++i) {
            wchar_t *wide = rxas_inline_wide(argv[i]);
            if (!wide) break;
            arguments[i] = rxcp_windows_quote_argument(wide);
            free(wide);
            if (!arguments[i]) break;
        }
        if (i == count)
            result = (int)_wspawnv(_P_WAIT, program,
                                   (const wchar_t *const *)arguments);
    }
    if (arguments) {
        for (i = 0u; i < count; ++i) free(arguments[i]);
        free(arguments);
    }
    free(program);
    return result;
}
#else
static char *rxas_inline_private_dir(void) {
    const char *base = getenv("TMPDIR");
    size_t length;
    char *name;
    if (!base || !*base) base = "/tmp";
    length = strlen(base);
    if (length > (size_t)-1 - 24u) return NULL;
    name = (char *)malloc(length + 24u);
    if (!name) return NULL;
    snprintf(name, length + 24u, "%s/rxas-inline-XXXXXX", base);
    if (mkdtemp(name)) return name;
    free(name);
    return NULL;
}

static int rxas_inline_execute(const char *vm, char *const *argv) {
    pid_t child = fork();
    int status;
    if (child < 0) return -1;
    if (child == 0) { execv(vm, argv); _exit(127); }
    while (waitpid(child, &status, 0) < 0) {
        if (errno != EINTR) return -1;
    }
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}
#endif

void rxas_inline_service_cleanup(char *private_dir, char *read_path) {
    char *source;
    if (read_path) remove(read_path);
    if (!private_dir) {
        free(read_path);
        return;
    }
    source = rxas_inline_join(private_dir, "source.rxas");
    if (source) { remove(source); free(source); }
#ifdef _WIN32
    _rmdir(private_dir);
#else
    rmdir(private_dir);
#endif
    free(read_path);
    free(private_dir);
}

int rxas_inline_service_run(const Assembler_Context *scanner,
        const char *service, const char *vm, const char *logical,
        const char *policy, char *const *runtime, size_t runtime_count,
        char **private_dir, char **read_path) {
    char *directory = NULL, *source = NULL, *expanded = NULL;
    char **argv = NULL;
    FILE *input = NULL, *output = NULL, *check = NULL;
    unsigned char buffer[8192];
    size_t count = 0u, used, i;
    int status = -1;
    if (!scanner || !service || !vm || !logical || !policy ||
        !private_dir || !read_path || runtime_count > (size_t)-1 - 16u)
        return -1;
    *private_dir = NULL;
    *read_path = NULL;
    directory = rxas_inline_private_dir();
    if (!directory) goto failed;
    source = rxas_inline_join(directory, "source.rxas");
    expanded = rxas_inline_join(directory, "expanded.rxas");
    if (!source || !expanded) goto failed;
    input = openfile(scanner->file_name,
                     has_any_extension(scanner->file_name) ? "" : "rxas",
                     scanner->location, "rb");
    if (!input) {
        fprintf(stderr, "RXAS_INLINE_SERVICE: cannot open source %s\n",
                scanner->file_name);
        goto failed;
    }
    output = fopen(source, "wb");
    if (!output) goto failed;
    while ((used = fread(buffer, 1u, sizeof(buffer), input)) != 0u) {
        if (fwrite(buffer, 1u, used, output) != used) goto failed;
    }
    if (ferror(input)) goto failed;
    if (fclose(output) != 0) {
        output = NULL;
        goto failed;
    }
    output = NULL;
    fclose(input);
    input = NULL;
    argv = (char **)calloc(runtime_count + 16u, sizeof(*argv));
    if (!argv) goto failed;
    argv[count++] = (char *)vm;
    argv[count++] = (char *)service;
    for (i = 0u; i < runtime_count; ++i)
        argv[count++] = runtime[i];
    argv[count++] = "-a";
    argv[count++] = "--module";
    argv[count++] = (char *)logical;
    argv[count++] = "--artifact";
    argv[count++] = (char *)logical;
    argv[count++] = "--policy";
    argv[count++] = (char *)policy;
    argv[count++] = "--no-cleanup";
    argv[count++] = "-o";
    argv[count++] = expanded;
    argv[count++] = source;
    argv[count] = NULL;
    status = rxas_inline_execute(vm, argv);
    if (status != 0) {
        fprintf(stderr, "RXAS_INLINE_SERVICE: service failed (%d)\n", status);
        goto failed;
    }
    check = fopen(expanded, "rb");
    if (!check) {
        fprintf(stderr, "RXAS_INLINE_SERVICE: service produced no RXAS\n");
        goto failed;
    }
    fclose(check);
    free(argv);
    free(source);
    *private_dir = directory;
    *read_path = expanded;
    return 0;
failed:
    if (check) fclose(check);
    if (input) fclose(input);
    if (output) fclose(output);
    free(argv);
    free(source);
    rxas_inline_service_cleanup(directory, expanded);
    if (status == -1)
        fprintf(stderr, "RXAS_INLINE_SERVICE: preparation or launch failed\n");
    return -1;
}
