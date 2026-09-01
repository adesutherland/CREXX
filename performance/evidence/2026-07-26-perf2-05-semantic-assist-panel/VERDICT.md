# Selection verdict

Status: **decision required — stopped before production**

## Recommended ladder

1. **R2a, first:** implement exact direct reference-attribute descriptor
   materialization at the earliest compiler/RXAS proof point, using canonical
   `copy_value` semantics and exact TRACE retargeting. Permit a private
   execution assist only if existing composition cannot remove the temporary
   without one. Do not copy the target object or raw target pointer.
2. **R1a, second and independently revertable:** select exact generated
   `UNLINK; LINKREF` pairs in the compiler/RXAS layer and execute them through
   a private canonical handler. Preserve unlink-before-validation state.
3. **B1a, later compiler-only investigation:** attempt scalar block-result
   register forwarding. The public `ICOPYBR` ceiling is neutral and is not a
   production candidate on this evidence.
4. **R2b, conditional:** investigate a narrower payload/descriptor helper only
   if R2a still leaves measurable canonical-copy cost and a mechanical shape
   proof covers retain/release, destination identity, status and invalidation.

R2a and R1a should remain separate production slices and receive separate
first ordinary Release verdicts. The combined PoC demonstrates compatibility,
not authorization to combine the edits.

## Placement decision recommended to Adrian

Select **multiple cooperating layers, with no new public instruction now**:

- compiler/RXAS analysis owns site eligibility, TRACE identity and fallback;
- a private VM/process instruction owns the indivisible execution only where
  it is still needed;
- canonical RXBIN remains unchanged; and
- the scratch public opcodes remain retained controls only.

This is not a recommendation for compiler-only generic object materialization,
public RXAS/RXBIN forms, runtime quickening based on unstable types, or raw
native-pointer shortcuts. R2 is a narrow weak-reference-descriptor transfer;
R1 is a narrow local-alias state transition.

## Neutral, rejected and deferred cases

- **B1 `ICOPY; BR`: neutral.** Exact instruction reduction is real, but
  ordinary Release timing is noisy and VM-dependent. Reopen only with a
  compiler result-forwarding prototype and a stable multi-workload result.
- **`ITOS; CONCAT`: not reopened.** Its apparent breadth is dominated by one
  RexxCPS workload, and the earlier exposed-temporary public form was already
  weak/neutral.
- **`DCOPY; DTOS`: deferred to PERF2-07.** It is one-module representation work
  with an open representation-validity concern.
- **adjacent `SETTP`: neutral.** No single combined semantic transition was
  proved.
- **adjacent `SCOPY`: rejected.** Independent/overlapping data movement is not
  a coherent semantic unit.
- **direct compare/branch: rejected historical control.** The larger current
  count does not invalidate NR-07's measured rejection.
- **public R1/R2/B1 instructions: not selected.** The scratch forms establish
  a ceiling but do not justify RXAS/RXBIN surface or compatibility cost.

No production source was edited, no public opcode was assigned, no full
portfolio or broad QA was run, and no commit or push was made. This package is
the mandatory Adrian selection stop.
