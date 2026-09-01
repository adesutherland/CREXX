## delword

```rexx
delword(string = .string, start = .int [, count = .int]) = .string
```

`delword` returns `string` with words beginning at the positive one-based
`start` removed. An omitted `count` deletes through the final word; a supplied
zero returns the source unchanged, and a count larger than the available words
clamps to the remaining words. A start beyond the word count also returns the
source unchanged.

Deletion includes all whitespace following the final deleted word but none of
the whitespace preceding the first deleted word. Level B uses the VM Unicode
whitespace definition. A non-positive start or supplied negative count signals
`INVALID_ARGUMENTS`.

The implementation scans the source once with direct blank/nonblank VM
operations and composes at most one prefix and one tail; it makes no other
selector calls. The focused harness is
`lib/rxfnsb/tests_functional/ts_delword.crexx`.
