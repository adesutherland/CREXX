# NR-09 Rule 1 numeric-context first Release verdict

Status: accepted by Adrian and broad Debug closeout complete.

This bundle is revision-separated from historical NR-05 and the pre-NR-08
sequence-ledger inputs. It will retain the first independently verdictable
NR-09 production rule: compiler emission of existing RXBIN 007 `NUMSCI` or
`NUMENG` for a fully constant, non-inherited, fuzz-zero compatible procedure
numeric context.

## Exact baseline

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch/commit: `develop`,
  `7b93bef73267ee1542295616db5a0148e7766a43`
- Upstream at orientation: `origin/develop` at the same commit
- Starting worktree: clean
- Product: ordinary `CMAKE_BUILD_TYPE=Release`,
  `CREXX_VM_PROFILING=OFF`, retained source/TRACE metadata
- Canonical workload: optimized RexxCPS 2.2d, default `100 x 100`, source
  SHA-256
  `2970c3d73fe2537ec8f81295c585495c4668b442d5b9a2335b1ee453a13bbdd6`

The current Release artifacts were checked against the retained accepted NR-08
candidate. All hashes match exactly:

| Artifact | SHA-256 |
| --- | --- |
| optimized RXAS | `05aa58c27041adc4f86cfc0224f7c0803836f982d19a5c4f6cea1e4887173f4a` |
| optimized RXBIN | `466de7f06414148e4a3f63337e716ce332e1c082363297974240f4b3cc9cabbe` |
| linked library | `6d1ae40af463fd53633f44603a57cc2291aaa3149bb4ef0d04a67511ea04cf13` |
| `rxvm` | `7e92a7d08efe828768985603fb4b6d65afbbbe64ebe5b6c8f3ef4ff6483d6b44` |
| `rxbvm` | `9b5920e1b7b4a84d8167e1fde04003632869551224017118e46b9b10be54a29a` |

Baseline sizes are 226,793 bytes RXAS, 79,853 bytes RXBIN and 881,192 bytes
for the linked library. The optimized program has five sites for each of the
five `SETNUM*` immediate opcodes, 25 static procedure-entry setters total, and
no `NUMSCI`/`NUMENG`.

The exact retained post-NR-08 schema-4 profiles record 542,500 numeric setter
executions on `rxvm` and 542,500 on `rxbvm`. They are retained at
`../2026-07-17-nr-08-lifetime-poc/candidate/profiles/`. The valid accepted
ordinary Release baseline is retained at
`../2026-07-17-nr-08-first-release-verdict/`: median 1,195,649 CPS / 8.38 s on
`rxvm` and 1,180,487 CPS / 8.48 s on `rxbvm`, from three serial recorded
samples per VM after one warmup.

Those retained samples will be reused under the programme baseline rule unless
their lifecycle or exact-input validity is disproved. Candidate raw samples,
focused correctness, exact static/dynamic/image deltas, provenance and the
first Release verdict will be added only after the selected production edit is
frozen.

## Frozen implementation and focused correctness

The compiler emitter now selects the existing `NUMSCI`/`NUMENG` instruction
only when all five effective fields are non-inherited compile-time constants,
fuzz is zero, the form/case/standard enums are valid, and digits is at least 5.
Digits 1-4 retain the individual setters because the combined handlers have a
stricter historical validation contract. Inherited and nonzero-fuzz contexts
also retain the existing path.

The generated focused contract covers optimized and no-opt structure,
scientific and engineering operands, inherited digits, nonzero fuzz, digits
1-4, both VMs, and default plus explicit `mc_decimal` and `db_decimal` plugin
execution. Focused Debug tests pass 12/12, including the existing RXAS combined
instruction, decimal and numeric-library coverage. The focused ordinary
Release test passes 1/1. See `focused-debug-ctest.log` and
`focused-release-ctest.log`.

## Exact instruction and size deltas

Canonical optimized RexxCPS changes as follows:

