/* Gate F F1e process crash/disconnect and replacement conformance. */

#include "../../inc/rxvm.h"
#include "platform.h"
#include "rxvmchannel.h"
#include "rxvmchannel_process.h"
#include "rxvmintp.h"
#include "rxvmworker.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char pool_hex[] =
    "5258435601000000a3000000000000000900000087000000000000001f00000000000000"
    "63726578782e6368616e6e656c2e70726f636573732d7461736b2d706f6f6c01000000"
    "0200000000000000110000000000000061646d697373696f6e4361706163697479030000"
    "00080000000000000002000000000000000b00000000000000776f726b6572436f756e"
    "740300000008000000000000000100000000000000";

static const char invoke_hex[] =
    "5258435601000000b701000000000000090000009b010000000000001900000000000000"
    "63726578782e6368616e6e656c2e7461736b2d696e766f6b650100000002000000000000"
    "000900000000000000617267756d656e7473080000001600000000000000010000000000"
    "000006000000020000000000000034310600000000000000746172676574090000002101"
    "000000000000190000000000000063726578782e6368616e6e656c2e7461736b2d746172"
    "6765740100000005000000000000000a0000000000000063616c6c61626c654964030000"
    "00080000000000000000000000000000001000000000000000666163746f727941726775"
    "6d656e747308000000080000000000000000000000000000000b00000000000000696d61"
    "676544696765737407000000200000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000004000000000000006b696e640300000008000000"
    "0000000001000000000000000f000000000000007369676e617475726544696765737407"
    "000000200000000000000000000000000000000000000000000000000000000000000000"
    "00000000000000";

static const char scope_hex[] =
    "5258435601000200d00000000000000009000000b4000000000000001800000000000000"
    "63726578782e6368616e6e656c2e7461736b2d73636f7065010000000300000000000000"
    "0d000000000000006661696c757265506f6c696379030000000800000000000000010000"
    "00000000000400000000000000706f6f6c0b000000180000000000000000000000000000"
    "0000000000000000000100000000000000130000000000000074696d656f75744d696372"
    "6f7365636f6e64730300000008000000000000008813000000000000";

static int failures;

#define CHECK(condition, message)                                             \
    do {                                                                      \
        if (!(condition)) {                                                   \
            fprintf(stderr, "FAIL: %s\n", (message));                        \
            failures++;                                                       \
        }                                                                     \
    } while (0)

static int hex_digit(char digit) {
    if (digit >= '0' && digit <= '9') return digit - '0';
    digit = (char)tolower((unsigned char)digit);
    return digit >= 'a' && digit <= 'f' ? digit - 'a' + 10 : -1;
}

static unsigned char *decode_hex(const char *hex, size_t *length_out) {
    size_t length = strlen(hex);
    unsigned char *bytes;
    size_t index;
    if (length_out) *length_out = 0u;
    if (!length_out || (length & 1u)) return 0;
    bytes = (unsigned char *)malloc(length / 2u);
    if (!bytes) return 0;
    for (index = 0u; index < length; index += 2u) {
        int high = hex_digit(hex[index]);
        int low = hex_digit(hex[index + 1u]);
        if (high < 0 || low < 0) {
            free(bytes);
            return 0;
        }
        bytes[index / 2u] = (unsigned char)((high << 4) | low);
    }
    *length_out = length / 2u;
    return bytes;
}

static uint64_t read_u64(const unsigned char *bytes) {
    uint64_t value = 0u;
    unsigned int index;
    for (index = 0u; index < 8u; index++) {
        value |= (uint64_t)bytes[index] << (index * 8u);
    }
    return value;
}

static void write_u64(unsigned char *bytes, uint64_t value) {
    unsigned int index;
    for (index = 0u; index < 8u; index++) {
        bytes[index] = (unsigned char)(value >> (index * 8u));
    }
}

