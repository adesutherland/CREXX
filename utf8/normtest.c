/* Normaliser PoC */
#include <assert.h>
#include <stdio.h>
#include <string.h>

int normaliser(const char *str, int len, char* out, int *out_len);

int main() {
    char out[100];
    int outlen;
    int errors = 0;
    char *input;
    int err;

    input = "He\xCC\x81llo 쳁 World!";
    input = "Héllo 쳁 World!";
    err = normaliser(input, strlen(input), out, &outlen);
    if (err) {
        printf("Error: %d\n", err);
        errors++;
    }
    else {
        /* Print input and out strings - these are not null terminated */
        printf("Input:  [%s] (%d bytes)\n", input, strlen(input));
        printf("Output: [%.*s] (%d bytes)\n", outlen, out, outlen);
    }



    return errors?1:0;
}
