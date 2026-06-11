#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <sys/stat.h>
#define CREXX_GETCWD _getcwd
#define CREXX_CHDIR _chdir
#define CREXX_MKDIR(path) _mkdir(path)
#define CREXX_RMDIR _rmdir
#define CREXX_GETPID _getpid
#define CREXX_OPEN _open
#define CREXX_CLOSE _close
#define CREXX_DUP _dup
#define CREXX_DUP2 _dup2
#define CREXX_OPEN_FLAGS (O_WRONLY | O_CREAT | O_TRUNC | O_BINARY)
#define CREXX_OPEN_MODE (_S_IREAD | _S_IWRITE)
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define CREXX_GETCWD getcwd
#define CREXX_CHDIR chdir
#define CREXX_MKDIR(path) mkdir(path, 0700)
#define CREXX_RMDIR rmdir
#define CREXX_GETPID getpid
#define CREXX_OPEN open
#define CREXX_CLOSE close
#define CREXX_DUP dup
#define CREXX_DUP2 dup2
#define CREXX_OPEN_FLAGS (O_WRONLY | O_CREAT | O_TRUNC)
#define CREXX_OPEN_MODE 0600
#endif

static int open_redirect_file(const char *path) {
    return CREXX_OPEN(path, CREXX_OPEN_FLAGS, CREXX_OPEN_MODE);
}

/* Function to copy a file - C90 compliant */
static int copy_file(const char *src_path, const char *dst_path) {
    FILE *src, *dst;
    char buffer[4096];
    size_t n;
    int src_close;
    int dst_close;

    src = fopen(src_path, "rb");
    if (!src) return 0;
    dst = fopen(dst_path, "wb");
    if (!dst) {
        fclose(src);
        return 0;
    }

    while ((n = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, n, dst) != n) {
            fclose(src);
            fclose(dst);
            return 0;
        }
    }

    if (ferror(src)) {
        fclose(src);
        fclose(dst);
        return 0;
    }

    src_close = fclose(src);
    dst_close = fclose(dst);
    return src_close == 0 && dst_close == 0;
}

/* Function to get base name of a path */
static const char *get_base_name(const char *path) {
    const char *last_slash = strrchr(path, '/');
    const char *last_backslash = strrchr(path, '\\');
    const char *name = path;
    if (last_slash && last_slash >= name) name = last_slash + 1;
    if (last_backslash && last_backslash >= name) name = last_backslash + 1;
    return name;
}

static int join_path(char *dest, size_t dest_size, const char *dir, const char *name) {
    int written;

    written = snprintf(dest, dest_size, "%s/%s", dir, name);
    return written > 0 && (size_t)written < dest_size;
}

static int create_temp_workspace(char *path, size_t path_size) {
    long pid;
    int attempt;

    pid = (long)CREXX_GETPID();
    for (attempt = 0; attempt < 1000; attempt++) {
        if (snprintf(path, path_size, ".crexx_test_driver_%ld_%d", pid, attempt) <= 0) {
            return 0;
        }
        if (CREXX_MKDIR(path) == 0) {
            return 1;
        }
        if (errno != EEXIST) {
            return 0;
        }
    }
    return 0;
}

#ifdef _WIN32
static int replace_fd_with_file(int target_fd, const char *path, int *saved_fd) {
    int file_fd;

    *saved_fd = CREXX_DUP(target_fd);
    if (*saved_fd < 0) return 0;

    file_fd = open_redirect_file(path);
    if (file_fd < 0) {
        CREXX_CLOSE(*saved_fd);
        *saved_fd = -1;
        return 0;
    }

    if (CREXX_DUP2(file_fd, target_fd) < 0) {
        CREXX_CLOSE(file_fd);
        CREXX_CLOSE(*saved_fd);
        *saved_fd = -1;
        return 0;
    }

    CREXX_CLOSE(file_fd);
    return 1;
}

static int restore_fd(int target_fd, int saved_fd) {
    int ok;

    if (saved_fd < 0) return 1;
    ok = CREXX_DUP2(saved_fd, target_fd) == 0;
    CREXX_CLOSE(saved_fd);
    return ok;
}

