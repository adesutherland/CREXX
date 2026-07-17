# NUMERIC-01 native numeric surface and RexxCPS typing worklist

Status: **complete; Adrian accepted candidate C and closeout passed**

This is the resumable control plane for `IDEA-NUMERIC-01`. It covers a bounded
Level B numeric-surface spike and the associated cREXX RexxCPS numeric-context
and type audit. It does not authorize production implementation before Adrian
approves the public API names and contracts below.

Correctness, Classic REXX fidelity and measured performance are separate
verdicts throughout. A function can be useful independently of timing, and a
faster RexxCPS candidate is unacceptable if it changes the intended Classic
arithmetic.

## Verified starting state

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch: clean `develop`
- HEAD: `e23d44e58ce597720bb4437b1b2ee5aef6be4eee`
- Remote relationship: `HEAD == origin/develop`; zero ahead and zero behind
- Expected commit: `e23d44e58c` is the exact HEAD and an ancestor of HEAD
- Other worktree: `/Users/adrian/.codex/worktrees/5d29/CREXX` is detached at
  `22fbf20`; the requested checkout is the only worktree holding `develop`
- Phase 1 build scope: only the four existing RexxCPS artifact targets were
  requested with `--parallel 10`; Ninja reported no work to do. No complete
  build or broad CTest was run.

## Scope and hard boundaries

- Stay within Level B unless Adrian explicitly widens scope.
- Preserve every existing decimal name and contract.
- Typed siblings must operate on their declared payload; an immediate
  conversion to `.decimal` does not fill the performance or type-fidelity gap.
- Do not add compiler overloading, syntax, VM opcodes, RXBIN changes or public
  ABI changes.
- Do not change cREXX's global 18-digit default.
- The RexxCPS candidate must explicitly select 9 digits for Classic decimal
  arithmetic. NetRexx's explicit 20-digit choice is independent and unchanged.
- Do not edit the dated programme report. It is a historical charter.
- Do not rewrite canonical or opaque RexxCPS sources until the API decision is
  approved and the candidate is being prepared as an exact-hash spike cell.
- Obey the mandatory first profiling-off Release verdict after the minimum
  focused correctness proof. Stop there for Adrian's direction before broad
  closeout validation.

## Mandatory orientation

- [x] `AGENTS.md`
- [x] `performance/AGENTS.md`
- [x] `performance/ROADMAP.md`
- [x] `performance/README.md`
- [x] Historical programme report, read without editing
- [x] `docs/ai-context/CREXX_LEVELB_AUTHORING.md`
- [x] `docs/ai-context/CREXX_LIBS.md`
- [x] `docs/books/crexx_language_reference/data_types.md`
- [x] `tests/benchmarks/README.md`
- [x] Sources, RexxDoc, Markdown and focused tests for `abs`, `min`, `max`,
  `sign`, `trunc`, `format`, `_itrunc` and `_ftrunc`
- [x] Canonical and opaque cREXX RexxCPS sources
- [x] Classic and NetRexx RexxCPS sources under `cross-runtime/rexxcps`
- [x] Current library and benchmark CMake wiring

## Phase 1 public numeric-surface inventory

The inventory below is complete for public `rxfnsb` functions whose primary
contract is a numeric value, a numeric setting, numeric/radix conversion, or
Classic numeric-text recognition. It deliberately excludes unrelated functions
that merely use `.int` for lengths, indices, statuses or handles.

Private `_rxsysb` helpers are not public inventory entries. In particular,
`_itrunc(.float) -> .string` and `_ftrunc(.float) -> .string` split the text of
a VM-formatted float around its decimal point. They are not typed Classic
`TRUNC` functions and must not dictate the public naming.

