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
#define rxcp_test_setenv(name, value) (_putenv_s((name), (value)) == 0)
#define rxcp_test_unsetenv(name) (_putenv_s((name), "") == 0)
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
#define rxcp_test_setenv(name, value) (setenv((name), (value), 1) == 0)
#define rxcp_test_unsetenv(name) (unsetenv((name)) == 0)
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

static const char *external_function_source =
        "options levelb\n"
        "import rxfnsb\n"
        "\n"
        "main: procedure\n"
        "\n"
        " name[1]  = \"Fred\"\n"
        " qty[1]   = 3\n"
        " price[1]= 12.45\n"
        "\n"
        " name[2]   = \"Alice\"\n"
        " qty[2]    = 15\n"
        " price[2]  = 3.11\n"
        "\n"
        " name[3]   = \"Bob\"\n"
        " qty[3]    = 7\n"
        " price[3]  = 98.75\n"
        "\n"
        " name[4]   = \"Charlotte\"\n"
        " qty[4]   = 102\n"
        " price[4]  = 1.05\n"
        "\n"
        " name[5]   = \"Dave\"\n"
        " qty[5]    = 1\n"
        " price[5] = 250.99\n"
        "\n"
        " do i=1 to name[0]\n"
        "    total = qty.i * price.i\n"
        "    fsay \"{i:>3} {name.i:<12} {qty.i:>5} {price.i:>6.2} Total={total:>6.2}\"\n"
        " end\n"
        " say 'SUCCESS'\n";

static const char *exit_source =
        "options levelb\n"
        "import rxfnsb\n"
        "\n"
        "main: procedure\n"
        "  i = .int\n"
        "  i = 99\n"
        "\n"
        "  dummy i\n"
        "\n"
        "  say 'after-exit ' || i\n"
        "  say 'SUCCESS'\n"
        "  return\n";

static int write_text_file(const char *path, const char *content) {
    FILE *fp;

    fp = fopen(path, "w");
    if (!fp) return 0;
    if (content) fputs(content, fp);
    fclose(fp);
    return 1;
}

static int copy_binary_file(const char *source_path, const char *target_path) {
    FILE *source;
    FILE *target;
    char buffer[4096];
    size_t bytes_read;

    source = fopen(source_path, "rb");
    if (!source) return 0;

    target = fopen(target_path, "wb");
    if (!target) {
        fclose(source);
        return 0;
    }

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        if (fwrite(buffer, 1, bytes_read, target) != bytes_read) {
            fclose(source);
            fclose(target);
            return 0;
        }
    }

    fclose(source);
    fclose(target);
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

static void cleanup_feature_cache_paths(const char *external_file,
                                        const char *exit_file,
                                        const char *library_file,
                                        const char *exit_module_file,
                                        const char *directory) {
    if (external_file) remove(external_file);
    if (exit_file) remove(exit_file);
    if (library_file) remove(library_file);
    if (exit_module_file) remove(exit_module_file);
    if (directory) rxcp_test_rmdir(directory);
}

static int build_bin_dir(const char *cwd, char *bin_dir, size_t bin_dir_size) {
    char *compiler_dir;
    char *build_root;
    int ok;

    if (!cwd || !bin_dir || bin_dir_size == 0) return 0;

    compiler_dir = file_dir(cwd);
    if (!compiler_dir) return 0;

    build_root = file_dir(compiler_dir);
    free(compiler_dir);
    if (!build_root) return 0;

    ok = snprintf(bin_dir, bin_dir_size, "%s/bin", build_root) < (int)bin_dir_size;
    free(build_root);
    return ok;
}