static int run_compiler_process(char **args,
                                const char *stdout_file,
                                const char *stderr_file,
                                int merge_stderr) {
    int saved_stdout = -1;
    int saved_stderr = -1;
    int ret;
    int ok = 1;

    if (stdout_file && !replace_fd_with_file(1, stdout_file, &saved_stdout)) {
        perror("stdout redirection failed");
        return 127;
    }

    if (merge_stderr) {
        saved_stderr = CREXX_DUP(2);
        if (saved_stderr < 0 || CREXX_DUP2(1, 2) < 0) {
            perror("stderr redirection failed");
            ok = 0;
        }
    } else if (stderr_file && !replace_fd_with_file(2, stderr_file, &saved_stderr)) {
        perror("stderr redirection failed");
        ok = 0;
    }

    if (ok) {
        ret = _spawnvp(_P_WAIT, args[0], (const char * const *)args);
        if (ret < 0) {
            perror("compiler launch failed");
            ret = 127;
        }
    } else {
        ret = 127;
    }

    restore_fd(2, saved_stderr);
    restore_fd(1, saved_stdout);
    return ret;
}
#else
static int redirect_child_fd(int target_fd, const char *path) {
    int file_fd;

    file_fd = open_redirect_file(path);
    if (file_fd < 0) return 0;
    if (CREXX_DUP2(file_fd, target_fd) < 0) {
        CREXX_CLOSE(file_fd);
        return 0;
    }
    CREXX_CLOSE(file_fd);
    return 1;
}

static int run_compiler_process(char **args,
                                const char *stdout_file,
                                const char *stderr_file,
                                int merge_stderr) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 127;
    }

    if (pid == 0) {
        if (stdout_file && !redirect_child_fd(1, stdout_file)) {
            perror("stdout redirection failed");
            _exit(127);
        }
        if (merge_stderr) {
            if (CREXX_DUP2(1, 2) < 0) {
                perror("stderr redirection failed");
                _exit(127);
            }
        } else if (stderr_file && !redirect_child_fd(2, stderr_file)) {
            perror("stderr redirection failed");
            _exit(127);
        }

        execvp(args[0], args);
        perror("compiler launch failed");
        _exit(127);
    }

    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid failed");
        return 127;
    }
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
    return 127;
}
#endif

static void cleanup_workspace(const char *original_work_dir,
                              const char *temp_work_dir,
                              const char *temp_source_file,
                              const char *temp_rxas,
                              char **copied_files,
                              int num_copied,
                              const char *stderr_file,
                              int normal_output_mode) {
    int i;

    if (temp_work_dir && temp_work_dir[0]) {
        if (CREXX_CHDIR(temp_work_dir) != 0) {
            /* Best-effort cleanup; continue with absolute cleanup paths. */
        }
    }
    if (temp_source_file && temp_source_file[0]) remove(temp_source_file);
    if (temp_rxas && temp_rxas[0]) remove(temp_rxas);
    for (i = 0; i < num_copied; i++) {
        if (copied_files[i]) remove(copied_files[i]);
    }
    if (original_work_dir && original_work_dir[0]) {
        if (CREXX_CHDIR(original_work_dir) != 0) {
            /* Best-effort cleanup. */
        }
    }
    if (temp_work_dir && temp_work_dir[0]) {
        CREXX_RMDIR(temp_work_dir);
    }
    if (normal_output_mode && stderr_file && stderr_file[0]) {
        remove(stderr_file);
    }
}

/* Function to strip leading/trailing whitespace and newlines */
static void normalize_line(char *str) {
    char *start = str;
    size_t len;

    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }

    len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }
}

