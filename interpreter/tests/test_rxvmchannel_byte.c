/*
 * Gate F F1d reusable byte-endpoint provider conformance.
 *
 * Proves runtime-scoped logical references across independent VM contexts,
 * rights restriction, canonical completion flags and forged-reference
 * rejection through the same private channel core used by RXAS.
 */

#include "../../inc/rxvm.h"
#include "rxvmchannel.h"
#include "rxvmchannel_byte.h"
#include "rxvmchannel_child.h"
#include "rxvmintp.h"
#include "rxvmworker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct bytes {
    unsigned char *data;
    size_t length;
    size_t capacity;
} bytes;

typedef struct field {
    const char *name;
    const unsigned char *node;
    size_t node_length;
} field;

static int start_and_wait(rxvm_context *context,
                          int64_t channel,
                          const bytes *request,
                          rxvm_channel_binary *completion_out);

static int failures;

static const unsigned char null_document[] = {
    'R', 'X', 'C', 'V', 1, 0, 0, 0,
    28, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#define CHECK(condition, message)                                             \
    do {                                                                      \
        if (!(condition)) {                                                   \
            fprintf(stderr, "FAIL: %s\n", (message));                        \
            failures++;                                                       \
        }                                                                     \
    } while (0)

static void put_u32(unsigned char *data, uint32_t value) {
    unsigned int index;
    for (index = 0u; index < 4u; index++) {
        data[index] = (unsigned char)(value >> (index * 8u));
    }
}

static void put_u64(unsigned char *data, uint64_t value) {
    unsigned int index;
    for (index = 0u; index < 8u; index++) {
        data[index] = (unsigned char)(value >> (index * 8u));
    }
}

static uint16_t get_u16(const unsigned char *data) {
    return (uint16_t)data[0] | (uint16_t)((uint16_t)data[1] << 8u);
}

static uint64_t get_u64(const unsigned char *data) {
    uint64_t value = 0u;
    unsigned int index;
    for (index = 0u; index < 8u; index++) {
        value |= (uint64_t)data[index] << (index * 8u);
    }
    return value;
}

static int append(bytes *output, const void *data, size_t length) {
    size_t needed;
    size_t next;
    unsigned char *replacement;
    if (!output || (!data && length) || length > SIZE_MAX - output->length) {
        return 0;
    }
    needed = output->length + length;
    if (needed > output->capacity) {
        next = output->capacity ? output->capacity : 128u;
        while (next < needed) {
            if (next > SIZE_MAX / 2u) return 0;
            next *= 2u;
        }
        replacement = (unsigned char *)realloc(output->data, next);
        if (!replacement) return 0;
        output->data = replacement;
        output->capacity = next;
    }
    if (length) memcpy(output->data + output->length, data, length);
    output->length = needed;
    return 1;
}

static int append_u32(bytes *output, uint32_t value) {
    unsigned char encoded[4];
    put_u32(encoded, value);
    return append(output, encoded, sizeof(encoded));
}

static int append_u64(bytes *output, uint64_t value) {
    unsigned char encoded[8];
    put_u64(encoded, value);
    return append(output, encoded, sizeof(encoded));
}

static bytes make_node(unsigned int tag, const void *payload, size_t length) {
    bytes node = {0};
    unsigned char header[12] = {0};
    header[0] = (unsigned char)tag;
    put_u64(header + 4u, length);
    if (!append(&node, header, sizeof(header)) ||
        !append(&node, payload, length)) {
        free(node.data);
        memset(&node, 0, sizeof(node));
    }
    return node;
}

static bytes integer_node(int64_t value) {
    uint64_t bits;
    unsigned char payload[8];
    memcpy(&bits, &value, sizeof(bits));
    put_u64(payload, bits);
    return make_node(3u, payload, sizeof(payload));
}

static bytes binary_node(const void *data, size_t length) {
    return make_node(7u, data, length);
}

static bytes string_node(const char *text) {
    return make_node(6u, text, strlen(text));
}

static bytes null_node(void) {
    return make_node(0u, 0, 0u);
}

static bytes string_array_node(const char *const *items, size_t count) {
    bytes payload = {0};
    bytes result = {0};
    size_t index;
    if (!append_u64(&payload, count)) return result;
    for (index = 0u; index < count; index++) {
        bytes item = string_node(items[index]);
        if (!item.data || !append(&payload, item.data, item.length)) {
            free(item.data);
            free(payload.data);
            return result;
        }
        free(item.data);
    }
    result = make_node(8u, payload.data, payload.length);
    free(payload.data);
    return result;
}

static bytes record_document(const char *schema,
                             const field *fields,
                             size_t field_count,
                             uint16_t flags) {
    bytes payload = {0};
    bytes root = {0};
    bytes document = {0};
    unsigned char header[16] = {'R', 'X', 'C', 'V', 1, 0, 0, 0};
    size_t index;
    size_t schema_length = strlen(schema);
    if (!append_u64(&payload, schema_length) ||
        !append(&payload, schema, schema_length) ||
        !append_u32(&payload, 1u) ||
        !append_u64(&payload, field_count)) goto failed;
    for (index = 0u; index < field_count; index++) {
        size_t name_length = strlen(fields[index].name);
        if (!append_u64(&payload, name_length) ||
            !append(&payload, fields[index].name, name_length) ||
            !append(&payload, fields[index].node,
                    fields[index].node_length)) goto failed;
    }
    root = make_node(9u, payload.data, payload.length);
    if (!root.data) goto failed;
    header[6] = (unsigned char)flags;
    header[7] = (unsigned char)(flags >> 8u);
    put_u64(header + 8u, 16u + root.length);
    if (!append(&document, header, sizeof(header)) ||
        !append(&document, root.data, root.length)) goto failed;
    free(payload.data);
    free(root.data);
    return document;

failed:
    free(payload.data);
    free(root.data);
    free(document.data);
    memset(&document, 0, sizeof(document));
    return document;
}

static int node_open(const unsigned char *data,
                     size_t length,
                     const unsigned char **payload_out,
                     size_t *payload_length_out,
                     size_t *node_length_out) {
    uint64_t payload_length;
    if (!data || length < 12u) return 0;
    payload_length = get_u64(data + 4u);
    if (payload_length > SIZE_MAX ||
        (size_t)payload_length > length - 12u) return 0;
    if (payload_out) *payload_out = data + 12u;
    if (payload_length_out) *payload_length_out = (size_t)payload_length;
    if (node_length_out) *node_length_out = 12u + (size_t)payload_length;
    return 1;
}

static int record_field_node(const unsigned char *document,
                             size_t document_length,
                             const char *wanted,
                             const unsigned char **node_out,
                             size_t *node_length_out) {
    const unsigned char *payload;
    size_t payload_length;
    const unsigned char *cursor;
    size_t remaining;
    uint64_t schema_length;
    uint64_t field_count;
    uint64_t index;
    if (!document || document_length < 28u || document[16u] != 9u ||
        !node_open(document + 16u, document_length - 16u,
                   &payload, &payload_length, 0) || payload_length < 20u) {
        return 0;
    }
    schema_length = get_u64(payload);
    if (schema_length > SIZE_MAX ||
        (size_t)schema_length > payload_length - 8u) return 0;
    cursor = payload + 8u + (size_t)schema_length;
    remaining = payload_length - 8u - (size_t)schema_length;
    if (remaining < 12u) return 0;
    field_count = get_u64(cursor + 4u);
    cursor += 12u;
    remaining -= 12u;
    for (index = 0u; index < field_count; index++) {
        uint64_t name_length;
        size_t node_length;
        if (remaining < 8u) return 0;
        name_length = get_u64(cursor);
        cursor += 8u;
        remaining -= 8u;
        if (name_length > SIZE_MAX ||
            (size_t)name_length > remaining) return 0;
        if (!node_open(cursor + (size_t)name_length,
                       remaining - (size_t)name_length,
                       0, 0, &node_length)) return 0;
        if (strlen(wanted) == (size_t)name_length &&
            !memcmp(cursor, wanted, (size_t)name_length)) {
            *node_out = cursor + (size_t)name_length;
            *node_length_out = node_length;
            return 1;
        }
        cursor += (size_t)name_length + node_length;
        remaining -= (size_t)name_length + node_length;
    }
    return 0;
}

static bytes memory_configuration(int direction, int capacity) {
    bytes capacity_node = integer_node(capacity);
    bytes direction_node = integer_node(direction);
    field fields[] = {
        {"capacity", capacity_node.data, capacity_node.length},
        {"direction", direction_node.data, direction_node.length}
    };
    bytes result = record_document(
            "crexx.channel.byte-memory", fields, 2u, 0u);
    free(capacity_node.data);
    free(direction_node.data);
    return result;
}

static bytes one_integer_request(const char *schema,
                                 const char *name,
                                 int64_t value) {
    bytes value_node = integer_node(value);
    field fields[] = {{name, value_node.data, value_node.length}};
    bytes result = record_document(schema, fields, 1u, 0u);
    free(value_node.data);
    return result;
}

static bytes export_request(int rights) {
    bytes rights_node = integer_node(rights);
    bytes scope_node = integer_node(1);
    field fields[] = {
        {"requestedRights", rights_node.data, rights_node.length},
        {"requestedScope", scope_node.data, scope_node.length}
    };
    bytes result = record_document(
            "crexx.channel.byte-export-reference", fields, 2u, 0u);
    free(rights_node.data);
    free(scope_node.data);
    return result;
}

static bytes child_provider_configuration(int capacity) {
    bytes capacity_node = integer_node(capacity);
    field fields[] = {{"capacity", capacity_node.data, capacity_node.length}};
    bytes result = record_document(
            "crexx.channel.child-process-provider", fields, 1u, 0u);
    free(capacity_node.data);
    return result;
}

static bytes environment_node(const char *name, const char *value) {
    bytes value_node = string_node(value);
    field fields[] = {{name, value_node.data, value_node.length}};
    bytes document = record_document(
            "crexx.channel.environment", fields, 1u, 0u);
    bytes result = {0};
    if (document.data) {
        append(&result, document.data + 16u, document.length - 16u);
    }
    free(document.data);
    free(value_node.data);
    return result;
}

static bytes child_start_request(
        const char *executable,
        int mode,
        const char *const *arguments,
        size_t argument_count,
        const bytes *environment,
        const bytes *stdin_reference,
        const bytes *stdout_reference,
        const bytes *stderr_reference) {
    bytes arguments_node = string_array_node(arguments, argument_count);
    bytes bindings_node = string_array_node(0, 0u);
    bytes executable_node = string_node(executable);
    bytes mode_node = integer_node(mode);
    bytes null_value = null_node();
    field fields[] = {
        {"arguments", arguments_node.data, arguments_node.length},
        {"bindings", bindings_node.data, bindings_node.length},
        {"environment", environment ? environment->data : null_value.data,
                        environment ? environment->length : null_value.length},
        {"executable", executable_node.data, executable_node.length},
        {"mode", mode_node.data, mode_node.length},
        {"stderr", stderr_reference ? stderr_reference->data : null_value.data,
                   stderr_reference ? stderr_reference->length : null_value.length},
        {"stdin", stdin_reference ? stdin_reference->data : null_value.data,
                  stdin_reference ? stdin_reference->length : null_value.length},
        {"stdout", stdout_reference ? stdout_reference->data : null_value.data,
                   stdout_reference ? stdout_reference->length : null_value.length},
        {"workingDirectory", null_value.data, null_value.length}
    };
    uint16_t flags = (stdin_reference || stdout_reference || stderr_reference)
            ? 1u : 0u;
    bytes result = record_document(
            "crexx.channel.child-process-start", fields, 9u, flags);
    free(arguments_node.data);
    free(bindings_node.data);
    free(executable_node.data);
    free(mode_node.data);
    free(null_value.data);
    return result;
}

static bytes export_reference(rxvm_context *context,
                              int64_t channel,
                              int rights) {
    bytes request = export_request(rights);
    bytes reference = {0};
    rxvm_channel_binary completion = {0};
    const unsigned char *node = 0;
    size_t node_length = 0u;
    if (request.data && start_and_wait(
            context, channel, &request, &completion) &&
        record_field_node(completion.data, completion.length, "result",
                          &node, &node_length)) {
        (void)append(&reference, node, node_length);
    }
    rxvm_channel_binary_free(&completion);
    free(request.data);
    return reference;
}

static int64_t completion_integer_field(
        const rxvm_channel_binary *completion,
        const char *name,
        int *valid) {
    const unsigned char *node = 0;
    size_t length = 0u;
    uint64_t bits;
    int64_t value = 0;
    *valid = record_field_node(
            completion->data, completion->length, name, &node, &length) &&
             length == 20u && node[0] == 3u && get_u64(node + 4u) == 8u;
    if (*valid) {
        bits = get_u64(node + 12u);
        memcpy(&value, &bits, sizeof(value));
    }
    return value;
}

static bytes reference_configuration(const unsigned char *reference,
                                     size_t reference_length) {
    field fields[] = {{"reference", reference, reference_length}};
    return record_document(
            "crexx.channel.byte-reference", fields, 1u, 1u);
}

static bytes write_request(const void *data, size_t length) {
    bytes value = binary_node(data, length);
    field fields[] = {{"bytes", value.data, value.length}};
    bytes result = record_document(
            "crexx.channel.byte-write", fields, 1u, 0u);
    free(value.data);
    return result;
}

static int start_and_wait(rxvm_context *context,
                          int64_t channel,
                          const bytes *request,
                          rxvm_channel_binary *completion_out) {
    int64_t ticket = 0;
    return rxvm_channel_start(
                   context, channel, request->data, request->length,
                   0, &ticket) == RXVM_CHANNEL_OK && ticket > 0 &&
           rxvm_channel_wait(
                   context, channel, 1000000, completion_out) ==
                   RXVM_CHANNEL_OK;
}

int main(void) {
    rxvm_runtime *runtime = rxvm_runtime_create();
    rxvm_context *producer = 0;
    rxvm_context *consumer = 0;
    bytes memory = memory_configuration(3, 8);
    bytes write = write_request("abc", 3u);
    bytes export = export_request(1);
    bytes read = one_integer_request(
            "crexx.channel.byte-read", "maximumBytes", 3);
    bytes drain = one_integer_request(
            "crexx.channel.byte-drain", "maximumBytes", 8);
    bytes reference_config = {0};
    rxvm_channel_binary completion = {0};
    const unsigned char *reference = 0;
    const unsigned char *result = 0;
    size_t reference_length = 0u;
    size_t result_length = 0u;
    int64_t producer_channel = 0;
    int64_t consumer_channel = 0;
    int64_t child_output_channel = 0;
    int64_t crexx_input_channel = 0;
    int64_t crexx_batch_output_channel = 0;
    int64_t crexx_output_channel = 0;
    int64_t ignored_input_channel = 0;
    int64_t failed_launch_input_channel = 0;
    int64_t saturated_output_channel = 0;
    int64_t child_channel = 0;
    int64_t rejected_ticket = 0;
    bytes child_output_memory = {0};
    bytes child_output_reference = {0};
    bytes child_configuration = {0};
    bytes child_request = {0};
    bytes child_environment = {0};
    bytes child_drain = {0};
    bytes crexx_input_memory = {0};
    bytes crexx_input_write = {0};
    bytes crexx_input_half_close = {0};
    bytes crexx_input_reference = {0};
    bytes crexx_batch_output_memory = {0};
    bytes crexx_batch_output_reference = {0};
    bytes crexx_batch_drain = {0};
    bytes crexx_batch_request = {0};
    bytes crexx_output_memory = {0};
    bytes crexx_output_reference = {0};
    bytes crexx_drain = {0};
    bytes crexx_request = {0};
    bytes ignored_input_memory = {0};
    bytes ignored_input_reference = {0};
    bytes failed_launch_input_memory = {0};
    bytes failed_launch_input_reference = {0};
    bytes saturated_output_memory = {0};
    bytes saturated_output_reference = {0};
    bytes cancel_request = {0};
    bytes timeout_request = {0};

    CHECK(runtime != 0, "runtime creates");
    CHECK(memory.data && write.data && export.data && read.data && drain.data,
          "canonical fixtures allocate");
    if (!runtime || !memory.data || !write.data || !export.data || !read.data ||
        !drain.data) {
        goto cleanup;
    }
    producer = rxvm_context_create_in_runtime(runtime);
    consumer = rxvm_context_create_in_runtime(runtime);
    CHECK(producer && consumer, "independent contexts attach to one runtime");
    if (!producer || !consumer) goto cleanup;

    CHECK(rxvm_channel_open(
                  producer, RXVM_CHANNEL_PROVIDER_BYTE_ENDPOINT, 0,
                  memory.data, memory.length, &producer_channel) ==
                  RXVM_CHANNEL_OK,
          "producer memory endpoint opens");
    CHECK(start_and_wait(producer, producer_channel, &write, &completion),
          "producer writes endpoint-owned bytes");
    rxvm_channel_binary_free(&completion);

    CHECK(start_and_wait(producer, producer_channel, &export, &completion),
          "read-only runtime reference exports");
    CHECK(get_u16(completion.data + 6u) == 1u,
          "completion propagates provider-reference canonical flag");
    CHECK(record_field_node(
                  completion.data, completion.length, "result",
                  &reference, &reference_length) && reference[0] == 10u,
          "export completion contains provider-reference node");
    if (!reference) goto cleanup;
    reference_config = reference_configuration(reference, reference_length);
    rxvm_channel_binary_free(&completion);

    CHECK(rxvm_channel_open(
                  consumer, RXVM_CHANNEL_PROVIDER_BYTE_ENDPOINT, 0,
                  reference_config.data, reference_config.length,
                  &consumer_channel) == RXVM_CHANNEL_OK,
          "second context resolves runtime-scoped reference");
    CHECK(rxvm_channel_start(
                  consumer, consumer_channel, write.data, write.length,
                  0, &rejected_ticket) == RXVM_CHANNEL_INVALID_CONFIGURATION &&
                  rejected_ticket == 0,
          "read-only reference rejects write before ticket allocation");
    CHECK(start_and_wait(consumer, consumer_channel, &read, &completion),
          "second context reads shared endpoint bytes");
    CHECK(record_field_node(
                  completion.data, completion.length, "result",
                  &result, &result_length) && result[0] == 7u &&
                  get_u64(result + 4u) == 3u &&
                  !memcmp(result + 12u, "abc", 3u),
          "cross-context read returns exact independent byte snapshot");
    rxvm_channel_binary_free(&completion);

    CHECK(start_and_wait(producer, producer_channel, &write, &completion),
          "producer refills bytes for nonblocking drain");
    rxvm_channel_binary_free(&completion);
    CHECK(start_and_wait(producer, producer_channel, &drain, &completion),
          "nonblocking drain returns consumed bytes without loss");
    CHECK(record_field_node(
                  completion.data, completion.length, "result",
                  &result, &result_length) && result[0] == 7u &&
                  get_u64(result + 4u) == 3u &&
                  !memcmp(result + 12u, "abc", 3u),
          "partial nonblocking drain materializes exact consumed bytes");
    rxvm_channel_binary_free(&completion);

    {
        int iteration;
        for (iteration = 0; iteration < 100; iteration++) {
            int64_t ticket = 0;
            int valid = 0;
            int64_t state;
            CHECK(rxvm_channel_start(
                          producer, producer_channel,
                          read.data, read.length, -1, &ticket) ==
                          RXVM_CHANNEL_OK && ticket > 0,
                  "blocked byte read starts before cancellation");
            CHECK(rxvm_channel_cancel(
                          producer, producer_channel, ticket,
                          null_document, sizeof(null_document)) ==
                          RXVM_CHANNEL_OK,
                  "accepted byte cancellation wins terminal publication");
            CHECK(rxvm_channel_wait(
                          producer, producer_channel, 1000000,
                          &completion) == RXVM_CHANNEL_OK,
                  "cancelled byte read publishes one completion");
            state = completion_integer_field(&completion, "state", &valid);
            CHECK(valid && state == 3,
                  "cancelled byte read cannot publish success");
            rxvm_channel_binary_free(&completion);
        }
    }

    CHECK(rxvm_channel_close(consumer, consumer_channel, 1) == RXVM_CHANNEL_OK,
          "reference adapter closes without closing shared endpoint");
    consumer_channel = 0;

    reference_config.data[reference_config.length - 1u] ^= 0x80u;
    CHECK(rxvm_channel_open(
                  consumer, RXVM_CHANNEL_PROVIDER_BYTE_ENDPOINT, 0,
                  reference_config.data, reference_config.length,
                  &consumer_channel) == RXVM_CHANNEL_STALE_CAPABILITY,
          "forged logical identity fails closed");
    consumer_channel = 0;

    child_output_memory = memory_configuration(3, 256);
    CHECK(rxvm_channel_open(
                  producer, RXVM_CHANNEL_PROVIDER_BYTE_ENDPOINT, 0,
                  child_output_memory.data, child_output_memory.length,
                  &child_output_channel) == RXVM_CHANNEL_OK,
          "child output endpoint opens");
    child_output_reference = export_reference(
            producer, child_output_channel, 3);
    CHECK(child_output_reference.data != 0,
          "child output endpoint exports duplex attachment reference");
    child_configuration = child_provider_configuration(1);
    CHECK(rxvm_channel_open(
                  producer, RXVM_CHANNEL_PROVIDER_CHILD_PROCESS, 0,
                  child_configuration.data, child_configuration.length,
                  &child_channel) == RXVM_CHANNEL_OK,
          "bounded child-process provider opens");
    child_drain = one_integer_request(
            "crexx.channel.byte-drain", "maximumBytes", 256);
    {
        int64_t drain_ticket = 0;
        CHECK(rxvm_channel_start(
                      producer, child_output_channel,
                      child_drain.data, child_drain.length,
                      -1, &drain_ticket) == RXVM_CHANNEL_OK &&
                      drain_ticket > 0,
              "controller starts bounded drain before child launch");
    }
    {
#if defined(_WIN32)
        const char *executable = "cmd.exe";
        const char *arguments[] = {
            "/D", "/S", "/C", "ping -n 6 127.0.0.1 >nul"
        };
        size_t argument_count = 4u;
#else
        const char *executable = "/bin/sh";
        const char *arguments[] = {"-c", "sleep 5"};
        size_t argument_count = 2u;
#endif
        int64_t ticket = 0;
        int valid = 0;
        int64_t state;
        cancel_request = child_start_request(
                executable, 0, arguments, argument_count, 0, 0, 0, 0);
        CHECK(rxvm_channel_start(
                      producer, child_channel,
                      cancel_request.data, cancel_request.length,
                      -1, &ticket) == RXVM_CHANNEL_OK && ticket > 0,
              "explicitly cancelled child starts");
        CHECK(rxvm_channel_cancel(
                      producer, child_channel, ticket,
                      null_document, sizeof(null_document)) ==
                      RXVM_CHANNEL_OK,
              "accepted child cancellation races safely with launch");
        CHECK(rxvm_channel_wait(
                      producer, child_channel, 3000000, &completion) ==
                      RXVM_CHANNEL_OK,
              "cancelled child terminates and reaps");
        state = completion_integer_field(&completion, "state", &valid);
        CHECK(valid && state == 3,
              "cancelled child cannot publish success");
        rxvm_channel_binary_free(&completion);
    }
    {
#if defined(_WIN32)
        const char *executable = "cmd.exe";
        const char *arguments[] = {
            "/D", "/S", "/C",
            "if defined PATH (<nul set /p =%CREXX_CHILD_TEST%:inherited) else (<nul set /p =missing)"
        };
        size_t argument_count = 4u;
#else
        const char *executable = "/bin/sh";
        const char *arguments[] = {
            "-c", "if [ -n \"$PATH\" ]; then printf \"%s:inherited\" \"$CREXX_CHILD_TEST\"; else printf missing; fi"
        };
        size_t argument_count = 2u;
#endif
        child_environment = environment_node(
                "CREXX_CHILD_TEST", "child-ok");
        child_request = child_start_request(
                executable, 0, arguments, argument_count,
                child_environment.data ? &child_environment : 0,
                0, &child_output_reference, 0);
    }
    {
        int64_t ticket = 0;
        int valid = 0;
        int64_t state;
        CHECK(rxvm_channel_start(
                      producer, child_channel,
                      child_request.data, child_request.length,
                      2000000, &ticket) == RXVM_CHANNEL_OK && ticket > 0,
              "child request accepts copied process image");
        CHECK(rxvm_channel_wait(
                      producer, child_channel, 3000000, &completion) ==
                      RXVM_CHANNEL_OK,
              "child process reaches one terminal completion");
        state = completion_integer_field(&completion, "state", &valid);
        CHECK(valid && state == 1,
              "zero-exit child is a successful provider completion");
        rxvm_channel_binary_free(&completion);
    }
    CHECK(rxvm_channel_wait(
                  producer, child_output_channel, 1000000, &completion) ==
                  RXVM_CHANNEL_OK,
          "prestarted controller drain completes at child output EOF");
    CHECK(record_field_node(
                  completion.data, completion.length, "result",
                  &result, &result_length) && result[0] == 7u &&
                  get_u64(result + 4u) == 18u &&
                  !memcmp(result + 12u, "child-ok:inherited", 18u),
          "child environment overlays inherited immutable snapshot");
    rxvm_channel_binary_free(&completion);

    crexx_input_memory = memory_configuration(3, 256);
    CHECK(rxvm_channel_open(
                  producer, RXVM_CHANNEL_PROVIDER_BYTE_ENDPOINT, 0,
                  crexx_input_memory.data, crexx_input_memory.length,
                  &crexx_input_channel) == RXVM_CHANNEL_OK,
          "CREXX batch input endpoint opens");
    crexx_input_write = write_request(
            "echo batch-one\necho batch-two\n", 30u);
    CHECK(start_and_wait(
                  producer, crexx_input_channel,
                  &crexx_input_write, &completion),
          "CREXX batch input snapshot is written before launch");
    rxvm_channel_binary_free(&completion);
    crexx_input_half_close = one_integer_request(
            "crexx.channel.byte-half-close", "direction", 2);
    CHECK(start_and_wait(
                  producer, crexx_input_channel,
                  &crexx_input_half_close, &completion),
          "CREXX batch input publishes write EOF before launch");
    rxvm_channel_binary_free(&completion);
    crexx_input_reference = export_reference(
            producer, crexx_input_channel, 1);

    crexx_batch_output_memory = memory_configuration(3, 256);
    CHECK(rxvm_channel_open(
                  producer, RXVM_CHANNEL_PROVIDER_BYTE_ENDPOINT, 0,
                  crexx_batch_output_memory.data,
                  crexx_batch_output_memory.length,
                  &crexx_batch_output_channel) == RXVM_CHANNEL_OK,
          "CREXX batch output endpoint opens");
    crexx_batch_output_reference = export_reference(
            producer, crexx_batch_output_channel, 2);
    crexx_batch_drain = one_integer_request(
            "crexx.channel.byte-drain", "maximumBytes", 256);
    {
        int64_t drain_ticket = 0;
        CHECK(rxvm_channel_start(
                      producer, crexx_batch_output_channel,
                      crexx_batch_drain.data, crexx_batch_drain.length,
                      -1, &drain_ticket) == RXVM_CHANNEL_OK &&
                      drain_ticket > 0,
              "CREXX batch drain starts before synchronous launch");
    }
    crexx_batch_request = child_start_request(
            "batch", 2, 0, 0u, 0,
            &crexx_input_reference, &crexx_batch_output_reference, 0);
    {
        int64_t ticket = 0;
        int valid = 0;
        int64_t state;
        CHECK(rxvm_channel_start(
                      producer, child_channel,
                      crexx_batch_request.data,
                      crexx_batch_request.length,
                      -1, &ticket) == RXVM_CHANNEL_OK && ticket > 0,
              "synchronous CREXX batch consumes an endpoint snapshot");
        CHECK(rxvm_channel_wait(
                      producer, child_channel, 1000000, &completion) ==
                      RXVM_CHANNEL_OK,
              "CREXX batch completion is published");
        state = completion_integer_field(&completion, "state", &valid);
        CHECK(valid && state == 1,
              "CREXX batch reports successful completion");
        rxvm_channel_binary_free(&completion);
    }
    CHECK(rxvm_channel_wait(
                  producer, crexx_batch_output_channel,
                  1000000, &completion) == RXVM_CHANNEL_OK,
          "CREXX batch drain completes at output EOF");
    CHECK(record_field_node(
                  completion.data, completion.length, "result",
                  &result, &result_length) && result[0] == 7u &&
                  get_u64(result + 4u) == 20u &&
                  !memcmp(result + 12u,
                          "batch-one\nbatch-two\n", 20u),
          "CREXX batch preserves endpoint input and exact output bytes");
    rxvm_channel_binary_free(&completion);
    CHECK(rxvm_channel_close(
                  producer, crexx_input_channel, 1) == RXVM_CHANNEL_OK,
          "CREXX batch input endpoint closes after consumption");
    crexx_input_channel = 0;
    CHECK(rxvm_channel_close(
                  producer, crexx_batch_output_channel, 1) ==
                  RXVM_CHANNEL_OK,
          "CREXX batch output endpoint closes after drain");
    crexx_batch_output_channel = 0;

    crexx_output_memory = memory_configuration(3, 256);
    CHECK(rxvm_channel_open(
                  producer, RXVM_CHANNEL_PROVIDER_BYTE_ENDPOINT, 0,
                  crexx_output_memory.data, crexx_output_memory.length,
                  &crexx_output_channel) == RXVM_CHANNEL_OK,
          "CREXX child output endpoint opens");
    crexx_output_reference = export_reference(
            producer, crexx_output_channel, 2);
    crexx_drain = one_integer_request(
            "crexx.channel.byte-drain", "maximumBytes", 256);
    {
        int64_t drain_ticket = 0;
        CHECK(rxvm_channel_start(
                      producer, crexx_output_channel,
                      crexx_drain.data, crexx_drain.length,
                      -1, &drain_ticket) == RXVM_CHANNEL_OK &&
                      drain_ticket > 0,
              "CREXX child drain starts before synchronous launch");
    }
    crexx_request = child_start_request(
            "echo crexx-ok", 2, 0, 0u, 0, 0,
            &crexx_output_reference, 0);
    {
        int64_t ticket = 0;
        int valid = 0;
        int64_t state;
        CHECK(rxvm_channel_start(
                      producer, child_channel,
                      crexx_request.data, crexx_request.length,
                      -1, &ticket) == RXVM_CHANNEL_OK && ticket > 0,
              "synchronous CREXX child starts with reusable output");
        CHECK(rxvm_channel_wait(
                      producer, child_channel, 1000000, &completion) ==
                      RXVM_CHANNEL_OK,
              "CREXX child completion is published");
        state = completion_integer_field(&completion, "state", &valid);
        CHECK(valid && state == 1,
              "CREXX child reports successful completion");
        rxvm_channel_binary_free(&completion);
    }
    CHECK(rxvm_channel_wait(
                  producer, crexx_output_channel, 1000000, &completion) ==
                  RXVM_CHANNEL_OK,
          "CREXX child drain completes at output EOF");
    CHECK(record_field_node(
                  completion.data, completion.length, "result",
                  &result, &result_length) && result[0] == 7u &&
                  get_u64(result + 4u) == 9u &&
                  !memcmp(result + 12u, "crexx-ok\n", 9u),
          "CREXX child stdout preserves exact bytes");
    rxvm_channel_binary_free(&completion);
    CHECK(rxvm_channel_close(
                  producer, crexx_output_channel, 1) == RXVM_CHANNEL_OK,
          "CREXX child output endpoint closes after drain");
    crexx_output_channel = 0;

    ignored_input_memory = memory_configuration(3, 8);
    CHECK(rxvm_channel_open(
                  producer, RXVM_CHANNEL_PROVIDER_BYTE_ENDPOINT, 0,
                  ignored_input_memory.data, ignored_input_memory.length,
                  &ignored_input_channel) == RXVM_CHANNEL_OK,
          "open child input endpoint opens");
    ignored_input_reference = export_reference(
            producer, ignored_input_channel, 1);
    {
#if defined(_WIN32)
        const char *executable = "cmd.exe";
        const char *arguments[] = {"/D", "/S", "/C", "exit /B 0"};
        size_t argument_count = 4u;
#else
        const char *executable = "/usr/bin/true";
        const char *arguments[] = {"ignored-input"};
        size_t argument_count = 1u;
#endif
        int64_t ticket = 0;
        int valid = 0;
        int64_t state;
        bytes ignored_input_request = child_start_request(
                executable, 0, arguments, argument_count, 0,
                &ignored_input_reference, 0, 0);
        CHECK(rxvm_channel_start(
                      producer, child_channel,
                      ignored_input_request.data,
                      ignored_input_request.length,
                      -1, &ticket) == RXVM_CHANNEL_OK && ticket > 0,
              "child starts with an open input stream it will ignore");
        CHECK(rxvm_channel_wait(
                      producer, child_channel, 2000000, &completion) ==
                      RXVM_CHANNEL_OK,
              "normal child exit wakes its blocked stdin producer");
        state = completion_integer_field(&completion, "state", &valid);
        CHECK(valid && state == 1,
              "ignored open stdin preserves successful child completion");
        rxvm_channel_binary_free(&completion);
        free(ignored_input_request.data);
    }
    CHECK(rxvm_channel_close(
                  producer, ignored_input_channel, 2) == RXVM_CHANNEL_OK,
          "ignored child input endpoint cancels and closes");
    ignored_input_channel = 0;

    failed_launch_input_memory = memory_configuration(3, 8);
    CHECK(rxvm_channel_open(
                  producer, RXVM_CHANNEL_PROVIDER_BYTE_ENDPOINT, 0,
                  failed_launch_input_memory.data,
                  failed_launch_input_memory.length,
                  &failed_launch_input_channel) == RXVM_CHANNEL_OK,
          "failed-launch child input endpoint opens");
    failed_launch_input_reference = export_reference(
            producer, failed_launch_input_channel, 1);
    {
#if defined(_WIN32)
        const char *executable = "crexx-definitely-missing-f1d.exe";
#else
        const char *executable = "/crexx-definitely-missing-f1d";
#endif
        const char *arguments[] = {"missing"};
        int64_t ticket = 0;
        int valid = 0;
        int64_t state;
        bytes failed_launch_request = child_start_request(
                executable, 0, arguments, 1u, 0,
                &failed_launch_input_reference, 0, 0);
        CHECK(rxvm_channel_start(
                      producer, child_channel,
                      failed_launch_request.data,
                      failed_launch_request.length,
                      -1, &ticket) == RXVM_CHANNEL_OK && ticket > 0,
              "missing child accepts a copied request before launch");
        CHECK(rxvm_channel_wait(
                      producer, child_channel, 1000000, &completion) ==
                      RXVM_CHANNEL_OK,
              "pre-launch failure releases a blocked stdin producer");
        state = completion_integer_field(&completion, "state", &valid);
        CHECK(valid && state == 2,
              "missing child publishes provider failure exactly once");
        rxvm_channel_binary_free(&completion);
        free(failed_launch_request.data);
    }
    CHECK(rxvm_channel_close(
                  producer, failed_launch_input_channel, 2) ==
                  RXVM_CHANNEL_OK,
          "failed-launch child input endpoint cancels and closes");
    failed_launch_input_channel = 0;

    saturated_output_memory = memory_configuration(3, 256);
    CHECK(rxvm_channel_open(
                  producer, RXVM_CHANNEL_PROVIDER_BYTE_ENDPOINT, 0,
                  saturated_output_memory.data,
                  saturated_output_memory.length,
                  &saturated_output_channel) == RXVM_CHANNEL_OK,
          "undrained deadline output endpoint opens");
    saturated_output_reference = export_reference(
            producer, saturated_output_channel, 2);
    {
#if defined(_WIN32)
        const char *executable = "cmd.exe";
        const char *arguments[] = {
            "/D", "/S", "/C",
            "for /L %i in (1,1,10000) do @echo 012345678901234567890123456789"
        };
        size_t argument_count = 4u;
#else
        const char *executable = "/bin/sh";
        const char *arguments[] = {
            "-c", "while :; do printf 012345678901234567890123456789; done"
        };
        size_t argument_count = 2u;
#endif
        int64_t ticket = 0;
        int valid = 0;
        int64_t state;
        bytes saturated_request = child_start_request(
                executable, 0, arguments, argument_count, 0, 0,
                &saturated_output_reference, 0);
        CHECK(rxvm_channel_start(
                      producer, child_channel,
                      saturated_request.data, saturated_request.length,
                      100000, &ticket) == RXVM_CHANNEL_OK && ticket > 0,
              "deadline child starts against an undrained bounded endpoint");
        CHECK(rxvm_channel_wait(
                      producer, child_channel, 3000000, &completion) ==
                      RXVM_CHANNEL_OK,
              "deadline wakes saturated redirect and reaps child");
        state = completion_integer_field(&completion, "state", &valid);
        CHECK(valid && state == 4,
              "saturated redirect preserves typed timeout completion");
        rxvm_channel_binary_free(&completion);
        free(saturated_request.data);
    }
    CHECK(rxvm_channel_close(
                  producer, saturated_output_channel, 2) == RXVM_CHANNEL_OK,
          "saturated deadline endpoint cancels and closes");
    saturated_output_channel = 0;

    {
#if defined(_WIN32)
        const char *executable = "cmd.exe";
        const char *arguments[] = {
            "/D", "/S", "/C", "ping -n 6 127.0.0.1 >nul"
        };
        size_t argument_count = 4u;
#else
        const char *executable = "/bin/sh";
        const char *arguments[] = {"-c", "sleep 5"};
        size_t argument_count = 2u;
#endif
        int64_t ticket = 0;
        int valid = 0;
        int64_t state;
        timeout_request = child_start_request(
                executable, 0, arguments, argument_count, 0, 0, 0, 0);
        CHECK(rxvm_channel_start(
                      producer, child_channel,
                      timeout_request.data, timeout_request.length,
                      100000, &ticket) == RXVM_CHANNEL_OK && ticket > 0,
              "deadline-controlled child starts");
        CHECK(rxvm_channel_wait(
                      producer, child_channel, 3000000, &completion) ==
                      RXVM_CHANNEL_OK,
              "deadline terminates and reaps child without hanging streams");
        state = completion_integer_field(&completion, "state", &valid);
        CHECK(valid && state == 4,
              "deadline child reports typed timeout completion");
        rxvm_channel_binary_free(&completion);
    }
    CHECK(rxvm_channel_close(producer, child_channel, 1) == RXVM_CHANNEL_OK,
          "child provider drains and closes");
    child_channel = 0;
    CHECK(rxvm_channel_close(
                  producer, child_output_channel, 1) == RXVM_CHANNEL_OK,
          "child output endpoint closes after drain");
    child_output_channel = 0;

cleanup:
    rxvm_channel_binary_free(&completion);
    if (consumer_channel && consumer) {
        (void)rxvm_channel_close(consumer, consumer_channel, 2);
    }
    if (producer_channel && producer) {
        (void)rxvm_channel_close(producer, producer_channel, 2);
    }
    if (child_channel && producer) {
        (void)rxvm_channel_close(producer, child_channel, 2);
    }
    if (child_output_channel && producer) {
        (void)rxvm_channel_close(producer, child_output_channel, 2);
    }
    if (crexx_output_channel && producer) {
        (void)rxvm_channel_close(producer, crexx_output_channel, 2);
    }
    if (crexx_batch_output_channel && producer) {
        (void)rxvm_channel_close(
                producer, crexx_batch_output_channel, 2);
    }
    if (crexx_input_channel && producer) {
        (void)rxvm_channel_close(producer, crexx_input_channel, 2);
    }
    if (saturated_output_channel && producer) {
        (void)rxvm_channel_close(
                producer, saturated_output_channel, 2);
    }
    if (ignored_input_channel && producer) {
        (void)rxvm_channel_close(producer, ignored_input_channel, 2);
    }
    if (failed_launch_input_channel && producer) {
        (void)rxvm_channel_close(
                producer, failed_launch_input_channel, 2);
    }
    if (consumer) rxvm_destroy(consumer);
    if (producer) rxvm_destroy(producer);
    if (runtime) {
        CHECK(rxvm_runtime_worker_count(runtime) == 0u,
              "byte-provider contexts unregister their workers");
        CHECK(rxvm_runtime_destroy(runtime) == 0u,
              "byte-provider runtime destroys without leaks");
    }
    free(memory.data);
    free(write.data);
    free(export.data);
    free(read.data);
    free(drain.data);
    free(reference_config.data);
    free(child_output_memory.data);
    free(child_output_reference.data);
    free(child_configuration.data);
    free(child_request.data);
    free(child_environment.data);
    free(child_drain.data);
    free(crexx_input_memory.data);
    free(crexx_input_write.data);
    free(crexx_input_half_close.data);
    free(crexx_input_reference.data);
    free(crexx_batch_output_memory.data);
    free(crexx_batch_output_reference.data);
    free(crexx_batch_drain.data);
    free(crexx_batch_request.data);
    free(crexx_output_memory.data);
    free(crexx_output_reference.data);
    free(crexx_drain.data);
    free(crexx_request.data);
    free(ignored_input_memory.data);
    free(ignored_input_reference.data);
    free(failed_launch_input_memory.data);
    free(failed_launch_input_reference.data);
    free(saturated_output_memory.data);
    free(saturated_output_reference.data);
    free(cancel_request.data);
    free(timeout_request.data);
    if (failures) {
        fprintf(stderr, "rxvm byte provider conformance: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("rxvm byte provider conformance: PASS");
    return 0;
}
