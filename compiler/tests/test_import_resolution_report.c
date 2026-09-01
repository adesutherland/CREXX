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

static char *read_file(const char *path) {
    FILE *file;
    long length;
    char *contents;

    file = fopen(path, "rb");
    if (!file) return 0;
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }
    length = ftell(file);
    if (length < 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    contents = (char *)malloc((size_t)length + 1);
    if (!contents) {
        fclose(file);
        return 0;
    }
    if (fread(contents, 1, (size_t)length, file) != (size_t)length) {
        free(contents);
        fclose(file);
        return 0;
    }
    contents[length] = 0;
    fclose(file);
    return contents;
}

static int contains(const char *text, const char *expected) {
    return text && expected && strstr(text, expected) != 0;
}

static void release_context_report(Context *context) {
    if (context->importable_file_list) {
        rxfl_fre(context->importable_file_list);
        context->importable_file_list = 0;
    }
    rxcp_import_report_free(context);
    free(context->import_resolution_report_path);
    context->import_resolution_report_path = 0;
}

static int run_newer_rxas_case(const char *base, const char *root0, const char *root1,
                               const char *rxbin_path, const char *rxas_path,
                               const char *report_path) {
    Context context;
    char *roots[3];
    char *report;
    int ok;

    if (!set_file_time(rxbin_path, (time_t)1700000000) ||
        !set_file_time(rxas_path, (time_t)1700000100)) return 0;

    memset(&context, 0, sizeof(context));
    context.master_context = &context;
    context.location = (char *)base;
    context.file_name = "consumer.crexx";
    context.initial_source_extension = "crexx";
    roots[0] = (char *)root0;
    roots[1] = (char *)root1;
    roots[2] = 0;
    context.import_locations = roots;
    context.auto_import_rxas = 1;
    context.executable_import_included = 1;
    context.executable_import_root_index = 1;
    context.import_resolution_report_path = strdup(report_path);
    if (!context.import_resolution_report_path) return 0;

    context.importable_file_list = rxfl_lst(&context);
    ok = context.importable_file_list && context.importable_file_list[0] &&
         context.importable_file_list[0]->type == REXX_FILE &&
         context.importable_file_list[0]->root_kind == RXCP_IMPORT_ROOT_PRIMARY_SOURCE &&
         context.importable_file_list[1] &&
         context.importable_file_list[1]->type == RXAS_FILE &&
         context.importable_file_list[1]->root_kind == RXCP_IMPORT_ROOT_BINARY &&
         context.importable_file_list[1]->root_index == 0 &&
         context.importable_file_list[2] == 0;
    if (ok) ok = rxcp_import_report_write(&context) == 0;
    if (ok) ok = rxcp_import_report_write(&context) == 0;
    report = ok ? read_file(report_path) : 0;
    if (ok) {
        ok = contains(report, "\"schema_version\": \"crexx.import-resolution-report/v1\"") &&
             contains(report, "\"status\": \"observed\"") &&
             contains(report, "\"binary_roots\": [\"@binary/0\", \"@executable/0\"]") &&
             contains(report, "@primary-source/0/choice.crexx") &&
             contains(report, "@executable/0/choice.rxbin") &&
             contains(report, "\"decision\": \"replaced\"") &&
             contains(report, "\"reason\": \"newer-mtime\"") &&
             contains(report, "\"reason\": \"earlier-ordered-root\"") &&
             contains(report, "\"auto_import_rxas\": true") &&
             contains(report, "\"executable_directory\": \"included\"") &&
             contains(report, "\"provider_bindings\": []");
    }
    free(report);
    release_context_report(&context);
    return ok;
}

