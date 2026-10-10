/* cREXX License (MIT), Copyright (c) 2026 Adrian Sutherland */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "rxcpmain.h"
#include "rxcp_frontend.h"
#if defined(CREXX_VM_SINGLE_THREADED)
int rxcp_classic_parser_mode(int stdio_mode, int port, const char *file, int debug) {
    (void)stdio_mode; (void)port; (void)file; (void)debug;
    return 2; /* Editor service belongs to the normal desktop compiler. */
}
int rxcp_classic_source_import(Context *c, const char *file, const char *location,
                               RexxLevel default_level) {
    RexxLevel level = UNKNOWN;
    if (rxcp_scan_source_header(location, file, default_level, &level, NULL) == 0 && level == LEVELC)
        return 0;
    mknd_err3(c->ast, "SYNTAX_ERROR_IN_IMPORT_DECL", "name", file,
              "import_file", file, "detail", "typed source compilation needs the host compiler");
    return -1;
}
#else
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

void rxcp_import_rxas_contracts(Context *, const char *, const char *);

#ifdef _WIN32
static wchar_t *wide_path(const char *s) {
    int n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, NULL, 0);
    wchar_t *w = n ? calloc((size_t)n, sizeof(*w)) : NULL;
    if (w && !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, w, n)) {
        free(w); w = NULL;
    }
    return w;
}
#endif

static int run_typed_compiler(const char *tool, char **argv) {
#ifdef _WIN32
    size_t count = 0, i;
    int result = -1;
    wchar_t **args, *program = wide_path(tool);
    while (argv[count]) ++count;
    args = calloc(count + 1, sizeof(*args));
    if (args && program) {
        for (i = 0; i < count; ++i) if (!(args[i] = wide_path(argv[i]))) break;
        if (i == count) result = (int)_wspawnv(_P_WAIT, program, (const wchar_t *const *)args);
    }
    if (args) { for (i = 0; i < count; ++i) free(args[i]); free(args); }
    free(program);
    return result;
#else
    pid_t child = fork();
    int status;
    if (child < 0) return -1;
    if (!child) { execv(tool, argv); _exit(127); }
    while (waitpid(child, &status, 0) < 0) if (errno != EINTR) return -1;
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif
}

static char *private_directory(void) {
#ifdef _WIN32
    wchar_t base[MAX_PATH], name[MAX_PATH];
    char *utf8;
    int length;
    DWORD size = GetTempPathW(MAX_PATH, base);
    if (!size || size >= MAX_PATH || !GetTempFileNameW(base, L"rxc", 0, name)) return NULL;
    if (!DeleteFileW(name) || !CreateDirectoryW(name, NULL)) return NULL;
    length = WideCharToMultiByte(CP_UTF8, 0, name, -1, NULL, 0, NULL, NULL);
    utf8 = length ? malloc((size_t)length) : NULL;
    if (utf8) WideCharToMultiByte(CP_UTF8, 0, name, -1, utf8, length, NULL, NULL);
    else RemoveDirectoryW(name);
    return utf8;
#else
    const char *base = getenv("TMPDIR");
    char *name = mprintf("%s/crexx-c-import-XXXXXX", base && *base ? base : "/tmp");
    if (name && mkdtemp(name)) return name;
    free(name);
    return NULL;
#endif
}

int rxcp_classic_parser_mode(int stdio_mode, int port, const char *file, int debug) {
    char *bin = exepath();
    char *tool;
    char *argv[8];
    char port_text[32], debug_text[32];
    int n = 0, result;
#ifdef _WIN32
    tool = mprintf("%s/rxc.exe", bin);
#else
    tool = mprintf("%s/rxc", bin);
#endif
    argv[n++] = tool;
    argv[n++] = stdio_mode ? "--syntaxhighlight" : "--port";
    if (!stdio_mode) { snprintf(port_text, sizeof(port_text), "%d", port); argv[n++] = port_text; }
    if (debug) { snprintf(debug_text, sizeof(debug_text), "-d%d", debug); argv[n++] = debug_text; }
    if (file) argv[n++] = (char *)file;
    argv[n] = NULL;
    result = run_typed_compiler(tool, argv);
    free(tool); free(bin);
    return result;
}

int rxcp_classic_source_import(Context *c, const char *file, const char *location,
                               RexxLevel default_level) {
    Context *root = c->master_context ? c->master_context : c;
    RexxLevel level = UNKNOWN;
    char *directory, *output, *assembly, *source, *tool, *bin;
    char **argv;
    size_t imports = 0, sources = 0, n = 0, i;
    int status;
    if (root->cli_level_override != UNKNOWN) default_level = root->cli_level_override;
    if (rxcp_scan_source_header(location, file, default_level, &level, NULL) != 0) return 0;
    if (level == LEVELC) return 0;
    directory = private_directory();
    if (!directory) {
        mknd_err(c->ast, "OUT_OF_MEMORY");
        return -1;
    }
    bin = exepath();
#ifdef _WIN32
    tool = mprintf("%s/rxc.exe", bin);
#else
    tool = mprintf("%s/rxc", bin);
#endif
    output = mprintf("%s/contract", directory);
    assembly = mprintf("%s.rxas", output);
    source = location ? mprintf("%s/%s", location, file) : strdup(file);
    while (root->import_locations && root->import_locations[imports]) ++imports;
    while (root->source_import_locations && root->source_import_locations[sources]) ++sources;
    argv = calloc(20 + 2 * imports + 2 * sources, sizeof(char *));
    argv[n++] = tool;
    /* Parent paths already contain its selected executable root, if any.
     * Preserve that order instead of letting the child add another root. */
    argv[n++] = "--no-exe-import";
    if (root->auto_import_rxas) argv[n++] = "--import-rxas";
    if (root->disable_exits) argv[n++] = "-x";
    if (!root->optimise) argv[n++] = "-n";
    argv[n++] = "--level";
    argv[n++] = level == LEVELG ? "g" : level == LEVELL ? "l" : "b";
    argv[n++] = "-o"; argv[n++] = output;
    for (i = 0; i < imports; ++i) { argv[n++] = "-i"; argv[n++] = root->import_locations[i]; }
    for (i = 0; i < sources; ++i) { argv[n++] = "-s"; argv[n++] = root->source_import_locations[i]; }
    argv[n++] = source;
    status = run_typed_compiler(tool, argv);
    if (!status) rxcp_import_rxas_contracts(c, assembly, file);
    else {
        fprintf(stderr, "Typed source import %s failed (rxc status %d)\n", file, status);
        mknd_err3(c->ast, "SYNTAX_ERROR_IN_IMPORT_DECL", "name", file,
                  "import_file", file, "detail", "typed source compiler failed");
    }
#ifdef _WIN32
    {
        wchar_t *file_path = wide_path(assembly), *dir_path = wide_path(directory);
        if (file_path) _wremove(file_path);
        if (dir_path) _wrmdir(dir_path);
        free(file_path); free(dir_path);
    }
#else
    remove(assembly);
    rmdir(directory);
#endif
    free(argv); free(source); free(assembly); free(output); free(tool); free(bin); free(directory);
    return status ? -1 : 1;
}

#endif
