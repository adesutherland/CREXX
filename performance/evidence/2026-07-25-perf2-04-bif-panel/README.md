# PERF2-04 current BIF census and bounded design panel

Date: 2026-07-25

Repository state: `develop` at
`6567f0ba23f20623e01322f5a62323b2347ab09d`, the documentation-only
PERF2-03 closeout whose accepted production parent is
`d1c5245d49c0bd9cc48a7d33ef16f2f4555cc986`.

Publication status: the decision package remains the retained design baseline.
Adrian accepted P04-CAS1, P04-SLC1, P04-CEX1 and P04-WRD1. The bounded broad
closeout is green and the accepted production ladder is local commit
`f8f34092e`; no push is authorized. No public instruction, serialized format,
ABI, VM or native BIF is changed.

## Decision snapshot

The current product has already removed the ordinary Level B call boundary for
the timed `LENGTH`, `SUBSTR` and `WORD` sites in RexxCPS and for every seed BIF
site in Base64. The important remaining costs are therefore not one generic
"make all BIFs native" problem:

1. `UPPER` is the strongest current PERF2-04 production candidate. Its exact
   Level B body is inlineable for a direct symbol actual, but all four hot
   RexxCPS calls pass literals or a computed expression and fail
   `inline.bind.actuals`. Materialising symbol actuals removes the calls;
   composing the existing `strupper` directly into the result is better; and
   all four exact arguments are already compiler constants, so a trace-correct
   constant fold is the machine ceiling.
2. The timed `LENGTH`, `SUBSTR` and `WORD` bodies already inline. Their remaining
   ceilings test post-inline result placement and constant/value composition,
   not a missing public BIF opcode.
   `LENGTH` removes exactly 84 instructions per RexxCPS iteration in its hand
   control, but the dual-VM ordinary Release verdict is neutral, so that
   PERF2-03-F03 case does not advance.
   `SUBSTR` is different: direct existing-primitive composition is
   +8.67%/+6.89%, and its exact constant ceiling is +11.55%/+8.08% on
   `rxvm`/`rxbvm`.
   `WORD`'s exact consumer predicate is +3.23%/+3.32%, while propagated
   constant-false evaluation reaches +6.71%/+6.81%.
3. Base64's old `SETSTRPOS`/copy attribution is current and material, but the
   wrapper BIFs are not the owner. A direct Level B codepoint-arithmetic
   algorithm removes about 2.9 million copies in the bounded control and is
   12.19x (`rxvm`) / 10.56x (`rxbvm`) faster than the current algorithm. One
   benchmark site does not justify a new assist, and CAP-03 requires the common
   benchmark to remain unchanged. The finding belongs to a separately proved
   Level B Base64 API, not a PERF2-04 production slice.
4. `POS` already composes to the existing `strpos` primitive at the relevant
   sites. `LEFT` is reporting-only in RexxCPS; `WORDS`, `WORDPOS`, `RIGHT`,
   `LASTPOS` and `LOWER` are absent from the current Tier A timed product.
5. No Level B `DATATYPE` or typed-conversion BIF is dynamically material in the
   optimized Tier A portfolio. RexxCPS's hot `ITOS`, `DTOS` and `STOD` events
   are language/value conversions and remain PERF2-07 evidence.
6. LEN-H1 also exposed one real, non-performance correctness case. An
   empty-initialized string updated by the valid `dcopy; dtos` sequence keeps a
   stale Unicode codepoint count, so both VMs return length zero for `"2.2"`.
   The exact known-failing regression is retained and opened only as
   `PERF2-07-V3-R01`; PERF2-04 installs no fix.

The direct cumulative control combines only the selected `UPPER`, `SUBSTR` and
`WORD` exact ceilings. It reduces normalized instructions by 46.52% on both
VMs and improves ordinary Release RexxCPS by 32.88% (`rxvm`) and 34.40%
(`rxbvm`). All 28 recorded executions pass and the candidate wins every paired
round. This supports the complete three-slice compiler ladder; it remains a
guard-heavy TRACE-off scratch ceiling, not a production implementation.

