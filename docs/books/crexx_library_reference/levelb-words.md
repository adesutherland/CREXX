## words

```rexx
words(string = .string) = .int
```

`words` counts the words in `string` using the VM Unicode whitespace
definition. Empty and blank-only strings contain zero words.

The implementation is a single allocation-free scan and makes no other
selector calls. The focused harness is
`lib/rxfnsb/tests_functional/ts_words.crexx`.
