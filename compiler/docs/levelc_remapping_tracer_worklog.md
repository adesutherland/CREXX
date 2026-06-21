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
