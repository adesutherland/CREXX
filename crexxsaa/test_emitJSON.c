//
// Created by Adrian Sutherland on 04/05/2024.
//
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <printf.h>
//#include "crexxsaa.h"
//#include "httpclient.h"
#include "jsnemit.h"

/* Helper for tests - Frees the SHVBLOCK Linked Lists and Children */
void FreeMemBlock(MEMBLOCK *memblock);
void FreeObjBlock(OBJBLOCK *shvobject);
void FreeSHVBLOCK(SHVBLOCK *shvblock);

/* Free a MEMBLOCK linked list */
void FreeMemBlock(MEMBLOCK *memblock) { // NOLINT(misc-no-recursion) - suppress the clang-tidy warning about recursion
    MEMBLOCK *memblock_next;

    while (memblock) {
        memblock_next = memblock->membernext;
        FreeObjBlock(memblock->memberobject);  // Recursively free the members
        free(memblock); // Free the object
        memblock = memblock_next;
    }
}

/* Free the OBJBLOCK */
void FreeObjBlock(OBJBLOCK *shvobject) { // NOLINT(misc-no-recursion) - suppress the clang-tidy warning about recursion
    if (!shvobject) return;
    if (shvobject->type == VALUE_OBJECT || shvobject->type == VALUE_ARRAY) {
        FreeMemBlock(shvobject->value.members); // Free the members
    }
    free(shvobject); // Free the object
}

// Free the OBJBLOCK linked list
void FreeSHVBLOCK(SHVBLOCK *shvblock) {
    SHVBLOCK *shvblock_next;

    /* Free the SHVBLOCK linked list */
    while (shvblock) {
        shvblock_next = shvblock->shvnext;
        FreeObjBlock(shvblock->shvobject); // Free the OBJBLOCK
        free(shvblock);
        shvblock = shvblock_next;
    }
}

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

// Test helper for different shvcodes
void test_shvcode(int shvcode, const char *expected_json_code) {
    MemoryBuffer *mem_context;
    SHVBLOCK *block;

    // Create the expected JSON
    char expected[1024];
    snprintf(expected, 1024, "{\"serviceBlocks\":[{\"name\":\"var\",\"request\":\"%s\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}", expected_json_code);

    // Create a SHVBLOCK
    block = malloc(sizeof(SHVBLOCK));
    block->shvname = "var";
    block->shvcode = shvcode;
    block->shvret = RXSHV_OK;
    block->shvnext = NULL;
    block->shvobject = malloc(sizeof(OBJBLOCK));
    block->shvobject->type = VALUE_STRING;
    block->shvobject->value.string = "Hello, World!";
    block->shvobject->typename = NULL;
    // Emit the JSON
    mem_context = NULL;
    jsonEMIT(block, emit_to_memory_buffer, (void*)&mem_context);
    // Check Output with expected value
    assert(compare_string_with_memory_buffer(expected, mem_context));
    // Cleanup
    FreeSHVBLOCK(block);
    emit_to_memory_buffer(ACTION_CLOSE, NULL, (void*)&mem_context);
}

// Test helper for different retcodes
void test_shvret(int shvret, const char *expected_json_code) {
    MemoryBuffer *mem_context;
    SHVBLOCK *block;

    // Create the expected JSON
    char expected[1024];
    snprintf(expected, 1024, "{\"serviceBlocks\":[{\"name\":\"var\",\"request\":\"set\",\"result\":\"%s\",\"value\":\"Hello, World!\"}]}", expected_json_code);

    // Create a SHVBLOCK
    block = malloc(sizeof(SHVBLOCK));
    block->shvname = "var";
    block->shvcode = RXSHV_SET;
    block->shvret = shvret;
    block->shvnext = NULL;
    block->shvobject = malloc(sizeof(OBJBLOCK));
    block->shvobject->type = VALUE_STRING;
    block->shvobject->value.string = "Hello, World!";
    block->shvobject->typename = NULL;
    // Emit the JSON
    mem_context = NULL;
    jsonEMIT(block, emit_to_memory_buffer, (void*)&mem_context);
    // Check Output with expected value
    assert(compare_string_with_memory_buffer(expected, mem_context));
    // Cleanup
    FreeSHVBLOCK(block);
    emit_to_memory_buffer(ACTION_CLOSE, NULL, (void*)&mem_context);
}


