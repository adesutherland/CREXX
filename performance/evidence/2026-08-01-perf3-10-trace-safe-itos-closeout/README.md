# PERF3-10 trace-safe ITOS closeout

Status: **complete Apple ARM64 closeout; C1/T1 accepted by Adrian on
2026-08-01**

PERF3-10 makes TRACE result delivery an ordered event stream at the reached
instruction boundary and adds a reusable RXAS storage/component proof.  The
selected C1/T1 product removes an `ITOS` only when the integer value, its
derived string representation, storage identity and numeric context remain
proved.  Direct link/swap/unlink identity, joins and component writes are
modelled; calls, ambiguous references, opaque effects and unproved signal
phases fail closed.  TRACE records remain present and are drained in metadata
order.  No numeric-address scan, source-step change, public RXAS/RXBIN change
or public VM ABI change is introduced.

## Accepted outcome

| VM | Pairs | Favourable | Median CPS change | Q1 / Q3 | Mean 95% interval |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 22 | 21/22 | **+10.376%** | +9.429% / +11.426% | +7.320% to +12.869% |
| `rxbvm` | 12 | 12/12 | **+10.612%** | +9.839% / +11.042% | +9.684% to +11.488% |

The ordinary profiling-off Release comparison used the exact pre-edit C0
product and the frozen C1 product.  The initial schedule was one warmup plus 12
balanced/interleaved recorded pairs for each VM.  RXVM crossed the standing
spread threshold, so the declared ten-pair append ran.  All 72/72 executions
passed, 68 were recorded, and no sample was removed.  The combined RXVM set
retains one adverse -7.711% pair and one favourable +31.247% pair; its mean
interval remains wholly favourable.

## Static and dynamic mechanism proof

The compiler-emitted optimized RXAS contains 24 `ITOS` instructions.  The
retained C0 assembler already reduced that to 17; C1 reduces it to 14.  In the
hot `main` loop, the six emitted `lvar` conversions were five in C0 and are two
in C1.  The incremental C1 effect is therefore three fewer static hot
conversions, not the larger source-to-product total.

A counts-only diagnostic ran identical, explicitly noncanonical 200 x 100
work under the profiling VM.  Both variants passed without calibration:

| Instruction | C0 | C1 | Reduction |
| --- | ---: | ---: | ---: |
| all VM instructions | 55,900,921 | 54,501,316 | 1,399,605 (2.504%) |
| `ITOS` | 2,520,006 | 1,120,006 | 1,400,000 (55.555%) |
| `STOD` | 1,660,000 | 1,660,000 | 0 |
| `DTOS` | 2,220,000 | 2,220,000 | 0 |

The raw profiles are diagnostic; the unprofiled Release comparison above is
the performance authority.

## Correctness and closeout

- The new same-boundary TRACE fixture was a genuine red proof: T0 failed all
  4/4 optimized/no-opt and `rxvm`/`rxbvm` combinations; T1 passed 4/4.
- The mandatory first-verdict minimum gate passed 53/53 focused checks.
- The accepted closeout rebuilt Debug and passed 59/59 focused tests.
- The first broad run passed 1,981/1,982.  `trace_stem_sugar` expected the
  factory event but not the now-correct same-boundary assignment event under
  intermediate tracing.  The expected ordered stream was updated, the exact
  test passed 1/1, and the final broad run passed 1,982/1,982.
- The five pre-existing untracked lifecycle images retain their starting
  hashes.

## Host and source boundary

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch/source anchor: `develop` at
  `6759a8cef543ca496be246402fa623edbb101c56` plus the checksum-closed dirty
  PERF3-10 implementation/evidence scope
- Host: Darwin 25.5.0 arm64, Apple M5, 10 logical CPUs
- Toolchain: Apple clang 21.0.0, CMake 4.3.2, Ninja 1.13.2
- Release: `CMAKE_BUILD_TYPE=Release`, `CREXX_VM_PROFILING=OFF`
- Debug: `CMAKE_BUILD_TYPE=Debug`, `CREXX_VM_PROFILING=OFF`
- Broad test parallelism: 30
- Canonical workload source SHA-256:
  `2970c3d73fe2537ec8f81295c585495c4668b442d5b9a2335b1ee453a13bbdd6`

Adrian declared the host ready for the rerun after removing the remote-terminal
load.  The paired rotation keeps C0 and C1 in the same session, but no separate
machine-readable pre/post power or thermal snapshot was retained; this is an
explicit evidence limitation rather than an inferred host-state claim.

## Evidence map

- `artifact-hashes.csv`: exact C0/C1 product identities;
- `source-identity.csv`: exact dirty implementation, test, documentation and
  live-control identities;
- `timing/`: initial and governed append manifests, samples, outputs and final
  pair ratios;
- `paired-summary.csv`: combined accepted pair statistics;
- `profiles/` and `dynamic-counts.csv`: equal-work counts-only proof;
- `static/` and `static-counts.csv`: C0/C1 disassembly and RXAS acceptance log;
- `validation.csv` and `logs/`: focused and broad closeout record, including
  the reviewed first broad result;
- `protected-lifecycle-hashes.csv`: unchanged pre-existing generated images;
- `COMMANDS.md`: reproduction commands; and
- `checksums.sha256`: recursive integrity for the bundle.

## Claim boundary

This closes the accepted PERF3-10 Apple ARM64 slice.  It does not claim a wider
portfolio gain, cross-platform validation, decimal-conversion elimination,
general loop hoisting, completed signal-phase classification, a T2 private VM
stream, sanitizer/install/package validation, publication, or push.
