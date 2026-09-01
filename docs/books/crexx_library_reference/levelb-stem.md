## .stem class

This document defines the stable Level B `.stem` and `.stemIterator` contract.

### Overview
A `stem` is a string-to-string dictionary supporting cREXX property and bracket
notation for Classic-style compound-variable tails. A missing key returns the
assigned stem default, or the empty string before a default is assigned. Use
`size()` or the key arrays when absence must be distinguished from an assigned
empty value.

### API

- `get(key = .string) = .string` returns the tail value, the stem default, or
  the empty string.
- `set(key = .string, value = .string) = .void` inserts or replaces a value.
  Property assignment with an omitted key, `stem. = value`, assigns the Classic
  stem default instead; an explicitly supplied empty key is still a normal key.
  Assigning a default also replaces all values belonging to existing tails.
- `size() = .int` returns the number of distinct keys.
- `key(index = .int) = .string`, `value(index = .int) = .string`, and
  `valueAt(index = .int) = .string` access the internal one-based insertion
  arrays. An index outside `1..size()` signals `INVALID_ARGUMENTS`.
- `tails() = .string[]` and `values() = .string[]` return independent arrays.
- `iterator() = .stemIterator` creates a live iterator;
  `snapshotIterator() = .stemIterator` creates a snapshot.

The iterator factory accepts only snapshot flags `0` and `1`; another flag
signals `INVALID_ARGUMENTS`.

### Implementation (hash map with separate chaining)

The implementation uses parallel arrays to manage a 256-bucket hash map:

*   `buckets`: An array of 256 integers pointing to the head of a chain.
*   `keys`: A flat array storing string keys.
*   `vals`: A flat array storing string values.
- `value_generations`: the default generation in which each explicit value was
  assigned.
- `next`: An integer array storing the index of the next item in the chain.

Stem-default assignment is constant time. It stores the new default and advances
the default generation; older explicit values then read as the default without
rewriting the value array. A later explicit tail assignment records the current
generation and becomes visible again. Indexed access, extracted values, and
iterators all resolve the effective value through the same generation rule.

The hash loop reads Unicode codepoints directly with VM string instructions and
uses a multiplier of 31. Each step is reduced modulo 256, which is equivalent
for the final bucket and prevents native integer overflow for long keys.

### Usage
Stems can be accessed using standard `get` and `set` methods, but the preferred approach is using cREXX object property syntax (`obj.key` or `obj["key"]`) which is automatically rewritten to method calls by the compiler.

```rexx
s = .stem()

s. = "fallback"
say s.unknown             /* fallback */

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

### Iteration

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

### Performance

Hashing uses direct `strlen` and `strchar` VM operations and makes no Level B
selector calls. Lookup and update use separate bucket chains with early exit.
The implementation remains Level B so it can evolve with the standard typed
array surface; no native-C replacement is required by this contract.

### Notes on Character Encoding
VM string parsing exposes Unicode codepoints rather than UTF-8 bytes, so equal
Unicode keys hash consistently regardless of their encoded byte width.
