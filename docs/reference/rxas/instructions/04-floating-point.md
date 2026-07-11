# Floating Point

Floating-point instructions operate on the binary64 payload of a VM register.
RXAS registers are not statically typed: a register operand means that the
instruction reads or writes the float, integer, or string payload named below.
Writing one payload does not copy the source register's other payloads, status
flags, or string/binary cursor.

Float literals must use a decimal point or exponent, such as `2.0` or `2e0`.
The arithmetic instructions use the host C binary64 operations and `libm`.
Consequently, infinity, NaN, signed zero, and rounding follow the platform's
binary64 behavior. These instructions do not translate floating-point status or
`errno` into RXAS signals: division by zero, invalid `fmod`/`pow` domains, and
overflowing finite results produce the corresponding binary64 result.

## `fadd`

Add two binary64 values. This is the normal float addition primitive.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x012c` | `fadd rDst,rLeft,rRight` | Set the float payload of `rDst` to `rLeft + rRight`. |
| `0x012d` | `fadd rDst,rLeft,float` | Add a float literal or float constant to `rLeft`. |

### Operands And Semantics

Register sources are read through their float payloads. Only the destination's
float payload is replaced; sources and all cursors are unchanged. Destination
aliasing with either source is allowed because the operands are read before the
result is stored.

### Signals

No signal is raised for binary64 overflow, underflow, NaN, or infinity.

### Example

<!-- rxas-example name="float-fadd" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,1.25
    fadd r0,r1,2.5
    ret
```

### Related

`fsub`, `fmult`, `fdiv`.

## `fcopy`

Copy one register's binary64 payload without copying its other value payloads
or status flags.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x000a` | `fcopy rDst,rSrc` | Copy the float payload of `rSrc` to `rDst`. |

### Operands And Semantics

Both operands are registers. The source is unchanged. The destination's string,
integer, decimal, attributes, status flags, and cursors are not copied.

### Signals

This instruction does not raise a signal.

### Example

<!-- rxas-example name="float-fcopy" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,3.5
    fcopy r0,r1
    ret
```

### Related

`copy`, `icopy`, `dcopy`, `scopy`.

## `fdiv`

Divide one binary64 value by another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0133` | `fdiv rDst,rDividend,rDivisor` | Divide two register float payloads. |
| `0x0134` | `fdiv rDst,rDividend,float` | Divide a register float payload by a float literal. |
| `0x0135` | `fdiv rDst,float,rDivisor` | Divide a float literal by a register float payload. |

### Operands And Semantics

`rDst` receives a float payload. Sources and cursors are unchanged, and source
registers may alias the destination.

### Signals

There is no RXAS divide-by-zero signal for `fdiv`. Binary64 division determines
the infinity or NaN result; overflow and underflow likewise do not raise a VM
signal.

### Example

<!-- rxas-example name="float-fdiv" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,7.5
    fdiv r0,r1,2.5
    ret
```

### Related

`fidiv`, `fmod`, `ddiv`, `idiv`.

## `feq`

Compare two binary64 values for exact IEEE equality and write an integer
Boolean result.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x013c` | `feq rResult,rLeft,rRight` | Set `rResult` to `1` when the register float payloads compare equal. |
| `0x013d` | `feq rResult,rLeft,float` | Compare a register float payload with a float literal. |

### Operands And Semantics

`rResult` receives integer `0` or `1`; its float payload is not the destination.
The sources are unchanged. Positive and negative zero compare equal. A NaN is
not equal to any value, including itself.

### Signals

This comparison does not raise a signal for NaN or infinity.

### Example

<!-- rxas-example name="float-feq" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,4.0
    feq r0,r1,4.0
    ret
```

### Related

`fne`, `fgt`, `fgte`, `flt`, `flte`.

## `fextr`

