//
// Created by Adrian Sutherland on 24/04/2024.
//
#include <assert.h>
#include <string.h>
#include <stdlib.h>
//#include "crexxsaa.h"
#include "httpclient.h"

void test_parseJSON() {

    // Basic Smoke Test
    // Create a JSON string - formatted to be human-readable
    char* json =
            "{"
                "\"serviceBlocks\": ["
                    "{"
                        "\"name\": \"test\","
                        "\"request\": \"set\","
                        "\"result\": \"ok\","
                        "\"value\": \"Hello, World!\""
                    "}"
                "]"
            "}";

    // Copy it into a dynamic buffer (parseJSON mutates the buffer)
    char* json_copy = malloc(strlen(json) + 1);
    strcpy(json_copy, json);

    // Call the parseJSON function
    SHVBLOCK* shvblock = NULL;
    int result = parseJSON(json_copy, &shvblock);

    // Check the result
    assert(result == 0); // Ensure parseJSON returned success

    // Check the SHVBLOCK structure
    assert(shvblock != NULL); // Ensure a SHVBLOCK was returned
    assert(strcmp(shvblock->shvname, "test") == 0); // Check the name
    assert(shvblock->shvcode == RXSHV_SET); // Check the request code
    assert(shvblock->shvret == RXSHV_OK); // Check the result code
    assert(shvblock->shvobject->type == VALUE_STRING); // Check the value type
    assert(strcmp(shvblock->shvobject->value.string, "Hello, World!") == 0); // Check the value

    // Free the SHVBLOCK structure (this also frees the JSON buffer)
    FreeRexxVariablePool(shvblock);

    // Tests with a valid CREXX serviceBlocks JSON strings that includes all possible types of values (string, number, boolean, binary, object, array, null).

    // Test with a valid CREXX serviceBlocks JSON string with a string value.
    json = "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":\"Hello, World!\"}]}";
    json_copy = malloc(strlen(json) + 1);
    strcpy(json_copy, json);
    result = parseJSON(json_copy, &shvblock);
    assert(result == 0);
    assert(shvblock != NULL);
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    assert(shvblock->shvobject->type == VALUE_STRING);
    assert(strcmp(shvblock->shvobject->value.string, "Hello, World!") == 0);
    FreeRexxVariablePool(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a integer value.
    json = "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":123}]}";
    json_copy = malloc(strlen(json) + 1);
    strcpy(json_copy, json);
    result = parseJSON(json_copy, &shvblock);
    assert(result == 0);
    assert(shvblock != NULL);
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    assert(shvblock->shvobject->type == VALUE_INT);
    assert(shvblock->shvobject->value.integer == 123);
    FreeRexxVariablePool(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a real value.
    json = "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":123.456}]}";
    json_copy = malloc(strlen(json) + 1);
    strcpy(json_copy, json);
    result = parseJSON(json_copy, &shvblock);
    assert(result == 0);
    assert(shvblock != NULL);
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    assert(shvblock->shvobject->type == VALUE_FLOAT);
    assert(shvblock->shvobject->value.real == 123.456);
    FreeRexxVariablePool(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a boolean value.
    json = "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":true}]}";
    json_copy = malloc(strlen(json) + 1);
    strcpy(json_copy, json);
    result = parseJSON(json_copy, &shvblock);
    assert(result == 0);
    assert(shvblock != NULL);
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    assert(shvblock->shvobject->type == VALUE_BOOL);
    assert(shvblock->shvobject->value.boolean == 1);
    FreeRexxVariablePool(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a null value.
    json = "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":null}]}";
    json_copy = malloc(strlen(json) + 1);
    strcpy(json_copy, json);
    result = parseJSON(json_copy, &shvblock);
    assert(result == 0);
    assert(shvblock != NULL);
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    assert(shvblock->shvobject->type == VALUE_NULL);
    FreeRexxVariablePool(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a binary value.
    // Note binary data is base64 encoded in a JSON object like this:
    // The string "string binary data" (length 18) is base64 encoded as "c3RyaW5nIGJpbmFyeSBkYXRh"
    // {
    //  "base64": "c3RyaW5nIGJpbmFyeSBkYXRh"
    //  }
    json = "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":{\"base64\":\"c3RyaW5nIGJpbmFyeSBkYXRh\"}}]}";
    json_copy = malloc(strlen(json) + 1);
    strcpy(json_copy, json);
    result = parseJSON(json_copy, &shvblock);
    assert(result == 0);
    assert(shvblock != NULL);
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    assert(shvblock->shvobject->type == VALUE_BINARY);
    assert(shvblock->shvobject->value.binary.length == 18);
    assert(strncmp(shvblock->shvobject->value.binary.data, "string binary data", 18) == 0);
    FreeRexxVariablePool(shvblock);

    // Test with a valid CREXX serviceBlocks JSON string with a object value.
    // Note the object is nested within the value and we will use a complex number example
    // {
    //     "class": "complex_number",
    //     "members": {
    //        "real": 3.2,
    //        "imaginary": 4.5
    //    }
    // }
    json = "{\"serviceBlocks\":[{\"name\":\"test\",\"request\":\"set\",\"result\":\"ok\",\"value\":{\"class\":\"complex_number\",\"members\":{\"real\":3.2,\"imaginary\":4.5}}}]}";
    json_copy = malloc(strlen(json) + 1);
    strcpy(json_copy, json);
    result = parseJSON(json_copy, &shvblock);
    assert(result == 0);
    assert(shvblock != NULL);
    assert(strcmp(shvblock->shvname, "test") == 0);
    assert(shvblock->shvcode == RXSHV_SET);
    assert(shvblock->shvret == RXSHV_OK);
    assert(shvblock->shvobject->type == VALUE_OBJECT);
    assert(strcmp(shvblock->shvobject->value.members->membername, "class") == 0);
    assert(shvblock->shvobject->value.members->memberobject->type == VALUE_STRING);
    assert(strcmp(shvblock->shvobject->value.members->memberobject->value.string, "complex_number") == 0);
    assert(strcmp(shvblock->shvobject->value.members->membernext->membername, "members") == 0);
    assert(shvblock->shvobject->value.members->membernext->memberobject->type == VALUE_OBJECT);
    assert(strcmp(shvblock->shvobject->value.members->membernext->memberobject->value.members->membername, "real") == 0);
    assert(shvblock->shvobject->value.members->membernext->memberobject->value.members->memberobject->type == VALUE_FLOAT);
    assert(shvblock->shvobject->value.members->membernext->memberobject->value.members->memberobject->value.real == 3.2);
    assert(strcmp(shvblock->shvobject->value.members->membernext->memberobject->value.members->membernext->membername, "imaginary") == 0);
    assert(shvblock->shvobject->value.members->membernext->memberobject->value.members->membernext->memberobject->type == VALUE_FLOAT);
    assert(shvblock->shvobject->value.members->membernext->memberobject->value.members->membernext->memberobject->value.real == 4.5);
    FreeRexxVariablePool(shvblock);




}

int main() {
    // Run the test
    test_parseJSON();

    return 0;
}
