/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxvmchannel_process.h"

#include "platform.h"
#include "rxvmbyteendpoint.h"
#include "rxvmchannel_internal.h"
#include "rxvmintp.h"
#include "rxvmprocessprotocol.h"
#include "rxvmprogram.h"

#include <errno.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#include <io.h>
#include <windows.h>
typedef CRITICAL_SECTION process_mutex;
typedef CONDITION_VARIABLE process_condition;
typedef HANDLE process_thread;
#define PROCESS_THREAD_RETURN DWORD WINAPI
static int process_mutex_init(process_mutex *mutex) {
    InitializeCriticalSection(mutex);
    return 1;
}
static void process_mutex_destroy(process_mutex *mutex) {
    DeleteCriticalSection(mutex);
}
static void process_mutex_lock(process_mutex *mutex) {
    EnterCriticalSection(mutex);
}
static void process_mutex_unlock(process_mutex *mutex) {
    LeaveCriticalSection(mutex);
}
static int process_condition_init(process_condition *condition) {
    InitializeConditionVariable(condition);
    return 1;
}
static void process_condition_destroy(process_condition *condition) {
    (void)condition;
}
static void process_condition_broadcast(process_condition *condition) {
    WakeAllConditionVariable(condition);
}
static int process_condition_wait(process_condition *condition,
                                  process_mutex *mutex,
                                  int64_t wait_microseconds) {
    DWORD milliseconds;
    BOOL result;
    if (wait_microseconds < 0) milliseconds = INFINITE;
    else {
        uint64_t rounded = ((uint64_t)wait_microseconds + 999u) / 1000u;
        milliseconds = rounded >= (uint64_t)INFINITE
                ? INFINITE - 1u : (DWORD)rounded;
    }
    result = SleepConditionVariableCS(condition, mutex, milliseconds);
    if (result) return 1;
    return GetLastError() == ERROR_TIMEOUT ? 0 : -1;
}
#else
#include <pthread.h>
#include <unistd.h>
typedef pthread_mutex_t process_mutex;
typedef pthread_cond_t process_condition;
typedef pthread_t process_thread;
#define PROCESS_THREAD_RETURN void *
static int process_mutex_init(process_mutex *mutex) {
    return pthread_mutex_init(mutex, 0) == 0;
}
static void process_mutex_destroy(process_mutex *mutex) {
    if (pthread_mutex_destroy(mutex) != 0) abort();
}
static void process_mutex_lock(process_mutex *mutex) {
    if (pthread_mutex_lock(mutex) != 0) abort();
}
static void process_mutex_unlock(process_mutex *mutex) {
    if (pthread_mutex_unlock(mutex) != 0) abort();
}
static int process_condition_init(process_condition *condition) {
    return pthread_cond_init(condition, 0) == 0;
}
static void process_condition_destroy(process_condition *condition) {
    if (pthread_cond_destroy(condition) != 0) abort();
}
static void process_condition_broadcast(process_condition *condition) {
    if (pthread_cond_broadcast(condition) != 0) abort();
}
static int process_condition_wait(process_condition *condition,
                                  process_mutex *mutex,
                                  int64_t wait_microseconds) {
    int result;
    if (wait_microseconds < 0) {
        result = pthread_cond_wait(condition, mutex);
        return result == 0 ? 1 : -1;
    }
#if defined(__APPLE__)
    {
        struct timespec relative;
        relative.tv_sec = (time_t)(wait_microseconds / INT64_C(1000000));
        relative.tv_nsec = (long)((wait_microseconds % INT64_C(1000000)) *
                                  INT64_C(1000));
        result = pthread_cond_timedwait_relative_np(
                condition, mutex, &relative);
    }
#else
    {
        struct timespec absolute;
        uint64_t nanoseconds;
        if (clock_gettime(CLOCK_REALTIME, &absolute) != 0) return -1;
        nanoseconds = (uint64_t)absolute.tv_nsec +
                      (uint64_t)wait_microseconds * UINT64_C(1000);
        absolute.tv_sec += (time_t)(nanoseconds / UINT64_C(1000000000));
        absolute.tv_nsec = (long)(nanoseconds % UINT64_C(1000000000));
        result = pthread_cond_timedwait(condition, mutex, &absolute);
    }
#endif
    if (result == 0) return 1;
    return result == ETIMEDOUT ? 0 : -1;
}
#endif

#define PROCESS_ENDPOINT_CAPACITY (256u * 1024u)
/*
 * Process workers must be able to start while a build or test host is under
 * sustained load.  This is a transport-health bound, not a task deadline;
 * task scopes retain their own independently enforced deadlines.
 */
#define PROCESS_START_TIMEOUT_US INT64_C(30000000)
#define PROCESS_CANCEL_GRACE_US UINT64_C(250000)

typedef struct process_shared process_shared;
typedef struct process_channel process_channel;
typedef struct process_worker process_worker;

typedef struct process_request {
    process_channel *owner;
    process_worker *worker;
    struct process_request *owner_next;
    struct process_request *queue_next;
    unsigned char *envelope;
    size_t envelope_length;
    unsigned char *completion_document;
    size_t completion_document_length;
    uint64_t request_id;
    uint64_t completion_order;
    uint64_t cancel_requested_at;
    int64_t terminal_state;
    int64_t terminal_error;
    unsigned char cancel_kind; /* 1 explicit, 2 deadline, 3 shutdown */
    unsigned char started;
    unsigned char terminal;
    unsigned char queued;
} process_request;

struct process_worker {
    process_shared *shared;
    process_thread dispatch_thread;
    process_thread monitor_thread;
    process_mutex write_mutex;
    process_request *current;
    rxvm_byte_endpoint *input;
    rxvm_byte_endpoint *output;
    rxvm_byte_endpoint *error;
    atomic_uchar process_cancelled;
    atomic_uchar input_stopped;
    atomic_uchar output_stopped;
    atomic_uchar monitor_done;
    atomic_uchar process_live;
    unsigned char write_mutex_initialized;
    unsigned char dispatch_started;
    unsigned char monitor_started;
    unsigned char monitor_joined;
    int spawn_status;
    int exit_status;
    int termination_reason;
    char *spawn_message;
    unsigned char header[RXVM_PROCESS_PROTOCOL_HEADER_SIZE];
    size_t header_length;
    rxvm_process_frame incoming;
    size_t payload_length;
};

