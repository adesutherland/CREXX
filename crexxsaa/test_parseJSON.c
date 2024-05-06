//
// Created by Adrian Sutherland on 24/04/2024.
//
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <printf.h>
//#include "crexxsaa.h"
#include "httpclient.h"

// Helper function to execute a test - returns the SHVBLOCK structure (or NULL if an error occurred)
// Checks the expected result (non-conformance means it will return null but print an error message)
// error_position is the position in the JSON string where the error occurred - it is passed as an integer
// so we can set it to -1 if we don't want to check the position
SHVBLOCK* execute_test(char* test_id, int expected_result, int error_position, char *json) {

    // Copy json into a dynamic buffer (parseJSON mutates the buffer)
    char* json_copy = malloc(strlen(json) + 1);
    strcpy(json_copy, json);

    // Call the parseJSON function
    SHVBLOCK* shvblock = NULL;
    PARSE_ERROR error;
    int result = parseJSON(json_copy, &shvblock, &error);

    /* If there is an error, print the error message */
    if (result != expected_result) {
        printf("Test %s failed with error: %d (expected %d)\n", test_id, result, expected_result);
        printf("  Error: %s\n", error.message);
        printf("  Error code: %d\n", error.error_code);
        printf("  Position: %zu\n", error.position);
        // Print the JSON string snippet around the error
        size_t start = error.position > 10 ? error.position - 10 : 0;
        size_t end = error.position + 10 < strlen(json) ? error.position + 10 : strlen(json);
        printf("  JSON: %.*s\n", (int)(end - start), json + start);
        // Print a pointer to the error position
        printf("         %*s\n\n", (int)(error.position - start), "^");
        if (shvblock) FreeRexxVariablePoolResult(shvblock); // Free the SHVBLOCK structure if it was created - on an error it may not be created
        return NULL;
    }
    else if (error_position > -1) {
        if (error.position != error_position) {
            printf("Test %s failed with the expected error: %d but not error position\n", test_id, result);
            printf("  Error: %s\n", error.message);
            printf("  Error code: %d\n", error.error_code);
            printf("  Position: %zu (expected %d)\n", error.position, error_position);
            // Print the JSON string snippet around the error
            size_t start = error.position > 10 ? error.position - 10 : 0;
            size_t end = error.position + 10 < strlen(json) ? error.position + 10 : strlen(json);
            printf("  JSON: %.*s\n", (int)(end - start), json + start);
            // Print a pointer to the error position
            printf("         %*s\n\n", (int)(error.position - start), "^");
            if (shvblock) FreeRexxVariablePoolResult(shvblock); // Free the SHVBLOCK structure if it was created - on an error it may not be created
            return NULL;
        }
    }
    // If there was an expected error the SHVBLOCK structure may or may not be NULL - make it always NULL for consistency
    if (result != PARSE_ERROR_OK) {
        if (shvblock) FreeRexxVariablePoolResult(shvblock); // Free the SHVBLOCK structure if it was created - on an error it may not be created
        shvblock = NULL;
    }
    return shvblock;
}