int main(void) {
    char original_cwd[MAXFILEPATH];
    char bin_dir[MAXFILEPATH];
    char sandbox[MAXFILEPATH];
    char dir_a[MAXFILEPATH];
    char dir_b[MAXFILEPATH];
    char main_a[MAXFILEPATH];
    char dep_a[MAXFILEPATH];
    char main_b[MAXFILEPATH];
    char dep_b[MAXFILEPATH];
    char feature_sandbox[MAXFILEPATH];
    char external_file[MAXFILEPATH];
    char exit_file[MAXFILEPATH];
    char library_source[MAXFILEPATH];
    char exit_module_source[MAXFILEPATH];
    char library_file[MAXFILEPATH];
    char exit_module_file[MAXFILEPATH];
    RXCPHighlightCacheStats stats1;
    RXCPHighlightCacheStats stats2;
    RXCPHighlightCacheStats stats3;
    RXCPHighlightCacheStats stats4;
    RXCPHighlightCacheStats external_stats1;
    RXCPHighlightCacheStats external_stats2;
    RXCPHighlightCacheStats exit_stats1;
    RXCPHighlightCacheStats exit_stats2;
    char *saved_exit_module;
    long pid;

    pid = (long)rxcp_test_getpid();
    if (!expect_true(rxcp_test_getcwd(original_cwd, sizeof(original_cwd)) != 0,
                     "Failed to capture original working directory")) return 1;
    if (!expect_true(build_bin_dir(original_cwd, bin_dir, sizeof(bin_dir)),
                     "Failed to derive build bin directory")) return 1;

    snprintf(sandbox, sizeof(sandbox), "highlight_cache_%ld", pid);
    snprintf(dir_a, sizeof(dir_a), "%s/a", sandbox);
    snprintf(dir_b, sizeof(dir_b), "%s/b", sandbox);
    snprintf(main_a, sizeof(main_a), "%s/a/main.rexx", sandbox);
    snprintf(dep_a, sizeof(dep_a), "%s/a/dep.rexx", sandbox);
    snprintf(main_b, sizeof(main_b), "%s/b/main.rexx", sandbox);
    snprintf(dep_b, sizeof(dep_b), "%s/b/dep.rexx", sandbox);
    snprintf(feature_sandbox, sizeof(feature_sandbox), "%s/highlight_feature_cache_%ld", bin_dir, pid);
    snprintf(external_file, sizeof(external_file), "%s/external_cache.rexx", feature_sandbox);
    snprintf(exit_file, sizeof(exit_file), "%s/exit_cache.rexx", feature_sandbox);
    snprintf(library_source, sizeof(library_source), "%s/library.rxbin", bin_dir);
    snprintf(exit_module_source, sizeof(exit_module_source), "%s/rxcexits.rxbin", bin_dir);
    snprintf(library_file, sizeof(library_file), "%s/library.rxbin", feature_sandbox);
    snprintf(exit_module_file, sizeof(exit_module_file), "%s/rxcexits.rxbin", feature_sandbox);
    saved_exit_module = getenv("RXCP_EXIT_MODULE") ? strdup(getenv("RXCP_EXIT_MODULE")) : 0;

    cleanup_test_paths(main_a, dep_a, dir_a);
    cleanup_test_paths(main_b, dep_b, dir_b);
    rxcp_test_rmdir(sandbox);
    cleanup_feature_cache_paths(external_file, exit_file, library_file, exit_module_file, feature_sandbox);

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

    if (!expect_true(rxcp_test_chdir(original_cwd) == 0, "Failed to return to compiler test directory")) goto fail;
    if (!expect_true(rxcp_test_mkdir(feature_sandbox) == 0, "Failed to create feature cache sandbox")) goto fail;
    if (!expect_true(copy_binary_file(library_source, library_file),
                     "Failed to copy library.rxbin into feature cache sandbox")) goto fail;
    if (!expect_true(copy_binary_file(exit_module_source, exit_module_file),
                     "Failed to copy rxcexits.rxbin into feature cache sandbox")) goto fail;
    if (!expect_true(write_text_file(external_file, external_function_source),
                     "Failed to write external-function cache fixture")) goto fail;
    if (!expect_true(write_text_file(exit_file, exit_source),
                     "Failed to write exit cache fixture")) goto fail;
    if (!expect_true(rxcp_test_chdir(feature_sandbox) == 0, "Failed to enter feature cache sandbox")) goto fail;
    if (!expect_true(rxcp_test_setenv("RXCP_EXIT_MODULE", "rxcexits"),
                     "Failed to set RXCP_EXIT_MODULE for cache fixture")) goto fail;

    rxcp_highlight_controller_reset_cache();

    if (!expect_true(parse_document(external_file, external_function_source),
                     "External-function cache warm parse failed")) goto fail;
    rxcp_highlight_controller_get_cache_stats(&external_stats1);
    if (!expect_true(external_stats1.generation == 1,
                     "External-function cache should start at generation 1")) goto fail;
    if (!expect_true(external_stats1.cached_import_function_count > 0,
                     "External-function parse should materialize imported function signatures")) goto fail;

    if (!expect_true(parse_document(external_file, external_function_source),
                     "External-function cache reuse parse failed")) goto fail;
    rxcp_highlight_controller_get_cache_stats(&external_stats2);
    if (!expect_true(external_stats2.generation == external_stats1.generation,
                     "External-function reuse should keep the same cache generation")) goto fail;
    if (!expect_true(external_stats2.import_inventory_warm_count == external_stats1.import_inventory_warm_count,
                     "External-function reuse should not rescan import inventory")) goto fail;
    if (!expect_true(external_stats2.cached_import_function_count == external_stats1.cached_import_function_count,
                     "External-function reuse should preserve imported signature count")) goto fail;

    rxcp_highlight_controller_reset_cache();

    if (!expect_true(parse_document(exit_file, exit_source),
                     "Exit cache warm parse failed")) goto fail;
    rxcp_highlight_controller_get_cache_stats(&exit_stats1);
    if (!expect_true(exit_stats1.generation == 1,
                     "Exit cache should start at generation 1")) goto fail;
    if (!expect_true(exit_stats1.cached_exit_primary_count > 0,
                     "Exit parse should register exits in the retained root")) goto fail;

    if (!expect_true(parse_document(exit_file, exit_source),
                     "Exit cache reuse parse failed")) goto fail;
    rxcp_highlight_controller_get_cache_stats(&exit_stats2);
    if (!expect_true(exit_stats2.generation == exit_stats1.generation,
                     "Exit reuse should keep the same cache generation")) goto fail;
    if (!expect_true(exit_stats2.exit_warm_count == exit_stats1.exit_warm_count,
                     "Exit reuse should not rerun exit discovery")) goto fail;
    if (!expect_true(exit_stats2.cached_exit_primary_count == exit_stats1.cached_exit_primary_count,
                     "Exit reuse should preserve registered exit count")) goto fail;

    rxcp_highlight_controller_reset_cache();
    rxcp_test_chdir(original_cwd);
    cleanup_test_paths(main_a, dep_a, dir_a);
    cleanup_test_paths(main_b, dep_b, dir_b);
    rxcp_test_rmdir(sandbox);
    cleanup_feature_cache_paths(external_file, exit_file, library_file, exit_module_file, feature_sandbox);
    if (saved_exit_module) {
        rxcp_test_setenv("RXCP_EXIT_MODULE", saved_exit_module);
        free(saved_exit_module);
    } else {
        rxcp_test_unsetenv("RXCP_EXIT_MODULE");
    }
    return 0;

fail:
    rxcp_highlight_controller_reset_cache();
    rxcp_test_chdir(original_cwd);
    cleanup_test_paths(main_a, dep_a, dir_a);
    cleanup_test_paths(main_b, dep_b, dir_b);
    rxcp_test_rmdir(sandbox);
    cleanup_feature_cache_paths(external_file, exit_file, library_file, exit_module_file, feature_sandbox);
    if (saved_exit_module) {
        rxcp_test_setenv("RXCP_EXIT_MODULE", saved_exit_module);
        free(saved_exit_module);
    } else {
        rxcp_test_unsetenv("RXCP_EXIT_MODULE");
    }
    return 1;
}
