/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// REXX Assembler
// Main Program
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "rxas.h"
#include "rxasassm.h"
#include "../binutils/include/rxdefs.h"

extern const OpInfo op_table[];

static char* operand_name_str(OperandType type) {
    switch(type) {
        case OP_ID: return "ID";
        case OP_REG: return "REG";
        case OP_FUNC: return "FUNC";
        case OP_INT: return "INT";
        case OP_FLOAT: return "FLOAT";
        case OP_CHAR: return "CHAR";
        case OP_STRING: return "STRING";
        case OP_DECIMAL: return "DECIMAL";
        case OP_BINARY: return "BINARY";
        case OP_NONE:
        default:
            return "NONE";
    }
}

static void prt_ops_new() {
    int i;
    printf("\n* REXX Assembly Instruction List\n");
    for (i = 0; op_table[i].mnemonic != NULL; i++) {
        size_t num_ops;
        if (!rxop_is_source_mnemonic(op_table[i].mnemonic)) continue;
        num_ops = rxop_format_operand_count(op_table[i].format);

        {
            char mnemonic[100];
            int j = 0;
            size_t operand_index;
            size_t format_width = num_ops ? 2 + (num_ops - 1) : strlen("no operand");
            while (op_table[i].mnemonic[j] && op_table[i].mnemonic[j] != '_') {
                mnemonic[j] = (char)tolower((unsigned char)op_table[i].mnemonic[j]);
                j++;
            }
            mnemonic[j] = 0;
            printf("0x%.4x %-10s ", op_table[i].opcode, mnemonic);
            if (!num_ops) {
                printf("no operand");
            } else {
                putchar('{');
                for (operand_index = 0; operand_index < num_ops; operand_index++) {
                    const char *name = operand_name_str(
                            rxop_format_operand_type(op_table[i].format, operand_index));
                    if (operand_index) putchar(',');
                    fputs(name, stdout);
                    format_width += strlen(name);
                }
                putchar('}');
            }
            while (format_width++ < 20) putchar(' ');
            printf(" %s\n", op_table[i].description);
        }
    }
    printf("\n");
}

static void help() {
    char* helpMessage =
        "cREXX Assembler\n"
        "Version : " rxversion "\n"
        "Usage   : rxas [options] source_file\n"
        "Options :\n"
        "  -h              Help Message\n"
        "  -c              Copyright & License Details\n"
        "  -v              Version\n"
        "  -a              Architecture Details\n"
        "  -i              Print Instructions\n"
        "  -d              Debug/Verbose Mode\n"
        "  -l location     Working Location (directory)\n"
        "  -o output_stem  RXBIN output stem or .rxbin file\n"
        "  -n              No Optimising\n"
        "Notes   :\n"
        "- source_file : The source file to be assembled; filetype (rxas) is added to the name.\n";

    printf("%s",helpMessage);
}

static void license() {
    char *message = CREXX_LICENSE_TEXT;

    printf("%s",message);
}

static void error_and_exit(int rc, char* message) {

    fprintf(stderr, "ERROR: %s - try \"rxas -h\"\n", message);
    exit(rc);
}

#ifdef ENABLE_PARSER_MODE
int rxas_parser_mode_main(int stdio_mode, int port, const char *file_name, int debug_mode);
#endif

