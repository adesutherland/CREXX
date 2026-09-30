/* Bounded scratch diagnostic: link-time wrappers count calls from static VM archives. */
#include <pthread.h>
#include <dlfcn.h>
#include <stdatomic.h>
#define main upstream_byte_main
#include "/Users/adrian/CLionProjects/CREXX-hotfix/interpreter/tests/test_rxvmchannel_byte.c"
#undef main
static _Atomic unsigned long creates, joins;
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*entry)(void *), void *arg) {
    int (*real_create)(pthread_t *, const pthread_attr_t *, void *(*)(void *), void *) =
        (int (*)(pthread_t *, const pthread_attr_t *, void *(*)(void *), void *))dlsym(RTLD_NEXT, "pthread_create");
    if (!real_create) abort();
    int rc = real_create(thread, attr, entry, arg);
    if (!rc) atomic_fetch_add(&creates, 1);
    return rc;
}
int pthread_join(pthread_t thread, void **value) {
    int (*real_join)(pthread_t, void **) = (int (*)(pthread_t, void **))dlsym(RTLD_NEXT, "pthread_join");
    if (!real_join) abort();
    int rc = real_join(thread, value);
    if (!rc) atomic_fetch_add(&joins, 1);
    return rc;
}
#define REQUIRE(x) do { if (!(x)) { fprintf(stderr,"join probe failed at %d: %s\n",__LINE__,#x); exit(1); } } while(0)
int main(void) {
    rxvm_runtime *runtime = rxvm_runtime_create();
    rxvm_context *ctx = rxvm_context_create_in_runtime(runtime);
    unsigned char payload[4096]; memset(payload, 0xa5, sizeof(payload));
    bytes config = memory_configuration(3, sizeof(payload));
    bytes write = write_request(payload, sizeof(payload));
    bytes read = one_integer_request("crexx.channel.byte-read", "maximumBytes", sizeof(payload));
    int64_t channel = 0;
    REQUIRE(runtime && ctx);
    REQUIRE(rxvm_channel_open(ctx, 4, 0, config.data, config.length, &channel) == 0);
    REQUIRE(atomic_load(&creates) == 0 && atomic_load(&joins) == 0);
    for (unsigned i = 0; i < 10000; i++) {
        int64_t ticket = 0; int valid = 0;
        rxvm_channel_binary completion = {0};
        const bytes *request = i & 1 ? &read : &write;
        REQUIRE(rxvm_channel_start(ctx, channel, request->data, request->length, 0, &ticket) == 0);
        REQUIRE(rxvm_channel_wait(ctx, channel, 1000000, &completion) == 0);
        REQUIRE(completion_integer_field(&completion, "state", &valid) == 1 && valid);
        REQUIRE(atomic_load(&creates) == i + 1 && atomic_load(&joins) == i);
        REQUIRE(rxvm_channel_release(ctx, channel, ticket) == 0);
        REQUIRE(atomic_load(&joins) == i + 1);
        REQUIRE(rxvm_channel_context_live_tickets(ctx) == 0);
        if (i & 1) {
            const unsigned char *node = 0; size_t length = 0;
            REQUIRE(record_field_node(completion.data, completion.length,"result", &node, &length));
            REQUIRE(length == sizeof(payload) + 12 && node[0] == 7 && memcmp(node + 12, payload, sizeof(payload)) == 0);
        }
        rxvm_channel_binary_free(&completion);
        if ((i + 1) % 1000 == 0) printf("operations=%u creates=%lu joins=%lu live_tickets=0 payload_bytes=4096\n", i + 1, atomic_load(&creates), atomic_load(&joins));
    }
    REQUIRE(rxvm_channel_close(ctx, channel, 1) == 0);
    rxvm_destroy(ctx); rxvm_runtime_destroy(runtime);
    free(config.data); free(write.data); free(read.data);
    puts("PASS: every observed asynchronous request joined during release; saved read payload survived release");
    return 0;
}
