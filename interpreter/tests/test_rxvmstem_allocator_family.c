/* Stem string buffers must use the same allocator family as value teardown. */

#include <stdio.h>
#include <string.h>

#include "platform.h"
#include "rxbin.h"
#include "rxvalue.h"
#include "rxvmstem.h"

#define CHECK(condition, message) do {                                      \
    if (!(condition)) {                                                     \
        fprintf(stderr, "FAIL line %d: %s\n", __LINE__, (message));        \
        failures++;                                                         \
    }                                                                       \
} while (0)

static int failures;

static int text_is(const value *v, const char *text) {
    size_t length = strlen(text);
    return v->string_length == length &&
           (!length || memcmp(v->string_value, text, length) == 0);
}

static void insert_and_check(value *stem, const char *key_text,
                             const char *value_text) {
    value key;
    value input;
    value output;
    rxstem_key_parts parts;

    value_init(&key);
    value_init(&input);
    value_init(&output);
    set_null_string(&key, key_text);
    set_null_string(&input, value_text);
    parts = rxstem_one_part(&key);

    CHECK(rxstem_set_parts(stem, &parts, &input) == RXSTEM_OK,
          "stem insertion succeeds");
    CHECK(rxstem_get_parts(&output, stem, &parts) == RXSTEM_OK,
          "stem lookup succeeds");
    CHECK(text_is(&output, value_text), "stem lookup preserves the value");

    clear_value(&output);
    clear_value(&input);
    clear_value(&key);
}

int main(void) {
    static const char long_key[] =
            "abcdefghijklmnopqrstuvwxyz-0123456789-long-key";
    static const char long_value[] =
            "abcdefghijklmnopqrstuvwxyz-0123456789-long-value";
    rxvm_memory_context *context = rxvm_memory_context_create();
    rxvm_memory_worker *worker;
    rxvm_memory_worker *previous;
    rxvm_memory_stats stats;
    value stem;

    CHECK(context != 0, "memory context creation succeeds");
    if (!context) return 1;
    worker = rxvm_memory_worker_create(context);
    CHECK(worker != 0, "memory worker creation succeeds");
    if (!worker) {
        (void)rxvm_memory_context_destroy(context);
        return 1;
    }

    previous = rxvm_memory_enter(worker);
    value_init(&stem);
    insert_and_check(&stem, "short-key", "short-value");
    insert_and_check(&stem, long_key, long_value);

    CHECK(rxvm_memory_owner(
                  stem.attributes[RXSTEM_KEYS_ATTRIBUTE]
                          ->attributes[1]->string_value) == worker,
          "long stem key uses the worker allocator");
    CHECK(rxvm_memory_owner(
                  stem.attributes[RXSTEM_VALUES_ATTRIBUTE]
                          ->attributes[1]->string_value) == worker,
          "long stem value uses the worker allocator");

    clear_value(&stem);
    rxvm_memory_get_stats(context, &stats);
    CHECK(stats.live_allocations == 0u,
          "stem teardown releases every allocation");
    CHECK(stats.invalid_frees == 0u,
          "stem teardown makes no cross-family release attempt");
    CHECK(stats.wrong_owner_frees == 0u,
          "stem teardown releases through its owning worker");
    rxvm_memory_leave(previous);

    CHECK(rxvm_memory_worker_destroy(worker) == 0u,
          "stem worker teardown is leak-free");
    CHECK(rxvm_memory_context_destroy(context) == 0u,
          "stem memory context teardown is leak-free");

    if (failures) {
        fprintf(stderr, "%d stem allocator-family test failure(s)\n", failures);
        return 1;
    }
    printf("rxvmstem allocator-family tests passed\n");
    return 0;
}
