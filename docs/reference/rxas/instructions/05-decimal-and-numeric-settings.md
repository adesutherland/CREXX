# Decimal And Numeric Settings

These instructions use the current frame's decimal plugin and numeric context
for decimal arithmetic, comparison, conversion, formatting, and settings. A
decimal literal uses RXAS decimal syntax such as `1d` or `1e3`. Unless an entry
states otherwise, plugin conditions are propagated as their corresponding VM
signal after the operation.

## `btod`

Convert an in-place Boolean integer to decimal.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00e2` | `btod rValue` | Set the decimal payload to zero or one. |

### Operands And Semantics

Zero in the integer payload converts to decimal zero; any nonzero value converts
to decimal one under the current context. Only the decimal payload changes.

### Signals

None. Boolean-to-decimal conversion uses the total integer-to-decimal plugin
contract and clears stale plugin diagnostics. Allocation failure follows the
VM panic-on-OOM convention.

### Example

<!-- rxas-example name="decimal-btod" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,1
    btod r0
    ret
```

### Related

`itod`, `ftod`, `dtob`.

## `dadd`

Add two decimal values under the current numeric context.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0168` | `dadd rResult,rLeft,rRight` | Add register decimals. |
| `0x0169` | `dadd rResult,rLeft,decimal` | Add a decimal literal. |

### Operands And Semantics

The destination decimal payload receives the context-rounded sum. Register
sources, scalar payloads, attributes, and remain unchanged; destination
aliasing with a source is supported by the plugin.

### Signals

Propagates decimal-plugin conditions, including invalid operands and context
overflow/underflow.

### Example

<!-- rxas-example name="decimal-dadd" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,1.5d
    dadd r0,r1,2.5d
    ret
```

### Related

`dsub`, `dmult`, `setnumdgts`.

## `dcall`

Call a procedure through an opaque runtime procedure pointer.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x002f` | `dcall rResult,rProcedure,rCount` | Invoke pointer with a contiguous argument block. |

### Operands And Semantics

`rProcedure.int` must hold a `proc_runtime` pointer, commonly produced by
`srcmethodsel` or `srcfprocsel`. `rCount.int` is the argument count and the
arguments occupy consecutive registers immediately after `rCount`. Native calls
copy their result into `rResult`; bytecode calls return into it after a new
frame whose arguments link to the caller registers. The pointer/count/arguments
are otherwise unchanged.

### Signals

Raises `FUNCTION_NOT_FOUND` for an unresolved procedure, `FAILURE` if a bytecode
frame cannot be allocated, and propagates native procedure signals. The opaque
pointer and argument-block bounds are not validated.

### Example

<!-- rxas-example name="decimal-dcall" test="assemble" -->
```rxas
.globals=0

main() .locals=3
    load r2,0
    dcall r0,r1,r2
    ret
```

### Related

`call`, `srcmethodsel`, `srcfprocsel`.

## `dcopy`

Copy only a decimal payload between registers.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x000c` | `dcopy rDestination,rSource` | Duplicate the source decimal storage. |

### Operands And Semantics

The destination decimal buffer is allocated or enlarged as necessary and
receives the exact encoded decimal. An absent or logically empty source clears
the destination's logical decimal length while retaining reusable backing
storage. Other payloads, attributes, type metadata, and flags remain
intact. Self-copy is a no-op; the source is unchanged.

### Signals

This instruction does not signal. Allocation failure remains fatal rather than
being translated into a VM signal.

### Example

<!-- rxas-example name="decimal-dcopy" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,1.25d
    dcopy r0,r1
    ret
```

### Related

`copy`, `dtos`, `load`.

## `ddiv`

Divide decimal values under the current numeric context.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x016f` | `ddiv rResult,rDividend,rDivisor` | Register decimals. |
| `0x0170` | `ddiv rResult,rDividend,divisor` | Literal divisor. |
| `0x0171` | `ddiv rResult,dividend,rDivisor` | Literal dividend. |

### Operands And Semantics

The destination decimal receives the context-rounded quotient. Register sources
and non-decimal destination state remain unchanged.

### Signals

Propagates decimal-plugin conditions, notably division by zero, invalid decimal
data, and context overflow/underflow.

### Example

<!-- rxas-example name="decimal-ddiv" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,9d
    ddiv r0,r1,2d
    ret
