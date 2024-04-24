# CREXX "SAA" API

# Introduction

Welcome to the CREXX "SAA" API documentation. This document provides a comprehensive guide to the C API for the CREXX 
"SAA" API, a superset of the REXX SAA API. The API is designed to facilitate interaction between REXX programs and the
CREXX REXXSAA VM Server, a RESTful API server.

The document covers the key structures used in the API, namely `SHVBLOCK` and `OBJBLOCK`, and their respective fields.
It also provides an overview of the `RexxVariablePool` function, which serves as the interface to the REXX variable pool.

In addition, the document discusses the C CREXXSAA Library, which communicates with the CREXX REXXSAA Server Component
via the HTTP Transport / Restful API. The library's implementation philosophy, strategies for handling communication 
with the server, and its JSON parser capabilities are also covered.

Lastly, the document touches on the potential for clients developed in other languages to use the restful JSON API 
directly, using the language's native JSON parser and HTTP client libraries.

This document is intended for developers who are working with the CREXX "SAA" API, providing them with the necessary
technical details to effectively use and understand the API.

# Summary

The CREXX "SAA" API is a powerful tool that allows REXX programs to interact with the CREXX REXXSAA Server Component 
in a RESTful manner. It provides a simple, flexible, and easy-to-use interface for setting and retrieving variables, 
executing REXX commands, and performing other operations.

The API is designed to be language-agnostic, allowing clients to be developed in any language that supports JSON and 
HTTP. This makes it a versatile tool that can be used in a wide range of applications and environments.

By understanding the structures, functions, and libraries discussed in this document, developers can effectively 
leverage the capabilities of the CREXX "SAA" API in their REXX programs.

## JSON First Design
Translating the SHVBLOCK and Object structures to a JSON representation,
especially considering the flexibility and complexity they encapsulate, 
involves careful consideration to ensure the JSON structure is both
sensible and practical for typical usage scenarios, including RESTful
communications. 

This design is intended to reflect both simplicity for direct use
and flexibility for complex data

## Basic SHVBLOCK Representation in JSON
For a SHVBLOCK to access a couple of variables, the JSON is quite simple. 

```json
{
  "serviceBlocks": [
    {
      "name": "variable1",
      "request": "fetch"
    },
    {
      "name": "variable2",
      "request": "fetch"
    }
  ]
}
```
The result could look like this (for simple string values).

```json
{
  "serviceBlocks": [
    {
      "name": "variable1",
      "request": "fetch",
      "result": "ok",
      "value": "stringValue1"
    },
    {
      "name": "variable2",
      "request": "fetch",
      "result": "ok",
      "value": "stringValue2"
    }
  ]
}
```
## Request and result fields

These are based on the REXXSAA function and return code #defines.

### Request Values

- `"set"`    Set variable value
- `"fetch"`  Fetch variable value
- `"drop"`   Drop variable - "blanks/zeros" value
- `"nextv"`  Fetch next variable
- `"priv"`   Access private variable pool - not implementable in CREXX
- `"syset"`  Set symbolic variable - not implementable in CREXX
- `"syfet"`  Fetch symbolic variable - not implementable in CREXX
- `"sydro"`  Drop symbolic variable - not implementable in CREXX
- `"sydel"`  Delayed fetch of symbolic variable - not implementable in CREXX

### Result Values
The result values return the status of the request including any errors.

- `"ok"`     Successful completion
- `"newv"`   New variable created - not implementable in CREXX
- `"lvar"`   Last variable retrieved
- `"trunc"`  Truncated value - not expected to be ever returned
- `"badn"`   Invalid variable name
- `"memfl"`  Memory allocation failed - not expected to be ever returned
- `"badf"`   Invalid function code
- `"noavl"`  No more variables available
- `"notex"`  Variable does not exist

## SHVBLOCK Object Representation in JSON
SHVBLOCK request to access a variables. 

```json
{
  "serviceBlocks": [
    {
      "name": "complex_variable",
      "request": "fetch"
    }
  ]
}
```
The result could look like this.

```json
{
  "serviceBlocks": [
    {
      "name": "complex_variable",
      "request": "fetch",
      "result": "ok",
      "value": {
        "class": "complex_number",
        "members": {
            "real": 3.2,
            "imaginary": 4.5
        }
      }
    }
  ]
}
```

