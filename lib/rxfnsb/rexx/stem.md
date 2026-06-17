# cREXX Level B Stem Class

This document details the internal design and usage of the `stem` class.

## Overview
A `stem` behaves as a string-to-string dictionary in cREXX Level B, supporting the classic REXX concept of compound variables (`stem.item`). 

## Implementation (Hash Map with Linear Chaining)
The implementation uses parallel arrays to manage a 256-bucket hash map with linear chaining for collision resolution:

*   `buckets`: An array of 256 integers pointing to the head of a chain.
*   `keys`: A flat array storing string keys.
*   `vals`: A flat array storing string values.
*   `next`: An integer array storing the index of the next item in the chain.

## Usage
Stems can be accessed using standard `get` and `set` methods, but the preferred approach is using cREXX object property syntax (`obj.key` or `obj["key"]`) which is automatically rewritten to method calls by the compiler.

```rexx
s = .stem()

/* Property syntax (Syntactic Sugar) */
s.key = "value"
val = s.key

/* Multi-tail property syntax preserves separators in the key */
customer = "acme"
invoice = "2026.05"
s.customer.invoice = "value4"  /* key is "acme.2026.05" */

/* Bracket notation for keys that aren't valid identifiers */
s["my-key"] = "value2"

/* Direct method calls */
call s.set("other_key", "value3")
val = s.get("other_key")
```

## Iteration

Stems are keyed containers, so iteration is over tails/keys rather than list
positions. Iteration order is unspecified. The current Rexx implementation
stores keys in insertion order, but callers must not depend on that ordering.

```rexx
s = .stem()
s.name = "Ada"
s.lang = "Rexx"

it = s.iterator()
loop while it.hasNext()
  tail = it.next()
  say tail "=" it.value()
end
```

`iterator()` returns an unsynchronized live iterator. It observes the current
stem through a weak reference; value updates after iterator creation are visible,
and newly added tails may be observed by the current implementation.

`snapshotIterator()` copies the stem's tails and values once in the iterator
factory. Use it when callers need stable factory-time contents while the stem
may be mutated.

The iterator API is:

* `hasNext() = .int`
* `next() = .string`, returning the next tail/key
* `value() = .string`, returning the value for the current tail
* `index() = .int`
* `reset() = .void`
* `isLive() = .int`

## Architecture & Future Optimizations
This Rexx implementation of the `stem` class is an **interim solution**.

In the future, to optimize performance, cREXX assembler assists (implemented in C) will be introduced to handle the heavy lifting:
*   **Hashing:** The hash function will be replaced with a faster native C assist.
*   **Indexing and Storage:** Array lookups and storage management will also be offloaded to C-level assists (exact details TBC).

## Notes on Character Encoding
Note that assembler instructions for string parsing convert internal UTF-8 to UTF-32 characters. Length is calculated in codepoints, not bytes. This impacts how the hash function processes Unicode strings.