```

### Related

`didiv`, `dmod`, `dmult`.

## `dec`

Decrement a selected integer payload in place.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x001d` | `dec rValue` | Subtract one from `rValue.int`. |

### Operands And Semantics

Despite its chapter placement and name, this is fixed-width integer decrement,
not decimal arithmetic. Only the integer payload changes.

### Signals

Raises `OVERFLOW_UNDERFLOW` at minimum `rxinteger` without wrapping or changing
the prior integer payload.

### Example

<!-- rxas-example name="decimal-dec" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,2
    dec r0
    ret
```

### Related

`dec0`, `dec1`, `dec2`, `inc`.

## `dec0`

Decrement `r0`'s integer payload using a compact operand-free opcode.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x001f` | `dec0` | Subtract one from `r0.int`. |

### Operands And Semantics

Only the integer payload of local register zero changes; the frame must provide
that register. This is not a decimal operation.

### Signals

Raises `OVERFLOW_UNDERFLOW` at minimum `rxinteger`; the prior `r0.int` is
preserved on that path.

### Example

<!-- rxas-example name="decimal-dec0" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,1
    dec0
    ret
```

### Related

`dec`, `dec1`, `dec2`.

## `dec1`

Decrement `r1`'s integer payload using a compact operand-free opcode.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0021` | `dec1` | Subtract one from `r1.int`. |

### Operands And Semantics

Only the integer payload of local register one changes; the frame must provide
that register. This is not a decimal operation.

### Signals

Raises `OVERFLOW_UNDERFLOW` at minimum `rxinteger`; the prior `r1.int` is
preserved on that path.

### Example

<!-- rxas-example name="decimal-dec1" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,1
    dec1
    ret
```

### Related

`dec`, `dec0`, `dec2`.

## `dec2`

Decrement `r2`'s integer payload using a compact operand-free opcode.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0023` | `dec2` | Subtract one from `r2.int`. |

### Operands And Semantics

Only the integer payload of local register two changes; the frame must provide
that register. This is not a decimal operation.

### Signals

Raises `OVERFLOW_UNDERFLOW` at minimum `rxinteger`; the prior `r2.int` is
preserved on that path.

### Example

<!-- rxas-example name="decimal-dec2" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r2,1
    dec2
    ret
```

### Related

`dec`, `dec0`, `dec1`.

## `decplnm`

Read the active decimal plugin's identity strings.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0190` | `decplnm rName,rDescription,rVersion` | Store three plugin strings. |

### Operands And Semantics

The three destination string payloads are replaced respectively with the
plugin name, human description, and version. Other
value state remains intact. Destinations should be distinct because writes are
performed in operand order.

### Signals

There is no translated VM signal; allocation failure is fatal.

### Example

<!-- rxas-example name="decimal-decplnm" test="run" -->
```rxas
.globals=0

main() .locals=3
    decplnm r0,r1,r2
    ret
```

### Related

`getnumdgts`, `getnumform`, `getnumstd`.

## `deq`

Compare decimal values for numeric equality.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0178` | `deq rResult,rLeft,rRight` | Compare register decimals. |
| `0x0179` | `deq rResult,rLeft,decimal` | Compare with a decimal literal. |

### Operands And Semantics

The destination is set to integer Boolean `1` when decimal numeric values are
equal, otherwise `0`. Exponent/encoding differences do not defeat numeric
equality. Sources are unchanged.

### Signals

Propagates decimal-plugin invalid-operand conditions.

### Example

<!-- rxas-example name="decimal-deq" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,1d
    deq r0,r1,1.0d
    ret
```

### Related

`dne`, `deqbr`, `dgt`.

## `deqbr`

Branch when two register decimals are numerically equal.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x018a` | `deqbr label,rLeft,rRight` | Branch on decimal equality. |

### Operands And Semantics

True transfers to the procedure-local label; false falls through. Sources and
all value state remains unchanged.

### Signals

Propagates decimal-plugin invalid-operand conditions.

### Example

<!-- rxas-example name="decimal-deqbr" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,1d
    load r1,1.0d
    deqbr equal,r0,r1
    ret
equal:
    ret
```

### Related

`deq`, `dgtbr`, `dltbr`.

## `dextr`

