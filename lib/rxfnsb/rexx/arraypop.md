# Level B `arraypop`

`arraypop` removes the final element of a one-based Level B string array:

```rexx
arraypop(expose array=.string[] [,default=.string]) = .string
```

When the array is nonempty, its last value is returned and its high-water mark
is reduced by one. Empty-string elements are values and are still removed. When
the array is empty, it is unchanged and `default` is returned; the default value
is the empty string.

```rexx
items = .string[]
items[1] = "A"
items[2] = "B"
arraypop(items) == "B"
items[0] == 1
```

The array is intentionally exposed because mutation is the operation's
contract. The implementation reads the high-water mark once and uses one native
`DELATTRS1` deletion at the known-valid last index; empty input returns before
that instruction. It performs no scan, shift, helper call, or allocation under
Level B control. There is no Level C BIF or class method named ARRAYPOP.

`lib/rxfnsb/tests_functional/ts_arraypop.crexx` covers values, empty-string
elements, repeated mutation, explicit/omitted defaults, and high-water state.