struct process_shared {
    process_mutex mutex;
    process_condition changed;
    process_worker *workers;
    process_channel *scopes;
    process_request *queue_head;
    process_request *queue_tail;
    char *program_path;
    char *worker_executable;
    size_t worker_count;
    size_t admission_capacity;
    size_t active_requests;
    size_t references;
    uint64_t next_request_id;
    uint64_t next_completion_order;
    uint64_t completion_generation;
    int test_crash_phase;
    unsigned char pool_closed;
    unsigned char stopping;
};

struct process_channel {
    process_shared *shared;
    process_channel *next_scope;
    process_request *requests;
    uint64_t deadline;
    int64_t failure_policy;
    unsigned char is_scope;
    unsigned char closed;
};

static uint64_t process_now(void) {
#if defined(_WIN32)
    return (uint64_t)GetTickCount64() * UINT64_C(1000);
#else
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) return 0u;
    return (uint64_t)now.tv_sec * UINT64_C(1000000) +
           (uint64_t)now.tv_nsec / UINT64_C(1000);
#endif
}

static int64_t process_remaining(uint64_t deadline) {
    uint64_t now;
    uint64_t remaining;
    if (!deadline) return -1;
    now = process_now();
    if (!now || now >= deadline) return 0;
    remaining = deadline - now;
    return remaining > (uint64_t)INT64_MAX
            ? INT64_MAX : (int64_t)remaining;
}

static char *process_strdup(const char *text) {
    size_t length;
    char *copy;
    if (!text) return 0;
    length = strlen(text);
    copy = (char *)malloc(length + 1u);
    if (copy) memcpy(copy, text, length + 1u);
    return copy;
}

static int process_thread_start(process_thread *thread,
                                PROCESS_THREAD_RETURN (*entry)(void *),
                                void *argument) {
#if defined(_WIN32)
    *thread = CreateThread(0, 0, entry, argument, 0, 0);
    return *thread != 0;
#else
    return pthread_create(thread, 0, entry, argument) == 0;
#endif
}

static void process_thread_join(process_thread thread) {
#if defined(_WIN32)
    if (WaitForSingleObject(thread, INFINITE) != WAIT_OBJECT_0) abort();
    CloseHandle(thread);
#else
    if (pthread_join(thread, 0) != 0) abort();
#endif
}

static int process_executable_basename_is_vm(const char *path) {
    const char *name;
    if (!path) return 0;
    name = strrchr(path, '/');
#if defined(_WIN32)
    {
        const char *backslash = strrchr(path, '\\');
        if (!name || (backslash && backslash > name)) name = backslash;
    }
#endif
    name = name ? name + 1 : path;
    return !strcmp(name, "rxvm") || !strcmp(name, "rxbvm") ||
           !strcmp(name, "rxtvm") || !strcmp(name, "rxvm.exe") ||
           !strcmp(name, "rxbvm.exe") || !strcmp(name, "rxtvm.exe");
}

static char *process_worker_executable(void) {
    char *current = exefqname();
    char *directory;
    char *result;
    size_t length;
    if (current && current[0] && process_executable_basename_is_vm(current)) {
        return current;
    }
    free(current);
    directory = exepath();
    if (!directory) return 0;
    length = strlen(directory);
    result = (char *)malloc(length + 24u);
    if (result) {
#if defined(_WIN32)
        FILE *probe;
        snprintf(result, length + 24u, "%s\\rxvm.exe", directory);
        probe = fopen(result, "rb");
        if (!probe) {
            snprintf(result, length + 24u,
                     "%s\\..\\bin\\rxvm.exe", directory);
        } else {
            fclose(probe);
        }
#else
        FILE *probe;
        snprintf(result, length + 24u, "%s/rxvm", directory);
        probe = fopen(result, "rb");
        if (!probe) {
            snprintf(result, length + 24u,
                     "%s/../bin/rxvm", directory);
        } else {
            fclose(probe);
        }
#endif
    }
    free(directory);
    return result;
}

static char *process_temp_program(
        const rxvm_program_generation *generation) {
    char *path = 0;
#if defined(_WIN32)
    char directory[MAX_PATH + 1u];
    char temporary[MAX_PATH + 1u];
    DWORD length = GetTempPathA(MAX_PATH, directory);
    if (!length || length > MAX_PATH ||
        !GetTempFileNameA(directory, "rxp", 0u, temporary)) return 0;
    path = process_strdup(temporary);
#else
    const char pattern[] = "/tmp/crexx-process-XXXXXX.rxbin";
    int descriptor;
    path = process_strdup(pattern);
    if (!path) return 0;
    descriptor = mkstemps(path, 6);
    if (descriptor < 0) {
        free(path);
        return 0;
    }
    close(descriptor);
#endif
    if (!path || !rxvm_program_generation_write_file(generation, path)) {
        if (path) remove(path);
        free(path);
        return 0;
    }
    return path;
}

static int process_endpoint_write_all(rxvm_byte_endpoint *endpoint,
                                      const void *bytes,
                                      size_t length,
                                      const atomic_uchar *cancelled) {
    size_t offset = 0u;
    while (offset < length) {
        size_t accepted = 0u;
        rxvm_channel_status status = rxvm_byte_endpoint_write(
                endpoint, (const unsigned char *)bytes + offset,
                length - offset, -1, cancelled, &accepted);
        offset += accepted;
        if (status != RXVM_CHANNEL_OK) return 0;
    }
    return 1;
}

static int process_send_frame(process_worker *worker,
                              uint16_t type,
                              uint64_t request_id,
                              const void *payload,
                              size_t payload_length) {
    unsigned char header[RXVM_PROCESS_PROTOCOL_HEADER_SIZE];
    int okay;
    if (!worker || !worker->input ||
        payload_length > RXVM_PROCESS_PROTOCOL_MAX_PAYLOAD) return 0;
    rxvm_process_frame_header(header, type, request_id, payload_length);
    process_mutex_lock(&worker->write_mutex);
    okay = process_endpoint_write_all(
            worker->input, header, sizeof(header),
            &worker->process_cancelled) &&
           (!payload_length || process_endpoint_write_all(
                   worker->input, payload, payload_length,
                   &worker->process_cancelled));
    process_mutex_unlock(&worker->write_mutex);
    return okay;
}