Extract a decimal into a coefficient string and integer exponent.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x018f` | `dextr rCoefficient,rExponent,rDecimal` | Split the decimal representation. |

### Operands And Semantics

The plugin normalizes and rounds to the current digits setting, trims trailing
coefficient zeros, writes the coefficient string (or `nan`, `inf`, `-inf`) to
the first destination, and writes the base-ten exponent integer to the second.
The source decimal is unchanged.

### Signals

The instruction does not perform the normal post-call plugin-signal check;
allocation failure is not translated into a VM signal.

### Example

<!-- rxas-example name="decimal-dextr" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r2,12.5d
    dextr r0,r1,r2
    ret
```

### Related

`dtos`, `dformat`, `getnumdgts`.

## `dgt`

Compare whether one decimal value is greater than another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x017c` | `dgt rResult,rLeft,rRight` | Register decimals. |
| `0x017d` | `dgt rResult,rLeft,decimal` | Register and literal. |
| `0x017e` | `dgt rResult,decimal,rRight` | Literal and register. |

### Operands And Semantics

The destination becomes integer Boolean `1` iff left is numerically greater,
otherwise `0`. Sources are unchanged.

### Signals

Propagates decimal-plugin invalid-operand conditions.

### Example

<!-- rxas-example name="decimal-dgt" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,2d
    dgt r0,3d,r1
    ret
```

### Related

`dgte`, `dlt`, `dgtbr`.

## `dgtbr`

Branch when one register decimal is greater than another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0188` | `dgtbr label,rLeft,rRight` | Branch on decimal greater-than. |

### Operands And Semantics

True transfers to the procedure-local label; false falls through. Sources and
value state is unchanged.

### Signals

Propagates decimal-plugin invalid-operand conditions.

### Example

<!-- rxas-example name="decimal-dgtbr" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,2d
    load r1,1d
    dgtbr greater,r0,r1
    ret
greater:
    ret
```

### Related

`dgt`, `dltbr`, `deqbr`.

## `dgte`

Compare whether one decimal value is greater than or equal to another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x017f` | `dgte rResult,rLeft,rRight` | Register decimals. |
| `0x0180` | `dgte rResult,rLeft,decimal` | Register and literal. |
| `0x0181` | `dgte rResult,decimal,rRight` | Literal and register. |

### Operands And Semantics

The destination becomes canonical integer Boolean greater-or-equal. Numeric
decimal comparison ignores encoding/exponent differences; sources are unchanged.

### Signals

Propagates decimal-plugin invalid-operand conditions.

### Example

<!-- rxas-example name="decimal-dgte" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,2d
    dgte r0,r1,2.0d
    ret
```

### Related

`dgt`, `dlte`, `deq`.

## `didiv`

Divide decimal values and truncate the quotient to an integral decimal.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0172` | `didiv rResult,rDividend,rDivisor` | Register decimals. |
| `0x0173` | `didiv rResult,rDividend,divisor` | Literal divisor. |
| `0x0174` | `didiv rResult,dividend,rDivisor` | Literal dividend. |

### Operands And Semantics

The plugin first performs context decimal division, then truncates the result
toward zero to a decimal with no fractional part. Register sources are unchanged.

### Signals

Propagates plugin division, invalid-operand, overflow, and underflow conditions.

### Example

<!-- rxas-example name="decimal-didiv" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,7d
    didiv r0,r1,2d
    ret
```

### Related

`ddiv`, `dmod`, `dtoi`.

## `dlt`

Compare whether one decimal value is less than another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0182` | `dlt rResult,rLeft,rRight` | Register decimals. |
| `0x0183` | `dlt rResult,rLeft,decimal` | Register and literal. |
| `0x0184` | `dlt rResult,decimal,rRight` | Literal and register. |

### Operands And Semantics

The destination becomes integer Boolean `1` iff left is numerically less,
otherwise `0`. Sources are unchanged.

### Signals

Propagates decimal-plugin invalid-operand conditions.

### Example

<!-- rxas-example name="decimal-dlt" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,3d
    dlt r0,2d,r1
    ret
```

### Related

`dlte`, `dgt`, `dltbr`.

## `dltbr`

