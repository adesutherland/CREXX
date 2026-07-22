# Integer, Logical, And Boolean

These instructions provide checked fixed-width integer arithmetic, comparisons,
logical and bitwise operations, Boolean/scalar conversion, and inclusive range
checks. Unless stated otherwise, a destination receives only the indicated
scalar payload; unrelated payloads, attributes, type metadata, flags, and
source cursors remain unchanged.

## `and`

Compute logical conjunction of two integer truth values.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00b4` | `and rResult,rLeft,rRight` | Store `1` iff both operands are nonzero. |

### Operands And Semantics

Both source integer payloads are read. The destination integer becomes the
canonical Boolean `0` or `1`; sources and all cursors are unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-and" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,1
    load r2,2
    and r0,r1,r2
    ret
```

### Related

`or`, `not`, `iand`.

## `btof`

Convert an in-place Boolean integer to floating point.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00e3` | `btof rValue` | Set the float payload to `0.0` or `1.0`. |

### Operands And Semantics

Zero in the register's integer payload produces `0.0`; any nonzero value
produces `1.0`. Only the float payload changes; the integer payload, cursor,
attributes, type metadata, and flags remain intact.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-btof" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,1
    btof r0
    ret
```

### Related

`btoi`, `btos`, `itof`.

## `btoi`

Canonicalize an in-place Boolean integer.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00e1` | `btoi rValue` | Replace any nonzero integer with `1`. |

### Operands And Semantics

An integer zero remains zero; every nonzero integer becomes one. No other
payload, cursor, attribute, type field, or flag is changed.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-btoi" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,9
    btoi r0
    ret
```

### Related

`btof`, `btos`, `itob`.

## `btos`

Convert an in-place Boolean integer to its one-character string form.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00e4` | `btos rValue` | Set the string payload to `"0"` or `"1"`. |

### Operands And Semantics

Zero in the integer payload selects `"0"`; nonzero selects `"1"`. The string
payload is replaced and its cursor reset. The integer payload and non-string
state remain unchanged.

### Signals

There is no translated VM signal; string allocation failure is fatal.

### Example

<!-- rxas-example name="integer-btos" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,1
    btos r0
    ret
```

### Related

`btof`, `btoi`, `itos`.

## `iadd`

Add two signed integers with overflow checking.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x000f` | `iadd rResult,rLeft,rRight` | Add two register integers. |
| `0x0010` | `iadd rResult,rLeft,integer` | Add a register integer and literal. |

### Operands And Semantics

The destination integer receives the mathematical sum when representable.
Sources are unchanged; only the destination integer payload is written.

### Signals

Raises `OVERFLOW_UNDERFLOW` when the signed result is outside `rxinteger`.
On that path the destination is not replaced with a wrapped result.

### Example

<!-- rxas-example name="integer-iadd" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,40
    iadd r0,r1,2
    ret
```

### Related

`isub`, `imult`, `inc`.

## `iand`

Compute bitwise AND of two fixed-width integers.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00a8` | `iand rResult,rLeft,rRight` | AND two register integers. |
| `0x00a9` | `iand rResult,rLeft,integer` | AND a register integer and literal. |

### Operands And Semantics

The operation covers every bit of the signed `rxinteger` representation and
writes only the destination integer payload. Sources and cursors are unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-iand" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,13
    iand r0,r1,6
    ret
```

### Related

`ior`, `ixor`, `inot`, `and`.

## `ichkrng`

Require an integer to lie within an inclusive lower/upper bound.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01f7` | `ichkrng rValue,minimum,maximum` | Register value, literal bounds. |
| `0x01f8` | `ichkrng rValue,minimum,rMaximum` | Register value, mixed bounds. |
| `0x01f9` | `ichkrng rValue,rMinimum,rMaximum` | All values in registers. |
| `0x01fa` | `ichkrng value,minimum,rMaximum` | Literal value/minimum, register maximum. |
| `0x01fb` | `ichkrng value,rMinimum,rMaximum` | Literal value, register bounds. |

### Operands And Semantics

Every operand is read as a signed integer. Equality with either bound succeeds.
The instruction is a check only: it does not modify registers or cursors, and
does not diagnose reversed bounds separately.

### Signals

Raises `OUT_OF_RANGE` when `value < minimum` or `value > maximum`.

### Example

<!-- rxas-example name="integer-ichkrng" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,5
    ichkrng r0,1,10
    ret
```