static int run_tied_case(const char *base, const char *root0,
                         const char *rxbin_path, const char *rxas_path,
                         const char *report_path) {
    Context context;
    char *roots[2];
    char *report;
    int ok;

    if (!set_file_time(rxbin_path, (time_t)1700000200) ||
        !set_file_time(rxas_path, (time_t)1700000200)) return 0;

    memset(&context, 0, sizeof(context));
    context.master_context = &context;
    context.location = (char *)base;
    context.file_name = "consumer.crexx";
    context.initial_source_extension = "crexx";
    roots[0] = (char *)root0;
    roots[1] = 0;
    context.import_locations = roots;
    context.auto_import_rxas = 1;
    context.executable_import_included = 0;
    context.import_resolution_report_path = strdup(report_path);
    if (!context.import_resolution_report_path) return 0;

    context.importable_file_list = rxfl_lst(&context);
    ok = context.importable_file_list && context.importable_file_list[0] &&
         context.importable_file_list[0]->type == REXX_FILE &&
         context.importable_file_list[0]->root_kind == RXCP_IMPORT_ROOT_PRIMARY_SOURCE &&
         context.importable_file_list[1] &&
         context.importable_file_list[1]->type == RXBIN_FILE &&
         context.importable_file_list[1]->root_kind == RXCP_IMPORT_ROOT_BINARY &&
         context.importable_file_list[1]->root_index == 0 &&
         context.importable_file_list[2] == 0;
    if (ok) ok = rxcp_import_report_write(&context) == 0;
    report = ok ? read_file(report_path) : 0;
    if (ok) {
        ok = contains(report, "\"reason\": \"kind-tiebreak-rxbin\"") &&
             contains(report, "\"binary_roots\": [\"@binary/0\"]") &&
             contains(report, "\"executable_directory\": \"excluded\"");
    }
    free(report);
    release_context_report(&context);
    return ok;
}

int main(void) {
    char base[TEST_PATH_SIZE];
    char root0[TEST_PATH_SIZE];
    char root1[TEST_PATH_SIZE];
    char root0_rxbin[TEST_PATH_SIZE];
    char root0_rxas[TEST_PATH_SIZE];
    char root1_rxbin[TEST_PATH_SIZE];
    char source_candidate[TEST_PATH_SIZE];
    char newer_report[TEST_PATH_SIZE];
    char tied_report[TEST_PATH_SIZE];
    int ok;

    if (snprintf(base, sizeof(base), "import-resolution-report-%ld", (long)test_getpid()) < 0 ||
        !make_path(root0, sizeof(root0), base, "root0") ||
        !make_path(root1, sizeof(root1), base, "root1") ||
        !make_path(root0_rxbin, sizeof(root0_rxbin), root0, "choice.rxbin") ||
        !make_path(root0_rxas, sizeof(root0_rxas), root0, "choice.rxas") ||
        !make_path(root1_rxbin, sizeof(root1_rxbin), root1, "choice.rxbin") ||
        !make_path(source_candidate, sizeof(source_candidate), base, "choice.crexx") ||
        !make_path(newer_report, sizeof(newer_report), base, "newer.json") ||
        !make_path(tied_report, sizeof(tied_report), base, "tied.json")) {
        fprintf(stderr, "Could not format import resolution fixture paths.\n");
        return 1;
    }

    if (test_mkdir(base) != 0 || test_mkdir(root0) != 0 || test_mkdir(root1) != 0 ||
        !write_file(root0_rxbin, "root0-rxbin\n") ||
        !write_file(root0_rxas, "root0-rxas\n") ||
        !write_file(root1_rxbin, "root1-rxbin\n") ||
        !write_file(source_candidate, "options levelb\n")) {
        fprintf(stderr, "Could not create import resolution fixture.\n");
        return 1;
    }
    set_file_time(root1_rxbin, (time_t)1700000300);

    ok = run_newer_rxas_case(base, root0, root1, root0_rxbin, root0_rxas, newer_report);
    if (!ok) fprintf(stderr, "Newer RXAS resolution case failed.\n");
    if (ok) {
        ok = run_tied_case(base, root0, root0_rxbin, root0_rxas, tied_report);
        if (!ok) fprintf(stderr, "RXAS/RXBIN tie resolution case failed.\n");
    }

    test_unlink(newer_report);
    test_unlink(tied_report);
    test_unlink(root0_rxbin);
    test_unlink(root0_rxas);
    test_unlink(root1_rxbin);
    test_unlink(source_candidate);
    test_rmdir(root0);
    test_rmdir(root1);
    test_rmdir(base);
    return ok ? 0 : 1;
}
