# Level B `arrayshift`

`arrayshift` removes the first element of a one-based Level B string array:

```rexx
arrayshift(expose array=.string[] [,default=.string]) = .string
```

When the array is nonempty, its first value is returned, later elements move
down one index, and its high-water mark is reduced by one. Empty-string elements
are values and are still removed. Empty input is unchanged and returns
`default`, whose default is the empty string.

```rexx
items = .string[]
items[1] = "A"
items[2] = "B"
arrayshift(items) == "A"
items[1] == "B"
```

The exposed array is intentional. The implementation reads the first value and
uses one native `DELATTRS1` deletion; that instruction performs the required
O(n) attribute shift without a Level B loop or helper call. Empty input returns
before mutation. There is no Level C BIF or class method named ARRAYSHIFT.

`lib/rxfnsb/tests_functional/ts_arrayshift.crexx` covers shifted indices,
empty-string/Unicode values, repeated mutation, defaults, and high-water state.
