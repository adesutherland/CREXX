#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxvml.h"

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define E2_GETCWD _getcwd
#define E2_PATH_BUFFER 32768
#else
#include <pthread.h>
#include <unistd.h>
#define E2_GETCWD getcwd
#define E2_PATH_BUFFER 4096
#endif

#ifndef CREXX_TEST_LIBRARY_PATH
#define CREXX_TEST_LIBRARY_PATH "library"
#endif

#ifndef CREXX_TEST_E2_ACTIVE_MODULE
#define CREXX_TEST_E2_ACTIVE_MODULE "e2_active_context.rxbin"
#endif
#ifndef CREXX_TEST_E2_DIRECTORY_A
#define CREXX_TEST_E2_DIRECTORY_A "."
#endif
#ifndef CREXX_TEST_E2_DIRECTORY_B
#define CREXX_TEST_E2_DIRECTORY_B ".."
#endif

typedef struct e2_shared_state {
#ifdef _WIN32
    CRITICAL_SECTION mutex;
    CONDITION_VARIABLE condition;
#else
    pthread_mutex_t mutex;
    pthread_cond_t condition;
#endif
    int load_turn;
    int ready_count;
    int first_a_entered;
    int first_b_entered;
    int second_seen;
    int state_entered;
    int wrong_context;
    int failures;
    int say_count[2];
} e2_shared_state;

typedef struct e2_thread_state {
    e2_shared_state *shared;
    int identity;
    int run_rc;
    int callback_count;
    const char *directory;
    const char *token;
#ifdef _WIN32
    DWORD owner_thread_id;
#else
    pthread_t owner_thread;
#endif
} e2_thread_state;

static void shared_lock(e2_shared_state *shared) {
#ifdef _WIN32
    EnterCriticalSection(&shared->mutex);
#else
    (void)pthread_mutex_lock(&shared->mutex);
#endif
}

static void shared_unlock(e2_shared_state *shared) {
#ifdef _WIN32
    LeaveCriticalSection(&shared->mutex);
#else
    (void)pthread_mutex_unlock(&shared->mutex);
#endif
}

static void shared_wait(e2_shared_state *shared) {
#ifdef _WIN32
    SleepConditionVariableCS(&shared->condition, &shared->mutex, INFINITE);
#else
    (void)pthread_cond_wait(&shared->condition, &shared->mutex);
#endif
}

static void shared_broadcast(e2_shared_state *shared) {
#ifdef _WIN32
    WakeAllConditionVariable(&shared->condition);
#else
    (void)pthread_cond_broadcast(&shared->condition);
#endif
}

static e2_shared_state *say_shared;

static int count_occurrences(const char *text, const char *pattern) {
    int count = 0;
    size_t length;
    if (!text || !pattern || !*pattern) return 0;
    length = strlen(pattern);
    while ((text = strstr(text, pattern)) != NULL) {
        count++;
        text += length;
    }
    return count;
}

static void e2_say_a(char *message) {
    (void)message;
    shared_lock(say_shared);
    say_shared->say_count[0]++;
    shared_unlock(say_shared);
}

static void e2_say_b(char *message) {
    (void)message;
    shared_lock(say_shared);
    say_shared->say_count[1]++;
    shared_unlock(say_shared);
}

static int e2_callback(rxvml_context *ctx,
                       const rxvml_address_request *request,
                       rxvml_address_response *response,
                       void *userdata) {
    e2_thread_state *state = (e2_thread_state *)userdata;
    e2_shared_state *shared = state ? state->shared : NULL;

    (void)ctx;
    if (!state || !shared || !request || !response) return -1;
    state->callback_count++;

    shared_lock(shared);
#ifdef _WIN32
    if (state->owner_thread_id != GetCurrentThreadId()) shared->wrong_context++;
#else
    if (!pthread_equal(state->owner_thread, pthread_self())) {
        shared->wrong_context++;
    }
#endif
    if (strcmp(request->command, "FIRST") == 0) {
        if (state->identity == 0) {
            shared->first_a_entered = 1;
            shared_broadcast(shared);
            while (!shared->first_b_entered) shared_wait(shared);
        } else {
            shared->first_b_entered = 1;
            shared_broadcast(shared);
            while (!shared->second_seen) shared_wait(shared);
        }
    } else if (strcmp(request->command, "SECOND") == 0) {
        shared->second_seen = 1;
        shared_broadcast(shared);
    } else if (strcmp(request->command, "STATE") == 0) {
        shared->state_entered++;
        shared_broadcast(shared);
        while (shared->state_entered != 2) shared_wait(shared);
    } else {
        shared->failures++;
    }
    shared_unlock(shared);

    response->rc = 0;
    return 0;
}

