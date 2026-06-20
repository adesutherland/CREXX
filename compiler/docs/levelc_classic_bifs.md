# Level C Classic BIF Implementation Notes

Status: extracted implementation reference for Level C BIF migration
Last updated: 2026-06-20

Source: publicly available Classic REXX language specification.

Related project notes:

- `compiler/docs/levelc_working_architecture.md`
- `compiler/docs/levelc_rexx_runtime_values.md`
- `compiler/docs/levelc_standard_error_messages.md`
- `docs/ai-context/CREXX_LIBS.md`
- `docs/ai-context/REXXSCRIPT_ARCHITECTURE.md`
- `docs/ai-context/REXXSCRIPT_CONCEPT.md`

This note fills the BIF gap deliberately left out of the Level C working
architecture note. It is a normalized implementation guide, not a verbatim copy
of the specification pseudocode.

## Fixed Direction

Level C will be implemented in Level B.

Conformant Level C built-in functions should therefore be written as Level B
runtime/library code over the shared `lib/rxfnsc` value layer:

- `RexxValue` for Classic scalar values and lazy string/int/float/decimal/binary
  materialization.
- `RexxStem` for stem tails and compound-variable storage.
- `RexxVariablePool` for Classic variable pools, exposure, dropped state, and
  `VALUE`/`SYMBOL` semantics.

RexxScript should share this runtime layer where its sandboxed intrinsic set
overlaps pure Classic operations. RexxScript remains intentionally smaller than
Level C: no `ADDRESS`, external calls, stream I/O, ambient host access, or full
variable-pool visibility unless an explicit host adapter later exposes that
capability.

Current Level B `rxfnsb` modules are useful source material, but they are not
automatically Level C conformant. Several are UTF text helpers, metadata
helpers, or early implementations. The Level C BIF work should audit each
function against this note and the public language-specification source.

## BIF Invocation Model

The Classic specification section defines each built-in as an external routine
evaluated from a common invocation record:

| Field | Meaning for Level B implementation |
| --- | --- |
| `#Bif` | Uppercase built-in name being executed. Shared helpers use this in messages. |
| `#Bif_Arg.0` | Argument count, including omitted optional argument positions. |
| `#Bif_Arg.i` | Argument value at position `i`. |
| `#Bif_ArgExists.i` | Whether argument `i` was supplied. Missing required arguments raise syntax. |
| `#Level` | Current execution level/frame, used for numeric, trace, source, condition, and argument state. |

A Level B implementation does not need to expose these names literally. It does
need an equivalent BIF context object or procedure contract so shared helpers
can see the function name, argument values, argument presence, caller numeric
settings, current variable pool, source lines, condition state, trace state,
stream state, and host/configuration adapters.

### Numeric Context

The specification helper code uses three numeric contexts:

- `NUM` and `WHOLENUM` checks use the caller's `NUMERIC DIGITS` and
  `NUMERIC FORM`.
- `WHOLE`, `WHOLE>=0`, and `WHOLE>0` checks use the BIF's own internal digits
  setting and scientific form.
- Internal date, radix, and formatting work may use enough precision to avoid
  introducing exponential notation in intermediate values.

For the `RexxValue` runtime, that means the BIF layer must set
`rexxvalue_numeric_digits()` and `rexxvalue_numeric_fuzz()` or equivalent
per-context settings before calling numeric materializers/operators. Do not add
ad hoc numeric parsing in every BIF.

## Shared Helper Contracts

### `CheckArgs`

The specification's `CheckArgs` helper takes a checklist string. Each argument item
starts with `r` for required or `o` for optional, followed by a type rule.

