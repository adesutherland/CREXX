# CRI-13 bounded numeric conversion decision

Status: **resolved by scoped signal translation; bounded helper deferred as `PERF2-07-C01`**

Adrian observed that the conversion signal can be caught. The selected path
therefore retained approved parse-time Alternative D and placed one handler
around the whole projection loop. The focused edge regression passes and the
mandatory R2 Release verdict is favorable. No new opcode was implemented.
See [`CRI13-R2-RELEASE-VERDICT.md`](CRI13-R2-RELEASE-VERDICT.md).

## Outcome first

Alternative D uses a well-established implementation pattern: derive semantic
facts while the lexer/parser already has each token in hand and retain compact
metadata with the parsed node. The exact two flags and the use of the private
u32 node field at offset 36 are CREXX-specific, not a named or standardized
JSON algorithm.

A bounded binary ASCII span-to-float operation is also a conventional shape.
Both offset and length are required: together they name the exact half-open
byte range without a NUL terminator, delimiter scan, source copy, or temporary
string. The current CREXX conversion implementation makes this more than a
performance idea: it exposes a correctness/status mismatch at binary64 range
edges.

No production source, RXAS opcode, RXBIN encoding, ABI, or public API was
changed during this audit.

## Minimized current-product reproduction

Source SHA-256
`123a46d99278bf017dbadaaa9c2812afd879c066f6019b712f79a99bf08eeafb`:

```rexx
options levelb comments_dash
import rxfnsb
import rxjson

cases = .string[]
cases[1] = "1e-320"
cases[2] = "1e-324"
cases[3] = "1.8e308"
cases[4] = "1e999"

do i = 1 to cases[0]
  document = .jsondocument("[" || cases[i] || "]")
  packed = .binary
  error = ""
  status = 999
  caught = 0
  do
    status = document.node_f32_array(document.root(), 1, packed, error)
  on signal conversion_error
    caught = 1
  end
  say "case=" || cases[i] || " status=" || status || " caught=" || caught || " bytes=" || binlength(packed) || " error=" || error
end

return 0
```

Exact commands, using the frozen R1 ordinary Release product:

```text
product_dir=/tmp/crexx-cri09-release-a2.N4ELYs/build
$product_dir/bin/rxc -i $product_dir/bin -x -o edge /private/tmp/crexx-cri13-conversion-edge.crexx
$product_dir/bin/rxas -o edge edge
$product_dir/bin/rxvm edge $product_dir/bin/library
$product_dir/bin/rxbvm edge $product_dir/bin/library
```

Both VMs produce the same result:

```text
case=1e-320 status=999 caught=1 bytes=4 error=
case=1e-324 status=999 caught=1 bytes=4 error=
case=1.8e308 status=999 caught=1 bytes=4 error=
case=1e999 status=-6 caught=0 bytes=0 error=element 1 cannot be represented as f32
```

The identical `rxvm` and `rxbvm` raw logs have SHA-256
`613537ea92add9b371d534294b0fdae4c4007f4891123166316a6fb3acf9e62b`
and are retained at
`/private/tmp/crexx-cri13-conversion-edge.zT0TIX/run.log` and
`/private/tmp/crexx-cri13-conversion-edge.zT0TIX/rxbvm.log`.

Product identity is
`crexx-1.0.0-beta.3+local.gd78c6fcfa81e.dirty`; `rxc`, `rxvm`, and `rxbvm`
SHA-256 values are respectively
`95b702b511191697c2a1b943104024fdf73b08936e1e82b68c5e78599f3cdb90`,
`b1bd31897ac26a4378f52f9498999a7b0b1d32c91443ce908c36ecadf888cb9e`,
and `92bcf6d11bb9c393b7116ac04565f641ea708e7ed9578b4dde31936c77d5538a`.

## Root cause

`rx_string_to_double` in `binutils/include/rxnumparse.h` copies the supplied
span to a NUL-terminated 128-byte local buffer or a heap allocation and calls
`strtod`. It rejects every `ERANGE` result, and accepts trailing whitespace.
The decimal point and accepted spellings inherit the active C locale and C
library `strtod` behavior. `string2integer` separately heap-allocates for every
call before using `strtoimax` through `rxinteger_parse`.

The JSON method first creates a binary slice, converts it to a validated UTF-8
string, copies that string for the float value, and reaches `STOF`. `STOF`
turns the converter failure into `CONVERSION_ERROR`. The public
`node_f32_array` contract instead promises status `-6`, a cleared output, and
an element diagnostic. The existing projection precheck catches only coarse
decimal-exponent cases such as `1e999`; representable decimal syntax around
the binary64 normal/subnormal and overflow edges reaches `STOF` and escapes.

The approved D flags as originally specified use an adjusted exponent band of
approximately -324 through 308. They would therefore retain this mismatch for
examples such as `1e-320` and `1.8e308`. Production D is paused before edit.

## Bounded helper contract under consideration

The placeholder RXAS form is deliberately not assigned a final mnemonic;
`BTOF` already means boolean-to-float:

```text
parse-f64-span  result_float, status_int, source_binary, offset_int, length_int
```

Proposed semantics:

- zero-based offset and non-negative length identify exactly `[offset,
  offset+length)`; ordinary binary range failure retains the VM's existing
  `OUT_OF_RANGE` behavior;
