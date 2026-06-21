# Level C Remapping Tracer Worklog

Status: active implementation log for the remapping/inlining tracer bullet.

This file records the steps, issues, and resolutions needed to replay the
tracer from clean code if the branch is rolled back. Keep entries factual and
ordered by implementation step.

## 2026-06-21: Inlining-First Tracer Start

### Goal

Implement the first remapping tracer by converting the existing inliner bucket
selection into stable remapping rule descriptors, while preserving current
inliner behaviour and tests.

The first pass deliberately avoids:

- a new external rule syntax;
- Level C compile enablement;
- semantic changes to inline clone/bind/return rewriting.

### Replay Steps

1. Add this worklog.
2. Keep `compiler/docs/levelc_remapping_target.md` as the design target.
3. In `compiler/rxcp_inline.c`, add internal inline remap rule descriptors for
   the existing call-site buckets:
   - `inline.rhs-eager-operator.capture-left`
   - `inline.expression.block`
   - `inline.assignment.whole-rhs`
   - `inline.call.statement`
4. Route `inline_procedure_walker()` through the descriptor dispatcher instead
   of open-coded bucket selection.
5. Keep the existing `ast_inline_*` builders as the semantic implementation.
6. Add debug-visible rule identities around rewrite attempts so existing
   fail-closed messages can be associated with the active remap rule.
7. Run focused inline tests and record results below.

### Issues And Resolutions

1. `16_classes_parse` golden output changed after active remap rule ids were
   added to fail-closed inline debug messages.
   - Symptom: generated output included
     `rule=inline.expression.block` on the existing `DEBUG_INLINE_FAILCLOSED`
     line.
   - Resolution: keep the rule id because tracer visibility is intentional,
     and update the single affected parsing golden line.
   - Replay note: rerun the focused parsing probe after updating
     `compiler/tests/golden/parsing/16_classes.txt`.

### Verification

Green stop for implementation stage 1:

- `cmake --build cmake-build-release --target rxc --parallel 4`
  - result: passed, no work needed after the final source state
- `ctest --test-dir cmake-build-release -R '16_classes|address_inline_then|address_exit_extended' --output-on-failure`
  - result: 3/3 passed
- `ctest --test-dir cmake-build-release -R 'inline|Inline' --output-on-failure`
  - result: 254/254 passed
- `git diff --check`
  - result: passed
- Direct trailing-whitespace check over touched source/doc/golden files
  - result: passed

### Stage 1 Result

`inline_procedure_walker()` now dispatches through internal remapping rule
descriptors for the four current top-level inline call-site buckets. Existing
`ast_inline_*` builders still own the semantic rewrites, so clone/bind/return
behaviour is unchanged.

Converted bucket rules:

- `inline.rhs-eager-operator.capture-left`
- `inline.expression.block`
- `inline.assignment.whole-rhs`
- `inline.call.statement`

Remaining tracer work:

- Lift structural eligibility into a rule/catalog form.
- Decide whether clone/bind/return services should become named remap
  subrules or remain services used by top-level rules.

## 2026-06-21: Structural Eligibility Rule Identity

### Goal

Lift the structural inlinability decision into the same remapping vocabulary as
the call-site buckets, without changing eligibility semantics or the existing
`DEBUG_INLINE` acceptance/rejection text.

### Replay Steps

1. Add the internal rule descriptor `inline.eligibility.structural`.
2. Keep `inline_analyse_callable_eligibility()` as the semantic gate.
3. Emit `DEBUG_INLINE_REMAP` accept/reject lines at `-d2` and higher using the
   new structural rule descriptor.
4. Keep normal parser goldens stable by not adding structural rule ids to the
   existing `DEBUG_INLINE` lines.

### Issues And Resolutions

No Stage 2 implementation issues recorded.

### Verification

Green stop for implementation stage 2:

- `cmake --build cmake-build-release --target rxc --parallel 4`
  - result: passed
- `ctest --test-dir cmake-build-release -R '16_classes|address_inline_then|address_exit_extended' --output-on-failure`
  - result: 3/3 passed
- `ctest --test-dir cmake-build-release -R 'inline|Inline' --output-on-failure`
  - result: 254/254 passed
- `cmake-build-release/bin/rxc -i cmake-build-release/bin -d2 -o /tmp/inline_remap_debug compiler/tests/rexx_src/inline_test_expr.crexx`
  - result: passed
  - observed `DEBUG_INLINE_REMAP` lines for
    `inline.eligibility.structural` and `inline.expression.block`
- `git diff --check`
  - result: passed
- Direct trailing-whitespace check over touched source files
  - result: passed

### Stage 2 Result

Structural eligibility is now named in the remapping tracer as
`inline.eligibility.structural`. The current eligibility code remains the
source of truth; the remap layer records the decision instead of reimplementing
it.

Remaining tracer work:

- Decide whether clone/bind/return services should become named remap
  subrules or remain services used by top-level rules.

## 2026-06-21: Bind Actuals Service Boundary

### Goal

Expose the shared actual-binding service as a named remap boundary while
leaving the existing binding implementation intact. This makes the tracer cover
the call-site selector, structural eligibility, and the first shared expansion
service without converting clone/bind/return internals into a new engine.

### Decision

`inline.bind.actuals` is represented as a remap service boundary. The bind
implementation remains the existing `inline_bind_call_arguments()` function.
This keeps by-value, optional/default, by-reference, vararg, method receiver,
and factory binding semantics unchanged.

Return rewriting, body cloning, and receiver copyback remain implementation
services for now. They should become separate remap subrules only when a later
change needs rule-specific guards, tests, or debug reasons inside those
services.

### Replay Steps

1. Add the internal rule descriptor `inline.bind.actuals`.
2. Use the existing `INLINE_BIND_RETURN()` macro in
   `inline_bind_call_arguments()` to emit `DEBUG_INLINE_REMAP` applied/rejected
   results at the service boundary.
3. Do not change any binding branches or return values.
4. Keep existing focused inline tests green.

### Issues And Resolutions

No Stage 3 implementation issues recorded.

### Verification

Green stop for implementation stage 3:

- `cmake --build cmake-build-release --target rxc --parallel 4`
  - result: passed
- `ctest --test-dir cmake-build-release -R '16_classes|address_inline_then|address_exit_extended' --output-on-failure`
  - result: 3/3 passed
- `ctest --test-dir cmake-build-release -R 'inline|Inline' --output-on-failure`
  - result: 254/254 passed
- `cmake-build-release/bin/rxc -i cmake-build-release/bin -d2 -o /tmp/inline_remap_debug compiler/tests/rexx_src/inline_test_expr.crexx`
  - result: passed
  - observed `DEBUG_INLINE_REMAP` lines for
    `inline.eligibility.structural`, `inline.bind.actuals`, and
    `inline.expression.block`
- `git diff --check`
  - result: passed
- Direct trailing-whitespace check over touched source files
  - result: passed

### Stage 3 Result

The current tracer now names:

- structural inlinability: `inline.eligibility.structural`
- call-site bucket rewrites:
  - `inline.rhs-eager-operator.capture-left`
  - `inline.expression.block`
  - `inline.assignment.whole-rhs`
  - `inline.call.statement`
- shared actual binding: `inline.bind.actuals`

That covers the current inliner rule set at the safe tracer boundary. Deeper
clone/return/copyback subrules remain deliberately unconverted services until
there is a concrete need to split their internal guard logic.