## SHVBLOCK Array Representation in JSON
Request:
```json
{
  "serviceBlocks": [
    {
      "name": "array_variable",
      "request": "fetch"
    }
  ]
}
```
For maps the JSON representation is similar to the object representation, but with an array of objects.

The result could look like this.

```json
{
  "serviceBlocks": [
    {
      "name": "array_variable",
      "request": "fetch",
      "result": "ok",
      "value": [
          {
            "1": {
              "class": "complex_number",
              "members": {
                "real": 3.2,
                "imaginary": 4.5
              }
            }
          },
        {
          "2": {
            "class": "complex_number",
            "members": {
              "real": 5.2,
              "imaginary": 4.8
            }
          }
        }
      ]
    }
  ]
}
```

## Value Types
The value element can be a 
- `string` ("string value")
- `number` (1234.56 - int or float)
- `boolean` (true/false)
- `binary` (base64 encoded - See Binary Representation, following)
- `object` (See REXX Object Representation, following)
- `array` (an array of value types [value1, value2, ...])
- `null` (null for an empty value)

## Binary Representation
The binary representation is a JSON object with a "base64" field with the encoded binary data. 

It is recommended to remove any trailing padding characters ('=').

```json
{
  "base64": "c3RyaW5nIGJpbmFyeSBkYXRh"
}
``` 

## REXX Object Representation

In the context of this API, a REXX object is represented as a JSON object. This JSON object contains two key fields: `class` and `members`.

- The `class` field is a string that denotes the class name of the REXX object.
- The `members` field is an array of JSON objects, each representing a member of the REXX object.

Each member object within the `members` array has two fields:

- The `name` field, a string that represents the name of the member.
- The `value` field, which holds the value of the member. The type of this field can vary and is one of the value types defined in the "Value Types" section.

Here's an example of how a REXX object is represented in JSON:

```json
{
    "class": "complex_number",
    "members": {
       "real": 3.2,
       "imaginary": 4.5
    }
}
```

In this example, a REXX object of class `complex_number` is represented. It has two members: `real` and `imaginary`, with respective values of `3.2` and `4.5`.

## Nested Objects and Collections

The objects and collections can be nested to any depth, and the JSON
representation will reflect this. This is nested example of a result.
    
```json
{
"serviceBlocks": [
    {
    "name": "nested_variable",
    "request": "fetch",
    "result": "ok",
    "value": {
        "class": "nested_object",
        "members": {
            "nested_member": {
                "class": "nested_object",
                "members": {
                    "nested_member": {
                        "class": "nested_object",
                        "members": {
                            "nested_member": "nested_value"
                        }
                    }
                }
            }
        }
    }
    }
]
}
```
## JSON Schema

Draft of a JSON Schema **(completely untested at this time!)**, this describes the structure of the JSON API for the
CREXX "SAA" API. It includes the `serviceBlocks` array, which contains objects that represent the `SHVBLOCK` structure.

Each object in the `serviceBlocks` array has `name`, `request`, and `result` fields, as well as a `value` field that
can be a string, number, boolean, binary data, object, array, or null. The object and array types are defined
recursively to allow for nested objects and collections to any depth.

TODO - Add Schema here

## HTTP Transport

HTTP/1.1 is used for the transport of the JSON data. The following sections describe the HTTP request and response formats,
HTTP headers, status codes, and long polling.

### HTTP Headers

The HTTP headers are used to pass metadata about the request and response. The following headers are used in the CREXX "SAA" API:
- Content-Type: application/json
- User-Agent: CREXX/0.1
- Keep-Alive: timeout=5, max=1000
- Connection: Keep-Alive
- Cache-Control: no-cache

Authorisation is handled using a Bearer token (see Environmental Variables) in the Authorisation header:
- Authorization: Bearer <session-id>

### HTTP Status Codes

The HTTP status codes are used to indicate the success or failure of the
overlying HTTP request. The following status codes are used:
- 200 OK
- 204 No Content (for long polling)
- 400 Bad Request
- 500 Internal Server Error

### Long Polling

Callbacks from the REXXSAA Server Component (e.g. for
subcommand handlers) are implemented using long-polling (i.e. the client sends a request and waits for a response), looping
until a response is received. 