Branch when one register decimal is less than another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0189` | `dltbr label,rLeft,rRight` | Branch on decimal less-than. |

### Operands And Semantics

True transfers to the procedure-local label; false falls through. Both decimal
sources remain unchanged.

### Signals

Propagates decimal-plugin invalid-operand conditions.

### Example

<!-- rxas-example name="decimal-dltbr" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,1d
    load r1,2d
    dltbr less,r0,r1
    ret
less:
    ret
```

### Related

`dlt`, `dgtbr`, `deqbr`.

## `dlte`

Compare whether one decimal value is less than or equal to another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0185` | `dlte rResult,rLeft,rRight` | Register decimals. |
| `0x0186` | `dlte rResult,rLeft,decimal` | Register and literal. |
| `0x0187` | `dlte rResult,decimal,rRight` | Literal and register. |

### Operands And Semantics

The destination becomes canonical integer Boolean less-or-equal. Decimal
sources, and other destination state remain unchanged.

### Signals

Propagates decimal-plugin invalid-operand conditions.

### Example

<!-- rxas-example name="decimal-dlte" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,2d
    dlte r0,r1,2d
    ret
```

### Related

`dlt`, `dgte`, `deq`.

## `dmod`

Compute the decimal remainder paired with truncating decimal division.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0175` | `dmod rResult,rDividend,rDivisor` | Register decimals. |
| `0x0176` | `dmod rResult,rDividend,divisor` | Literal divisor. |
| `0x0177` | `dmod rResult,dividend,rDivisor` | Literal dividend. |

### Operands And Semantics

The VM computes `dividend - truncate(dividend/divisor) * divisor` using plugin
decimal operations and the current context. Register sources remain unchanged;
aliasing the destination with either source is supported.

### Signals

Propagates plugin conditions from division, truncation, multiplication, or
subtraction, including division by zero.

### Example

<!-- rxas-example name="decimal-dmod" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,7d
    dmod r0,r1,3d
    ret
```

### Related

`didiv`, `ddiv`, `dmult`.

## `dmult`

Multiply decimal values under the current numeric context.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x016d` | `dmult rResult,rLeft,rRight` | Multiply register decimals. |
| `0x016e` | `dmult rResult,rLeft,decimal` | Multiply by a literal. |

### Operands And Semantics

The destination decimal receives the context-rounded product. Sources and
non-decimal value state remain unchanged; destination aliasing is supported.

### Signals

Propagates decimal-plugin invalid, overflow, and underflow conditions.

### Example

<!-- rxas-example name="decimal-dmult" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,2.5d
    dmult r0,r1,4d
    ret
```

### Related

`dadd`, `ddiv`, `dpow`.

## `dne`

Compare decimal values for numeric inequality.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x017a` | `dne rResult,rLeft,rRight` | Compare register decimals. |
| `0x017b` | `dne rResult,rLeft,decimal` | Compare with a literal. |

### Operands And Semantics

The destination becomes integer Boolean `1` when numeric decimal values differ
and `0` when equal. Sources, decimal encodings, and remain unchanged.

### Signals

Propagates decimal-plugin invalid-operand conditions.

### Example

<!-- rxas-example name="decimal-dne" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,1d
    dne r0,r1,2d
    ret
```

### Related

`deq`, `dgt`, `dlt`.

## `dpow`

Raise a decimal base to a decimal exponent under the current context.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x018b` | `dpow rResult,rBase,rExponent` | Register operands. |
| `0x018c` | `dpow rResult,rBase,exponent` | Literal exponent. |
| `0x018d` | `dpow rResult,base,rExponent` | Literal base. |

### Operands And Semantics

The decimal plugin computes and context-rounds the power into the destination
decimal payload. Register sources and unrelated value state are unchanged.

### Signals

Propagates plugin invalid-operation, overflow, underflow, and allocation
conditions, including unsupported base/exponent combinations.

### Example

<!-- rxas-example name="decimal-dpow" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,2d
    dpow r0,r1,3d
    ret
```

### Related

`dmult`, `ipow`, `fpow`.

## `dropchar`

Append source characters that do not occur in a removal-list string.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00a0` | `dropchar rDestination,rSource,rRemovalList` | Filter by Unicode code point. |

### Operands And Semantics

Each source character is compared with every removal-list character and is
appended to the destination only when absent. The destination is not cleared,
so pre-existing text is retained.
During scanning the VM overwrites `rSource.int` and `rRemovalList.int` with the
last examined code points, while leaving their strings unchanged.

### Signals

Raises `UNICODE_ERROR` if either source string is invalid UTF-8. String-growth
allocation failure is fatal.

### Example

<!-- rxas-example name="decimal-dropchar" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r0,""
    load r1,"banana"
    load r2,"an"
    dropchar r0,r1,r2
    ret
```