| Rule | Required behavior | Generic message codes |
| --- | --- | --- |
| Argument count | Too few, too many, or missing required arguments fail before function logic. | `40.3`, `40.4`, `40.5` |
| `ANY` | Any string/RexxValue string view is accepted. | none |
| `NUM` | Argument must be numeric under caller settings; normalized numeric value replaces the argument copy. | `40.9`, `40.11` |
| `WHOLE` | Whole number under BIF settings; normalized whole number replaces the argument copy. | `40.12` |
| `WHOLE>=0` | Whole number greater than or equal to zero. | `40.12`, `40.13` |
| `WHOLE>0` | Whole number greater than zero. | `40.12`, `40.14` |
| `WHOLENUM` | D2X-style whole number under caller settings. | `40.12` |
| `WHOLENUM>=0` | D2X-style non-negative whole number under caller settings. | `40.12`, `40.13` |
| `0_90` | `ERRORTEXT` message code in range `0` through `90.9`, without exponential notation. | `40.11`, source helper calls `40.16`; reconcile with catalog before implementation |
| `PAD` | Exactly one character. | `40.23` |
| `HEX` | Hex string according to `DATATYPE(value, "X")`. | `40.25` |
| `BIN` | Binary string according to `DATATYPE(value, "B")`. | `40.24` |
| `SYM` | Valid symbol according to `DATATYPE(value, "S")`. | `40.26` |
| `STREAM` | Valid stream name according to `Config_Stream_Qualify`. | `40.27` |
| Option set, for example `LTB` | Non-null; first character, uppercased, must be in the option set. | `40.21`, `40.28` |
| `ACEFILNOR` | TRACE option set; leading `?` characters are allowed before the option letter. | `40.28` |

The message catalog is in `compiler/docs/levelc_standard_error_messages.md`.
The extracted helper visibly calls `40.16` for the `0_90` range failure,
while the extracted catalog has a nearby `40.17` range-specific text. Treat this
as a source reconciliation item before wiring `ERRORTEXT` tests.

## Implemented Proof Slice

The first executable proof slice lives in `lib/rxfnsc/RexxClassicBifs.crexx`.
It is intentionally string-first and shares the `rxfnsc` runtime image with
`RexxValue`, `RexxStem`, and `RexxVariablePool`.

Implemented BIFs:

```text
ABBREV ABS COPIES DATATYPE LEFT LENGTH LOWER MAX MIN POS RIGHT SIGN SPACE
STRIP SUBSTR UPPER VERIFY WORD WORDS
```

`UPPER` and `LOWER` are included for cREXX/RexxScript practicality even though
the strict Classic catalog normally expresses uppercasing through `TRANSLATE`.

The public proof API is:

```text
RexxBifCallContext
rexxclassicbif_call(reference context)
rexxclassicbif_check_args(reference context, checklist)
```

`RexxBifCallContext` carries the uppercased BIF name, argument values, argument
presence flags, and a live caller `RexxVariablePool` reference. Level C lowering
should pass the current visible activation pool. RexxScript passes only its
sandbox/script pool.

The current `CheckArgs` subset supports:

```text
ANY NUM WHOLE WHOLE>=0 WHOLE>0 PAD ABLMNSUWX LTB MN
```

The helper currently returns a structured status array:

```text
OK, value, "", ""
ERROR, "", RXC-LC-code, message
```

This is a temporary proof contract. Level C must remain Classic Rexx compliant:
Classic argument validation, message codes, and condition behavior are not to be
weakened to match Level B convenience behavior. Separately, the planned Level B
library review will move Level B library failures toward signal-based reporting,
and that change is not expected to be backward compatible. Keep the bridge
centralized in `RexxClassicBifs.crexx` through `rexxclassicbif_check_args`,
`_set_bif_error`, and `_context_error`; do not spread one-off error formatting
or signal decisions through individual BIF bodies.

JavaDoc-style tags are present on the implemented BIF helpers for generated user
documentation. The current tags are `@bif`, `@signature`, `@checkargs`,
`@sandbox`, and `@return`.

One important Classic comparison trap: do not use normal string equality to test
for exact empty strings in BIF logic. Under Rexx comparison rules a blank string
can compare equal to `""`. Use `length(value) = 0` when the distinction matters,
as in `POS("", haystack)` versus `POS(" ", haystack)`.

### Date Helpers

`DATE` and `TIME` share date/time helpers:

- `Time2Date(timestamp)` validates timestamp bounds and returns year, month,
  day, hour, minute, second, microsecond, base-day count, and day-of-year.
