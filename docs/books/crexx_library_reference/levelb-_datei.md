## Level B `_datei`

`_datei(idate=.string, format=.string [,isep=""]) -> .int` is the private
input half of the extended Level B DATE implementation. It converts one format
documented in `date.md` to a validated Julian day number.

The implementation performs direct Unicode-codepoint scans for integer,
fixed-width, separated, and named-month forms. It does not call general WORD,
SUBSTR, POS, RIGHT, or abbreviation BIFs on the parsing path. Two-digit years
map to 2000 through 2099; X formats require four digits; QUALIFIED input checks
that the supplied weekday matches the date. Negative EPOCH values use floor
division so `-1` identifies 1969-12-31.

Malformed text, unknown/output-only formats, invalid dates, out-of-range
origins, and a separator longer than one codepoint signal `INVALID_ARGUMENTS`.
The public `date()` wrapper also validates separator length before dispatch.
