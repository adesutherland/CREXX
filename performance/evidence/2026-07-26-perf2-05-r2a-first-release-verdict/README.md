# PERF2-05 R2a first Release verdict

Status: **accepted; R2a closeout complete**

This is the mandatory first ordinary profiling-off Release verdict for the
approved R2a private reference-descriptor materialization. It compares the
accepted clean-HEAD product with the integrated dirty-tree product, using the
same retained optimized List image and library in both VM modes. Public RXAS,
canonical RXBIN and the ABI remain unchanged.

## Verdict

The exact accepted work-100 List cell is clearly favorable in both VMs. Times
are 12 serial, position-balanced pairs after one warmup per cell. Negative
elapsed deltas are favorable.

| VM | Pairs | Q1 | Paired median | Q3 | Mean 95% interval | Favorable |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 12 | -2.907% | **-2.731%** | -1.820% | -3.230% to -1.665% | 11/12 |
| `rxbvm` | 12 | -2.300% | **-1.745%** | -1.487% | -2.565% to -1.478% | 12/12 |

Every absolute cell remains below the repository noise thresholds. Relative
MAD is 0.262%/0.821% for Q0/R2a `rxvm` and 0.531%/0.372% for Q0/R2a `rxbvm`;
the largest min/max span is 3.861%.

The initial work-10 setup capture is retained unchanged under
`timing/list-work10-guard/` and summarized separately. It is explicitly a
startup-mixed guard, not the decision cell: it used the profiling-census work
argument before the accepted scratch work-100 argument was rechecked.

## Correctness and compatibility

- Focused Debug reference/bounds/fallback/TRACE-breakpoint tests pass 10/10 in
  the direct-threaded and switch VMs.
- Compiler-generated optimized/no-opt, imported accessor and reference
  lifetime coverage passes 49/49.
- The ordinary Release fixture passes the integrated `rxvm` and `rxbvm`.
- The clean baseline VMs also run the newly assembled fixture, proving that it
  contains only canonical instructions.
- The integrated VMs run the retained accepted optimized List RXBIN and
  library without rebuilding either input.
- The execution image recognizes only exact
  `LINKATTR1; COPY; UNLINK` shapes. Non-reference payloads, debug/breakpoint
  execution, bounds failure and nonmatching shapes take the canonical path.

## R2b review

R2b is **deferred and not selected**. The earlier public scratch `COPYATTR1`
ceiling and production R2a both call canonical `copy_value`; therefore the gap
between the scratch -6.173%/-6.154% medians and R2a's
-2.731%/-1.745% paired medians does not isolate general value-copy work and is
not an estimate of a payload-only helper's benefit. The production form also
adds the required runtime reference-shape and observability guards and retains
the nine-cell canonical stream in its private execution image.

A future R2b PoC requires separate accepted profiling that attributes material
time to `copy_value` after R2a, plus mechanical proof for destination cleanup,
reference-cell retain/release, reference identity, representation flags and
invalidation. None of that work is authorized by this verdict.

## Accepted closeout

Adrian accepted the favorable verdict on 2026-07-26. The full Debug and
ordinary profiling-off Release products then rebuilt successfully. Broad CTest
passes 1,922/1,922 in each configuration. This is the required post-verdict
closeout for the selected slice.

R2b remains deferred and R1a remains a separate, not-yet-started rung. In line
with the approved closeout path, no sanitizer, install/package,
cross-platform, expanded-portfolio or repeated-baseline work was added.

## Evidence map

- `VERDICT.md`: accepted decision and closeout boundary.
- `PROVENANCE.md`: exact source, product, input and host identity.
- `COMMANDS.md`: focused build, compatibility and balanced capture commands.
- `manifests/`: retained work-10 guard and accepted work-100 decision inputs.
- `timing/list-formal/`: authoritative work-100 samples, outputs and summaries.
- `timing/list-work10-guard/`: retained setup guard; not used for selection.
- `paired-summary.csv`: governed work-100 paired distribution and mean interval.
- `build/`: fresh external profiling-off Release configure/build logs.