- `Leap(year)` returns whether the Gregorian year is a leap year.
- The current time is frozen per clause through `#ClauseTime.#Level` and
  `#ClauseLocal.#Level`.

Level B implementation should make this a shared helper module, not duplicate
the conversion in both `DATE` and `TIME`.

### Radix Helper

`ReRadix(subject, fromRadix, toRadix)` converts between radices 2, 10, and 16
through decimal, preserving binary/hex widths for `B2X` and `X2B`.

This should become a shared Level B helper over `RexxValue` numeric/binary
views. Do not route arbitrary encoded bytes through `.string` unless the Level C
byte-text policy explicitly permits it.

### Raise Helper

The extracted `Raise` helper raises `SYNTAX` and always includes the BIF name as
the first insert for `40.*` errors. It does not return.

The current `lib/rxfnsb/rexx/raise.crexx` is only a placeholder printer. Level C
needs a real condition/message bridge using the specification message catalog and
the runtime condition model.

### Configuration Dependencies

Several BIFs are not pure string/numeric functions. They depend on host or
configuration services:

| Service area | Needed by |
| --- | --- |
| Character length/substr/encoding validation | `LENGTH`, `SUBSTR`, `SYMBOL`, invalid character/data string diagnostics |
| Uppercase and collation/range services | `TRANSLATE`, `XRANGE`, comparisons where configuration collation matters |
| Coded-character to bits and bits to coded-character conversion | `BITAND`, `BITOR`, `BITXOR`, `C2D`, `C2X`, `D2C`, `X2C` |
| Streams, stream state, stream position, stream count, stream commands | `CHARIN`, `CHAROUT`, `CHARS`, `LINEIN`, `LINEOUT`, `LINES`, `QUALIFY`, `STREAM` |
| External data queue | `QUEUED` |
| Time and local-time adjustment | `DATE`, `TIME` |
| Random seed and next value | `RANDOM` |
| External variable pools | `VALUE(name, newvalue, pool)` |
| Internal variable pools | `SYMBOL`, `VALUE`, compound variable expansion |
| Current execution state | `ADDRESS`, `ARG`, `CONDITION`, `DIGITS`, `FORM`, `FUZZ`, `SOURCELINE`, `TRACE` |

## Message Codes Used By BIFs

Generic `CheckArgs` errors are listed above. Function bodies add these notable
conditions:

| Code or condition | Used by | Meaning |
| --- | --- | --- |
| `23.1` | `LENGTH`, `SUBSTR`, `SYMBOL` | Invalid data/character string after configuration length or encoding checks. |
| `40.19` | `DATE`, `TIME` | Input value does not match the declared input format, or input argument is missing when input format is supplied. |
| `40.29` | `TIME` | Conversion to elapsed/reset/offset formats is not allowed. |
| `40.31` | `RANDOM` | Single-argument maximum exceeds `100000`. |
| `40.32` | `RANDOM` | Requested random range is wider than `100000`. |
| `40.33` | `RANDOM` | Minimum is greater than maximum. |
| `40.34` | `SOURCELINE` | Requested source line exceeds available source lines. |
| `40.35` | `C2D`, `X2D` | Decimal conversion cannot be expressed as a whole number under current digits. |
| `40.36` | `VALUE` | External pool reports the requested variable name is not found/valid. |
| `40.37` | `VALUE` | External pool name is not valid. |
| `40.38` | `FORMAT` | Formatted number does not fit requested integer or exponent width. |
| `40.39` | `LINEIN` | `LINEIN` count argument is not zero or one. |
| `40.41` | `CHARIN`, `CHAROUT`, `LINEIN`, `LINEOUT` | Position argument is outside stream bounds. |
| `40.42` | `CHARIN`, `CHAROUT`, `LINEIN`, `LINEOUT` | Stream cannot be positioned. |
| `NOTREADY` | stream input/output BIFs | Raised from stream configuration failures; stream state may be marked `ERROR`. |

## Function Catalog

Argument checklist values are normalized from the public language-specification
source for planning. This is implementation guidance, not a code copy.

### Character Built-in Functions