Decompose a binary64 value into a normalized decimal coefficient string and a
base-10 integer exponent. It is the preferred primitive for caller-controlled
float formatting.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0152` | `fextr rCoefficient,rExponent,rFloat` | Extract `rFloat = coefficient * 10**exponent`. |

### Operands And Semantics

`rCoefficient` receives a string such as `"1.25"`; `rExponent` receives an
integer. The coefficient uses the current numeric case setting for `nan` and
`inf`. Precision is the current numeric digits value clamped to the VM minimum
and `DBL_DIG`; trailing fractional zeroes are removed. Finite nonzero output is
normalized to one digit before the decimal point. Zero produces coefficient
`"0"` and exponent `0`; infinities and NaNs produce `inf`, `-inf`, or `nan`
and exponent `0`. The source float payload is unchanged. The coefficient's
string cursor is reset to zero.

### Signals

This instruction does not signal for non-finite values.

### Example

<!-- rxas-example name="float-fextr" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r2,125.0
    fextr r0,r1,r2
    ret
```

### Related

`ftos`, `fformat`, `getnumdgts`, `getnumcas`.

## `fformat`

Format a binary64 value with a restricted printf-style format. This instruction
is deprecated; use `fextr` for new code.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0106` | `fformat rString,rFloat,rFormat` | Format `rFloat` according to the string payload of `rFormat`. |

### Operands And Semantics

The checked parser accepts one conversion from `a`, `A`, `e`, `E`, `f`, `F`,
`g`, or `G`, with optional decimal width and precision, plus literal text and
`%%`. Unsupported or additional conversion text is copied literally. The
destination string is replaced and its byte and character cursors are reset to
zero. The format register may be NUL-terminated internally as part of the
operation; callers should therefore treat its private string buffer state as
mutable even though its logical text is unchanged.

### Signals

Malformed or unsupported format text does not raise a signal; it is retained as
literal text from the first unaccepted conversion onward.

### Example

<!-- rxas-example name="float-fformat" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,12.5
    load r2,"%.1f"
    fformat r0,r1,r2
    ret
```

### Related

`fextr`, `ftos`.

## `fgt`

Compare two binary64 values and return whether the left value is greater.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0140` | `fgt rResult,rLeft,rRight` | Compare two register float payloads. |
| `0x0141` | `fgt rResult,rLeft,float` | Compare a register float payload with a literal. |
| `0x0142` | `fgt rResult,float,rRight` | Compare a literal with a register float payload. |

### Operands And Semantics

`rResult` receives integer `0` or `1`. Sources and cursors are unchanged. Any
ordered comparison involving NaN is false.

### Signals

No signal is raised for NaN or infinity.

### Example

<!-- rxas-example name="float-fgt" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,3.0
    fgt r0,r1,2.0
    ret
```

### Related

`fgtbr`, `fgte`, `flt`.

## `fgtbr`

Branch when one register's binary64 value is greater than another's.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x014c` | `fgtbr label,rLeft,rRight` | Branch to `label` when `rLeft > rRight`; otherwise continue. |

### Operands And Semantics

`label` is a procedure-local RXAS label. Both value operands are registers read
through their float payloads. No register or cursor is changed. A NaN makes the
condition false.

### Signals

The comparison does not raise a signal.

### Example

<!-- rxas-example name="float-fgtbr" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,2.0
    load r1,1.0
    fgtbr greater,r0,r1
    ret
greater:
    ret
```

### Related

`fgt`, `fltbr`, `brt`.

## `fgte`

Compare two binary64 values and return whether the left value is greater than
or equal to the right value.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0143` | `fgte rResult,rLeft,rRight` | Compare two register float payloads. |
| `0x0144` | `fgte rResult,rLeft,float` | Compare a register float payload with a literal. |
| `0x0145` | `fgte rResult,float,rRight` | Compare a literal with a register float payload. |

### Operands And Semantics

`rResult` receives integer `0` or `1`. Sources and cursors are unchanged.
Positive and negative zero compare equal; any ordered comparison involving NaN
is false.

### Signals

The comparison does not raise a signal.

### Example

<!-- rxas-example name="float-fgte" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,2.0
    fgte r0,r1,2.0
    ret
```

