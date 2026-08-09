# PERF3-12D exit-owned compiled pattern processing plan

**Status:** complete — feature commit `ef6e3fd77` integrated with `develop`;
direct reuse plus a semantically guarded `link` fallback is publication-ready

## Objective

Make `PARSE` the first frontend for an exit-owned compiled pattern-processing
substrate that can later support regular expressions, PEGs and other packed
language processors. The compiler exit continues to own language recognition,
semantic validation, binding declaration and lowering. Runtime text-template
parsing is a compatibility fallback rather than the normal compiled form.

The opening PoC must use the facilities already in the product before proposing
new instructions:

- typed compiler-exit bindings and replacement lines;
- ordinary integer, branch and assignment operations;
- `strlen`, `strpos`, `substring`, word-scan and existing string instructions;
- `.binary` values, read-only binary constants and little-endian `bget*`
  operations for packed descriptors; and
- the existing RXC and RXAS optimisation pipelines.

A new generic instruction is eligible for a later proposal only when retained
evidence identifies a recurring operation that existing instructions express
with materially excessive dynamic work, allocation, code size or semantic
awkwardness. PARSE-only opcode names are not the default answer.

## Architectural boundary

### Frontend ownership

The certified `PARSE` compiler exit remains the owner of:

1. token and template recognition;
2. contextual keyword claims and synthetic bindings in `pre_process()`;
3. Classic PARSE evaluation, cursor and assignment order;
4. compilation to the internal Pattern Program; and
5. selection of an expansion, packed-program or compatibility backend.

Future regex and PEG exits or functions may compile to the same low-level
substrate, but they retain their own matching semantics. Sharing cursor, search,
span, capture and checkpoint operations must not flatten deterministic PARSE,
regex alternatives/repetition and PEG ordered choice into one source language.

### Pattern Program

The target internal representation is a versioned, endian-stable packed program
that is suitable for a `.binary` constant. The exact format remains private
during this activity. The design envelope is:

- a fixed header containing magic, version, flags, operation count, capture
  count and literal-table size;
- size-tagged operation records so unknown versions or operations fail closed;
- operand sources tagged as immediate, literal-table span, external typed
  value, or completed capture/result index;
- codepoint cursor units for Level B text and byte units only for explicitly
  binary languages;
- capture spans represented as start/end pairs until materialisation is
  semantically required; and
- no embedded source variable names on the runtime success path.

The packed form is a compiler/backend boundary, not a requirement that every
compiled pattern be interpreted. Small deterministic programs should expand to
ordinary instructions. Larger regex/PEG programs may justify a compact executor
when expansion loses on code size or control-state cost.

## Semantic invariants

Every backend must preserve:

1. a stable source snapshot when source and targets alias;
2. trigger evaluation exactly once and in authored order;
3. assignment visibility between triggers, including reuse of a target name;
4. the most recently completed assignment for a later `(name)` reference;
5. Classic absolute and relative cursor clipping and anchor rules;
6. empty, absent, repeated and multibyte delimiter behaviour;
7. TRACE/source mapping and diagnostic provenance; and
8. the selected failure-before-write or ordered-write contract.

The final item is an explicit approval decision. Regina and ooRexx assign
targets as triggers complete. Current cREXX generic lowering first constructs a
result vector. The PoC may compare both, but it may not silently change public
failure visibility.

## Backend panel

### E0 — former compatibility fallback (retired)

`parseExec` supplied the initial correctness oracle while the compiled forms
were proved. The approved production implementation retires that textual
runtime-plan path; retained evidence remains the comparison control, while all
supported forms now lower through exit-compiled inline or packed plans.

### E1 — existing-instruction expansion

The compiler exit emits normal typed source and inline assembler using existing
string/integer instructions. The first bounded shapes are:

1. two captures separated by a runtime `(identifier)` delimiter; and
2. the issue-667 three-capture `=(identifier)` then `+(identifier)` form.

The generated sequence snapshots the source, evaluates each runtime operand at
its trigger point, scans or moves the cursor, extracts spans and performs target
assignments in the proved order. Constant propagation is permitted but is not
an eligibility condition or performance assumption.

