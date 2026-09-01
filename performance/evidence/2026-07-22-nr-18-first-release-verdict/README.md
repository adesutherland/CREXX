# NR-18 first ordinary Release verdict

Status: accepted and complete.

## Frozen construction result

- Exact accepted-NR-27 to NR-18 19-image census: 45,476 -> 45,178 executable
  instructions, -298 with no image growth.
- Incremental accounting: +294 unreachable removals and +4 typed-copy
  removals. H1 has no retained 19-image footprint; J1 supplies the production
  reduction.
- Focused structural checks pass 6/6, existing jump-table checks pass 42/42,
  and the optimized/`-n` runtime fixture passes in both VMs (4/4).
- The generated-evaluator scale control retains 2,345 of 2,367 newly unlocked
  removals and completes in 3.65 seconds with debug enabled, versus 91.56
  seconds for the unbatched implementation and 1.36 seconds for accepted
  NR-27.

## First Release result

- The exact linked library is 55,664 -> 54,829 instructions (-835, -1.500%)
  and 860,816 -> 858,896 bytes (-1,920).
- Sieve, Permute, Bounce, Richards and Base64 modules are unchanged. Each
  accepted-NR-27/NR-18 linked pair is byte-identical even with its matching
  library, so the common timing portfolio has no NR-18 exposure. This proves
  no regression for those exact products and makes a wall-clock campaign
  non-discriminating.
- Six changed end-to-end paths were checked as matching baseline/candidate
  products under both ordinary profiling-off Release VMs and both profile VMs.
  All 24 ordinary and 24 profile runs return zero and preserve their expected
  output. The cross-runtime matrix self-test executes 2,208 -> 2,189
  instructions (-19, -0.861%) in both VM modes. The evidence tool (2,410),
  lifecycle tool (388), artifact inventory (354), generic frozen PARSE (522),
  and benchmark-runner Sieve dispatch (11,601) are dynamically unchanged.
- The verdict is favorable for code size/preparation and includes one bounded
  dynamic reduction. It is not evidence of a common-portfolio wall-clock
  speedup.

Adrian accepted the verdict on 2026-07-22. The complete Debug build, final
affected-surface 63/63 and broad Debug CTest 1,891/1,891 all pass. No sanitizer,
install/package, cross-platform or repeated timing campaign was added to the
approved shortest closeout. `static-census.csv`, `artifact-inventory.csv`,
`dynamic-census.csv`, `commands.md` and `qa-closeout/README.md` retain the exact
evidence and interpretation boundary.