/* Returns 1 for a frame, 0 for timeout, and -1 for EOF/protocol failure. */
static int process_receive_frame(process_worker *worker,
                                 int64_t wait_microseconds,
                                 rxvm_process_frame *frame_out) {
    size_t received = 0u;
    int eof = 0;
    rxvm_channel_status status;
    if (!worker || !frame_out || !worker->output) return -1;
    while (worker->header_length < sizeof(worker->header)) {
        status = rxvm_byte_endpoint_read(
                worker->output,
                worker->header + worker->header_length,
                sizeof(worker->header) - worker->header_length,
                wait_microseconds, 0, &received, &eof);
        worker->header_length += received;
        if (eof) return -1;
        if (status == RXVM_CHANNEL_TIMEOUT ||
            status == RXVM_CHANNEL_WOULD_BLOCK) return 0;
        if (status != RXVM_CHANNEL_OK) return -1;
        if (!received) return 0;
    }
    if (!worker->incoming.type) {
        if (!rxvm_process_frame_header_parse(
                worker->header, &worker->incoming.type,
                &worker->incoming.request_id,
                &worker->incoming.payload_length)) return -1;
        if (worker->incoming.payload_length) {
            worker->incoming.payload = (unsigned char *)malloc(
                    worker->incoming.payload_length);
            if (!worker->incoming.payload) return -1;
        }
    }
    while (worker->payload_length < worker->incoming.payload_length) {
        status = rxvm_byte_endpoint_read(
                worker->output,
                worker->incoming.payload + worker->payload_length,
                worker->incoming.payload_length - worker->payload_length,
                wait_microseconds, 0, &received, &eof);
        worker->payload_length += received;
        if (eof) return -1;
        if (status == RXVM_CHANNEL_TIMEOUT ||
            status == RXVM_CHANNEL_WOULD_BLOCK) return 0;
        if (status != RXVM_CHANNEL_OK) return -1;
        if (!received) return 0;
    }
    *frame_out = worker->incoming;
    memset(&worker->incoming, 0, sizeof(worker->incoming));
    worker->header_length = 0u;
    worker->payload_length = 0u;
    return 1;
}

static PROCESS_THREAD_RETURN process_monitor_run(void *opaque) {
    process_worker *worker = (process_worker *)opaque;
    REDIRECT *input_redirect = rxspawn_redirect_from_byte_endpoint(
            worker->input, &worker->input_stopped);
    REDIRECT *output_redirect = rxspawn_redirect_to_byte_endpoint(
            worker->output, &worker->output_stopped);
    REDIRECT *error_redirect = rxspawn_redirect_to_byte_endpoint(
            worker->error, &worker->output_stopped);
    const char *argv[4];
    int spawn_status = SHELLSPAWN_FAILURE;
    int exit_status = 0;
    int termination_reason = 0;
    char *message = 0;

    argv[0] = worker->shared->worker_executable;
    argv[1] = "--rxvm-process-worker";
    argv[2] = worker->shared->program_path;
    argv[3] = 0;
    if (input_redirect && output_redirect && error_redirect) {
        spawn_status = shellspawn_argv_snapshot(
                argv, 3, input_redirect, output_redirect, error_redirect,
                0, 0, -1, &worker->process_cancelled,
                &worker->input_stopped, &worker->output_stopped,
                &termination_reason, &exit_status, &message);
    }
    atomic_store_explicit(&worker->input_stopped, 1u, memory_order_release);
    atomic_store_explicit(&worker->output_stopped, 1u, memory_order_release);
    rxvm_byte_endpoint_wake(worker->input);
    rxvm_byte_endpoint_wake(worker->output);
    (void)rxspawn_redirect_byte_endpoint_destroy(input_redirect);
    (void)rxvm_byte_endpoint_half_close(
            worker->input, RXVM_BYTE_ENDPOINT_READ);
    (void)rxspawn_redirect_byte_endpoint_destroy(output_redirect);
    (void)rxspawn_redirect_byte_endpoint_destroy(error_redirect);

    process_mutex_lock(&worker->shared->mutex);
    worker->spawn_status = spawn_status;
    worker->exit_status = exit_status;
    worker->termination_reason = termination_reason;
    worker->spawn_message = message;
    atomic_store_explicit(&worker->monitor_done, 1u, memory_order_release);
    atomic_store_explicit(&worker->process_live, 0u, memory_order_release);
    process_condition_broadcast(&worker->shared->changed);
    process_mutex_unlock(&worker->shared->mutex);
#if defined(_WIN32)
    return 0u;
#else
    return 0;
#endif
}

static void process_worker_reset_receive(process_worker *worker) {
    rxvm_process_frame_free(&worker->incoming);
    worker->header_length = 0u;
    worker->payload_length = 0u;
}

static void process_worker_cleanup_process(process_worker *worker,
                                           int terminate) {
    if (!worker) return;
    if (terminate && worker->monitor_started) {
        atomic_store_explicit(
                &worker->process_cancelled, 1u, memory_order_release);
        rxvm_byte_endpoint_wake(worker->input);
        rxvm_byte_endpoint_wake(worker->output);
    }
    if (worker->monitor_started && !worker->monitor_joined) {
        process_thread_join(worker->monitor_thread);
        worker->monitor_joined = 1u;
    }
    process_worker_reset_receive(worker);
    rxvm_byte_endpoint_release(worker->input);
    rxvm_byte_endpoint_release(worker->output);
    rxvm_byte_endpoint_release(worker->error);
    worker->input = 0;
    worker->output = 0;
    worker->error = 0;
    worker->monitor_started = 0u;
    worker->monitor_joined = 0u;
    atomic_store_explicit(&worker->monitor_done, 0u, memory_order_release);
    atomic_store_explicit(&worker->process_live, 0u, memory_order_release);
    free(worker->spawn_message);
    worker->spawn_message = 0;
}

static int process_worker_start_process(process_worker *worker) {
    rxvm_process_frame ready;
    uint64_t deadline;
    int receive_result = 0;
    if (!worker) return 0;
    process_worker_cleanup_process(worker, 1);
    worker->input = rxvm_byte_endpoint_create(
            RXVM_BYTE_ENDPOINT_DUPLEX, PROCESS_ENDPOINT_CAPACITY, 0, 0u, 0);
    worker->output = rxvm_byte_endpoint_create(
            RXVM_BYTE_ENDPOINT_DUPLEX, PROCESS_ENDPOINT_CAPACITY, 0, 0u, 0);
    worker->error = rxvm_byte_endpoint_create(
            RXVM_BYTE_ENDPOINT_WRITE, 1u, 0, 0u, 1);
    if (!worker->input || !worker->output || !worker->error) {
        process_worker_cleanup_process(worker, 1);
        return 0;
    }
    atomic_store_explicit(&worker->process_cancelled, 0u,
                          memory_order_release);
    atomic_store_explicit(&worker->input_stopped, 0u,
                          memory_order_release);
    atomic_store_explicit(&worker->output_stopped, 0u,
                          memory_order_release);
    atomic_store_explicit(&worker->monitor_done, 0u,
                          memory_order_release);
    atomic_store_explicit(&worker->process_live, 0u,
                          memory_order_release);
    if (!process_thread_start(&worker->monitor_thread,
                              process_monitor_run, worker)) {
        process_worker_cleanup_process(worker, 1);
        return 0;
    }
    worker->monitor_started = 1u;
    deadline = process_now() + (uint64_t)PROCESS_START_TIMEOUT_US;
    do {
        int64_t remaining = process_remaining(deadline);
        if (!remaining) break;
        if (remaining > INT64_C(50000)) remaining = INT64_C(50000);
        receive_result = process_receive_frame(worker, remaining, &ready);
    } while (receive_result == 0);
    if (receive_result != 1 || ready.type != RXVM_PROCESS_FRAME_READY ||
        ready.request_id || ready.payload_length) {
        if (receive_result == 1) rxvm_process_frame_free(&ready);
        process_worker_cleanup_process(worker, 1);
        return 0;
    }
    rxvm_process_frame_free(&ready);
    process_mutex_lock(&worker->shared->mutex);
    atomic_store_explicit(&worker->process_live, 1u, memory_order_release);
    process_mutex_unlock(&worker->shared->mutex);
    return 1;
}