### E2 — packed-program backend

Reuse `.binary` constants and the existing binary-memory surface to retain a
private Pattern Program in the target. This candidate is opened only after E1
establishes where expansion becomes too large or repetitive. A future executor
must accept typed operands/capture slots rather than rebuilding a textual plan.

### E3 — evidence-backed generic instruction

Nominate an instruction only when E1/E2 evidence proves a reusable missing
operation. Candidate families include span materialisation, capture commit,
character-class scan or checkpoint/restore. Any proposal must specify generic
semantics, operand forms, signal timing, alias behaviour, optimiser metadata,
portable and threaded VM handling, disassembly, feature gating and tests.

## Approved production design selection

Adrian approved full implementation on 2026-08-08, selected retirement of the
textual `parseExec` execution path, and asked that common deterministic forms be
inlined unless retained evidence proves a better compact form. The selected
division of responsibility is:

1. the certified compiler exit remains the language compiler and sole owner of
   trigger ordering, captures, cursor rules, typed bindings and backend choice;
2. small deterministic programs and runtime-operand common patterns expand to
   normal typed/assembler instructions so RXC can fold, propagate and inline
   them and RXAS sees an ordinary optimizable instruction stream;
3. arbitrary frozen topology may use the existing exit-compiled packed
   `parseplan` descriptor where full expansion loses materially on artifact
   size; and
4. the old runtime `kind,length:text;` construction/decoding protocol is retired
   from generated PARSE code. Remaining logging, `INTO`, signed-relative and
   general dynamic-operand cases move to compiled forms in gated slices.

The status quo and rejected alternatives are retained explicitly:

- **S0, retain `parseExec`: rejected as the production architecture.** Even
  after RXC folds the whole textual plan to a literal, runtime stream decoding,
  calls, result-vector setup and allocations remain. It is only a provisional
  compatibility oracle while the remaining semantic matrix moves to compiled
  forms.
- **S1, inline every template without a size policy: rejected.** The opening
  two-capture microkernel grew RXBIN 22.37% and RXAS 41.29%. Small expansions
  are selected, but arbitrary regex/PEG-scale control programs need an explicit
  expansion/packed crossover rather than unconditional duplication.
- **S2, compiled hybrid: selected.** Inline proved common forms and use an
  exit-compiled packed program for topology whose expansion cost crosses the
  retained threshold. Neither backend reparses source-language syntax at
  runtime.
- **S3, general RXAS folding/hoisting: selected as the residual optimization
  owner, but not as a prerequisite for E1.** RXAS owns dominator, loop, SSA and
  preheader-eligibility facts, but production currently performs no general
  LICM or constant-load preheader movement. After the E1 first Release verdict,
  extend those general proofs rather than adding PARSE-specific replanning or
  hoisting to RXC. Source and TRACE metadata constrain motion rather than
  granting it; zero-trip signal/allocation timing and register/value identity
  remain fail-closed gates. The consumer must benefit any qualifying instruction
  stream, including future regex/PEG lowering, not recognize PARSE protocol.
- **S4, add a new generic pattern opcode now: not selected.** E1 proves the
  current instructions are sufficient for the opening hot forms. A new generic
  character/capture/checkpoint operation requires a retained residual sequence,
  dynamic/count and size evidence, complete effect/signal metadata, and a
  separate ISA/RXBIN decision.

### Optimizer ownership finding

RXC optimization runs after compiler-exit expansion. Its constant propagation
therefore sees exit-generated AST and assembler operands. Before substituting a
constant into an `ASSEMBLER` statement it checks the RXAS instruction database
and permits the substitution only when a legal encoded operand form exists.
In the E1 canonical RexxCPS artifact RXC proves `p0` is the constant `"b"`, but
`strpos` accepts only register operands, so each site contains a literal load
followed by `strpos`; RXC does not currently common or hoist those loads.