The client sends a request to the server, and the server holds the request until a response
is ready. The client then receives the response and sends another request, repeating the process.

## Environment Variables

Environment variables are used to store configuration information that is used by the client to communicate with the server.
- `CREXX_SAA_SESSION_ID`: This is used to store the session ID, which is also a shared secret between the client and server (see HTTP Headers).
- `CREXX_SAA_SERVER_URL`: This is used to store the server URL, which includes the protocol, host, and port details.
 
## CREXX "SAA" C API

The `crexxsaa.h` file defines the C API for the CREXX "SAA" API. This API is a superset of the REXX SAA API and is 
designed to be used with the CREXX REXXSAA VM Server, a RESTful API server that allows REXX programs to interact with 
the server to set and retrieve variables, execute REXX commands, and perform other operations.

The `crexxsaa.h` file defines two main structures: `SHVBLOCK` and `OBJBLOCK`.

### SHVBLOCK

The `SHVBLOCK` structure is used for sharing variable data. It contains the following fields:

- `shvnext`: Pointer to the next `SHVBLOCK`.
- `shvname`: Null-terminated string representing the variable name.
- `shvvalue`: Null-terminated string representing the variable value. This field is used for backward compatibility.
- `shvobject`: Pointer to an array of `OBJBLOCK` structures representing nested objects or collections. 
Only one of `shvvalue` or `shvobject` should be used.
- `shvcode`: Function code.
- `shvret`: Return code.

The `shvcode` field can take one of several function codes, such as `RXSHV_SET` for setting a variable value, `RXSHV_FETCH` 
for fetching a variable value, and `RXSHV_DROP` for dropping a variable.

The `shvret` field is used to store the return code of a function, indicating the status of the function, including 
any errors that occurred.

### OBJBLOCK

The `OBJBLOCK` structure is used for representing nested objects or collections. It contains the following fields:

- `type`: Type of the value, see VALUE TYPES.
- `typeName`: Name of the type, used only for `OBJBLOCK` type values.
- `value`: Union representing the value, which can be a string, binary data, integer, float, bool, or an array of `MEMBLOCK` structures.

The `crexxsaa.h` file also provides the `RexxVariablePool` function, which serves as the interface to the REXX variable pool. 
This function takes a pointer to a `SHVBLOCK` structure and returns a status code.

### MEMBLOCK

The `MEMBLOCK` structure is used for representing an object member. It contains the following fields:

- `membernext`: Pointer to the next `MEMBLOCK`.
- `membername`: Name of the member.
- `membervalue`: Pointer to an `OBJBLOCK` structure representing the value of the member.

## C CREXXSAA Library

This library communicates to the CREXX REXXSAA Server Component via a the HTTP Transport / Restful API described herein.
The library has different strategies for handling the communication with the server, including:
- TCP/IP Sockets
- FILE Handles
- Named Pipes
- Secure Shell (SSH)

The implementation philosophy is to keep the client as simple as possible with no external dependencies 
(except for optional libraries such as SSL). The client is not designed to be a full-featured HTTP client but is only
designed to work with the CREXX REXXSAA Server Component.

Blocking I/O is used for simplicity, single-threaded, except callbacks from the REXXSAA Server Component (e.g. for 
subcommand handlers) which are implemented using long-polling (i.e. the client sends a request and waits for a response) 
and in separate threads.

The JSON parser can only handle a subset of JSON, and the library is not designed to be a full-featured JSON parser. 
Specifically it only handles the JSON structures described in this document.

## Other Languages
It is envisaged that clients developed in other languages will use the restful JSON API described herein directly, using
the language's native JSON parser and HTTP client libraries. The API is designed to be simple and easy to use, and the
JSON structure is designed to be easy to parse and manipulate in any language that supports JSON.

Language support libraries may be developed in the future to provide a more idiomatic interface to the API in specific
languages, but this is not a priority at this time.

## Conclusion
The CREXX "SAA" API is designed to be simple, flexible, and easy to use. It provides a way for REXX programs to interact
with the CREXX REXXSAA Server Component in a RESTful manner, allowing them to set and retrieve variables, execute REXX
commands, and perform other operations. The API is designed to be language-agnostic, allowing clients to be developed in
any language that supports JSON and HTTP.

