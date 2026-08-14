/*
 * Gate F GF-B09 private provider-registry conformance fixture.
 *
 * The installed provider ABI remains deferred to F2.  This test proves the
 * rebuild-together F1 seam through the same channel capability path used by
 * RXAS: atomic registration, complete operations, channel pinning, safe
 * unregister and deterministic module release.
 */

#include "../../inc/rxvm.h"
#include "rxvmchannel.h"
#include "rxvmintp.h"
#include "rxvmworker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FAKE_PROVIDER_TYPE INT64_C(65536)

static const unsigned char null_document[] = {
    'R', 'X', 'C', 'V', 1, 0, 0, 0,
    28, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

typedef struct fake_module {
    size_t retains;
    size_t releases;
    size_t opens;
    size_t starts;
    size_t closes;
} fake_module;

typedef struct fake_channel {
    fake_module *module;
    uint64_t generation;
    uint64_t next_order;
    int closed;
} fake_channel;

typedef struct fake_request {
    uint64_t order;
    int terminal;
} fake_request;

static int failures;

#define CHECK(condition, message)                                             \
    do {                                                                      \
        if (!(condition)) {                                                   \
            fprintf(stderr, "FAIL: %s\n", (message));                        \
            failures++;                                                       \
        }                                                                     \
    } while (0)

static int is_null_document(const void *data, size_t length) {
    return data && length == sizeof(null_document) &&
           memcmp(data, null_document, sizeof(null_document)) == 0;
}

static void fake_module_retain(void *module_state) {
    ((fake_module *)module_state)->retains++;
}

static void fake_module_release(void *module_state) {
    ((fake_module *)module_state)->releases++;
}

static rxvm_channel_status fake_open(
        void *module_state,
        rxvm_context *context,
        const void *configuration,
        size_t configuration_length,
        void **channel_state_out) {
    fake_module *module = (fake_module *)module_state;
    fake_channel *channel;
    if (channel_state_out) *channel_state_out = 0;
    if (!module || !context || !channel_state_out ||
        !is_null_document(configuration, configuration_length)) {
        return RXVM_CHANNEL_INVALID_CONFIGURATION;
    }
    channel = (fake_channel *)calloc(1u, sizeof(*channel));
    if (!channel) return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    channel->module = module;
    module->opens++;
    *channel_state_out = channel;
    return RXVM_CHANNEL_OK;
}

static rxvm_channel_status fake_start(
        void *channel_state,
        const void *envelope,
        size_t envelope_length,
        int64_t wait_microseconds,
        void **request_state_out) {
    fake_channel *channel = (fake_channel *)channel_state;
    fake_request *request;
    if (request_state_out) *request_state_out = 0;
    if (!channel || channel->closed || !request_state_out ||
        wait_microseconds < -1 ||
        !is_null_document(envelope, envelope_length)) {
        return RXVM_CHANNEL_INVALID_ARGUMENT;
    }
    request = (fake_request *)calloc(1u, sizeof(*request));
    if (!request) return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    request->terminal = 1;
    request->order = ++channel->next_order;
    channel->generation++;
    channel->module->starts++;
    *request_state_out = request;
    return RXVM_CHANNEL_OK;
}

static int fake_terminal_snapshot(
        void *channel_state,
        void *request_state,
        rxvm_channel_provider_completion *completion_out,
        uint64_t *completion_order_out) {
    fake_channel *channel = (fake_channel *)channel_state;
    fake_request *request = (fake_request *)request_state;
    if (completion_out) memset(completion_out, 0, sizeof(*completion_out));
    if (completion_order_out) *completion_order_out = 0u;
    if (!channel || !request || !request->terminal) return 0;
    if (completion_out) {
        completion_out->state = 1;
        completion_out->message = "";
    }
    if (completion_order_out) *completion_order_out = request->order;
    return 1;
}

static uint64_t fake_completion_generation(void *channel_state) {
    fake_channel *channel = (fake_channel *)channel_state;
    return channel ? channel->generation : 0u;
}

static int fake_completion_wait(
        void *channel_state,
        uint64_t observed_generation,
        int64_t wait_microseconds) {
    fake_channel *channel = (fake_channel *)channel_state;
    if (!channel || wait_microseconds < -1) return -1;
    return channel->generation != observed_generation ? 1 : 0;
}

static rxvm_channel_status fake_cancel(
        void *channel_state,
        void *request_state,
        const void *reason,
        size_t reason_length) {
    fake_request *request = (fake_request *)request_state;
    (void)channel_state;
    if (!request || (reason && !is_null_document(reason, reason_length))) {
        return RXVM_CHANNEL_INVALID_ARGUMENT;
    }
    return request->terminal
        ? RXVM_CHANNEL_ALREADY_TERMINAL : RXVM_CHANNEL_OK;
}

static rxvm_channel_status fake_close(void *channel_state, int64_t mode) {
    fake_channel *channel = (fake_channel *)channel_state;
    if (!channel || channel->closed || (mode != 1 && mode != 2)) {
        return RXVM_CHANNEL_CLOSED;
    }
    channel->closed = 1;
    channel->module->closes++;
    return RXVM_CHANNEL_OK;
}

static rxvm_channel_status fake_request_destroy(
        void *channel_state,
        void *request_state) {
    (void)channel_state;
    free(request_state);
    return RXVM_CHANNEL_OK;
}

static void fake_channel_destroy(void *channel_state) {
    fake_channel *channel = (fake_channel *)channel_state;
    if (!channel) return;
    if (!channel->closed) abort();
    free(channel);
}

static rxvm_channel_provider_descriptor fake_descriptor(
        fake_module *module,
        int64_t type,
        const char *name) {
    rxvm_channel_provider_descriptor descriptor;
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.type = type;
    descriptor.name = name;
    descriptor.abi_version = RXVM_CHANNEL_PROVIDER_ABI_VERSION;
    descriptor.configuration_version_min = 1u;
    descriptor.configuration_version_max = 1u;
    descriptor.capabilities = UINT64_C(0x0001);
    descriptor.operations.open = fake_open;
    descriptor.operations.start = fake_start;
    descriptor.operations.terminal_snapshot = fake_terminal_snapshot;
    descriptor.operations.completion_generation = fake_completion_generation;
    descriptor.operations.completion_wait = fake_completion_wait;
    descriptor.operations.cancel = fake_cancel;
    descriptor.operations.close = fake_close;
    descriptor.operations.request_destroy = fake_request_destroy;
    descriptor.operations.channel_destroy = fake_channel_destroy;
    descriptor.module_state = module;
    descriptor.module_retain = fake_module_retain;
    descriptor.module_release = fake_module_release;
    return descriptor;
}

int main(void) {
    fake_module module = {0};
    rxvm_channel_provider_descriptor descriptor = fake_descriptor(
            &module, FAKE_PROVIDER_TYPE, "example.test.fake-channel");
    rxvm_channel_provider_descriptor duplicate_name = fake_descriptor(
            &module, FAKE_PROVIDER_TYPE + 1,
            "example.test.fake-channel");
    rxvm_channel_provider_descriptor invalid = descriptor;
    rxvm_runtime *runtime = rxvm_runtime_create();
    rxvm_context *context = 0;
    rxvm_channel_binary completion = {0};
    int64_t channel = 0;
    int64_t ticket = 0;

    CHECK(runtime != 0, "runtime creates for provider registry");
    if (!runtime) return 1;
    context = rxvm_context_create_in_runtime(runtime);
    CHECK(context != 0, "controller context attaches to provider runtime");
    if (!context) goto cleanup;

    invalid.operations.close = 0;
    CHECK(rxvm_channel_provider_register(runtime, &invalid) ==
              RXVM_CHANNEL_PROVIDER_REGISTRATION_INVALID,
          "incomplete operation table fails atomically");
    CHECK(module.retains == 0u && module.releases == 0u,
          "invalid registration does not pin module");

    CHECK(rxvm_channel_provider_register(runtime, &descriptor) ==
              RXVM_CHANNEL_PROVIDER_REGISTRATION_OK,
          "extension provider registers");
    CHECK(rxvm_channel_provider_register(runtime, &descriptor) ==
              RXVM_CHANNEL_PROVIDER_REGISTRATION_DUPLICATE_TYPE,
          "duplicate provider type is rejected");
    CHECK(rxvm_channel_provider_register(runtime, &duplicate_name) ==
              RXVM_CHANNEL_PROVIDER_REGISTRATION_DUPLICATE_NAME,
          "duplicate canonical provider name is rejected");
    CHECK(module.retains == 3u && module.releases == 2u,
          "failed duplicate candidates release their temporary module pins");

    CHECK(rxvm_channel_open(
              context, FAKE_PROVIDER_TYPE, 1,
              null_document, sizeof(null_document), &channel) ==
              RXVM_CHANNEL_OK && channel != 0,
          "registered provider opens through common capability path");
    CHECK(rxvm_channel_provider_unregister(runtime, FAKE_PROVIDER_TYPE) ==
              RXVM_CHANNEL_PROVIDER_REGISTRATION_PINNED,
          "open channel prevents provider unload");
    CHECK(module.releases == 2u,
          "pinned unregister does not release the registered module");

    CHECK(rxvm_channel_start(
              context, channel, null_document, sizeof(null_document),
              0, &ticket) == RXVM_CHANNEL_OK && ticket != 0,
          "fake provider starts through common request path");
    CHECK(rxvm_channel_wait(context, channel, 0, &completion) ==
              RXVM_CHANNEL_OK && completion.length > sizeof(null_document) &&
              memcmp(completion.data, "RXCV", 4u) == 0,
          "fake provider completion is core-materialized canonical RXCV");
    rxvm_channel_binary_free(&completion);
    CHECK(rxvm_channel_cancel(
              context, channel, ticket,
              null_document, sizeof(null_document)) ==
              RXVM_CHANNEL_ALREADY_TERMINAL,
          "fake provider preserves one terminal completion");

    CHECK(rxvm_channel_close(context, channel, 1) == RXVM_CHANNEL_OK,
          "fake provider drains and closes");
    CHECK(rxvm_channel_close(context, channel, 1) ==
              RXVM_CHANNEL_STALE_CAPABILITY,
          "closed fake-provider capability is stale");
    CHECK(module.opens == 1u && module.starts == 1u && module.closes == 1u,
          "fake provider receives each common lifecycle operation once");
    CHECK(rxvm_channel_provider_unregister(runtime, FAKE_PROVIDER_TYPE) ==
              RXVM_CHANNEL_PROVIDER_REGISTRATION_OK,
          "provider unregisters after its final channel unpins");
    CHECK(module.retains == 3u && module.releases == 3u,
          "successful unregister releases the registered module pin");
    channel = 0;
    CHECK(rxvm_channel_open(
              context, FAKE_PROVIDER_TYPE, 1,
              null_document, sizeof(null_document), &channel) ==
              RXVM_CHANNEL_INVALID_PROVIDER && channel == 0,
          "unregistered extension code is invalid and failure-atomic");
    CHECK(rxvm_channel_provider_unregister(runtime, FAKE_PROVIDER_TYPE) ==
              RXVM_CHANNEL_PROVIDER_REGISTRATION_NOT_FOUND,
          "unregistered provider cannot be removed twice");

cleanup:
    if (completion.data) rxvm_channel_binary_free(&completion);
    if (context) rxvm_destroy(context);
    CHECK(rxvm_runtime_worker_count(runtime) == 0u,
          "provider test context unregisters its worker");
    CHECK(rxvm_runtime_destroy(runtime) == 0u,
          "provider registry runtime destroys without allocator leaks");
    CHECK(module.retains == module.releases,
          "provider module retain/release accounting is balanced");
    if (failures) {
        fprintf(stderr, "%d provider-registry check(s) failed\n", failures);
        return 1;
    }
    puts("PASS: Gate F GF-B09 private provider registry");
    return 0;
}
