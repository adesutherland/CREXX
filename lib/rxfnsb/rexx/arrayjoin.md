# Level B `arrayjoin`

`arrayjoin` combines a one-based Level B string array without modifying it:

```rexx
arrayjoin(array=.string[] [,separator=.string]) = .string
```

The separator is inserted between adjacent elements only and defaults to the
empty string. Empty-string elements still participate in separator placement.
An empty array returns empty; a one-element array returns that value unchanged.

```rexx
items = .string[]
items[1] = "A"
items[2] = ""
items[3] = "C"
arrayjoin(items, "|") == "A||C"
```

The implementation reads the high-water mark once and appends each separator
and element directly into one growing destination. It is O(total result text),
creates no per-element concatenation result, and has no helper calls. Inputs are
read-only. There is no Level C BIF or class method named ARRAYJOIN.

`lib/rxfnsb/tests_functional/ts_arrayjoin.crexx` covers empty/single arrays,
empty/Unicode values and separators, omission, and non-mutation.
