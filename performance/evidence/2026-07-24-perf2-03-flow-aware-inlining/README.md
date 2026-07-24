# PERF2-03 flow-aware inlining decision evidence

This is the retained decision bundle for analysis and isolated prototypes at
exact `develop` commit
`086138f1e93da8e84d45f4cd3ba9b6620f792a14`. It does not contain or authorize
a production compiler change.

Contents:

- `P0-CENSUS.md`: current no-opt/optimized static and dynamic call/instruction
  census with site classifications.
- `PROTOTYPE-RESULTS.md`: independent P1-P3 and combined P4 results, focused
  correctness and exact-head AC timing.
- `PROVENANCE.md`: isolation, build identities and prototype checksums.
- `p2-timing-*.{csv,json}`: raw exact-head AC timing summary, samples and
  capture manifest.
- `production/`: accepted production-slice verdicts and raw timing summaries.
- `prototypes/`: exact patches for replay or inspection only.
- `sha256sums.txt`: bundle integrity manifest, excluding itself.

The selected design and approved production ladder are in
`performance/PERF2-03-ARCHITECTURE.md`. Approved production slices 1-4 are
complete; PERF2-03 is paused before unauthorized slice 5. Prototype patches
remain inspection evidence; only separately approved and validated mechanisms
may enter the production compiler.
