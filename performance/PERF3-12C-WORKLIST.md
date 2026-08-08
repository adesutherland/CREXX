# PERF3-12C dynamic PARSE planning and hoisting worklist

Status: comparative PoC complete — recommend RXC late constant re-planning;
production design and issue-667 completion remain unapproved

Started: 2026-08-08

Purpose: qualify the new unprefixed parenthesized-variable `PARSE` delimiter
implementation now on `origin/develop`, check its claimed issue-667 coverage,
reproduce its canonical RexxCPS cost, and compare the smallest safe
compiler/assembler hoisting route with prepared dynamic-plan execution. The
canonical benchmark remains unchanged.

## Authority and boundaries

- Starting branch: `codex/peter-parse-planning-poc`.
- Starting commit: `06fe132872b1ef51409cb071cb907da9f3714632`, exactly the
  fetched `origin/develop` tip when this activity started.
- Incoming implementation commits:
  `ea3b11ef372e135a26c5c4f273e6fda6795974cb` and
  `06fe132872b1ef51409cb071cb907da9f3714632`.
- The incoming report is approximately 11.06M clauses/s before dynamic
  parenthesized templates, 3.02M after, and 12.11M after a diagnostic source
  rewrite that moved the four statements outside the hot loop. These are
  unqualified leads, not retained product evidence.
- The correct language semantics stay enabled. Do not modify canonical
  `tests/benchmarks/rexxcps_levelb.crexx`, add a reduced-support mode, or treat
  the previous unsupported lowering as a valid performance result.
- This work authorizes isolated compiler, RXAS and prepared-plan prototypes.
  It does not select a language, public RXAS/RXBIN, ABI, runtime-cache or
  production architecture change.
- Retain each negative result and stop before productionization. A selected
  production candidate requires its own focused correctness gate and mandatory
  first ordinary profiling-off Release verdict.
- Commit locally only if requested; do not push.

## Problem statement

`PARSE VAR text p1 (p0) p5` must read `p0` at execution time. The incoming
compiler exit falls back from frozen `parseplan` lowering and emits a serialized
`parseExec` plan expression. In the optimized canonical image, later RXC
constant propagation has already folded that expression to one string literal,
including `p0 = "b"`, but the earlier planner decision is not revisited.
`parseExec` then decodes the already-constant plan at every execution. Canonical
RexxCPS has four such statements in the 14-iteration `lvar` loop while `p0` is
assigned before that loop.

The investigation must separate:

1. serialization/concatenation of the dynamic plan;
2. decoding the serialized plan in `parseExec`;
3. the actual source scan and two-result transaction;
4. result/source temporary copies and grouped initialization already owned by
   the queued transactional PARSE work; and
5. loop/frame/call effects that make apparent invariance unsafe.

## Comparative designs

### S0 — current dynamic serialized-plan fallback

Keep the exact incoming implementation as the semantic and performance
control. Record generated RXAS, static/dynamic instruction counts, calls,
allocations, artifact size and ordinary Release timing under both VMs.

### H1 — RXAS loop-invariant plan construction

Hoist only the plan-construction value to the nearest proved safe preheader;
the source scan and user assignments remain at every authored `PARSE`.
Eligibility requires a reducible natural loop, a dominating definition of
every dynamic delimiter, unchanged string `ValueId` and context, no reference,
call-window, signal, TRACE or metadata observation, and a placement that does
not introduce a new allocation/signal on a zero-trip path. Reuse inside one
loop generation is a narrower fallback when preheader placement is unsafe.

The first PoC may hand-transform the exact generated RXAS as a ceiling. Any
general production consumer must use the existing graph, loop, component SSA
and sparse use services; it may not add a tactical register-number scan.

### H2 — RXC semantic invariant/specialized-plan lowering

Use compiler source/symbol knowledge to prove the dynamic delimiter value at
the parse site and emit a frozen/prepared plan or one retained plan local.
Compare this as a control because it may express source-level facts more
simply, but do not select it merely because the benchmark spelling is visible
to `rxc`. Calls, aliases, references, branches, loop zero trips, exception
timing and TRACE remain explicit proof obligations.

### P1 — prepared dynamic plan plus runtime delimiter operand

Compile the immutable template topology once and pass changing delimiter
values separately at each execution. Compare:

1. a narrow one-delimiter/two-result direct primitive as the machine-level
   ceiling;
2. a generic prepared descriptor with fixed-arity dynamic operands; and
3. the existing serialized `parseExec` path.

This route preserves truly changing `(variable)` semantics without requiring
LICM. The PoC must measure eager versus per-use preparation only where both are
plausible, and must not add an unbounded process cache or name lookup on the
success path.

### P2 — static plan text with dynamic values, still using Level B parsing

Keep `parseExec` as the executor but avoid rebuilding the whole plan string by
passing the compiled topology and dynamic value separately. This bounds how
much of S0 is serialization versus plan decoding/execution. It is a diagnostic
control, not a presumed production endpoint.

## Semantic gate