| Dimension | Baseline | Candidate | Delta |
| --- | ---: | ---: | ---: |
| static numeric setup | 25 setters | 5 `NUMSCI` | -20 |
| dynamic setup, `rxvm` | 542,500 | 108,508 | -433,992 |
| dynamic setup, `rxbvm` | 542,500 | 108,508 | -433,992 |
| RXAS bytes | 226,793 | 226,493 | -300 |
| RXBIN bytes | 79,853 | 79,861 | +8 |
| linked library bytes | 881,192 | 880,384 | -808 |

The exact dynamic candidate partition is identical on both VMs: 108,498
`NUMSCI`, four `SETNUMFUZ`, two `SETNUMCAS` and four `SETNUMSTD`. The candidate
profile-build RXAS, RXBIN and linked-library hashes exactly match the ordinary
Release candidate, so the profile counts use the timed product image rather
than a divergent generated form.

The retained exact library disassembly changes 3,123 setup instructions (622
digits, 627 fuzz, 623 form, 624 case and 627 standard) to 651 (618 `NUMSCI`,
four digits, nine fuzz, five form, six case and nine standard), a reduction of
2,472. The standalone benchmark RXBIN grows by eight bytes even though RXAS and
instruction count shrink; the linked library shrinks by 808 bytes. These are
reported as exact format outcomes, not a generalized compression claim.

Machine-readable details are in `static-deltas.csv`,
`library-static-deltas.csv`, `dynamic-deltas.csv` and `size-deltas.csv`. Raw
schema-4 profiles and correctness output are retained under `profiles/` and
`profile-raw/`.

## Mandatory ordinary Release verdict

Lifecycle: canonical optimized RexxCPS 2.2d, retained source/TRACE metadata,
default `100 x 100`, serial processes, one candidate warmup and three recorded
candidate samples per VM. The exact accepted post-NR-08 baseline has the same
lifecycle and three recorded samples per VM and was reused without rerunning.

Higher benchmark-native CPS and lower process elapsed time are better:

| VM | Baseline median CPS (range) | Candidate median CPS (range) | CPS delta | Baseline/candidate median elapsed | Elapsed delta |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 1,195,649 (1,180,932-1,203,585) | 1,203,145 (1,194,385-1,213,159) | +0.627% | 8.38 s / 8.33 s | -0.597% |
| `rxbvm` | 1,180,487 (1,179,414-1,184,294) | 1,183,390 (1,176,634-1,189,443) | +0.246% | 8.48 s / 8.46 s | -0.236% |

All samples pass and CPS/elapsed direction agrees, but both candidate ranges
overlap their retained baseline ranges and the changes are small. The first
Release verdict is therefore neutral-to-slightly-positive, not a material
product win.

Adrian accepted the improvement on 2026-07-18. The implementation is retained
as the first NR-09 production rule.

## Accepted closeout

The complete Debug build passed. Initial broad Debug CTest then reported 226
failures: 222 compiler RXAS goldens and four RXPA signal-address expectations.
Before updating anything, an exact replay proved that all 222 generated files
differed only by 517 replacements of the five default numeric setters with one
`NUMSCI`; there were zero unrelated changes. The documented golden driver
updated 111 no-opt and 111 optimized files. A post-update diff audit found 517
added `NUMSCI` lines and exactly 2,585 removed `SETNUM*` lines, with no other
line changes.

The RXPA tests continued to report the same `SIGNAL ERROR` at
`signal_funcs.crexx:10`; the shorter prologue moved the expected bytecode
address from 15 (`0xf`) to 9 (`0x9`). The rerun-failed selection passed
227/227, including its linked fixture, and final broad Debug CTest passed
1,852/1,852 in 205.57 seconds.

See `closeout-golden-audit.txt` and the retained `closeout-*.log` files. No
sanitizer, install/package, cross-platform, additional timing campaign, commit
or push was added.
