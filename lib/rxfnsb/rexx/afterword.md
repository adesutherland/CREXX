## `afterword` (Level B)

```rexx
afterword(string = .string, wordnum = .int) = .string
```

`afterword` returns the original source text following the positive one-based
`wordnum`.

The selected word itself is not included. All characters after that word are
returned unchanged, including leading and trailing whitespace.

A word number beyond the available words returns an empty string.

Unlike `subword()`, `afterword()` preserves the original text exactly as it
appears in the source. It does not normalize the returned value to word
boundaries.

Words use the VM Unicode whitespace definition. A non-positive word number
signals `INVALID_ARGUMENTS`.

Examples:

```rexx
afterword("one two three", 1)      /* " two three" */

afterword("one two   three", 1)    /* "  two   three" */

afterword("one two   three", 2)    /* "   three" */

afterword("one two   three", 3)    /* "" */

afterword("one two   ", 2)         /* "   " */

afterword(".section getters     return _name", 2)
/* "     return _name" */
```

The implementation uses `wordindex()` to locate the selected word, identifies
the following whitespace boundary, and extracts the remaining source text once.

The focused harness is `lib/rxfnsb/tests_functional/ts_afterword.crexx`.