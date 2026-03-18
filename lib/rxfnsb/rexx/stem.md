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

## Known Limitations and Discovered Bugs
During the initial implementation, the following VM and Compiler issues were discovered and tracked for future fixes:

1. **Class Array Property Shadowing**: Directly initializing an array property in a method (e.g., `keys = .string[]`) or indexing it (`keys[1] = ...`) generates invalid assembler instructions (`minattrs r-1`). 
   * *Workaround used*: Arrays are initialized via local variable proxies (`t_keys = .string[]; keys = t_keys`).
2. **Dynamic Array Allocation in Classes**: Accessing an uninitialized or dynamically growing array index inside a class causes the `rxvm` interpreter to Segfault.
   * *Workaround used*: Arrays are statically pre-allocated to size `1024` for now (`t_keys = .string[1024]`).
3. **String/Assembler Instructions in Class Methods Segfault**: Calling string built-in functions (like `length()`, `substr()`, `c2d()`) or string-related assembler instructions (`assembler strlen`, `assembler hexchar`) inside a class method crashes `rxvm` with a Segmentation Fault (Exit Code 139).
   * *Workaround used*: The `hash` method is temporarily hardcoded to return `1`. This validates the linear chaining algorithm's logic correctly, but reduces performance to O(N) until the VM bug is resolved.
4. **UTF-8 to UTF-32 Conversion**: Note that assembler instructions for string parsing convert internal UTF-8 to UTF-32 characters. Length is calculated in codepoints, not bytes. This must be considered when writing Unicode tests for the hash function once the VM string parsing bug inside class methods is resolved.