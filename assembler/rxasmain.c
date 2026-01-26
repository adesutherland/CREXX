// REXX Assembler
// Main Program
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "rxvminst.h"
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
        char buffer[100];
        OperandType types[3];
        int num_ops = 0;
        switch (op_table[i].format) {
            case FMT_EMPTY: num_ops = 0; break;
            case FMT_C: types[0] = OP_CHAR; num_ops = 1; break;
            case FMT_F: types[0] = OP_FLOAT; num_ops = 1; break;
            case FMT_I: types[0] = OP_INT; num_ops = 1; break;
            case FMT_I_I: types[0] = OP_INT; types[1] = OP_INT; num_ops = 2; break;
            case FMT_I_I_I: types[0] = OP_INT; types[1] = OP_INT; types[2] = OP_INT; num_ops = 3; break;
            case FMT_I_I_R: types[0] = OP_INT; types[1] = OP_INT; types[2] = OP_REG; num_ops = 3; break;
            case FMT_I_R: types[0] = OP_INT; types[1] = OP_REG; num_ops = 2; break;
            case FMT_I_R_R: types[0] = OP_INT; types[1] = OP_REG; types[2] = OP_REG; num_ops = 3; break;
            case FMT_L: types[0] = OP_ID; num_ops = 1; break;
            case FMT_L_L_R: types[0] = OP_ID; types[1] = OP_ID; types[2] = OP_REG; num_ops = 3; break;
            case FMT_L_P_S: types[0] = OP_ID; types[1] = OP_FUNC; types[2] = OP_STRING; num_ops = 3; break;
            case FMT_L_R: types[0] = OP_ID; types[1] = OP_REG; num_ops = 2; break;
            case FMT_L_R_I: types[0] = OP_ID; types[1] = OP_REG; types[2] = OP_INT; num_ops = 3; break;
            case FMT_L_R_R: types[0] = OP_ID; types[1] = OP_REG; types[2] = OP_REG; num_ops = 3; break;
            case FMT_L_S: types[0] = OP_ID; types[1] = OP_STRING; num_ops = 2; break;
            case FMT_P: types[0] = OP_FUNC; num_ops = 1; break;
            case FMT_P_S: types[0] = OP_FUNC; types[1] = OP_STRING; num_ops = 2; break;
            case FMT_R: types[0] = OP_REG; num_ops = 1; break;
            case FMT_R_C: types[0] = OP_REG; types[1] = OP_CHAR; num_ops = 2; break;
            case FMT_R_D: types[0] = OP_REG; types[1] = OP_DECIMAL; num_ops = 2; break;
            case FMT_R_D_R: types[0] = OP_REG; types[1] = OP_DECIMAL; types[2] = OP_REG; num_ops = 3; break;
            case FMT_R_F: types[0] = OP_REG; types[1] = OP_FLOAT; num_ops = 2; break;
            case FMT_R_F_I: types[0] = OP_REG; types[1] = OP_FLOAT; types[2] = OP_INT; num_ops = 3; break;
            case FMT_R_F_R: types[0] = OP_REG; types[1] = OP_FLOAT; types[2] = OP_REG; num_ops = 3; break;
            case FMT_R_I: types[0] = OP_REG; types[1] = OP_INT; num_ops = 2; break;
            case FMT_R_I_I: types[0] = OP_REG; types[1] = OP_INT; types[2] = OP_INT; num_ops = 3; break;
            case FMT_R_I_R: types[0] = OP_REG; types[1] = OP_INT; types[2] = OP_REG; num_ops = 3; break;
            case FMT_R_P: types[0] = OP_REG; types[1] = OP_FUNC; num_ops = 2; break;
            case FMT_R_P_R: types[0] = OP_REG; types[1] = OP_FUNC; types[2] = OP_REG; num_ops = 3; break;
            case FMT_R_R: types[0] = OP_REG; types[1] = OP_REG; num_ops = 2; break;
            case FMT_R_R_D: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_DECIMAL; num_ops = 3; break;
            case FMT_R_R_F: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_FLOAT; num_ops = 3; break;
            case FMT_R_R_I: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_INT; num_ops = 3; break;
            case FMT_R_R_R: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_REG; num_ops = 3; break;
            case FMT_R_R_S: types[0] = OP_REG; types[1] = OP_REG; types[2] = OP_STRING; num_ops = 3; break;
            case FMT_R_S: types[0] = OP_REG; types[1] = OP_STRING; num_ops = 2; break;
            case FMT_R_S_I: types[0] = OP_REG; types[1] = OP_STRING; types[2] = OP_INT; num_ops = 3; break;
            case FMT_R_S_R: types[0] = OP_REG; types[1] = OP_STRING; types[2] = OP_REG; num_ops = 3; break;
            case FMT_R_S_S: types[0] = OP_REG; types[1] = OP_STRING; types[2] = OP_STRING; num_ops = 3; break;
            case FMT_S: types[0] = OP_STRING; num_ops = 1; break;
            case FMT_S_R: types[0] = OP_STRING; types[1] = OP_REG; num_ops = 2; break;
            case FMT_S_S: types[0] = OP_STRING; types[1] = OP_STRING; num_ops = 2; break;
            case FMT_S_S_R: types[0] = OP_STRING; types[1] = OP_STRING; types[2] = OP_REG; num_ops = 3; break;
        }
        switch (num_ops) {
            case 0: snprintf(buffer, 100, "no operand"); break;
            case 1: snprintf(buffer, 100, "{%s}", operand_name_str(types[0])); break;
            case 2: snprintf(buffer, 100, "{%s,%s}", operand_name_str(types[0]), operand_name_str(types[1])); break;
            case 3: snprintf(buffer, 100, "{%s,%s,%s}", operand_name_str(types[0]), operand_name_str(types[1]), operand_name_str(types[2])); break;
        }

        {
            char mnemonic[100];
            int j = 0;
            while (op_table[i].mnemonic[j] && op_table[i].mnemonic[j] != '_') {
                mnemonic[j] = (char)tolower((unsigned char)op_table[i].mnemonic[j]);
                j++;
            }
            mnemonic[j] = 0;
            printf("0x%.4x %-10s %-20s %s\n",
                   op_table[i].opcode, mnemonic, buffer, op_table[i].description);
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
        "  -o output_file  Binary Output File\n"
        "  -n              No Optimising\n";

    printf("%s",helpMessage);
}