RXAS does not currently repair that shape: its graph layer records loop and
preheader facts, including candidate-specific eligibility proofs, but the
production routes deliberately perform no general preheader movement. The
primary PARSE win comes from exit-owned compiled expansion. Residual constant
load/common-expression folding and invariant motion belong in a general RXAS
consumer; no PARSE-specific RXC optimizer case is selected.

### S3 first consumer: dominated string-literal reuse

The first general RXAS consumer is deliberately narrower than preheader LICM.
It recognizes repeated `load rN,"literal"` materializations only when all
qualifying sites are inside one reducible natural loop and an earlier load
dominates the later load. It retains the first materialization at its authored
location, redirects proved exact-string uses of later equivalent values to the
dominating register, and deletes the redundant later loads. TRACE reads are
retargeted to the dominating register rather than removed, preserving their
authored ordering, names and values.

The selected proof is fail-closed:

1. both destinations are private, unaliased local storages;
2. the earlier string component is still the same literal at the later site;
3. both sites belong to the same natural loop and dominance is explicit;
4. every later-value observation is an exact string read that can be rewritten,
   or an owned TRACE record that can be deleted;
5. calls, metadata observations, opaque accesses, read/write operands,
   reference/native cleanup, branch ambiguity and irreducible control flow
   reject the candidate; and
6. no instruction is moved across a zero-trip edge or an allocation/signal
   boundary.

Alternatives considered for this slice are:

- extending M02 only to same-storage string loads, which cannot address RXC's
  distinct temporary registers and therefore misses the retained RexxCPS
  residual;
- full load movement into a loop preheader, which changes the timing of string
  materialization and TRACE observations and is deferred until a separate
  zero-trip/motion proof exists; and
- a PARSE-aware RXC or RXAS special case, rejected because ordinary string
  consumers and future regex/PEG lowering need the same optimization.

The NetRexx 5.10-GA generated RexxCPS Java provides a useful cross-check. Its
PARSE programs are `private static final char[]` constants, dynamic delimiters
are supplied through prefilled result-array slots, and `RexxParse.parse()`
consumes the compiled program directly. The authored `p0='b'` assignment stays
inside the kernel iteration and the resulting value is reused by four parse
calls. This supports stable plan hoisting and dominated value reuse, but not
speculative movement of the authored assignment outside its loop.

### Mandatory first production gate

The first selected production slice is the already measured E1 lowering for a
runtime delimiter and the issue-667 `=(start)` / `+(span)` form. After the
minimum focused exit/runtime correctness checks pass, implementation freezes;
the ordinary profiling-off Release RexxCPS comparison is run and reported to
Adrian. Broader `parseExec` retirement, the general RXAS folding/hoisting
consumer, representation tuning, full validation, documentation closeout and
any new-instruction PoC remain behind acceptance of that first verdict.

## Opening PoC gates

- [x] The exit emits E1 using existing instructions; no new opcode is added.
- [x] A genuinely runtime delimiter works in optimized and no-opt images under
      `rxtvm` and `rxbvm`.
- [x] Exact `=(start)` / `+(length)` issue-667 syntax compiles and runs.
- [x] A target reused before `(target)` resolves its latest completed value.
- [x] Empty, absent, multibyte and source-alias delimiter cases agree with the
      compatibility control.
- [x] Generated RXAS proves that `parseExec`, stream decoding and textual plan
      construction are absent from eligible E1 sites.
- [x] Static instructions, dynamic instructions, artifact size and ordinary
      profiling-off Release elapsed/CPS are recorded for E0 and E1.
- [x] Any suggested new instruction is supported by an exact residual sequence
      and measured cost; otherwise the result explicitly says none is needed.

## Opening PoC result

The bounded E1 implementation is complete for review. It adds no instruction
and no public format. The compiler exit recognizes only the two approved shapes.
Other supported dynamic-delimiter templates retain E0; unproved dynamic-position
shapes fail compiler validation. Eligible sites emit `strpos`, `strlen`,
`substring`, integer arithmetic, branches and assignments.

At one million runtime-delimiter parses, eight interleaved profiling-off
Release samples produce these medians:

