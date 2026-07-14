# Level B `arrayreverse`

`arrayreverse` reverses a one-based Level B string array in place:

```rexx
arrayreverse(expose array=.string[]) = .int
```

The return value is the unchanged high-water mark. Empty, single-element,
empty-string, and Unicode elements are ordinary array values.

```rexx
items = .string[]
items[1] = "A"
items[2] = "B"
items[3] = "C"
arrayreverse(items) == 3
items[1] == "C"
```

The implementation swaps the first/last, second/penultimate, and subsequent
pairs until the indices meet. It is O(n), uses constant temporary storage, and
performs exactly half as many swaps as elements; it does not allocate or return
a copied array and has no helper calls. There are no invalid-input branches,
Level C BIF, or class method named ARRAYREVERSE.

`lib/rxfnsb/tests_functional/ts_arrayreverse.crexx` covers empty, single, odd,
even, Unicode/empty values, return state, and reversing twice.