static void process_publish_terminal(process_worker *worker,
                                     process_request *request,
                                     rxvm_process_frame *result,
                                     int transport_lost) {
    process_shared *shared = worker->shared;
    process_mutex_lock(&shared->mutex);
    if (request->terminal) abort();
    if (request->cancel_kind == 1u) {
        request->terminal_state = 3;
        request->terminal_error = 3;
    } else if (request->cancel_kind == 2u) {
        request->terminal_state = 4;
        request->terminal_error = 6;
    } else if (request->cancel_kind == 3u) {
        request->terminal_state = 6;
        request->terminal_error = 8;
    } else if (transport_lost) {
        request->terminal_state = request->started ? 8 : 7;
        request->terminal_error = RXVM_CHANNEL_PROVIDER_FAILURE;
    } else if (result) {
        rxvm_channel_provider_completion completion;
        if (rxvm_channel_decode_process_completion(
                result->payload, result->payload_length,
                &completion) == RXVM_CHANNEL_OK) {
            request->completion_document = result->payload;
            request->completion_document_length = result->payload_length;
            result->payload = 0;
            result->payload_length = 0u;
            request->terminal_state = completion.state;
            request->terminal_error = completion.error_code;
        } else {
            request->terminal_state = 2;
            request->terminal_error = RXVM_CHANNEL_PROVIDER_FAILURE;
        }
    } else {
        request->terminal_state = 2;
        request->terminal_error = RXVM_CHANNEL_PROVIDER_FAILURE;
    }
    request->worker = 0;
    request->terminal = 1u;
    request->completion_order = ++shared->next_completion_order;
    if (!shared->active_requests) abort();
    shared->active_requests--;
    shared->completion_generation++;
    worker->current = 0;
    process_condition_broadcast(&shared->changed);
    process_mutex_unlock(&shared->mutex);
}

static PROCESS_THREAD_RETURN process_dispatch_run(void *opaque) {
    process_worker *worker = (process_worker *)opaque;
    process_shared *shared = worker->shared;
    for (;;) {
        process_request *request;
        int test_crash_phase;
        int sent;
        int transport_lost = 0;
        rxvm_process_frame result;
        memset(&result, 0, sizeof(result));

        process_mutex_lock(&shared->mutex);
        while (!shared->stopping && !shared->queue_head) {
            if (process_condition_wait(&shared->changed,
                                       &shared->mutex, -1) < 0) abort();
        }
        if (shared->stopping && !shared->queue_head) {
            process_mutex_unlock(&shared->mutex);
            break;
        }
        request = shared->queue_head;
        shared->queue_head = request->queue_next;
        if (!shared->queue_head) shared->queue_tail = 0;
        request->queue_next = 0;
        request->queued = 0u;
        request->worker = worker;
        worker->current = request;
        test_crash_phase = shared->test_crash_phase;
        shared->test_crash_phase = 0;
        process_mutex_unlock(&shared->mutex);

        if (test_crash_phase == 1) {
            atomic_store_explicit(&worker->process_cancelled, 1u,
                                  memory_order_release);
            rxvm_byte_endpoint_wake(worker->input);
            rxvm_byte_endpoint_wake(worker->output);
            transport_lost = 1;
        }
        if (!atomic_load_explicit(&worker->process_live,
                                  memory_order_acquire) &&
            !process_worker_start_process(worker)) {
            transport_lost = 1;
        }
        sent = !transport_lost && process_send_frame(
                worker, RXVM_PROCESS_FRAME_INVOKE, request->request_id,
                request->envelope, request->envelope_length);
        if (!sent) transport_lost = 1;
        if (sent) {
            int cancelled;
            process_mutex_lock(&shared->mutex);
            cancelled = request->cancel_kind != 0u;
            process_mutex_unlock(&shared->mutex);
            if (cancelled) {
                (void)process_send_frame(worker, RXVM_PROCESS_FRAME_CANCEL,
                                         request->request_id, 0, 0u);
            }
        }
        while (!transport_lost && !result.type) {
            rxvm_process_frame frame;
            int received = process_receive_frame(
                    worker, INT64_C(10000), &frame);
            if (received < 0) {
                transport_lost = 1;
                break;
            }
            if (received == 1) {
                if (frame.type == RXVM_PROCESS_FRAME_STARTED &&
                    frame.request_id == request->request_id &&
                    !frame.payload_length && !request->started) {
                    request->started = 1u;
                    rxvm_process_frame_free(&frame);
                    if (test_crash_phase == 2) {
                        atomic_store_explicit(
                                &worker->process_cancelled, 1u,
                                memory_order_release);
                        rxvm_byte_endpoint_wake(worker->input);
                        rxvm_byte_endpoint_wake(worker->output);
                    }
                } else if (frame.type == RXVM_PROCESS_FRAME_RESULT &&
                           frame.request_id == request->request_id &&
                           frame.payload_length) {
                    result = frame;
                } else {
                    rxvm_process_frame_free(&frame);
                    transport_lost = 1;
                }
            }
            process_mutex_lock(&shared->mutex);
            if (request->cancel_kind && request->cancel_requested_at &&
                process_now() - request->cancel_requested_at >=
                        PROCESS_CANCEL_GRACE_US) {
                atomic_store_explicit(
                        &worker->process_cancelled, 1u,
                        memory_order_release);
                rxvm_byte_endpoint_wake(worker->input);
                rxvm_byte_endpoint_wake(worker->output);
            }
            if (atomic_load_explicit(&worker->monitor_done,
                                     memory_order_acquire) && !result.type) {
                transport_lost = 1;
            }
            process_mutex_unlock(&shared->mutex);
        }
        process_publish_terminal(
                worker, request, result.type ? &result : 0, transport_lost);
        rxvm_process_frame_free(&result);
        if (transport_lost ||
            atomic_load_explicit(&worker->process_cancelled,
                                 memory_order_acquire)) {
            process_worker_cleanup_process(worker, 1);
        }
    }
    if (atomic_load_explicit(&worker->process_live, memory_order_acquire)) {
        (void)process_send_frame(worker, RXVM_PROCESS_FRAME_SHUTDOWN,
                                 0u, 0, 0u);
    }
    process_worker_cleanup_process(worker, 1);
#if defined(_WIN32)
    return 0u;
#else
    return 0;
#endif
}

