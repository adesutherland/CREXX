/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxvmprocessworker.h"

#include "rxvmchannel_internal.h"
#include "rxvmexecutor.h"
#include "rxvmprocessprotocol.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#define process_close _close
#define process_read _read
#define process_write _write
#else
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#define process_close close
#define process_read read
#define process_write write
#endif

static int worker_write_all(int descriptor,
                            const unsigned char *data,
                            size_t length) {
    size_t offset = 0u;
#if !defined(_WIN32)
    sigset_t blocked;
    sigset_t previous;
    int keep_blocked = 0;
    sigemptyset(&blocked);
    sigaddset(&blocked, SIGPIPE);
    /* Protocol transport loss is data, not authority to terminate the worker
     * process. Block SIGPIPE only around these control-pipe writes. If EPIPE
     * is generated, leave it blocked until the worker immediately exits so a
     * pending signal cannot fire while unwinding the failed protocol. */
    if (pthread_sigmask(SIG_BLOCK, &blocked, &previous) != 0) return 0;
#endif
    while (offset < length) {
#if defined(_WIN32)
        unsigned int amount = length - offset > INT_MAX
                ? INT_MAX : (unsigned int)(length - offset);
        int written = process_write(descriptor, data + offset, amount);
#else
        ssize_t written = process_write(descriptor, data + offset,
                                        length - offset);
#endif
        if (written < 0) {
            if (errno == EINTR) continue;
#if !defined(_WIN32)
            if (errno == EPIPE) keep_blocked = 1;
#endif
            break;
        }
        if (!written) break;
        offset += (size_t)written;
    }
#if !defined(_WIN32)
    if (!keep_blocked) {
        (void)pthread_sigmask(SIG_SETMASK, &previous, 0);
    }
#endif
    return offset == length;
}

static int worker_read_all(int descriptor,
                           unsigned char *data,
                           size_t length) {
    size_t offset = 0u;
    while (offset < length) {
#if defined(_WIN32)
        unsigned int amount = length - offset > INT_MAX
                ? INT_MAX : (unsigned int)(length - offset);
        int received = process_read(descriptor, data + offset, amount);
#else
        ssize_t received = process_read(descriptor, data + offset,
                                        length - offset);
#endif
        if (received < 0) {
            if (errno == EINTR) continue;
            return 0;
        }
        if (!received) return 0;
        offset += (size_t)received;
    }
    return 1;
}

static int worker_send_frame(int descriptor,
                             uint16_t type,
                             uint64_t request_id,
                             const void *payload,
                             size_t payload_length) {
    unsigned char header[RXVM_PROCESS_PROTOCOL_HEADER_SIZE];
    rxvm_process_frame_header(header, type, request_id, payload_length);
    return worker_write_all(descriptor, header, sizeof(header)) &&
           (!payload_length || worker_write_all(
                   descriptor, (const unsigned char *)payload,
                   payload_length));
}

static int worker_receive_frame(int descriptor, rxvm_process_frame *frame) {
    unsigned char header[RXVM_PROCESS_PROTOCOL_HEADER_SIZE];
    memset(frame, 0, sizeof(*frame));
    if (!worker_read_all(descriptor, header, sizeof(header)) ||
        !rxvm_process_frame_header_parse(
                header, &frame->type, &frame->request_id,
                &frame->payload_length)) return 0;
    if (frame->payload_length) {
        frame->payload = (unsigned char *)malloc(frame->payload_length);
        if (!frame->payload || !worker_read_all(
                descriptor, frame->payload, frame->payload_length)) {
            rxvm_process_frame_free(frame);
            return 0;
        }
    }
    return 1;
}

static int worker_input_ready(int descriptor, unsigned int milliseconds) {
#if defined(_WIN32)
    HANDLE handle = (HANDLE)_get_osfhandle(descriptor);
    DWORD available = 0u;
    unsigned int waited = 0u;
    while (waited <= milliseconds) {
        if (!PeekNamedPipe(handle, 0, 0, 0, &available, 0)) return -1;
        if (available) return 1;
        if (waited == milliseconds) return 0;
        Sleep(1u);
        waited++;
    }
    return 0;
#else
    struct pollfd item;
    int result;
    item.fd = descriptor;
    item.events = POLLIN;
    item.revents = 0;
    do {
        result = poll(&item, 1, (int)milliseconds);
    } while (result < 0 && errno == EINTR);
    if (result <= 0) return result;
    return (item.revents & POLLIN) ? 1 : -1;
#endif
}

static int worker_redirect_standard_streams(int *input_out, int *output_out) {
    int input_descriptor;
    int output_descriptor;
    int null_descriptor;
#if defined(_WIN32)
    input_descriptor = _dup(0);
    output_descriptor = _dup(1);
    null_descriptor = _open("NUL", _O_RDWR | _O_BINARY);
    if (input_descriptor >= 0) _setmode(input_descriptor, _O_BINARY);
    if (output_descriptor >= 0) _setmode(output_descriptor, _O_BINARY);
    if (null_descriptor >= 0) {
        (void)_dup2(null_descriptor, 0);
        (void)_dup2(null_descriptor, 1);
        (void)_dup2(null_descriptor, 2);
    }
#else
    input_descriptor = dup(STDIN_FILENO);
    output_descriptor = dup(STDOUT_FILENO);
    null_descriptor = open("/dev/null", O_RDWR);
    if (null_descriptor >= 0) {
        (void)dup2(null_descriptor, STDIN_FILENO);
        (void)dup2(null_descriptor, STDOUT_FILENO);
        (void)dup2(null_descriptor, STDERR_FILENO);
    }
#endif
    if (null_descriptor >= 0) process_close(null_descriptor);
    if (input_descriptor < 0 || output_descriptor < 0 ||
        null_descriptor < 0) {
        if (input_descriptor >= 0) process_close(input_descriptor);
        if (output_descriptor >= 0) process_close(output_descriptor);
        return 0;
    }
    *input_out = input_descriptor;
    *output_out = output_descriptor;
    return 1;
}

