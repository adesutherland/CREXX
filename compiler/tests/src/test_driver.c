#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Function to copy a file - C90 compliant */
static int copy_file(const char *src_path, const char *dst_path) {
    FILE *src, *dst;
    char buffer[4096];
    size_t n;

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

    fclose(src);
    fclose(dst);
    return 1;
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
    if (strstr(line, ".srcfile")) {
        return 1;
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
    char command[8192];
    int i;
    int ret;
    FILE *f_out;
    FILE *f_gold;
    char line_out[4096];
    char line_gold[4096];
    int line_num_out = 0;
    int line_num_gold = 0;
    int expect_fail = 0;
    int ast_mode = 0;
    int update_gold = 0;
    int arg_ptr = 1;
    char *copied_files[100];
    int num_copied = 0;

    while (arg_ptr < argc && argv[arg_ptr][0] == '-') {
        if (strcmp(argv[arg_ptr], "--expect-fail") == 0) {
            expect_fail = 1;
            arg_ptr++;
        } else if (strcmp(argv[arg_ptr], "--ast") == 0) {
            ast_mode = 1;
            arg_ptr++;
        } else if (strcmp(argv[arg_ptr], "--update-gold") == 0) {
            update_gold = 1;
            arg_ptr++;
        } else if (strcmp(argv[arg_ptr], "--copy") == 0) {
            arg_ptr++;
            if (arg_ptr < argc) {
                const char *src = argv[arg_ptr];
                const char *base = get_base_name(src);
                if (copy_file(src, base)) {
                    copied_files[num_copied++] = (char*)base;
                } else {
                    fprintf(stderr, "Failed to copy dependency %s\n", src);
                    for (i = 0; i < num_copied; i++) remove(copied_files[i]);
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
        for (i = 0; i < num_copied; i++) remove(copied_files[i]);
        return 1;
    }

    golden_file = argv[arg_ptr++];
    output_file = argv[arg_ptr++];
    /* The compiler command starts at arg_ptr and the last one is the source file */
    original_source = argv[argc - 1];
    base_name = get_base_name(original_source);

    /* Setup temp filenames based on original base name to preserve .meta info */
    strcpy(temp_source_file, base_name);
    strcpy(temp_source_name, base_name);
    {
        char *dot = strrchr(temp_source_name, '.');
        if (dot) *dot = '\0';
    }
    snprintf(temp_rxas, sizeof(temp_rxas), "%s.rxas", temp_source_name);

    /* Sandbox: Copy original source (last arg) to current directory */
    if (!copy_file(original_source, temp_source_file)) {
        fprintf(stderr, "Could not copy source file %s to %s\n", original_source, temp_source_file);
        for (i = 0; i < num_copied; i++) remove(copied_files[i]);
        return 1;
    }

    /* Reconstruct command: Replace original source with temp_source_name (or file), and remove -o */
    command[0] = '\0';
    for (i = arg_ptr; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            i++; /* Skip -o and its value */
            continue;
        }
        if (command[0] != '\0') {
            strcat(command, " ");
        }
        strcat(command, "\"");
        if (i == argc - 1) {
            /* Pass the name without extension so rxc adds .rexx and .rxas correctly */
            strcat(command, temp_source_name);
        } else {
            strcat(command, argv[i]);
        }
        strcat(command, "\"");
    }

    /* Redirect output */
    if (ast_mode) {
        strcat(command, " > \"");
        strcat(command, output_file);
        strcat(command, "\" 2>&1");
    } else {
        strcat(command, " 2> \"");
        strcat(command, output_file);
        strcat(command, "\"");
    }

#ifdef _WIN32
    {
        char wrapped_command[8200];
        sprintf(wrapped_command, "\"%s\"", command);
        ret = system(wrapped_command);
    }
#else
    ret = system(command);
#endif

    if (!ast_mode) {
        if (expect_fail) {
            if (ret == 0) {
                fprintf(stderr, "Test failed: Expected compiler to fail, but it succeeded.\n");
                remove(temp_source_file);
                remove(temp_rxas);
                for (i = 0; i < num_copied; i++) remove(copied_files[i]);
                return 1;
            }
        } else {
            if (ret != 0) {
                fprintf(stderr, "Compiler failed with exit code %d. See %s for details.\n", ret, output_file);
                remove(temp_source_file);
                remove(temp_rxas);
                for (i = 0; i < num_copied; i++) remove(copied_files[i]);
                return 1;
            }
        }
    }

    /* Update Gold Logic */
    if (update_gold) {
        const char *src_for_gold = (expect_fail || ast_mode) ? output_file : temp_rxas;
        if (copy_file(src_for_gold, golden_file)) {
            printf("Updated golden file: %s\n", golden_file);
            remove(temp_source_file);
            remove(temp_rxas);
            for (i = 0; i < num_copied; i++) remove(copied_files[i]);
            return 0;
        } else {
            fprintf(stderr, "Failed to update golden file from %s\n", src_for_gold);
            remove(temp_source_file);
            remove(temp_rxas);
            for (i = 0; i < num_copied; i++) remove(copied_files[i]);
            return 1;
        }
    }

    /* Comparison Logic */
    if (expect_fail || ast_mode) {
        f_out = fopen(output_file, "r");
    } else {
        f_out = fopen(temp_rxas, "r");
    }

    if (!f_out) {
        fprintf(stderr, "Could not open result file for comparison (Gen: %s, Err: %s)\n", temp_rxas, output_file);
        remove(temp_source_file);
        remove(temp_rxas);
        for (i = 0; i < num_copied; i++) remove(copied_files[i]);
        return 1;
    }

    f_gold = fopen(golden_file, "r");
    if (!f_gold) {
        fprintf(stderr, "Could not open golden file: %s\n", golden_file);
        fclose(f_out);
        remove(temp_source_file);
        remove(temp_rxas);
        for (i = 0; i < num_copied; i++) remove(copied_files[i]);
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
                remove(temp_source_file); remove(temp_rxas);
                for (i = 0; i < num_copied; i++) remove(copied_files[i]);
                return 1;
            }
            break;
        }

        if (strcmp(line_out, line_gold) != 0) {
            fprintf(stderr, "Mismatch at lines %d/%d:\nGen: [%s]\nGold: [%s]\n", 
                    line_num_out, line_num_gold, line_out, line_gold);
            fclose(f_out); fclose(f_gold);
            remove(temp_source_file); remove(temp_rxas);
            for (i = 0; i < num_copied; i++) remove(copied_files[i]);
            return 1;
        }
    }

    fclose(f_out);
    fclose(f_gold);
    remove(temp_source_file);
    remove(temp_rxas);
    for (i = 0; i < num_copied; i++) remove(copied_files[i]);
    printf("Test passed: matches %s\n", golden_file);
    return 0;
}
