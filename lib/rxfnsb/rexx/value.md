# Level B `value`

`value(name)` is a Level B caller-frame introspection helper. It is distinct
from the assigning Classic `VALUE` built-in function.

```rexx
value(name = .string) = .string
```

The helper searches the immediate caller procedure's source metadata for a
visible scalar register or constant. Names are matched case-insensitively and
exactly. Integer and float registers are converted to the function's string
result; string values are returned unchanged.

- An empty name returns an empty string.
- A name that is absent, cleared, or outside the immediate caller procedure
  returns its uppercase spelling.
- The helper is read-only and does not create or assign a variable.
- Arrays, objects, stems, and compound Classic variables are outside this
  Level B helper's contract.

The metadata scan stops at the caller's `.meta_func` boundary. This is required
because `metalinkpreg` can safely read only a register number belonging to the
immediate parent frame.

## Examples

```rexx
count = 12
say value("count")    /* 12 */
say value("missing")  /* MISSING */
say value("")         /* empty string */
```

The focused test `lib/rxfnsb/tests_functional/ts_value.crexx` covers integer,
float, string, constant, blank, empty, missing, exact-name, visibility, and
procedure-boundary behavior.