| VM | E0 serialized plan | E1 existing instructions | elapsed reduction |
| --- | ---: | ---: | ---: |
| `rxtvm` | 1.375 s | 0.085 s | 93.82% (16.18x) |
| `rxbvm` | 1.315 s | 0.080 s | 93.92% (16.44x) |

The unchanged canonical RexxCPS source was then compiled directly with the E1
exit into a fresh artifact. After one discarded warm-up, eight profiling-off
Release runs per VM produced:

| VM | retained E0 median CPS | E1 median CPS | ratio |
| --- | ---: | ---: | ---: |
| `rxtvm` | 10,039,724.5 | 43,505,413.5 | 4.33x (+333.33%) |
| `rxbvm` | 10,877,602.0 | 45,972,653.0 | 4.23x (+322.64%) |

All 16 recorded E1 processes used the canonical-default contract, calibrated to
`effective_count=500` and printed `PASS: RexxCPS 2.2d cREXX port`. Generated
RXAS contains four `strpos` expansions and no `parseExec` call or `parseplan`
instruction. This is a valid PoC score, not a first Release production verdict.

At 100,000 parses, counts-only profiling is identical between concrete VMs:
67,700,030 dynamic instructions in E0 versus 3,400,030 in E1, a 94.98%
reduction. E0 also activates/reuses 800,001/799,997 frames and allocates
600,034 string buffers; E1 uses one frame activation and 14 string buffers for
the complete process. The benchmark RXBIN grows from 5,682 to 6,953 bytes
(+22.37%), and RXAS grows from 11,578 to 16,359 bytes (+41.29%).

This proves that the current instruction set is already sufficient and fast
for the opening deterministic forms. It does **not** prove a new instruction is
needed. The residual concern is expansion size, not execution cost. E2 should
therefore be a separate template-complexity crossover PoC using packed binary
constants. E3 remains closed until that comparison isolates a recurring fused
operation whose dynamic or size cost cannot be solved by existing instructions
and ordinary optimization.

Retained evidence:
[`2026-08-08-perf3-12d-existing-instruction-poc`](evidence/2026-08-08-perf3-12d-existing-instruction-poc/).

## Generated-binding register-reuse verdict

The first H02 form preserved compiler-generated pattern-register metadata by
replacing three repeated one-character literal loads with `scopy`. A bounded
follow-up compared that form with `link`, then corrected the underlying
contract: exit-created PARSE temporaries are internal assignments and must not
emit public variable `.meta` records.

The original no-H02/H02 comparison crossed rebuilt VMs and libraries; its
-2.198028%/-2.618265% result is retracted. Adrian requested a complete rerun.
One frozen profiling-off Release VM/library product now crosses the two exact
RXC outputs with old no-H02 RXAS and new H02 RXAS, plus a matched `scopy`
control. The 40-round five-cell matrix records 400 passing processes after two
warmups per cell and VM. It confirms that metadata suppression alone is
runtime-neutral and that old RXAS leaves the same four executable loads with
or without those records.

With PARSE-generated bindings declared `internal`, RXC emits no variable
metadata for `__rxcpx_*`; RXAS redirects all four pattern consumers to the
dominating register. The final image has one `load r68,"b"` and no associated
`load`, `scopy`, or `link` for `r69`-`r71`. It executes 31,240,346 instructions
versus 31,660,346 for either dispatch-preserving form (-420,000, -1.326581%)
and shrinks 76,033 -> 73,161 bytes (-3.777307%).

Against the matching metadata-free old-RXAS control, direct reuse improves
median CPS by +0.571154%/+0.595225% on `rxtvm`/`rxbvm`. Against the original
metadata-retaining old-RXAS form, the complete metadata plus RXAS change is
+0.436060%/+0.146506%. These are small positive tendencies; only the isolated
`rxbvm` direct-reuse paired interval excludes zero.

The expanded fallback comparison does not prove `link` faster than `scopy`.
The dedicated 80-pair ratio-of-medians result is +0.305177%/+0.085937%, but
paired geometric changes are +0.227154%/-0.129669% with both 95% intervals
crossing zero and only 49/80 and 40/80 `link`-favourable pairs. The combined
120-pair sensitivity result remains mixed. All 720 newly recorded matrix and
dedicated-panel processes pass the canonical calibrated contract.