| Function | Current public contract | Classification | Typed-sibling decision |
| --- | --- | --- | --- |
| `abs` | `(.decimal) -> .decimal` | Decimal-specific Classic value surface; integer and float candidates | Add `.int` and `.float` siblings |
| `min` | `(.decimal, ... .decimal) -> .decimal` | Decimal-specific Classic value surface; integer and float candidates | Add homogeneous `.int` and `.float` siblings |
| `max` | `(.decimal, ... .decimal) -> .decimal` | Decimal-specific Classic value surface; integer and float candidates | Add homogeneous `.int` and `.float` siblings |
| `sign` | `(.decimal) -> .int` | Decimal-specific Classic input with an already-integral result; integer and float candidates | Add `.int` and `.float` input siblings, both returning `.int` |
| `trunc` | `(.decimal, fraction=.int) -> .string` | Decimal-specific Classic text surface; float candidate | Add a `.float` sibling; no `.int` sibling because an integer has no fractional payload and `intformat` covers requested fixed zero padding |
| `format` | `(.decimal, controls=.int) -> .string` | Decimal-specific Classic text surface; integer and float candidates | Add `.int` and `.float` siblings |
| `digits` | `() -> .int` | Numeric-context result already correctly integer-typed | No sibling |
| `fuzz` | `() -> .int` | Numeric-context result already correctly integer-typed | No sibling |
| `form` | `() -> .string` | Numeric-context keyword, not a numeric value | Unsuitable: the result domain is `SCIENTIFIC`/`ENGINEERING` text |
| `numcase` | `() -> .string` | Numeric-context keyword, not a numeric value | Unsuitable: the result domain is a case-setting keyword |
| `standard` | `() -> .string` | Numeric-context keyword, not a numeric value | Unsuitable: the result domain is a numeric-standard keyword |
| `random` | `(.int, .int, .int) -> .int` | Bounds, seed and result already correctly integer-typed | Unsuitable: a float/decimal sibling would define a new distribution rather than remove a crossing |
| `fnv` | `(.string) -> .int` | Modulo-2^32 hash already returned as a native integer | Unsuitable: decimal/float variants would change the hash contract rather than remove a crossing |
| `b2d` | `(.string) -> .int` | Numeric/radix conversion already returns native integer | No sibling; overflow is already bounded to signed 64-bit |
| `c2d` | `(.string) -> .int` | Character conversion already returns native integer | No sibling |
| `x2d` | `(.string, signed_length=.int) -> .int` | Numeric/radix conversion and control already integer-typed | No sibling; overflow is already explicit |
| `d2b` | `(.int) -> .string` | Numeric/radix conversion already consumes native integer | No sibling |
| `d2c` | `(.int, output_length=.int) -> .string` | Character conversion and control already integer-typed | No sibling |
| `d2x` | `(.int, output_length=.int) -> .string` | Numeric/radix conversion and control already integer-typed | No sibling |
| `b2x` | `(.string) -> .string` | Text-native radix transformation | Unsuitable: it has no numeric value boundary |
| `x2b` | `(.string) -> .string` | Text-native radix transformation | Unsuitable: it has no numeric value boundary |
| `c2x` | `(.string) -> .string` | Text-native character/radix transformation | Unsuitable: it has no numeric value boundary |
| `x2c` | `(.string) -> .string` | Text-native character/radix transformation | Unsuitable: it has no numeric value boundary |
| `datatype` | `(.string, type=.string) -> .string` | Classic lexical classifier, including `N`, not a numeric value operation | Unsuitable: typed inputs would defeat its text-classification contract |

Conclusion: Level B is not generally decimal-only. The material gap is the six
decimal value/text BIFs above, and even those do not warrant identical sibling
coverage. The proposed bounded surface is eleven functions.

The broader public library was also checked for false positives. `TIME` and
`DATE` are option-dependent information/text APIs; `FSAYFMT` is a source
generator; and string, word, array, binary, I/O and search functions use native
integer lengths, positions, counts or statuses as part of non-numeric primary
contracts. Those already-typed controls/results do not create decimal numeric
BIF families and are outside this sibling decision.

## Naming decision required

### Scheme A — full type prefix (recommended)

Use `intabs`, `floatabs`, `intmin`, `floatmin`, `intmax`, `floatmax`,
`intsign`, `floatsign`, `floattrunc`, `intformat` and `floatformat`.

Reasons:

- Type/shape prefixes already read naturally in the library (`bin*`,
  `objectarray*`) and group each typed family in indexes and searches.
- The full words remain clear at call sites without language polymorphism.
- They avoid the existing `rxmath.fabs` name.
- They are visibly distinct from private `_rxsysb._itrunc` and
  `_rxsysb._ftrunc`, whose contracts are merely formatted-float text splitting.

### Scheme B — full type suffix

