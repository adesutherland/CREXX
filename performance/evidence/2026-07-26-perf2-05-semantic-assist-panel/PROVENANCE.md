# Provenance

## Repository and retained predecessor

- Branch: `develop`
- Source commit: `537d3b3d276606767535ecb84ad2a3c80073e5dd`
- Source subject: `perf: generalize constant call evaluation`
- `origin/develop`: `d1c5245d49c0bd9cc48a7d33ef16f2f4555cc986`
- Initial relation: `+4/-0`
- Initial ordinary checkout: clean
- Host: `Darwin 25.5.0 arm64`, Apple M5
- CMake: 4.3.2
- compiler: Apple clang 21.0.0 (`clang-2100.1.1.101`)

The accepted P05-CF1 evidence package was replayed from
`performance/evidence/2026-07-26-perf2-05-generic-partial-evaluation/` with
`shasum -a 256 -c checksums.sha256`; all 52 entries passed.

## Isolated products

All products were outside the ordinary checkout under the transient root
`/tmp/crexx-perf2-05-sa1.MzdvHT`.

| Product | Source | Build | Profiling | Identity |
| --- | --- | --- | --- | --- |
| B0 ordinary | clean detached source at the exact source commit | `baseline-release` | OFF | ordinary post-P05 product |
| B0 diagnostic | same clean detached source | `baseline-profile` | ON | post-P05 counts and RXSEQ only |
| V1/N1 PoC ordinary | separate detached source plus the retained scratch patch | `candidate-release` | OFF | public opcode spelling used only as a one-handler/native ceiling |
| V1/N1 PoC diagnostic | same patched scratch source | `candidate-profile` | ON | candidate counts and RXSEQ only |

Every build used `CMAKE_BUILD_TYPE=Release` and `CMAKE_C_FLAGS_RELEASE=-O3
-DNDEBUG`. The baseline and candidate products never shared a build directory.

### Product hashes

| Product | SHA-256 |
| --- | --- |
| baseline ordinary `rxas` | `91ea4d176cad3b2b1ed5efe743c4c6f557146610ab895408c1141b06a6d4978f` |
| baseline ordinary `rxvm` | `840f29837df7797fe357eec5e81e96fbe73a5a9c547cc5797f53f9d4d8d38254` |
| baseline ordinary `rxbvm` | `141cc0384ddf21f391d1f401fa89f57fb41cd319d2c3f380aa5428c36877e05a` |
| baseline profile `rxvm` | `a0b9de97a569b1c84dc6bcd1ce013503bf6920a2833fc412d4aa4c545ef0aece` |
| baseline profile `rxbvm` | `9644ca3e7a2763a09c9de8a7952a7bc1fb27b431d1ece6c090bba9809016a7e8` |
| baseline profile `rxseq` | `79413938bf29595387d0842b96fe457bf2ecd0b8a14b94c5d6ca0c3548236308` |
| candidate ordinary `rxas` | `c4bbdf4ef966e78bf239dfec69007888550dffc207f8eab883650b46ae1438bf` |
| candidate ordinary `rxvm` | `b303f02a14833cc4bf92f6475bbc3a218181cabf87292d11ed879f697b1fc70b` |
| candidate ordinary `rxbvm` | `be520fdda826b5cec21ab1da3a0dcd1b4672d21e9fe2c7e2a685f46db9b4cfaf` |
| candidate ordinary `rxdas` | `6d980f32a0bf81080ea90a641a3903bc66535b142063fc57e6d5b6be826fd940` |
| candidate profile `rxvm` | `be70b42eec713a3e4d89c466cdb6e6cc4c895cb98e93103462934edd13a9b801` |
| candidate profile `rxbvm` | `d713d6d2a79805ff350f7ce3b618e635d44b2b29bbf183b069847fba4981637a` |
| candidate profile `rxseq` | `1d37710570e9f2e628fcbc05ec148aa14f829f7e2203cb3a038ec2d3eb625a57` |

The clean product versions are
`crexx-1.0.0-beta.3+local.g537d3b3d2766`; scratch candidate versions add the
expected `.dirty` suffix. The retained
`poc/scratch-public-opcode-control.patch` has SHA-256
`4303b4cd2b2e0ad802c74c33f66beb685c3ef0dc1bd9a7c9b70355786acce378`.

## Evidence inputs and capture

The refreshed post-P05 exact-image manifest has SHA-256
`f5167c70b1fa13aa743b92947b7aa96ba35e80102d9167ad6708202ee4e4b94c`.
It contains the governed 11-workload optimized/no-opt pairs. Both profile
builds captured counts and RXSEQ N=2/3/4 for every entry. The copied bundle
manifests retain exact workload, library, executable and command hashes.

The PoC manifest is retained at `poc/input-manifest.txt` and has SHA-256
`485d16ccfb2f867ae1f05a923db1b57e87ee3a2eb90a7088cd690fcecd796e58`.
Each of its four independently named pairs used two serial warmups and 15
serial recorded samples on the ordinary profiling-off Release VM, followed by
one counts profile and independent RXSEQ N=2/3/4 captures. The maintained
Level B runner was `performance/tools/run_evidence_bundle.crexx`, SHA-256
`2495bc2c630e5862368e16dfebc89f459b4e3b5268357dd4aef8ed2ee9af5713`;
its self-test passed before capture.

The benchmark RXAS transformations were mechanical and are retained under
`poc/images/`. The public experimental mnemonics occupy formerly reserved
scratch opcodes only. They do not appear in the ordinary checkout and do not
constitute an RXAS/RXBIN selection.

## Correctness guard

The retained `poc/semantic-guard/semantic-assists.rxas` exercises:

- valid reference relink and write-through;
- invalid relink with unlink-first destination state;
- canonical copy of a weak-reference descriptor from an object attribute;
- `refvalid` and `deref` after descriptor copy; and
- attribute bounds failure before destination mutation.

The assembled guard passed both candidate `rxvm` and `rxbvm` with empty
stderr. All 240 recorded benchmark samples also passed their benchmark
correctness marker, and every profile/RXSEQ run passed the same marker.