- [ ] Exact issue-667 `=(start)` / `+(length)` positives pass. **Current
      result: failed at RXC validation.** Peter's commits implement unprefixed
      dynamic delimiters, not dynamic absolute/relative positions.
- [x] Delimiter changes on every iteration remain visible in optimized and
      no-opt images under both concrete VMs.
- [x] A delimiter naming an earlier PARSE target uses the just-parsed target;
      an external variable uses its current caller value.
- [x] Empty, multibyte, repeated, absent and source-alias delimiters agree in
      the bounded semantic harness.
- [x] Existing focused tests cover repeated targets, dropped targets,
      source/result aliasing and cursor behavior.
- [ ] Zero/one/many loop iterations, conditional definitions, branches,
      calls, references, handlers and TRACE fail closed or remain equivalent.
- [ ] Allocation/failure-visible write ordering remains transactional.

## Evidence and execution stages

### Stage 0 — control plane

- [x] Read the complete mail thread and live issue context.
- [x] Fetch `origin/develop`, identify Peter's exact commits and create an
      isolated worktree/branch at the remote tip.
- [x] Re-read repository, Level B and performance controls plus NR-14 and
      PERF3-12 evidence.
- [x] Record the status quo and at least two plausible implementation routes
      before production-source experimentation.

### Stage 1 — qualification and reproduction

- [x] Build the ordinary profiling-off Release control in the isolated tree.
- [x] Run the focused parenthesized-PARSE tests and exact canonical RexxCPS
      correctness cell.
- [x] Retain generated RXAS/RXBIN hashes and map the four source statements to
      plan construction, parse execution and result assignment.
- [x] Capture schema-5 counts under both concrete VMs at equal work.
- [x] Reproduce the ordinary Release delta through same-session diagnostic
      comparisons. The pre-change control is retained only as an invalid
      unsupported lowering because it overwrites `p0`.

### Stage 2 — ceilings and comparative PoCs

- [ ] Measure a truly changing source-scan-only/direct delimiter ceiling.
      Deferred because the current ISA has no prepared dynamic operand and a
      narrow inline rewrite would not preserve the 1,000-clause contract or
      transactional semantics.
- [x] Measure P2 without plan-string reconstruction. The optimized S0 image
      already provides this control: RXC folds each complete plan to one
      literal, yet performance remains approximately 10-11M clauses/s.
- [x] Measure exact H1 hand-hoisted RXAS or equivalent instruction-faithful
      control without changing benchmark semantics.
- [ ] Measure P1 narrow prepared-dynamic execution. This needs an explicit
      internal descriptor/runtime-operand contract and remains behind Adrian's
      architecture gate.
- [x] Compare both VMs, exact work/output, startup/load, artifact size and peak
      RSS; keep raw serial samples and negative results.

### Stage 3 — ownership decision

- [x] Decide whether the material cost belongs primarily to plan construction,
      plan decoding, source scanning, result transactions or a combination.
- [x] Rank H1, H2, P1, P2 and the S0 fallback by coverage, proof complexity,
      runtime benefit and compatibility cost.
- [x] Update the live roadmap and retained evidence with a recommendation or a
      named defer.
- [x] Stop for Adrian before any production implementation or format/ABI
      selection.

## Comparative result and recommendation

- S0 optimized canonical code already contains the full constant plan
  `3,1:1;1,2:p1;2,1:b;1,2:p5;`; no concatenation remains to hoist.
- H1 hoists a strengthened call-frame setup ceiling but leaves `parseExec`.
  Eight-pair canonical medians improve only +1.33% (`rxtvm`) and +2.17%
  (`rxbvm`). This is a retained negative as the primary remedy.
- H2 exposes the constant value to the existing prepared planner. The four
  statements become `parseplan`; medians improve +314.46% and +298.03%.
- At equal 20-million-clause work, H2 reduces dynamic instructions from
  775,479,029 to 53,199,057 (-93.14%), removes 1,120,000 `parseExec` and
  6,720,000 `_stream_next_item` calls, and cuts frame activations -96.94%.
- Ownership recommendation: add a late constant-plan specialization in RXC,
  where PARSE semantics and compiler-exit provenance are owned. Do not make
  generic RXAS recognize and decode the `rxfnsb.parseexec` library protocol.
- General truly changing delimiters remain a P1 prepared-topology plus explicit
  runtime-operands design problem. No public opcode, RXBIN feature, ABI, cache
  or production implementation is selected here.
- Exact issue 667 remains a separate correctness blocker: dynamic `=`/`+`/`-`
  position controls need plan representation and tests beyond Peter's
  unprefixed dynamic-delimiter addition.

Evidence:
[`2026-08-08-perf3-12c-dynamic-parse-poc`](evidence/2026-08-08-perf3-12c-dynamic-parse-poc/).

## Exit criterion

PERF3-12C's PoC gate completes when the correct incoming implementation is
reproduced locally, the dominant cost is attributed, at least one hoisting and
one better-planning control have been compared on identical work, semantic
risks are explicit, and the roadmap contains a bounded recommendation. Useful
prototype code alone is not completion and is not production approval.