static void run_thread(e2_thread_state *state) {
    e2_shared_state *shared = state->shared;
    rxvml_context *ctx = NULL;
    rxvml_value *result = NULL;
    rxvml_value *state_args[3] = {NULL, NULL, NULL};
    const char *state_text = NULL;
    size_t state_length = 0;
    rxinteger result_code = -1;

#ifdef _WIN32
    state->owner_thread_id = GetCurrentThreadId();
#else
    state->owner_thread = pthread_self();
#endif

    shared_lock(shared);
    while (shared->load_turn != state->identity) shared_wait(shared);
    shared_unlock(shared);

    ctx = rxvml_create(NULL, 0);
    if (!ctx ||
        rxvml_load_module_file(ctx, CREXX_TEST_LIBRARY_PATH) <= 0 ||
        rxvml_load_module_file(ctx, CREXX_TEST_E2_ACTIVE_MODULE) <= 0 ||
        rxvml_address_register_callback_environment(
            ctx, "EDITOR", state->identity ? "worker-b" : "worker-a",
            e2_callback, NULL, state) != 0) {
        shared_lock(shared);
        shared->failures++;
        shared->load_turn++;
        shared->ready_count++;
        shared_broadcast(shared);
        shared_unlock(shared);
        if (ctx) rxvml_destroy(ctx);
        return;
    }

    shared_lock(shared);
    shared->load_turn++;
    shared->ready_count++;
    shared_broadcast(shared);
    while (shared->ready_count != 2) shared_wait(shared);
    if (state->identity == 1) {
        while (!shared->first_a_entered) shared_wait(shared);
    }
    shared_unlock(shared);

    rxvml_set_context_say_exit(ctx, state->identity ? e2_say_b : e2_say_a);

    if (rxvml_call_procedure_descriptor(
            ctx, "rxsig1|e2_active_context.run|.int|", 0, NULL,
            &result) != 0 || !result ||
        rxvml_to_int(ctx, result, &result_code) != 0) {
        state->run_rc = -1;
    } else {
        state->run_rc = (int)result_code;
    }

    if (result) rxvml_value_free(result);
    result = NULL;

    state_args[0] = rxvml_value_new(ctx);
    state_args[1] = rxvml_value_new(ctx);
    state_args[2] = rxvml_value_new(ctx);
    if (!state_args[0] || !state_args[1] || !state_args[2] ||
        rxvml_set_str(state_args[0], state->directory,
                      strlen(state->directory)) != 0 ||
        rxvml_set_str(state_args[1], state->token,
                      strlen(state->token)) != 0 ||
        rxvml_set_str(state_args[2],
#ifdef _WIN32
                      "windows", sizeof("windows") - 1u) != 0 ||
#else
                      "posix", sizeof("posix") - 1u) != 0 ||
#endif
        rxvml_call_procedure_descriptor(
            ctx,
            "rxsig1|e2_active_context.state|.string|directory=.string,token=.string,platform=.string",
            3, state_args, &result) != 0 || !result ||
        rxvml_to_str(ctx, result, &state_text, &state_length) != 0 ||
        !state_text || count_occurrences(state_text, state->directory) != 2 ||
        count_occurrences(state_text, state->token) != 2) {
        shared_lock(shared);
        shared->failures++;
        shared_unlock(shared);
    }
    if (result) rxvml_value_free(result);
    if (state_args[0]) rxvml_value_free(state_args[0]);
    if (state_args[1]) rxvml_value_free(state_args[1]);
    if (state_args[2]) rxvml_value_free(state_args[2]);
    rxvml_destroy(ctx);
}

