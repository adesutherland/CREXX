# Level B `copies`

`copies` concatenates a string a non-negative number of times:

```rexx
copies(string = .string, count = .int) = .string
```

Count zero returns the null string; count one returns the source value. A
negative count raises `INVALID_ARGUMENTS`.

```rexx
copies("abc", 3)  /* "abcabcabc" */
copies("abc", 0)  /* "" */
copies("", 2)     /* "" */
```

The source argument is read-only. For larger counts the implementation uses
binary decomposition: it doubles a repeated chunk and appends only chunks for
set bits in `count`. This reduces the number of VM append operations from
`count` to O(log `count`) while producing the same exact result.

`lib/rxfnsb/tests_functional/tscopies.crexx` covers zero, one, ordinary,
non-power-of-two, Unicode, empty, negative, and 4,096-copy cases in optimized
and unoptimized overlays.
