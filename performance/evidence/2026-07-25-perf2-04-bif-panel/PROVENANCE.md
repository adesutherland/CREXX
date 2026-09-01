# PERF2-04 evidence provenance

## Repository identity

| Field | Exact value |
| --- | --- |
| repository | `/Users/adrian/CLionProjects/CREXX` |
| branch | `develop` |
| HEAD | `6567f0ba23f20623e01322f5a62323b2347ab09d` |
| subject | `docs: close PERF2-03 and hand over PERF2-04` |
| upstream | `origin/develop` at `d1c5245d49c0bd9cc48a7d33ef16f2f4555cc986` |
| ahead/behind at start | `+1/-0` |
| accepted production predecessor | `d1c5245d49c0bd9cc48a7d33ef16f2f4555cc986`, `perf: complete proved receiver inlining` |
| HEAD-to-predecessor delta | 12 paths, 413 insertions, 32 deletions; documentation/evidence only |
| binary diff SHA-256 | `7eaf1ba251515bb830cf95a07189975e40e0ac64784a6a23207eb9a76d3d66a3` |
| starting worktree | clean |

The PERF2-03 retained checksum manifest was replayed before the PERF2-04
activity: 36/36 entries matched.

During the original decision-package capture, the only ordinary worktree scope
was the PERF2-04 roadmap, worklist and this new evidence directory. No
production source, build script, ABI, instruction table or serialized artifact
was edited in that phase. Adrian subsequently accepted every provisional
slice and the complete ladder was packaged atomically as production commit
`f8f34092eed34812950dd591525e7d927dc0d88a`. Each stable slice retains its own
incremental first-verdict evidence. No public instruction, serialized format,
ABI or VM implementation was added.

## Host and power state

Start capture on 2026-07-25:

| Field | Value |
| --- | --- |
| host/model | `Mac.lan`, `Mac17,3` |
| architecture/CPU | Apple arm64, 10 logical CPUs, Apple M5 |
| memory | 24 GiB |
| OS | macOS 26.5.2 build 25F84; Darwin 25.5.0 |
| power | AC attached, battery 80%, AC/Battery low-power modes both `0` |
| start load | `3.06 2.22 1.81` |
| storage | approximately 632 GiB free on the data volume and `/private/tmp` |

Interim post-capture snapshot at `2026-07-25T08:44:07Z`: AC attached, battery
80%, low-power mode `0`, no thermal/performance warning, load
`1.80 2.01 2.09`, approximately 629 GiB free. Each maintained timing matrix is
serial and rotates cells by round; profiler runs never overlap a product
throughput claim.

Final package snapshot at `2026-07-25T16:48:45Z`: AC attached, battery 80%,
AC and battery low-power modes both `0`, no recorded thermal or performance
warning, load `2.09 2.04 2.06`, approximately 627 GiB free, and no workload VM
or evidence driver remained active. The final combined matrix was likewise
captured on AC at 80% with no warning.

## Toolchain and exact product hashes

| Tool/product | Identity |
| --- | --- |
| Apple clang | 21.0.0 |
| CMake | 4.3.2 |
| Ninja | 1.13.2 |
| Git | 2.50.1 |
| cREXX version | `crexx-1.0.0-beta.3+local.g6567f0ba23f2` |
| B0-R `rxc` | `900c2ba2229632c74da2a00cc313efa517beeb13ed8ab58f7e0e1afb41bad857` |
| B0-R `rxas` | `80d3ff3e5b28e7132158c1b186513755457baf1472c2edd653874005a2648fc4` |
| B0-R `rxlink` | `679b5d5ed756e0cdf0c3985e400da8130b3fc49017078e62a51292ba7a31042a` |
| B0-R `rxvm` | `aab099d2f1e52f09976002935b21b189c104200f7a4b4155c65eef6eb21ac1d4` |
| B0-R `rxbvm` | `a4a61df9cceac8a0178ef5583835953f55f882f7b4bab29c754d3c41aed87b5f` |
| B0-R `library.rxbin` | `d4b35ddefa1b7d6711788b38dc33d0b66ff8a1af43690f2367c98a8ee5f7fcf1` |
| B0-P `rxvm` | `44705d0c9d39a659f80350fbefb7d3f51019a7cf1ae4dc72f453c88e7468da93` |
| B0-P `rxbvm` | `9ae6721988a5605d29433129e9bcdad95f9d1c4ca9c366d5dcf237c892484795` |