### Related

`igt`, `ilt`, `asserttype`.

## `icopy`

Copy only an integer payload between registers.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0009` | `icopy rDestination,rSource` | Assign `rDestination.int = rSource.int`. |

### Operands And Semantics

Only the destination integer payload changes. Its string, float, decimal and
binary payloads, attributes, cursor, type metadata, and flags remain intact;
the source is unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-icopy" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,42
    icopy r0,r1
    ret
```

### Related

`copy`, `fcopy`, `scopy`, `bcopy`.

## `idiv`

Divide signed integers with truncation toward zero.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0016` | `idiv rResult,rDividend,rDivisor` | Divide two register integers. |
| `0x0017` | `idiv rResult,rDividend,divisor` | Use a literal divisor. |
| `0x0018` | `idiv rResult,dividend,rDivisor` | Use a literal dividend. |

### Operands And Semantics

The destination integer receives the C signed-integer quotient, truncated
toward zero. Sources are unchanged and no remainder is retained.

### Signals

Raises `DIVISION_BY_ZERO` for a zero divisor. Raises
`OVERFLOW_UNDERFLOW` for minimum-`rxinteger` divided by `-1`; no wrapped result
is written on either path.

### Example

<!-- rxas-example name="integer-idiv" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,7
    idiv r0,r1,2
    ret
```

### Related

`imod`, `imult`.

## `ieq`

Compare signed integers for equality.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0064` | `ieq rResult,rLeft,rRight` | Compare two register integers. |
| `0x0065` | `ieq rResult,rLeft,integer` | Compare with a literal. |

### Operands And Semantics

The destination integer becomes canonical Boolean `1` for equality, otherwise
`0`. Sources and cursors are unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-ieq" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,4
    ieq r0,r1,4
    ret
```

### Related

`ine`, `rseq`, `req`.

## `igt`

Compare whether one signed integer is greater than another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0068` | `igt rResult,rLeft,rRight` | Test register > register. |
| `0x0069` | `igt rResult,rLeft,integer` | Test register > literal. |
| `0x006a` | `igt rResult,integer,rRight` | Test literal > register. |

### Operands And Semantics

The destination integer becomes `1` when the ordered comparison is true and
`0` otherwise. Only that payload changes; sources are unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-igt" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,3
    igt r0,5,r1
    ret
```

### Related

`igte`, `ilt`, `igtbr`.

## `igtbr`

Branch when one register integer is greater than another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0157` | `igtbr label,rLeft,rRight` | Branch if `rLeft.int > rRight.int`. |

### Operands And Semantics

The label is resolved within the current procedure. A true signed comparison
transfers control; false falls through. Neither source is mutated.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-igtbr" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,2
    load r1,1
    igtbr greater,r0,r1
    ret
greater:
    ret
```

### Related

`iltbr`, `igt`, `brt`.

## `igte`

Compare whether one signed integer is greater than or equal to another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x006b` | `igte rResult,rLeft,rRight` | Test register >= register. |
| `0x006c` | `igte rResult,rLeft,integer` | Test register >= literal. |
| `0x006d` | `igte rResult,integer,rRight` | Test literal >= register. |

### Operands And Semantics

The destination integer becomes canonical Boolean `0` or `1`. Only that
payload changes; both sources remain unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-igte" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,5
    igte r0,r1,5
    ret
```

### Related

`igt`, `ilte`, `ichkrng`.

## `ilt`

Compare whether one signed integer is less than another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x006e` | `ilt rResult,rLeft,rRight` | Test register < register. |
| `0x006f` | `ilt rResult,rLeft,integer` | Test register < literal. |
| `0x0070` | `ilt rResult,integer,rRight` | Test literal < register. |

### Operands And Semantics

The destination integer becomes `1` when true and `0` otherwise. Only that
payload changes; sources remain unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-ilt" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,5
    ilt r0,r1,6
    ret
```

### Related

`ilte`, `igt`, `iltbr`.

## `iltbr`

Branch when one register integer is less than another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0158` | `iltbr label,rLeft,rRight` | Branch if `rLeft.int < rRight.int`. |

### Operands And Semantics

A true signed comparison transfers to the procedure-local label; false falls
through. Neither register nor cursor is changed.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-iltbr" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,1
    load r1,2
    iltbr less,r0,r1
    ret
less:
    ret
```