Use `absint`, `absfloat`, `minint`, `minfloat`, `maxint`, `maxfloat`,
`signint`, `signfloat`, `truncfloat`, `formatint` and `formatfloat`.

This scheme is coherent and readable, but it scatters type families and is less
consistent with existing type/shape-prefixed library naming.

An exact-token scan of repository sources, tests, documentation and CMake files
found no collision for either complete scheme. Abbreviations such as `iabs`,
`fabs`, `itrunc` and `ftrunc` are intentionally rejected: `fabs` already exists
in `rxmath`, and the truncation names would be confused with the private helpers.

**Approval recommendation:** Scheme A, full type prefix.

## Proposed signature matrix

All variadic families require at least the named first argument. Variadic
arguments are homogeneous with the declared type; mixed types use the normal
typed call conversion rules rather than selecting another overload.

| Proposed function | Exact Level B signature | Native semantic contract |
| --- | --- | --- |
| `intabs` | `intabs(number=.int) -> .int` | Absolute value without decimal/float conversion; `INT64_MIN` signals `OVERFLOW_UNDERFLOW` |
| `floatabs` | `floatabs(number=.float) -> .float` | Absolute binary64 value; both signed zeros become `+0`; infinities become `+infinity`; NaN is propagated, with payload details not promised |
| `intmin` | `intmin(first=.int, ...=.int) -> .int` | Native signed-integer comparison; first occurrence wins a tie; no arithmetic overflow path |
| `floatmin` | `floatmin(first=.float, ...=.float) -> .float` | Native binary64 comparison; first occurrence wins an ordinary tie, including preservation of the first zero's sign; infinities are ordered; any NaN signals `INVALID_ARGUMENTS` |
| `intmax` | `intmax(first=.int, ...=.int) -> .int` | Native signed-integer comparison; first occurrence wins a tie; no arithmetic overflow path |
| `floatmax` | `floatmax(first=.float, ...=.float) -> .float` | Native binary64 comparison; first occurrence wins an ordinary tie, including preservation of the first zero's sign; infinities are ordered; any NaN signals `INVALID_ARGUMENTS` |
| `intsign` | `intsign(number=.int) -> .int` | Return `-1`, `0` or `1` using native integer comparison |
| `floatsign` | `floatsign(number=.float) -> .int` | Return `-1`, `0` or `1`; either signed zero returns zero, infinities return their sign, and NaN signals `INVALID_ARGUMENTS` |
| `floattrunc` | `floattrunc(number=.float, fraction=.int) -> .string`, `fraction` optional with default `0` | Render the binary64 value under the caller's numeric context, then truncate toward zero to fixed non-exponent text with exactly `fraction` digits and no decimal conversion; non-finite input signals `INVALID_ARGUMENTS` |
| `intformat` | `intformat(number=.int, before=.int, after=.int, expp=.int, expt=.int) -> .string`, all controls optional | Apply the existing Classic `FORMAT` omission, rounding, width, exponent-trigger, numeric-form and numeric-case rules to the native signed integer under the caller's numeric context, without decimal conversion; the full signed-64-bit input range, including `INT64_MIN`, is accepted |
| `floatformat` | `floatformat(number=.float, before=.int, after=.int, expp=.int, expt=.int) -> .string`, all controls optional | Apply the existing Classic `FORMAT` omission, rounding, width, exponent-trigger, numeric-form and numeric-case rules to the binary64 value rendered under the caller's numeric context, without decimal conversion; non-finite input signals `INVALID_ARGUMENTS` |

Additional contract details:

- Existing `abs`, `min`, `max`, `sign`, `trunc` and `format` names and decimal
  behavior remain unchanged.
- `intformat` and `floatformat` use the same optional-argument presence rules as
  `format`; an omitted control is distinct from an explicit zero.
- Negative fraction/width/exponent controls and a requested field that cannot
  fit signal `INVALID_ARGUMENTS`, matching the decimal text functions.
- A source argument that cannot be converted to its declared typed parameter,
  or is outside `.int` range, signals `CONVERSION_ERROR` at the typed call
  boundary. A representable float with an unsupported domain value (NaN or
  infinity where listed above) signals `INVALID_ARGUMENTS` inside the BIF.
- `floattrunc` canonicalizes exact signed zero to unsigned zero text. A negative
  nonzero value that truncates to zero retains the minus sign, matching decimal
  `TRUNC`'s sign-before-truncation behavior.
