# Level C Standard Error Messages

Status: extracted reference for Level C diagnostics
Last updated: 2026-05-10

Source: draft ANSI REXX standard PDF,
`draft-ansi-rexx-standard.pdf`, section 8.2.1, printed pages 63-70
(PDF pages 77-84).

This file records the standard `#ErrorText.` catalog in a normalized
one-message-per-line form. It is intended to become the Level C diagnostic
message catalog used by parser recovery, validation, `ERRORTEXT`, and later
runtime condition reporting.

The PDF is the source of truth. An older `docs/bifs/#ansibifs.rexx#` copy exists
in the repository, but it differs from the PDF in several entries and should not
be used as the Level C baseline without reconciliation.

## Parser-Recovery Priority Subset

The first Level C parser recovery work should target messages that help bad
input resynchronize at clause, group, expression, and template boundaries:

- `6`, `6.1`, `6.2`, `6.3`: unmatched comment or quote.
- `7.*`: `SELECT` body recovery.
- `8.*`, `18.*`: unexpected or missing `THEN`/`ELSE`.
- `9.*`: unexpected `WHEN`/`OTHERWISE`.
- `10.*`, `14.*`: `END`, `DO`, `SELECT`, `IF` structure recovery.
- `13.*`: invalid source character.
- `15.*`: invalid hexadecimal/binary strings.
- `19.*`, `20.*`, `21.*`, `25.*`: keyword-tail and clause-end recovery.
- `27.*`, `28.*`: `DO`, `LEAVE`, and `ITERATE` structural validation.
- `31.*`: invalid assignment or variable-symbol left side.
- `35.*`, `36`, `37.*`: expression recovery.
- `38.*`: parse-template recovery.
- `46.*`, `50.*`, `51.*`: variable/reserved/function-name syntax.

## Interim Diagnostic Record Format

Until the shared formatter exists, Level C parser and validation code should
emit the standard identity plus named inserts in the existing diagnostic string
field:

```text
RXC-LC-<standard-code> [insert-name="escaped value" ...]
```

Examples:

```text
RXC-LC-18.1 linenumber="12"
RXC-LC-35.1 token="then"
RXC-LC-40.4 name="FOO"
```

The initial implementation uses this form for DSLSH and parser-mode tests.
Message rendering from this catalog is a later common stage, so new tests should
assert the `RXC-LC-...` identity first and only assert insert payloads where
the insert value is part of the expected parser contract.

## Output Prefixes

| Code | Standard message text |
| --- | --- |
| `0.1` | Error `<value>` running `<source>`, line `<linenumber>`: |
| `0.2` | Error `<value>` in interactive trace: |
| `0.3` | Interactive trace. "Trace Off" to end debug. ENTER to continue. |

## Catalog