### Related

`transchar`, `strlower`, `strupper`.

## `dsex`

Negate a decimal payload in place.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x018e` | `dsex rValue` | Replace decimal value with its sign inverse. |

### Operands And Semantics

Only the decimal payload changes; other payloads, attributes, type
metadata, and flags remain intact.

### Signals

Raises `INVALID_ARGUMENTS` when the register has no decimal payload. The
instruction does not perform the usual post-plugin signal check after negation.

### Example

<!-- rxas-example name="decimal-dsex" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,1.5d
    dsex r0
    ret
```

### Related

`dsub`, `isex`, `fsex`.

## `dsub`

Subtract decimal values under the current numeric context.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x016a` | `dsub rResult,rLeft,rRight` | Register decimals. |
| `0x016b` | `dsub rResult,rLeft,decimal` | Literal subtrahend. |
| `0x016c` | `dsub rResult,decimal,rRight` | Literal minuend. |

### Operands And Semantics

The destination decimal receives the context-rounded difference. Register
sources and unrelated destination state remain unchanged; aliasing is supported.

### Signals

Propagates decimal-plugin invalid, overflow, underflow, and allocation
conditions.

### Example

<!-- rxas-example name="decimal-dsub" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,2d
    dsub r0,5d,r1
    ret
```

### Related

`dadd`, `dsex`, `dmult`.

## `dtob`

Convert a decimal payload to Boolean integer in place.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00f1` | `dtob rValue` | Set integer to `0` for decimal zero, else `1`. |

### Operands And Semantics

Only the integer payload changes; the decimal value remains available. The
plugin's decimal zero test determines the canonical Boolean.

### Signals

Propagates decimal-plugin invalid-operand conditions.

### Example

<!-- rxas-example name="decimal-dtob" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,0.1d
    dtob r0
    ret
```

### Related

`btod`, `dtoi`, `dtof`.

## `dtof`

Convert a decimal payload to host floating point in place.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00f4` | `dtof rValue` | Write the nearest supported `double` payload. |

### Operands And Semantics

Only the float payload changes; the decimal payload and other value state are
preserved. Conversion can round when the decimal is not exactly representable.

### Signals

Propagates plugin invalid, overflow, and underflow conversion conditions.

### Example

<!-- rxas-example name="decimal-dtof" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,1.25d
    dtof r0
    ret
```

### Related

`ftod`, `dtoi`, `dtos`.

## `dtoi`

Convert a decimal payload to fixed-width integer in place.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00f0` | `dtoi rValue` | Write the converted integer payload. |

### Operands And Semantics

The plugin converts under its integer-conversion rules and writes only the
integer payload; the original decimal remains intact.

### Signals

Propagates invalid, nonintegral, and out-of-range plugin conversion conditions.

### Example

<!-- rxas-example name="decimal-dtoi" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,42d
    dtoi r0
    ret
```

### Related

`itod`, `dtob`, `didiv`.

## `dtos`

Format a decimal payload as a string in place.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00ef` | `dtos rValue` | Replace the string payload with plugin formatting. |

### Operands And Semantics

The current digits, form, case, fuzz, and standard settings govern formatting.
The string buffer is replaced with the NUL-terminated result; the decimal
payload remains unchanged. Decimal absence formats as `nan`. The completed
ASCII write refreshes string validity metadata.

### Signals

This instruction does not signal. Decimal-plugin formatting diagnostics are
cleared at this total conversion boundary; buffer allocation failure remains
fatal rather than being translated into a VM signal.

### Example

<!-- rxas-example name="decimal-dtos" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,12.5d
    dtos r0
    ret
```

### Related

`stod`, `dextr`, `dtof`.

## `ftod`

Convert a host floating-point payload to decimal in place.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00f3` | `ftod rValue` | Replace the decimal payload from `rValue.float`. |

### Operands And Semantics

The current decimal context governs conversion and rounding. Only the decimal
payload changes; the source float remains stored in the same register.

### Signals

Propagates plugin invalid, overflow, underflow, and allocation conditions.

### Example

<!-- rxas-example name="decimal-ftod" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,1.25
    ftod r0
    ret
```