## Independent scratch products

Scratch root: `/private/tmp/crexx-perf2-04.3k3Dfw`.

| ID | Source/build | Configuration/purpose |
| --- | --- | --- |
| B0 source | `baseline-src` | detached exact HEAD |
| B0-R | `build-release` | Ninja Release, `-O3 -DNDEBUG`, `CREXX_VM_PROFILING=OFF` |
| B0-P | `build-profile` | Ninja Release, `-O3 -DNDEBUG`, `CREXX_VM_PROFILING=ON` |
| CAS-L0 | `upper-inline-src` | exact current UPPER body after symbol actual materialization |
| CAS-H1 | `upper-direct-src` | direct dynamic-input `strupper` result ceiling |
| CAS-L2 | `upper-const-src` | exact-site UPPER constant ceiling |
| LEN-H1 | `length-direct-src` | direct `strlen` result ceiling |
| SLC-H1 | `substr-direct-src` | exact-site existing-primitive composition ceiling |
| SLC-CF1 | `substr-const-src` | exact-site `SUBSTR` constant ceiling |
| WRD-L0 | `word-ceiling-src` | inlined reusable first-word predicate helper control |
| WRD-H1 | `word-predicate-first-src` | exact-site direct first-word predicate ceiling |
| WRD-CF1 | `word-constant-false-src` | exact-site propagated false predicate ceiling |
| CF-COMB1 | `rexxcps-combined-src` | separable cumulative CAS-L2 + SLC-CF1 + WRD-CF1 ceiling |
| P04-CEX1 | live provisional worktree plus retained SLC1 `rxc` | exact `LOWER`/`LENGTH`/`LEFT`/`RIGHT` certificate and Release verdict |
| P04-WRD1 | live provisional worktree plus retained CEX1 RexxCPS snapshot | exact constant `WORD` certificate, general pre/post fold ordering and Release verdict |

Each worktree is detached at the exact HEAD and has a separately retained patch
and generated artifact identity. No candidate overwrites B0-R or another
candidate image.

## Capture control plane

Maintained analysis/orchestration uses existing cREXX Level B tools:

- `performance/tools/run_evidence_bundle.crexx` captured the complete current
  optimized/no-opt workload census, ordinary Release correctness/timing sample,
  profile counts and N=2/3/4 RXSEQ evidence.
- `performance/tools/run_cross_runtime_matrix.crexx` captured serial ordinary
  Release wall matrices with explicit manifests, correctness text, warmups and
  raw samples.
- the retained performance inventory tool produces the final bundle checksum
  manifest.

Shell/RXAS inspection was diagnostic only. No Python census, analysis or
orchestration program was introduced. The C Base64 program is a deliberately
fixed-valid native ceiling, not a maintained PERF2-04 control plane or product
candidate.

The two current census bundles contain 22 exact-image entries each: all eleven
governed Tier A workloads in optimized and no-opt form. Their own
`checksums.sha256` manifests replay successfully. The one ordinary sample per
image is census qualification, not a full formal portfolio.

The independently rooted retained manifests replay as follows:

