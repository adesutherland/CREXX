# Strings And Characters

String instructions operate on length-delimited string payloads. RXAS registers
are not statically typed: each instruction reads or writes the string or integer
payload described below without copying unrelated payloads or public flags.

In UTF builds, character positions and lengths are zero-based Unicode code-point
counts unless an instruction explicitly says one-based or byte-oriented.
Instructions that validate text raise `UNICODE_ERROR`; strict byte comparisons
and the legacy byte-oriented trim/truncate operations do not validate UTF-8.

## `append`

Append one register's complete string payload to another in place.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x008f` | `append rDst,rSrc` | Set `rDst` to `rDst || rSrc`. |

### Operands And Semantics

Both operands are string registers. `rDst` is resized and its VM-private UTF
lookup cache is reset. `rSrc` is unchanged, and self-append is supported.

### Signals

No catchable VM signal is defined; allocation failure is fatal.

### Example

<!-- rxas-example name="string-append" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,"Hello"
    load r1,"!"
    append r0,r1
    ret
```

### Related

`concat`, `sappend`.

## `appendchar`

Append one Unicode code point, supplied as an integer payload, to a string.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x008c` | `appendchar rString,rCodepoint` | Append `rCodepoint` to `rString`. |

### Operands And Semantics

`rCodepoint` is an integer register. UTF builds encode it as UTF-8. The
destination lookup cache is reset; the source register is unchanged.

### Signals

No signal is raised. A non-scalar integer is not rejected, but clears the
destination's known-valid UTF state.

### Example

<!-- rxas-example name="string-appendchar" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,"A"
    load r1,66
    appendchar r0,r1
    ret
```

### Related

`concchar`, `padstr`.

## `concat`

Concatenate two strings without inserting a blank.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0086` | `concat rDst,rLeft,rRight` | Concatenate two register strings. |
| `0x0087` | `concat rDst,rLeft,"text"` | Use a string constant on the right. |
| `0x0088` | `concat rDst,"text",rRight` | Use a string constant on the left. |

### Operands And Semantics

The destination string is replaced and its private lookup cache resets. Register
sources are unchanged and may alias the destination.

### Signals

No catchable VM signal is defined.

### Example

<!-- rxas-example name="string-concat" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"Hello"
    load r2,"World"
    concat r0,r1,r2
    ret
```

### Related

`append`, `sconcat`.

## `concchar`

Append the code point at a zero-based character index in a source string.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x008d` | `concchar rDst,rString,rIndex` | Append `rString[rIndex]` to `rDst`. |

### Operands And Semantics

`rIndex` is an integer register and is restored after temporary internal use.
Locating the code point may update the source's private lookup cache; this is
not observable RXAS state. `rDst` is appended to, not cleared.

### Signals

Invalid UTF-8 raises `UNICODE_ERROR`. There is no explicit index-range signal;
the index must name an existing character.

### Example

<!-- rxas-example name="string-concchar" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r0,""
    load r1,"abc"
    load r2,1
    concchar r0,r1,r2
    ret
```

### Related

`appendchar`, `strchar`.

## `hexchar`

Format a selected source character as uppercase hexadecimal.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0098` | `hexchar rDst,rString,rIndex` | Format the zero-based character at `rIndex`. |

### Operands And Semantics

By default `rDst` receives the low byte of the code point as two hex digits. If
its incoming string contains `UTFV1`, it receives four UTF-8 bytes padded to
eight digits; `UTFV2` requests only the actual UTF-8 bytes. The destination
lookup cache resets; locating the character may update the source's private
cache without changing its value.

### Signals

Invalid UTF-8 raises `UNICODE_ERROR`. There is no explicit range signal;
`rIndex` must name an existing character.

### Example

<!-- rxas-example name="string-hexchar" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r0,""
    load r1,"A"
    load r2,0
    hexchar r0,r1,r2
    ret
```

### Related

`strchar`.

## `padstr`

Append one integer code point repeatedly to an existing string.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00a3` | `padstr rDst,rCodepoint,rCount` | Append the code point `rCount` times. |

### Operands And Semantics

`rCodepoint` and `rCount` are integer registers. The destination is not cleared
first. A zero or negative count appends nothing; otherwise its private lookup
cache is reset.

### Signals

No signal is raised. Invalid Unicode scalar integers are not rejected.

### Example