### Related

`dtof`, `itod`, `stod`.

## `getnumcas`

Read the numeric exponent-letter case setting.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0100` | `getnumcas rCase` | Store `1` for lower or `2` for upper. |

### Operands And Semantics

Only the destination integer payload changes; the numeric context is unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="decimal-getnumcas" test="run" -->
```rxas
.globals=0

main() .locals=1
    getnumcas r0
    ret
```

### Related

`setnumcas`, `numeng`, `numsci`.

## `getnumdgts`

Read the current decimal precision in digits.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00f7` | `getnumdgts rDigits` | Store the context digits value. |

### Operands And Semantics

Only the destination integer payload changes; no context is mutated.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="decimal-getnumdgts" test="run" -->
```rxas
.globals=0

main() .locals=1
    getnumdgts r0
    ret
```

### Related

`setnumdgts`, `getnumfuz`.

## `getnumfrm`

Read the numeric notation form.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00fd` | `getnumfrm rForm` | Store `1` scientific or `2` engineering. |

### Operands And Semantics

Only the destination integer payload changes.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="decimal-getnumfrm" test="run" -->
```rxas
.globals=0

main() .locals=1
    getnumfrm r0
    ret
```

### Related

`setnumfrm`, `numeng`, `numsci`.

## `getnumfuz`

Read the current numeric fuzz value.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00fa` | `getnumfuz rFuzz` | Store the context fuzz digits. |

### Operands And Semantics

Only the destination integer payload changes.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="decimal-getnumfuz" test="run" -->
```rxas
.globals=0

main() .locals=1
    getnumfuz r0
    ret
```

### Related

`setnumfuz`, `getnumdgts`.

## `getnumstd`

Read the numeric standard selector.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0103` | `getnumstd rStandard` | Store `1` common or `2` classic. |

### Operands And Semantics

Only the destination integer payload changes.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="decimal-getnumstd" test="run" -->
```rxas
.globals=0

main() .locals=1
    getnumstd r0
    ret
```

### Related

`setnumstd`, `numeng`, `numsci`.

## `itod`

Convert an integer payload to decimal in place.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00f2` | `itod rValue` | Replace the decimal payload from `rValue.int`. |

### Operands And Semantics

The current decimal context governs conversion. Only the decimal payload
changes; the original integer and other value state remain intact.

### Signals

None. Integer-to-decimal conversion is total for every `rxinteger` and clears
stale plugin diagnostics. Allocation failure follows the VM panic-on-OOM
convention rather than becoming a language signal.

### Example

<!-- rxas-example name="decimal-itod" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,42
    itod r0
    ret
```

### Related

`dtoi`, `btod`, `ftod`.

## `numeng`

Install a complete engineering-notation numeric context.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0105` | `numeng digits,case,standard` | Set engineering form and reset fuzz. |

### Operands And Semantics

`digits` must be at least 5; `case` is `1` lower or `2` upper; `standard` is
`1` common or `2` classic. On success the frame receives those settings,
engineering form `2`, and fuzz zero, then synchronizes the decimal plugin.

### Signals

Raises `INVALID_ARGUMENTS` before mutation when any literal is outside its
accepted range.

### Example

<!-- rxas-example name="decimal-numeng" test="run" -->
```rxas
.globals=0

main() .locals=0
    numeng 9,1,1
    ret
```

### Related

`numsci`, `setnumfrm`, `setnumdgts`.

## `numsci`

Install a complete scientific-notation numeric context.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0104` | `numsci digits,case,standard` | Set scientific form and reset fuzz. |

### Operands And Semantics

`digits` must be at least 5; `case` is `1` lower or `2` upper; `standard` is
`1` common or `2` classic. Success installs those settings, scientific form
`1`, and fuzz zero, then synchronizes the decimal plugin.

### Signals

Raises `INVALID_ARGUMENTS` before context mutation for an invalid literal.

### Example

<!-- rxas-example name="decimal-numsci" test="run" -->
```rxas
.globals=0

main() .locals=0
    numsci 9,2,1
    ret
