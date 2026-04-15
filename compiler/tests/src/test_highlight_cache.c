#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#include <process.h>
#include <sys/utime.h>
#define rxcp_test_mkdir(path) _mkdir(path)
#define rxcp_test_rmdir(path) _rmdir(path)
#define rxcp_test_chdir(path) _chdir(path)
#define rxcp_test_getcwd _getcwd
#define rxcp_test_getpid _getpid
#define rxcp_test_utimbuf _utimbuf
#define rxcp_test_utime _utime
#else
#include <unistd.h>
#include <utime.h>
#define rxcp_test_mkdir(path) mkdir(path, 0700)
#define rxcp_test_rmdir(path) rmdir(path)
#define rxcp_test_chdir(path) chdir(path)
#define rxcp_test_getcwd getcwd
#define rxcp_test_getpid getpid
#define rxcp_test_utimbuf utimbuf
#define rxcp_test_utime utime
#endif

#include "platform.h"
#include "rxcp_highlight_controller.h"
#include "dslsyntax_common.h"
#include "dslsyntax_editor.h"

static int expect_true(int condition, const char *message) {
    if (condition) return 1;
    fprintf(stderr, "%s\n", message);
    return 0;
}

static int write_text_file(const char *path, const char *content) {
    FILE *fp;

    fp = fopen(path, "w");
    if (!fp) return 0;
    if (content) fputs(content, fp);
    fclose(fp);
    return 1;
}

static int bump_file_timestamp(const char *path) {
    struct stat st;
    struct rxcp_test_utimbuf times;

    if (stat(path, &st) != 0) return 0;
    times.actime = st.st_atime;
    times.modtime = st.st_mtime + 2;
    return rxcp_test_utime(path, &times) == 0;
}

static int parse_document(const char *document_path, const char *content) {
    CodeBuffer *cb;
    InitialLoad *load;
    int ok;

    cb = create_code_buffer(NULL, rxc_highlight_controller_parse);
    if (!cb) return 0;

    load = create_initial_load(document_path, content);
    if (!load) {
        free_code_buffer(cb);
        return 0;
    }

    base_load_initial_content(cb, load);
    ok = cb->parse_tree && cb->parse_tree->root && cb->parse_tree->root->type == PARSE_TREE_FILE;
    free_code_buffer(cb);
    return ok;
}

static void cleanup_test_paths(const char *main_file,
                               const char *dep_file,
                               const char *directory) {
    if (main_file) remove(main_file);
    if (dep_file) remove(dep_file);
    if (directory) rxcp_test_rmdir(directory);
}

