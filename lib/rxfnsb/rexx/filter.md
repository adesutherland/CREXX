# Level B `filter`

`filter` removes characters from a string:

```rexx
filter(input_value=.string, filter=.string) = .string
```

Every source character that occurs in `filter` is removed. Comparison is by
Unicode code point, repeated removal characters have no additional effect, and
the retained characters keep their original order.

```rexx
filter("a🙂bλ🙂c", "🙂λ") == "abc"
```

An empty removal set returns an equal copy of the source; an empty source returns
empty. Both arguments remain unchanged. Valid Level B `.string` values require
valid UTF-8; the underlying VM instruction raises `UNICODE_ERROR` if an invalid
string reaches it, and allocation failure is fatal.

The implementation initializes one result and executes the VM `dropchar`
instruction once. The VM compares the removal set directly without a Level B
helper call or intermediate normalized copy. There is no Level C BIF or class
method with this name. `lib/rxfnsb/tests_functional/ts_filter.crexx` covers the
documented behavior.