<!-- rxas-example name="string-padstr" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r0,""
    load r1,42
    load r2,3
    padstr r0,r1,r2
    ret
```

### Related

`appendchar`, `trunc`.

## `poschar`

Find the first occurrence of an integer code point in a string.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0099` | `poschar rIndex,rString,rCodepoint` | Return a zero-based index or `-1`. |

### Operands And Semantics

The result overwrites `rIndex`'s integer payload. The search starts at the
beginning. The logical string and code-point register remain unchanged; any
UTF lookup caching is private to the VM.

### Signals

Invalid UTF-8 raises `UNICODE_ERROR`; not found returns `-1`.

### Example

<!-- rxas-example name="string-poschar" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"abc"
    load r2,98
    poschar r0,r1,r2
    ret
```

### Related

`strpos`, `strchar`.

## `sappend`

Append one ASCII blank and then another register's string payload.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x008e` | `sappend rDst,rSrc` | Set `rDst` to `rDst || " " || rSrc`. |

### Operands And Semantics

The blank is inserted even if either string is empty. `rDst` is resized and its
private lookup cache resets. `rSrc` is unchanged; self-append is supported.

### Signals

No catchable VM signal is defined; allocation failure is fatal.

### Example

<!-- rxas-example name="string-sappend" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,"Hello"
    load r1,"World"
    sappend r0,r1
    ret
```

### Related

`append`, `sconcat`.

## `sconcat`

Concatenate two strings with exactly one ASCII blank between them.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0089` | `sconcat rDst,rLeft,rRight` | Join two register strings. |
| `0x008a` | `sconcat rDst,rLeft,"text"` | Use a string constant on the right. |
| `0x008b` | `sconcat rDst,"text",rRight` | Use a string constant on the left. |

### Operands And Semantics

The destination is replaced and its private lookup cache resets. The intervening
blank is unconditional. Register sources are unchanged and may alias `rDst`.

### Signals

No catchable VM signal is defined; allocation failure is fatal.

### Example

<!-- rxas-example name="string-sconcat" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"Hello"
    load r2,"World"
    sconcat r0,r1,r2
    ret
```

### Related

`concat`, `sappend`.

## `scopy`

Copy only a register's string payload.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x000b` | `scopy rDst,rSrc` | Copy the logical string to `rDst`. |

### Operands And Semantics

The destination receives the bytes, character count, and VM-private UTF
validity state; its private lookup cache resets. Integer, float, decimal, binary,
attributes, and public flags are not copied. The source is unchanged.

### Signals

This instruction does not raise a signal.

### Example

<!-- rxas-example name="string-scopy" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,"copy"
    scopy r0,r1
    ret
```

### Related

`copy`, `concat`.

## `seq`

Test two strings for exact equality.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0074` | `seq rResult,rLeft,rRight` | Compare two register strings. |
| `0x0075` | `seq rResult,rLeft,"text"` | Compare a register with a string constant. |

### Operands And Semantics

This is a length-aware byte comparison with no numeric coercion or
trailing-blank padding. `rResult` receives integer `1` for equality or `0`
otherwise. Sources are unchanged.

### Signals

The comparison does not validate UTF-8 and does not raise a signal.

### Example

<!-- rxas-example name="string-seq" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"same"
    load r2,"same"
    seq r0,r1,r2
    ret
```

### Related

`sne`, `rseq`.

## `sgt`

Test exact lexicographic greater-than ordering.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x007a` | `sgt rResult,rLeft,rRight` | Compare two register strings. |
| `0x007b` | `sgt rResult,rLeft,"text"` | Constant on the right. |
| `0x007c` | `sgt rResult,"text",rRight` | Constant on the left. |

### Operands And Semantics

Strings are compared as length-aware byte sequences with no numeric coercion
or blank padding. `rResult` receives integer `1` or `0`; sources are unchanged.

### Signals

The comparison does not validate UTF-8 and does not signal.

### Example

<!-- rxas-example name="string-sgt" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"b"
    load r2,"a"
    sgt r0,r1,r2
    ret
```

### Related

`sgte`, `slt`.

## `sgte`

Test exact lexicographic greater-than-or-equal ordering.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x007d` | `sgte rResult,rLeft,rRight` | Compare two register strings. |
| `0x007e` | `sgte rResult,rLeft,"text"` | Constant on the right. |
| `0x007f` | `sgte rResult,"text",rRight` | Constant on the left. |

