# Provenance and live freeze

## Git identity

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch: `develop`
- live source HEAD: `b08611179db5ff4257c3be3103f3aeab55ea5b50`
- upstream: `origin/develop` at
  `53f7757c5b21c15d405b17920d4cd7f6c554c46b`
- accepted VM-C1b commit:
  `a39608426e2c1bb84d5fc0c4f767f4c9492339a9`
- PERF2-06 tactical closeout:
  `d4dd0d84a5093f5530a86e63b75a943e707eaed1`

The main worktree was clean at the live freeze. HEAD was the later
documentation-only combined-plan commit and was one commit ahead of the
accepted upstream. The correctness and evidence changes in this package are
the only main-worktree changes made by this activity. Existing linked
worktrees, including dirty rejected-PoC trees, were not modified; the complete
snapshot is in [`WORKTREES.md`](WORKTREES.md).

## Apple host and session state

Recorded at 2026-07-27T10:57:36Z:

- MacBook Air `Mac17,3`, Apple M5, ARM64;
- macOS 26.5.2 build 25F84, Darwin 25.5;
- 10 logical CPUs and 24 GiB physical memory;
- AC attached, battery 80%, not charging, low-power mode off;
- no thermal or performance warning; load averages 3.60/2.93/3.40;
- Apple clang 21.0.0 (`clang-2100.1.1.101`);
- CMake 4.3.2, Ninja 1.13.2 and Git 2.50.1.

No serial number or hardware UUID is retained.

## Independently identified products

| Role | Build root | Relevant options |
| --- | --- | --- |
| clean original control | `/private/tmp/crexx-perf2-0607.HOrlKC/build-release` | Release, ordinary product |
| corrected ordinary product | `/private/tmp/crexx-perf2-0607.HOrlKC/build-v3-fixed` | `CMAKE_BUILD_TYPE=Release`, `BUILD_TESTING=ON`, `CREXX_VM_PROFILING=OFF`, Ninja, native ARM64 |
| corrected count/RXSEQ product | `/private/tmp/crexx-perf2-0607.HOrlKC/build-v3-fixed-counts` | Release, `BUILD_TESTING=ON`, `CREXX_VM_PROFILING=ON`, Ninja, native ARM64 |
| representation trace diagnostic | `/private/tmp/crexx-perf2-0607.HOrlKC/build-v3-trace` | independent diagnostic only |

An early directory named `build-v3-fixed-profile` was configured with the
irrelevant `CREXX_ENABLE_PROFILING` cache variable while the real
`CREXX_VM_PROFILING` option remained off. It was detected before attribution,
discarded and is not used anywhere in this package.

Corrected ordinary product identities:

| Product | bytes | SHA-256 |
| --- | ---: | --- |
| `rxvm` | 998,904 | `8b54d9f0116d66ea74fd5367241f8f787dfad9fa61a36b761cae9f88ecd4d508` |
| `rxbvm` | 999,064 | `3468d9c0f5006e4dcdf04f436611a881810766532723c186c69eab12b66e05e8` |
| `rxc` | 2,802,496 | `dd59a29e87035fb0215ffc94fb82ec18bc79324bdbec368ac24d7d153544395b` |
| `rxas` | 579,736 | `b858ae916721baad5386fa826208b671ca47da352c8246be25302c5a580bf843` |
| `rxlink` | 145,416 | `b3cdedeebb096e2b9513c86f0c2e5fb20c53dda336f7a650c2b43dc2d08b2625` |
| `library.rxbin` | 862,920 | `71e13d66ca5da6c5cc7998f1a7ced0be9e33ac4ec92b5950179d8b453899a15a` |
| `classlib_native.rxbin` | 25,148 | `459ade9c12a6641840cb6aa38067303053b3e12f78dceb49f7b0eb6071ebfe4d` |

The count product hashes are
`df05ef1efb399f40eb02e5b0849331c2167f741176cdec7c104c0232f7966e52`
(`rxvm`),
`92fde641e6936370a54709daeaf6ca3fe0ec2037af2a71374587bfdd40f0366d`
(`rxbvm`) and
`79413938bf29595387d0842b96fe457bf2ecd0b8a14b94c5d6ca0c3548236308`
(`rxseq`).

## Apple ARM64 layout freeze

- `value`: size 248, alignment 8.
- `stack_frame`: size 168, alignment 8.
- side structures: `native_payload_ops` 24, `reference_cell` 80,
  `reference_context` 544, `interrupt_entry` 40,
  `interrupt_state_saved` 56, `numeric_context` 20 and `decplugin` 272 bytes.

