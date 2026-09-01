# PERF2-06/07 V1R01-R1 first Release verdict

This bundle retains the reworked compiler-owned direct-placement candidate and
its mandatory first ordinary profiling-off Release verdict. `VERDICT.md` is the
decision summary. **ACCEPT is recommended**, but the candidate remains isolated,
provisional and uncommitted pending Adrian.

The proof boundary is semantic rather than benchmark-shaped: direct nested
`§this` placement is allowed through arbitrary internal branches and calls
when the reconstructed method has no more than one explicit return; fallthrough
is included. Multiple explicit returns retain materialisation until a future
summary can prove receiver-owned link balance on every exit. Already-proved
receiver aliases are preserved when a containing inline body is cloned again.

Evidence map:

- `VERDICT.md`: capped 36-pair result, guard scorecard and stop boundary;
- `exact-count-summary.csv` and `counts/`: exact dual-VM operation, byte and
  allocation proof;
- `paired-summary-36.csv` and `common-geomean-36.csv`: final governed reducer
  output; 12/24-pair intermediate summaries remain auditable;
- `timing/`: initial and two unchanged append blocks, with raw samples, outputs
  and capture manifests; `consolidated-36/` is the maintained matrix driver's
  final absolute-cell summary;
- `freeze/`: candidate RXAS, pre-link RXBIN and linked images;
- `candidate-*.sha256`, `baseline-products.sha256` and `SHA256SUMS`: source,
  product, image and whole-bundle identity;
- `v1r01-r1-plus-v3.patch`: replayable combined prerequisite/candidate patch
  against detached HEAD `b08611179db5ff4257c3be3103f3aeab55ea5b50`;
- `logs/`: configure/build, focused 10/10 and clean patch-apply checks;
- `PROVENANCE.md` and `COMMANDS.md`: exact identities and reproduction path.

The selection panel and current-product freeze remain authoritative in
`../2026-07-27-perf2-06-07-selection-panel/`; they are referenced rather than
duplicated.
