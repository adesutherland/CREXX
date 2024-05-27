//
// Created by Adrian Sutherland on 04/05/2024.
//
// Tests that take a JSON string, convert it to a SHVBLOCK structure, and then emit it back to a JSON string

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//#include "crexxsaa.h"
#include "httpclient.h"
#include "jsnemit.h"

// Function to compate a null-terminated string with a memory buffer */
int compare_string_with_memory_buffer(const char *expected, MemoryBuffer *mem_buffer) {
    size_t i;
    if (mem_buffer->length != strlen(expected)) {
        return 0;
    }

    for (i = 0; i < mem_buffer->length; i++) {
        if (expected[i] != mem_buffer->data[i]) {
            return 0;
        }
    }

    return 1;
}

// Execute a test of the emit and parse JSON functions
void do_test(char *json) {
    MemoryBuffer *mem_context;
    SHVBLOCK *block;

    // Copy json into a dynamic buffer (parseJSON mutates the buffer)
    char* json_in = malloc(strlen(json) + 1);
    strcpy(json_in, json);

    // Parse the JSON
    SHVBLOCK *shvblock_handle;
    PARSE_ERROR error;
    int rc = parseJSON(json_in, &shvblock_handle, &error);
    if (rc != 0) {
        printf("Test failed with error: %d (at pos %d in \"%s\")\n", rc, (int)error.position, json);
        printf("  Error: %s\n", error.message);
        printf("  Error code: %d\n", error.error_code);
        printf("  Position: %zu\n", error.position);
        // Print the JSON string snippet around the error
        size_t start = error.position > 10 ? error.position - 10 : 0;
        size_t end = error.position + 10 < strlen(json) ? error.position + 10 : strlen(json);
        printf("  JSON: %.*s\n", (int)(end - start), json + start);
        // Print a pointer to the error position
        printf("         %*s\n\n", (int)(error.position - start), "^");
        assert(0);
    }
    assert(shvblock_handle != NULL);

    // Emit the JSON
    mem_context = NULL;
    jsonEMIT(shvblock_handle, emit_to_memory_buffer, (void*)&mem_context);

    // - Check Output with expected value
    //   If the JSON is not the same as the original JSON, print the expected and generated JSON
    if (!compare_string_with_memory_buffer(json, mem_context)) {
        // Print to stderr
        fprintf(stderr, "Error\nExpected: %s\n", json);
        fprintf(stderr, "Generated: %s\n", mem_context->data);
        assert(0);
    }
    // - Cleanup
    FreeRexxVariablePoolResult(shvblock_handle);
    emit_to_memory_buffer(ACTION_CLOSE, NULL, (void*)&mem_context);
}