static void process_queue_remove(process_shared *shared,
                                 process_request *request) {
    process_request *cursor = shared->queue_head;
    process_request *previous = 0;
    while (cursor && cursor != request) {
        previous = cursor;
        cursor = cursor->queue_next;
    }
    if (!cursor) return;
    if (previous) previous->queue_next = cursor->queue_next;
    else shared->queue_head = cursor->queue_next;
    if (shared->queue_tail == cursor) shared->queue_tail = previous;
    cursor->queue_next = 0;
    cursor->queued = 0u;
}

static void process_publish_queued_cancel(process_request *request,
                                          unsigned char kind) {
    process_shared *shared = request->owner->shared;
    process_queue_remove(shared, request);
    request->cancel_kind = kind;
    request->terminal_state = kind == 2u ? 4 : (kind == 3u ? 6 : 3);
    request->terminal_error = kind == 2u ? 6 : (kind == 3u ? 8 : 3);
    request->terminal = 1u;
    request->completion_order = ++shared->next_completion_order;
    if (!shared->active_requests) abort();
    shared->active_requests--;
    shared->completion_generation++;
    process_condition_broadcast(&shared->changed);
}

static void process_request_cancel_locked(process_request *request,
                                          unsigned char kind,
                                          process_worker **signal_out) {
    if (signal_out) *signal_out = 0;
    if (request->terminal || request->cancel_kind) return;
    if (request->queued) {
        process_publish_queued_cancel(request, kind);
        return;
    }
    request->cancel_kind = kind;
    request->cancel_requested_at = process_now();
    if (!request->cancel_requested_at) request->cancel_requested_at = 1u;
    if (signal_out) *signal_out = request->worker;
}

static void process_signal_cancel(process_worker *worker,
                                  uint64_t request_id) {
    if (worker) {
        (void)process_send_frame(worker, RXVM_PROCESS_FRAME_CANCEL,
                                 request_id, 0, 0u);
    }
}

static void process_expire_scope(process_channel *channel) {
    process_request *request;
    process_worker **workers = 0;
    uint64_t *request_ids = 0;
    size_t count = 0u;
    size_t capacity = 0u;
    if (!channel || !channel->is_scope || !channel->deadline ||
        process_remaining(channel->deadline) != 0) return;
    process_mutex_lock(&channel->shared->mutex);
    for (request = channel->requests; request; request = request->owner_next) {
        process_worker *worker = 0;
        process_request_cancel_locked(request, 2u, &worker);
        if (worker) {
            if (count == capacity) {
                size_t next = capacity ? capacity * 2u : 4u;
                process_worker **new_workers = (process_worker **)realloc(
                        workers, next * sizeof(*workers));
                uint64_t *new_ids = (uint64_t *)realloc(
                        request_ids, next * sizeof(*request_ids));
                if (!new_workers || !new_ids) {
                    free(new_workers ? new_workers : workers);
                    free(new_ids ? new_ids : request_ids);
                    workers = 0;
                    request_ids = 0;
                    count = 0u;
                    break;
                }
                workers = new_workers;
                request_ids = new_ids;
                capacity = next;
            }
            workers[count] = worker;
            request_ids[count++] = request->request_id;
        }
    }
    process_mutex_unlock(&channel->shared->mutex);
    while (count) {
        count--;
        process_signal_cancel(workers[count], request_ids[count]);
    }
    free(workers);
    free(request_ids);
}

