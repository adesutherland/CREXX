# Level B `index`

`index` is the VM/TSO-compatible haystack-first spelling of substring search:

```rexx
index(haystack=.string, needle=.string [,start=.int]) = .int
```

It returns the one-based Unicode character position of the first exact,
case-sensitive occurrence at or after `start`, or zero. `start` defaults to
`1`.

```rexx
index("Saturday", "day") == 6
index("banana", "ana", 3) == 4
```

An empty needle or haystack returns zero. A non-positive `start` signals
`INVALID_ARGUMENTS`. Inputs are read-only.

The implementation validates once and executes the VM `strpos` instruction
directly. It performs no substring allocation, text copy, or call through POS;
only the public argument order differs. There is no Level C BIF or class method
named INDEX. `lib/rxfnsb/tests_functional/ts_index.crexx` covers the documented
surface.