/* Function to check if a line is volatile metadata */
static int is_volatile(const char *line) {
    if (line[0] == ';' || line[0] == '*' || (line[0] == '/' && line[1] == '*')) {
        if (strstr(line, "Version") || strstr(line, "VERSION") || strstr(line, "Time") ||
            strstr(line, "Date") || strstr(line, "Timestamp") ||
            strstr(line, "BUILT") || strstr(line, "SOURCE")) {
            return 1;
        }
    }
    if (strcmp(line, "/*") == 0 || strcmp(line, " */") == 0) {
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    char *golden_file;
    char *output_file;
    char *original_source;
    const char *base_name;
    char temp_source_file[2048];
    char temp_source_name[2048];
    char temp_rxas[2100];
    char compiler_output_base[2048];
    char stderr_file[2100];
    char original_work_dir[2048];
    char temp_work_dir[4096];
    char temp_path[4096];
    int i;
    int ret;
    int normal_output_mode = 0;
    int output_replaced = 0;
    FILE *f_out;
    FILE *f_gold;
    char line_out[4096];
    char line_gold[4096];
    int line_num_out = 0;
    int line_num_gold = 0;
    int expect_fail = 0;
    int ast_mode = 0;
    int warnings_mode = 0;
    int update_gold = 0;
    int arg_ptr = 1;
    char *copied_files[100];
    int num_copied = 0;
    char **compiler_args = 0;
    int compiler_arg_count = 0;
    const char *stdout_redirect = 0;
    const char *stderr_redirect = 0;
    int merge_stderr = 0;

    temp_source_file[0] = '\0';
    temp_rxas[0] = '\0';
    stderr_file[0] = '\0';
    original_work_dir[0] = '\0';
    temp_work_dir[0] = '\0';

    if (!CREXX_GETCWD(original_work_dir, sizeof(original_work_dir))) {
        fprintf(stderr, "Could not determine current working directory\n");
        return 1;
    }
    if (!create_temp_workspace(temp_work_dir, sizeof(temp_work_dir))) {
        fprintf(stderr, "Could not create temporary test workspace\n");
        return 1;
    }
    if (!join_path(temp_path, sizeof(temp_path), original_work_dir, temp_work_dir)) {
        fprintf(stderr, "Temporary test workspace path is too long\n");
        CREXX_RMDIR(temp_work_dir);
        return 1;
    }
    if (snprintf(temp_work_dir, sizeof(temp_work_dir), "%s", temp_path) <= 0 ||
        strlen(temp_path) >= sizeof(temp_work_dir)) {
        fprintf(stderr, "Temporary test workspace path is too long\n");
        CREXX_RMDIR(temp_path);
        return 1;
    }

    while (arg_ptr < argc && argv[arg_ptr][0] == '-') {
        if (strcmp(argv[arg_ptr], "--expect-fail") == 0) {
            expect_fail = 1;
            arg_ptr++;
        } else if (strcmp(argv[arg_ptr], "--ast") == 0) {
            ast_mode = 1;
            arg_ptr++;
        } else if (strcmp(argv[arg_ptr], "--warnings") == 0) {
            warnings_mode = 1;
            arg_ptr++;
        } else if (strcmp(argv[arg_ptr], "--update-gold") == 0) {
            update_gold = 1;
            arg_ptr++;
        } else if (strcmp(argv[arg_ptr], "--copy") == 0) {
            arg_ptr++;
            if (arg_ptr < argc) {
                const char *src = argv[arg_ptr];
                const char *base = get_base_name(src);
                if (!join_path(temp_path, sizeof(temp_path), temp_work_dir, base)) {
                    fprintf(stderr, "Temporary dependency path is too long for %s\n", src);
                    cleanup_workspace(original_work_dir, temp_work_dir, temp_source_file, temp_rxas,
                                      copied_files, num_copied, stderr_file, normal_output_mode);
                    return 1;
                }
                if (copy_file(src, temp_path)) {
                    copied_files[num_copied++] = (char*)base;
                } else {
                    fprintf(stderr, "Failed to copy dependency %s\n", src);
                    cleanup_workspace(original_work_dir, temp_work_dir, temp_source_file, temp_rxas,
                                      copied_files, num_copied, stderr_file, normal_output_mode);
                    return 1;
                }
                arg_ptr++;
            }
        } else {
            break;
        }
    }

    if (argc - arg_ptr < 3) {
        fprintf(stderr, "Usage: %s [--expect-fail] [--ast] [--update-gold] [--copy dep] <golden_file> <output_file> <compiler_cmd_part1> ...\n", argv[0]);
        cleanup_workspace(original_work_dir, temp_work_dir, temp_source_file, temp_rxas,
                          copied_files, num_copied, stderr_file, normal_output_mode);
        return 1;
    }

    golden_file = argv[arg_ptr++];
    output_file = argv[arg_ptr++];
    normal_output_mode = !expect_fail && !ast_mode && !warnings_mode;
    /* The compiler command starts at arg_ptr and the last one is the source file */
    original_source = argv[argc - 1];
    base_name = get_base_name(original_source);

    /* Setup temp filenames based on original base name to preserve .meta info */
    if (snprintf(temp_source_file, sizeof(temp_source_file), "%s", base_name) <= 0 ||
        strlen(base_name) >= sizeof(temp_source_file) ||
        snprintf(temp_source_name, sizeof(temp_source_name), "%s", base_name) <= 0 ||
        strlen(base_name) >= sizeof(temp_source_name)) {
        fprintf(stderr, "Temporary source name is too long for %s\n", base_name);
        cleanup_workspace(original_work_dir, temp_work_dir, temp_source_file, temp_rxas,
                          copied_files, num_copied, stderr_file, normal_output_mode);
        return 1;
    }
    {
        char *dot = strrchr(temp_source_name, '.');
        if (dot) *dot = '\0';
    }
    if (snprintf(temp_rxas, sizeof(temp_rxas), "%s.rxas", temp_source_name) <= 0 ||
        strlen(temp_source_name) + 5 >= sizeof(temp_rxas) ||
        snprintf(compiler_output_base, sizeof(compiler_output_base), "%s", output_file) <= 0 ||
        strlen(output_file) >= sizeof(compiler_output_base)) {
        fprintf(stderr, "Compiler output path is too long\n");
        cleanup_workspace(original_work_dir, temp_work_dir, temp_source_file, temp_rxas,
                          copied_files, num_copied, stderr_file, normal_output_mode);
        return 1;
    }
    {
        char *dot = strrchr(compiler_output_base, '.');
        if (dot && strcmp(dot, ".rxas") == 0) *dot = '\0';
    }
    if (snprintf(stderr_file, sizeof(stderr_file), "%s.err", output_file) <= 0 ||
        strlen(output_file) + 4 >= sizeof(stderr_file)) {
        fprintf(stderr, "Compiler stderr path is too long\n");
        cleanup_workspace(original_work_dir, temp_work_dir, temp_source_file, temp_rxas,
                          copied_files, num_copied, stderr_file, normal_output_mode);
        return 1;
    }

    /* Sandbox: copy source and dependencies into a per-test working directory. */
    if (!join_path(temp_path, sizeof(temp_path), temp_work_dir, temp_source_file)) {
        fprintf(stderr, "Temporary source path is too long for %s\n", original_source);
        cleanup_workspace(original_work_dir, temp_work_dir, temp_source_file, temp_rxas,
                          copied_files, num_copied, stderr_file, normal_output_mode);
        return 1;
    }
    if (!copy_file(original_source, temp_path)) {
        fprintf(stderr, "Could not copy source file %s to %s\n", original_source, temp_source_file);
        cleanup_workspace(original_work_dir, temp_work_dir, temp_source_file, temp_rxas,
                          copied_files, num_copied, stderr_file, normal_output_mode);
        return 1;
    }
    if (CREXX_CHDIR(temp_work_dir) != 0) {
        fprintf(stderr, "Could not enter temporary test workspace %s\n", temp_work_dir);
        cleanup_workspace(original_work_dir, temp_work_dir, temp_source_file, temp_rxas,
                          copied_files, num_copied, stderr_file, normal_output_mode);
        return 1;
    }

    /* Reconstruct argv: replace original source with temp_source_name and normalize -o. */
    compiler_args = (char **)calloc((size_t)(argc - arg_ptr + 3), sizeof(char *));
    if (!compiler_args) {
        fprintf(stderr, "Could not allocate compiler argument vector\n");
        cleanup_workspace(original_work_dir, temp_work_dir, temp_source_file, temp_rxas,
                          copied_files, num_copied, stderr_file, normal_output_mode);
        return 1;
    }
    for (i = arg_ptr; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (normal_output_mode) {
                compiler_args[compiler_arg_count++] = "-o";
                compiler_args[compiler_arg_count++] = compiler_output_base;
                output_replaced = 1;
            }
            i++; /* Skip -o and its value */
            continue;
        }
        if (i == argc - 1) {
            /* Pass the name without extension so rxc adds .crexx and .rxas correctly */
            compiler_args[compiler_arg_count++] = temp_source_name;
        } else {
            compiler_args[compiler_arg_count++] = argv[i];
        }
    }
    if (normal_output_mode && !output_replaced) {
        compiler_args[compiler_arg_count++] = "-o";
        compiler_args[compiler_arg_count++] = compiler_output_base;
    }
    compiler_args[compiler_arg_count] = 0;

    /* Redirect output */
    if (ast_mode) {
        stdout_redirect = output_file;
        merge_stderr = 1;
    } else if (normal_output_mode) {
        stderr_redirect = stderr_file;
    } else {
        stderr_redirect = output_file;
    }

    if (normal_output_mode) {
        remove(output_file);
        remove(stderr_file);
        remove(temp_rxas);
    }

    ret = run_compiler_process(compiler_args, stdout_redirect, stderr_redirect, merge_stderr);
    free(compiler_args);
    compiler_args = 0;

    if (!ast_mode) {
        if (expect_fail) {
            if (ret == 0) {
                fprintf(stderr, "Test failed: Expected compiler to fail, but it succeeded.\n");
                cleanup_workspace(original_work_dir, temp_work_dir, temp_source_file, temp_rxas,
                                  copied_files, num_copied, stderr_file, normal_output_mode);
                return 1;
            }
        } else {
            if (ret != 0) {
                fprintf(stderr, "Compiler failed with exit code %d. See %s for details.\n", ret,
                        normal_output_mode ? stderr_file : output_file);
                cleanup_workspace(original_work_dir, temp_work_dir, temp_source_file, temp_rxas,
                                  copied_files, num_copied, stderr_file, normal_output_mode);
                return 1;
            }
        }
    }

    /* Update Gold Logic */
    if (update_gold) {
        const char *src_for_gold = output_file;
        if (copy_file(src_for_gold, golden_file)) {
            printf("Updated golden file: %s\n", golden_file);
            cleanup_workspace(original_work_dir, temp_work_dir, temp_source_file, temp_rxas,
                              copied_files, num_copied, stderr_file, normal_output_mode);
            return 0;
        } else {
            fprintf(stderr, "Failed to update golden file from %s\n", src_for_gold);
            cleanup_workspace(original_work_dir, temp_work_dir, temp_source_file, temp_rxas,
                              copied_files, num_copied, stderr_file, normal_output_mode);
            return 1;
        }
    }

    /* Comparison Logic */
    if (expect_fail || ast_mode || warnings_mode) {
        f_out = fopen(output_file, "r");
    } else {
        f_out = fopen(output_file, "r");
    }

    if (!f_out) {
        fprintf(stderr, "Could not open result file for comparison (Gen: %s, Err: %s)\n",
                normal_output_mode ? output_file : temp_rxas,
                normal_output_mode ? stderr_file : output_file);
        cleanup_workspace(original_work_dir, temp_work_dir, temp_source_file, temp_rxas,
                          copied_files, num_copied, stderr_file, normal_output_mode);
        return 1;
    }

    f_gold = fopen(golden_file, "r");
    if (!f_gold) {
        fprintf(stderr, "Could not open golden file: %s\n", golden_file);
        fclose(f_out);
        cleanup_workspace(original_work_dir, temp_work_dir, temp_source_file, temp_rxas,
                          copied_files, num_copied, stderr_file, normal_output_mode);
        return 1;
    }

    while (1) {
        char *r_out;
        char *r_gold;

        do {
            r_out = fgets(line_out, sizeof(line_out), f_out);
            if (r_out) {
                normalize_line(line_out);
                if (is_volatile(line_out) || line_out[0] == '\0') r_out = NULL;
                else line_num_out++;
            }
        } while (r_out == NULL && !feof(f_out));

        do {
            r_gold = fgets(line_gold, sizeof(line_gold), f_gold);
            if (r_gold) {
                normalize_line(line_gold);
                if (is_volatile(line_gold) || line_gold[0] == '\0') r_gold = NULL;
                else line_num_gold++;
            }
        } while (r_gold == NULL && !feof(f_gold));

        if (r_out == NULL || r_gold == NULL) {
            if (r_out != r_gold) {
                fprintf(stderr, "File length mismatch. Out: %d, Gold: %d\n", line_num_out, line_num_gold);
                fclose(f_out); fclose(f_gold);
                cleanup_workspace(original_work_dir, temp_work_dir, temp_source_file, temp_rxas,
                                  copied_files, num_copied, stderr_file, normal_output_mode);
                return 1;
            }
            break;
        }

        if (strcmp(line_out, line_gold) != 0) {
            fprintf(stderr, "Mismatch at lines %d/%d:\nGen: [%s]\nGold: [%s]\n", 
                    line_num_out, line_num_gold, line_out, line_gold);
            fclose(f_out); fclose(f_gold);
            cleanup_workspace(original_work_dir, temp_work_dir, temp_source_file, temp_rxas,
                              copied_files, num_copied, stderr_file, normal_output_mode);
            return 1;
        }
    }

    fclose(f_out);
    fclose(f_gold);
    cleanup_workspace(original_work_dir, temp_work_dir, temp_source_file, temp_rxas,
                      copied_files, num_copied, stderr_file, normal_output_mode);
    printf("Test passed: matches %s\n", golden_file);
    return 0;
}