static rxvm_channel_status process_open(
        void *module_state,
        rxvm_context *context,
        const void *configuration,
        size_t configuration_length,
        void **channel_state_out) {
    size_t worker_count;
    size_t admission_capacity;
    int64_t pool_capability;
    int64_t failure_policy;
    int64_t timeout_microseconds;
    rxvm_channel_status status;
    process_channel *channel;
    process_shared *shared;
    const rxvm_program_generation *generation;
    rxvm_program_result program_result;
    size_t index;
    (void)module_state;
    if (channel_state_out) *channel_state_out = 0;
    if (!context || !channel_state_out) return RXVM_CHANNEL_INVALID_ARGUMENT;

    status = rxvm_channel_parse_task_pool_configuration(
            configuration, configuration_length,
            RXVM_CHANNEL_PROVIDER_PROCESS,
            &worker_count, &admission_capacity);
    if (status != RXVM_CHANNEL_OK) {
        void *pool_state = 0;
        status = rxvm_channel_parse_task_scope_configuration(
                configuration, configuration_length,
                RXVM_CHANNEL_PROVIDER_PROCESS, &pool_capability,
                &failure_policy, &timeout_microseconds);
        if (status != RXVM_CHANNEL_OK) return status;
        status = rxvm_channel_resolve_provider_state(
                context, pool_capability, RXVM_CHANNEL_PROVIDER_PROCESS,
                &pool_state);
        if (status != RXVM_CHANNEL_OK) return status;
        channel = (process_channel *)pool_state;
        if (!channel || channel->is_scope || channel->closed ||
            !channel->shared) return RXVM_CHANNEL_INVALID_CONFIGURATION;
        shared = channel->shared;
        channel = (process_channel *)calloc(1u, sizeof(*channel));
        if (!channel) return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
        process_mutex_lock(&shared->mutex);
        if (shared->pool_closed || shared->references == SIZE_MAX) {
            process_mutex_unlock(&shared->mutex);
            free(channel);
            return RXVM_CHANNEL_INVALID_CONFIGURATION;
        }
        shared->references++;
        channel->shared = shared;
        channel->failure_policy = failure_policy;
        channel->is_scope = 1u;
        channel->next_scope = shared->scopes;
        shared->scopes = channel;
        if (timeout_microseconds >= 0) {
            uint64_t now = process_now();
            uint64_t duration = (uint64_t)timeout_microseconds;
            channel->deadline = duration > UINT64_MAX - now
                    ? UINT64_MAX : now + duration;
            if (!channel->deadline) channel->deadline = 1u;
        }
        process_mutex_unlock(&shared->mutex);
        *channel_state_out = channel;
        return RXVM_CHANNEL_OK;
    }

    generation = rxvm_program_generation_current(context);
    if (!generation) {
        program_result = rxvm_program_generation_seal(context, &generation);
        if (program_result != RXVM_PROGRAM_OK) {
            return program_result == RXVM_PROGRAM_OUT_OF_MEMORY
                    ? RXVM_CHANNEL_RESOURCE_EXHAUSTED
                    : RXVM_CHANNEL_PROVIDER_UNAVAILABLE;
        }
    }
    channel = (process_channel *)calloc(1u, sizeof(*channel));
    shared = (process_shared *)calloc(1u, sizeof(*shared));
    if (!channel || !shared) {
        free(channel);
        free(shared);
        return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    }
    if (!process_mutex_init(&shared->mutex)) {
        free(channel);
        free(shared);
        return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    }
    if (!process_condition_init(&shared->changed)) {
        process_mutex_destroy(&shared->mutex);
        free(channel);
        free(shared);
        return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    }
    shared->program_path = process_temp_program(generation);
    shared->worker_executable = process_worker_executable();
    shared->workers = (process_worker *)calloc(
            worker_count, sizeof(*shared->workers));
    if (!shared->program_path || !shared->worker_executable ||
        !shared->workers) goto open_failure;
    shared->worker_count = worker_count;
    shared->admission_capacity = admission_capacity;
    shared->references = 1u;
    shared->next_request_id = 1u;
    channel->shared = shared;
    for (index = 0u; index < worker_count; index++) {
        shared->workers[index].shared = shared;
        atomic_init(&shared->workers[index].process_cancelled, 0u);
        atomic_init(&shared->workers[index].input_stopped, 0u);
        atomic_init(&shared->workers[index].output_stopped, 0u);
        atomic_init(&shared->workers[index].monitor_done, 0u);
        atomic_init(&shared->workers[index].process_live, 0u);
    }
    for (index = 0u; index < worker_count; index++) {
        if (!process_mutex_init(&shared->workers[index].write_mutex)) {
            goto open_failure;
        }
        shared->workers[index].write_mutex_initialized = 1u;
        if (!process_worker_start_process(&shared->workers[index])) {
            goto open_failure;
        }
    }
    for (index = 0u; index < worker_count; index++) {
        if (!process_thread_start(
                &shared->workers[index].dispatch_thread,
                process_dispatch_run, &shared->workers[index])) {
            goto open_failure;
        }
        shared->workers[index].dispatch_started = 1u;
    }
    *channel_state_out = channel;
    return RXVM_CHANNEL_OK;

open_failure:
    process_mutex_lock(&shared->mutex);
    shared->stopping = 1u;
    process_condition_broadcast(&shared->changed);
    process_mutex_unlock(&shared->mutex);
    if (shared->workers) {
        for (index = 0u; index < worker_count; index++) {
            if (shared->workers[index].dispatch_started) {
                process_thread_join(shared->workers[index].dispatch_thread);
            } else {
                process_worker_cleanup_process(
                        &shared->workers[index], 1);
            }
            if (shared->workers[index].write_mutex_initialized) {
                process_mutex_destroy(&shared->workers[index].write_mutex);
            }
        }
    }
    if (shared->program_path) remove(shared->program_path);
    free(shared->program_path);
    free(shared->worker_executable);
    free(shared->workers);
    process_condition_destroy(&shared->changed);
    process_mutex_destroy(&shared->mutex);
    free(shared);
    free(channel);
    return RXVM_CHANNEL_PROVIDER_UNAVAILABLE;
}

static rxvm_channel_status process_start(
        void *channel_state,
        const void *envelope,
        size_t envelope_length,
        int64_t wait_microseconds,
        void **request_state_out) {
    process_channel *channel = (process_channel *)channel_state;
    process_shared *shared;
    process_request *request;
    rxvm_channel_task_invoke invoke;
    rxvm_channel_status status;
    uint64_t operation_deadline = 0u;
    if (request_state_out) *request_state_out = 0;
    if (!channel || !channel->shared || channel->closed ||
        !request_state_out || wait_microseconds < -1 ||
        !envelope || !envelope_length ||
        envelope_length > RXVM_PROCESS_PROTOCOL_MAX_PAYLOAD) {
        return RXVM_CHANNEL_INVALID_ARGUMENT;
    }
    status = rxvm_channel_parse_task_invoke(
            envelope, envelope_length, &invoke);
    if (status != RXVM_CHANNEL_OK) return status;
    rxvm_channel_task_invoke_free(&invoke);
    request = (process_request *)calloc(1u, sizeof(*request));
    if (!request) return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    request->envelope = (unsigned char *)malloc(envelope_length);
    if (!request->envelope) {
        free(request);
        return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    }
    memcpy(request->envelope, envelope, envelope_length);
    request->envelope_length = envelope_length;
    request->owner = channel;
    shared = channel->shared;
    if (wait_microseconds > 0) {
        uint64_t now = process_now();
        operation_deadline = (uint64_t)wait_microseconds > UINT64_MAX - now
                ? UINT64_MAX : now + (uint64_t)wait_microseconds;
    }
    process_mutex_lock(&shared->mutex);
    while (shared->active_requests >= shared->admission_capacity) {
        int64_t remaining = wait_microseconds;
        int waited;
        if (shared->stopping || shared->pool_closed || channel->closed) {
            process_mutex_unlock(&shared->mutex);
            free(request->envelope);
            free(request);
            return RXVM_CHANNEL_SHUTTING_DOWN;
        }
        if (wait_microseconds == 0) {
            process_mutex_unlock(&shared->mutex);
            free(request->envelope);
            free(request);
            return RXVM_CHANNEL_BACKPRESSURE;
        }
        if (operation_deadline) remaining = process_remaining(operation_deadline);
        if (channel->deadline) {
            int64_t scope_remaining = process_remaining(channel->deadline);
            if (remaining < 0 || scope_remaining < remaining) {
                remaining = scope_remaining;
            }
        }
        if (!remaining) {
            process_mutex_unlock(&shared->mutex);
            free(request->envelope);
            free(request);
            return RXVM_CHANNEL_TIMEOUT;
        }
        waited = process_condition_wait(
                &shared->changed, &shared->mutex, remaining);
        if (waited <= 0 && wait_microseconds >= 0) {
            process_mutex_unlock(&shared->mutex);
            free(request->envelope);
            free(request);
            return waited < 0 ? RXVM_CHANNEL_PROVIDER_FAILURE
                              : RXVM_CHANNEL_TIMEOUT;
        }
    }
    if (shared->stopping || channel->closed ||
        (channel->is_scope && shared->pool_closed)) {
        process_mutex_unlock(&shared->mutex);
        free(request->envelope);
        free(request);
        return RXVM_CHANNEL_SHUTTING_DOWN;
    }
    if (channel->deadline && process_remaining(channel->deadline) == 0) {
        process_mutex_unlock(&shared->mutex);
        free(request->envelope);
        free(request);
        return RXVM_CHANNEL_TIMEOUT;
    }
    if (!shared->next_request_id) {
        process_mutex_unlock(&shared->mutex);
        free(request->envelope);
        free(request);
        return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    }
    request->request_id = shared->next_request_id++;
    request->owner_next = channel->requests;
    channel->requests = request;
    request->queued = 1u;
    if (shared->queue_tail) shared->queue_tail->queue_next = request;
    else shared->queue_head = request;
    shared->queue_tail = request;
    shared->active_requests++;
    process_condition_broadcast(&shared->changed);
    process_mutex_unlock(&shared->mutex);
    *request_state_out = request;
    return RXVM_CHANNEL_OK;
}

