# cREXX Level B Stem Class

This document details the internal design and usage of the `stem` class.

## Overview
A `stem` behaves as a string-to-string dictionary in cREXX Level B, supporting the classic REXX concept of compound variables (`stem.item`). 

## Implementation (Hash Map with Linear Chaining)
The implementation uses parallel arrays to manage a 256-bucket hash map with linear chaining for collision resolution:

*   `buckets`: An array of 256 integers pointing to the head of a chain.
*   `keys`: A flat array storing string keys.
*   `values`: A flat array storing string values.
*   `next`: An integer array storing the index of the next item in the chain.

## Usage
```rexx
s = .stem()
call s.set("key", "value")
val = s.get("key")
```

## Architecture & Future Optimizations
This Rexx implementation of the `stem` class is an **interim solution**.

In the future, to optimize performance, cREXX assembler assists (implemented in C) will be introduced to handle the heavy lifting:
*   **Hashing:** The hash function will be replaced with a faster native C assist.
*   **Indexing and Storage:** Array lookups and storage management will also be offloaded to C-level assists (exact details TBC).

## Notes on Character Encoding
Note that assembler instructions for string parsing convert internal UTF-8 to UTF-32 characters. Length is calculated in codepoints, not bytes. This impacts how the hash function processes Unicode strings.