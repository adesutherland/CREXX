#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rxvml.h"

/* Minimal reproducibility harness for VM external call side-effect issue */

void my_say_exit(char* message) {
    printf("SAY: %s\n", message);
}

/* Helper to load file content */
char* load_file(const char* path, size_t* size_out) {
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        perror(path);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char* buffer = malloc(fsize);
    if (fread(buffer, 1, fsize, fp) != (size_t)fsize) {
        perror(path);
        free(buffer);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    if (size_out) *size_out = (size_t)fsize;
    return buffer;
}

int main(int argc, char** argv) {
    printf("Starting reproduction harness...\n");
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working dir: %s\n", cwd);
    }

    /* 1. Initialize VM Context */
    rxvml_set_say_exit(my_say_exit);
    rxvml_context* ctx = rxvml_create("repro_harness", 0);
    if (!ctx) {
        fprintf(stderr, "Failed to create VM context\n");
        return 1;
    }

    /* 1.5 Load library.rxbin first */
    /* Relative path from cmake-build-debug/compiler/exits/dump to lib/rxfnsb/library.rxbin */
    const char* lib_path = "../../../lib/rxfnsb/library.rxbin";
    printf("Loading library.rxbin from %s...\n", lib_path);
    size_t lib_size;
    char* lib_buf = load_file(lib_path, &lib_size);
    if (!lib_buf) {
        fprintf(stderr, "Failed to read library.rxbin\n");
        rxvml_destroy(ctx);
        return 1;
    }
    
    if (rxvml_load_module_buffer(ctx, lib_buf, lib_size) != 0) {
        const char* err;
        rxvml_last_error(ctx, &err);
        fprintf(stderr, "Failed to load library module: %s\n", err ? err : "Unknown error");
        free(lib_buf);
        rxvml_destroy(ctx);
        return 1;
    }
    free(lib_buf);

    /* 2. Load the Repro module manually */
    printf("Loading Repro.rxbin...\n");
    size_t repro_size;
    char* repro_buf = load_file("Repro.rxbin", &repro_size);
    if (!repro_buf) {
        fprintf(stderr, "Failed to read Repro.rxbin\n");
        rxvml_destroy(ctx);
        return 1;
    }
    
    if (rxvml_load_module_buffer(ctx, repro_buf, repro_size) != 0) {
        const char* err;
        rxvml_last_error(ctx, &err);
        fprintf(stderr, "Failed to load Repro module from buffer: %s\n", err ? err : "Unknown error");
        free(repro_buf);
        rxvml_destroy(ctx);
        return 1;
    }
    free(repro_buf);

    /* 3. Instantiate the class */
    printf("Instantiating Repro...\n");
    rxvml_class_info* classes = NULL;
    size_t class_count = 0;
    rxvml_discover_classes(ctx, "rxcp", &classes, &class_count);

    if (!classes || class_count == 0) {
        fprintf(stderr, "No classes found in Repro.rxbin\n");
        rxvml_destroy(ctx);
        return 1;
    }

    /* Find Repro class */
    int found = 0;
    char factory_name[512];
    char class_name[256];
    size_t i;
    
    for (i = 0; i < class_count; i++) {
        if (strstr(classes[i].class_name, "Repro")) {
             strcpy(factory_name, classes[i].factory_proc);
             strcpy(class_name, classes[i].class_name);
             found = 1;
             break;
        }
    }

    if (!found) {
        fprintf(stderr, "Repro class not found\n");
        if (classes) free(classes);
        rxvml_destroy(ctx);
        return 1;
    }
    
    /* Call factory */
    rxvml_value* obj = NULL;
    
    if (rxvml_call_plugin(ctx, factory_name, NULL, &obj) != 0 || !obj) {
         const char* err;
         rxvml_last_error(ctx, &err);
         fprintf(stderr, "Failed to instantiate Repro: %s\n", err ? err : "Unknown error");
         if (classes) free(classes);
         rxvml_destroy(ctx);
         return 1;
    }
    
    /* 4. Check initial status */
    rxvml_value* response = NULL;
    const char* status_str;
    size_t status_len;

    printf("Checking initial status...\n");
    if (rxvml_call_method(ctx, obj, class_name, "get_status", 0, NULL, &response) != 0) {
        fprintf(stderr, "Failed to call get_status (initial)\n");
        return 1;
    }
    rxvml_to_str(ctx, response, &status_str, &status_len);
    printf("Initial status: %.*s\n", (int)status_len, status_str);
    
    if (strncmp(status_str, "INITIAL", status_len) != 0) {
        fprintf(stderr, "FAIL: Initial status incorrect\n");
    }
    rxvml_value_free(response);
    response = NULL;

    /* 5. Call process() to modify state */
    printf("Calling process() to modify state...\n");
    if (rxvml_call_method(ctx, obj, class_name, "process", 0, NULL, &response) != 0) {
         fprintf(stderr, "Failed to call process\n");
         return 1;
    }
    rxvml_value_free(response);
    response = NULL;

    /* 6. Check modified status */
    printf("Checking modified status...\n");
    if (rxvml_call_method(ctx, obj, class_name, "get_status", 0, NULL, &response) != 0) {
        fprintf(stderr, "Failed to call get_status (modified)\n");
        return 1;
    }
    rxvml_to_str(ctx, response, &status_str, &status_len);
    printf("Modified status: %.*s\n", (int)status_len, status_str);

    if (strncmp(status_str, "MODIFIED", status_len) == 0) {
        printf("SUCCESS: State was preserved!\n");
    } else {
        printf("FAILURE: State was LOST! Expected 'MODIFIED', got '%.*s'\n", (int)status_len, status_str);
    }

    rxvml_value_free(response);
    rxvml_value_free(obj);
    if (classes) free(classes);
    rxvml_destroy(ctx);

    return 0;
}
