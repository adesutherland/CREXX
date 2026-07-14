# Level B `parse`

`parse` is the convenience wrapper around [`parsecompile`](parsecompile.md) and
[`parsestring`](parsestring.md):

```rexx
parse(source = .string,
      template = .string,
      expose variable = .string[],
      expose variable_content = .string[],
      strip_option = .int optional,
      case_option = .int optional) = .int
```

It returns the number of variables and fills aligned name and value arrays.
`strip_option` is `0` to preserve values or `1` to remove leading and trailing
whitespace. `case_option` is `0` to preserve case, `1` for uppercase, or `2` for
lowercase. Transformations are applied after parsing, with stripping first.

```rexx
names = .string[]
values = .string[]
count = parse("  ALPHA , BETA  ", "first','second", names, values, 1, 2)
/* count = 2; values[1] = "alpha"; values[2] = "beta" */
```

Invalid option values or an invalid dynamic template signal
`INVALID_ARGUMENTS`. Template validation occurs before the exposed output
arrays are changed. See `parsecompile` for the complete legacy template format.

The wrapper uses direct VM strip and case operations. It is a Level B runtime
helper, not a Level C BIF.
