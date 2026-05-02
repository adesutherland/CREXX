/*
 * rxcpack tool to convert rxbin files (or any binary file) into a c file
 * that can be linked to a C exe.
 *
 * This is based on the public domain bin2c by Serge Fukanchik.
 * See https://github.com/gwilymk/bin2c
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "crexx_version.h"
#include "platform.h"

#define NAME_BUFFER_SIZE 256
#define GLOBAL_SYMBOL "rx__pg"

static void help() {
    char* helpMessage =
                        "cREXX rxcpack. Tool to convert rxbin files (or any binary file) into a c file\n"
                        "              that can be linked to a C exe\n"
                        "Version : " rxversion "\n"
                        "Usage   : rxcpack [options] input_file_1 input_file_2 ... input_file_n\n"
                        "                           (.rxbin is appended to input file names)\n"
                        "Options :\n"
                        "  -h              Help Message\n"
                        "  -c              Copyright & License Details\n"
                        "  -v              Version\n"
                        "  -o output_file  Binary Output File (.c is appended - default is input_file_1.c)\n";

    printf("%s",helpMessage);
}

static void license() {
    char *message =
            CREXX_LICENSE_TEXT "\n"
            "NOTE - This tool is based on the public domain bin2c by Serge Fukanchik.\n"
            "       See https://github.com/gwilymk/bin2c\n"
            "       Thank you Serge :-)\n";

    printf("%s",message);
}

static void error_and_exit(char* message) {
    fprintf(stderr, "ERROR: %s - try \"rxcpack -h\"\n", message);
    exit(2);
}

int main(int argc, char *argv[])
{
    char *buf;
    size_t i, file_size, j = 0, a;
    char need_comma = 0;
    char n_input[NAME_BUFFER_SIZE], n_output[NAME_BUFFER_SIZE];
    FILE *f_input, *f_output;
    char *output_file_name = 0;

    platform_install_signal_handlers();

    /* Parse arguments  */
    for (i = 1; i < argc && argv[i][0] == '-'; i++) {
        if (strlen(argv[i]) > 2) {
            error_and_exit("Invalid argument");
        }
        switch (toupper((argv[i][1]))) {
            case '-':
                break;

            case 'O': /* Output File */
                i++;
                if (i >= argc) {
                    error_and_exit("Missing output file after -o");
                }
                output_file_name = argv[i];
                break;

            case 'V': /* Version */
                printf("%s\n", rxversion);
                exit(0);

            case 'H': /* Help */
            case '?':
                help();
                exit(0);

            case 'C': /* License */
                license();
                exit(0);

            default:
                error_and_exit("Invalid argument");
        }
    }

    if (i == argc) {
        error_and_exit("Missing input source file");
    }

    if (output_file_name) snprintf(n_output, NAME_BUFFER_SIZE, "%s.c", output_file_name);
    else snprintf(n_output, NAME_BUFFER_SIZE, "%s.c", argv[1]);
    f_output = fopen(n_output, "w");
    if (f_output == NULL) {
        fprintf(stderr, "%s: can't open %s for writing\n", argv[0], n_output);
        return 2;
    }
    fprintf(f_output, "/* Auto-generated rxbin file(s) converted to a c array - %s */\n\n#include <stddef.h>\n\nchar %s[] = {", n_output, GLOBAL_SYMBOL);

    for (a=i; a<argc; a++) {
        snprintf(n_input, NAME_BUFFER_SIZE, "%s.rxbin", argv[a]);
        f_input = fopen(n_input, "rb");
        if (f_input == NULL) {
            fprintf(stderr, "%s: can't open %s for reading\n", argv[0], n_input);
            return 2;
        }

        // Get the file length
        fseek(f_input, 0, SEEK_END);
        file_size = ftell(f_input);
        fseek(f_input, 0, SEEK_SET);

        buf = (char *) malloc(file_size);
        assert(buf);

        fread(buf, file_size, 1, f_input);
        fclose(f_input);

        for (i = 0; i < file_size; ++i) {
            if (need_comma) fprintf(f_output, ", ");
            else need_comma = 1;

            if ((j % 11) == 0) fprintf(f_output, "\n\t");
            fprintf(f_output, "0x%.2x", buf[i] & 0xff);

            j++;
        }
        free(buf);
    }

    fprintf(f_output, "\n};\n\n");
    fprintf(f_output, "size_t %s_l = %lu;\n", GLOBAL_SYMBOL, j);
    fclose(f_output);

    return 0;
}