The recommended production ladder and exact selected/rejected ownership are in
[`DESIGN-PANEL.md`](DESIGN-PANEL.md). P04-SLC1's first verdict is retained
under `measurements/p04-slc1-release-verdict/`. Its general certified-call
registry initially folds constant `UPPER` and the proved positive/in-range
`SUBSTR` domain. Against accepted P04-CAS1 it removes 102 executable RXAS
instructions and improves ordinary Release RexxCPS by
14.436115%/14.060120% on `rxvm`/`rxbvm`; all 18 invocations pass. This
evidence supported acceptance of P04-SLC1.

P04-CEX1 extends that registry with exact `LOWER`, `LENGTH`, `LEFT` and `RIGHT`
certificates. Its focused constant-use cell removes 93.007% of executable RXAS
and 85.309% of RXBIN bytes, and the repeated ordinary Release cell is
21.094049x/21.934257x faster on `rxvm`/`rxbvm`. Current RexxCPS, linked library
and rxpp artifacts remain byte-identical, so this is a general constant-use
claim rather than a current Tier A claim. Raw evidence is retained under
`measurements/p04-cex1-release-verdict/`; Adrian accepted it before P04-WRD1.

P04-WRD1 adds an exact constant `WORD` certificate and general pre/post
certified-call constant propagation. It removes the complete timed
`word("Key Bee", 1) = "?"` scan/slice/consumer path while retaining the
dynamic setup-only `word(version_info, 1)` call. Whole-module/main executable
RXAS falls by 68, RXBIN falls by 3,660 bytes, and ordinary Release RexxCPS
improves by 7.650234%/8.812155% on `rxvm`/`rxbvm`. Raw evidence is retained
under `measurements/p04-wrd1-release-verdict/`; Adrian accepted it on
2026-07-26 and the completed broad correctness record is in
[`CLOSEOUT.md`](CLOSEOUT.md).

## Evidence map

- [`CURRENT-BIF-CENSUS.md`](CURRENT-BIF-CENSUS.md) explains the complete
  560-callable Level B surface, seed-family artifacts, current workload sites,
  inlining results and ranked dispositions.
- [`SEMANTIC-MATRIX.md`](SEMANTIC-MATRIX.md) fixes the family contracts and the
  proof boundary of each scratch ceiling.
- [`DESIGN-PANEL.md`](DESIGN-PANEL.md) compares clean Level B, hand-equivalent,
  best Level B algorithm, compiler composition, general assist and native
  controls and assigns the most efficient correct owner.
- [`CLOSEOUT.md`](CLOSEOUT.md) records the accepted production identity, broad
  Debug verdict, generated-code review, product hashes and evidence replay.
- `census/` contains the complete exported-signature and 142-module artifact
  inventories, seed-family static tables, exact selected workload sites and
  the machine-readable ranked dispositions.
- `profiles/current-rxvm/` and `profiles/current-rxbvm/` are independently
  verified 22-entry optimized/no-opt current-product census bundles.
- `profiles/current-selected-timing/` retains profiler-timing attribution for
  RexxCPS and Base64; it is not throughput evidence.
- `controls/` contains maintained Level B PoC sources and exact timing
  manifests. The C Base64 control is explicitly an attribution-only native
  ceiling.
- `pocs/` contains separable scratch patches against the exact HEAD.
- `measurements/` contains the machine-readable six-role candidate panel,
  static summaries, normalized current-profile counts and ordinary
  profiling-off Release wall-clock matrices.
- `correctness/` retains 72 focused product CTests and 32 dual-VM optimized/
  no-opt control checks.

## Measurement boundary

Profiler data is used only for attribution. Production-facing PoC comparisons
use ordinary `-O3 -DNDEBUG`, profiling-off Release `rxvm` and `rxbvm`, run
serially and correctness-qualified. The current all-workload census has one
bounded sample per optimized/no-opt image; it is not a formal portfolio verdict.
No full formal portfolio was run.

The Base64 wall control deliberately runs a fixed valid-input contract. The C
control proves only a native upper bound and cannot select native ownership.
The RexxCPS source-rewrite ceilings preserve TRACE-off output and add explicit
post-timer semantic assertions, but cannot reproduce an original BIF function
TRACE event. A production compiler candidate must preserve source/TRACE
identity or fail closed; the scratch speedups alone do not waive that gate.
