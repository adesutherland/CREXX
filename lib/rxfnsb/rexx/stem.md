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
During the initial implementation, the following VM and Compiler issues were discovered. Most have now been resolved:

1. **Class Array Property Shadowing (RESOLVED)**: Directly initializing an array property in a method (e.g., `keys = .string[]`) or indexing it (`keys[1] = ...`) generated invalid assembler instructions (`minattrs r-1`). This has been fixed in the compiler by appropriately assigning a temporary register for class property arrays.
2. **Dynamic Array Allocation in Classes (RESOLVED)**: It was initially reported that accessing an uninitialized or dynamically growing array index inside a class caused the `rxvm` interpreter to Segfault. Upon further investigation, this segfault was actually misattributed to dynamic arrays, and was in fact caused by the string operations bug below. Dynamic array growth on properties works perfectly.
3. **Internal Method Calls Missing Context (RESOLVED)**: Calling sibling methods internally (like `hash(key)`) without a `.object` prefix failed to pass the implicit `§this` context. This led to a mismatch in argument registers where `rxvm` encountered a NULL register and threw a Segmentation Fault. The compiler has been updated to implicitly rewrite internal method calls into proper `MEMBER_CALL` nodes.
4. **UTF-8 to UTF-32 Conversion**: Note that assembler instructions for string parsing convert internal UTF-8 to UTF-32 characters. Length is calculated in codepoints, not bytes. This must be considered when writing Unicode tests for the hash function.