### Related

`fgt`, `flte`, `feq`.

## `fidiv`

Divide two binary64 values and truncate the quotient toward zero, retaining the
result as a float payload.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0136` | `fidiv rDst,rDividend,rDivisor` | Truncate a register/register quotient. |
| `0x0137` | `fidiv rDst,rDividend,float` | Truncate a register/literal quotient. |
| `0x0138` | `fidiv rDst,float,rDivisor` | Truncate a literal/register quotient. |

### Operands And Semantics

The VM computes binary64 division, applies C `trunc`, and writes that binary64
value to `rDst`. Despite the mnemonic, it does not write an integer payload.
Sources and cursors are unchanged.

### Signals

Division by zero and non-finite results do not raise a VM signal.

### Example

<!-- rxas-example name="float-fidiv" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,7.5
    fidiv r0,r1,2.0
    ret
```

### Related

`fdiv`, `fmod`, `ftoi`.

## `flt`

Compare two binary64 values and return whether the left value is less.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0146` | `flt rResult,rLeft,rRight` | Compare two register float payloads. |
| `0x0147` | `flt rResult,rLeft,float` | Compare a register float payload with a literal. |
| `0x0148` | `flt rResult,float,rRight` | Compare a literal with a register float payload. |

### Operands And Semantics

`rResult` receives integer `0` or `1`. Sources and cursors are unchanged. Any
ordered comparison involving NaN is false.

### Signals

The comparison does not raise a signal.

### Example

<!-- rxas-example name="float-flt" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,1.0
    flt r0,r1,2.0
    ret
```

### Related

`fltbr`, `flte`, `fgt`.

## `fltbr`

Branch when one register's binary64 value is less than another's.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x014d` | `fltbr label,rLeft,rRight` | Branch to `label` when `rLeft < rRight`; otherwise continue. |

### Operands And Semantics

`label` is a procedure-local RXAS label. Both value operands are registers read
through their float payloads. No register or cursor is changed. A NaN makes the
condition false.

### Signals

The comparison does not raise a signal.

### Example

<!-- rxas-example name="float-fltbr" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,1.0
    load r1,2.0
    fltbr smaller,r0,r1
    ret
smaller:
    ret
```

### Related

`flt`, `fgtbr`, `brt`.

## `flte`

Compare two binary64 values and return whether the left value is less than or
equal to the right value.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0149` | `flte rResult,rLeft,rRight` | Compare two register float payloads. |
| `0x014a` | `flte rResult,rLeft,float` | Compare a register float payload with a literal. |
| `0x014b` | `flte rResult,float,rRight` | Compare a literal with a register float payload. |

### Operands And Semantics

`rResult` receives integer `0` or `1`. Sources and cursors are unchanged.
Positive and negative zero compare equal; any ordered comparison involving NaN
is false.

### Signals

The comparison does not raise a signal.

### Example

<!-- rxas-example name="float-flte" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,2.0
    flte r0,r1,2.0
    ret
```

### Related

`flt`, `fgte`, `feq`.

## `fmod`

Compute the C `fmod` remainder of two binary64 values. The result has the sign
of the dividend and is not the same as a floor-modulus operation for negative
operands.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0139` | `fmod rDst,rDividend,rDivisor` | Remainder of two register float payloads. |
| `0x013a` | `fmod rDst,rDividend,float` | Remainder with a literal divisor. |
| `0x013b` | `fmod rDst,float,rDivisor` | Remainder with a literal dividend. |

### Operands And Semantics

`rDst` receives a float payload. Sources and cursors are unchanged. Destination
aliasing is allowed.

### Signals

A zero divisor or other invalid binary64 input produces the platform `fmod`
NaN result; it does not raise a VM signal.

### Example

<!-- rxas-example name="float-fmod" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,7.5
    fmod r0,r1,2.0
    ret
```

### Related

`fdiv`, `fidiv`, `imod`, `dmod`.

## `fmult`

