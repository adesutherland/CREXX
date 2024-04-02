# Migrating to CREXX from Traditional REXX Implementations

## Type Definitions

| Old Type | New Type      | Description                                    |
|----------|---------------|------------------------------------------------|
| SHORT    | int16_t       | Signed 16-bit integer                          |
| ULONG    | uint32_t      | Unsigned 32-bit integer                        |
| LONG     | int32_t       | Signed 32-bit integer                          |
| etc.     |               |                                                |

*Recommended Action*: Replace occurrences of old type definitions with the corresponding new types from `<stdint.h>`. For example, replace `ULONG` with `uint32_t` to explicitly specify an unsigned 32-bit integer.

## Handling Strings

In transitioning your REXX scripts and applications to CREXX, a key consideration is the handling of string data. CREXX modernizes the approach to string management, moving away from the RXSTRING structure used in traditional REXX implementations to a simpler, more universally compatible char* format. This change aligns CREXX with contemporary programming practices and facilitates its integration with web technologies, including communication via REST/JSON.

### What Changes?

String Representation: CREXX uses null-terminated strings (char*), simplifying the handling and manipulation of text data. This approach is more straightforward for JSON serialization and deserialization, enhancing the efficiency of REST/JSON communication between the client library and the REXX interpreter.
Binary Data and Special Characters: Given the nature of null-terminated strings, representing binary data or strings containing special characters (e.g., embedded zeros) requires encoding. CREXX recommends using Base64 encoding for binary data to ensure compatibility and maintain data integrity during transmission.

### Migration Considerations

1. Updating String Handling: Migrate existing uses of RXSTRING to char*. This may involve adjusting how strings are passed to and from external functions and the interpreter. Ensure all strings are properly null-terminated.
2. Encoding Binary Data: For scripts and applications that manipulate binary data or strings containing special characters, implement Base64 encoding before transmitting data and decoding upon receipt. This ensures that such data is handled correctly within the CREXX environment.
3. Testing: Rigorously test migrated scripts to verify that string manipulations behave as expected, especially in scenarios involving data serialization and REST/JSON communication.

### Benefits

Adopting this modernized approach to string handling in CREXX not only simplifies the development process but also enhances compatibility with web technologies and streamlines the integration of REXX applications within contemporary IT ecosystems.
