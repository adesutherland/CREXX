# Level B `length`

`length` returns the number of Unicode codepoints in a string:

```rexx
length(string = .string) = .int
```

The result is zero for an empty string. It counts characters/codepoints, not
UTF-8 storage bytes; a combining codepoint is counted separately from the base
character it follows.

```rexx
length("")        /* 0 */
length("abcdefgh") /* 8 */
length("é日🙂")    /* 3 */
length("é")       /* 2: e plus a combining acute accent */
```

The `.string` argument is read-only and valid in the configured string model,
so the Level B surface has no data-error branch. The implementation is one
direct VM `strlen` instruction with no helper call or intermediate string.
