# Data structures

## Stem variables and Arrays

Level B arrays are typed values such as `.string[]` and `.int[]`. They are
indexed with square brackets:

```rexx
names = .string[]
names[1] = "Ada"
names[2] = "Grace"
say names[0]
```

`array[0]` is the high-water mark used by the current array helpers. User data
is stored from `array[1]` through `array[array[0]]`. The `array*` built-in
helpers in `rxfnsb` are the supported array manipulation surface for new code.

Classic one-dot notation remains available as array-compatible syntax:

```rexx
names.1 = "Ada"
say names.1
```

For new Level B code, prefer `names[1]` because it is explicit and does not
look like a hierarchical stem.

CREXX hierarchical stems are currently a precompiler feature, not a separate VM
storage type in the core language. Multi-dot names such as
`Customer.IBM.name` are handled by RXPP/stem processing and mapped onto a
key/value backing store. The optional `dotisstem` precompiler mode can treat
single-dot names as stems, but that changes the meaning of classic `a.i`
array-style code, so it should be chosen deliberately.

Related references:

* `bifs_function_reference.md` documents the current array helper functions.
* `lib/plugins/precomp/rxpp-Users-Guide.md` documents RXPP preprocessing.
* `lib/plugins/map/Compounds.md` describes the current stem-processing model.
