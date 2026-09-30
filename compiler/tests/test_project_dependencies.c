/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#if defined(_WIN32)
#include <direct.h>
#include <io.h>
#include <process.h>
#include <sys/utime.h>
#define test_getpid _getpid
#define test_mkdir(path) _mkdir(path)
#define test_rmdir _rmdir
#define test_unlink _unlink
#define test_utimbuf _utimbuf
#define test_utime _utime
#else
#include <unistd.h>
#include <utime.h>
#define test_getpid getpid
#define test_mkdir(path) mkdir((path), 0700)
#define test_rmdir rmdir
#define test_unlink unlink
#define test_utimbuf utimbuf
#define test_utime utime
#endif

#include "rxcpmain.h"

#define TEST_PATH_SIZE 1024

static int make_path(char *path, size_t path_size, const char *directory, const char *name) {
    int written;

    written = snprintf(path, path_size, "%s/%s", directory, name);
    return written >= 0 && (size_t)written < path_size;
}

static int write_file(const char *path, const char *contents) {
    FILE *file;
    size_t length;

    file = fopen(path, "wb");
    if (!file) return 0;
    length = strlen(contents);
    if (fwrite(contents, 1, length, file) != length) {
        fclose(file);
        return 0;
    }
    return fclose(file) == 0;
}

static int set_file_time(const char *path, time_t value) {
    struct test_utimbuf times;

    times.actime = value;
    times.modtime = value;
    return test_utime(path, &times) == 0;
}

/* Exercise the resolver protocol directly with inert files: no plugin code or
 * bytecode is executed. Each check gets a fresh discovery list, as rxc does. */
static int snapshot(const char *base, const char *root0, const char *root1,
                    const char *path, int check, int loaded) {
    Context context;
    char *roots[3];
    size_t i;
    int result;
    memset(&context, 0, sizeof(context));
    context.master_context = &context;
    context.location = (char *)base;
    context.file_name = "consumer.crexx";
    context.initial_source_extension = "crexx";
    context.cli_level_override = UNKNOWN;
    roots[0] = (char *)root0;
    roots[1] = (char *)root1;
    roots[2] = 0;
    context.import_locations = roots;
    context.auto_import_rxas = 1;
    context.importable_file_list = rxfl_lst(&context);
    if (!context.importable_file_list) return 1;
    if (loaded) {
        for (i = 0; context.importable_file_list[i]; i++) {
            if (context.importable_file_list[i]->type == REXX_FILE)
                context.importable_file_list[i]->imported = 1;
        }
    }
    result = rxcp_project_dependencies(&context, path, check);
    rxfl_fre(context.importable_file_list);
    return result;
}

#define REQUIRE(condition) do { if (!(condition)) { \
    fprintf(stderr, "dependency check failed at line %d: %s\n", __LINE__, #condition); \
    ok = 0; goto cleanup; } } while (0)

int main(void) {
    char base[TEST_PATH_SIZE], root0[TEST_PATH_SIZE], root1[TEST_PATH_SIZE];
    char source[TEST_PATH_SIZE], binary[TEST_PATH_SIZE], assembly[TEST_PATH_SIZE];
    char shadow[TEST_PATH_SIZE], path[TEST_PATH_SIZE], primary[TEST_PATH_SIZE];
    int ok = 1;
    snprintf(base, sizeof(base), "project dependencies-%ld", (long)test_getpid());
    make_path(root0, sizeof(root0), base, "first root");
    make_path(root1, sizeof(root1), base, "second root");
    make_path(source, sizeof(source), base, "hidden.crexx");
    make_path(primary, sizeof(primary), base, "consumer.crexx");
    make_path(binary, sizeof(binary), root1, "choice.rxbin");
    make_path(assembly, sizeof(assembly), root1, "choice.rxas");
    make_path(shadow, sizeof(shadow), root0, "choice.rxbin");
    make_path(path, sizeof(path), base, "snapshot");
    REQUIRE(test_mkdir(base) == 0);
    REQUIRE(test_mkdir(root0) == 0);
    REQUIRE(test_mkdir(root1) == 0);
    REQUIRE(write_file(primary, "options levelb\nsay 1\n"));
    REQUIRE(write_file(source, "options levelb\nnamespace hidden\nx = 1\n"));
    REQUIRE(write_file(binary, "inert binary candidate"));
    REQUIRE(write_file(assembly, "inert assembly candidate"));
    REQUIRE(set_file_time(binary, (time_t)1700000200));
    REQUIRE(set_file_time(assembly, (time_t)1700000100));
    REQUIRE(snapshot(base, root0, root1, path, 0, 0) == 0);
    REQUIRE(snapshot(base, root0, root1, path, 1, 0) == 0);
    /* Excluded implementation is not read by validation/optimisation. */
    REQUIRE(write_file(source, "options levelb\nnamespace hidden\nx = 2\n"));
    REQUIRE(snapshot(base, root0, root1, path, 1, 0) == 0);
    REQUIRE(write_file(source, "options levelb\nnamespace visible\nx = 2\n"));
    REQUIRE(snapshot(base, root0, root1, path, 1, 0) != 0);
    /* Loaded source bodies remain dependencies, including private changes. */
    REQUIRE(snapshot(base, root0, root1, path, 0, 1) == 0);
    REQUIRE(write_file(source, "options levelb\nnamespace visible\nx = 3\n"));
    REQUIRE(snapshot(base, root0, root1, path, 1, 0) != 0);
    REQUIRE(snapshot(base, root0, root1, path, 0, 1) == 0);
    REQUIRE(write_file(binary, "changed binary candidate"));
    REQUIRE(set_file_time(binary, (time_t)1700000200));
    REQUIRE(snapshot(base, root0, root1, path, 1, 0) != 0);
    REQUIRE(snapshot(base, root0, root1, path, 0, 1) == 0);
    /* Timestamp-only RXAS/RXBIN selection must not escape a content key. */
    REQUIRE(set_file_time(assembly, (time_t)1700000300));
    REQUIRE(snapshot(base, root0, root1, path, 1, 0) != 0);
    REQUIRE(snapshot(base, root0, root1, path, 0, 1) == 0);
    REQUIRE(write_file(shadow, "earlier root wins"));
    REQUIRE(snapshot(base, root0, root1, path, 1, 0) != 0);
    REQUIRE(snapshot(base, root0, root1, path, 0, 1) == 0);
    REQUIRE(snapshot(base, root1, root0, path, 1, 0) != 0);
    REQUIRE(test_unlink(shadow) == 0);
    REQUIRE(snapshot(base, root0, root1, path, 1, 0) != 0);
    REQUIRE(write_file(path, "malformed"));
    REQUIRE(snapshot(base, root0, root1, path, 1, 0) != 0);
cleanup:
    test_unlink(primary); test_unlink(source); test_unlink(binary); test_unlink(assembly);
    test_unlink(shadow); test_unlink(path);
    test_rmdir(root0); test_rmdir(root1); test_rmdir(base);
    if (ok) puts("PASS: compiler project dependency invalidation");
    return ok ? 0 : 1;
}