int main(void) {
    char original_cwd[MAXFILEPATH];
    char sandbox[MAXFILEPATH];
    char dir_a[MAXFILEPATH];
    char dir_b[MAXFILEPATH];
    char main_a[MAXFILEPATH];
    char dep_a[MAXFILEPATH];
    char main_b[MAXFILEPATH];
    char dep_b[MAXFILEPATH];
    RXCPHighlightCacheStats stats1;
    RXCPHighlightCacheStats stats2;
    RXCPHighlightCacheStats stats3;
    RXCPHighlightCacheStats stats4;
    long pid;

    pid = (long)rxcp_test_getpid();
    if (!expect_true(rxcp_test_getcwd(original_cwd, sizeof(original_cwd)) != 0,
                     "Failed to capture original working directory")) return 1;

    snprintf(sandbox, sizeof(sandbox), "highlight_cache_%ld", pid);
    snprintf(dir_a, sizeof(dir_a), "%s/a", sandbox);
    snprintf(dir_b, sizeof(dir_b), "%s/b", sandbox);
    snprintf(main_a, sizeof(main_a), "%s/a/main.rexx", sandbox);
    snprintf(dep_a, sizeof(dep_a), "%s/a/dep.rexx", sandbox);
    snprintf(main_b, sizeof(main_b), "%s/b/main.rexx", sandbox);
    snprintf(dep_b, sizeof(dep_b), "%s/b/dep.rexx", sandbox);

    cleanup_test_paths(main_a, dep_a, dir_a);
    cleanup_test_paths(main_b, dep_b, dir_b);
    rxcp_test_rmdir(sandbox);

    if (!expect_true(rxcp_test_mkdir(sandbox) == 0, "Failed to create cache test sandbox")) return 1;
    if (!expect_true(rxcp_test_mkdir(dir_a) == 0, "Failed to create first cache test directory")) {
        rxcp_test_rmdir(sandbox);
        return 1;
    }
    if (!expect_true(rxcp_test_mkdir(dir_b) == 0, "Failed to create second cache test directory")) {
        cleanup_test_paths(main_a, dep_a, dir_a);
        rxcp_test_rmdir(sandbox);
        return 1;
    }
    if (!expect_true(rxcp_test_chdir(sandbox) == 0, "Failed to enter cache test sandbox")) {
        cleanup_test_paths(main_a, dep_a, dir_a);
        cleanup_test_paths(main_b, dep_b, dir_b);
        rxcp_test_rmdir(sandbox);
        return 1;
    }

    if (!expect_true(write_text_file("a/main.rexx", "say 'file a'\n"), "Failed to write first main file")) goto fail;
    if (!expect_true(write_text_file("a/dep.rexx", "say 'dep a'\n"), "Failed to write first dependency file")) goto fail;
    if (!expect_true(write_text_file("b/main.rexx", "say 'file b'\n"), "Failed to write second main file")) goto fail;
    if (!expect_true(write_text_file("b/dep.rexx", "say 'dep b'\n"), "Failed to write second dependency file")) goto fail;

    rxcp_highlight_controller_reset_cache();

    if (!expect_true(parse_document("a/main.rexx", "fsay 'one'\n"), "Initial parser-mode cache warm parse failed")) goto fail;
    rxcp_highlight_controller_get_cache_stats(&stats1);
    if (!expect_true(stats1.generation == 1, "First parse should create cache generation 1")) goto fail;
    if (!expect_true(stats1.invalidation_count == 0, "First parse should not count as an invalidation")) goto fail;
    if (!expect_true(stats1.import_inventory_warm_count == 1, "First parse should warm import inventory once")) goto fail;
    if (!expect_true(stats1.exit_warm_count == 1, "First parse should warm exit discovery once")) goto fail;
    if (!expect_true(stats1.cached_import_file_count > 0, "Warm cache should retain at least one importable file")) goto fail;
    if (!expect_true(stats1.watched_directory_count > 0, "Warm cache should watch parser search directories")) goto fail;
    if (!expect_true(stats1.watched_file_count > 0, "Warm cache should watch importable files and modules")) goto fail;

    if (!expect_true(parse_document("a/main.rexx", "fsay 'two'\n"), "Second parser-mode parse failed")) goto fail;
    rxcp_highlight_controller_get_cache_stats(&stats2);
    if (!expect_true(stats2.generation == stats1.generation, "Stable cache inputs should reuse the same generation")) goto fail;
    if (!expect_true(stats2.invalidation_count == stats1.invalidation_count, "Stable cache inputs should not invalidate")) goto fail;
    if (!expect_true(stats2.import_inventory_warm_count == stats1.import_inventory_warm_count,
                     "Stable cache inputs should not rescan import inventory")) goto fail;
    if (!expect_true(stats2.exit_warm_count == stats1.exit_warm_count,
                     "Stable cache inputs should not rediscover exits")) goto fail;

    if (!expect_true(write_text_file("a/dep.rexx", "say 'dep a changed'\n"), "Failed to rewrite first dependency file")) goto fail;
    if (!expect_true(bump_file_timestamp("a/dep.rexx"), "Failed to bump dependency timestamp")) goto fail;

    if (!expect_true(parse_document("a/main.rexx", "fsay 'three'\n"), "Timestamp invalidation parse failed")) goto fail;
    rxcp_highlight_controller_get_cache_stats(&stats3);
    if (!expect_true(stats3.generation > stats2.generation, "Dependency timestamp change should rebuild the cache")) goto fail;
    if (!expect_true(stats3.invalidation_count > stats2.invalidation_count, "Dependency timestamp change should increment invalidations")) goto fail;
    if (!expect_true(stats3.import_inventory_warm_count > stats2.import_inventory_warm_count,
                     "Dependency timestamp change should rescan import inventory")) goto fail;
    if (!expect_true(stats3.exit_warm_count > stats2.exit_warm_count,
                     "Dependency timestamp change should rediscover exits")) goto fail;

    if (!expect_true(parse_document("b/main.rexx", "fsay 'four'\n"), "Search-path invalidation parse failed")) goto fail;
    rxcp_highlight_controller_get_cache_stats(&stats4);
    if (!expect_true(stats4.generation > stats3.generation, "Changing document search paths should rebuild the cache")) goto fail;
    if (!expect_true(stats4.invalidation_count > stats3.invalidation_count,
                     "Changing document search paths should increment invalidations")) goto fail;

    rxcp_highlight_controller_reset_cache();
    rxcp_test_chdir(original_cwd);
    cleanup_test_paths(main_a, dep_a, dir_a);
    cleanup_test_paths(main_b, dep_b, dir_b);
    rxcp_test_rmdir(sandbox);
    return 0;

fail:
    rxcp_highlight_controller_reset_cache();
    rxcp_test_chdir(original_cwd);
    cleanup_test_paths(main_a, dep_a, dir_a);
    cleanup_test_paths(main_b, dep_b, dir_b);
    rxcp_test_rmdir(sandbox);
    return 1;
}