- `TRUNC` and `FORMAT` variants return ordinary `.string` values. They do not
  acquire a hidden numeric payload or context-dependent return type. Arithmetic
  callers must convert deliberately.
- The implementations may use native integer/float operations and the VM's
  numeric-context-aware integer/float extraction/string operations. They must
  not route the value through `.decimal`.

## RexxCPS numeric audit

### Source and context facts

- Classic RexxCPS 2.2 issues no `NUMERIC DIGITS`; Classic default 9 therefore
  governs its decimal arithmetic.
- The cREXX canonical and opaque ports currently issue no explicit source
  statement, and their generated procedures use `setnumdgts 18`.
- NetRexx 2.1n and 2.2n explicitly select 20 digits. That port decision remains
  untouched.
- In no-opt artifacts, `main`, `cps_subroutine`, `fail` and the two generated
  trace handlers each issue 18. Optimized artifacts retain `main` and the trace
  handlers after inlining and each issues 18.
- The candidate must explicitly select 9 in every authored procedure whose
  decimal behavior could participate, then verify generated procedure contexts.
  Generated trace handlers and imported library modules must be reported
  separately; their unrelated 18-digit contexts are not evidence that the
  benchmark workload remained at 18.

### Exhaustive numeric classification for canonical and opaque variants

| Path | Current type/behavior | Candidate classification |
| --- | --- | --- |
| `count`, `initial_count`, `averaging`, `calibrated`, `fixed_counts`, `trial`, `i`, `lvar`, `rc`, `opaque_zero`, `opaque_one` | Generated `.int` where present | Keep `.int` through declarations, literals, loop bounds, comparisons, assignments, parameters and results |
| `argv[0]`, lengths, positions and BIF controls | Existing BIF contracts/results are `.int` | Keep `.int`; no decimal/float temporaries |
| `j = 1.1 to 2.2 by 1.1` | Generated `.float`; `fgt`/`fadd`, then `ftod` for the compound comparison | Change `j`, bounds and step to `.decimal` under digits 9; this is genuine Classic decimal loop arithmetic, not an optimization opportunity |
| `acompound.*` and `avar.*` values | Stored as `.string` to model Classic compound variables; explicitly converted to `.decimal` for arithmetic | Keep string storage and decimal arithmetic under digits 9; conversions are semantic, not avoidable typing failures |
| Compound comparison and `+ 1` | Current comparison converts `j` float and compound string to decimal; increment uses decimal with an integer-to-decimal literal crossing | With decimal `j`, compare decimal-to-decimal; use a decimal unit in the genuine decimal increment path |
| `5 + 99.7`, and both compound `* 1.1` expressions | Float literal paths cross into string/decimal behavior | Make the Classic arithmetic explicitly decimal under digits 9; do not choose float because observed values happen to fit |
| `flag` | String values such as `"0"`, `"1"`, `"string"` and compound tails participate in Classic string comparisons | Keep `.string`; it is semantically not an integer flag despite its name |
| `empty`, `full`, `total`, `innertime`, `thousand`, `time("R")` conversions, elapsed and CPS divisions | Current intentional `.float` timer adaptation | May remain `.float` only as a documented cREXX port adaptation, with output/correctness equivalence and both-VM behavior proven |
| Calibration zero/one/hundred literals and `count` update | Current `trunc(1 / total)` crosses float to decimal, returns string, then crosses through float arithmetic before `ftoi` returns to `count` | Keep float only through `1.0 / total`; use `floattrunc`, then explicitly convert its text to `.int` before integer `+ 1`, multiplication and assignment |
| Timing display `format(total, , 1)` | Current float-to-decimal BIF crossing | Use `floatformat` if approved; preserve exact text contract |
| Rate display `format(1000 / thousand, , 0)` | Current integer-to-float division followed by float-to-decimal BIF crossing | Use a deliberate float numerator and `floatformat`; preserve timing adaptation and output contract |
| `j as .string`, timer/rate output and concatenation | Intentional numeric-to-text observation | Retain conversions required by output or Classic comparison semantics; list them separately from avoidable decimal crossings |

Integer-purity exceptions are therefore limited and explicit:

