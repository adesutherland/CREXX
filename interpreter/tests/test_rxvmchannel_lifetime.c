/* Completed-request release regression. Reuse the canonical RXCV fixture
 * encoders; the renamed upstream entry point is not run by this test. */
#define main byte_provider_conformance_main
#include "test_rxvmchannel_byte.c"
#undef main

/* Fail promptly, avoiding unbounded follow-on output after a broken lifecycle. */
#define REQUIRE(condition) do { if (!(condition)) { \
    fprintf(stderr, "lifetime failure line %d: %s\n", __LINE__, #condition); \
    exit(1); } } while (0)

static int64_t submit(rxvm_context *ctx, int64_t channel, const bytes *request) {
    int64_t ticket = 0;
    REQUIRE(rxvm_channel_start(ctx, channel, request->data, request->length,
                               0, &ticket) == RXVM_CHANNEL_OK);
    return ticket;
}

static void observe(rxvm_context *ctx, int64_t channel) {
    rxvm_channel_binary completion = {0};
    REQUIRE(rxvm_channel_wait(ctx, channel, 1000000, &completion) == RXVM_CHANNEL_OK);
    rxvm_channel_binary_free(&completion);
}

int main(void) {
    rxvm_runtime *runtime = rxvm_runtime_create();
    rxvm_context *ctx = rxvm_context_create_in_runtime(runtime);
    bytes config = memory_configuration(3, 8);
    bytes half = one_integer_request("crexx.channel.byte-half-close", "direction", 2);
    bytes read = one_integer_request("crexx.channel.byte-read", "maximumBytes", 1);
    bytes write = write_request("x", 1);
    int64_t a = 0, b = 0, ticket, old, spare;
    rxvm_channel_binary saved = {0}, none = {0};
    size_t i;
    int valid = 0;
    int64_t *retained = calloc(65535u, sizeof(*retained));
    REQUIRE(runtime && ctx && retained);
    REQUIRE(rxvm_channel_open(ctx, 4, 0, config.data, config.length, &a) == 0);
    REQUIRE(rxvm_channel_open(ctx, 4, 0, config.data, config.length, &b) == 0);
    ticket = submit(ctx, a, &half);
    REQUIRE(rxvm_channel_release(ctx, a, ticket) == RXVM_CHANNEL_WOULD_BLOCK);
    REQUIRE(rxvm_channel_release(ctx, b, ticket) == RXVM_CHANNEL_UNKNOWN_TICKET);
    REQUIRE(rxvm_channel_wait(ctx, a, 0, &saved) == 0);
    REQUIRE(rxvm_channel_cancel(ctx, a, ticket, null_document, sizeof(null_document)) ==
            RXVM_CHANNEL_ALREADY_TERMINAL);
    REQUIRE(rxvm_channel_wait(ctx, a, 0, &none) == RXVM_CHANNEL_WOULD_BLOCK);
    old = ticket;
    REQUIRE(rxvm_channel_release(ctx, a, ticket) == 0);
    REQUIRE(rxvm_channel_release(ctx, a, old) == RXVM_CHANNEL_STALE_CAPABILITY);
    REQUIRE(completion_integer_field(&saved, "state", &valid) == 1 && valid);

    /* At least three generation-retirement boundaries, with one live request. */
    for (i = 0; i < 200000u; i++) {
        ticket = submit(ctx, a, &half);
        observe(ctx, a);
        REQUIRE(rxvm_channel_release(ctx, a, ticket) == 0);
    }
    REQUIRE(rxvm_channel_context_live_tickets(ctx) == 0);
    REQUIRE(rxvm_channel_release(ctx, a, old) == RXVM_CHANNEL_STALE_CAPABILITY);
    /* Retired slots cannot be used to reach the full capacity in this context.
     * Start a fresh execution for the precise simultaneous-ticket limit. */
    REQUIRE(rxvm_channel_close(ctx, a, 1) == 0);
    REQUIRE(rxvm_channel_close(ctx, b, 2) == 0);
    rxvm_channel_binary_free(&saved);
    rxvm_destroy(ctx);
    ctx = rxvm_context_create_in_runtime(runtime);
    REQUIRE(rxvm_channel_open(ctx, 4, 0, config.data, config.length, &a) == 0);
    REQUIRE(rxvm_channel_open(ctx, 4, 0, config.data, config.length, &b) == 0);
    REQUIRE(rxvm_channel_release(ctx, a, old) == RXVM_CHANNEL_WRONG_OWNER);
    for (i = 0; i < 65535u; i++) {
        int64_t channel = i & 1u ? b : a;
        retained[i] = submit(ctx, channel, &half);
        observe(ctx, channel);
    }
    REQUIRE(rxvm_channel_start(ctx, b, half.data, half.length, 0, &spare) ==
            RXVM_CHANNEL_RESOURCE_EXHAUSTED && spare == 0);
    /* Non-head, alternating removals followed by reuse and mixed-channel close. */
    for (i = 0; i < 65535u; i += 2u) {
        REQUIRE(rxvm_channel_release(ctx, a, retained[i]) == 0);
    }
    REQUIRE(rxvm_channel_context_live_tickets(ctx) == 32767u);
    ticket = submit(ctx, b, &half);
    observe(ctx, b);
    REQUIRE(rxvm_channel_release(ctx, b, ticket) == 0);
    REQUIRE(rxvm_channel_close(ctx, a, 1) == 0);
    REQUIRE(rxvm_channel_close(ctx, b, 2) == 0);
    REQUIRE(rxvm_channel_context_live_tickets(ctx) == 0);

    REQUIRE(rxvm_channel_open(ctx, 4, 0, config.data, config.length, &a) == 0);
    for (i = 0; i < 128u; i++) {
        ticket = submit(ctx, a, &write); observe(ctx, a);
        REQUIRE(rxvm_channel_release(ctx, a, ticket) == 0);
        ticket = submit(ctx, a, &read); observe(ctx, a);
        REQUIRE(rxvm_channel_release(ctx, a, ticket) == 0);
    }
    /* An empty nonblocking read publishes a terminal miss; cancellation may
     * win or find it terminal, but observation must precede release. */
    ticket = submit(ctx, a, &read);
    REQUIRE(rxvm_channel_release(ctx, a, ticket) == RXVM_CHANNEL_WOULD_BLOCK);
    {
        rxvm_channel_status status = rxvm_channel_cancel(
                ctx, a, ticket, null_document, sizeof(null_document));
        REQUIRE(status == 0 || status == RXVM_CHANNEL_ALREADY_TERMINAL);
    }
    observe(ctx, a);
    REQUIRE(rxvm_channel_release(ctx, a, ticket) == 0);
    REQUIRE(rxvm_channel_context_live_tickets(ctx) == 0);
    /* Teardown still owns abandoned/unobserved requests. */
    (void)submit(ctx, a, &half);
    rxvm_destroy(ctx);
    rxvm_runtime_destroy(runtime);
    free(retained); free(config.data); free(half.data); free(read.data); free(write.data);
    puts("channel lifetime: 200000 released, capacity recovery, async I/O and stale authority passed");
    return 0;
}