### Operands And Semantics

Comparison is strict and byte-oriented, without numeric conversion or blank
padding. `rResult` receives integer `1` or `0`. Sources are unchanged.

### Signals

The comparison does not validate UTF-8 and does not signal.

### Example

<!-- rxas-example name="string-sgte" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"b"
    load r2,"b"
    sgte r0,r1,r2
    ret
```

### Related

`sgt`, `slte`.

## `slt`

Test exact lexicographic less-than ordering.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0080` | `slt rResult,rLeft,rRight` | Compare two register strings. |
| `0x0081` | `slt rResult,rLeft,"text"` | Constant on the right. |
| `0x0082` | `slt rResult,"text",rRight` | Constant on the left. |

### Operands And Semantics

Comparison is a strict length-aware byte ordering. `rResult` receives integer
`1` or `0`; sources remain unchanged.

### Signals

The comparison does not validate UTF-8 and does not signal.

### Example

<!-- rxas-example name="string-slt" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"a"
    load r2,"b"
    slt r0,r1,r2
    ret
```

### Related

`slte`, `sgt`.

## `slte`

Test exact lexicographic less-than-or-equal ordering.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0083` | `slte rResult,rLeft,rRight` | Compare two register strings. |
| `0x0084` | `slte rResult,rLeft,"text"` | Constant on the right. |
| `0x0085` | `slte rResult,"text",rRight` | Constant on the left. |

### Operands And Semantics

Comparison is strict and byte-oriented. `rResult` receives integer `1` or `0`;
source strings are unchanged.

### Signals

The comparison does not validate UTF-8 and does not signal.

### Example

<!-- rxas-example name="string-slte" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"a"
    load r2,"a"
    slte r0,r1,r2
    ret
```

### Related

`slt`, `sgte`.

## `sne`

Test two strings for exact inequality.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0078` | `sne rResult,rLeft,rRight` | Compare two register strings. |
| `0x0079` | `sne rResult,rLeft,"text"` | Compare a register with a constant. |

### Operands And Semantics

This is the inverse of strict `seq`, including length. `rResult` receives
integer `1` when the byte strings differ and `0` otherwise. Sources are unchanged.

### Signals

The comparison does not validate UTF-8 and does not signal.

### Example

<!-- rxas-example name="string-sne" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"a"
    load r2,"b"
    sne r0,r1,r2
    ret
```

### Related

`seq`.

## `stob`

Convert a string payload to the VM's narrow Boolean convention.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00eb` | `stob rValue` | Write an integer Boolean in the same register. |

### Operands And Semantics

The integer payload becomes `1` only when the string is exactly the single byte
`1`; every other string becomes `0`. The string payload is unchanged.

### Signals

This instruction does not raise a conversion signal.

### Example

<!-- rxas-example name="string-stob" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,"1"
    stob r0
    ret
```

### Related

`stoi`, `itob`, `btos`.

## `stof`

Parse a register's complete string payload as a binary64 value.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00ec` | `stof rValue` | Write the float payload in the same register. |

### Operands And Semantics

The string payload remains unchanged. The whole logical string must
match the VM numeric parser's float grammar.

### Signals

Invalid or out-of-range numeric text raises `CONVERSION_ERROR`.

### Example

<!-- rxas-example name="string-stof" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,"12.5"
    stof r0
    ret
```

### Related

`ftos`, `stoi`, `stod`.

## `stoi`

Parse a register's complete string payload as a VM integer, either in place or
directly into a separate destination.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00ed` | `stoi rValue` | Write the integer payload in the same register. |
| `0x0128` | `stoi rResult,rValue` | Parse `rValue.string` directly into `rResult.int`. |

### Operands And Semantics

The source string payload remains unchanged. In the two-register form
the source register receives no integer side effect. The parser requires a
complete valid integer representation within the VM integer range.

### Signals

Invalid or out-of-range text raises `CONVERSION_ERROR`.

### Example

<!-- rxas-example name="string-stoi" test="run" -->
```rxas
.globals=0

main() .locals=1
    load r0,"42"
    stoi r0
    ret
```

### Related

`itos`, `stof`, `stob`.

## `strchar`