void test_parseJSON() {
    SHVBLOCK* shvblock;

    // Basic Smoke Test
    shvblock = execute_test("Smoke Test", PARSE_ERROR_OK, -1,
            "{"
                "\"serviceBlocks\": ["
                    "{"
                        "\"name\": \"test\","
                        "\"request\": \"set\","
                        "\"result\": \"ok\","
                        "\"value\": \"Hello, World!\""
                    "}"
                "]"
            "}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(strcmp(shvblock->shvname, "test") == 0); // Check the name
    assert(shvblock->shvcode == RXSHV_SET); // Check the request code
    assert(shvblock->shvret == RXSHV_OK); // Check the result code
    assert(shvblock->shvobject->type == VALUE_STRING); // Check the value type
    assert(strcmp(shvblock->shvobject->value.string, "Hello, World!") == 0); // Check the value
    FreeRexxVariablePoolResult(shvblock); // Free the SHVBLOCK structure

    // Tests to test all valid and an invalid shvcode
    // Test with a valid CREXX serviceBlocks JSON string with a valid shvcode (RXSHV_SET).
    shvblock = execute_test("Shvcode RXSHV_SET Test", PARSE_ERROR_OK, -1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a valid shvcode (RXSHV_FETCH).
    shvblock = execute_test("Shvcode RXSHV_FETCH Test", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"fetch\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(shvblock->shvcode == RXSHV_FETCH);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a valid shvcode (RXSHV_DROP).
    shvblock = execute_test("Shvcode RXSHV_DROP Test", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"drop\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(shvblock->shvcode == RXSHV_DROP);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a valid shvcode (RXSHV_NEXTV).
    shvblock = execute_test("Shvcode RXSHV_NEXTV Test", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"nextv\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(shvblock->shvcode == RXSHV_NEXTV);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a valid shvcode (RXSHV_PRIV).
    shvblock = execute_test("Shvcode RXSHV_PRIV Test", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"priv\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(shvblock->shvcode == RXSHV_PRIV);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a valid shvcode (RXSHV_SYSET).
    shvblock = execute_test("Shvcode RXSHV_SYSET Test", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"syset\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(shvblock->shvcode == RXSHV_SYSET);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a valid shvcode (RXSHV_SYFET).
    shvblock = execute_test("Shvcode RXSHV_SYFET Test", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"syfet\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(shvblock->shvcode == RXSHV_SYFET);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a valid shvcode (RXSHV_SYDRO).
    shvblock = execute_test("Shvcode RXSHV_SYDRO Test", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"sydro\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(shvblock->shvcode == RXSHV_SYDRO);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a valid shvcode (RXSHV_SYDEL).
    shvblock = execute_test("Shvcode RXSHV_SYDEL Test", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"sydel\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(shvblock->shvcode == RXSHV_SYDEL);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with an invalid shvcode
    shvblock = execute_test("Shvcode Invalid Test", PARSE_ERROR_INVALID_REQUEST, 44,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"bad\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK is NULL

    // Tests to test all valid and an invalid shvret

    // Test with a valid CREXX serviceBlocks JSON string with a valid shvret (RXSHV_OK).
    shvblock = execute_test("Shvret RXSHV_OK Test", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(shvblock->shvret == RXSHV_OK);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a valid shvret (RXSHV_NEWV).
    shvblock = execute_test("Shvret RXSHV_NEWV Test", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"newv\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(shvblock->shvret == RXSHV_NEWV);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a valid shvret (RXSHV_LVAR).
    shvblock = execute_test("Shvret RXSHV_LVAR Test", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"lvar\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(shvblock->shvret == RXSHV_LVAR);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a valid shvret (RXSHV_TRUNC).
    shvblock = execute_test("Shvret RXSHV_TRUNC Test", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"trunc\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(shvblock->shvret == RXSHV_TRUNC);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a valid shvret (RXSHV_BADN).
    shvblock = execute_test("Shvret RXSHV_BADN Test", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"badn\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(shvblock->shvret == RXSHV_BADN);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a valid shvret (RXSHV_MEMFL).
    shvblock = execute_test("Shvret RXSHV_MEMFL Test", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"memfl\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(shvblock->shvret == RXSHV_MEMFL);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a valid shvret (RXSHV_BADF).
    shvblock = execute_test("Shvret RXSHV_BADF Test", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"badf\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(shvblock->shvret == RXSHV_BADF);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a valid shvret (RXSHV_NOAVL).
    shvblock = execute_test("Shvret RXSHV_NOAVL Test", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"noavl\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(shvblock->shvret == RXSHV_NOAVL);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a valid shvret (RXSHV_NOTEX).
    shvblock = execute_test("Shvret RXSHV_NOTEX Test", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"notex\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(shvblock->shvret == RXSHV_NOTEX);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with an invalid shvret
    shvblock = execute_test("Shvret Invalid Test", PARSE_ERROR_INVALID_RESULT, 59,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"badret\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK is NULL

    // Tests with a valid CREXX serviceBlocks JSON strings that includes all possible types of values (string, number, boolean, binary, object, array, null).

    // Test with a valid CREXX serviceBlocks JSON string with a string value.
    shvblock = execute_test("Value Test String", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\" }]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    assert(shvblock->shvobject->type == VALUE_STRING);
    assert(strcmp(shvblock->shvobject->value.string, "Hello, World!") == 0);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a string value using all the JSON escapes
    shvblock = execute_test("Value Test String Escapes", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"\\\"\\\\\\/\\b\\f\\n\\r\\t\\u0020\"}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    assert(shvblock->shvobject->type == VALUE_STRING);
    assert(strcmp(shvblock->shvobject->value.string, "\"\\/\b\f\n\r\t ") == 0);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a integer value.
    shvblock = execute_test("Value Test Integer", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":123}]}");
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    assert(shvblock->shvobject->type == VALUE_INT);
    assert(shvblock->shvobject->value.integer == 123);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a real value.
    shvblock = execute_test("Value Test Real", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":123.456789}]}");
    assert(shvblock != NULL);
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    assert(shvblock->shvobject->type == VALUE_FLOAT);
    assert(shvblock->shvobject->value.real == 123.456789);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a real value with an exponent.
    shvblock = execute_test("Value Test Real Exponent", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":1.23456789e+87}]}");
    assert(shvblock != NULL);
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    assert(shvblock->shvobject->type == VALUE_FLOAT);
    assert(shvblock->shvobject->value.real == 1.23456789e+87);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a boolean value (true).
    shvblock = execute_test("Value Test Boolean True", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":true}]}");
    assert(shvblock != NULL);
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    assert(shvblock->shvobject->type == VALUE_BOOL);
    assert(shvblock->shvobject->value.boolean == 1);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a boolean value (false).
    shvblock = execute_test("Value Test Boolean False", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":false}]}");
    assert(shvblock != NULL);
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    assert(shvblock->shvobject->type == VALUE_BOOL);
    assert(shvblock->shvobject->value.boolean == 0);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a null value.
    shvblock = execute_test("Value Test Null", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":null}]}");
    assert(shvblock != NULL);
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    assert(shvblock->shvobject->type == VALUE_NULL);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a binary value.
    // Note binary data is base64 encoded in a JSON object like this:
    // The string "string binary data" (length 18) is base64 encoded as "c3RyaW5nIGJpbmFyeSBkYXRh"
    // {
    //  "base64": "c3RyaW5nIGJpbmFyeSBkYXRh"
    //  }
    shvblock = execute_test("Value Test Binary", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":{\"base64\":\"c3RyaW5nIGJpbmFyeSBkYXRh\"}}]}");
    assert(shvblock != NULL);
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    assert(shvblock->shvobject->type == VALUE_BINARY);
    assert(shvblock->shvobject->value.binary.length == 18);
    assert(strncmp(shvblock->shvobject->value.binary.data, "string binary data", 18) == 0);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a object value.
    // Note the object is nested within the value and we will use a complex number example
    // {
    //     "class": "complex_number",
    //     "members": {
    //        "real": 3.2,
    //        "imaginary": 4.5
    //     }
    // }
    shvblock = execute_test("Value Test Object", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":{\"class\":\"complex_number\",\"members\":{\"real\":3.2,\"imaginary\":4.5}} }]}");
    assert(shvblock != NULL);
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    OBJBLOCK *object = shvblock->shvobject;
    assert(object->type == VALUE_OBJECT);
    assert(strcmp(object->typename, "complex_number") == 0);
    assert(object->value.members != NULL);
    MEMBLOCK *members = object->value.members;
    assert(strcmp(members->membername, "real") == 0);
    assert(members->memberobject->type == VALUE_FLOAT);
    assert(members->memberobject->value.real == 3.2);
    assert(members->membernext != NULL);
    members = members->membernext;
    assert(strcmp(members->membername, "imaginary") == 0);
    assert(members->memberobject->type == VALUE_FLOAT);
    assert(members->memberobject->value.real == 4.5);
    FreeRexxVariablePoolResult(shvblock);


    // Tests that include nested objects and arrays.

    // Test with a valid CREXX serviceBlocks JSON string with a object value with an embedded object.
    // Note the object is nested within the value and we will use an object with two complex numbers (start and finish) example
    // {
    //     "class": "line",
    //     "members": {
    //        "start": {
    //            "class": "complex_number",
    //            "members": {
    //                "real": 1.0,
    //                "imaginary": 2.0
    //            }
    //        },
    //        "finish": {
    //            "class": "complex_number",
    //            "members": {
    //                "real": 3.0,
    //                "imaginary": 4.0
    //            }
    //        }
    //     }
    // }
    shvblock = execute_test("Value Test Object with Embedded Object", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":{\"class\":\"line\",\"members\":{\"start\":{\"class\":\"complex_number\",\"members\":{\"real\":1.0,\"imaginary\":2.0}},\"finish\":{\"class\":\"complex_number\",\"members\":{\"real\":3.0,\"imaginary\":4.0}}}} }]}");
    assert(shvblock != NULL);
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    object = shvblock->shvobject;
    assert(object->type == VALUE_OBJECT);
    assert(strcmp(object->typename, "line") == 0);
    assert(object->value.members != NULL);
    members = object->value.members;
    assert(strcmp(members->membername, "start") == 0);
    assert(members->memberobject->type == VALUE_OBJECT);
    OBJBLOCK *start = members->memberobject;
    assert(strcmp(start->typename, "complex_number") == 0);
    assert(start->value.members != NULL);
    MEMBLOCK *start_members = start->value.members;
    assert(strcmp(start_members->membername, "real") == 0);
    assert(start_members->memberobject->type == VALUE_FLOAT);
    assert(start_members->memberobject->value.real == 1.0);
    assert(start_members->membernext != NULL);
    start_members = start_members->membernext;
    assert(strcmp(start_members->membername, "imaginary") == 0);
    assert(start_members->memberobject->type == VALUE_FLOAT);
    assert(start_members->memberobject->value.real == 2.0);
    members = members->membernext;
    assert(strcmp(members->membername, "finish") == 0);
    assert(members->memberobject->type == VALUE_OBJECT);
    OBJBLOCK *finish = members->memberobject;
    assert(strcmp(finish->typename, "complex_number") == 0);
    assert(finish->value.members != NULL);
    MEMBLOCK *finish_members = finish->value.members;
    assert(strcmp(finish_members->membername, "real") == 0);
    assert(finish_members->memberobject->type == VALUE_FLOAT);
    assert(finish_members->memberobject->value.real == 3.0);
    assert(finish_members->membernext != NULL);
    finish_members = finish_members->membernext;
    assert(strcmp(finish_members->membername, "imaginary") == 0);
    assert(finish_members->memberobject->type == VALUE_FLOAT);
    assert(finish_members->memberobject->value.real == 4.0);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a array value.
    // Note the array is nested within the value and we will use a list of strings example
    // e.g. "value": [{"1": "String 1"},{"2": "String 2"}]
    shvblock = execute_test("Value Test Array", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":[{\"1\":\"String 1\"},{\"2\":\"String 2\"}]}]}");
    assert(shvblock != NULL);
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    OBJBLOCK *array = shvblock->shvobject;
    assert(array->type == VALUE_ARRAY);
    MEMBLOCK *element = array->value.members;
    assert(strcmp(element->membername, "1") == 0);
    assert(element->memberobject->type == VALUE_STRING);
    assert(strcmp(element->memberobject->value.string, "String 1") == 0);
    element = element->membernext;
    assert(strcmp(element->membername, "2") == 0);
    assert(element->memberobject->type == VALUE_STRING);
    assert(strcmp(element->memberobject->value.string, "String 2") == 0);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a array value with an embedded object.
    // Note the array is nested within the value and we will use a list of complex numbers example
    // e.g. "value": [{"class": "complex_number","members": {"real": 1.0,"imaginary": 2.0}},{"class": "complex_number","members": {"real": 3.0,"imaginary": 4.0}}]
    shvblock = execute_test("Value Test Array with Embedded Object", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":["
                                    "{\"1\":{\"class\":\"complex_number\",\"members\":{\"real\":1.0,\"imaginary\":2.0}}},"
                                    "{\"2\":{\"class\":\"complex_number\",\"members\":{\"real\":3.0,\"imaginary\":4.0}}}"
                                "]}"
                            "]}");
    assert(shvblock != NULL);
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    array = shvblock->shvobject;
    assert(array->type == VALUE_ARRAY);
    element = array->value.members;
    assert(strcmp(element->membername, "1") == 0);
    assert(element->memberobject->type == VALUE_OBJECT);
    OBJBLOCK *element_object = element->memberobject;
    assert(element_object->value.members != NULL);
    MEMBLOCK *element_members = element_object->value.members;
    assert(strcmp(element_members->membername, "real") == 0);
    assert(element_members->memberobject->type == VALUE_FLOAT);
    assert(element_members->memberobject->value.real == 1.0);
    assert(element_members->membernext != NULL);
    element_members = element_members->membernext;
    assert(strcmp(element_members->membername, "imaginary") == 0);
    assert(element_members->memberobject->type == VALUE_FLOAT);
    assert(element_members->memberobject->value.real == 2.0);
    element = element->membernext;
    assert(strcmp(element->membername, "2") == 0);
    assert(element->memberobject->type == VALUE_OBJECT);
    element_object = element->memberobject;
    assert(element_object->value.members != NULL);
    element_members = element_object->value.members;
    assert(strcmp(element_members->membername, "real") == 0);
    assert(element_members->memberobject->type == VALUE_FLOAT);
    assert(element_members->memberobject->value.real == 3.0);
    assert(element_members->membernext != NULL);
    element_members = element_members->membernext;
    assert(strcmp(element_members->membername, "imaginary") == 0);
    assert(element_members->memberobject->type == VALUE_FLOAT);
    assert(element_members->memberobject->value.real == 4.0);
    FreeRexxVariablePoolResult(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with an object with a array member.
    // Note the array is nested within the object and we will use a list of strings example
    // e.g. "value": {"class":"arraywrapper",members{"array":[{"1": "String 1"},{"2": "String 2"}]}}
    shvblock = execute_test("Value Test Object with Array", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":{\"class\":\"arraywrapper\",\"members\":{\"array\":[{\"1\":\"String 1\"},{\"2\":\"String 2\"}]}} }]}");
    assert(shvblock != NULL);
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    object = shvblock->shvobject;
    assert(object->type == VALUE_OBJECT);
    assert(strcmp(object->typename, "arraywrapper") == 0);
    MEMBLOCK *array_member = object->value.members;
    assert(strcmp(array_member->membername, "array") == 0);
    assert(array_member->memberobject->type == VALUE_ARRAY);
    MEMBLOCK *array_element = array_member->memberobject->value.members;
    assert(strcmp(array_element->membername, "1") == 0);
    assert(array_element->memberobject->type == VALUE_STRING);
    assert(strcmp(array_element->memberobject->value.string, "String 1") == 0);
    array_element = array_element->membernext;
    assert(strcmp(array_element->membername, "2") == 0);
    assert(array_element->memberobject->type == VALUE_STRING);
    assert(strcmp(array_element->memberobject->value.string, "String 2") == 0);
    FreeRexxVariablePoolResult(shvblock);

    // Test with an empty JSON string as a value.
    shvblock = execute_test("Value Test Empty String", PARSE_ERROR_OK,-1,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"\"}]}");
    assert(shvblock != NULL);
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    assert(shvblock->shvobject->type == VALUE_STRING);
    assert(shvblock->shvobject->value.string[0] == '\0');
    FreeRexxVariablePoolResult(shvblock);

    // Tests with a JSON string that is not properly formatted (missing brackets, missing commas, etc.).

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_EXPECTING_OPEN_CURLY).
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_EXPECTING_OPEN_CURLY)", PARSE_ERROR_EXPECTING_OPEN_CURLY, 18,
                            "{\"serviceBlocks\":[\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_EXPECTING_SERVICE_BLOCKS).
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_EXPECTING_SERVICE_BLOCKS)", PARSE_ERROR_EXPECTING_SERVICE_BLOCKS, 2,
                            "{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\"}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_EXPECTING_STRING)
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_EXPECTING_STRING)", PARSE_ERROR_EXPECTING_STRING, 26,
                            "{\"serviceBlocks\":[{\"name\":test\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_EXPECTING_COLON).
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_EXPECTING_COLON)", PARSE_ERROR_EXPECTING_COLON, 25,
                            "{\"serviceBlocks\":[{\"name\"\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_EXPECTING_ARRAY)
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_EXPECTING_ARRAY)", PARSE_ERROR_EXPECTING_ARRAY, 17,
                            "{\"serviceBlocks\":{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_EXPECTING_CLOSE_CURLY)
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_EXPECTING_CLOSE_CURLY)", PARSE_ERROR_EXPECTING_CLOSE_CURLY, 92,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":[{\"1\":\"Hello, World!\"]}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_DUPLICATE_NAME)
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_DUPLICATE_NAME)", PARSE_ERROR_DUPLICATE_NAME, 34,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_DUPLICATE_REQUEST)
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_DUPLICATE_REQUEST)", PARSE_ERROR_DUPLICATE_REQUEST, 50,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_INVALID_REQUEST)
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_INVALID_REQUEST)", PARSE_ERROR_INVALID_REQUEST, 44,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"bad\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_DUPLICATE_RESULT)
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_DUPLICATE_RESULT)", PARSE_ERROR_DUPLICATE_RESULT, 64,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_INVALID_RESULT)
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_INVALID_RESULT)", PARSE_ERROR_INVALID_RESULT, 59,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"badret\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_DUPLICATE_VALUE)
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_DUPLICATE_VALUE)", PARSE_ERROR_DUPLICATE_VALUE, 88,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_INVALID_ATTRIBUTE)
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_INVALID_ATTRIBUTE)", PARSE_ERROR_INVALID_ATTRIBUTE, 64,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"bad\":\"Hello, World!\"}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_MISSING_NAME)
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_MISSING_NAME)", PARSE_ERROR_MISSING_NAME, 72,
                            "{\"serviceBlocks\":[{\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_DUPLICATE_CLASS) - i.e. to class names in an object
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_DUPLICATE_CLASS)", PARSE_ERROR_DUPLICATE_CLASS, 98,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":{\"class\":\"complex_number\",\"class\":\"complex_number\",\"members\":{\"real\":3.2,\"imaginary\":4.5}} }]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_DUPLICATE_MEMBERS) - i.e. two members blocks in an object
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_DUPLICATE_MEMBERS)", PARSE_ERROR_DUPLICATE_MEMBERS, 137,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":{\"class\":\"complex_number\",\"members\":{\"real\":3.2,\"imaginary\":4.5},\"members\":{\"real\":3.2,\"imaginary\":4.5}} }]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_MISSING_CLASS) - i.e. missing class name in an object
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_MISSING_CLASS)", PARSE_ERROR_MISSING_CLASS, 110,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":{\"members\":{\"real\":3.2,\"imaginary\":4.5}} }]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_MISSING_MEMBERS) - i.e. missing members block in an object
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_MISSING_MEMBERS)", PARSE_ERROR_MISSING_MEMBERS, 96,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":{\"class\":\"complex_number\"} }]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_INVALID_TYPE)
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_INVALID_TYPE)", PARSE_ERROR_INVALID_TYPE, 71,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":bad}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_UNEXPECTED_TOKEN) e.g. using "nuller" instead of "null"
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_UNEXPECTED_TOKEN)", PARSE_ERROR_UNEXPECTED_TOKEN, 71,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":nuller}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_EXPECTING_BASE64) - i.e. missing base64 attribute in a binary object
    // Note this can't be checked because the error code cannot be returned in the current implementation - it would assume an invalid object type

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_INVALID_BASE64)
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_INVALID_BASE64)", PARSE_ERROR_INVALID_BASE64, 82,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":{\"base64\":\"-c3Rya-W5nIGJpbmFyeSBkYXRh\"}}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_INVALID_STRING) - i.e. invalid escape sequence
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_INVALID_STRING)", PARSE_ERROR_INVALID_STRING, 71,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\\x\"}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_EXPECTED_COMMA_OR_CLOSE_CURLY)
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_EXPECTED_COMMA_OR_CLOSE_CURLY)", PARSE_ERROR_EXPECTED_COMMA_OR_CLOSE_CURLY, 86,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\"]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

    // Test with a JSON string that is not properly formatted (PARSE_ERROR_EXPECTED_COMMA_OR_CLOSE_ARRAY)
    shvblock = execute_test("Invalid JSON Test (PARSE_ERROR_EXPECTED_COMMA_OR_CLOSE_ARRAY)", PARSE_ERROR_EXPECTED_COMMA_OR_CLOSE_ARRAY, 93,
                            "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":[{\"1\":\"Hello, World!\"}}]}");
    assert(shvblock == NULL); // Ensure a SHVBLOCK was not returned

}

int main() {
    // Run the test
    test_parseJSON();

    return 0;
}
