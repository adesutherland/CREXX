#include <stdio.h>
#include <stdlib.h>

static int append_file(FILE *out, const char *path) {
    FILE *in;
    unsigned char buffer[4096];
    size_t read_count;

    in = fopen(path, "rb");
    if (!in) {
        fprintf(stderr, "Failed to open %s\n", path);
        return 0;
    }

    while ((read_count = fread(buffer, 1, sizeof(buffer), in)) != 0) {
        if (fwrite(buffer, 1, read_count, out) != read_count) {
            fclose(in);
            fprintf(stderr, "Failed to append %s\n", path);
            return 0;
        }
    }

    fclose(in);
    return 1;
}

int main(int argc, char *argv[]) {
    FILE *out;
    int i;

    if (argc < 3) {
        fprintf(stderr, "Usage: concat_files output input [input...]\n");
        return 1;
    }

    out = fopen(argv[1], "wb");
    if (!out) {
        fprintf(stderr, "Failed to open %s for writing\n", argv[1]);
        return 1;
    }

    for (i = 2; i < argc; i++) {
        if (!append_file(out, argv[i])) {
            fclose(out);
            return 1;
        }
    }

    fclose(out);
    return 0;
}