- the source is read-only and remains unchanged;
- conversion consumes the complete slice, is locale-independent, performs no
  allocation or copy, and rounds exactly to IEEE-754 binary64, ties to even;
- parse outcome is non-throwing: success, invalid syntax, overflow, and
  underflow are distinguishable in `status_int`;
- the result is a CREXX `.float` (binary64); f32 packing remains an explicit
  checked `bsetf32`, so source nonzero to stored-zero underflow can be rejected;
- JSON grammar remains the parser's responsibility. The conversion core must
  also expose a compatibility profile for existing `STOF`, which currently
  supports CREXX uses of signs, whitespace and special values such as `inf`
  and `nan`; a global switch to strict JSON grammar would be incompatible.

Five operands are mechanically representable by current RXAS. The historical
three-operand implementation constraint is no longer the limiting factor.

## Alternatives

### A — retain approved D exactly and keep current conversion

No ABI or serialized change and likely much faster than R1, but the minimized
edge cases still escape the documented method status. This is not acceptable.

### B — change D to an f32-specific conservative exponent prefilter

Cache nonzero plus an f32-candidate exponent band during parsing, reject clear
f32 overflow/underflow before `STOF`, and retain the current materialized
conversion for boundary values. This can preserve the public JSON method
contract without an RXAS/RXBIN change and is the smallest local repair.

It is JSON/f32-specific duplicated numeric policy, leaves `node_float`, `STOF`,
and integer conversion debt in place, retains slice/string copies and
locale-sensitive `strtod`, and may need revisiting when the conversion layer is
fixed. It is a useful fallback or performance comparator, not the recommended
final architecture.

### C — bounded conversion core plus public span operation (recommended)

Introduce one tested bounded, locale-independent numeric conversion core and a
non-throwing binary span-to-binary64 VM operation. Use it from the JSON class;
then separately route `STOF` through the compatible profile after exhaustive
behavioral inventory. Add a matching bounded signed-integer path only as a
separate measured rung.

For CRI-13 this removes the binary slice, UTF-8 validation, string copy,
NUL-copy, allocation possibility, and signal/status mismatch. A successful
binary64 result itself supplies the nonzero fact needed by the later f32
underflow check, while explicit conversion status handles values outside
binary64. The original D metadata becomes unnecessary, so the parser and node
remain unchanged.

This is a new public RXAS/RXBIN instruction and therefore a serialized-format
and architecture decision. It also requires selecting and maintaining an exact
decimal-to-binary engine. A C99 port of the established fast-float/Eisel-Lemire
family is a plausible comparator; the current `strtod` wrapper and a
handwritten exact converter must be measured and correctness-tested rather
than selected by assertion. Writing the JSON tokenizer by hand is reasonable;
writing a correctly rounded decimal-to-binary converter is a different and
substantially harder problem.

### D — private execution-image fusion of the existing canonical sequence

Recognize and fuse the compiler's exact slice-to-string-to-float sequence while
preparing the process-local execution image. This preserves public RXAS/RXBIN
but must reproduce allocation failure, UTF-8, register mutation, signal and
TRACE semantics for every fallback. It is less reusable, more shape-sensitive,
and does not give `node_float` a clean non-throwing status surface. Keep only as
a comparator if a public instruction is rejected.

## Converter proof required before selection

Any replacement engine must pass exhaustive focused cases for signed zero,
normal/subnormal boundaries, half-way rounding, maximum finite values,
overflow, underflow, long mantissas/exponents, full-slice consumption,
whitespace, signs, `inf`/`nan`, invalid forms and locale changes. Results must
be byte-compared with an accepted correctly rounded oracle, fuzzed with retained
seeds, run under normal Debug and ASan, and measured for allocations, copied
bytes, time, code size and both VM variants. External benchmark claims are not
CREXX acceptance evidence.

## Superseded recommendation and resolution

The pre-resolution recommendation was to measure Alternative C immediately.
Adrian instead selected the smaller scoped catch, which preserves D and the
public JSON contract without a VM-format decision. R2 passes every rule and is
now at its own mandatory acceptance stop. The broader bounded converter review
is queued independently as `PERF2-07-C01`; the continuation prompt below is
retained only as the exact unselected C proposal.

## Paste-ready continuation prompt

```text
Resume /Users/adrian/CLionProjects/CREXX at the CRI-13 bounded numeric
conversion decision stop using performance/CREXX-RAG-INTEGRATION-WORKLIST.md
and performance/evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-BOUNDED-NUMERIC-CONVERSION-DECISION.md.

I approve CRI-13 Alternative C as a provisional, independently revertible
architecture/performance rung. Implement a bounded, allocation-free,
locale-independent, correctly rounded numeric conversion core and a
non-throwing five-operand binary span-to-binary64 VM operation with exact
offset/length, full-slice consumption, distinct invalid/overflow/underflow
status and unchanged source. Do not change general STOF semantics or add the
integer companion in this rung. Add the exact edge regression, focused
opt/no-opt dual-VM correctness, allocation/copy evidence and converter corpus;
freeze; compare the f32-specific D fallback and C on the same ordinary Release
CRI-13 matrix; report the first verdict and stop before production selection,
broad QA, C classes, CRI-14, ABI publication or crexx-rag work.
```