Multiply two binary64 values.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0131` | `fmult rDst,rLeft,rRight` | Multiply two register float payloads. |
| `0x0132` | `fmult rDst,rLeft,float` | Multiply a register float payload by a literal. |

### Operands And Semantics

Only `rDst`'s float payload is replaced. Sources and cursors are unchanged, and
the destination may alias a source.

### Signals

No signal is raised for binary64 overflow, underflow, NaN, or infinity.

### Example

<!-- rxas-example name="float-fmult" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,2.5
    fmult r0,r1,4.0
    ret
```

### Related

`fadd`, `fdiv`, `fpow`.

## `fndblnk`

Find a Unicode whitespace character in a string. The historical `fnd` prefix
means "find"; this instruction operates on string and integer payloads, not
float payloads.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00a5` | `fndblnk rIndex,rString,rStart` | Find the next blank at or after the requested position. |

### Operands And Semantics

`rString` supplies a string and `rStart` an integer character index. A
nonnegative start searches forward. A negative start requests a reverse search
from the absolute index, clamped to the last character. `rIndex` receives the
zero-based character index. A failed forward search returns `-length`; a failed
reverse search returns a negative value. In UTF builds, a non-ASCII scan updates
the source register's internal string byte/character cursor while examining
characters; do not rely on its previous cursor afterward. The logical string
and start register are unchanged.

### Signals

Invalid UTF-8 in `rString` raises `UNICODE_ERROR`. Out-of-range start positions
are handled by the search rules rather than raising `OUT_OF_RANGE`.

### Example

<!-- rxas-example name="float-fndblnk" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"one two"
    load r2,0
    fndblnk r0,r1,r2
    ret
```

### Related

`fndnblnk`, `getstrpos`, `setstrpos`.

## `fndnblnk`

Find a non-whitespace Unicode character in a string. Despite its placement in
the historical floating-point opcode range, it operates on string and integer
payloads.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00a6` | `fndnblnk rIndex,rString,rStart` | Find the next nonblank at or after the requested position. |

### Operands And Semantics

`rString` supplies a string and `rStart` an integer character index. A
nonnegative start searches forward. A negative start searches backward from the
absolute index, clamped to the last character. `rIndex` receives the zero-based
character index. A failed forward search returns `-length`; a failed reverse
search returns `-1`. In UTF builds, a non-ASCII scan updates the source
register's internal string byte/character cursor while examining characters;
the logical string and start register are unchanged.

### Signals

Invalid UTF-8 in `rString` raises `UNICODE_ERROR`. Out-of-range start positions
are handled by the search rules rather than raising `OUT_OF_RANGE`.

### Example

<!-- rxas-example name="float-fndnblnk" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"  word"
    load r2,0
    fndnblnk r0,r1,r2
    ret
```

### Related

`fndblnk`, `getstrpos`, `setstrpos`.

## `fne`

Compare two binary64 values for IEEE inequality and write an integer Boolean
result.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x013e` | `fne rResult,rLeft,rRight` | Compare two register float payloads. |
| `0x013f` | `fne rResult,rLeft,float` | Compare a register float payload with a literal. |

### Operands And Semantics

`rResult` receives integer `0` or `1`; sources and cursors are unchanged. A NaN
compares unequal to every value, including itself.

### Signals

The comparison does not raise a signal.

### Example

<!-- rxas-example name="float-fne" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,3.0
    fne r0,r1,4.0
    ret
```

### Related

`feq`, `fgt`, `flt`.

## `fpow`

Raise a binary64 base to a binary64 exponent using the platform C `pow`
function.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x014e` | `fpow rDst,rBase,rExponent` | Power with two register float payloads. |
| `0x014f` | `fpow rDst,rBase,float` | Power with a literal exponent. |
| `0x0150` | `fpow rDst,float,rExponent` | Power with a literal base. |

### Operands And Semantics

`rDst` receives the `pow` binary64 result. Sources and cursors are unchanged,
and destination aliasing is allowed.

### Signals

Domain, pole, overflow, and underflow results are not translated into VM
signals; inspect the resulting NaN, infinity, zero, or finite value.

### Example

<!-- rxas-example name="float-fpow" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,2.0
    fpow r0,r1,3.0
    ret
```

