# NR-08 first ordinary Release verdict

Status: **accepted positive verdict; broad closeout complete and staged without
commit or push**

This bundle is the mandatory first profiling-off Release verdict for the
selected NR-08 Gate A candidate. It retains the exact candidate implementation,
ordinary Release build/focused test logs, host/build/worktree provenance, raw
canonical RexxCPS samples, a comparison with the already-retained valid
NUMERIC-01 accepted baseline, and the corrected imported-library audit.

## Product and comparison boundary

- Repository/branch/HEAD: `/Users/adrian/CLionProjects/CREXX`, `develop`,
  `e748621d1f9242f68f30fe8aa3f58fb804be68fb`, exactly equal to
  `origin/develop` before the uncommitted NR-08/09 work.
- Host: macOS 26.5.2, Darwin arm64, Apple M5, 10 logical CPUs.
- Toolchain: Apple clang 21.0.0, CMake 4.3.2, Ninja 1.13.2.
- Product: complete ordinary `CMAKE_BUILD_TYPE=Release`,
  `CREXX_VM_PROFILING=OFF`, retained source/TRACE metadata.
- Workload: optimized canonical RexxCPS 2.2d, no argv, default `100 x 100`
  contract, source SHA-256
  `2970c3d73fe2537ec8f81295c585495c4668b442d5b9a2335b1ee453a13bbdd6`.
- Lifecycle: serial processes; one candidate warmup and three candidate
  recorded runs per VM. The retained accepted baseline used the same lifecycle
  and has three recorded runs per VM.

The retained baseline is
`performance/evidence/2026-07-17-numeric-01-first-release-verdict/accepted-c/`.
It is the exact final canonical 2.2d source/product accepted immediately before
commit `e748621d1`; the commit records those measured library/benchmark changes
and introduces no later VM implementation delta. Its source hash exactly
matches the candidate. Re-running it was therefore unnecessary under the
programme's retained-baseline rule.

Candidate exact hashes:

| Artifact | SHA-256 |
| --- | --- |
| optimized RXAS | `05aa58c27041adc4f86cfc0224f7c0803836f982d19a5c4f6cea1e4887173f4a` |
| optimized RXBIN | `466de7f06414148e4a3f63337e716ce332e1c082363297974240f4b3cc9cabbe` |
| linked library | `6d1ae40af463fd53633f44603a57cc2291aaa3149bb4ef0d04a67511ea04cf13` |
| `rxvm` | `7e92a7d08efe828768985603fb4b6d65afbbbe64ebe5b6c8f3ef4ff6483d6b44` |
| `rxbvm` | `9b5920e1b7b4a84d8167e1fde04003632869551224017118e46b9b10be54a29a` |
| production candidate patch | `ccb4e5b53216696e65f560556d5b915990bcf2a3f03fe15f020acf5afc9c4fa6` |

## Minimum correctness gate

- Complete ordinary Release build: pass.
- Focused Release CTest: 4/4 pass, including the linked fixture,
  `rxc_scoped_register_reuse`, and both stem modes.
- Bounded PoC reference/structural matrix: 26/26 pass.
- Bounded stem matrix: 3/3 pass.
- Optimized/no-opt reference inline/block controls: byte-identical.
- Candidate timing executions: 8/8 pass with canonical-default provenance,
  `effective_count=100`, and the RexxCPS correctness marker.

## Corrected imported-library audit

The required audit found a defect in the earlier static-count script: an
instruction attached to a branch label was not counted because the label was
read as the opcode. `analyze_library.zsh` and the retained-program analyzer are
now label-aware, both phase checksum manifests pass, and the exact retained
RXBINs/dynamic profiles are unchanged.

| Operation | Baseline | Candidate | Delta |
| --- | ---: | ---: | ---: |
| `NULL` | 4,293 | 4,293 | 0 |
| `ENDLIFE` | 8,006 | 151 | -7,855 |
| typed/general copies | 8,478 | 8,478 | 0 |
| `UNLINK` | 1,758 | 1,758 | 0 |
| reference operations | 7 | 7 | 0 |

All 629 procedures reconcile. Exactly 263 change, every change is an
`ENDLIFE` reduction, no procedure gains `ENDLIFE`, and there are zero non-
`ENDLIFE` changes. The previously suspected downstream NULL/copy DCE cascade
was an analysis undercount, not a generated-code change.

## Release result

Higher benchmark-native CPS and lower process elapsed time are better.

| VM | Baseline median CPS (range) | Candidate median CPS (range) | CPS delta | Baseline/candidate median elapsed | Elapsed delta |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 1,145,721 (1,138,702-1,172,036) | 1,195,649 (1,180,932-1,203,585) | **+4.358%** | 8.74 s / 8.38 s | **-4.119%** |
| `rxbvm` | 1,114,685 (1,098,014-1,132,910) | 1,180,487 (1,179,414-1,184,294) | **+5.903%** | 8.99 s / 8.48 s | **-5.673%** |

The result is positive in both VM modes, native CPS and elapsed time agree,
the candidate ranges do not overlap the retained baseline ranges, and every
sample passes. This is a clear positive first Release verdict for Gate A. It is
not a portfolio-wide forecast.

## Accepted closeout verification

Adrian accepted the verdict and approved the shortest required closeout.

- Complete Debug build: pass; retained in `debug-build.log`.
- Initial full Debug CTest: 1,780/1,851 pass, with 71 compiler-output golden
  mismatches; retained in `debug-ctest.log`.
- Exact generated/golden audit: 170 removed scalar `ENDLIFE` lines, zero added
  instructions and zero unrelated changed lines; retained in
  `golden-audit-summary.txt`.
- Documented `crexx_test_driver --update-gold`: exactly 71 golden files
  updated; retained in `golden-update.log`.
- Failed-test rerun: 71/71 pass in 7.31 seconds; retained in
  `debug-ctest-rerun.log`.
- Final full Debug CTest at parallelism 30: 1,851/1,851 pass in 217.35 seconds;
  retained in `debug-ctest-final.log`.

The matching runtime tests had passed during the initial broad run, and the
golden diff contains only the expected Gate-A removal. The implementation,
focused structural tests, intentional goldens, records and evidence are staged
together. No sanitizer, install/package, cross-platform or additional
portfolio/timing campaign was added. No commit or push was performed.