static void process_cancel_channel(process_channel *channel,
                                   unsigned char kind);

static int process_terminal_snapshot(
        void *channel_state,
        void *request_state,
        rxvm_channel_provider_completion *completion_out,
        uint64_t *completion_order_out) {
    process_channel *channel = (process_channel *)channel_state;
    process_request *request = (process_request *)request_state;
    int terminal;
    int fail_fast = 0;
    if (completion_out) memset(completion_out, 0, sizeof(*completion_out));
    if (completion_order_out) *completion_order_out = 0u;
    if (!channel || !request || request->owner != channel) return -1;
    process_expire_scope(channel);
    process_mutex_lock(&channel->shared->mutex);
    terminal = request->terminal;
    if (terminal && completion_out) {
        if (request->completion_document) {
            if (rxvm_channel_decode_process_completion(
                    request->completion_document,
                    request->completion_document_length,
                    completion_out) != RXVM_CHANNEL_OK) {
                process_mutex_unlock(&channel->shared->mutex);
                return -1;
            }
        } else {
            completion_out->state = request->terminal_state;
            completion_out->error_code = request->terminal_error;
            completion_out->message = request->terminal_state == 3
                    ? "cancelled"
                    : (request->terminal_state == 4
                       ? "deadline exceeded"
                       : (request->terminal_state == 6
                          ? "task rejected during shutdown"
                          : (request->terminal_state == 7
                             ? "process transport lost before execution"
                             : (request->terminal_state == 8
                                ? "process outcome unknown after transport loss"
                                : "process task failed"))));
        }
        if (channel->failure_policy == 1 && completion_out->state != 1) {
            fail_fast = 1;
        }
    }
    if (terminal && completion_order_out) {
        *completion_order_out = request->completion_order;
    }
    process_mutex_unlock(&channel->shared->mutex);
    if (fail_fast) process_cancel_channel(channel, 1u);
    return terminal ? 1 : 0;
}

static uint64_t process_completion_generation(void *channel_state) {
    process_channel *channel = (process_channel *)channel_state;
    uint64_t generation;
    if (!channel || !channel->shared) return 0u;
    process_expire_scope(channel);
    process_mutex_lock(&channel->shared->mutex);
    generation = channel->shared->completion_generation;
    process_mutex_unlock(&channel->shared->mutex);
    return generation;
}

static int process_completion_wait(void *channel_state,
                                   uint64_t observed_generation,
                                   int64_t wait_microseconds) {
    process_channel *channel = (process_channel *)channel_state;
    uint64_t deadline = 0u;
    int result = 0;
    if (!channel || !channel->shared || wait_microseconds < -1) return -1;
    if (wait_microseconds > 0) {
        uint64_t now = process_now();
        deadline = (uint64_t)wait_microseconds > UINT64_MAX - now
                ? UINT64_MAX : now + (uint64_t)wait_microseconds;
    }
    for (;;) {
        int64_t remaining = wait_microseconds;
        process_expire_scope(channel);
        process_mutex_lock(&channel->shared->mutex);
        if (channel->shared->completion_generation != observed_generation) {
            process_mutex_unlock(&channel->shared->mutex);
            return 1;
        }
        if (deadline) remaining = process_remaining(deadline);
        if (channel->deadline) {
            int64_t scope_remaining = process_remaining(channel->deadline);
            if (!scope_remaining) {
                if (remaining < 0 || remaining > INT64_C(10000)) {
                    remaining = INT64_C(10000);
                }
            } else if (remaining < 0 || scope_remaining < remaining) {
                remaining = scope_remaining;
            }
        }
        if (!remaining) {
            process_mutex_unlock(&channel->shared->mutex);
            return 0;
        }
        result = process_condition_wait(
                &channel->shared->changed,
                &channel->shared->mutex, remaining);
        process_mutex_unlock(&channel->shared->mutex);
        if (result < 0) return -1;
        if (!result && (!channel->deadline ||
                       process_remaining(channel->deadline) != 0)) return 0;
    }
}

static rxvm_channel_status process_cancel(void *channel_state,
                                          void *request_state,
                                          const void *reason,
                                          size_t reason_length) {
    process_channel *channel = (process_channel *)channel_state;
    process_request *request = (process_request *)request_state;
    process_worker *worker = 0;
    (void)reason;
    (void)reason_length;
    if (!channel || !request || request->owner != channel) {
        return RXVM_CHANNEL_INVALID_ARGUMENT;
    }
    process_mutex_lock(&channel->shared->mutex);
    if (request->terminal || request->cancel_kind) {
        process_mutex_unlock(&channel->shared->mutex);
        return RXVM_CHANNEL_ALREADY_TERMINAL;
    }
    process_request_cancel_locked(request, 1u, &worker);
    process_mutex_unlock(&channel->shared->mutex);
    process_signal_cancel(worker, request->request_id);
    return RXVM_CHANNEL_OK;
}