### Related

`igtbr`, `ilt`, `brt`.

## `ilte`

Compare whether one signed integer is less than or equal to another.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0071` | `ilte rResult,rLeft,rRight` | Test register <= register. |
| `0x0072` | `ilte rResult,rLeft,integer` | Test register <= literal. |
| `0x0073` | `ilte rResult,integer,rRight` | Test literal <= register. |

### Operands And Semantics

The destination integer becomes canonical Boolean `0` or `1`. Sources and
cursors are unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-ilte" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,5
    ilte r0,5,r1
    ret
```

### Related

`ilt`, `igte`, `ichkrng`.

## `imod`

Compute the signed remainder paired with truncating integer division.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0019` | `imod rResult,rDividend,rDivisor` | Register remainder register. |
| `0x001a` | `imod rResult,rDividend,divisor` | Literal divisor. |
| `0x001b` | `imod rResult,dividend,rDivisor` | Literal dividend. |

### Operands And Semantics

The result follows C signed `%`: it is consistent with division truncated
toward zero and has the dividend's sign when nonzero. Only the destination
integer changes; sources remain unchanged.

### Signals

Raises `DIVISION_BY_ZERO` for zero divisor and `OVERFLOW_UNDERFLOW` for
minimum-`rxinteger` modulo `-1`; no result is written on failure.

### Example

<!-- rxas-example name="integer-imod" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,7
    imod r0,r1,3
    ret
```

### Related

`idiv`, `imult`.

## `imult`

Multiply two signed integers with overflow checking.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0014` | `imult rResult,rLeft,rRight` | Multiply register integers. |
| `0x0015` | `imult rResult,rLeft,integer` | Multiply by a literal. |

### Operands And Semantics

The destination integer receives the product when representable. Sources and
cursors remain unchanged.

### Signals

Raises `OVERFLOW_UNDERFLOW` rather than storing a wrapped product.

### Example

<!-- rxas-example name="integer-imult" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,6
    imult r0,r1,7
    ret
```

### Related

`iadd`, `idiv`, `ipow`.

## `inc`

Increment a selected register integer in place.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x001c` | `inc rValue` | Add one to `rValue.int`. |

### Operands And Semantics

Only the register's integer payload changes. Its other payloads, cursor,
attributes, type metadata, and flags remain intact.

### Signals

Raises `OVERFLOW_UNDERFLOW` at maximum `rxinteger` without wrapping.

### Example

<!-- rxas-example name="integer-inc" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,0
    inc r0
    ret
```

### Related

`inc0`, `inc1`, `inc2`, `iadd`.

## `inc0`

Increment local register `r0` without encoding an operand.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x001e` | `inc0` | Add one to `r0.int`. |

### Operands And Semantics

Only `r0`'s integer payload changes. The compact form requires the current
frame to provide register zero.

### Signals

Raises `OVERFLOW_UNDERFLOW` at maximum `rxinteger`.

### Example

<!-- rxas-example name="integer-inc0" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,0
    inc0
    ret
```

### Related

`inc`, `inc1`, `inc2`.

## `inc1`

Increment local register `r1` without encoding an operand.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0020` | `inc1` | Add one to `r1.int`. |

### Operands And Semantics

Only `r1`'s integer payload changes. The frame must provide register one.

### Signals

Raises `OVERFLOW_UNDERFLOW` at maximum `rxinteger`.

### Example

<!-- rxas-example name="integer-inc1" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,0
    inc1
    ret
```

### Related

`inc`, `inc0`, `inc2`.

## `inc2`

Increment local register `r2` without encoding an operand.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0022` | `inc2` | Add one to `r2.int`. |

### Operands And Semantics

Only `r2`'s integer payload changes. The frame must provide register two.

### Signals

Raises `OVERFLOW_UNDERFLOW` at maximum `rxinteger`.

### Example

<!-- rxas-example name="integer-inc2" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r2,0
    inc2
    ret
```

### Related

`inc`, `inc0`, `inc1`.

## `ine`

Compare signed integers for inequality.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0066` | `ine rResult,rLeft,rRight` | Compare register integers. |
| `0x0067` | `ine rResult,rLeft,integer` | Compare with a literal. |

### Operands And Semantics

The destination integer becomes canonical Boolean `1` when unequal and `0`
when equal. Only that payload changes; sources and cursors are unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-ine" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,4
    ine r0,r1,5
    ret
```