int main(int argc, char *argv[]) {
    Assembler_Context scanner;
    char *combined_location = 0;
    char *exe_path = 0;
    FILE *trace_file = 0;
    int assemble_rc;
    int i;
#ifdef ENABLE_PARSER_MODE
    int parser_mode = 0;
    int stdio_mode = 1;
    int port = 0;
#endif

    platform_install_signal_handlers();

    /* Load Instruction Database */
    /* init_ops(); - NO LONGER NEEDED */

    memset(&scanner, 0, sizeof(scanner));

    /* scanner context parameter */
    scanner.debug_mode = 0;
    scanner.traceFile = 0;
    scanner.optimise = 1;
    scanner.file_name = 0;
    scanner.output_file_name = 0;
    scanner.location = 0;
    scanner.quiet = 0;

    /* Parse arguments  */
    for (i = 1; i < argc && argv[i][0] == '-'; i++) {
#ifdef ENABLE_PARSER_MODE
        if (strcmp(argv[i], "--syntaxhighlight") == 0) {
            parser_mode = 1;
            continue;
        } else if (strcmp(argv[i], "--stdio") == 0) {
            stdio_mode = 1;
            continue;
        } else if (strcmp(argv[i], "--port") == 0) {
            stdio_mode = 0;
            i++;
            if (i >= argc) {
                error_and_exit(2, "Missing port after --port");
            }
            port = atoi(argv[i]);
            continue;
        }
#endif
        if (strlen(argv[i]) > 2 && argv[i][1] != '-') {
            error_and_exit(2, "Invalid argument");
        }
        switch (toupper((argv[i][1]))) {
            case '-':
                continue;

            case 'O': /* Output File */
                i++;
                if (i >= argc) {
                    error_and_exit(2, "Missing output file after -o");
                }
                scanner.output_file_name = argv[i];
                break;

            case 'L': /* Working Location / Directory */
                i++;
                if (i >= argc) {
                    error_and_exit(2, "Missing location after -l");
                }
                scanner.location = argv[i];
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

            case 'A': /* Architecture */
                printf("OS Architecture Details\n");
                printf("Type sizes: int=%d, char=%d, void*=%d, double=%d, long double=%d, long=%d, rxinteger(aka long long)=%d size_t=%d\n",
                       (int) sizeof(int),  (int) sizeof(char),        (int) sizeof(void*),     (int) sizeof(double),
                       (int) sizeof(long double), (int) sizeof(long), (int) sizeof(rxinteger), (int) sizeof(size_t));
                exit(0);

            case 'I': /* Instructions */
                prt_ops_new();
                exit(0);

            case 'N': /* No optimisation */
                scanner.optimise = 0;
                break;

            case 'D': /* Debug Mode */
                scanner.debug_mode = 1;
                break;

            default:
                error_and_exit(2, "Invalid argument");
        }
    }

#ifdef ENABLE_PARSER_MODE
    if (parser_mode) {
        const char *file_name = NULL;
        if (i < argc) {
            file_name = argv[i];
        }
        exit(rxas_parser_mode_main(stdio_mode, port, file_name, scanner.debug_mode));
    }
#endif

    if (i == argc) {
        error_and_exit(2, "Missing input source file");
    }

    /* Add current and executable path to location */
    exe_path = exepath();
    if (scanner.location) {
        size_t combined_location_size = strlen(scanner.location) + strlen(exe_path) + 5;
        combined_location = malloc(combined_location_size);
        if (!combined_location) {
            RX_PANIC_OOM("malloc rxas location list", combined_location_size,
                         scanner.location);
        }
        sprintf(combined_location, ".;%s;%s", scanner.location, exe_path);
        scanner.location = combined_location;
    } else {
        size_t combined_location_size = strlen(exe_path) + 5;
        combined_location = malloc(combined_location_size);
        if (!combined_location) {
            RX_PANIC_OOM("malloc rxas location list", combined_location_size, exe_path);
        }
        sprintf(combined_location, ".;%s", exe_path);
        scanner.location = combined_location;
    }
    free(exe_path);

    scanner.file_name = argv[i++];

    if (i < argc) {
        error_and_exit(2, "Unexpected Arguments");
    }

    /* Open trace file */
    if (scanner.debug_mode) {
        scanner.traceFile = openfile("trace", "out", scanner.location, "w");
        if (scanner.traceFile == NULL) {
            fprintf(stderr, "Can't open trace file\n");
            free(combined_location);
            return -1;
        }
        trace_file = scanner.traceFile;
    }

    /* Assemble */
    assemble_rc = rxasmble(&scanner);

    /* Free Instruction Database */
    /* init_ops() / free_ops() - NO LONGER NEEDED */

    if (trace_file) fclose(trace_file);
    free(combined_location);

    if (assemble_rc) return -1;
    return 0;
}
