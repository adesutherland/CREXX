# `XRANGE` (Level C Classic BIF)

```text
XRANGE([start [, end]])
CheckArgs: oPAD oPAD
```

Classic Level C XRANGE asks the active character configuration for the encoded
characters from `start` through `end`. Both arguments are optional one-character
values; defaults and ordering belong to the `Config_Xrange` service. This is
not necessarily a Unicode-codepoint range.

The repository does not yet define that configuration service or its default
coded-character policy. The standalone direct BIF and harness are therefore
parked rather than substituting the native UTF-8 Level B helper. The same
dependency blocks Classic TRANSLATE when its input table is omitted.

The distinct native U+0000..U+00FF wrapping helper is documented in
[`lib/rxfnsb/rexx/xrange.md`](../rxfnsb/rexx/xrange.md).