```

### Related

`numeng`, `setnumfrm`, `setnumdgts`.

## `setnumcas`

Set the numeric exponent-letter case and synchronize the decimal plugin.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00fe` | `setnumcas rCase` | Read case from an integer payload. |
| `0x00ff` | `setnumcas case` | Use a literal case selector. |

### Operands And Semantics

Accepted values are `1` for lowercase and `2` for uppercase exponent letters.
Success changes the current frame context and immediately synchronizes the
decimal plugin. A register operand remains unchanged.

### Signals

Raises `INVALID_ARGUMENTS` before mutation for values outside `1..2`.

### Example

<!-- rxas-example name="decimal-setnumcas" test="run" -->
```rxas
.globals=0

main() .locals=0
    setnumcas 2
    ret
```

### Related

`getnumcas`, `numeng`, `numsci`.

## `setnumdgts`

Set the decimal precision and synchronize the decimal plugin.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00f5` | `setnumdgts rDigits` | Read precision from an integer payload. |
| `0x00f6` | `setnumdgts digits` | Use a literal precision. |

### Operands And Semantics

The implementation accepts every positive value (`>=1`), despite the older
assembler description saying greater than four. Success updates the frame's
digits setting and synchronizes the decimal plugin; a source register is
unchanged.

### Signals

Raises `INVALID_ARGUMENTS` before mutation for zero or negative precision.

### Example

<!-- rxas-example name="decimal-setnumdgts" test="run" -->
```rxas
.globals=0

main() .locals=0
    setnumdgts 9
    ret
```

### Related

`getnumdgts`, `numeng`, `numsci`.

## `setnumfrm`

Set scientific or engineering numeric form.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00fb` | `setnumfrm rForm` | Read form from an integer payload. |
| `0x00fc` | `setnumfrm form` | Use a literal form selector. |

### Operands And Semantics

`1` selects scientific notation and `2` engineering notation. Success updates
the current frame and synchronizes the decimal plugin; register operands remain
unchanged.

### Signals

Raises `INVALID_ARGUMENTS` before mutation outside `1..2`.

### Example

<!-- rxas-example name="decimal-setnumfrm" test="run" -->
```rxas
.globals=0

main() .locals=0
    setnumfrm 2
    ret
```

### Related

`getnumfrm`, `numeng`, `numsci`.

## `setnumfuz`

Set numeric fuzz and synchronize the decimal plugin.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00f8` | `setnumfuz rFuzz` | Read fuzz from an integer payload. |
| `0x00f9` | `setnumfuz fuzz` | Use a literal fuzz value. |

### Operands And Semantics

Nonnegative values are the supported domain. Success updates the current frame
and synchronizes the plugin; a register operand is unchanged. Both forms
validate before changing or synchronizing the current numeric context.

### Signals

Raises `INVALID_ARGUMENTS` for a negative value. The prior fuzz setting is
preserved on that path.

### Example

<!-- rxas-example name="decimal-setnumfuz" test="run" -->
```rxas
.globals=0

main() .locals=0
    setnumfuz 0
    ret
```

### Related

`getnumfuz`, `setnumdgts`, `numeng`.

## `setnumstd`

Set the decimal numeric standard selector.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0101` | `setnumstd rStandard` | Read selector from an integer payload. |
| `0x0102` | `setnumstd standard` | Use a literal selector. |

### Operands And Semantics

`1` selects common and `2` classic behavior. Success updates the frame context
and synchronizes the plugin; a register operand is unchanged.

### Signals

Raises `INVALID_ARGUMENTS` before mutation outside `1..2`.

### Example

<!-- rxas-example name="decimal-setnumstd" test="run" -->
```rxas
.globals=0

main() .locals=0
    setnumstd 1
    ret
```

### Related

`getnumstd`, `numeng`, `numsci`.

## `stod`

Parse an in-place string payload as a decimal value.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00ee` | `stod rValue` | Replace the decimal payload from the string. |

### Operands And Semantics

The string is NUL-terminated in its existing buffer if necessary, then parsed
under the current decimal context. Only the decimal payload changes; string
content otherwise remain unchanged.

### Signals

Propagates decimal-plugin syntax, range, and allocation conditions.

### Example

<!-- rxas-example name="decimal-stod" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,"12.5"
    stod r0
    ret
```

### Related

`dtos`, `itod`, `ftod`.
