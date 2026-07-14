# Level B `changestr`

`changestr` replaces every case-sensitive, non-overlapping occurrence of
`needle` in the original `haystack`:

```rexx
changestr(needle = .string, haystack = .string,
          replacement = .string) = .string
```

Searching proceeds from left to right. Inserted replacement text is never
searched again, even when it contains `needle`. A null `needle` or a needle
that is not found returns the haystack unchanged. A null replacement deletes
matches.

```rexx
changestr("the", "the cat and the dog", "a")  /* "a cat and a dog" */
changestr("aa", "aaaaa", "X")                 /* "XXa" */
changestr("a", "banana", "")                 /* "bnn" */
changestr("", "unchanged", "!")              /* "unchanged" */
```

All inputs are read-only and there are no value-domain signals. Their static
Level B types enforce the three mandatory string arguments.

The implementation searches the original haystack directly and appends each
unmatched slice and replacement to one result. It does not rebuild and
re-search the growing result after every match.

`lib/rxfnsb/tests_functional/ts_changestr.crexx` covers deletion, expansion,
non-overlap, empty inputs, case sensitivity, Unicode, argument non-mutation,
and a repeated replacement workload in optimized and unoptimized overlays.