Read a Unicode code point from a string into an integer payload.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0096` | `strchar rCodepoint,rString,rIndex` | Read zero-based character `rIndex`. |

### Operands And Semantics

UTF builds return the Unicode scalar value at the explicit zero-based character
index; non-UTF builds return the byte value. The source is unchanged.

### Signals

Invalid UTF-8 raises `UNICODE_ERROR`. An index outside the source raises
`OUT_OF_RANGE`.

### Example

<!-- rxas-example name="string-strchar" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"A"
    load r2,0
    strchar r0,r1,r2
    ret
```

### Related

`hexchar`, `concchar`, `substring`.

## `strlen`

Return a string's logical character length.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0095` | `strlen rLength,rString` | Write the character count to `rLength`. |

### Operands And Semantics

UTF builds count Unicode code points; non-UTF builds count bytes. The source
string is unchanged. Only the destination integer payload is
written.

### Signals

Invalid UTF-8 raises `UNICODE_ERROR`.

### Example

<!-- rxas-example name="string-strlen" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,"abc"
    strlen r0,r1
    ret
```

### Related

`substring`, `trunc`.

## `strlower`

Copy a string and convert it to lowercase.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x009d` | `strlower rDst,rSrc` | Lowercase `rSrc` into `rDst`. |

### Operands And Semantics

The destination is replaced and its private lookup cache resets. UTF builds use the
project Unicode case mapping. The source is unchanged; destination aliasing is
supported.

### Signals

Invalid UTF-8 raises `UNICODE_ERROR`. Length-changing Unicode mappings are a
documentation-only implementation ambiguity: the opcode does not explicitly
recompute logical length metadata after conversion.

### Example

<!-- rxas-example name="string-strlower" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,"Mixed"
    strlower r0,r1
    ret
```

### Related

`strupper`.

## `strpos`

Find a substring using one-based Rexx positions.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00a4` | `strpos rStartResult,rNeedle,rHaystack` | Search from and return a one-based position. |

### Operands And Semantics

On entry `rStartResult` contains the one-based start; on return it is overwritten
with the one-based match position or `0`. UTF positions count code points. The
haystack value is unchanged. Both source buffers may be NUL-terminated
internally without changing logical text.

### Signals

Invalid UTF-8 raises `UNICODE_ERROR`. A nonpositive start or a start beyond the
haystack returns `0`.

### Example

<!-- rxas-example name="string-strpos" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r0,1
    load r1,"World"
    load r2,"Hello World"
    strpos r0,r1,r2
    ret
```

### Related

`poschar`, `seq`.

## `strupper`

Copy a string and convert it to uppercase.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x009e` | `strupper rDst,rSrc` | Uppercase `rSrc` into `rDst`. |

### Operands And Semantics

The destination is replaced and its private lookup cache resets. UTF builds use the
project Unicode case mapping. The source remains unchanged and may alias the
destination.

### Signals

Invalid UTF-8 raises `UNICODE_ERROR`. The current implementation does not
explicitly refresh length metadata after a length-changing Unicode mapping.

### Example

<!-- rxas-example name="string-strupper" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r1,"Mixed"
    strupper r0,r1
    ret
```

### Related

`strlower`.

## `substcut`

Truncate a string in place to a number of leading Unicode characters.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00a2` | `substcut rString,rLength` | Keep at most `rLength` leading characters. |

### Operands And Semantics

`rLength` is an integer register. Byte and character lengths are updated and
the private lookup cache resets. A length beyond the string leaves it
unchanged. A negative integer converts to a large unsigned size and therefore
also leaves the string unchanged.

### Signals

Invalid UTF-8 raises `UNICODE_ERROR`; out-of-range lengths do not signal.

### Example

<!-- rxas-example name="string-substcut" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,"abcdef"
    load r1,3
    substcut r0,r1
    ret
```

### Related

`trunc`, `substring`.

## `substring`

Copy a character range using an explicit zero-based start and length.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x00a1` | `substring rDst,rSrc,rStart,rLength` | Copy characters `[rStart,rStart+rLength)`. |

### Operands And Semantics

`rStart` and `rLength` are integer registers. A negative start clamps to zero;
a nonpositive length yields an empty destination; and a start or length beyond
the source clips to its character count. The source is unchanged and
destination/source aliasing is supported.

### Signals

Invalid UTF-8 raises `UNICODE_ERROR`; clipping does not signal.

### Example