| Evidence block | Verified entries | Manifest SHA-256 |
| --- | ---: | --- |
| current `rxvm` census | 541/541 | `77ed88daf3287e039e9c4c96e74f2ac24c769ca0cca1258c8beabac06fc31354` |
| current `rxbvm` census | 541/541 | `85798fca42a0fcab38f2d5ba95062ccc78be676cc1d08012f6780dbe1fa39ba5` |
| LENGTH panel | 26/26 | `126c4ce305e3432c0cebcfbc2ad91da27ab963f10aa8aa8fa0caa4c4d98e292c` |
| SUBSTR direct ceiling | 25/25 | `4b80f1e7962a7a6a705a05a73770b4dd1ac59ffdc2d6dbbc2627b6926e4d2b9e` |
| SUBSTR constant ceiling | 34/34 | `1b4c00648f88a07e1df94b0a6070bfef47e9fb645c604dc02a0767210c8eef33` |
| WORD panel | 93/93 | `53fae7da854b725e69ec829060f7c84e53d985fe559ba20601ce755a27e8dba2` |
| combined selected ceiling | 49/49 | `4db920829b5795a459a624f7b81da2e82e90e45db383ee73d7ef671de3133acc` |
| P04-SLC1 first Release verdict | 11/11 | `98dc650f0d885cdee7a2c2c8b584c915c516c40bf9618f38611a7b38da58a0fc` |
| P04-CEX1 first Release verdict | 11/11 | `c6eaff910e05432d98d1d2bac94dbb1bb92e7e8f9f3d9eb543d7781b2af48485` |
| P04-WRD1 first Release verdict | 12/12 | `2c48472d9e48217ea876f081fd3c691aa9e407e9f74ccde8e7a46cef52ac53ee` |

## P04-WRD1 first-verdict product

The frozen ordinary Release product uses `CMAKE_BUILD_TYPE=Release` and
`CREXX_VM_PROFILING=OFF`. It was measured on AC power at 80% battery, with both
low-power modes disabled, no thermal/performance warning and pre-capture load
`1.50 3.25 3.95`. The capture ran from `2026-07-25T20:44:00Z` to
`2026-07-25T20:44:24Z`.

| Product | SHA-256 |
| --- | --- |
| P04-WRD1 `rxc` | `350b8d7d02b7b938a4d84fade97b9c3ccecd7cd80b6e0081b041022b9ca3eeaa` |
| unchanged `rxvm` | `225952dbd23a56baffd032461977d8a686866dfbef77613ac8bbd40ebe615815` |
| unchanged `rxbvm` | `0a880d163f0fb3fc653ce42adc48356e3390f12b36eeccb104b251865cf3b144` |
| unchanged `library.rxbin` | `221727fccba50f74a0da57a0ae0dcd94241f21bfe6d5cc60145a767a41f0aab4` |
| P04-WRD1 RexxCPS RXAS | `815b277269430de5b011d32f8fb4e113358e1be75467b3a01be3909a7e05ae51` |
| P04-WRD1 RexxCPS RXBIN | `208874392b8f4b41b5627f7e6ddb97a5d79ad6c586d4ab1b8a0d3b3bd6a136c2` |

The accepted CEX1 RexxCPS RXAS/RXBIN are byte-identical to accepted SLC1 and
were snapshotted before rebuilding. The retained SLC1 medians are therefore a
valid exact predecessor baseline. WRD1's maintained serial matrix used two
warmups and seven recorded samples per VM; all 18 invocations passed, stderr
was empty and neither cell requested a rerun.

## Evidence interpretation rules

- B0-P timing is attribution only. It never supports a throughput claim.
- B0-R ordinary profiling-off Release wall clock is the product-facing metric.
- `rxvm` and `rxbvm` are separate cells; no mixed-VM aggregate selects a design.
- machine-work advancement precedes wall timing: mathematical proof plus fewer
  instructions, scans, copies or allocations is required.
- source rewrite controls are not production implementations. TRACE/source,
  signal, evaluation-order and alias gaps are listed next to every result.
- native controls bound residual work. They do not automatically select native
  ownership.
- the accepted broad closeout belongs to [`CLOSEOUT.md`](CLOSEOUT.md). No full
  formal portfolio or push belongs to this package.