| Function | Signature | Checklist | Definition summary | Level B/RexxValue notes |
| --- | --- | --- | --- | --- |
| `ABBREV` | `ABBREV(string, abbrev [,length])` | `rANY rANY oWHOLE>=0` | Returns `1` when `abbrev` matches the leading characters of `string` and is at least `length` characters long. | Pure string helper over `RexxValue.asString()`. |
| `CENTER` | `CENTER(string, length [,pad])` | `rANY rWHOLE>=0 oPAD` | Centers or trims `string` to `length`, using `pad` or blank. | Alias target for `CENTRE`; preserve one-character pad rule. |
| `CENTRE` | `CENTRE(string, length [,pad])` | same as `CENTER` | Alternative spelling of `CENTER`. | Implement as direct alias, not duplicate logic. |
| `CHANGESTR` | `CHANGESTR(needle, haystack, replacement)` | `rANY rANY rANY` | Replaces all non-overlapping occurrences of `needle` in `haystack`. | Define empty-needle behavior from compatibility tests before optimizing. |
| `COMPARE` | `COMPARE(left, right [,pad])` | `rANY rANY oPAD` | Returns `0` if equal, otherwise the first differing 1-based character position after padding the shorter side. | Character positions, not bytes, unless Level C byte-text mode says otherwise. |
| `COPIES` | `COPIES(string, count)` | `rANY rWHOLE>=0` | Concatenates `count` copies of `string`. | Guard resource exhaustion through normal string limits. |
| `COUNTSTR` | `COUNTSTR(needle, haystack)` | `rANY rANY` | Counts non-overlapping appearances of `needle` in `haystack`. | Same search rules as `POS`; empty-needle behavior needs tests. |
| `DATATYPE` | `DATATYPE(string [,type])` | `rANY oABLMNSUWX` | With no type, returns numeric/character classification. With type, tests alphanumeric, binary, lowercase, mixed letters, number, symbol, uppercase, whole, or hex. | This is a core helper for `CheckArgs`; it must match Classic syntax, not current Level B keyword rules. |
| `DELSTR` | `DELSTR(string, start [,length])` | `rANY rWHOLE>0 oWHOLE>=0` | Deletes a character substring from `start`; omitted length deletes through the end. | 1-based character indexes. |
| `DELWORD` | `DELWORD(string, start [,count])` | `rANY rWHOLE>0 oWHOLE>=0` | Deletes words beginning at word `start`; omitted count deletes through the end. | Word boundaries are blank/equivalent blank based on Classic rules. |
| `INSERT` | `INSERT(new, target [,before [,length [,pad]]])` | `rANY rANY oWHOLE>=0 oWHOLE>=0 oPAD` | Inserts `new` after `before` characters of `target`, padding/truncating inserted text to `length` when supplied. | The specification text says "before the insert"; behavior is the classic 0-based insertion point in a 1-based API. |
| `LASTPOS` | `LASTPOS(needle, haystack [,start])` | `rANY rANY oWHOLE>0` | Finds the last occurrence of `needle`, optionally searching leftward from `start`. | Returns `0` on no match. |
| `LEFT` | `LEFT(string, length [,pad])` | `rANY rWHOLE>=0 oPAD` | Returns leftmost `length` characters, padding on the right if needed. | Character indexing. |
| `LENGTH` | `LENGTH(string)` | `rANY` | Returns the configuration character length. Raises `23.1` if the string is invalid for the configuration. | In normal Level B UTF builds, `.string` is already valid UTF-8; Level C byte-text mode still needs a policy. |
| `OVERLAY` | `OVERLAY(new, target [,start [,length [,pad]]])` | `rANY rANY oWHOLE>0 oWHOLE>=0 oPAD` | Overlays `new` onto `target` at `start`, padding/truncating overlay text to `length` when supplied. | Similar shared helper with `INSERT`. |
| `POS` | `POS(needle, haystack [,start])` | `rANY rANY oWHOLE>0` | Finds the first occurrence of `needle` at or after `start`; returns `0` if not found. | `needle == ""` returns `0` in the specification code. |
| `REVERSE` | `REVERSE(string)` | `rANY` | Reverses the sequence of characters. | Must not reverse UTF-8 bytes in normal UTF builds. |
| `RIGHT` | `RIGHT(string, length [,pad])` | `rANY rWHOLE>=0 oPAD` | Returns rightmost `length` characters, padding on the left if needed. | Character indexing. |
| `SPACE` | `SPACE(string [,count [,pad]])` | `rANY oWHOLE>=0 oPAD` | Removes leading/trailing/intermediate blank runs and rejoins words with `count` pad characters. | Default count is one, default pad is blank. |
| `STRIP` | `STRIP(string [,option [,char]])` | `rANY oLTB oPAD` | Strips leading, trailing, or both occurrences of `char`; default is both blanks. | Option first letter: `L`, `T`, `B`. |
| `SUBSTR` | `SUBSTR(string, start [,length [,pad]])` | `rANY rWHOLE>0 oWHOLE>=0 oPAD` | Returns substring from `start`, padding if requested length extends beyond input. | Standard checks that requested start can reference the string or raise invalid data as appropriate. |
| `SUBWORD` | `SUBWORD(string, start [,count])` | `rANY rWHOLE>0 oWHOLE>=0` | Returns a substring made of words from word `start`, for `count` words or through the end. | Share word scanner with `WORD*` functions. |
| `TRANSLATE` | `TRANSLATE(string [,outputTable [,inputTable [,pad]]])` | `rANY oANY oANY oPAD` | Uppercases by configuration when no tables are supplied; otherwise maps characters from input table to output table, using pad for missing output entries. | Requires configuration uppercase/range services. |
| `VERIFY` | `VERIFY(string, reference [,option [,start]])` | `rANY rANY oMN oWHOLE>0` | With `M`, returns first character position in `string` that is in `reference`; with `N`, first position not in `reference`; `0` if no such character. | Default option is `N`, default start is `1`. |
| `WORD` | `WORD(string, n)` | `rANY rWHOLE>0` | Returns word `n`, or null if absent. | Can delegate to `SUBWORD(string,n,1)`. |
| `WORDINDEX` | `WORDINDEX(string, n)` | `rANY rWHOLE>0` | Returns the character index of word `n`, or `0` if absent. | Needs shared word scanner that preserves original spacing. |
| `WORDLENGTH` | `WORDLENGTH(string, n)` | `rANY rWHOLE>0` | Returns the length of word `n`, or `0` if absent. | Can share `WORD` extraction. |
| `WORDPOS` | `WORDPOS(phrase, string [,start])` | `rANY rANY oWHOLE>0` | Finds a sequence of words from `phrase` in `string`; returns the word position or `0`. | Compare normalized word sequences, not raw spacing. |
| `WORDS` | `WORDS(string)` | `rANY` | Counts blank-delimited words. | Share word scanner. |
| `XRANGE` | `XRANGE([start [,end]])` | `oPAD oPAD` | Returns configuration-defined characters from start through end. | Requires `Config_Xrange`; not a Unicode range API. |