1. `j` and the compound workload are decimal because the benchmark intends
   Classic arbitrary-precision decimal arithmetic, even when a particular value
   is whole.
2. Compound-variable values and `flag` remain strings because Classic storage,
   tails and string comparisons are part of the simulated clause mix.
3. Timer readings and elapsed/rate arithmetic may remain float as a documented
   port adaptation; they are not integral paths.
4. Numeric-to-string conversions needed for benchmark output remain, but must
   not introduce decimal payloads when a typed text BIF exists.

### Current generated-code baseline

The current no-opt canonical `main` includes decimal `dadd=1`, `dmult=2`,
`dgt=1`; float `fadd=5`, `fdiv=5`, `fmult=1`; and conversions `ftod=6`,
`ftoi=1`, `itod=1`, `itof=12`, `stod=4`, `stof=3`, `stoi=1`. Optimized
canonical `main` still contains `ftod=4`, `ftoi=1`, `stod=4`, `stof=3` and
`stoi=1`. Opaque output has the same principal decimal BIF crossings. These are
diagnostic counts, not performance verdicts; every candidate must regenerate
and recount its own exact image.

Current exact hashes:

| Artifact | SHA-256 |
| --- | --- |
| canonical source | `91fd5380346bd2ce247b73c9373783cea9d26ee4c8cc23bce9dec6d5c15425f8` |
| opaque source | `0a0d9a09f23f60ea0e8dfd4f7d6db2ed94bba2bc8d30161f00c94f2b2619b656` |
| canonical no-opt RXAS | `e291dd05103a83c11b71a99df2732a935f48e8d4ff828e61d6fbb88f216deab8` |
| canonical optimized RXAS | `ecf4c40fc963d5be442b72d2e74110f0eba8d7f83cc418bb0788fa758e9a081c` |
| canonical no-opt RXBIN | `f1f55cbc5a45d652dc2fe64088767a533271bf014805646cbd040201a28a012d` |
| canonical optimized RXBIN | `3af6ad415dfbba2f45e1ede1e9963defc19b18a95a10a2279e1e22a3478e19a1` |
| opaque no-opt RXAS | `7afc69c39a2be47ef31f97b7d5ffaa70b3b1c3705ccd8523f954c817d57e80c3` |
| opaque optimized RXAS | `1b94410d555b7d0f55453edc90072bee1f5a02b0383bbbbf88b8749ba36f5ecf` |
| opaque no-opt RXBIN | `27ccd0222ed6d57a8dd1fa568420dbebfc14c8fe623df61e507203c903e36177` |
| opaque optimized RXBIN | `25096241d846e041167255faeddabea36c9035c564bbca95bd28ebfa6bdb7332` |

## Approval gate

- [x] Complete public numeric inventory
- [x] Two coherent naming schemes and collision/confusion audit
- [x] Recommended naming scheme
- [x] Exact signature and semantic matrix
- [x] Canonical and opaque RexxCPS type/context audit
- [x] **Adrian approved the full type-prefix names and proposed contracts**

Adrian additionally selected the RexxCPS representation rule: document cREXX
adaptations honestly; use `.decimal` for genuine Classic decimal mathematics,
keep semantically integral paths `.int` end to end, and use `.float` where it is
an appropriate fast representation such as timer/rate arithmetic. Use the
matching typed BIFs to avoid unnecessary conversions.

## Production design selection

### Status quo

Calls with native integer or float values enter the existing decimal BIFs and
cross through `.decimal`. RexxCPS timer formatting and calibration therefore
pay float-to-decimal crossings, while its current inferred float literals also
leak into arithmetic that should be Classic decimal.

### Selected approach — independent typed Level B implementations

Add explicit public Level B sources with `.int` or `.float` signatures. Use
existing native arithmetic/comparison operations plus the numeric-context-aware
integer/float extraction and string operations already available to Level B.
Reuse or factor only string-layout logic whose inputs remain text and integer
metadata. Generated RXAS must show no value conversion to `.decimal`.

This approach is selected because it closes the public type gap, preserves the
current VM/ABI/RXBIN boundaries, gives optimized and unoptimized code the same
contract, and lets RexxCPS call the representation-appropriate function.

### Rejected approaches

1. Typed wrappers that immediately convert to `.decimal`: smallest source diff,
   but preserves the performance gap and fails the native-operation contract.
