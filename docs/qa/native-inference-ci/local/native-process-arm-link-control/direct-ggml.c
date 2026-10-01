#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
extern void *ggml_backend_load(const char *);
extern size_t ggml_backend_reg_dev_count(void *);
extern void ggml_log_set(void (*)(int, const char *, void *), void *);
static void quiet(int level, const char *text, void *data) {}
int main(int argc, char **argv) {
    if (argc < 4) return 2;
    if (!strcmp(argv[1], "quiet")) ggml_log_set(quiet, NULL);
    int best = 0, best_score = 0;
    for (int i = 3; i < argc; ++i) {
        void *library = dlopen(argv[i], RTLD_NOW | RTLD_LOCAL);
        if (!library) { fprintf(stderr, "%s\n", dlerror()); return 3; }
        int (*score)(void) = (int (*)(void))dlsym(library, "ggml_backend_score");
        int value = score ? score() : 1;
        if (value > best_score) { best = i; best_score = value; }
        /* Match the permanent probe's extra retained first-CPU leases. */
        if (i == 3 && (!dlopen(argv[i], RTLD_NOW | RTLD_LOCAL) ||
                      !dlopen(argv[i], RTLD_NOW | RTLD_LOCAL))) return 4;
    }
    if (!best || !ggml_backend_load(argv[best])) return 5;
    puts("DIRECT_NATIVE_STAGE: Metal registration"); fflush(stdout);
    void *metal = ggml_backend_load(argv[2]);
    if (!metal || !ggml_backend_reg_dev_count(metal)) return 6;
    for (uint32_t i = 0; i < _dyld_image_count(); ++i) {
        const char *name = _dyld_get_image_name(i);
        printf("LOADED_IMAGE: %s\n", name);
        if (strstr(name, "libcrexx-llama") || strstr(name, "rxvm")) return 7;
    }
    puts("DIRECT_NATIVE_END: 0");
    return 0;
}