### Arithmetic Built-in Functions

| Function | Signature | Checklist | Definition summary | Level B/RexxValue notes |
| --- | --- | --- | --- | --- |
| `ABS` | `ABS(number)` | `rNUM` | Returns the absolute value after caller-context numeric normalization. | Use `RexxValue.asDecimal()` under caller numeric settings. |
| `FORMAT` | `FORMAT(number [,before [,after [,expp [,expt]]]])` | `rNUM oWHOLE>=0 oWHOLE>=0 oWHOLE>=0 oWHOLE>=0` | Formats a number with requested integer, fractional, and exponent widths, using caller form/scientific/engineering rules. | Raises `40.38` when the number cannot fit. This should become a shared numeric formatter, not copied into callers. |
| `MAX` | `MAX(number, ...)` | generated `rNUM...` | Returns the largest numeric argument. At least one argument is required. | `MAX()` with zero args raises `40.3`. |
| `MIN` | `MIN(number, ...)` | generated `rNUM...` | Returns the smallest numeric argument. At least one argument is required. | `MIN()` with zero args raises `40.3`. |
| `SIGN` | `SIGN(number)` | `rNUM` | Returns `-1`, `0`, or `1` according to numeric sign. | Use normalized numeric comparison. |
| `TRUNC` | `TRUNC(number [,digits])` | `rNUM oWHOLE>=0` | Truncates to integer or to `digits` fractional digits without rounding. | Preserve Classic string form, including leading `0.` construction. |

