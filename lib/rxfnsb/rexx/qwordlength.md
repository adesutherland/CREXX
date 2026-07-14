# Level B `qwordlength`

`qwordlength` returns the character length of the span selected by the Level B
quote-aware `qword` helper:

```rexx
qwordlength(text=.string, word_number=.int) = .int
```

`word_number` is positive and one-based. A number beyond the available words
returns `0`; a number below one signals `INVALID_ARGUMENTS`. The input is not
modified. The exact treatment of quote delimiters, doubled/unmatched quotes,
attached quoted text, and configured blanks follows `qword` and remains tracked
as the shared quote-aware word-span dependency in the Release 1 work list.

The implementation calls `qword` once and measures its returned span once. It
does not expose or copy the source argument. There is no Level C BIF or class
method named QWORDLENGTH.

`ts_qwordlength.crexx` covers typed unquoted lengths, missing/empty input,
non-mutation, and the invalid-index signal without freezing the parked quote
grammar.
