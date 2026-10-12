/* Create exact non-finite pool fixtures through the maintained RXBIN reader
 * and writer, avoiding a second implementation of the portable container. */
#include "rxbin.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    FILE *file;
    module_file *module = NULL;
    uint64_t bits = 0;
    size_t i;
    int found = 0;
    int result;
    if (argc != 4 || strlen(argv[3]) != 16) return 2;
    for (i = 0; i < 16; ++i) {
        unsigned int digit;
        char c = argv[3][i];
        if (c >= '0' && c <= '9') digit = (unsigned int)(c - '0');
        else if (c >= 'a' && c <= 'f') digit = (unsigned int)(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') digit = (unsigned int)(c - 'A' + 10);
        else return 2;
        bits = (bits << 4u) | digit;
    }
    file = fopen(argv[1], "rb");
    if (!file) return 2;
    result = read_module(&module, file);
    fclose(file);
    if (result != 0 || !module) return 2;
    for (i = 0; i < module->header.constant_size;) {
        chameleon_constant *entry = (chameleon_constant *)(module->constant + i);
        if (!entry->size_in_pool || entry->size_in_pool > module->header.constant_size - i) break;
        if (entry->type == FLOAT_CONST && isinf(((float_constant *)entry)->double_value)) {
            memcpy(&((float_constant *)entry)->double_value, &bits, sizeof(bits));
            found = 1;
        }
        i += entry->size_in_pool;
    }
    if (!found) {
        free_module(module);
        return 2;
    }
    file = fopen(argv[2], "wb");
    if (!file) {
        free_module(module);
        return 2;
    }
    result = write_module(module, file);
    if (fclose(file)) result = -1;
    free_module(module);
    return result ? 2 : 0;
}