### Related

`ieq`, `rne`.

## `inot`

Invert every bit of an integer.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00b2` | `inot rResult,rValue` | Complement a register integer. |
| `0x00b3` | `inot rResult,integer` | Complement a literal integer. |

### Operands And Semantics

The destination integer receives the fixed-width two's-complement bitwise
complement. Only its integer payload changes; a register source is unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-inot" test="run" -->
```rxas
.globals=0

main() .locals=1
    inot r0,0
    ret
```

### Related

`iand`, `ior`, `ixor`, `not`.

## `ior`

Compute bitwise OR of two integers.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00aa` | `ior rResult,rLeft,rRight` | OR register integers. |
| `0x00ab` | `ior rResult,rLeft,integer` | OR with a literal. |

### Operands And Semantics

Every `rxinteger` bit participates. Only the destination integer payload
changes; sources and cursors remain unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-ior" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,8
    ior r0,r1,3
    ret
```

### Related

`iand`, `ixor`, `inot`, `or`.

## `ipow`

Raise a signed integer base to an integer exponent with checked multiplication.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0153` | `ipow rResult,rBase,rExponent` | Register base and exponent. |
| `0x0154` | `ipow rResult,rBase,exponent` | Literal exponent. |
| `0x0155` | `ipow rResult,base,rExponent` | Literal base. |

### Operands And Semantics

Nonnegative powers use exponentiation by squaring; exponent zero returns one.
Negative exponents return `1` for base `1`, parity-selected `1`/`-1` for base
`-1`, and otherwise fail after setting the destination integer to zero.
Sources are unchanged.

### Signals

Raises `OVERFLOW_UNDERFLOW` for an unrepresentable intermediate/product or an
unsupported negative exponent.

### Example

<!-- rxas-example name="integer-ipow" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,3
    ipow r0,r1,4
    ret
```

### Related

`imult`, `fpow`.

## `irand`

Generate a process-library pseudorandom integer, optionally reseeding first.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01d0` | `irand rResult,rSeed` | Use a seed from a register. |
| `0x01d1` | `irand rResult,seed` | Use a literal seed. |

### Operands And Semantics

A nonnegative seed reseeds the C library generator on every call. A negative
seed leaves an established sequence alone; on the first such call it seeds from
seconds since midnight. The destination is set to the next nonnegative `rand()`
result; the seed register is unchanged.

### Signals

This instruction does not signal. The generator is global C-library state and
is not specified as cryptographically secure or cross-platform reproducible.

### Example

<!-- rxas-example name="integer-irand" test="run" -->
```rxas
.globals=0

main() .locals=1
    irand r0,1
    ret
```

### Related

`rxhash`.

## `isex`

Negate a signed integer in place.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0156` | `isex rValue` | Replace `rValue.int` with its additive inverse. |

### Operands And Semantics

Only the integer payload changes; all other payloads, cursor, attributes, type
metadata, and flags remain intact.

### Signals

Raises `OVERFLOW_UNDERFLOW` for minimum `rxinteger`, whose positive counterpart
is not representable. The value is not wrapped.

### Example

<!-- rxas-example name="integer-isex" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,5
    isex r0
    ret
```

### Related

`isub`, `fsex`, `dsex`.

## `ishl`

Shift a signed integer bit pattern left.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00ae` | `ishl rResult,rValue,rCount` | Register shift count. |
| `0x00af` | `ishl rResult,rValue,count` | Literal shift count. |

### Operands And Semantics

The VM applies the host C signed-left-shift operator and writes only the
destination integer payload. Sources are unchanged. Callers must provide a
nonnegative count smaller than the `rxinteger` bit width and a value whose
shifted result is representable.

### Signals

There is no bounds or overflow check and no VM signal for invalid shift input;
out-of-domain counts or signed overflow have host-C undefined behavior.

### Example

<!-- rxas-example name="integer-ishl" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,3
    ishl r0,r1,2
    ret
```

### Related

`ishr`, `imult`.

## `ishr`

Shift a signed integer bit pattern right.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00b0` | `ishr rResult,rValue,rCount` | Register shift count. |
| `0x00b1` | `ishr rResult,rValue,count` | Literal shift count. |

### Operands And Semantics

The VM applies host C signed right shift and writes only the destination
integer. Nonnegative values shift logically; negative-value fill behavior is
the host compiler's signed-shift behavior. Sources remain unchanged.

### Signals

No count validation or signal is provided. Counts must be nonnegative and less
than the `rxinteger` bit width; other counts have undefined host-C behavior.

### Example

<!-- rxas-example name="integer-ishr" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,12
    ishr r0,r1,2
    ret
```