<!-- rxas-example name="string-substring" test="run" -->
```rxas
.globals=0

main() .locals=4
    load r1,"abcdef"
    load r2,1
    load r3,2
    substring r0,r1,r2,r3
    ret
```

### Related

`strchar`, `substcut`, `trunc`.

## `swap`

Exchange two complete register bindings. The instruction is deprecated, but
its effect is persistent.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x01fe` | `swap rLeft,rRight` | Exchange the two register storage bindings. |

### Operands And Semantics

The VM swaps the two pointers in the current frame's register table. All value
payloads, attributes, reference/native payloads, and status flags therefore
move together with their storage. This is not a
payload copy and does not allocate. Subsequent uses of either register number
see the other register's former complete value.

### Signals

This instruction does not raise a signal.

### Example

<!-- rxas-example name="string-swap" test="run" -->
```rxas
.globals=0

main() .locals=2
    load r0,"left"
    load r1,"right"
    swap r0,r1
    say r0
    say r1
    ret
```

<!-- rxas-output for="string-swap" -->
```text
right
left
```

### Related

`copy`, `move`, `link`.

## `transchar`

Translate one integer code point through two parallel character lists.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x009f` | `transchar rCodepoint,rReplacementList,rSearchList` | Replace a matched code point. |

### Operands And Semantics

`rCodepoint` supplies and receives an integer. The VM scans `rSearchList`; a
match at character index `i` selects character `i` from `rReplacementList`. No
match preserves the input. VM-private UTF lookup caches may change during
scanning without becoming observable value state.

### Signals

Invalid UTF-8 in either list raises `UNICODE_ERROR`. The replacement list must
cover every matching search-list position; the VM has no separate bounds
signal for a shorter replacement list.

### Example

<!-- rxas-example name="string-transchar" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r0,97
    load r1,"A"
    load r2,"a"
    transchar r0,r1,r2
    ret
```

### Related

`poschar`, `strchar`.

## `triml`

Remove repeated leading bytes from a string.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0090` | `triml rString,rIgnored` | Deprecated in-place ASCII-space trim; operand 2 is ignored. |
| `0x0092` | `triml rDst,rSrc,rTrim` | Copy, then trim the first byte of `rTrim`. |

### Operands And Semantics

The two-register form removes leading ASCII spaces. The three-register form is
byte-oriented and removes only repetitions of `rTrim`'s first byte; an empty
trim string removes nothing. The result is NUL-terminated. Character-count
metadata is not consistently recomputed by these legacy forms.

### Signals

No signal is raised and UTF-8 is not validated.

### Example

<!-- rxas-example name="string-triml" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"   text"
    load r2," "
    triml r0,r1,r2
    ret
```

### Related

`trimr`, `trunc`.

## `trimr`

Remove repeated trailing bytes from a string.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0091` | `trimr rString,rIgnored` | Deprecated in-place ASCII-space trim; operand 2 is ignored. |
| `0x0093` | `trimr rDst,rSrc,rTrim` | Copy, then trim the first byte of `rTrim`. |

### Operands And Semantics

The two-register form removes trailing ASCII spaces. The three-register form
removes only repetitions of the first byte in `rTrim`; an empty trim string
removes nothing. The source remains unchanged in the three-register form.
Character-count metadata is not consistently refreshed.

### Signals

No signal is raised and UTF-8 is not validated.

### Example

<!-- rxas-example name="string-trimr" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"text!!!"
    load r2,"!"
    trimr r0,r1,r2
    ret
```

### Related

`triml`, `trunc`.

## `trunc`

Copy a string and cap its logical byte length.

### Forms

| Opcode | Form | Effect |
| --- | --- | --- |
| `0x0094` | `trunc rDst,rSrc,rLength` | Keep at most `rLength` bytes. |

### Operands And Semantics

Despite the historical description saying characters, the implementation
compares `rLength` with the byte length. Negative lengths become zero. It can
therefore split a UTF-8 sequence and does not recompute character metadata.
Use `substcut` for character truncation.

### Signals

This instruction does not validate UTF-8 and raises no range signal.

### Example

<!-- rxas-example name="string-trunc" test="run" -->
```rxas
.globals=0

main() .locals=3
    load r1,"abcdef"
    load r2,3
    trunc r0,r1,r2
    ret
```

### Related

`substcut`, `substring`.
