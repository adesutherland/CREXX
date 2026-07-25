## Level B `fnv`

The typed Level B function returns the conventional 32-bit FNV-1a hash of a
text value:

```rexx
fnv(input_value = .string) = .int
```

The input is hashed as its exact UTF-8 byte sequence, in forward byte order.
The result is the unsigned 32-bit value represented as an integer in
`0..4294967295`. For example, `fnv("")` is `2166136261`, `fnv("a")` is
`3826002220`, and `fnv("abc")` is `440920331`.

Embedded NUL bytes participate in the hash. The function does not mutate the
input and does not signal numeric overflow because the algorithm deliberately
uses modulo-2^32 unsigned arithmetic. Invalid text is rejected by the normal
`.string`/UTF-8 runtime rules before it can be hashed.

`fnv` is a Level B library function, not a Level C Classic BIF. The backing
`rxhash` instruction accepts an explicit byte count; `fnv` supplies the full
encoded byte length.
