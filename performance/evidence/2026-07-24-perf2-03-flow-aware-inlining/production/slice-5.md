# Production slice 5 - proved receiver and reference-accessor placement

Date: 2026-07-24

Starting commit: `8b050ff70fc9`

## Scope

- Shared a method receiver with the caller's object register only when I6/body
  evidence proves the method identity and the receiver is a direct local
  object variable, excluding computed, exposed/formal/generated, enclosing
  `§this`, already-aliased and flow-substituted storage. Those excluded cases
  retain materialisation and copyback.
- Preserved call-entry ordering by capturing actuals that read receiver-owned
  state before a shared receiver can be mutated. A real call already binds the
  receiver value pointer directly as `a1`, so pre-existing weak references to
  a proved direct local remain attached to the same storage; the dedicated
  alias regression observes the post-call mutation. Nested enclosing `§this`,
  computed receivers and every unproved ownership/lifetime case remain closed.
- Opened the separately proved exact reference-attribute getter/setter family.
  A getter is one final return of one receiver-owned reference attribute; a
  setter is one required by-value reference formal, one matching attribute
  assignment and a final bare return. Any additional expression, call,
  dereference, optional/reference/vararg formal or control effect retains the
  ordinary call.
- Transported the required `TYPE_REFERENCE` descriptor through I6 only after
  the imported body reconstructs and exactly matches the producer summary.
- Fixed a source-import fixed-point defect found while adding the regression:
  an already-known outer reference type no longer prevents its unknown
  referent type from converging. The update is monotonic and is covered by the
  source-import reference-accessor test.
- Added no public RXAS, RXBIN, ABI or VM contract.

## Correctness and QA

- Focused local/source/binary reference-accessor, receiver alias/lifetime,
  nested-`§this` and contradictory-summary set: 8/8 pass before measurement.
- Exact reference getter/setter tests pass in local, source-import and
  binary-import forms on both `rxvm` and `rxbvm`; the side-effecting near miss
  remains a call.
- The first full Debug CTest exposed four optimized golden changes. Review
  found only proved receiver-copy removal and resulting register renumbering;
  the two existing runtime counterparts passed, and the dependency/import
  cases were additionally assembled and run successfully on both VMs. The
  refreshed focused neighborhood passes 9/9.
- Complete Debug build: pass.
- Final full Debug CTest: 1,915/1,915 pass at `--parallel 30`.
- Five-workload baseline/candidate output guard: 20/20 pass across both VMs.

## Artifact verdict

The baseline is the preserved ordinary Release product from exact pre-slice-5
HEAD `8b050ff70fc9`. The post-slice-4 merge already changed some recorded
slice-4 identities; therefore this gate compares against the immediate
pre-edit product rather than attributing those earlier changes to slice 5.

| Workload | Baseline instructions / peak locals / RXBIN bytes | Slice 5 | Verdict |
| --- | ---: | ---: | --- |
| List | 233 / 34 / 16,186 | 239 / 34 / 16,650 | +6 instructions, locals unchanged, +464 bytes |
| Permute | 225 / 27 / 11,925 | identical | exact parity |
| Richards | 1,865 / 62 / 79,086 | identical | exact parity; all earlier gains retained |
| JSON | 46 / 9 / 4,001 | identical | exact parity |
| RexxCPS | 1,402 / 105 / 77,438 | identical | exact parity |

The shared library is byte-identical at 54,224 executable instructions and
859,953 bytes, SHA-256
`0bbb30bab0701d47078e9264c60738c8fc9a4e51c6e382a8354d187e8be4500d`.
Permute, Richards, JSON and RexxCPS RXAS and RXBIN are byte-identical. List
RXAS grows from 35,394 to 37,003 bytes and its exact static shape changes as
follows:

| Metric | Baseline | Slice 5 | Delta |
| --- | ---: | ---: | ---: |
| general copies | 7 | 14 | +7 |
| typed copies | 2 | 2 | 0 |
| call opcodes | 21 | 15 | -6 |
| `ListElement.next()` call sites | 5 | 0 | -5 |
| `ListElement.setNext()` call sites | 1 | 0 | -1 |

The extra link/copy/unlink scaffold is a deliberate static trade-off. It
removes the measured 3,820,600 dynamic `next()` calls instead of claiming that
small accessors are free merely because their bodies are short.

## Release timing verdict

Stable AC power, low-power mode 0, ordinary profiling-off Release products,
List work=100, one warmup plus three serially rotated recorded runs per cell.
The preserved pre-edit VMs ran both baseline and candidate program/library
images. Every recorded pair favored slice 5.

| VM | Baseline median | Slice 5 median | Elapsed reduction | Throughput ratio | Relative MAD baseline / slice 5 |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 182.367 ms | 86.044 ms | 52.818% | 2.119x | 0.203% / 1.089% |
| `rxbvm` | 201.974 ms | 94.499 ms | 53.212% | 2.137x | 0.106% / 0.097% |

Verdict: **decisively favorable**. Adrian accepted the verdict on 2026-07-24
and authorized broad closeout. Slice 5 is eligible for its independent commit.

Raw artifact, sample, output and capture data are retained beside this report
as `slice-5-*` files.

## Host and build

- Branch: `develop`; dirty scope limited to the approved slice-5 production,
  regression, evidence and live-status files before commit.
- Host: Darwin 25.5.0 arm64, 10 logical CPUs.
- CMake 4.3.2, Ninja, `CMAKE_BUILD_TYPE=Release`,
  `CREXX_VM_PROFILING=OFF`.
- Release `rxc` SHA-256:
  `4889a1725adaded5aec7b1f9d251086a934d0f456e8de5625599135900f1231e`.
- List RXBIN SHA-256:
  `6a0b52d8da3930b2edafc849c83faffa7a8e855d96fef4e43d8d8d52784ca0c4`.

## Implementation identities before commit

- `compiler/rxcp_inline.c`:
  `2d91ecf46f134b8e530e9d960de8e1b07eab2497e1a3353d0c2ce817370ce415`
- `compiler/rxcp_inline_analysis.c`:
  `47acd8dc654faf4ea775d71ea83fa7c5a03be11e5db7bdaf358e1bc8da64067e`
- `compiler/rxcp_inline_bind.c`:
  `3a32100f1bf81e97daadfe4ab289ebe7bbd1fff1bb3cfd5b0afc5350bb49be54`
- `compiler/rxcp_inline_internal.h`:
  `42eb1fdee05595fbb7fbde20aa496ce149ecdcdf1facd742cc045008c938c7f3`
- `compiler/rxcp_inline_payload.c`:
  `47367fe8f15cc904265a989df74739eec96d2d6a3273f210349966d8bb4c6f14`
- `compiler/rxcp_inline_rewrite.c`:
  `90aaeaf3e11cd4ac1bda19c420f4cfa5dddf837bc892338385277e79e63f043c`
- `compiler/rxcp_sym.h`:
  `691af6aa28a5f909ad4ad3c55ad00c96f5da192e77c1e9e9de64575a767f40fd`
- `compiler/rxcp_val_orch.c`:
  `afc0a19b51999377d0d371e658ff43abfe81f7d5c4d19a02365d8280aadfb5ab`
- `compiler/rxcp_val_type.c`:
  `f30ad80e3930db47b35204d5fad8b121190070c5513a6a5e1ed809f6db4dc9b2`
- `compiler/rxcpemit.c`:
  `284d6100abb472927d0fc9af43feee6fa269b3b2dcc70a19a71042f3288e7441`