void test_emitJSON() {

        MemoryBuffer *mem_context;
        SHVBLOCK *block;

        // Smoke test for emitJSON - tests string value
        // - Create a SHVBLOCK
        block = malloc(sizeof(SHVBLOCK));
        block->shvname = "hellovar";
        block->shvcode = RXSHV_SET;
        block->shvret = RXSHV_OK;
        block->shvnext = NULL;
        block->shvobject = malloc(sizeof(OBJBLOCK));
        block->shvobject->type = VALUE_STRING;
        block->shvobject->value.string = "Hello, World!";
        block->shvobject->typename = NULL;
        // - Emit the JSON
        mem_context = NULL;
        // jsonEMIT(block, emit_to_stdout, NULL);
        jsonEMIT(block, emit_to_memory_buffer, (void*)&mem_context);
        // - Check Output with expected value
        assert(compare_string_with_memory_buffer("{\"serviceBlocks\":[{\"name\":\"hellovar\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}", mem_context));
        // - Cleanup
        FreeSHVBLOCK(block);
        emit_to_memory_buffer(ACTION_CLOSE, NULL, (void*)&mem_context);

        // Test with different shvcodes - "set","fetch","drop","nextv","priv","syset","syfet","sydro","sydel"
        test_shvcode(RXSHV_SET, "set");
        test_shvcode(RXSHV_FETCH, "fetch");
        test_shvcode(RXSHV_DROP, "drop");
        test_shvcode(RXSHV_NEXTV, "nextv");
        test_shvcode(RXSHV_PRIV, "priv");
        test_shvcode(RXSHV_SYSET, "syset");
        test_shvcode(RXSHV_SYFET, "syfet");
        test_shvcode(RXSHV_SYDRO, "sydro");
        test_shvcode(RXSHV_SYDEL, "sydel");

        // Test with different retcodes - "ok","newv","lvar","trunc","badn","memfl","badf","noavl","notex"
        test_shvret(RXSHV_OK, "ok");
        test_shvret(RXSHV_NEWV, "newv");
        test_shvret(RXSHV_LVAR, "lvar");
        test_shvret(RXSHV_TRUNC, "trunc");
        test_shvret(RXSHV_BADN, "badn");
        test_shvret(RXSHV_MEMFL, "memfl");
        test_shvret(RXSHV_BADF, "badf");
        test_shvret(RXSHV_NOAVL, "noavl");
        test_shvret(RXSHV_NOTEX, "notex");

        // Test with a binary value
        // - Create a SHVBLOCK
        block = malloc(sizeof(SHVBLOCK));
        block->shvname = "binaryvar";
        block->shvcode = 1;
        block->shvret = 0;
        block->shvnext = NULL;
        block->shvobject = malloc(sizeof(OBJBLOCK));
        block->shvobject->type = VALUE_BINARY;
        block->shvobject->value.binary.data = "Hello, World!";
        // Note that the base64 encoded value is "SGVsbG8sIFdvcmxkIQ" (without the trailing ==)
        block->shvobject->value.binary.length = 13;
        block->shvobject->typename = NULL;
        // - Emit the JSON
        mem_context = NULL;
        // jsonEMIT(block, emit_to_stdout, NULL); // For debugging
        jsonEMIT(block, emit_to_memory_buffer, (void*)&mem_context);
        // - Check Output with expected value
        assert(compare_string_with_memory_buffer("{\"serviceBlocks\":[{\"name\":\"binaryvar\",\"request\":\"set\",\"result\":\"ok\",\"value\":{\"base64\":\"SGVsbG8sIFdvcmxkIQ\"}}]}", mem_context));
        // - Cleanup
        FreeSHVBLOCK(block);
        emit_to_memory_buffer(ACTION_CLOSE, NULL, (void*)&mem_context);

        // Test with NULL value
        // - Create a SHVBLOCK
        block = malloc(sizeof(SHVBLOCK));
        block->shvname = "nullvar";
        block->shvcode = 1;
        block->shvret = 0;
        block->shvnext = NULL;
        block->shvobject = malloc(sizeof(OBJBLOCK));
        block->shvobject->type = VALUE_NULL;
        block->shvobject->value.string = NULL;
        block->shvobject->typename = NULL;
        // - Emit the JSON
        mem_context = NULL;
        // jsonEMIT(block, emit_to_stdout, NULL); // For debugging
        jsonEMIT(block, emit_to_memory_buffer, (void*)&mem_context);
        // - Check Output with expected value
        assert(compare_string_with_memory_buffer("{\"serviceBlocks\":[{\"name\":\"nullvar\",\"request\":\"set\",\"result\":\"ok\",\"value\":null}]}", mem_context));
        // - Cleanup
        FreeSHVBLOCK(block);
        emit_to_memory_buffer(ACTION_CLOSE, NULL, (void*)&mem_context);

        // Test with true value
        // - Create a SHVBLOCK
        block = malloc(sizeof(SHVBLOCK));
        block->shvname = "truevar";
        block->shvcode = 1;
        block->shvret = 0;
        block->shvnext = NULL;
        block->shvobject = malloc(sizeof(OBJBLOCK));
        block->shvobject->type = VALUE_BOOL;
        block->shvobject->value.boolean = 1;
        block->shvobject->typename = NULL;
        // - Emit the JSON
        mem_context = NULL;
        // jsonEMIT(block, emit_to_stdout, NULL); // For debugging
        jsonEMIT(block, emit_to_memory_buffer, (void*)&mem_context);
        // - Check Output with expected value
        assert(compare_string_with_memory_buffer("{\"serviceBlocks\":[{\"name\":\"truevar\",\"request\":\"set\",\"result\":\"ok\",\"value\":true}]}", mem_context));
        // - Cleanup
        FreeSHVBLOCK(block);
        emit_to_memory_buffer(ACTION_CLOSE, NULL, (void*)&mem_context);

        // Test with false value
        // - Create a SHVBLOCK
        block = malloc(sizeof(SHVBLOCK));
        block->shvname = "falsevar";
        block->shvcode = 1;
        block->shvret = 0;
        block->shvnext = NULL;
        block->shvobject = malloc(sizeof(OBJBLOCK));
        block->shvobject->type = VALUE_BOOL;
        block->shvobject->value.boolean = 0;
        block->shvobject->typename = NULL;
        // - Emit the JSON
        mem_context = NULL;
        // jsonEMIT(block, emit_to_stdout, NULL); // For debugging
        jsonEMIT(block, emit_to_memory_buffer, (void*)&mem_context);
        // - Check Output with expected value
        assert(compare_string_with_memory_buffer("{\"serviceBlocks\":[{\"name\":\"falsevar\",\"request\":\"set\",\"result\":\"ok\",\"value\":false}]}", mem_context));
        // - Cleanup
        FreeSHVBLOCK(block);
        emit_to_memory_buffer(ACTION_CLOSE, NULL, (void*)&mem_context);

        // Test with two svhblocks
        // - Create a SHVBLOCK
        block = malloc(sizeof(SHVBLOCK));
        block->shvname = "var1";
        block->shvcode = RXSHV_SET;
        block->shvret = RXSHV_OK;
        block->shvnext = malloc(sizeof(SHVBLOCK));
        block->shvnext->shvname = "var2";
        block->shvnext->shvcode = RXSHV_SET;
        block->shvnext->shvret = RXSHV_OK;
        block->shvnext->shvnext = NULL;
        block->shvnext->shvobject = malloc(sizeof(OBJBLOCK));
        block->shvnext->shvobject->type = VALUE_STRING;
        block->shvnext->shvobject->value.string = "Hello, World!";
        block->shvnext->shvobject->typename = NULL;
        block->shvobject = malloc(sizeof(OBJBLOCK));
        block->shvobject->type = VALUE_STRING;
        block->shvobject->value.string = "Hello, World!";
        block->shvobject->typename = NULL;
        // - Emit the JSON
        mem_context = NULL;
        // jsonEMIT(block, emit_to_stdout, NULL); // For debugging
        jsonEMIT(block, emit_to_memory_buffer, (void*)&mem_context);
        // - Check Output with expected value
        assert(compare_string_with_memory_buffer("{\"serviceBlocks\":[{\"name\":\"var1\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\"},{\"name\":\"var2\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}", mem_context));
        // - Cleanup
        FreeSHVBLOCK(block);
        emit_to_memory_buffer(ACTION_CLOSE, NULL, (void*)&mem_context);

        // Test with an integer value
        // - Create a SHVBLOCK
        block = malloc(sizeof(SHVBLOCK));
        block->shvname = "intvar";
        block->shvcode = 1;
        block->shvret = 0;
        block->shvnext = NULL;
        block->shvobject = malloc(sizeof(OBJBLOCK));
        block->shvobject->type = VALUE_INT;
        block->shvobject->value.integer = 123;
        block->shvobject->typename = NULL;
        // - Emit the JSON
        mem_context = NULL;
        // jsonEMIT(block, emit_to_stdout, NULL); // For debugging
        jsonEMIT(block, emit_to_memory_buffer, (void*)&mem_context);
        // - Check Output with expected value
        assert(compare_string_with_memory_buffer("{\"serviceBlocks\":[{\"name\":\"intvar\",\"request\":\"set\",\"result\":\"ok\",\"value\":123}]}", mem_context));
        // - Cleanup
        FreeSHVBLOCK(block);
        emit_to_memory_buffer(ACTION_CLOSE, NULL, (void*)&mem_context);

        // Test with a float value
        // - Create a SHVBLOCK
        block = malloc(sizeof(SHVBLOCK));
        block->shvname = "floatvar";
        block->shvcode = 1;
        block->shvret = 0;
        block->shvnext = NULL;
        block->shvobject = malloc(sizeof(OBJBLOCK));
        block->shvobject->type = VALUE_FLOAT;
        block->shvobject->value.real = 123.456;
        block->shvobject->typename = NULL;
        // - Emit the JSON
        mem_context = NULL;
        // jsonEMIT(block, emit_to_stdout, NULL); // For debugging
        jsonEMIT(block, emit_to_memory_buffer, (void*)&mem_context);
        // - Check Output with expected value
        assert(compare_string_with_memory_buffer("{\"serviceBlocks\":[{\"name\":\"floatvar\",\"request\":\"set\",\"result\":\"ok\",\"value\":123.456}]}", mem_context));
        // - Cleanup
        FreeSHVBLOCK(block);
        emit_to_memory_buffer(ACTION_CLOSE, NULL, (void*)&mem_context);

        // Test with an object value
        // - Create a SHVBLOCK
        block = malloc(sizeof(SHVBLOCK));
        block->shvname = "objectvar";
        block->shvcode = 1;
        block->shvret = 0;
        block->shvnext = NULL;
        block->shvobject = malloc(sizeof(OBJBLOCK));
        block->shvobject->type = VALUE_OBJECT;
        block->shvobject->typename = "testclass";
        block->shvobject->value.members = malloc(sizeof(MEMBLOCK));
        block->shvobject->value.members->membername = "member1";
        block->shvobject->value.members->memberobject = malloc(sizeof(OBJBLOCK));
        block->shvobject->value.members->memberobject->type = VALUE_STRING;
        block->shvobject->value.members->memberobject->value.string = "Hello, World!";
        block->shvobject->value.members->memberobject->typename = NULL;
        // - Emit the JSON
        mem_context = NULL;
        // jsonEMIT(block, emit_to_stdout, NULL); // For debugging
        jsonEMIT(block, emit_to_memory_buffer, (void*)&mem_context);
        // - Check Output with expected value
        assert(compare_string_with_memory_buffer("{\"serviceBlocks\":[{\"name\":\"objectvar\",\"request\":\"set\",\"result\":\"ok\",\"value\":{\"class\":\"testclass\",\"members\":{\"member1\":\"Hello, World!\"}}}]}", mem_context));
        // - Cleanup
        FreeSHVBLOCK(block);
        emit_to_memory_buffer(ACTION_CLOSE, NULL, (void*)&mem_context);

        // Test with an array value
        // - Create a SHVBLOCK
        block = malloc(sizeof(SHVBLOCK));
        block->shvname = "arrayvar";
        block->shvcode = 1;
        block->shvret = 0;
        block->shvnext = NULL;
        block->shvobject = malloc(sizeof(OBJBLOCK));
        block->shvobject->type = VALUE_ARRAY;
        block->shvobject->typename = NULL;
        block->shvobject->value.members = malloc(sizeof(MEMBLOCK));
        block->shvobject->value.members->membername = "0";
        block->shvobject->value.members->memberobject = malloc(sizeof(OBJBLOCK));
        block->shvobject->value.members->memberobject->type = VALUE_STRING;
        block->shvobject->value.members->memberobject->value.string = "Hello, World!";
        block->shvobject->value.members->memberobject->typename = NULL;
        // - Emit the JSON
        mem_context = NULL;
        // jsonEMIT(block, emit_to_stdout, NULL); // For debugging
        jsonEMIT(block, emit_to_memory_buffer, (void*)&mem_context);
        // - Check Output with expected value
        assert(compare_string_with_memory_buffer("{\"serviceBlocks\":[{\"name\":\"arrayvar\",\"request\":\"set\",\"result\":\"ok\",\"value\":[{\"0\":\"Hello, World!\"}]}]}", mem_context));
        // - Cleanup
        FreeSHVBLOCK(block);
        emit_to_memory_buffer(ACTION_CLOSE, NULL, (void*)&mem_context);








}

int main() {
    // Run the test
    test_emitJSON();

    return 0;
}