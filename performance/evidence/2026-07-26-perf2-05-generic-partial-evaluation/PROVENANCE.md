# PERF2-05 P05-CF1 provenance

## Repository identity

| Field | Exact value |
| --- | --- |
| repository | `/Users/adrian/CLionProjects/CREXX` |
| branch | `develop` |
| starting/final parent HEAD | `22dd01a5b2e98e7e05682141b025633f8aecdd0a` |
| subject | `docs: close PERF2-04 BIF campaign` |
| accepted PERF2-04 production commit | `f8f34092eed34812950dd591525e7d927dc0d88a` |
| upstream at start/closeout | `origin/develop` at `d1c5245d49c0bd9cc48a7d33ef16f2f4555cc986` |
| ahead/behind before P05 commit | `+3/-0` |
| starting worktree | clean |
| final uncommitted scope | P05-CF1 compiler, opcode metadata, focused tests/goldens, roadmap/worklist and this evidence bundle |

The production commit is the commit containing this bundle; its parent is the
exact HEAD above. No push is part of the closeout.

## Host, toolchain and builds

| Field | Value |
| --- | --- |
| host/model | `Adrians-MacBook-Air.local`, `Mac17,3` |
| architecture/CPU | Apple arm64, 10 logical CPUs, Apple M5 |
| memory | 24 GiB |
| OS | macOS 26.5.2 build 25F84; Darwin 25.5.0 |
| compiler | Apple clang 21.0.0 |
| CMake / Ninja / Git | 4.3.2 / 1.13.2 / 2.50.1 |
| timing power | AC attached, battery 80%, both low-power modes `0` |
| timing thermal state | no thermal or performance warning recorded |
| final storage | approximately 629 GiB free |

The ordinary product is Ninja Release with `CMAKE_BUILD_TYPE=Release`,
`CREXX_VM_PROFILING=OFF` and `-O3 -DNDEBUG`. The broad QA product is Ninja
Debug with profiling off. Baseline and candidate sources/builds remained
separate under
`/var/folders/nr/7ckzqpl91kz80mcy3316h1tr0000gn/T/crexx-perf2-05.KM0ZeuBw5m`.

## Exact ordinary products

| Product | Accepted SHA-256 / bytes | Final P05-CF1 SHA-256 / bytes |
| --- | --- | --- |
| `rxc` | `350b8d7d02b7b938a4d84fade97b9c3ccecd7cd80b6e0081b041022b9ca3eeaa` / 2,786,432 | `43e802c17c22478f38e1d9ca7f5a71eb44d04143fae1534cb0f52694c1200fdc` / 2,802,384 |
| `rxas` | `6fdb7ba0b7979608a6faa20ae2ce8ebf5ba9bd66a24cacc1e654dd6515ac5aff` / 563,080 | `3574bafb2d95c0827604231c838b89366ea9abd515ec04f88cece3412d560d3d` / 579,736 |
| `rxvm` | `225952dbd23a56baffd032461977d8a686866dfbef77613ac8bbd40ebe615815` / 982,264 | `5ec03673f9676942c04eb33449746b8184911026dbf805924a7784cdc10ca332` / 982,264 |
| `rxbvm` | `0a880d163f0fb3fc653ce42adc48356e3390f12b36eeccb104b251865cf3b144` / 982,440 | `53beb048ee635377bcc38264fea77c8360c2fc750240f99ae818cee0c63dda1b` / 982,440 |
| `library.rxbin` | `221727fccba50f74a0da57a0ae0dcd94241f21bfe6d5cc60145a767a41f0aab4` / 862,096 | byte-identical |
| `rxcexits.rxbin` | `3f0a72b993e6d351ceb25d82a6ad5a8ab5ce0150b5884c28de3b479ec95dcacf` / 1,481,312 | byte-identical |

The VM executable hashes changed through the affected-target relink; file
sizes and VM sources are unchanged. The RexxCPS guard therefore compares the
accepted and relinked executables with one identical workload/library image.

## Decisive generated identity

| Artifact | SHA-256 | Bytes |
| --- | --- | ---: |
| accepted RXAS | `fe96277e4828d89a9b57e33cd46ce8b513a6acbcb4d53b9c87970955a0f25b87` | 10,438 |
| accepted RXBIN | `820737a8327995967113cae5ba6c31f18250f8893aee322168814e4bb5b17ba4` | 5,207 |
| timed/final P05 RXAS | `c42c40484c15677ae5d55d5972f44a55ac91f7f193012584764cc2fc76099cb1` | 2,674 |
| timed/final P05 RXBIN | `62fba975e002ed43d00784850994bea12ec784f9ee699142df727a96937a198d` | 2,311 |

The final reviewed `rxc` reproduced the timed candidate RXAS byte-for-byte.
The RXBIN initially appeared 136 bytes smaller when assembled from a shorter
relative input pathname; its `RXM7` metadata stores that pathname. Reassembling
the identical RXAS with the original absolute input pathname reproduced the
timed candidate RXBIN byte-for-byte. No executable-code difference exists.

## Capture and replay boundary

The existing Level B `run_cross_runtime_matrix.crexx` captured all timing
blocks serially with correctness qualification and rotating cells. The
existing Level B inventory tool generates the bundle manifest. No new Python
or second performance control plane was introduced.

Profiler timing is not used. Ordinary profiling-off Release wall clock is the
product verdict, separately for `rxvm` and `rxbvm`. The full formal portfolio,
sanitizer, install/package and cross-platform matrices were not run because
the accepted shortest closeout path and final diff supplied no risk-specific
reason to add them.
