# Related numeric defect scan — 11 October 2026

This follow-up audits numeric display/value boundaries and adjacent float
edge behavior using the already qualified, locally repaired current-develop
Debug tools. **Eight related findings are confirmed and remain unfixed.**
The scan makes no compiler/runtime edits, publication, or research-checkout
changes. The original float repair source hashes match the earlier receipt.
The authoritative findings and next actions are `FSCAN-01` through `FSCAN-08`
in [the existing worklist](../../../../compiler/docs/float_constant_folding_worklist.md).

Every Rexx finding was exercised in B/G, compiler opt/noopt, assembler
opt/noopt, and rxtvm/rxbvm. Only a compiler/assembler failure prevents its VM
execution. Complete diagnostics and actual exit statuses remain in each
stage's `scan.json`; `summary.json` provides compact per-level/compiler
outcomes. A phase rejecting invalid input at compile time rather than runtime
is not itself classified as a defect. The cast findings below instead show
invalid input being accepted, or valid results being changed.
FSCAN-08 affects both compiler modes identically; typed-destination controls
and the documented checked-cast contract establish its incorrect behavior.

## Confirmed observations

| Finding | Minimal shape | Compiler `-n` | Optimized compiler |
| --- | --- | --- | --- |
| FSCAN-01: integer precision | DIGITS 5; `i = 9007199254740993; say i = 9007199254740993` | `1` | `0` |
| FSCAN-01: integer fold | DIGITS 5; `say 9007199254740992 + 1 = 9007199254740993` | `1` | `0` |
| FSCAN-01: integer overflow | DIGITS 5; `i = 9223372036854775807; say i = 9223372036854775807` | `1` | Compiler overflow, exit 2 |
| FSCAN-01: integer transport | DIGITS 5; `i = 123456; say i + runtime_int = 123457` | `1` with input `1` | RXAS failure, exit 255 |
| FSCAN-02: float-to-decimal | DIGITS 18; typed `f = 1.234567890123456; d = f as .decimal; say d = 1.234567890123456d` | `1` | `0` |
| FSCAN-03: float-to-integer | Typed `.int` destination; `i = 1.0000000000000002 as .int` | CONVERSION_ERROR, exit 6 | Accepts and prints `1`, exit 0 |
| FSCAN-03: display rounding | DIGITS 5; `f = 1.000001; i = f as .int` with typed integer destination | CONVERSION_ERROR, exit 6 | Accepts and prints `1`, exit 0 |
| FSCAN-04: NaN equality | `(0.0 / 0.0) = (0.0 / 0.0)` | `0` | `1` |
| FSCAN-04: NaN inequality | `(0.0 / 0.0) <> (0.0 / 0.0)` | `1` | `0` |
| FSCAN-04: NaN ordering | NaN `>= 1.0` and `<= 1.0` | `0`, `0` | `1`, `1` |
| FSCAN-05: subnormal literal | Typed float `a = 1e-310; say a > 0.0` | `1` | BAD_CONVERSION, exit 2 |
| FSCAN-06: non-finite operand | `say (1.0 / 0.0) + runtime_float`; equivalent NaN case | `inf` / `nan` | RXAS failure on `inf.0` / `nan.0`, exit 255 |
| FSCAN-07: signed zero | Typed float `z = -0.0; say runtime_float / z` | `inf` with input `1.0` | `-inf` |
| FSCAN-08: ignored boolean cast | `say (2 as .boolean)` | Incorrect `2` | Incorrect `2` |
| FSCAN-08: ignored checked cast | `say (1.5 as .int)` or typed `.string` assignment of that expression | Incorrect `1.5`, no signal | Incorrect `1.5`, no compile failure/signal |

The integer precision/overflow cases follow the same family as the original
float defect: an exact integer becomes contextual scientific notation, then
integer conversion falls back through binary64, losing low bits or rounding
INT64_MAX beyond its bound. Integer emission also preserves scientific notation
as an RXAS operand, which the lexer classifies as float. For example, a valid
integer add emits `iadd ...,1.23456e+5`, or the already corrupted large integer
emits `iadd ...,9.007199254740992e+15`; these forms are invalid integer operands.