static void process_cancel_channel(process_channel *channel,
                                   unsigned char kind) {
    process_request *request;
    if (!channel) return;
    for (;;) {
        process_worker *worker = 0;
        uint64_t request_id = 0u;
        process_mutex_lock(&channel->shared->mutex);
        for (request = channel->requests; request;
             request = request->owner_next) {
            if (!request->terminal && !request->cancel_kind) {
                process_request_cancel_locked(request, kind, &worker);
                request_id = request->request_id;
                break;
            }
        }
        process_mutex_unlock(&channel->shared->mutex);
        if (!request) break;
        process_signal_cancel(worker, request_id);
    }
}

static void process_wait_channel(process_channel *channel) {
    for (;;) {
        process_request *request;
        process_mutex_lock(&channel->shared->mutex);
        for (request = channel->requests; request;
             request = request->owner_next) {
            if (!request->terminal) break;
        }
        if (!request) {
            process_mutex_unlock(&channel->shared->mutex);
            return;
        }
        if (process_condition_wait(
                &channel->shared->changed,
                &channel->shared->mutex, INT64_C(10000)) < 0) abort();
        process_mutex_unlock(&channel->shared->mutex);
        process_expire_scope(channel);
    }
}

static rxvm_channel_status process_close(void *channel_state, int64_t mode) {
    process_channel *channel = (process_channel *)channel_state;
    process_channel *scope;
    if (!channel || !channel->shared || channel->closed) {
        return RXVM_CHANNEL_INTERNAL_ERROR;
    }
    channel->closed = 1u;
    if (!channel->is_scope) {
        process_mutex_lock(&channel->shared->mutex);
        channel->shared->pool_closed = 1u;
        process_mutex_unlock(&channel->shared->mutex);
    }
    if (mode == 2) process_cancel_channel(channel, 3u);
    process_wait_channel(channel);
    if (!channel->is_scope) {
        for (scope = channel->shared->scopes; scope;
             scope = scope->next_scope) {
            if (mode == 2) process_cancel_channel(scope, 3u);
            process_wait_channel(scope);
        }
    }
    return RXVM_CHANNEL_OK;
}

static rxvm_channel_status process_request_destroy(void *channel_state,
                                                   void *request_state) {
    process_channel *channel = (process_channel *)channel_state;
    process_request *request = (process_request *)request_state;
    process_request **cursor;
    if (!channel || !request || request->owner != channel) {
        return RXVM_CHANNEL_INVALID_ARGUMENT;
    }
    process_mutex_lock(&channel->shared->mutex);
    if (!request->terminal || request->worker || request->queued) {
        process_mutex_unlock(&channel->shared->mutex);
        return RXVM_CHANNEL_INTERNAL_ERROR;
    }
    cursor = &channel->requests;
    while (*cursor && *cursor != request) cursor = &(*cursor)->owner_next;
    if (*cursor != request) {
        process_mutex_unlock(&channel->shared->mutex);
        return RXVM_CHANNEL_INTERNAL_ERROR;
    }
    *cursor = request->owner_next;
    process_mutex_unlock(&channel->shared->mutex);
    free(request->envelope);
    free(request->completion_document);
    free(request);
    return RXVM_CHANNEL_OK;
}

static void process_shared_destroy(process_shared *shared) {
    size_t index;
    if (!shared) return;
    process_mutex_lock(&shared->mutex);
    shared->stopping = 1u;
    process_condition_broadcast(&shared->changed);
    process_mutex_unlock(&shared->mutex);
    for (index = 0u; index < shared->worker_count; index++) {
        process_worker *worker = &shared->workers[index];
        if (worker->dispatch_started) {
            process_thread_join(worker->dispatch_thread);
        } else {
            process_worker_cleanup_process(worker, 1);
        }
        if (worker->write_mutex_initialized) {
            process_mutex_destroy(&worker->write_mutex);
        }
    }
    if (shared->program_path) remove(shared->program_path);
    free(shared->program_path);
    free(shared->worker_executable);
    free(shared->workers);
    process_condition_destroy(&shared->changed);
    process_mutex_destroy(&shared->mutex);
    free(shared);
}

static void process_channel_destroy(void *channel_state) {
    process_channel *channel = (process_channel *)channel_state;
    process_shared *shared;
    process_channel **cursor;
    int destroy_shared;
    if (!channel) return;
    shared = channel->shared;
    if (!channel->closed || channel->requests || !shared) abort();
    process_mutex_lock(&shared->mutex);
    if (channel->is_scope) {
        cursor = &shared->scopes;
        while (*cursor && *cursor != channel) cursor = &(*cursor)->next_scope;
        if (*cursor != channel) abort();
        *cursor = channel->next_scope;
    }
    if (!shared->references) abort();
    shared->references--;
    destroy_shared = shared->references == 0u;
    process_mutex_unlock(&shared->mutex);
    free(channel);
    if (destroy_shared) process_shared_destroy(shared);
}

void rxvm_channel_process_provider_descriptor(
        rxvm_channel_provider_descriptor *descriptor) {
    if (!descriptor) return;
    memset(descriptor, 0, sizeof(*descriptor));
    descriptor->type = RXVM_CHANNEL_PROVIDER_PROCESS;
    descriptor->name = "crexx.core.isolated-process";
    descriptor->abi_version = RXVM_CHANNEL_PROVIDER_ABI_VERSION;
    descriptor->configuration_version_min = 1u;
    descriptor->configuration_version_max = 1u;
    descriptor->capabilities = RXVM_CHANNEL_PROCESS_CAPABILITIES;
    descriptor->operations.open = process_open;
    descriptor->operations.start = process_start;
    descriptor->operations.terminal_snapshot = process_terminal_snapshot;
    descriptor->operations.completion_generation =
            process_completion_generation;
    descriptor->operations.completion_wait = process_completion_wait;
    descriptor->operations.cancel = process_cancel;
    descriptor->operations.close = process_close;
    descriptor->operations.request_destroy = process_request_destroy;
    descriptor->operations.channel_destroy = process_channel_destroy;
}

int rxvm_channel_process_test_crash_next(
        rxvm_context *context,
        int64_t channel_capability,
        int phase) {
    process_channel *channel = 0;
    rxvm_channel_status status;
    if (phase != 1 && phase != 2) return 0;
    status = rxvm_channel_resolve_provider_state(
            context, channel_capability, RXVM_CHANNEL_PROVIDER_PROCESS,
            (void **)&channel);
    if (status != RXVM_CHANNEL_OK || !channel || channel->is_scope ||
        channel->closed || !channel->shared) return 0;
    process_mutex_lock(&channel->shared->mutex);
    if (channel->shared->test_crash_phase) {
        process_mutex_unlock(&channel->shared->mutex);
        return 0;
    }
    channel->shared->test_crash_phase = phase;
    process_mutex_unlock(&channel->shared->mutex);
    return 1;
}