static int64_t completion_state(
        const rxvm_channel_binary *completion) {
    static const unsigned char field[] = {
        5, 0, 0, 0, 0, 0, 0, 0, 's', 't', 'a', 't', 'e',
        3, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0
    };
    size_t index;
    if (!completion || !completion->data) return -1;
    for (index = 0u; index + sizeof(field) + 8u <= completion->length;
         index++) {
        if (!memcmp(completion->data + index, field, sizeof(field))) {
            uint64_t bits = read_u64(
                    completion->data + index + sizeof(field));
            int64_t state;
            memcpy(&state, &bits, sizeof(state));
            return state;
        }
    }
    return -1;
}

static int run_one(rxvm_context *context,
                   int64_t channel,
                   const unsigned char *invoke,
                   size_t invoke_length,
                   int expected_state,
                   const char *description) {
    int64_t ticket = 0;
    rxvm_channel_binary completion = {0};
    rxvm_channel_status status = rxvm_channel_start(
            context, channel, invoke, invoke_length, 0, &ticket);
    if (status != RXVM_CHANNEL_OK || !ticket) {
        fprintf(stderr, "FAIL: %s: start status=%d ticket=%lld\n",
                description, (int)status, (long long)ticket);
        failures++;
    }
    if (status != RXVM_CHANNEL_OK) return 0;
    status = rxvm_channel_wait(context, channel, -1, &completion);
    if (status != RXVM_CHANNEL_OK) {
        fprintf(stderr, "FAIL: %s: wait status=%d\n",
                description, (int)status);
        failures++;
    }
    if (status == RXVM_CHANNEL_OK) {
        int64_t actual_state = completion_state(&completion);
        if (actual_state != expected_state) {
            fprintf(stderr,
                    "FAIL: %s: state=%lld expected=%d\n",
                    description, (long long)actual_state, expected_state);
            failures++;
        }
    }
    rxvm_channel_binary_free(&completion);
    return status == RXVM_CHANNEL_OK;
}