### State Built-in Functions

| Function | Signature | Checklist | Definition summary | Level B/RexxValue notes |
| --- | --- | --- | --- | --- |
| `ADDRESS` | `ADDRESS([option])` | `oEINO` | Returns current command environment name, or input/output target position/type/resource for option `E`, `I`, or `O`. | Use current `ADDRESS` runtime state, not command dispatch. |
| `ARG` | `ARG([n [,option]])` | `oWHOLE>0 oENO`, or required both when option supplied | No args returns argument count. One arg returns argument `n`. With option, returns existence/omission state. | Needs routine/program argument vector and existence flags. |
| `CONDITION` | `CONDITION([option])` | `oCDEIS` | Returns current condition name, description, extra data, instruction, or enabled state. Null when no current condition. | Depends on real condition runtime. |
| `DIGITS` | `DIGITS()` | none | Returns current `NUMERIC DIGITS`. | Existing `numeric.crexx` is relevant but Level C must use Classic current frame settings. |
| `ERRORTEXT` | `ERRORTEXT(code [,option])` | `r0_90 oSN` | Returns unexpanded message text; `S` requests specification English, `N` allows localized text. | Backed by `levelc_standard_error_messages.md`; reconcile `0_90` subcode issue. |
| `FORM` | `FORM()` | none | Returns current `NUMERIC FORM`. | Must return Classic form wording. |
| `FUZZ` | `FUZZ()` | none | Returns current `NUMERIC FUZZ`. | Must follow current frame. |
| `SOURCELINE` | `SOURCELINE([n])` | `oWHOLE>0` | No arg returns visible source line count or `0`; arg returns source line `n`. | Raises `40.34` beyond available source. Needs retained source lines. |
| `TRACE` | `TRACE([option])` | `oACEFILNOR` | Returns prior trace setting, optionally toggling interactive mode with leading `?` and setting new trace mode. | Current trace runtime is a good source, but BIF surface must return previous setting. |

### Conversion Built-in Functions

| Function | Signature | Checklist | Definition summary | Level B/RexxValue notes |
| --- | --- | --- | --- | --- |
| `B2X` | `B2X(binaryDigits)` | `rBIN` | Removes blanks and converts binary digit text to hexadecimal digit text. | Operates on textual `0`/`1` digits, not `.binary` buffers. |
| `BITAND` | `BITAND(left [,right [,pad]])` | `rANY oANY oPAD` | Converts encoded characters to bits, applies bitwise AND over the common length, and preserves the longer tail. | Depends on `Config_C2B` and coded-character policy. |
| `BITOR` | `BITOR(left [,right [,pad]])` | same as `BITAND` | Same as `BITAND`, using bitwise OR. | Implement through one shared bit helper keyed by `#Bif`. |
| `BITXOR` | `BITXOR(left [,right [,pad]])` | same as `BITAND` | Same as `BITAND`, using bitwise exclusive OR. | Implement through one shared bit helper keyed by `#Bif`. |
| `C2D` | `C2D(string [,length])` | `rANY oWHOLE>=0` | Converts coded characters to decimal. With length, treats the rightmost `length` characters as a signed twos-complement value. | Raises `40.35` when result cannot fit current digits. |
| `C2X` | `C2X(string)` | `rANY` | Converts coded characters to hexadecimal through configuration bit encoding. | Empty input returns null. |
| `D2C` | `D2C(number [,length])` | `rWHOLENUM>=0`, or `rWHOLENUM rWHOLE>=0` | Converts decimal whole number to coded characters; with length, pads/truncates using zero or high-value characters depending on sign. | Depends on `Config_B2C`; negative values use twos-complement. |
| `D2X` | `D2X(number [,length])` | `rWHOLENUM>=0`, or `rWHOLENUM rWHOLE>=0` | Converts decimal whole number to hex; with length, pads/truncates with `0` or `F` depending on sign. | Negative values use twos-complement. |
| `X2B` | `X2B(hex)` | `rHEX` | Removes blanks and converts hex digit text to binary digit text. | Empty input returns null. |
| `X2C` | `X2C(hex)` | `rHEX` | Converts hex digit text to coded characters, left-padding to a full byte as needed. | Depends on `Config_B2C`; empty input returns null. |
| `X2D` | `X2D(hex [,length])` | `rHEX oWHOLE>=0` | Converts hex digit text to decimal. With length, interprets sign bit for twos-complement. | Raises `40.35` when result cannot fit current digits. |

