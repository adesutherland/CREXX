# Production slice 4 - versioned callable summaries

Date: 2026-07-24

Starting commit: `6b97c2ffd2fb2d7077c7e5c6fb24138dafd9f895`

Accepted production commit: `26f4aeb6fa314252c10a5bc3b1b831b62cf84add`

## Scope

- Added an immutable schema-1 callable summary to local symbols and the
  compiler-owned I6 inline payload. It records formal type/shape and
  read/write/escape facts, result/control/context facts and structural cost.
- Reconstructed every imported summary from the transported typed body and
  checked its result shape against the independently parsed callable
  declaration before enabling the imported template.
- Opened one narrow, mathematically proved case: an exact, private, read-only
  scalar actual may share the formal register when the I6 evidence proves the
  binding safe, even if the older `is_const_arg` emitter hint is absent.
- Kept missing, I4/I5, malformed or contradictory evidence on the ordinary
  call path. Reference/object alias and ownership cases remain closed for the
  then-unauthorized Slice 5.
- Corrected two attachment paths found during code review: compiler-created
  implicit `main` wrappers are skipped, and generic class-factory registry
  declarations do not receive semantically different FACTORY payloads.
- Preserved public RXAS, RXBIN 007, ABI and VM contracts. I6 is compiler-owned
  metadata retained by library artifacts and stripped from final programs.

## Correctness and QA

- Focused Debug `rxc` build: pass.
- Evidence-open/fail-closed/importer regression set: 10/10 pass, including
  source and binary imports, old-version fallback, mismatched-result fallback,
  contradictory formal-effect fallback, direct-register binding equivalence
  and `address_inline_then_parse`.
- The first broad run found four expected SELECT compiler goldens whose inline
  payloads changed only from I5 to I6 plus the new `c` record. Normalizing that
  header made each generated file byte-identical to its previous golden; the
  four reviewed goldens were refreshed and passed 4/4.
- Final full Debug CTest: 1,910/1,910 pass at `--parallel 30`.
- Ordinary profiling-off Release rebuild and immediate no-op rebuild: pass.
- Five-workload artifact inventory and rotated Richards comparison on both
  VMs: pass.

## Reproducible artifact verdict

All five optimized program images are instruction-, local-, byte- and
hash-identical to slice 3. In particular, Richards remains 1,867 instructions,
62 peak locals and 79,094 bytes with SHA-256
`6aad1ca91ddb53089fe0b5040f47e1267cabf6bf13c1cc8527b8940d77b50f9a`.
The accepted slice-3 Richards improvement relative to P0 is therefore retained
exactly: 30 fewer instructions, four fewer peak locals and the previously
accepted 33.308% `rxvm` / 31.118% `rxbvm` timing improvement.

| Workload | Slice 3 | Slice 4 | Verdict |
| --- | ---: | ---: | --- |
| List | 233 / 34 / 16,186 | 233 / 34 / 16,186 | exact parity |
| Permute | 227 / 28 / 11,925 | 227 / 28 / 11,925 | exact parity |
| Richards | 1,867 / 62 / 79,094 | 1,867 / 62 / 79,094 | exact parity; slice-3 gain retained |
| JSON | 46 / 9 / 4,001 | 46 / 9 / 4,001 | exact parity |
| RexxCPS | 1,402 / 105 / 77,438 | 1,402 / 105 / 77,438 | exact parity |

The shared library remains 54,387 executable instructions. Its RXBIN grows
from the retained slice-3 856,873 bytes to 860,545 bytes: 3,672 bytes or
0.429%, attributable to the I6 proof metadata retained for downstream
compilation. The final library hash is
`175c26eefce0628ce361ceb4774d85edd79121ad230d17852ae64151eef5e599`.

An earlier incremental artifact appeared to contain 42,333 shared-library
instructions and 762,496 bytes. It did not survive a complete dependent
Release rebuild and did not reproduce in repeated rebuilds. That transient
artifact and its associated timing are rejected and excluded from retained
evidence; they are not a slice-4 improvement.

## Release timing interpretation

Stable AC power, low-power mode 0, ordinary profiling-off Release products,
Richards work=10, one warmup plus three serially rotated recorded runs per
cell. The slice-3 and slice-4 cells used the same slice-4 VM and the exact same
Richards program image; only the linked library differed by retained metadata.
Power remained attached throughout the 17:52:27Z-17:53:39Z capture.

| VM | Slice 3 median | Slice 4 median | Apparent slice-4 delta | Relative MAD slice 3 / slice 4 |
| --- | ---: | ---: | ---: | ---: |
| `rxvm` | 4.524012 s | 4.465286 s | +1.315% | 1.852% / 0.524% |
| `rxbvm` | 4.617673 s | 4.613615 s | +0.088% | 0.125% / 0.105% |

These instruction-identical cells do not prove an additive runtime gain. The
`rxvm` control is visibly noisy and the `rxbvm` difference is negligible.
Verdict: **runtime-neutral proof/metadata slice; slice-3 gains retained**. The
earlier reported approximately 0.13%/0.146% difference came from the rejected
transient artifact capture and must not be added to the slice-3 improvement.

## Host and build

- Branch: `develop`; dirty scope limited to the approved slice-4 files and
  evidence before commit.
- Host: Darwin 25.5.0 arm64, Apple M5.
- CMake 4.3.2, Ninja 1.13.2, `CMAKE_BUILD_TYPE=Release`,
  `CREXX_VM_PROFILING=OFF`.
- Final Release `rxc` SHA-256:
  `168f2e0a4e1c215e93c7037a059c73a6c5b0b757a12bd48a058151dbf2f6688b`.

## Implementation identities before commit

- `compiler/rxcp_inline_analysis.c`:
  `a9393943bab7c6b0991cbcea4b4790cc343b797dabbbd477f3dd95263a5ed34e`
- `compiler/rxcp_inline_bind.c`:
  `544ea81d66b0934960278d69ec684aaaecee56eaffb7be816a926ed28dc99435`
- `compiler/rxcp_inline_payload.c`:
  `28505529ade17cb0a37832409d6f60186ecde1f993ab4f8b08af4330c9867b42`
- `compiler/rxcp_sym.h`:
  `89ac5fb9d47b6ab7b45eae1a3749c31513f23f1a0a25285f03fe833b4b9e5216`
- `compiler/rxcpfunc.c`:
  `ebc592ca2d7b727207457d267e9797fdf6a135d2593a5d37da9406aa6c2bfd95`
- `compiler/rxcpsymb.c`:
  `55f1e4aa74a504f49dd97c1ad67c45d2134a1b7a27d9a776187420969910795c`

Slice 4 was committed independently as `26f4aeb6f`. Execution then paused
before Slice 5 as required; Adrian later authorized, accepted and closed that
separate slice at `d1c5245d4`.