int main(int argc, char **argv) {
    rxvm_runtime *runtime = 0;
    rxvm_context *context = 0;
    unsigned char *pool = 0;
    unsigned char *invoke = 0;
    unsigned char *scope = 0;
    size_t pool_length = 0u;
    size_t invoke_length = 0u;
    size_t scope_length = 0u;
    int64_t channel = 0;
    int64_t scope_channel = 0;

    if (argc != 2) {
        fprintf(stderr, "usage: %s process-fixture.rxbin\n", argv[0]);
        return 2;
    }
    platform_install_signal_handlers();
    pool = decode_hex(pool_hex, &pool_length);
    invoke = decode_hex(invoke_hex, &invoke_length);
    scope = decode_hex(scope_hex, &scope_length);
    CHECK(pool && invoke && scope,
          "decode canonical process conformance documents");
    runtime = rxvm_runtime_create();
    CHECK(runtime != 0, "create process-provider test runtime");
    if (!runtime || !pool || !invoke || !scope) goto cleanup;
    context = rxvm_context_create_in_runtime(runtime);
    CHECK(context != 0, "create process-provider controller context");
    if (!context) goto cleanup;
    CHECK(rxldmod(context, argv[1]) > 0,
          "load process-provider semantic image");
    CHECK(rxvm_channel_open(
              context, RXVM_CHANNEL_PROVIDER_PROCESS,
              (int64_t)RXVM_CHANNEL_PROCESS_CAPABILITIES,
              pool, pool_length, &channel) == RXVM_CHANNEL_OK && channel != 0,
          "open isolated process pool for crash conformance");
    if (!channel) goto cleanup;

    CHECK(rxvm_channel_process_test_crash_next(context, channel, 1),
          "arm before-start transport loss");
    (void)run_one(context, channel, invoke, invoke_length, 7,
                  "pre-start process loss is TRANSPORT_LOST");
    (void)run_one(context, channel, invoke, invoke_length, 1,
                  "worker replacement succeeds after pre-start loss");

    /* Callable id 1 is the fixture's nonterminating body, so the child cannot
     * legitimately publish RESULT before the after-STARTED sever occurs. */
    write_u64(invoke + 225u, 1u);
    CHECK(rxvm_channel_process_test_crash_next(context, channel, 2),
          "arm after-start transport loss");
    (void)run_one(context, channel, invoke, invoke_length, 8,
                  "post-start process loss is UNKNOWN_OUTCOME");
    write_u64(invoke + 225u, 0u);
    (void)run_one(context, channel, invoke, invoke_length, 1,
                  "worker replacement succeeds after unknown outcome");

    CHECK(rxvm_channel_close(context, channel, 1) == RXVM_CHANNEL_OK,
          "close crash-conformance process pool");
    channel = 0;

    /* Fail-fast must send a cooperative CANCEL frame to a running sibling,
     * not merely wait out the provider's hard-termination grace. */
    write_u64(pool + pool_length - 8u, 2u);
    CHECK(rxvm_channel_open(
              context, RXVM_CHANNEL_PROVIDER_PROCESS,
              (int64_t)RXVM_CHANNEL_PROCESS_CAPABILITIES,
              pool, pool_length, &channel) == RXVM_CHANNEL_OK && channel != 0,
          "open two-worker isolated process pool for fail-fast");
    if (!channel) goto cleanup;
    write_u64(scope + 137u, (uint64_t)channel);
    write_u64(scope + 153u, RXVM_CHANNEL_PROVIDER_PROCESS);
    write_u64(scope + scope_length - 8u, UINT64_C(2000000));
    CHECK(rxvm_channel_open(
              context, RXVM_CHANNEL_PROVIDER_PROCESS,
              (int64_t)RXVM_CHANNEL_PROCESS_CAPABILITIES,
              scope, scope_length, &scope_channel) == RXVM_CHANNEL_OK &&
          scope_channel != 0,
          "open isolated fail-fast scope");
    if (scope_channel) {
        int64_t loop_ticket = 0;
        int64_t missing_ticket = 0;
        rxvm_channel_binary completion = {0};
        write_u64(invoke + 225u, 1u);
        CHECK(rxvm_channel_start(
                  context, scope_channel, invoke, invoke_length, 0,
                  &loop_ticket) == RXVM_CHANNEL_OK && loop_ticket != 0,
              "start running fail-fast sibling");
        write_u64(invoke + 225u, 99u);
        CHECK(rxvm_channel_start(
                  context, scope_channel, invoke, invoke_length, 0,
                  &missing_ticket) == RXVM_CHANNEL_OK && missing_ticket != 0,
              "start failing task in fail-fast scope");
        CHECK(rxvm_channel_wait(
                  context, scope_channel, -1, &completion) ==
                      RXVM_CHANNEL_OK &&
              completion_state(&completion) == 2,
              "observe fail-fast trigger completion");
        rxvm_channel_binary_free(&completion);
        CHECK(rxvm_channel_wait(
                  context, scope_channel, -1, &completion) ==
                      RXVM_CHANNEL_OK &&
              completion_state(&completion) == 3,
              "running fail-fast sibling is cooperatively cancelled");
        rxvm_channel_binary_free(&completion);
        CHECK(rxvm_channel_close(
                  context, scope_channel, 1) == RXVM_CHANNEL_OK,
              "close isolated fail-fast scope");
        scope_channel = 0;
    }
    CHECK(rxvm_channel_close(context, channel, 1) == RXVM_CHANNEL_OK,
          "close fail-fast process pool");
    channel = 0;

cleanup:
    if (scope_channel && context) {
        (void)rxvm_channel_close(context, scope_channel, 2);
    }
    if (channel && context) (void)rxvm_channel_close(context, channel, 2);
    if (context) rxvm_destroy(context);
    if (runtime) {
        CHECK(rxvm_runtime_worker_count(runtime) == 0u,
              "process crash test unregisters controller worker");
        CHECK(rxvm_runtime_destroy(runtime) == 0u,
              "process crash test runtime has no allocator leaks");
    }
    free(pool);
    free(invoke);
    free(scope);
    if (failures) {
        fprintf(stderr, "%d process-provider check(s) failed\n", failures);
        return 1;
    }
    puts("PASS: Gate F F1e process crash and replacement");
    return 0;
}