### Input/Output Built-in Functions

The Classic I/O BIFs are configuration stream APIs. Existing Level B `fileio.crexx`
functions are UTF text conveniences and should not be treated as conformant
Level C stream implementations without an audit.

| Function | Signature | Checklist | Definition summary | Level B/RexxValue notes |
| --- | --- | --- | --- | --- |
| `CHARIN` | `CHARIN([stream [,position [,count]]])` | `oSTREAM oWHOLE>0 oWHOLE>=0` | Reads `count` characters from a stream, optionally positioning first. Count defaults to `1`; count `0` touches the stream and returns null. | Raises `40.41`, `40.42`, or `NOTREADY`. Binary-mode streams use encoding conversion. |
| `CHAROUT` | `CHAROUT([stream [,string [,position]]])` | `oSTREAM oANY oWHOLE>0` | Writes string characters, optionally positioning first. With no string and no position, closes/positions to end. Returns remaining character count. | Raises `40.41`, `40.42`, or `NOTREADY`. |
| `CHARS` | `CHARS([stream [,option]])` | `oSTREAM oCN` | Indicates whether characters remain, or returns an immediately available count. | Delegates to stream count service. |
| `LINEIN` | `LINEIN([stream [,line [,count]]])` | `oSTREAM oWHOLE>0 oWHOLE>=0` | Reads one line unless count is `0`; positioning is optional. Count greater than `1` is invalid. | Raises `40.39`, `40.41`, `40.42`, or `NOTREADY`. |
| `LINEOUT` | `LINEOUT([stream [,string [,line]]])` | `oSTREAM oANY oWHOLE>0` | Writes string followed by an end-of-line marker; returns `0` for success and `1` for unsuccessful write. | Raises `40.41`, `40.42`, or `NOTREADY`. |
| `LINES` | `LINES([stream [,option]])` | `oSTREAM oCN` | Returns line availability/count according to stream count option. | The specification rationale constrains when `LINES(stream,"N")` may return zero. |
| `QUALIFY` | `QUALIFY([stream])` | `oSTREAM` | Returns a qualified stream name more persistently associated with the resource. | Requires configuration stream qualification. |
| `STREAM` | `STREAM(stream [,operation [,command]])` | `rSTREAM oCDS`, or `rSTREAM rCDS rANY` for command | Operation `C` sends a stream command, `D` returns detailed state, and `S` returns `READY`, `NOTREADY`, `UNKNOWN`, or `ERROR`. | `ERROR` can come from cached stream state after failed I/O. |

### Other Built-in Functions

