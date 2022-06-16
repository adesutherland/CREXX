/*
 * rxpack tool to convert rxbin files (or any binary file) into a c file
 * that can be linked to a C exe.
 *
 * This is based on the public domain bin2c by Serge Fukanchik.
 * See https://github.com/gwilymk/bin2c
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME_BUFFER_SIZE 128
#define GLOBAL_SYMBOL "__rxpg"

int main(int argc, char *argv[])
{
    char *buf;
    size_t i, file_size, j = 0;;
    char need_comma = 0;
    char n_input[NAME_BUFFER_SIZE], n_output[NAME_BUFFER_SIZE];
    FILE *f_input, *f_output;
    int a;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s {name 1} {name 2} ... {name n}\n", argv[0]);
        return -1;
    }

    snprintf(n_output, NAME_BUFFER_SIZE, "%s.c", argv[1]);
    f_output = fopen(n_output, "w");
    if (f_output == NULL) {
        fprintf(stderr, "%s: can't open %s for writing\n", argv[0], n_output);
        return -1;
    }
    fprintf(f_output, "/* Auto-generated rxbin file(s) converted to a c buffer - %s */\n\n#include <stddef.h>\n\nchar %s[] = {", n_output, GLOBAL_SYMBOL);

   for (a=1; a<argc; a++) {
       snprintf(n_input, NAME_BUFFER_SIZE, "%s.rxbin", argv[a]);
       f_input = fopen(n_input, "rb");
       if (f_input == NULL) {
           fprintf(stderr, "%s: can't open %s for reading\n", argv[0], n_input);
           return -1;
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
       buf = 0;
   }

    fprintf(f_output, "\n};\n\n");
    fprintf(f_output, "size_t %s_l = %lu;\n", GLOBAL_SYMBOL, j);
    fclose(f_output);

    return 0;
}