### Related

`ishl`, `idiv`.

## `isub`

Subtract signed integers with overflow checking.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0011` | `isub rResult,rLeft,rRight` | Register minus register. |
| `0x0012` | `isub rResult,rLeft,integer` | Register minus literal. |
| `0x0013` | `isub rResult,integer,rRight` | Literal minus register. |

### Operands And Semantics

The destination receives the mathematical difference when representable. Only
its integer payload changes; source registers remain unchanged.

### Signals

Raises `OVERFLOW_UNDERFLOW` instead of writing a wrapped result.

### Example

<!-- rxas-example name="integer-isub" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,2
    isub r0,10,r1
    ret
```

### Related

`iadd`, `isex`.

## `itob`

Canonicalize an integer payload as Boolean in place.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00ea` | `itob rValue` | Replace zero with `0`, nonzero with `1`. |

### Operands And Semantics

Only the integer payload is normalized. Other payloads, cursor, attributes,
type metadata, and flags remain unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-itob" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,-3
    itob r0
    ret
```

### Related

`btoi`, `ftob`, `stob`.

## `itof`

Convert an integer payload to floating point in the same register or directly
into a separate destination.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00e7` | `itof rValue` | Assign `(double)rValue.int` to its float payload. |
| `0x0127` | `itof rResult,rValue` | Copy `rValue.int` and its floating-point conversion into `rResult`. |

### Operands And Semantics

The in-place form changes only the destination float payload. The two-register
form copies the source integer payload into the destination and writes its
floating-point conversion there; the source register is unchanged. Large
integers can round to the nearest representable host `double`.

### Signals

This instruction does not report precision loss or signal.

### Example

<!-- rxas-example name="integer-itof" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,42
    itof r0
    ret
```

### Related

`ftoi`, `btof`, `itos`.

## `itos`

Format an integer payload as a decimal string in the same register.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00e5` | `itos rValue` | Replace the string payload with the integer spelling. |

### Operands And Semantics

Formatting uses the current frame numeric context. The string payload is
replaced and its cursor reset; the integer payload and other value state remain
unchanged.

### Signals

There is no translated VM signal; allocation failure is fatal.

### Example

<!-- rxas-example name="integer-itos" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,-42
    itos r0
    ret
```

### Related

`stoi`, `itof`, `btos`.

## `ixor`

Compute bitwise exclusive OR of two integers.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00ac` | `ixor rResult,rLeft,rRight` | XOR register integers. |
| `0x00ad` | `ixor rResult,rLeft,integer` | XOR with a literal. |

### Operands And Semantics

Every fixed-width integer bit participates. Only the destination integer
payload changes; source registers and cursors are unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-ixor" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,10
    ixor r0,r1,6
    ret
```

### Related

`iand`, `ior`, `inot`.

## `not`

Compute logical negation of an integer truth value.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00b6` | `not rResult,rValue` | Store `1` for zero, otherwise `0`. |

### Operands And Semantics

The destination integer becomes a canonical Boolean. Only that payload changes;
the source and both cursors remain unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-not" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,0
    not r0,r1
    ret
```

### Related

`and`, `or`, `inot`.

## `or`

Compute logical disjunction of two integer truth values.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00b5` | `or rResult,rLeft,rRight` | Store `1` iff either operand is nonzero. |

### Operands And Semantics

The destination integer becomes canonical Boolean `0` or `1`. Only its integer
payload changes; sources and cursors remain unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-or" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,0
    load r2,2
    or r0,r1,r2
    ret
```

### Related

`and`, `not`, `ior`.

## `req`

Perform loose REXX equality comparison.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0233` | `req rResult,rLeft,rRight` | Compare register strings. |
| `0x0234` | `req rResult,rLeft,"right"` | Compare register with constant. |
| `0x0235` | `req rResult,"left",rRight` | Compare constant with register. |

### Operands And Semantics

If both complete strings parse as floating-point numbers, numeric values are
compared. Otherwise bytes are compared lexically after padding the shorter
string on the right with spaces. The destination integer becomes `0` or `1`;
sources and cursors are unchanged.

### Signals

This instruction does not signal for nonnumeric text or invalid UTF-8.

### Example

<!-- rxas-example name="integer-req" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,"01"
    req r0,r1,"1"
    ret
```