| Code | Standard message text |
| --- | --- |
| `2` | Failure during finalization |
| `2.1` | Failure during finalization: `<description>` |
| `3` | Failure during initialization |
| `3.1` | Failure during initialization: `<description>` |
| `4` | Program interrupted |
| `4.1` | Program interrupted with HALT condition: `<description>` |
| `5` | System resources exhausted |
| `5.1` | System resources exhausted: `<description>` |
| `6` | Unmatched "/*" or quote |
| `6.1` | Unmatched comment delimiter ("/*") |
| `6.2` | Unmatched single quote (') |
| `6.3` | Unmatched double quote (") |
| `7` | WHEN or OTHERWISE expected |
| `7.1` | SELECT on line `<linenumber>` requires WHEN; found "`<token>`" |
| `7.2` | SELECT on line `<linenumber>` requires WHEN, OTHERWISE, or END; found "`<token>`" |
| `7.3` | All WHEN expressions of SELECT on line `<linenumber>` are false; OTHERWISE expected |
| `8` | Unexpected THEN or ELSE |
| `8.1` | THEN has no corresponding IF or WHEN clause |
| `8.2` | ELSE has no corresponding THEN clause |
| `9` | Unexpected WHEN or OTHERWISE |
| `9.1` | WHEN has no corresponding SELECT |
| `9.2` | OTHERWISE has no corresponding SELECT |
| `10` | Unexpected or unmatched END |
| `10.1` | END has no corresponding DO or SELECT |
| `10.2` | END corresponding to DO on line `<linenumber>` must have a symbol following that matches the control variable (or no symbol); found "`<token>`" |
| `10.3` | END corresponding to DO on line `<linenumber>` must not have a symbol following it because there is no control variable; found "`<token>`" |
| `10.4` | END corresponding to SELECT on line `<linenumber>` must not have a symbol following; found "`<token>`" |
| `10.5` | END must not immediately follow THEN |
| `10.6` | END must not immediately follow ELSE |
| `13` | Invalid character in program |
| `13.1` | Invalid character in program "`<char>`" ('`<hex-encoding>`'X) |
| `14` | Incomplete DO/SELECT/IF |
| `14.1` | DO instruction requires a matching END |
| `14.2` | SELECT instruction requires a matching END |
| `14.3` | THEN requires a following instruction |
| `14.4` | ELSE requires a following instruction |
| `15` | Invalid hexadecimal or binary string |
| `15.1` | Invalid location of blank in position `<position>` in hexadecimal string |
| `15.2` | Invalid location of blank in position `<position>` in binary string |
| `15.3` | Only 0-9, a-f, A-F, and blank are valid in a hexadecimal string; found "`<char>`" |
| `15.4` | Only 0, 1, and blank are valid in a binary string; found "`<char>`" |
| `16` | Label not found |
| `16.1` | Label "`<name>`" not found |
| `16.2` | Cannot SIGNAL to label "`<name>`" because it is inside an IF, SELECT or DO group |
| `16.3` | Cannot invoke label "`<name>`" because it is inside an IF, SELECT or DO group |
| `17` | Unexpected PROCEDURE |
| `17.1` | PROCEDURE is valid only when it is the first instruction executed after an internal CALL or function invocation |
| `18` | THEN expected |
| `18.1` | IF keyword on line `<linenumber>` requires matching THEN clause; found "`<token>`" |
| `18.2` | WHEN keyword on line `<linenumber>` requires matching THEN clause; found "`<token>`" |
| `19` | String or symbol expected |
| `19.1` | String or symbol expected after ADDRESS keyword; found "`<token>`" |
| `19.2` | String or symbol expected after CALL keyword; found "`<token>`" |
| `19.3` | String or symbol expected after NAME keyword; found "`<token>`" |
| `19.4` | String or symbol expected after SIGNAL keyword; found "`<token>`" |
| `19.6` | String or symbol expected after TRACE keyword; found "`<token>`" |
| `19.7` | Symbol expected in parsing pattern; found "`<token>`" |
| `20` | Name expected |
| `20.1` | Name required; found "`<token>`" |
| `20.2` | Found "`<token>`" where only a name is valid |
| `21` | Invalid data on end of clause |
| `21.1` | The clause ended at an unexpected token; found "`<token>`" |
| `22` | Invalid character string |
| `22.1` | Invalid character string '`<hex-encoding>`'X |
| `23` | Invalid data string |
| `23.1` | Invalid data string '`<hex-encoding>`'X |
| `24` | Invalid TRACE request |
| `24.1` | TRACE request letter must be one of "ACEFILNOR"; found "`<value>`" |
| `25` | Invalid sub-keyword found |
| `25.1` | CALL ON must be followed by one of the keywords `<keywords>`; found "`<token>`" |
| `25.2` | CALL OFF must be followed by one of the keywords `<keywords>`; found "`<token>`" |
| `25.3` | SIGNAL ON must be followed by one of the keywords `<keywords>`; found "`<token>`" |
| `25.4` | SIGNAL OFF must be followed by one of the keywords `<keywords>`; found "`<token>`" |
| `25.5` | ADDRESS WITH must be followed by one of the keywords `<keywords>`; found "`<token>`" |
| `25.6` | INPUT must be followed by one of the keywords `<keywords>`; found "`<token>`" |
| `25.7` | OUTPUT must be followed by one of the keywords `<keywords>`; found "`<token>`" |
| `25.8` | APPEND must be followed by one of the keywords `<keywords>`; found "`<token>`" |
| `25.9` | REPLACE must be followed by one of the keywords `<keywords>`; found "`<token>`" |
| `25.11` | NUMERIC FORM must be followed by one of the keywords `<keywords>`; found "`<token>`" |
| `25.12` | PARSE must be followed by one of the keywords `<keywords>`; found "`<token>`" |
| `25.13` | UPPER must be followed by one of the keywords `<keywords>`; found "`<token>`" |
| `25.14` | ERROR must be followed by one of the keywords `<keywords>`; found "`<token>`" |
| `25.15` | NUMERIC must be followed by one of the keywords `<keywords>`; found "`<token>`" |
| `25.16` | FOREVER must be followed by one of the keywords `<keywords>` or nothing; found "`<token>`" |
| `25.17` | PROCEDURE must be followed by the keyword EXPOSE or nothing; found "`<token>`" |
| `26` | Invalid whole number |
| `26.1` | Whole numbers must fit within current DIGITS setting(`<value>`); found "`<value>`" |
| `26.2` | Value of repetition count expression in DO instruction must be zero or a positive whole number; found "`<value>`" |
| `26.3` | Value of FOR expression in DO instruction must be zero or a positive whole number; found "`<value>`" |
| `26.4` | Positional pattern of parsing template must be a whole number; found "`<value>`" |
| `26.5` | NUMERIC DIGITS value must be a positive whole number; found "`<value>`" |
| `26.6` | NUMERIC FUZZ value must be zero or a positive whole number; found "`<value>`" |
| `26.7` | Number used in TRACE setting must be a whole number; found "`<value>`" |
| `26.8` | Operand to right of the power operator ("**") must be a whole number; found "`<value>`" |
| `26.11` | Result of `<value>` % `<value>` operation would need exponential notation at current NUMERIC DIGITS `<value>` |
| `26.12` | Result of % operation used for `<value>` // `<value>` operation would need exponential notation at current NUMERIC DIGITS `<value>` |
| `27` | Invalid DO syntax |
| `27.1` | Invalid use of keyword "`<token>`" in DO clause |
| `28` | Invalid LEAVE or ITERATE |
| `28.1` | LEAVE is valid only within a repetitive DO loop |
| `28.2` | ITERATE is valid only within a repetitive DO loop |
| `28.3` | Symbol following LEAVE ("`<token>`") must either match control variable of a current DO loop or be omitted |
| `28.4` | Symbol following ITERATE ("`<token>`") must either match control variable of a current DO loop or be omitted |
| `29` | Environment name too long |
| `29.1` | Environment name exceeds `#Limit_EnvironmentName` characters; found "`<name>`" |
| `30` | Name or string too long |
| `30.1` | Name exceeds `#Limit_Name` characters |
| `30.2` | Literal string exceeds `#Limit_Literal` characters |
| `31` | Name starts with number or "." |
| `31.1` | A value cannot be assigned to a number; found "`<token>`" |
| `31.2` | Variable symbol must not start with a number; found "`<token>`" |
| `31.3` | Variable symbol must not start with a "."; found "`<token>`" |
| `33` | Invalid expression result |
| `33.1` | Value of NUMERIC DIGITS ("`<value>`") must exceed value of NUMERIC FUZZ "(`<value>`)" |
| `33.2` | Value of NUMERIC DIGITS ("`<value>`") must not exceed `#Limit_Digits` |
| `33.6` | Result of expression following NUMERIC FORM must start with "E" or "S"; found "`<value>`" |
| `34` | Logical value not "0" or "1" |
| `34.1` | Value of expression following IF keyword must be exactly "0" or "1"; found "`<value>`" |
| `34.2` | Value of expression following WHEN keyword must be exactly "0" or "1"; found "`<value>`" |
| `34.3` | Value of expression following WHILE keyword must be exactly "0" or "1"; found "`<value>`" |
| `34.4` | Value of expression following UNTIL keyword must be exactly "0" or "1"; found "`<value>`" |
| `34.5` | Value of expression to left of logical operator "`<operator>`" must be exactly "0" or "1"; found "`<value>`" |
| `34.6` | Value of expression to right of logical operator "`<operator>`" must be exactly "0" or "1"; found "`<value>`" |
| `35` | Invalid expression |
| `35.1` | Invalid expression detected at "`<token>`" |
| `36` | Unmatched "(" in expression |
| `37` | Unexpected "," or ")" |
| `37.1` | Unexpected "," |
| `37.2` | Unmatched ")" in expression |
| `38` | Invalid template or pattern |
| `38.1` | Invalid parsing template detected at "`<token>`" |
| `38.2` | Invalid parsing position detected at "`<token>`" |
| `38.3` | PARSE VALUE instruction requires WITH keyword |
| `40` | Incorrect call to routine |
| `40.1` | External routine "`<name>`" failed |
| `40.3` | Not enough arguments in invocation of `<bif>`; minimum expected is `<argnumber>` |
| `40.4` | Too many arguments in invocation of `<bif>`; maximum expected is `<argnumber>` |
| `40.5` | Missing argument in invocation of `<bif>`; argument `<argnumber>` is required |
| `40.9` | `<bif>` argument `<argnumber>` exponent exceeds `#Limit_ExponentDigits` digits; found "`<value>`" |
| `40.11` | `<bif>` argument `<argnumber>` must be a number; found "`<value>`" |
| `40.12` | `<bif>` argument `<argnumber>` must be a whole number; found "`<value>`" |
| `40.13` | `<bif>` argument `<argnumber>` must be zero or positive; found "`<value>`" |
| `40.14` | `<bif>` argument `<argnumber>` must be positive; found "`<value>`" |
| `40.15` | `<bif>` argument `<argnumber>` must fit in `<value>` digits; found "`<value>`" |
| `40.16` | `<bif>` argument 1 requires a whole number fitting within DIGITS(`<value>`); found "`<value>`" |
| `40.17` | `<bif>` argument 1 must have an integer part in the range 0:90 and a decimal part no larger than .9; found "`<value>`" |
| `40.18` | `<bif>` conversion must have a year in the range 0001 to 9999 |
| `40.19` | `<bif>` argument 2, "`<value>`", is not in the format described by argument 3, "`<value>`" |
| `40.21` | `<bif>` argument `<argnumber>` must not be null |
| `40.23` | `<bif>` argument `<argnumber>` must be a single character; found "`<value>`" |
| `40.24` | `<bif>` argument 1 must be a binary string; found "`<value>`" |
| `40.25` | `<bif>` argument 1 must be a hexadecimal string; found "`<value>`" |
| `40.26` | `<bif>` argument 1 must be a valid symbol; found "`<value>`" |
| `40.27` | `<bif>` argument 1 must be a valid stream name; found "`<value>`" |
| `40.28` | `<bif>` argument `<argnumber>`, option must start with one of "`<optionslist>`"; found "`<value>`" |
| `40.29` | `<bif>` conversion to format "`<value>`" is not allowed |
| `40.31` | `<bif>` argument 1 ("`<value>`") must not exceed 100000 |
| `40.32` | `<bif>` the difference between argument 1 ("`<value>`") and argument 2 ("`<value>`") must not exceed 100000 |
| `40.33` | `<bif>` argument 1 ("`<value>`") must be less than or equal to argument 2 ("`<value>`") |
| `40.34` | `<bif>` argument 1 ("`<value>`") must be less than or equal to the number of lines in the program (`<sourceline()>`) |
| `40.35` | `<bif>` argument 1 cannot be expressed as a whole number; found "`<value>`" |
| `40.36` | `<bif>` argument 1 must be the name of a variable in the pool; found "`<value>`" |
| `40.37` | `<bif>` argument 3 must be the name of a pool; found "`<value>`" |
| `40.38` | `<bif>` argument `<argnumber>` is not large enough to format "`<value>`" |
| `40.39` | `<bif>` argument 3 is not zero or one; found "`<value>`" |
| `40.41` | `<bif>` argument `<argnumber>` must be within the bounds of the stream; found "`<value>`" |
| `40.42` | `<bif>` argument 1; cannot position on this stream; found "`<value>`" |
| `41` | Bad arithmetic conversion |
| `41.1` | Non-numeric value ("`<value>`") to left of arithmetic operation "`<operator>`" |
| `41.2` | Non-numeric value ("`<value>`") to right of arithmetic operation "`<operator>`" |
| `41.3` | Non-numeric value ("`<value>`") used with prefix operator "`<operator>`" |
| `41.4` | Value of TO expression in DO instruction must be numeric; found "`<value>`" |
| `41.5` | Value of BY expression in DO instruction must be numeric; found "`<value>`" |
| `41.6` | Value of control variable expression of DO instruction must be numeric; found "`<value>`" |
| `41.7` | Exponent exceeds `#Limit_ExponentDigits` digits; found "`<value>`" |
| `42` | Arithmetic overflow/underflow |
| `42.1` | Arithmetic overflow detected at "`<value>` `<operation>` `<value>`"; exponent of result requires more than `#Limit_ExponentDigits` digits |
| `42.2` | Arithmetic underflow detected at "`<value>` `<operation>` `<value>`"; exponent of result requires more than `#Limit_ExponentDigits` digits |
| `42.3` | Arithmetic overflow; divisor must not be zero |
| `43` | Routine not found |
| `43.1` | Could not find routine "`<name>`" |
| `44` | Function did not return data |
| `44.1` | No data returned from function "`<name>`" |
| `45` | No data specified on function RETURN |
| `45.1` | Data expected on RETURN instruction because routine "`<name>`" was called as a function |
| `46` | Invalid variable reference |
| `46.1` | Extra token ("`<token>`") found in variable reference; ")" expected |
| `47` | Unexpected label |
| `47.1` | INTERPRET data must not contain labels; found "`<name>`" |
| `48` | Failure in system service |
| `48.1` | Failure in system service: `<description>` |
| `49` | Interpretation Error |
| `49.1` | Interpretation Error: `<description>` |
| `50` | Unrecognized reserved symbol |
| `50.1` | Unrecognized reserved symbol "`<token>`" |
| `51` | Invalid function name |
| `51.1` | Unquoted function names must not end with a period; found "`<token>`" |
| `52` | Result returned by "`<name>`" is longer than `#Limit_String` characters |
| `53` | Invalid option |
| `53.1` | Variable reference expected after STREAM keyword; found "`<token>`" |
| `53.2` | Variable reference expected after STEM keyword; found "`<token>`" |
| `53.3` | Argument to STEM must have one period, as its last character; found "`<name>`" |
| `54` | Invalid STEM value |
| `54.1` | For this STEM APPEND, the value of "`<name>`" must be a count of lines; found: "`<value>`" |

## Notes for Compiler Integration

- Standard message numbers are decimal values. The integer part is the error
  code and the fractional part is the subcode.
- Subcodes beginning or ending in zero are not used by the standard.
- Error codes 1 through 90 and subcodes through `.9` are reserved for the
  standard-defined errors.
- `49` is retained for `ERRORTEXT`; a conforming processor should not raise
  `SYNTAX` condition `49`.
- Parser-mode diagnostics should carry both a local compiler diagnostic code
  and, once mapped, a standard Level C error number plus rendered text.
- For recovery, prefer the most specific standard subcode that can be emitted
  without suppressing continued parsing. When uncertain, recover structurally
  first and refine the error-number mapping in the validation walker.
