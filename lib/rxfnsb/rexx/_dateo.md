## Level B `_dateo`

`_dateo(jdn=.int, format=.string [,osep=""]) -> .string` is the private
output half of the extended Level B DATE implementation. Its formats and
separator defaults are documented in `date.md`.

The helper validates the JDN once, derives all Gregorian fields once, and then
formats with direct digit/codepoint append operations. It does not route
numeric fields through general RIGHT, WORD, or SUBSTR calls. Every branch
returns `.string`, including the signed UNIX/EPOCH and numeric BASE/JDN forms.

An invalid JDN, output format, or separator longer than one codepoint signals
`INVALID_ARGUMENTS`; there is no fallback format.