// Note the name is a misnomer - it really parses and creates a SHVBLOCK structure, then emits it back to JSON
void test_emitparseJSON() {

        MemoryBuffer *mem_context;
        SHVBLOCK *block;

        // Smoke test
        do_test("{\"serviceBlocks\":[{\"name\":\"hellovar\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");

        // Test with different shvcodes - "set","fetch","drop","nextv","priv","syset","syfet","sydro","sydel"
        do_test("{\"serviceBlocks\":[{\"name\":\"setvar\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
        do_test("{\"serviceBlocks\":[{\"name\":\"fetchvar\",\"request\":\"fetch\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
        do_test("{\"serviceBlocks\":[{\"name\":\"dropvar\",\"request\":\"drop\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
        do_test("{\"serviceBlocks\":[{\"name\":\"nextvvar\",\"request\":\"nextv\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
        do_test("{\"serviceBlocks\":[{\"name\":\"privvar\",\"request\":\"priv\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
        do_test("{\"serviceBlocks\":[{\"name\":\"sysetvar\",\"request\":\"syset\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
        do_test("{\"serviceBlocks\":[{\"name\":\"syfetvar\",\"request\":\"syfet\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
        do_test("{\"serviceBlocks\":[{\"name\":\"sydrovar\",\"request\":\"sydro\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
        do_test("{\"serviceBlocks\":[{\"name\":\"sydelvar\",\"request\":\"sydel\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");

        // Test with different retcodes - "ok","newv","lvar","trunc","badn","memfl","badf","noavl","notex"
        do_test("{\"serviceBlocks\":[{\"name\":\"okvar\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
        do_test("{\"serviceBlocks\":[{\"name\":\"newvvar\",\"request\":\"set\",\"result\":\"newv\",\"value\":\"Hello, World!\"}]}");
        do_test("{\"serviceBlocks\":[{\"name\":\"lvarvar\",\"request\":\"set\",\"result\":\"lvar\",\"value\":\"Hello, World!\"}]}");
        do_test("{\"serviceBlocks\":[{\"name\":\"truncvar\",\"request\":\"set\",\"result\":\"trunc\",\"value\":\"Hello, World!\"}]}");
        do_test("{\"serviceBlocks\":[{\"name\":\"badnvar\",\"request\":\"set\",\"result\":\"badn\",\"value\":\"Hello, World!\"}]}");
        do_test("{\"serviceBlocks\":[{\"name\":\"memflvar\",\"request\":\"set\",\"result\":\"memfl\",\"value\":\"Hello, World!\"}]}");
        do_test("{\"serviceBlocks\":[{\"name\":\"badfvar\",\"request\":\"set\",\"result\":\"badf\",\"value\":\"Hello, World!\"}]}");
        do_test("{\"serviceBlocks\":[{\"name\":\"noavlvar\",\"request\":\"set\",\"result\":\"noavl\",\"value\":\"Hello, World!\"}]}");
        do_test("{\"serviceBlocks\":[{\"name\":\"notexvar\",\"request\":\"set\",\"result\":\"notex\",\"value\":\"Hello, World!\"}]}");

        // Test with a binary value
        do_test("{\"serviceBlocks\":[{\"name\":\"binaryvar\",\"request\":\"set\",\"result\":\"ok\",\"value\":{\"base64\":\"SGVsbG8sIFdvcmxkIQ\"}}]}");

        // Test with a null value
        do_test("{\"serviceBlocks\":[{\"name\":\"nullvar\",\"request\":\"set\",\"result\":\"ok\",\"value\":null}]}");

        // Test with a boolean value
        do_test("{\"serviceBlocks\":[{\"name\":\"truevar\",\"request\":\"set\",\"result\":\"ok\",\"value\":true}]}");
        do_test("{\"serviceBlocks\":[{\"name\":\"falsevar\",\"request\":\"set\",\"result\":\"ok\",\"value\":false}]}");

        // Test with two service blocks
        do_test("{\"serviceBlocks\":[{\"name\":\"hellovar\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\"},{\"name\":\"goodbyvar\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Goodbye, World!\"}]}");

        // Test with an integer value
        do_test("{\"serviceBlocks\":[{\"name\":\"integervar\",\"request\":\"set\",\"result\":\"ok\",\"value\":42}]}");

        // Test with a real value
        do_test("{\"serviceBlocks\":[{\"name\":\"realvar\",\"request\":\"set\",\"result\":\"ok\",\"value\":3.14159}]}");

        // Test with an object value
        do_test("{\"serviceBlocks\":[{\"name\":\"objectvar\",\"request\":\"set\",\"result\":\"ok\",\"value\":{\"class\":\"object\",\"members\":{\"hello\":\"world\",\"goodbye\":\"world\"}}}]}");

        // Test with an array value with two elements
        do_test("{\"serviceBlocks\":[{\"name\":\"arrayvar\",\"request\":\"set\",\"result\":\"ok\",\"value\":[{\"0\":\"hello\"},{\"1\":\"world\"}]}]}");

        // Test with an array of one object
        do_test("{\"serviceBlocks\":[{\"name\":\"arrayofobjectsvar\",\"request\":\"set\",\"result\":\"ok\",\"value\":[{\"0\":{\"class\":\"object\",\"members\":{\"hello\":\"world\"}}}]}]}");

        // Test with an array of two objects
        do_test("{\"serviceBlocks\":[{\"name\":\"arrayofobjectsvar\",\"request\":\"set\",\"result\":\"ok\",\"value\":[{\"0\":{\"class\":\"object\",\"members\":{\"hello\":\"world\"}}},{\"1\":{\"class\":\"object\",\"members\":{\"goodbye\":\"world\"}}}]}]}");

        // Test with an object with an array member
        do_test("{\"serviceBlocks\":[{\"name\":\"objectwitharrayvar\",\"request\":\"set\",\"result\":\"ok\",\"value\":{\"class\":\"object\",\"members\":{\"array\":[{\"0\":\"hello\"},{\"1\":\"world\"}]}}}]}");

        // Test with an object with an object member
        do_test("{\"serviceBlocks\":[{\"name\":\"objectwithobjectvar\",\"request\":\"set\",\"result\":\"ok\",\"value\":{\"class\":\"object\",\"members\":{\"object\":{\"class\":\"object\",\"members\":{\"hello\":\"world\"}}}}}]}");

}

int main() {
    // Run the test
    test_emitparseJSON();

    return 0;
}