#ifdef _WIN32
static DWORD WINAPI thread_entry(LPVOID argument) {
    run_thread((e2_thread_state *)argument);
    return 0;
}
#else
static void *thread_entry(void *argument) {
    run_thread((e2_thread_state *)argument);
    return NULL;
}
#endif

int main(void) {
    e2_shared_state shared;
    e2_thread_state states[2];
    char cwd_before[E2_PATH_BUFFER];
    char cwd_after[E2_PATH_BUFFER];
    const char *environment_before_value;
    char *environment_before = NULL;
    int i;

    memset(&shared, 0, sizeof(shared));
    memset(states, 0, sizeof(states));
    say_shared = &shared;
    if (!E2_GETCWD(cwd_before, sizeof(cwd_before))) return 1;
    environment_before_value = getenv("CREXX_E2_TOKEN");
    if (environment_before_value) {
        environment_before = (char *)malloc(strlen(environment_before_value) + 1u);
        if (!environment_before) return 1;
        strcpy(environment_before, environment_before_value);
    }
#ifdef _WIN32
    InitializeCriticalSection(&shared.mutex);
    InitializeConditionVariable(&shared.condition);
#else
    (void)pthread_mutex_init(&shared.mutex, NULL);
    (void)pthread_cond_init(&shared.condition, NULL);
#endif
    for (i = 0; i < 2; i++) {
        states[i].shared = &shared;
        states[i].identity = i;
        states[i].run_rc = -1;
        states[i].directory = i ? CREXX_TEST_E2_DIRECTORY_B
                                : CREXX_TEST_E2_DIRECTORY_A;
        states[i].token = i ? "worker-b-token" : "worker-a-token";
    }

#ifdef _WIN32
    {
        HANDLE threads[2];
        threads[0] = CreateThread(NULL, 0, thread_entry, &states[0], 0, NULL);
        threads[1] = CreateThread(NULL, 0, thread_entry, &states[1], 0, NULL);
        if (!threads[0] || !threads[1]) {
            fprintf(stderr, "failed to create E2 worker threads\n");
            return 1;
        }
        WaitForMultipleObjects(2, threads, TRUE, INFINITE);
        CloseHandle(threads[0]);
        CloseHandle(threads[1]);
    }
#else
    {
        pthread_t threads[2];
        if (pthread_create(&threads[0], NULL, thread_entry, &states[0]) != 0 ||
            pthread_create(&threads[1], NULL, thread_entry, &states[1]) != 0) {
            fprintf(stderr, "failed to create E2 worker threads\n");
            return 1;
        }
        (void)pthread_join(threads[0], NULL);
        (void)pthread_join(threads[1], NULL);
    }
#endif

#ifdef _WIN32
    DeleteCriticalSection(&shared.mutex);
#else
    (void)pthread_cond_destroy(&shared.condition);
    (void)pthread_mutex_destroy(&shared.mutex);
#endif

    if (!E2_GETCWD(cwd_after, sizeof(cwd_after)) ||
        strcmp(cwd_before, cwd_after) != 0 ||
        ((environment_before == NULL) != (getenv("CREXX_E2_TOKEN") == NULL)) ||
        (environment_before &&
         strcmp(environment_before, getenv("CREXX_E2_TOKEN")) != 0)) {
        shared.failures++;
    }
    free(environment_before);

    if (shared.failures || shared.wrong_context ||
        states[0].run_rc != 0 || states[1].run_rc != 0 ||
        states[0].callback_count != 3 || states[1].callback_count != 3 ||
        shared.say_count[0] != 1 || shared.say_count[1] != 1) {
        fprintf(stderr,
                "E2 active-context isolation failed: failures=%d wrong=%d "
                "rc=%d/%d callbacks=%d/%d say=%d/%d\n",
                shared.failures, shared.wrong_context,
                states[0].run_rc, states[1].run_rc,
                states[0].callback_count, states[1].callback_count,
                shared.say_count[0], shared.say_count[1]);
        return 1;
    }

    puts("PASS: concurrent RXVML contexts retain worker-owned callback state");
    return 0;
}
