# PERF2-02 stable-site semantic quickening PoC evidence

Status: complete bounded PoC evidence; architecture decision required; no
prototype is selected production source.

This bundle compares clean-HEAD Q0-Q7 placement controls for the accepted
Bounce reference-storage and Richards value-copy sites. Product timing uses
ordinary profiling-off Release binaries. Diagnostic profile/RXSEQ, correctness
and lifecycle results are retained separately.

## Isolation

- Accepted source commit: `d5b25a78fd6cd2b5b5962b45e508f3cb2bb782e6`.
- Q0: accepted clean PERF2-01 product binaries and images.
- Q3: zero-persistent-state current-frame local/immediate-child owner helper.
- Q4: eager purpose-built private `MKREF` handlers for proved local and
  `MINLINKATTR1` physical-child routes; canonical fallback on every miss.
- Q7: reusable module-owned record/header plus private reference and lazy
  plain-scalar COPY clients.
- Q1 Richards: compiler-only receiver capture-fold image, executed by the
  unchanged accepted Q0 VMs and library.

The exact tracked prototype diffs are under `prototypes/`. Scratch build trees
and generated timing directories are recorded in `provenance.md`; they remain
outside the main worktree.

## Evidence map

- `provenance.md`: source, patch and binary identity.
- `measurements.md`: bounded and paired timing/RSS/startup/state results.
- `correctness.md`: focused semantic, fallback, lifecycle and observability
  results; `correctness/q3b-q4-counter-diagnostics.md` and
  `correctness/q7-diagnostics.md` retain the build-private counter panels.
- `timing/`: retained compact raw/summary CSV and capture manifests.
- `manifests/`: exact workload command matrices.
- `prototypes/`: isolated clean-HEAD patch files only; not production source.

## Decision

The measured disposition is **direct value/reference helper work belongs first
in PERF2-07/PERF2-06**. The final same-guard Q3b control keeps canonical
`MKREF_REG_REG`, has no site state, and beats eager Q4 by 7.584% in `rxvm`; in
`rxbvm`, Q4's 0.243% edge is inside the paired noise band. Q3b reduces canonical
Bounce elapsed time by 80.261%/78.503% (`rxvm`/`rxbvm`).

Richards independently shows that its receiver-copy work is compiler-owned:
the Q1 capture fold reduces elapsed time by 24.305%/23.961%, while Q7 is neutral.
That is an owner-reassignment result, not a second PERF2-02 recommendation.

Q7 is not justified: it is tied with Q4 on Bounce, neutral on Richards, requests
56,264/62,536 bytes for the measured Bounce/Richards processes before allocator
overhead, and retains lifecycle gaps. See `measurements.md`, `correctness.md`
and the architecture record for the complete qualified reasoning.

No accepted PERF2-01 artifact or historical programme record is modified by
this bundle.

`checksums.sha256` closes 136 non-manifest files and verifies 136/136.
