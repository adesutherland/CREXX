# Windows initial-concurrency qualification

Date: 2026-08-17

Branch: `develop`

Qualified source commit: `2b793c81e0987f627ab72e3c4e505ae5c6a95abe`

Status: **QA-D PASS on MSVC, Clang and GCC.** Each lane used the versioned
Windows runner from a clean exact commit, a fresh MinSizeRel Ninja build, the
SCHANNEL TLS backend, two build jobs and two CTest jobs. Builds and tests were
kept strictly sequential within each lane.

## Results

| Compiler lane | Maintained concurrency matrix | Complete CTest | Installed and ZIP smoke |
| --- | ---: | ---: | --- |
| MSVC 19.44.35228 | 137/137 in 83.40 s | 2,076/2,076 in 631.07 s | `rxbvm` PASS |
| Clang 22.1.7 | 194/194 in 112.57 s | 2,220/2,220 in 810.77 s | `rxbvm` and `rxtvm` PASS |
| GCC 16.1.0 | 194/194 in 123.05 s | 2,220/2,220 in 978.57 s | `rxbvm` and `rxtvm` PASS |

All three lanes also passed:

- every non-empty SP-01 through SP-09 label inventory;
- live trusted-host and hostname-mismatch TLS verification;
- every unchanged concurrency-stress test for 20 repetitions;
- a fresh external install and installed-toolchain concurrency example;
- archive inventory, extraction and extracted-toolchain smoke; and
- independent verification of every full evidence-bundle digest and package
  digest.

MSVC intentionally builds only the portable `rxbvm` core. Clang selected
`rxbvm` as product `rxvm`; GCC selected `rxtvm`. The non-MSVC package smokes
ran both explicit VM executables in addition to proving the selected product.

## Bounded repairs before the formal replay

The qualification campaign found and repaired Windows portability defects,
then replayed every formal lane at the exact commit above:

- the PowerShell runner's clean-check count is strict-mode safe;
- MSVC receives C11 atomics and does not enable the POSIX parser-mode sibling
  by default;
- Windows numeric underflow expectations and RXJSON subnormal rejection are
  deterministic across the supported compiler runtimes;
- the system plugin includes the standard `errno` definitions it uses; and
- Clang/MinGW TLS storage uses supported `__thread` syntax, preserving worker
  and ODBC per-thread state.

The final Clang TLS repair was additionally exercised by 100 consecutive
active-context isolation repetitions, four ODBC cases and the full maintained
Clang matrix before the exact-commit qualification replay.

## Evidence map

Each compiler directory retains `RESULT.txt`, provenance and runner transcript,
configure/build logs, the maintained concurrency matrix, live TLS proof,
20-cycle stress proof, full CTest, all label inventories, install/package
inventories, and installed/extracted smoke logs.

- `msvc/`: Microsoft compiler evidence;
- `clang/`: LLVM/MinGW evidence;
- `gcc/`: GCC/MinGW evidence;
- `SHA256SUMS`: digests for this curated repository evidence set; and
- [`COMMANDS.md`](COMMANDS.md): exact environment and runner commands.

`SOURCE-SHA256SUMS` in each lane is the verified digest ledger for the complete
external runner bundle, including installed binaries that are deliberately not
committed. The ZIP binaries remain outside the repository in the artifact
directories named by each `RESULT.txt`. Their retained SHA-256 values are:

- MSVC: `dce78405895af96e36a4b0d4be207b15244085cfbb37752320daa30b45817606`;
- Clang: `ccc98a5a85de5cbd0c16d6e18264c48e52af4a1f20c56d3cba59d57348f86f6c`;
- GCC: `e265d8ba16cabae2d1e3aff34666f005ab72fe30e2b3548b829a76698e123df0`.

The large build trees were removed only after evidence and package verification;
the complete evidence directories and sibling ZIP artifacts remain under
`C:\crexx-qa`.