Important `value` offsets are: status 0, integer 8, float 16, decimal 24,
decimal length/capacity 32/40, string 48, string length/capacity/position
56/64/72, codepoint count/position 80/88, binary 96, binary length/position/
capacity 104/112/120, native ops/flags 128/136, reference identity/payload
144/152, object type 160, attributes 168, unlinked attributes 176, buffers
184, maximum attributes 192, numeric context 200, numeric buffers 208 and
small buffer 216.

Important `stack_frame` offsets are: previous 0, parent 8, procedure 16,
return PC/register 24/32, local/nominal counts 40/48, arguments 56,
reference-lifetime bit 64, interrupt state 65/66, caller argument base 68,
interrupt table 72, owned values 80, saved stack 88, numeric context 96,
decimal plugin/loaded flag 120/128, Unicode plugin/loaded flag 136/144,
base locals 152 and locals 160.

These are target-ABI facts, not a cross-platform layout claim.

## Frozen workload and image identities

The package retains each selected source plus its current no-opt/optimized
RXAS, RXBIN and linked image. Source SHA-256 values are:

| Workload | SHA-256 |
| --- | --- |
| Sieve | `395cc773ded763af39cd7ca0813582281e7f1be5768e5207f05e379e1756c18d` |
| Permute | `272178f1429f704f8b90e530b96a4e161acb627872530a277d2c7e6e9daed7e4` |
| Bounce | `9d569ff3f2995dfc8408352b5b085bc0e91e828d05c99684802677c5105284f1` |
| Richards | `cd136e6cc8bc5dd487db99abe706fc96d161b3e4b102e13b20cce69bc4692372` |
| Base64 | `b024222aff7a2e60dd7812da2c4aa54f2a9bc65b978915a2e670e74bad7d9e13` |
| RexxCPS | `2970c3d73fe2537ec8f81295c585495c4668b442d5b9a2335b1ee453a13bbdd6` |

Linked image SHA-256 values are:

| Workload | no-opt | optimized |
| --- | --- | --- |
| Sieve | `65af38f4a6eb2c4b78c01991a9966b505599669890c7b1349242f84469f7a982` | `e4a931f92d0b7274a02187c3dfde8a114af0ddd2f069d6c760b20933ff78668b` |
| Permute | `04363dccdb0afe88b3a37f2603ee74fb85d1f92c6d3728ea5f87275d69214aca` | `5d941ae1853b6221c5f33a51ee59746a301f2818d991f0922b737558d40bfbf0` |
| Bounce | `cfa35e7c1c175ba81502876401d937507b475c9d598e0175bdee84442ad87b97` | `8af293a790f577a562125f096776b7dc30ff274ec44838ed3944be3dfc044608` |
| Richards | `429bea1e1865b09af2c328ccf42e4cb8c7f873d5b3b7f2f41f381cf67c5cc4a7` | `614a40d78b7b4697cb02e8c2ab2b1c9a653ec57601c0ece3265605026a5f3e9b` |
| Base64 | `1b1175d381f4aa4431cd0ae0e3bd047b87193e5861ab8ff4305d0092171da50e` | `8d2a61c44a095ffd57520aedb65ebabf64d627a6c5f4d2fad13ea0c7d30d44a0` |
| RexxCPS | `1c8952379aa096e4a81dacde4fd758fa2116788ca0273d10004f21d2b67d886a` | `93b94fda4b46ffd24b05cab77e28cc93c78ca7248420e6d8d796c61cfbc8d873` |

The exact linked manifest SHA-256 is
`82c7e632a8a13644b36296695ca1725517d00a8ed1e8f78c70502adda32c5da4`.
The retained bundle checksum-file hashes are
`ef7839250ff7f089e8672afc4695b945f7b8056332dec182b5fd27738a35a857`
for `rxvm` and
`46d05926ccacfb1789249ee442f751e13f88693e9edc77898c3b839e189b33df`
for `rxbvm`.

## Retained evidence audit

The raw audit logs preserve every line. Results used by this panel:

| Package | verified entries | result |
| --- | ---: | --- |
| PERF2-01 current baseline | 1,948/1,948 | pass |
| PERF2-02 quickening PoC | 136/136 | pass |
| PERF2-02 first Release verdict | 23/23 | pass |
| PERF2-05 semantic-assist panel | 2,073/2,073 | pass |
| PERF2-05 R2a / R1a verdicts | 25/25 and 35/35 | pass |
| PERF2-06 VM audit | 30/30 | pass |
| VM-C1b first Release verdict | 43/44 | one documented historical-manifest mismatch |
| C2/reset rejection | 123/123 | pass from repository root, as required by root-relative manifest |
| C3 tactical rejection | 9/9 | pass |

The VM-C1b mismatch is only `CODE-LAYOUT-DEBT.md`: its manifest records the
pre-closeout file hash, while later commit `d4dd0d84a` supplemented that
historical debt note without regenerating the old manifest. The 43 retained
payload files match, and the later commit is verified separately. This is not
silently reported as a 44/44 package.