static void license() {
    char *message =
    "cREXX License (MIT)\n"
    "Copyright (c) 2020-2026 Adrian Sutherland\n\n"

    "Permission is hereby granted, free of charge, to any person obtaining a copy\n"
    "of this software and associated documentation files (the \"Software\"), to deal\n"
    "in the Software without restriction, including without limitation the rights\n"
    "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
    "copies of the Software, and to permit persons to whom the Software is\n"
    "furnished to do so, subject to the following conditions:\n\n"

    "The above copyright notice and this permission notice shall be included in all\n"
    "copies or substantial portions of the Software.\n\n"

    "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
    "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
    "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
    "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
    "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
    "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
    "SOFTWARE.\n\n"
    "See https://github.com/adesutherland/CREXX for project details\n";

    printf("%s",message);
}

static void error_and_exit(int rc, char* message) {

    fprintf(stderr, "ERROR: %s - try \"rxas -h\"\n", message);
    exit(rc);
}

int main(int argc, char *argv[]) {
    Assembler_Context scanner;
    int i;

    /* Load Instruction Database */
    /* init_ops(); - NO LONGER NEEDED */

    /* scanner context parameter */
    scanner.debug_mode = 0;
    scanner.traceFile = 0;
    scanner.optimise = 1;
    scanner.file_name = 0;
    scanner.output_file_name = 0;
    scanner.location = 0;

    /* Parse arguments  */
    for (i = 1; i < argc && argv[i][0] == '-'; i++) {
        if (strlen(argv[i]) > 2) {
            error_and_exit(2, "Invalid argument");
        }
        switch (toupper((argv[i][1]))) {
            case '-':
                break;

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

    if (i == argc) {
        error_and_exit(2, "Missing input source file");
    }

    scanner.file_name = argv[i++];

    if (i < argc) {
        error_and_exit(2, "Unexpected Arguments");
    }

    /* Open trace file */
    if (scanner.debug_mode) {
        scanner.traceFile = openfile("trace", "out", scanner.location, "w");
        if (scanner.traceFile == NULL) {
            fprintf(stderr, "Can't open trace file\n");
            exit(-1);
        }
    }

    /* Assemble */
    if (rxasmble(&scanner)) exit(-1);

    /* Free Instruction Database */
    /* init_ops() / free_ops() - NO LONGER NEEDED */

    if (scanner.traceFile) fclose(scanner.traceFile);

    return(scanner.severity);
}
