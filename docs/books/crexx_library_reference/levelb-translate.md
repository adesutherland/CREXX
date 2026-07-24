## `translate` (Level B)

```rexx
translate(source = .string [, output_table = .string
          [, input_table = .string [, pad = .string]]]) = .string
```

With both tables omitted, `translate` applies the Level B locale-independent
simple uppercase mapping. It does not normalize the text and does not perform
full case folding.

With an explicit `input_table`, each source codepoint is replaced by the
codepoint at the same position in `output_table`. The first duplicate input
entry wins. A match beyond the output table uses `pad`; an omitted output table
is an empty table. Source codepoints absent from the input table are unchanged.

When `output_table` is supplied but `input_table` is omitted, the input table
is the fixed Level B U+0000 through U+00FF byte-domain range. Codepoints outside
that range are unchanged. The implementation calculates those positions
directly and does not allocate a 256-codepoint table.

`pad` defaults to blank and must contain exactly one Unicode codepoint;
otherwise `INVALID_ARGUMENTS` is raised. Positions and table entries are
codepoint based, not UTF-8 byte based.

The Classic Level C profile-dependent contract is documented separately in
[`lib/rxfnsc/translate.md`](../../rxfnsc/translate.md).
