# PERF2-02 first Release verdict

Status: **accepted; broad QA and closeout complete**

This is the mandatory first ordinary profiling-off Release verdict for the
approved zero-state A-LOCAL/A-ATTR guard in canonical `MKREF_REG_REG`. It
compares the accepted clean-HEAD Q0 binaries with the integrated dirty-tree
production binaries, using the same accepted optimized Bounce image and
library in both VM modes.

The comparison was intentionally the smallest decisive end-to-end cell. Adrian
accepted the favorable verdict on 2026-07-24 and authorized every remaining QA
step plus a local commit. The implementation is therefore no longer
provisional.

## Verdict

The approved direct canonical guard is clearly favorable in both VM modes.
Against the accepted clean-HEAD Q0 product, canonical Bounce elapsed falls by a
paired median **80.596%** on `rxvm` and **78.464%** on `rxbvm`. Every balanced
pair is favorable and both two-sided mean 95% Student-t intervals are wholly on
the favorable side of zero.

| VM | Pairs | Paired Q1 | Paired median | Paired Q3 | Mean 95% interval | Favorable |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 12 | -80.821% | **-80.596%** | -80.455% | -80.863% to -80.457% | 12/12 |
| `rxbvm` | 22 | -78.546% | **-78.464%** | -78.356% | -78.638% to -78.293% | 22/22 |

Negative elapsed is favorable. The absolute-median Q0/production ratios are
5.169217x (`rxvm`) and 4.641437x (`rxbvm`).

## Absolute cells

All durations are seconds. Raw nanosecond observations remain authoritative.

| Cell | n | Minimum | Q1 | Median | Q3 | Maximum | MAD |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Q0 `rxvm` | 12 | 6.714846 | 6.823513 | 6.862549 | 6.948800 | 7.036197 | 0.066295 |
| production `rxvm` | 12 | 1.310852 | 1.317371 | 1.327580 | 1.332127 | 1.371643 | 0.007640 |
| Q0 `rxbvm` | 22 | 7.027112 | 7.062893 | 7.076670 | 7.168514 | 7.839082 | 0.036761 |
| production `rxbvm` | 22 | 1.512944 | 1.520467 | 1.524672 | 1.540722 | 1.684745 | 0.006301 |

The initial `rxbvm` blocks crossed the 10% min/max-span rule, so ten serial
samples per affected cell were appended unchanged. The append itself is stable
(0.377%/0.174% relative MAD and 2.066%/0.910% span for Q0/production). Policy
retains the original slow episode, so the combined absolute min/max span still
exceeds 10%; no sample was removed. The 22-pair interval is nevertheless
decisive, so no additional balanced-pair append is required.

## Correctness and footprint

- Focused Debug CTest passes 10/10 in the direct-threaded and switch VMs,
  including the new owned-child/direct-local routes, mandatory external-link
  fallback, existing reference/catch behavior, signal unwind, dynamic load and
  re-entry.
- The ordinary profiling-off Release guard fixture passes in both VM modes.
- Canonical RXAS/RXBIN and public ABI remain unchanged. The implementation has
  no persistent site state, allocation, execution-image mutation, publication,
  invalidation or teardown path; every miss executes the existing generic
  owner/lifetime discovery.
- Artifact file deltas are 0 bytes (`rxvm`) and +48 bytes (`rxbvm`); Mach-O
  text deltas are +416 and +3,964 bytes, below the 5%/4 KiB escalation gate and
  identical to the selected Q3b PoC code shape.

After Adrian accepted this verdict, broad Debug, Release, sanitizer, isolated
install, native-toolchain and retained-bytecode compatibility gates all passed.
The full closeout record is in `qa-closeout.md`. No push was authorized.

## Evidence map

- `provenance.md`: exact source, build, artifact, host and focused-correctness
  identity.
- `commands.md`: exact formal capture command.
- `manifests/`: four-cell Q0/production and two-cell noise-append matrices.
- `timing/bounce-formal/`: initial generated raw samples and outputs.
- `timing/bounce-rxbvm-noise-append/`: governed ten-sample append.
- `timing/bounce-combined/summary.csv`: combined absolute summary.
- `paired-summary.csv`: governed paired distribution and mean interval.
- `qa-closeout.md`: post-acceptance regression repair, broad validation,
  installed-tree and compatibility results.