2. Compiler polymorphism or implicit overload selection: wider language and
   architecture decision, unnecessary for an explicit Level B surface.
3. New VM opcodes or native public ABI entry points: disproportionate scope;
   existing operations are sufficient and must be measured first.

## Phase 2 implementation checklist

- [x] Record Adrian's exact naming/contract decision here
- [x] Implement only the selected `.int` and `.float` Level B functions
- [x] Preserve and extend source RexxDoc and per-function Markdown layout
- [x] Wire sources through existing `rxfnsb` build structure
- [x] Add focused optimized and unoptimized tests for every signature
- [x] Cover `INT64_MIN`, zero, signed zero, ordinary ties, homogeneous
  variadics, NaN, infinities, invalid controls and conversion errors as relevant
- [x] Inspect generated RXAS signatures and native operations
- [x] Prove no typed implementation hides an immediate decimal conversion

## RexxCPS candidate cells

Keep canonical and opaque candidates paired and retain each exact source/image
hash. Never mix revisions as one formal cross-runtime baseline.

- [x] **A — current cREXX RexxCPS:** retained current exact-hash control
- [x] **B — digits/type correction only:** explicit 9-digit authored contexts,
  decimal `j` and compound workload, integer-pure calibration result, no typed
  BIF substitution
- [x] **C — B plus approved typed BIF calls:** substituted only the approved
  `floattrunc`/`floatformat` calls needed by RexxCPS
- [x] **D — not required:** B/C already isolates all three BIF substitutions;
  their calibration/output placement explains the neutral timing result

For every cell:

- [x] Use quick target-only builds with deliberate high parallelism
- [x] Run only focused BIF tests and RexxCPS correctness smoke first
- [x] Record source, RXAS and RXBIN hashes
- [x] Audit every generated procedure's `setnumdgts`
- [x] Record named variable metadata and inferred temporary types
- [x] Count integer, float and decimal opcodes and all cross-type conversions
- [x] Record relevant BIF calls and confirm integer-pure paths end to end
- [x] Dynamic profile counts not required: static call placement and the B/C
  isolate explain the null BIF timing result

The full source/artifact hash matrix, reconstructing patches, generated RXAS
audit and smoke/timing proof are retained in
`performance/evidence/2026-07-17-numeric-01-first-release-verdict/`.

## Other standard cREXX benchmark numeric-purity audit

Scope: the portable standard workloads under `tests/benchmarks`, their runner,
and the performance evidence tool that measures/formats their results. Internal
microbenchmarks under `tests/performance` are excluded because some deliberately
measure decimal operations.

- [x] `awfy_sieve`, `awfy_permute`, `awfy_towers`, `awfy_storage`, `awfy_list`,
  `awfy_richards`, `json_parser` and `base64_roundtrip`: optimized and no-opt
  RXAS have zero decimal metadata, decimal opcodes and decimal BIF calls; their
  algorithms are integer/string/object workloads.
- [x] `awfy_mandelbrot`: optimized and no-opt RXAS have zero decimal metadata,
  decimal opcodes and decimal BIF calls; its non-integral mathematics is
  intentionally binary64 (`19` relevant float operations in each image).
- [x] `run_benchmarks.crexx`: timing samples and summaries remain `.int`; its
  generated optimized/no-opt RXAS has no decimal path.
- [x] `cross-runtime/lifecycle/lifecycle_probe.crexx`: no numeric workload.
- [x] `performance/tools/run_cross_runtime.crexx` and
  `performance/tools/run_lifecycle.crexx`: separate optimized/unoptimized
  compilation has zero decimal metadata, operations, conversions and decimal
  BIF calls.
- [x] `awfy_bounce`: found four calls from `.int` velocities into decimal
  `abs`, producing four decimal BIF calls plus `12` decimal instructions no-opt
  and `40` after optimized inlining. Selected correction: `intabs` at all four
  sites. Pre-correction source SHA-256 is
  `7cbb1682ea9ed55c2d866b7a5ae9d308a8cb829944f8cfc5d9b1004e30fc7de6`.
- [x] `performance/tools/run_evidence_bundle.crexx`: found two float results
  entering decimal `format`, expanding to two no-opt and seven optimized
  float-to-decimal conversions. Selected correction: `floatformat`; this is
  measurement/reporting arithmetic, not a decimal benchmark workload.