static int worker_execute(int input_descriptor,
                          int output_descriptor,
                          const char *program_path,
                          const rxvm_process_frame *invoke_frame) {
    rxvm_channel_task_invoke invoke;
    rxvm_executor *executor = 0;
    rxvm_executor_request *request = 0;
    rxvm_executor_result executor_result;
    rxvm_executor_completion completion;
    unsigned char *completion_document = 0;
    size_t completion_length = 0u;
    int okay = 0;

    if (rxvm_channel_parse_task_invoke(
            invoke_frame->payload, invoke_frame->payload_length,
            &invoke) != RXVM_CHANNEL_OK) return 0;
    executor = rxvm_executor_create(
            program_path, 1u, 1u, &executor_result);
    if (!executor) goto cleanup;
    executor_result = rxvm_executor_submit_task_binding_registers_result(
            executor, 0u, invoke.task_binding,
            invoke.factory_argument_count, invoke.factory_arguments,
            invoke.argument_count, invoke.arguments,
            (invoke.target_kind == 1 ||
             (invoke.target_kind == 2 &&
              (invoke.task_binding[76] || invoke.task_binding[77] ||
               invoke.task_binding[78] || invoke.task_binding[79])))
                    ? RXVM_EXECUTOR_REGISTER_NONE
                    : (invoke.target_kind == 3
                       ? RXVM_EXECUTOR_REGISTER_CHANNEL_VALUE
                       : RXVM_EXECUTOR_REGISTER_BINARY),
            &request);
    if (executor_result != RXVM_EXECUTOR_OK || !request) goto cleanup;
    if (!worker_send_frame(output_descriptor, RXVM_PROCESS_FRAME_STARTED,
                           invoke_frame->request_id, 0, 0u)) goto cleanup;

    for (;;) {
        rxvm_executor_request_state state =
                rxvm_executor_request_state_get(request);
        int ready;
        if (state >= RXVM_EXECUTOR_REQUEST_COMPLETED) break;
        ready = worker_input_ready(input_descriptor, 10u);
        if (ready < 0) goto cleanup;
        if (ready) {
            rxvm_process_frame control;
            if (!worker_receive_frame(input_descriptor, &control)) goto cleanup;
            if (control.type == RXVM_PROCESS_FRAME_CANCEL &&
                control.request_id == invoke_frame->request_id &&
                !control.payload_length) {
                (void)rxvm_executor_cancel(request);
            } else if (control.type == RXVM_PROCESS_FRAME_SHUTDOWN &&
                       !control.payload_length) {
                (void)rxvm_executor_cancel(request);
            } else {
                rxvm_process_frame_free(&control);
                goto cleanup;
            }
            rxvm_process_frame_free(&control);
        }
    }
    (void)rxvm_executor_request_wait_completion(request, &completion);
    if (rxvm_channel_encode_process_completion(
            &completion, &completion_document,
            &completion_length) != RXVM_CHANNEL_OK) goto cleanup;
    okay = worker_send_frame(
            output_descriptor, RXVM_PROCESS_FRAME_RESULT,
            invoke_frame->request_id,
            completion_document, completion_length);

cleanup:
    free(completion_document);
    if (request) {
        if (rxvm_executor_request_state_get(request) <
                RXVM_EXECUTOR_REQUEST_COMPLETED) {
            (void)rxvm_executor_cancel(request);
            (void)rxvm_executor_request_wait_completion(request, 0);
        }
        (void)rxvm_executor_request_destroy(request);
    }
    if (executor) (void)rxvm_executor_destroy(executor);
    rxvm_channel_task_invoke_free(&invoke);
    return okay;
}

int rxvm_process_worker_main(const char *program_path) {
    int input_descriptor = -1;
    int output_descriptor = -1;
    int result = 2;
    if (!program_path || !program_path[0] ||
        !worker_redirect_standard_streams(
                &input_descriptor, &output_descriptor)) return 2;
    if (!worker_send_frame(output_descriptor, RXVM_PROCESS_FRAME_READY,
                           0u, 0, 0u)) goto cleanup;
    for (;;) {
        rxvm_process_frame frame;
        if (!worker_receive_frame(input_descriptor, &frame)) break;
        if (frame.type == RXVM_PROCESS_FRAME_SHUTDOWN &&
            !frame.request_id && !frame.payload_length) {
            rxvm_process_frame_free(&frame);
            result = 0;
            break;
        }
        if (frame.type != RXVM_PROCESS_FRAME_INVOKE || !frame.request_id ||
            !frame.payload_length ||
            !worker_execute(input_descriptor, output_descriptor,
                            program_path, &frame)) {
            rxvm_process_frame_free(&frame);
            break;
        }
        rxvm_process_frame_free(&frame);
    }

cleanup:
    process_close(input_descriptor);
    process_close(output_descriptor);
    return result;
}
