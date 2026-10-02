# Performance closeout plan

Status: Stages 1–4 and 6 complete; Stage 5 Apple scorecard complete with
formal Linux QA-C pending; Stage 7 cleanup under review. Refreshed 2026-09-30.

## Vision and intended outcomes

Close the completed PERF3/POSTPERF programme with one correctness-gated,
platform-labelled scorecard, honest capability/comparison boundaries, durable
accepted/rejected decisions and a compact reproducible baseline. No new
compiler, RXAS, VM, language, RXBIN, ABI or architecture programme is authorized.

The [Release 1 plan](../docs/release-1-plan.md) owns dates and product scope;
[ROADMAP.md](ROADMAP.md) owns current performance candidates. Platform features
freeze at Beta 5 (2027-01-31), optimization implementation at Beta 6
(2027-03-31), followed by April candidate qualification. The historical Apple
scorecard does not qualify those later candidates.

## Acceptance criteria

- [x] **PC-AC-01 — frozen product:** the named combined source candidate is
  `81f15918676d92a7d3e88954d94779ed759e9db8`; portfolio-v3 was reviewed and
  correctness-qualified before measurement. Evidence: the final
  [Stage 5 bundle](evidence/2026-08-18-performance-closeout-stage5/).
- [x] **PC-AC-02 — provider and concurrency decisions:** retain `mc_decimal`;
  rejected tuned-decNumber, decQuad and libmpdec outcomes and reopening triggers
  are in [DECISIONS.md](DECISIONS.md). The unchanged Mac concurrency timing
  replay is waived; prior Windows QA-D is not repeated without changed inputs
  or a concrete inconsistency.
- [x] **PC-AC-03 — Apple scorecard:** all 89 qualified cells, exact workload
  and runtime identities, serial samples, noise dispositions, separate timing,
  RSS, lifecycle and artifact results remain in the final bundle. Report genuine
  Rexx ports, adaptations and Java/CPython controls separately.
- [ ] **PC-AC-04 — Linux disposition:** complete or obtain Adrian's explicit
  disposition of formal exact-commit Linux QA-C for `81f15918676d92a7d3e88954d94779ed759e9db8`.
  Owner: upstream release maintainer. Newer unmatched Linux runs cannot silently
  satisfy this criterion. No fresh Mac timing or Windows QA-D is requested.
- [x] **PC-AC-05 — consolidated authorities:** README, RESULTS, DECISIONS,
  PERFORMANCE-GOVERNANCE and the live project/performance roadmaps own their
  respective subjects. Completed programme logs are available in Git history.
- [ ] **PC-AC-06 — compact baseline:** final manifests, source/tool/runtime
  identities, consolidated raw tables, host/build provenance, correctness
  outcomes and benchmark-source digests are sufficient without ignored or
  transient files. Check links/inventory and reproduce the retained result
  boundary from a fresh checkout before claiming overall closeout complete.
- [ ] **PC-AC-07 — weak-row dispositions:** retain bounded accept/reject/defer
  decisions for CD, DeltaBlue, Towers and Havlak under the selected Beta 6
  review queue. Storage/List ownership and NBody/Permute questions remain
  explicit product/candidate work, not promised production optimization.

## Implementation steps

1. **Stage 1 — complete** (AC-01): freeze the combined source candidate.
2. **Stage 2 — complete** (AC-02): retain current-provider measurements and
   the explicit unchanged Mac concurrency replay waiver.
3. **Stage 3 — complete** (AC-02): reject the measured decimal alternatives;
   retain the production provider and enduring reasons.
4. **Stage 4 — complete** (AC-01/03): review portfolio-v3 sources, comparability,
   optimizer resistance and correctness before timing.
5. **Stage 5 — partially complete** (AC-03/04): Apple scorecard retained;
   exact-commit Linux QA-C remains open with the release maintainer.
6. **Stage 6 — complete** (AC-05): consolidate status, results and decisions.
7. **Stage 7 — under review** (AC-05/06): remove superseded runs, generated
   build products, checksum forests, obsolete prompts and completed worklists;
   retain essential current inputs and verify the compact baseline. The 30 September source-tree cleanup has passed its
   uncommitted deletion, dependency and link review. Linux qualification and
   fresh-checkout reproduction remain separate unmet criteria.

A correctness, equivalence or capability-label defect returns the affected
cell to source review. Version any changed workload/timing boundary and rerun
only the affected scope; never pool pre- and post-revision results.

Overall closeout remains open while any criterion above is unmet. Commit,
publication and release qualification are separate authorized actions.