- [x] Rebuild corrected sources and prove zero decimal metadata/opcodes/calls.
  Corrected Bounce source/RXAS/RXBIN hashes are respectively
  `9d569ff3f2995dfc8408352b5b085bc0e91e828d05c99684802677c5105284f1`,
  `9fd5c72adea922ee2f519d77586e4c1334d6ed77a219078578bbfff9028f24ca`,
  `3c0f2fb527472f936fa22e5b8b5c485aa44f3cc32dc8d79f3f37a5918d5a38a5`,
  `a01fd94efbb26c724f4d13a5d85abae75fffd6075052c1adb29945945db4e80e`
  and `9cb8129f390ba82ead529d2e157ecf3eefaec7f0b1299cbfc028e17b23f2c4bc`
  for source, no-opt RXAS, optimized RXAS, no-opt RXBIN and optimized RXBIN.
- [x] Run focused optimized/no-opt correctness for Bounce and the evidence-tool
  self-test.

These corrections are separate from RexxCPS A/B/C attribution and must be
reported independently.

## Mandatory first profiling-off Release verdict

After minimum focused correctness passes:

- [x] Freeze implementation and candidate sources
- [x] Build the ordinary profiling-off Release product with high parallelism
- [x] Run the smallest decisive serial, order-balanced A/B/C comparison on
  `rxvm` and `rxbvm`
- [x] Retain raw samples
- [x] Report benchmark-native CPS separately from process wall time
- [x] Report semantic/correctness verdict independently
- [x] Report Classic-fidelity verdict independently
- [x] Report generated-code/type-conversion change
- [x] Report Release timing delta for each isolated candidate and each VM
- [x] State whether the public surface is useful independently of timing
- [x] Recommend keep, revise or revert
- [x] Stop for Adrian's direction

Verdict: focused semantics and Classic fidelity pass. B/C versus A shows a
small positive mean observation on both VMs, but overlapping ranges prevent a
decisive speedup claim. C versus B is neutral/noisy, as expected for calls
outside the default timed clause loop. Recommend keeping the native public
surface and standard-benchmark purity fixes and accepting C as the honest
RexxCPS type direction without claiming a typed-BIF timing win.

## Closeout after acceptance only

- [x] Remove disposable candidate material
- [x] Complete focused validation
- [x] Run full Debug CTest with `--parallel 30` — 1,851/1,851 pass
- [x] Run the required Release benchmark smoke
- [x] If the candidate is accepted, update canonical and opaque sources,
  RexxCPS adaptation/provenance/version wording and documentation together
- [x] Establish a new exact-hash baseline; do not merge old and new revisions
- [x] Update this worklist and the live roadmap with positive and negative data
- [x] Review per-function Markdown and RexxDoc coverage
- [x] Report final diff and validation
- [x] Do not commit or push unless Adrian explicitly requests it

## Accepted RexxCPS 2.2d baseline

Candidate C's arithmetic/type implementation is now canonical and opaque. The
2.2d version/provenance text distinguishes it from historical 2.2c and changes
the final artifact hashes without changing the accepted candidate semantics.

| Artifact/result | Final value |
| --- | --- |
| canonical source SHA-256 | `2970c3d73fe2537ec8f81295c585495c4668b442d5b9a2335b1ee453a13bbdd6` |
| opaque source SHA-256 | `72a7c2d1d284900032078f19f8b92dd1d1d852300fe628492abdec208d8260a3` |
| canonical optimized RXAS SHA-256 | `aba5b72b9e7d9654af4d3c9407a6aab936d161157061998f3eb6ee08beb49c13` |
| canonical optimized RXBIN SHA-256 | `c885dcf9d6a3818119757b9cafed892ec9dd5a9a96bd6d2dd9fcc23fc68495a9` |
| `rxvm` accepted median | 1,145,721 native CPS; 8.74 s process real |
| `rxbvm` accepted median | 1,114,685 native CPS; 8.99 s process real |

The accepted baseline used one warmup and three recorded serial
canonical-default runs per VM. Raw output, sample CSV, summary and exact binary
hashes are under
`performance/evidence/2026-07-17-numeric-01-first-release-verdict/accepted-c/`.