### Related

`fmult`, `ipow`, `dpow`.

## `fsex`

Change the sign of a register's binary64 payload in place.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0151` | `fsex rValue` | Set the float payload to `0 - rValue`. |

### Operands And Semantics

The register is both source and destination. Other payloads, status flags, and
cursors are unchanged. Because the implementation subtracts from positive
zero, applying it to positive zero produces positive zero rather than a
bit-level sign toggle; NaN sign bits are likewise not a portable contract.

### Signals

This instruction does not raise a signal.

### Example

<!-- rxas-example name="float-fsex" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,2.5
    fsex r0
    ret
```

### Related

`fsub`, `isex`, `dsex`.

## `fsub`

Subtract one binary64 value from another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x012e` | `fsub rDst,rLeft,rRight` | Subtract two register float payloads. |
| `0x012f` | `fsub rDst,rLeft,float` | Subtract a literal from a register float payload. |
| `0x0130` | `fsub rDst,float,rRight` | Subtract a register float payload from a literal. |

### Operands And Semantics

Only `rDst`'s float payload is replaced. Sources and cursors are unchanged, and
the destination may alias a source.

### Signals

No signal is raised for binary64 overflow, underflow, NaN, or infinity.

### Example

<!-- rxas-example name="float-fsub" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,5.0
    fsub r0,r1,1.5
    ret
```

### Related

`fadd`, `fmult`, `fdiv`.

## `ftob`

Convert a register's binary64 payload to an integer Boolean payload in place.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00e9` | `ftob rValue` | Store integer `0` for float zero, otherwise integer `1`. |

### Operands And Semantics

The float payload is read and the integer payload of the same register is
written. Both positive and negative zero become `0`; finite nonzero values
become `1`. Other payloads, status flags, and cursors are unchanged.

### Signals

No signal is raised. The implementation does not explicitly guard NaN or
infinite inputs before its internal float-to-integer step, so portable RXAS
should apply `ftob` only to finite values.

### Example

<!-- rxas-example name="float-ftob" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,-0.25
    ftob r0
    ret
```

### Related

`ftoi`, `btof`, `itob`, `stob`.

## `ftoi`

Convert a register's binary64 payload to an integer payload using the VM's
nearest-integer conversion and require an exact round trip.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00e8` | `ftoi rValue` | Write the converted integer payload in the same register. |

### Operands And Semantics

The conversion starts with `floor(value)` and increments when the remaining
fraction is greater than `0.5`; exact half values therefore stay at the lower
integer. The integer payload is written before exactness is checked. The float
payload, other payloads, status flags, and cursors are unchanged.

### Signals

Raises `CONVERSION_ERROR` unless converting the result back to binary64 exactly
equals the original float. Fractional inputs therefore signal even though the
rounded integer payload has been written. The implementation has no explicit
pre-conversion bounds or non-finite check; portable code must restrict input to
finite values representable by the VM integer type.

### Example

<!-- rxas-example name="float-ftoi" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,42.0
    ftoi r0
    ret
```

### Related

`ftob`, `itof`, `stoi`, `fidiv`.

## `ftos`

Format a register's binary64 payload as a Rexx numeric string using the current
numeric context.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00e6` | `ftos rValue` | Replace the register's string payload with the formatted float value. |

### Operands And Semantics

The float payload remains available and the string payload of the same register
is replaced. Formatting honors numeric digits, form, and case after the
extractor clamps binary64 precision to its supported range. The string cursor
is reset to zero. NaN and infinity are rendered using the current numeric case.

### Signals

This conversion does not signal for NaN or infinity.

### Example

<!-- rxas-example name="float-ftos" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,12.5
    ftos r0
    ret
```

### Related

`fextr`, `fformat`, `stof`, `dtos`.