### Related

`rne`, `rseq`, `seq`.

## `rgt`

Perform loose REXX greater-than comparison.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0239` | `rgt rResult,rLeft,rRight` | Register strings. |
| `0x023a` | `rgt rResult,rLeft,"right"` | Register and constant. |
| `0x023b` | `rgt rResult,"left",rRight` | Constant and register. |

### Operands And Semantics

Both numeric strings compare as floating-point numbers; otherwise comparison is
unsigned-byte lexical with right-space padding. The destination integer becomes
`1` only when left is greater. Sources and cursors are unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-rgt" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,"9"
    rgt r0,r1,"2"
    ret
```

### Related

`rgte`, `rlt`, `req`.

## `rgte`

Perform loose REXX greater-than-or-equal comparison.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x023c` | `rgte rResult,rLeft,rRight` | Register strings. |
| `0x023d` | `rgte rResult,rLeft,"right"` | Register and constant. |
| `0x023e` | `rgte rResult,"left",rRight` | Constant and register. |

### Operands And Semantics

Comparison is numeric when both strings parse as floating point, otherwise
unsigned-byte lexical with right-space padding. The destination receives
canonical Boolean greater-or-equal; sources and cursors are unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-rgte" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,"2"
    rgte r0,r1,"2.0"
    ret
```

### Related

`rgt`, `rlte`, `req`.

## `rlt`

Perform loose REXX less-than comparison.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x023f` | `rlt rResult,rLeft,rRight` | Register strings. |
| `0x0240` | `rlt rResult,rLeft,"right"` | Register and constant. |
| `0x0241` | `rlt rResult,"left",rRight` | Constant and register. |

### Operands And Semantics

Both numeric strings compare as floating-point numbers; otherwise comparison is
unsigned-byte lexical with right-space padding. The destination integer is `1`
only when left is less. Sources and cursors are unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-rlt" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,"2"
    rlt r0,r1,"10"
    ret
```

### Related

`rlte`, `rgt`, `req`.

## `rlte`

Perform loose REXX less-than-or-equal comparison.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0242` | `rlte rResult,rLeft,rRight` | Register strings. |
| `0x0243` | `rlte rResult,rLeft,"right"` | Register and constant. |
| `0x0244` | `rlte rResult,"left",rRight` | Constant and register. |

### Operands And Semantics

Comparison is numeric when both strings parse as floating point, otherwise
unsigned-byte lexical with right-space padding. The destination receives
canonical Boolean less-or-equal; sources and cursors are unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-rlte" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,"10"
    rlte r0,r1,"10.0"
    ret
```

### Related

`rlt`, `rgte`, `req`.

## `rne`

Perform loose REXX inequality comparison.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0236` | `rne rResult,rLeft,rRight` | Compare register strings. |
| `0x0237` | `rne rResult,rLeft,"right"` | Register and constant. |
| `0x0238` | `rne rResult,"left",rRight` | Constant and register. |

### Operands And Semantics

Both numeric strings compare as floating-point numbers; otherwise the VM uses
unsigned-byte lexical comparison with right-space padding. The destination is
`1` when unequal and `0` when equal. Sources and cursors are unchanged.

### Signals

This instruction does not signal.

### Example

<!-- rxas-example name="integer-rne" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,"2"
    rne r0,r1,"3"
    ret
```

### Related

`req`, `rseq`, `sne`.

## `rseq`

Compare strings for equality after trimming ASCII spaces at both ends.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0076` | `rseq rResult,rLeft,rRight` | Compare two register strings. |
| `0x0077` | `rseq rResult,rLeft,"right"` | Compare register with constant. |

### Operands And Semantics

Leading and trailing byte `0x20` spaces are ignored independently, then the
remaining bytes must match exactly. Unlike `req`, numeric spellings are not
converted and internal spaces are significant. The destination integer becomes
`0` or `1`; sources and cursors are unchanged.

### Signals

This byte comparison does not validate UTF-8 or signal.

### Example

<!-- rxas-example name="integer-rseq" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,"  value "
    rseq r0,r1,"value"
    ret
```

### Related

`req`, `seq`, `sne`.
