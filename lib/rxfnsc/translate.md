# `TRANSLATE` (Level C Classic BIF)

```text
TRANSLATE(string [, output_table [, input_table [, pad]]])
CheckArgs: rANY oANY oANY oPAD
```

With both tables omitted, the standalone `rexxclassicbif_translate` entry uses
the active profile's locale-independent uppercase mapping. With an explicit
input table, the first matching entry selects the parallel output entry; a
missing output entry uses `pad`, which defaults to blank. Unmatched characters
are unchanged.

In BYTE, table entries and PAD are exact bytes. If output is supplied while
input is omitted, the implicit input table is the configured `00` through `FF`
XRANGE, calculated directly without allocating the table. ASCII lowercase is
the BYTE uppercase mapping.

In UTF8, tables, PAD, and positions are Unicode codepoints and no normalization
occurs. Output-with-omitted-input reports `RXC-LC-40.1` because UTF8 XRANGE is
not defined. Invalid UTF-8 reports `23.1`; an invalid profile-sized PAD reports
`40.23`; standard presence/count errors use `40.3`, `40.4`, and `40.5`.

The direct optimized/unoptimized RexxValue harness covers uppercase, duplicate
table entries, missing output padding, implicit BYTE XRANGE, Unicode mapping,
arbitrary bytes, profile rejection, and standard errors. The native Level B
codepoint API is documented separately in
[`lib/rxfnsb/rexx/translate.md`](../rxfnsb/rexx/translate.md).
