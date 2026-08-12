/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "rxcpmain.h"

#define TEST_PATH_SIZE 1024
#define MOVING_FILE_COUNT 48
#define SNAPSHOT_ITERATIONS 200

typedef struct {
    char directory[TEST_PATH_SIZE];
    volatile int stop;
} MutationState;

static int make_path(char *path, size_t path_size, const char *directory, const char *name) {
    int written = snprintf(path, path_size, "%s/%s", directory, name);
    return written >= 0 && (size_t)written < path_size;
}

static int create_empty_file(const char *path) {
    FILE *file = fopen(path, "wb");
    if (!file) return 0;
    return fclose(file) == 0;
}

static int moving_paths(const char *directory, int index,
                        char *hidden, size_t hidden_size,
                        char *visible, size_t visible_size) {
    char hidden_name[64];
    char visible_name[64];
    int hidden_written;
    int visible_written;

    hidden_written = snprintf(hidden_name, sizeof(hidden_name), ".moving-%02d.hold", index);
    visible_written = snprintf(visible_name, sizeof(visible_name), "moving-%02d.rxbin", index);
    if (hidden_written < 0 || (size_t)hidden_written >= sizeof(hidden_name) ||
        visible_written < 0 || (size_t)visible_written >= sizeof(visible_name)) return 0;
    return make_path(hidden, hidden_size, directory, hidden_name) &&
           make_path(visible, visible_size, directory, visible_name);
}

static void *mutate_import_directory(void *userdata) {
    MutationState *state = (MutationState *)userdata;
    char hidden[TEST_PATH_SIZE];
    char visible[TEST_PATH_SIZE];
    int i;

    while (!state->stop) {
        for (i = 0; i < MOVING_FILE_COUNT; i++) {
            if (moving_paths(state->directory, i, hidden, sizeof(hidden), visible, sizeof(visible))) {
                rename(hidden, visible);
            }
        }
        for (i = 0; i < MOVING_FILE_COUNT; i++) {
            if (moving_paths(state->directory, i, hidden, sizeof(hidden), visible, sizeof(visible))) {
                rename(visible, hidden);
            }
        }
    }
    return 0;
}

static void cleanup_test_directory(const char *directory) {
    static const char *fixed_names[] = {"alpha.rxbin", "middle.rxbin", "zeta.rxbin"};
    char hidden[TEST_PATH_SIZE];
    char visible[TEST_PATH_SIZE];
    char path[TEST_PATH_SIZE];
    size_t i;
    int moving_index;

    for (i = 0; i < sizeof(fixed_names) / sizeof(fixed_names[0]); i++) {
        if (make_path(path, sizeof(path), directory, fixed_names[i])) unlink(path);
    }
    for (moving_index = 0; moving_index < MOVING_FILE_COUNT; moving_index++) {
        if (moving_paths(directory, moving_index, hidden, sizeof(hidden), visible, sizeof(visible))) {
            unlink(hidden);
            unlink(visible);
        }
    }
    rmdir(directory);
}

int main(void) {
    static const char *fixed_names[] = {"alpha.rxbin", "middle.rxbin", "zeta.rxbin"};
    char directory[TEST_PATH_SIZE];
    char path[TEST_PATH_SIZE];
    char hidden[TEST_PATH_SIZE];
    char visible[TEST_PATH_SIZE];
    char *import_locations[2];
    Context context;
    MutationState state;
    pthread_t mutator;
    importable_file **files;
    size_t i;
    int moving_index;
    int iteration;
    int thread_started = 0;
    int result = 1;

    if (snprintf(directory, sizeof(directory), "import-discovery-snapshot-%ld", (long)getpid()) < 0) {
        fprintf(stderr, "Could not format snapshot test directory.\n");
        return 1;
    }
    if (mkdir(directory, 0700) != 0) {
        fprintf(stderr, "Could not create snapshot test directory %s.\n", directory);
        return 1;
    }

    for (i = 0; i < sizeof(fixed_names) / sizeof(fixed_names[0]); i++) {
        if (!make_path(path, sizeof(path), directory, fixed_names[i]) || !create_empty_file(path)) {
            fprintf(stderr, "Could not create fixed import entry %s.\n", fixed_names[i]);
            goto finish;
        }
    }
    for (moving_index = 0; moving_index < MOVING_FILE_COUNT; moving_index++) {
        if (!moving_paths(directory, moving_index, hidden, sizeof(hidden), visible, sizeof(visible)) ||
            !create_empty_file(hidden)) {
            fprintf(stderr, "Could not create moving import entry %d.\n", moving_index);
            goto finish;
        }
    }

    memset(&context, 0, sizeof(context));
    context.location = directory;
    context.file_name = "consumer.crexx";
    context.initial_source_extension = "crexx";
    import_locations[0] = directory;
    import_locations[1] = 0;
    context.import_locations = import_locations;

    memset(&state, 0, sizeof(state));
    if (snprintf(state.directory, sizeof(state.directory), "%s", directory) < 0) {
        fprintf(stderr, "Could not store snapshot test directory.\n");
        goto finish;
    }
    if (pthread_create(&mutator, 0, mutate_import_directory, &state) != 0) {
        fprintf(stderr, "Could not start import directory mutator.\n");
        goto finish;
    }
    thread_started = 1;

    for (iteration = 0; iteration < SNAPSHOT_ITERATIONS; iteration++) {
        int found_alpha = 0;
        int found_middle = 0;
        int found_zeta = 0;
        const char *previous_name = 0;

        files = rxfl_lst(&context);
        if (!files) {
            fprintf(stderr, "Import discovery returned no list at iteration %d.\n", iteration);
            goto finish;
        }
        for (i = 0; files[i]; i++) {
            if (previous_name && strcmp(previous_name, files[i]->name) > 0) {
                fprintf(stderr, "Import discovery was not sorted at iteration %d: %s before %s.\n",
                        iteration, previous_name, files[i]->name);
                rxfl_fre(files);
                goto finish;
            }
            previous_name = files[i]->name;
            if (strcmp(files[i]->name, "alpha.rxbin") == 0) found_alpha = 1;
            if (strcmp(files[i]->name, "middle.rxbin") == 0) found_middle = 1;
            if (strcmp(files[i]->name, "zeta.rxbin") == 0) found_zeta = 1;
        }
        rxfl_fre(files);
        if (!found_alpha || !found_middle || !found_zeta) {
            fprintf(stderr, "Import discovery lost a fixed entry at iteration %d.\n", iteration);
            goto finish;
        }
    }

    result = 0;

finish:
    if (thread_started) {
        state.stop = 1;
        pthread_join(mutator, 0);
    }
    cleanup_test_directory(directory);
    return result;
}
