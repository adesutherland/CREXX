# Level C Classic BIF Implementation Notes

Status: Classic BIF semantic reference with current cREXX boundary notes
Last updated: 2026-10-08

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
The [compatibility worklist](../../docs/planning/release-1/levelc-compatibility-worklist.md#individual-bif-inventory)
owns live per-BIF implementation status. Earlier BYTE/UTF8 profile proposals
and sequencing notes in this file are superseded by the approved Unicode-first
compiled Level C contract: visible scalars are Unicode text; byte-valued BIFs
use the fixed Latin-1 ordinal bridge and signal for unmappable scalars.
`RexxValue` still has binary storage for direct clients and future facilities.
No BIF is fully reference-qualified merely because its direct runtime entry
exists. The [detailed compatibility-layer review](levelc_compatibility_layer.md)
records every name's actual compiler and runtime entry, arguments, state, tests
and remaining conformance proof. It also distinguishes verified defects from
unprobed inspection concerns; this guide's target tables are not blanket
claims that the full contract is implemented.

Adrian's 2026-10-07 scope clarification preserves current Unicode/I/O
infrastructure; Unicode-caused signals and logic errors are currently undefined
in B/C/G pending compatibility and architectural assessment. It does not approve
new stream support or revise RexxScript's separate binary-capable value model.

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

`RexxBifCallContext` owns a default `RexxClassicConfig` or holds a reference to
an evaluator/host-supplied one. Compiled Level C passes its activation's
Unicode text configuration through direct BIF calls; character positions use
codepoints and byte-valued conversions use the fixed Latin-1 ordinal bridge.
Direct binary clients may still select the shared runtime's BYTE mode; that is
not a compiled Level C language profile. The configuration also carries
character tables, scoped RANDOM state and named external VALUE pools. Value
flags describe a `RexxValue` representation, not a Level C profile selector.

### Numeric Context

The specification helper code uses three numeric contexts:

- `NUM` and `WHOLENUM` checks use the caller's `NUMERIC DIGITS` and
  `NUMERIC FORM`.
- `WHOLE`, `WHOLE>=0`, and `WHOLE>0` checks accept exact whole decimal and
  exponent forms, expand them to plain signed digits, and use the BIF's own internal digits
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
| `ANY` | Compiled Level C requires valid Unicode text; direct binary consumers may use exact byte values through the shared runtime. | `23.1` for invalid text at the text boundary |
| `NUM` | Argument must be numeric under caller settings; normalized numeric value replaces the argument copy. | `40.9`, `40.11` |
| `WHOLE` | Whole number under BIF settings; normalized whole number replaces the argument copy. | `40.12` |
| `WHOLE>=0` | Whole number greater than or equal to zero. | `40.12`, `40.13` |
| `WHOLE>0` | Whole number greater than zero. | `40.12`, `40.14` |
| `WHOLENUM` | D2X-style whole number under caller settings. | `40.12` |
| `WHOLENUM>=0` | D2X-style non-negative whole number under caller settings. | `40.12`, `40.13` |
| `0_90` | `ERRORTEXT` message code with integer part `0` through `90` and fractional part at most `.9`; normalized catalog key must not use exponential notation. | `40.11`, `40.17` |
| `PAD` | Exactly one Unicode codepoint in compiled Level C; direct binary clients use one byte. | `40.23` |
| `HEX` | Hex string according to `DATATYPE(value, "X")`. | `40.25` |
| `BIN` | Binary string according to `DATATYPE(value, "B")`. | `40.24` |
| `SYM` | Valid symbol according to `DATATYPE(value, "S")`. | `40.26` |
| `STREAM` | Valid stream name according to `Config_Stream_Qualify`. | `40.27` |
| Option set, for example `LTB` | Non-null; first character, uppercased, must be in the option set. | `40.21`, `40.28` |
| `ACEFILNOR` | Normalized Classic TRACE option reference; the current direct TRACE BIF actually validates `oANY` and sends the full string to its existing state parser, including documented extensions. | `40.28` |

The message catalog is in `compiler/docs/levelc_standard_error_messages.md`.
The extracted helper calls `40.16` for the `0_90` range failure. The standard
catalog and Regina identify this error as `40.17`; the implementation uses
that range-specific identity. Decimal subcode digits, including trailing zeros,
remain part of the catalog key. `ERRORTEXT` shares the generated English catalog
with diagnostics and uses Classic angle-bracket place-markers. Undefined codes
return empty text; `N` falls back to the shipped English catalog.

## Implemented Runtime Slice

The shared context and legacy proof dispatcher live in
`lib/rxfnsc/RexxClassicBifs.crexx`. New direct BIF implementations are
standalone `RexxClassicBif*.crexx` modules in the same runtime image with
`RexxValue`, `RexxStem`, and `RexxVariablePool`. Most per-name harnesses call
those context entries directly, without compiler lowering or the common
name dispatcher. Important exceptions remain: the bitwise compiler entries
call context functions in `RexxClassicBifs`, and the named SUBWORD unit still
exercises its retained shared-module body rather than the standalone compiler
target. Compiled SUBWORD panels reach the standalone entry, but the unit
name alone does not qualify that entry's complete error/configuration matrix.

Classic BIF names reachable through the compiler's direct table (62 of 70):

```text
ABBREV ABS ADDRESS ARG B2X BITAND BITOR BITXOR C2D C2X CENTER CENTRE
CHANGESTR COMPARE CONDITION COPIES COUNTSTR D2C D2X DATE DATATYPE DELSTR
DELWORD DIGITS ERRORTEXT FORM FORMAT FUZZ INSERT LASTPOS LEFT LENGTH MAX
MIN OVERLAY POS QUEUED RANDOM REVERSE RIGHT SIGN SOURCELINE SPACE STRIP
SUBSTR SUBWORD SYMBOL TIME TRACE TRANSLATE TRUNC VALUE VERIFY WORD
WORDINDEX WORDLENGTH WORDPOS WORDS X2B X2C X2D XRANGE
```

Aliases and the bitwise entries share their existing implementations. LOWER and
UPPER are additional direct names outside the 70-name Classic catalog. CHARIN,
CHAROUT, CHARS, LINEIN, LINEOUT, LINES, QUALIFY and STREAM remain pending by
Adrian's 2026-10-07 direction: no new stream/Unicode infrastructure is approved.
QUEUED uses the existing selected queue; SOURCELINE retains the compilation
unit's original physical lines; ERRORTEXT shares the standard diagnostic catalog.
The worklist owns their contract receipts and remaining source/host limits.

The legacy proof dispatcher remains for compatibility tests. Compiled Level C
uses the existing direct table and one common argument/result path; it does
not call the dispatcher. Retained same-named shared bodies and typed wrappers
must be distinguished from that table's actual targets.

The public proof API is:

```text
RexxBifCallContext
rexxclassicbif_call(reference context)
rexxclassicbif_check_args(reference context, checklist)
rexxclassicbif_length(value)
```

`RexxBifCallContext` carries the uppercased BIF name, `RexxValue` argument
values, argument presence flags, a live caller `RexxVariablePool` reference,
and the active `RexxClassicConfig` reference.
The BIF operand count is the presence-vector slot count (`exists[0]`). It
counts positions, including omissions, rather than true presence flags or
nonblank values. This preserves omitted positions such as `xxx(,a,,b)`. Level
C compiler lowering was unchanged by the original library programme. Current
Level C lowering calls direct BIF entries with the visible activation pool,
argument presence and configuration; the legacy dispatcher is no longer on
its expression path. RexxScript calls
the available standalone entries directly, passes only its sandbox/script pool,
and adapts the returned `RexxValue` back to its public string result model.

The shared `CheckArgs` validator supports:

```text
ANY NUM WHOLE WHOLE>=0 WHOLE>0 WHOLENUM WHOLENUM>=0 0_90 PAD BIN HEX SYM
```

Option-set rules are supplied by each BIF (for example `ABLMNSUWX`, `LTB`,
`CDEIS`, `SN` and `EO`). Stream-name validation is still a pending host contract.
Positional/count WHOLE operands use the approved inclusive signed 64-bit range;
outside it they report source-anchored `40.12` before VM conversion. WHOLENUM
radix numeric operands retain arbitrary precision. This integer argument limit
is not a promise that every in-range allocation can be fulfilled.

ANSI/Classic caller-DIGITS initial rounding applies to ABS, MAX, MIN, SIGN,
TRUNC and FORMAT. The existing B/G decimal BIFs use the same rounding and
inherited DIGITS/FORM while retaining their typed arguments, results and
signals. The common numeric implementation performs normalization once; direct
ABS/MAX/MIN/SIGN entries are wrappers over it. Regina 3.9.7's preserved-operand
behavior at reduced DIGITS differs from the approved ANSI rule. Unicode-caused
signals or other logic errors in B/C/G remain undefined within the current infrastructure
until Adrian's compatibility/architecture assessment.

`RexxClassicDatatype.crexx` is the shared implementation for `NUM`, `WHOLE`,
`BIN`, `HEX`, and `SYM`. It uses the call context's character configuration, configured
extra letter/digit maps, configured B/X blanks, and exponent-digit limit. The
standalone public entry is `RexxClassicBifDatatype.crexx`; the same shared symbol
classifier is used by `RexxClassicBifSymbol.crexx`. Strict Level C retains the
standard `ABLMNSUWX` option set, while Level B separately owns the cREXX `D`
extension.

`rexxclassicbif_call()` returns a `RexxValue`. On validation or dispatch
failure it returns a blank value and records the error on the context through
`hasError()`, `errorCode()`, and `errorMessage()`. This keeps the shared BIF
engine value-native for Level C and future optimising rewrites while preserving
a single place for Classic message construction. Level C direct helpers such as
`rexxclassicbiflength.rexxclassicbif_length(context_ref)` return freshly
materialised `RexxValue` results;
they do not return aliases to values held by a variable pool.

Level C must remain Classic Rexx compliant: Classic argument validation,
message codes, and condition behavior are not to be weakened to match Level B
convenience behavior. Separately, the planned Level B library review will move
Level B library failures toward signal-based reporting, and that change is not
expected to be backward compatible. Keep the bridge centralized in
`RexxClassicBifs.crexx` through `rexxclassicbif_check_args`, `_set_bif_error`,
and `_context_error`; do not spread one-off error formatting or signal
decisions through individual BIF bodies.

The compiler now materialises a reusable BIF argument frame, including
provided flags for omitted positions, and calls direct entries. The full
argument, error and configured-context reference audit remains open.

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
- The Classic target freezes one clock sample per clause through
  `#ClauseTime.#Level` and `#ClauseLocal.#Level`. Current shared runtime state
  is not refreshed by production clause lowering: DATE/TIME call
  `ensureClauseTime()`, and no production caller invokes `beginClauseTime()`.
  A linked opt/no-opt probe confirms unchanged TIME(L) and TIME(E)=0 across
  a one-second host sleep. DATE shares that cached sample, with calendar
  crossing itself unprobed. Injected-clock unit results do not qualify this
  known integration defect.

DATE and TIME share `RexxDateTimeState` and Gregorian helpers.
Their conversion tests remain valid while clause refresh and platform clock
lifecycle remain open under LC-GAP-02/04 and LC-REF-057.

### Radix Helper

`ReRadix(subject, fromRadix, toRadix)` converts between radices 2, 10, and 16
through decimal, preserving binary/hex widths for `B2X` and `X2B`.

This should become a shared Level B helper over `RexxValue` numeric/binary
views. Compiled Level C string results must use the approved Latin-1 ordinal
bridge; arbitrary encoded bytes are not valid Level C `.string` payloads.

### Raise Helper

The extracted `Raise` helper raises `SYNTAX` and always includes the BIF name as
the first insert for `40.*` errors. It does not return.

The older `lib/rxfnsb/rexx/raise.crexx` placeholder is not the compiled-C
condition bridge. Current direct BIFs record a standard identity and message inserts on
`RexxBifCallContext` through the shared error builder. The common compiler
result guard raises `CLASSIC_SYNTAX` at the authored call; the activation and
generated diagnostic catalog supply admitted trap/CONDITION reporting. A
direct library caller can inspect the context without raising. Full producer,
.MN, locale and source traceback equivalence remains open in the worklist.

### Character Configuration

The character configuration is implemented entirely in the library/runtime:

- `RexxClassicConfig` keeps BYTE for direct binary consumers and supplies the
  Unicode text configuration used by compiled Level C, plus configured blanks;
- `RexxClassicCharacterScan` supplies configured word scans, retaining the
  VM Unicode fast path when there are no extra blanks;
- `RexxClassicEncoding` supplies exact byte/hex conversion for binary
  consumers and the reversible Latin-1 ordinal bridge for Level C;
- the Unicode data contract is pinned to Unicode 17.0.0;
- Level C byte-valued BIFs map `U+00XX` to byte `XX` and back; a higher scalar
  reports `RXC-LC-23.1` through the normal BIF context.

`XRANGE` uses the same ordinal mapping and wraps after `U+00FF`. It is not a
general Unicode range operation. Compiled Level C initializes its text
configuration explicitly; the shared BYTE default is for direct clients only.

### Configuration Dependencies

Several BIFs are not pure string/numeric functions. The following table maps
Classic service obligations; presence in it does not mean every hook is a
shipped cREXX host API:

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

Rows normalize the Classic target and identify current implementation notes.
The detailed compatibility-layer review supplies each name's actual paths,
tests and remaining obligations; stream rows below describe deferred target behavior.

Argument checklist values are normalized from the public language-specification
source for planning. This is implementation guidance, not a code copy.

### Character Built-in Functions

| Function | Signature | Checklist | Definition summary | Level B/RexxValue notes |
| --- | --- | --- | --- | --- |
| `ABBREV` | `ABBREV(string, abbrev [,length])` | `rANY rANY oWHOLE>=0` | Returns `1` when `abbrev` matches the leading characters of `string` and is at least `length` characters long. | Direct configured prefix helper over RexxValue; default C positions are codepoints. |
| `CENTER` | `CENTER(string, length [,pad])` | `rANY rWHOLE>=0 oPAD` | Centers or trims `string` to `length`, using `pad` or blank. | Alias target for `CENTRE`; preserve one-character pad rule. |
| `CENTRE` | `CENTRE(string, length [,pad])` | same as `CENTER` | Alternative spelling of `CENTER`. | Direct alias already forwards to the shared CENTER implementation. |
| `CHANGESTR` | `CHANGESTR(needle, haystack, replacement)` | `rANY rANY rANY` | Replaces all non-overlapping occurrences of `needle` in `haystack`. | Empty needle returns unchanged haystack; the named runtime unit asserts it. Compiled reference cases cover nonempty replacement. |
| `COMPARE` | `COMPARE(left, right [,pad])` | `rANY rANY oPAD` | Returns `0` if equal, otherwise the first differing 1-based character position after padding the shorter side. | Compiled Level C positions count Unicode codepoints. |
| `COPIES` | `COPIES(string, count)` | `rANY rWHOLE>=0` | Concatenates `count` copies of `string`. | Existing VM allocation mechanisms apply; full limit/exhaustion behavior remains unqualified. |
| `COUNTSTR` | `COUNTSTR(needle, haystack)` | `rANY rANY` | Counts non-overlapping appearances of `needle` in `haystack`. | Shared configured search; empty needle returns zero and is tested. |
| `DATATYPE` | `DATATYPE(string [,type])` | `rANY oABLMNSUWX` | With no type, returns numeric/character classification. With type, tests alphanumeric, binary, lowercase, mixed letters, number, symbol, uppercase, whole, or hex. | This is a core helper for `CheckArgs`; it must match Classic syntax, not current Level B keyword rules. |
| `DELSTR` | `DELSTR(string, start [,length])` | `rANY rWHOLE>0 oWHOLE>=0` | Deletes a character substring from `start`; omitted length deletes through the end. | 1-based character indexes. |
| `DELWORD` | `DELWORD(string, start [,count])` | `rANY rWHOLE>0 oWHOLE>=0` | Deletes words beginning at word `start`; omitted count deletes through the end. | Word boundaries are blank/equivalent blank based on Classic rules. |
| `INSERT` | `INSERT(new, target [,before [,length [,pad]]])` | `rANY rANY oWHOLE>=0 oWHOLE>=0 oPAD` | Inserts `new` after `before` characters of `target`, padding/truncating inserted text to `length` when supplied. | The specification text says "before the insert"; behavior is the classic 0-based insertion point in a 1-based API. |
| `LASTPOS` | `LASTPOS(needle, haystack [,start])` | `rANY rANY oWHOLE>0` | Finds the last occurrence of `needle`, optionally searching leftward from `start`. | Returns `0` on no match. |
| `LEFT` | `LEFT(string, length [,pad])` | `rANY rWHOLE>=0 oPAD` | Returns leftmost `length` characters, padding on the right if needed. | Character indexing. |
| `LENGTH` | `LENGTH(string)` | `rANY` | Returns the configuration character length. Raises `23.1` if the string is invalid for the configuration. | Compiled Level C `.string` values are valid Unicode text and length counts codepoints. |
| `OVERLAY` | `OVERLAY(new, target [,start [,length [,pad]]])` | `rANY rANY oWHOLE>0 oWHOLE>=0 oPAD` | Overlays `new` onto `target` at `start`, padding/truncating overlay text to `length` when supplied. | Similar shared helper with `INSERT`. |
| `POS` | `POS(needle, haystack [,start])` | `rANY rANY oWHOLE>0` | Finds the first occurrence of `needle` at or after `start`; returns `0` if not found. | `needle == ""` returns `0` in the specification code. |
| `REVERSE` | `REVERSE(string)` | `rANY` | Reverses the sequence of characters. | Must not reverse UTF-8 bytes in normal UTF builds. |
| `RIGHT` | `RIGHT(string, length [,pad])` | `rANY rWHOLE>=0 oPAD` | Returns rightmost `length` characters, padding on the left if needed. | Character indexing. |
| `SPACE` | `SPACE(string [,count [,pad]])` | `rANY oWHOLE>=0 oPAD` | Removes leading/trailing/intermediate blank runs and rejoins words with `count` pad characters. | Default count is one, default pad is blank. |
| `STRIP` | `STRIP(string [,option [,char]])` | `rANY oLTB oPAD` | Strips leading, trailing, or both occurrences of `char`; default is both blanks. | Option first letter: `L`, `T`, `B`. |
| `SUBSTR` | `SUBSTR(string, start [,length [,pad]])` | `rANY rWHOLE>0 oWHOLE>=0 oPAD` | Returns substring from `start`, padding if requested length extends beyond input. | Standard checks that requested start can reference the string or raise invalid data as appropriate. |
| `SUBWORD` | `SUBWORD(string, start [,count])` | `rANY rWHOLE>0 oWHOLE>=0` | Returns a substring made of words from word `start`, for `count` words or through the end. | Share word scanner with `WORD*` functions. |
| `TRANSLATE` | `TRANSLATE(string [,outputTable [,inputTable [,pad]]])` | `rANY oANY oANY oPAD` | Uppercases by configuration when no tables are supplied; otherwise maps characters from input table to output table, using pad for missing output entries. | Compiled Level C's implicit input table is U+0000–U+00FF in ordinal order; higher Unicode scalars remain unchanged. Direct BYTE consumers retain byte behavior. |
| `VERIFY` | `VERIFY(string, reference [,option [,start]])` | `rANY rANY oMN oWHOLE>0` | With `M`, returns first character position in `string` that is in `reference`; with `N`, first position not in `reference`; `0` if no such character. | Default option is `N`, default start is `1`. |
| `WORD` | `WORD(string, n)` | `rANY rWHOLE>0` | Returns word `n`, or null if absent. | Can delegate to `SUBWORD(string,n,1)`. |
| `WORDINDEX` | `WORDINDEX(string, n)` | `rANY rWHOLE>0` | Returns the character index of word `n`, or `0` if absent. | Needs shared word scanner that preserves original spacing. |
| `WORDLENGTH` | `WORDLENGTH(string, n)` | `rANY rWHOLE>0` | Returns the length of word `n`, or `0` if absent. | Can share `WORD` extraction. |
| `WORDPOS` | `WORDPOS(phrase, string [,start])` | `rANY rANY oWHOLE>0` | Finds a sequence of words from `phrase` in `string`; returns the word position or `0`. | Compare normalized word sequences, not raw spacing. |
| `WORDS` | `WORDS(string)` | `rANY` | Counts blank-delimited words. | Share word scanner. |
| `XRANGE` | `XRANGE([start [,end]])` | `oPAD oPAD` | Returns configured byte ordinals from start through end, wrapping at `FF`. | In compiled Level C, returns `U+00XX` scalars. |

### Arithmetic Built-in Functions

| Function | Signature | Checklist | Definition summary | Level B/RexxValue notes |
| --- | --- | --- | --- | --- |
| `ABS` | `ABS(number)` | `rNUM` | Returns the absolute value after caller-context numeric normalization. | Use `RexxValue.asDecimal()` under caller numeric settings. |
| `FORMAT` | `FORMAT(number [,before [,after [,expp [,expt]]]])` | `rNUM oWHOLE>=0 oWHOLE>=0 oWHOLE>=0 oWHOLE>=0` | Formats a number with requested integer, fractional, and exponent widths, using caller form/scientific/engineering rules. | Initial caller-DIGITS rounding follows the approved ANSI rule; 40.38 reports insufficient width. C and typed B/G paths retain distinct validation and error APIs. |
| `MAX` | `MAX(number, ...)` | generated `rNUM...` | Returns the largest numeric argument. At least one argument is required. | `MAX()` with zero args raises `40.3`. |
| `MIN` | `MIN(number, ...)` | generated `rNUM...` | Returns the smallest numeric argument. At least one argument is required. | `MIN()` with zero args raises `40.3`. |
| `SIGN` | `SIGN(number)` | `rNUM` | Returns `-1`, `0`, or `1` according to numeric sign. | Use normalized numeric comparison. |
| `TRUNC` | `TRUNC(number [,digits])` | `rNUM oWHOLE>=0` | Truncates to integer or to `digits` fractional digits without rounding. | Preserve Classic string form, including leading `0.` construction. |

### State Built-in Functions

| Function | Signature | Checklist | Definition summary | Level B/RexxValue notes |
| --- | --- | --- | --- | --- |
| `ADDRESS` | `ADDRESS([option])` | `oEINO` | Returns current command environment name, or input/output target position/type/resource for option `E`, `I`, or `O`. | Use current `ADDRESS` runtime state, not command dispatch. |
| `ARG` | `ARG([n [,option]])` | `oWHOLE>0 oEO`; option with omitted n is 40.5 | Calling with no arguments returns the highest supplied invocation position, excluding trailing omissions. One argument returns argument n. E/O test whether that position was supplied or omitted. | Direct activation entry preserves supplied-empty versus omitted slots. |
| `CONDITION` | `CONDITION([option])` | `oCDEIS` | Returns current condition name, description, extra data, instruction, or enabled state. Null when no current condition. | Admitted C/D/E/I/S fields, seven IDs, ON/OFF/DELAY, extra clearing and SYNTAX/NOVALUE/LOSTDIGITS/live ADDRESS producers have evidence. Real host HALT and complete cross-service lifecycle remain open. |
| `DIGITS` | `DIGITS()` | none | Returns current `NUMERIC DIGITS`. | Existing `numeric.crexx` is relevant but Level C must use Classic current frame settings. |
| `ERRORTEXT` | `ERRORTEXT(code [,option])` | `r0_90 oSN` | Returns unexpanded message text; `S` requests specification English, `N` allows localized text. | Uses the same generated English catalog as diagnostics; N uses English fallback, and undefined codes return empty. Range errors use 40.17. |
| `FORM` | `FORM()` | none | Returns current `NUMERIC FORM`. | Must return Classic form wording. |
| `FUZZ` | `FUZZ()` | none | Returns current `NUMERIC FUZZ`. | Must follow current frame. |
| `SOURCELINE` | `SOURCELINE([n])` | `oWHOLE>0` | No arg returns visible source line count or `0`; arg returns source line `n`. | Raises `40.34` beyond available source. Uses compiler-retained physical lines on ordinary source input, shared by local routines and isolated per separately compiled Classic unit. Generated source-map input reports unavailable (count zero); full mapped-source inventory remains open. |
| `TRACE` | `TRACE([option])` | Current implementation: `oANY` | Returns previous setting and applies the full option string through the existing TRACE state parser. | Compiled entry uses activation state; direct client entry uses pool state. Empty reset, numeric and toggle forms, and AS/ASM/LL/LLM extensions are current behaviors; approved practical divergences are recorded under LC-I-24. |

### Conversion Built-in Functions

| Function | Signature | Checklist | Definition summary | Level B/RexxValue notes |
| --- | --- | --- | --- | --- |
| `B2X` | `B2X(binaryDigits)` | `rBIN` | Removes blanks and converts binary digit text to hexadecimal digit text. | Operates on textual `0`/`1` digits, not `.binary` buffers. |
| `BITAND` | `BITAND(left [,right [,pad]])` | `rANY oANY oPAD` | Applies bitwise AND over the common ordinal length and preserves the longer tail. | Level C uses the shared Latin-1 ordinal bridge; higher scalars raise `23.1`. |
| `BITOR` | `BITOR(left [,right [,pad]])` | same as `BITAND` | Same as `BITAND`, using bitwise OR. | Uses the same bridge and helper. |
| `BITXOR` | `BITXOR(left [,right [,pad]])` | same as `BITAND` | Same as `BITAND`, using bitwise exclusive OR. | Uses the same bridge and helper. |
| `C2D` | `C2D(string [,length])` | `rANY oWHOLE>=0` | Converts byte ordinals to decimal. With length, treats the rightmost `length` bytes as a signed twos-complement value. | Level C uses `U+00XX`; higher scalars raise `23.1`, and digits overflow raises `40.35`. |
| `C2X` | `C2X(string)` | `rANY` | Converts byte ordinals to uppercase hexadecimal. | Level C uses `U+00XX`; higher scalars raise `23.1`. |
| `D2C` | `D2C(number [,length])` | `rWHOLENUM>=0`, or `rWHOLENUM rWHOLE>=0` | Converts a decimal whole number to byte ordinals; with length, pads/truncates according to sign. | Level C returns text `U+00XX`; negative values use twos-complement. |
| `D2X` | `D2X(number [,length])` | `rWHOLENUM>=0`, or `rWHOLENUM rWHOLE>=0` | Converts decimal whole number to hex; with length, pads/truncates with `0` or `F` depending on sign. | Negative values use twos-complement. |
| `X2B` | `X2B(hex)` | `rHEX` | Removes blanks and converts hex digit text to binary digit text. | Empty input returns null. |
| `X2C` | `X2C(hex)` | `rHEX` | Converts hex digit text to byte ordinals, left-padding to a full byte as needed. | Level C returns text `U+00XX`, including `U+0000` and `U+00FF`. |
| `X2D` | `X2D(hex [,length])` | `rHEX oWHOLE>=0` | Converts hex digit text to decimal. With length, interprets sign bit for twos-complement. | Raises `40.35` when result cannot fit current digits. |

### Input/Output Built-in Functions

The Classic I/O BIFs are configuration stream APIs. Existing Level B `fileio.crexx`
functions are UTF text conveniences and should not be treated as conformant
Level C stream implementations without an audit. The eight names below have
recognition only, with no direct compiled-C implementation. Adrian deferred
stream/Unicode infrastructure changes pending a compatibility and architectural
assessment; these rows retain target obligations and do not approve the RXPA
stream proposal or a new ABI.

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
| `DATE` | `DATE([option [,date [,inoption]]])` | `oBDEMNOSUW oANY oBDENOSU` | With no date, returns current local date in requested format. With date, converts from `inoption` to output option. | Explicit conversion and injected-sample formatting are tested; current production sample is pool-cached without clause refresh. Calendar-crossing consequences are inferred, not probed. Invalid conversion is 40.19. |
| `QUEUED` | `QUEUED()` | none | Returns number of lines in the external data queue. | Direct entry counts the existing selected execution-local repository without consuming input. Named host selection uses RXQUEUE; a wider C/Level C selector remains open. |
| `RANDOM` | `RANDOM([max])` or `RANDOM([min [,max [,seed]]])` | `oWHOLE>=0 oWHOLE>=0 oWHOLE>=0` | Returns pseudo-random whole number in range. One argument means `0..arg`; defaults are `0..999`. | Range must be no wider than `100000`; raises `40.31`, `40.32`, `40.33`. |
| `SYMBOL` | `SYMBOL(name)` | prose-defined | Returns `BAD` if argument is not a valid symbol, `LIT` if symbol is valid but dropped/literal, or `VAR` if it has a value. | Must use Level C symbol recognition and `RexxVariablePool`, not Level B keyword metadata. |
| `TIME` | `TIME([option [,time [,inoption]]])` | `oCEHLMNORS oANY oCHLMNS` | With no time, returns current local time, elapsed time, reset elapsed time, or offset. With time, converts from `inoption` to output option. | Conversion to E/R/O is invalid (40.29). Injected tests cover formatting/elapsed/reset, but a linked opt/no-opt probe reproduces missing real clause refresh and elapsed zero after one second. |
| `VALUE` | `VALUE(name [,newvalue [,pool]])` | `rSYM oANY oANY`, or `rANY oANY oANY` with external pool | Returns old value of a variable and optionally assigns a new value. With external pool, calls configuration get/set. | Internal form must expand compound tails through `RexxVariablePool`; external form raises `40.36`/`40.37` from pool failures. |

## Level B source material and Level C status

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

Current admitted stateful surfaces include ADDRESS, ARG, CONDITION, ERRORTEXT,
QUEUED and SOURCELINE. CONDITION fields/producers and the shared English catalog
have focused receipts; SOURCELINE retains original ordinary physical source
lines but source-map input reports unavailable/count zero. Physical source NUL
truncation remains a known source defect. These entries do not close wider
host, mapped-source, locale or lifecycle obligations.

The eight missing compiled-C surfaces are CHARIN, CHAROUT, CHARS, LINEIN,
LINEOUT, LINES, QUALIFY and STREAM. The bit BIFs already have direct entries
over the current fixed Latin-1 boundary. The shared condition/message bridge
is implemented on admitted producers; real host HALT, complete cross-service
state and full diagnostic/source proof remain open.

## Remaining obligations and known issues

The worklist owns delivery order and decisions. This guide does not create a
competing implementation plan. Current outstanding items are:

1. The reproduced TIME clause-clock integration defect. DATE shares the
   inspected cache; calendar crossing was not probed. Pure conversion and
   manually injected clock tests do not establish real clock progression.
2. Complete per-name reference/error/source/resource proof for the 62-entry
   admitted baseline, including mapped/NUL source, real host HALT and wider
   host/context isolation obligations. Positional range acceptance is not an
   all-in-range allocation promise.
3. Configured BIN/HEX extra blank/digit validation versus ASCII-only conversion
   loops: an inspected custom-context concern needing a focused probe, not a
   reproduced default-ASCII failure. Unicode-caused behavior is currently
   undefined within Adrian's deferred assessment.
4. SUBWORD's retained shared body versus standalone compiler target: its named
   unit covers the retained body, while compiled panels cover the standalone
   entry. Full per-name unit-matrix equivalence/migration remains open.
5. The eight stream BIFs and new Unicode/I/O infrastructure remain deferred
   pending assessment. Existing selected-queue count is implemented; a wider
   host selector does not follow from QUEUED.
6. RexxScript retains its sandbox allow-list and separate binary-capable value
   behavior. Any future intrinsic migration must preserve that authority
   boundary. Legacy dispatcher consumers remain distinct from the compiler
   direct path; source comments describing current compiler use are stale
   cleanup items, left unchanged by this documentation-only review.

Keep Level B `.string` UTF-8 guarantees and the approved Level C Unicode scalar
contract. Raw binary I/O and explicit Unicode BIFs have separate open design
and qualification rows in the worklist.

QUEUED has a direct compiler/runtime entry. It counts the same execution-local
selected repository as PULL/PUSH/QUEUE, without consuming a line or falling back
to default input. The existing Level B RXQUEUE API selects named queues for direct
hosts; adding a Level C/C host selector remains separate host work. STEP-90C in
the worklist records focused direct/linked opt/no-opt proof. Adrian deferred
changes to Unicode and I/O infrastructure on 2026-10-07; Unicode-driven signals
and logic behavior are currently undefined for this BIF checkpoint.

Numeric `NUM` operands of ABS, MAX, MIN, SIGN, TRUNC and FORMAT are first
rounded as `number + 0` under caller DIGITS and FORM (ANSI/Classic rules).
The Level B/G typed decimal BIFs use the same rule. Positional/count `WHOLE`
operands accept exact decimal/exponent spellings within the signed 64-bit
integer range; values outside it report Classic 40.12 before VM conversion.
Radix `WHOLENUM` operands retain their arbitrary-precision caller-DIGITS rule.