| Function | Signature | Checklist | Definition summary | Level B/RexxValue notes |
| --- | --- | --- | --- | --- |
| `DATE` | `DATE([option [,date [,inoption]]])` | `oBDEMNOSUW oANY oBDENOSU` | With no date, returns current local date in requested format. With date, converts from `inoption` to output option. | Uses frozen clause time, `Time2Date`, and `Leap`. Raises `40.19` for invalid conversion. |
| `QUEUED` | `QUEUED()` | none | Returns number of lines in the external data queue. | Needs configuration queue service. |
| `RANDOM` | `RANDOM([max])` or `RANDOM([min [,max [,seed]]])` | `oWHOLE>=0 oWHOLE>=0 oWHOLE>=0` | Returns pseudo-random whole number in range. One argument means `0..arg`; defaults are `0..999`. | Range must be no wider than `100000`; raises `40.31`, `40.32`, `40.33`. |
| `SYMBOL` | `SYMBOL(name)` | prose-defined | Returns `BAD` if argument is not a valid symbol, `LIT` if symbol is valid but dropped/literal, or `VAR` if it has a value. | Must use Level C symbol recognition and `RexxVariablePool`, not Level B keyword metadata. |
| `TIME` | `TIME([option [,time [,inoption]]])` | `oCEHLMNORS oANY oCHLMNS` | With no time, returns current local time, elapsed time, reset elapsed time, or offset. With time, converts from `inoption` to output option. | Conversion to `E`, `R`, or `O` is invalid (`40.29`). Uses frozen clause time. |
| `VALUE` | `VALUE(name [,newvalue [,pool]])` | `rSYM oANY oANY`, or `rANY oANY oANY` with external pool | Returns old value of a variable and optionally assigns a new value. With external pool, calls configuration get/set. | Internal form must expand compound tails through `RexxVariablePool`; external form raises `40.36`/`40.37` from pool failures. |

## Current cREXX Coverage Snapshot

Existing direct `lib/rxfnsb/rexx/*.crexx` modules correspond to many pure
character, arithmetic, conversion, and other BIF names:

```text
ABBREV ABS B2X C2D C2X CENTER CENTRE CHANGESTR COMPARE COPIES COUNTSTR
DATATYPE DATE DELSTR DELWORD D2C D2X FORMAT INSERT LASTPOS LEFT LENGTH MAX
MIN OVERLAY POS RANDOM REVERSE RIGHT SIGN SPACE STRIP SUBSTR SUBWORD SYMBOL
TIME TRACE TRANSLATE TRUNC VALUE VERIFY WORD WORDINDEX WORDLENGTH WORDPOS
WORDS XRANGE X2B X2C X2D
```

Grouped or partial Level B coverage exists for:

- `CHARIN`, `CHAROUT`, `LINEIN`, `LINEOUT`, and `LINES` in `fileio.crexx`.
- `DIGITS`, `FORM`, and `FUZZ` in `numeric.crexx`.
- command-environment runtime internals in `_address.crexx`.
- trace runtime internals in `trace.crexx`.

Likely Level C work items rather than complete existing surfaces:

- `ARG`, `ADDRESS`, `CONDITION`, `ERRORTEXT`, `QUEUED`, and `SOURCELINE`
  as stateful BIFs.
- `BITAND`, `BITOR`, `BITXOR`, `CHARS`, `QUALIFY`, and `STREAM`.
- `SYMBOL` and `VALUE` over `RexxVariablePool` rather than Level B metadata.
- `DATATYPE` using Level C symbol, numeric, binary, and hex rules exactly.
- `CHARIN`/`CHAROUT`/`LINEIN`/`LINEOUT`/`LINES` as Classic stream functions rather
  than current Level B UTF text helpers.
- A real `Raise`/condition bridge and `ERRORTEXT` catalog lookup.

## Implementation Sequencing

1. Build the shared Level B BIF context and `CheckArgs` helper over
   `RexxValue`.
2. Port/audit pure character and arithmetic BIFs first, using existing
   `rxfnsb` code only where behavior matches this reference.
3. Add `DATATYPE`, `SYMBOL`, and `VALUE` against `RexxVariablePool`; these
   unlock much of the specification helper logic.
4. Add `DATE`, `TIME`, `RANDOM`, `ERRORTEXT`, and numeric state functions.
5. Add coded-character conversion and bit BIFs after the Level C byte-text
   policy is explicit.
6. Add stream and external data queue BIFs once the Level C configuration
   adapter surface is settled.
7. Route RexxScript pure intrinsics through the shared helpers where doing so
   does not grant extra authority to the sandbox.

Do not weaken Level B `.string` UTF-8 guarantees while implementing Classic
byte-oriented behavior. If Level C needs byte-text compatibility, keep it behind
the explicit Level C policy described in the architecture notes.