Float-to-decimal folding uses 15-digit display text while the actual runtime
FTOD provider uses 16 significant digits. Both current `mc_decimal` and
`db_decimal` implement that 16-digit boundary. The runtime contract must be
preserved when repairing compiler equivalence; changing FTOD to a new precision
would require separate review. Typed float-to-integer folding also has an
explicit epsilon absent from the VM's exact checked conversion.

NaN has an unordered comparison result. The compiler's three-way subtraction
comparator collapses it to equality. Unary negation has a separate cause:
unoptimized emission uses `fsub dst,0.0,source`, yielding positive zero for
`+0.0`, whereas unary `-` must produce negative zero in the binary64 model.
The existing nonoptimized lowering is unchanged by the preceding repair;
retaining the optimized typed payload exposes that disagreement.

Subnormal rejection comes from treating all `strtod` ERANGE outcomes as invalid
in the shared text parser, while RXAS's literal `atof` admits them. A future
repair must distinguish literal transport from any intentionally strict string
conversion/comparison policy. Non-finite float transport has another concrete
lexer boundary: compiler `inf.0`/`nan.0` and RXDAS `inf`/`-inf` are outside the
current numeric literal grammar.

The cast-consumer probes expose another boundary: the cast emitter selects
`emit_promotion[child1->value_type][node->target_type]`, where the parent string
consumer has retargeted the node. It bypasses the explicit cast type carried
by child2. In unoptimized RXAS, the boolean case emits only ITOS, with no ITOB;
the fractional integer case emits only FTOS, with no FTOI. Folded casts also
rewrite directly to the consumer target. A typed boolean destination emits
the required conversion and prints `1`, so this is not an alternative Boolean
display policy. It explains why output-only scalar probes can hide conversion
bugs and why both typed destinations and string consumers need regressions.

## Transport and controls

The `transport/` stage assembles an infinity constant using currently accepted
`1e999` or `-1e999`, runs it on both VMs, disassembles it and attempts to
reassemble the emitted RXAS. Both assembler modes fail reassembly on the
`inf`/`-inf` spelling. A finite `0.30000000000000004` control round-trips with
the full 17-digit spelling and assembles in both modes. `rxdas` was rebuilt
normally for this check without warnings; its build log is retained.

Passing controls include integer values/default-context large integers at
DIGITS 18, exact integer casts, decimal rounding at DIGITS 5, decimal-to-float
conversion, INT64-to-float rounding, normal-small `1e-307`, finite float
transport, ordinary infinity display/equality and ordinary NaN display.
The source audit confirms AST cloning copies typed payloads, inline metadata
uses 17-digit float text, the contract reader retains default literal strings,
and RXAS pools/deduplicates exact float bits. No additional fault was observed
at those audited boundaries; this is not exhaustive numeric proof.

## Evidence layout and limits

- `manifest.json`: owning-source/tool hashes, scope and stage command counts.
- `scan.json` and top-level `.crexx`: initial screening. Compact console output
  originally mixed assembler/compiler mode names; the full structured records
  and corrected `summary.json` are authoritative.
- `deep/`: explicit typed destinations and non-finite transport. Some numeric
  contexts initially followed ARG and were rejected by the correct ordering
  rule; those setup errors are not findings and were corrected in `context/`.
- `context/`: corrected procedure-level numeric directives. Chained constructor
  probe errors are not used as conformance evidence; supported typed-local
  forms are confirmed in `conversions/`.
- `conversions/`: supported typed float/decimal boundaries and passing controls.
- `integers/`: low-digit integer precision/overflow versus DIGITS 18 controls.
- `integer-transport/`: constants used with actual runtime integer input.
- `transport/`: RXAS/RXDAS infinity round-trip failure and finite control.
- `cast-consumer/`: ignored explicit casts in SAY/string assignments, compared
  with a correctly typed boolean destination.
- `*.rxas.txt`: selected relevant generated assembly; each stage's `scan.json`
  retains complete argv, result, stdout and stderr for every command.

All evidence is local macOS normal Debug. Existing valid original repair
regressions and ASan evidence were reused; no compiler/test implementation
changed and no broad or overnight tests were run. Normal runtime conversion
signals and intentional assembler failures are recorded as such, without
misclassifying them as sanitizer findings. These findings need local repair
plans and permanent regressions before wider optimizer-equivalence closure.