Adrian locked the production disposition on 2026-08-08: keep the
type-independent internal-binding metadata contract and direct reuse for
exit-generated temporaries. Prefer `link` over `scopy` when metadata requires
an exact destination register, because avoiding a payload copy should scale
better for large strings and later binary/object values. RXAS must first prove
that the persistent alias is safe for both bindings' full lifetime; otherwise
it retains the original materialisation. The RexxCPS panel does not prove that
`link` is faster than `scopy` for its current small-string workload. Closeout
QA is complete. Adrian separately authorized a direct `develop` merge and push.

Retained evidence:
[`2026-08-08-perf3-12d-register-reuse-verdict`](evidence/2026-08-08-perf3-12d-register-reuse-verdict/).

## Closeout QA result

The selected implementation passed the required broad closeout on 2026-08-08:

- the fresh profiling-off Debug product built completely;
- the affected Debug and ordinary profiling-off Release matrices each passed
  14/14 tests, covering the compiled/no-opt PARSE forms, generated-image
  fixtures, frozen RXBIN contract, flow graph, opcode metadata and string
  literal reuse;
- the safe metadata-visible case emits exactly one `link r2,r1`, while a later
  independent write to either binding retains both loads and reports
  `alias-lifetime-unsafe` in optimizer diagnostics;
- full Debug CTest passed 2,000/2,000 tests in 216.15 seconds at
  `--parallel 30`; and
- a fresh canonical RexxCPS compile retained no `__rxcpx_` variable metadata;
  its optimized disassembly contains one `load r68,"b"`, no replacement
  dispatch for `r69`-`r71`, and all four `strpos` sites consume `r68`.

The isolated implementation was committed as `ef6e3fd77`. Develop integration
also retained the existing no-per-record-allocation queue-snapshot design from
`6cde2c509`: inline snapshots are re-pinned after sparse-index growth. The
PARSE branch's alternative heap-stable snapshot representation was not carried
forward. The combined build initially exposed an invalid mixed representation
while assembling `rxjson`; correcting the merge to the established inline and
re-pin contract removed that failure. Final integrated qualification passed:

- complete profiling-off Debug and Release builds;
- the original optimized `rxjson` assembly reproducer;
- 4/4 focused RXAS flow/metadata/literal-reuse tests;
- 20/20 focused Release PARSE, RXAS and RXQUEUE integration tests; and
- full Debug CTest at 2,002/2,002 in 216.57 seconds with `--parallel 30`.

The integrated logs are retained in the register-verdict evidence under
`qa-integration/`. No sanitizer, install/package, cross-platform or additional
benchmark sweep was added beyond the approved closeout and merge path.

## PoC sequence

1. Preserve current E0 binaries, RXAS and semantic output before editing.
2. Add fail-closed E1 recognition and bindings in the existing PARSE exit.
3. Emit only existing instructions for the two bounded shapes.
4. Run direct exit-protocol tests before runtime tests.
5. Run the semantic matrix on both concrete VMs in optimized and no-opt modes.
6. Compare a runtime-unknown delimiter microkernel at equal work.
7. Inspect generated RXAS and dynamic profiles for residual cost.
8. Stop for Adrian with the design, diff, evidence and any E2/E3 recommendation.

## Approval boundary

Production implementation of the compiled hybrid and retirement of generated
`parseExec` use are approved. The corrected mandatory first Release verdict has
now run: direct reuse has a small positive tendency, metadata suppression is
neutral, and RexxCPS does not prove `link` faster than `scopy`. Adrian selected
direct reuse plus a fail-closed, lifetime-proved `link` fallback on 2026-08-08
because avoiding payload copies should scale better for large strings and
later binary/object values. Closeout and develop-integration QA are complete.
Adrian separately authorized the direct `develop` commit and push. No new
public Pattern Program format, RXAS/RXBIN instruction, or regex/PEG syntax is
authorized by this activity.
