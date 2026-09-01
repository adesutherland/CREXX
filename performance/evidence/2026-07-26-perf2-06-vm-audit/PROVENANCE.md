# PERF2-06 provenance

## Repository and host

- Baseline commit: `e7090198e45002a6a73b654f6d98b9eb91d2e5cb`
  (`perf: relink exact references privately`).
- Branch at start: `develop`; upstream
  `d1c5245d49c0bd9cc48a7d33ef16f2f4555cc986`; relation `+6/-0`;
  worktree clean before the PERF2-06 worklist/evidence edits.
- Host: MacBook Air `Mac17,3`, Apple M5, arm64, 10 logical CPUs, 24 GiB.
- OS: macOS 26.5.2 build 25F84; Darwin 25.5.0.
- Toolchain: Apple clang 21.0.0 (`clang-2100.1.1.101`), CMake 4.3.2,
  Ninja 1.13.2.
- Formal timing blocks ran serially on AC at 80% battery with low-power mode
  disabled and no recorded thermal, performance or CPU-power warning. Loads
  were 3.36/2.48/2.26 to 3.13/2.52/2.28 for the first block and
  3.16/2.67/2.35 to 2.35/2.55/2.33 for the capped append.

## Products

All products use detached source trees at the baseline commit and independent
Ninja build directories. Product timing uses `Release`, `-O3 -DNDEBUG`,
`CREXX_VM_PROFILING=OFF`, `BUILD_TESTING=ON`, and the NETWORK TLS backend.

| Role | Artifact | SHA-256 |
| --- | --- | --- |
| current product | `rxvm` | `24f18bc5aad49299fbf2b0bfae714c82d83a4be93252b8cf6f2fdfef3b77f31a` |
| current product | `rxbvm` | `b4560b04d91aa8ad7f99c6c3cbd74507978522e733860dafd1e1d76137960113` |
| current product | CMake cache | `605dcce25aea9f5169d06ef65cfda1714a27ef5de5e892af82564b1ac46730a1` |
| current diagnostic | `rxvm` | `6aceaeb332525ad7be3666f51b3c2fd34db609f68a93dd6584599d55fe27ba97` |
| current diagnostic | `rxbvm` | `7f244852e3fc199d12ed7e669fb43a3791bf2c1dc8723fc1ad6a710c2acbb055` |
| COW product | `rxvm` | `a8ec9ef21e7ace511cbc5cf9b800298232591ae83a1664d7aad0d9ab87fb3e8e` |
| COW product | `rxbvm` | `58d77ab2d6a73a4449c94602bc270807b0f08da0e83edae1203009ed8e367a33` |
| COW product | CMake cache | `ee1d4bcc076f1c5599629ceb3fa75b7828bfb1a9928afc9711591aa331e42caf` |
| COW diagnostic | `rxvm` | `ada6904cb62a93e12597a156f007b348d9ac49513812e86214037eb398ed9100` |
| COW diagnostic | `rxbvm` | `6fe482546e37c78268f48e60eb93334059cdcc1bd096dc2a4c67e8b84f6bc23f` |
| current lifecycle harness | `rxvm` | `bfeb4338fe5c48cb8cec874b4db2155411e37a14109351eb6daf5100c11f8bf4` |
| current lifecycle harness | `rxbvm` | `fe4da086a14442b0397301c4f6fa1137724b4f78db11385a266919062eff125e` |

The product and PoC use the same current-head optimized inputs:

| Input | SHA-256 |
| --- | --- |
| Permute | `bd7bc9c4d4b09a5a582e7666e4a5d5ef3b48eb35c9135cd6d2dfd07f1a00e6ae` |
| List | `6a0b52d8da3930b2edafc849c83faffa7a8e855d96fef4e43d8d8d52784ca0c4` |
| Base64 | `43f98f3ea06826881c71970c8447954f4a1e84efe4ffbd53649b2893cc0e0464` |
| Sieve | `75d264a9acb4f17aab6c85e1a2991904580950ec6cd4da1c235d1010319e4f7f` |
| library | `a9eee54dfeacd0631271841f69481c0bffcd1288f3c0dcc0ce7f60ef1d8f260f` |

## COW PoC identity and guards

- Retained patch: `poc/cow-interrupt-table.patch`.
- Patch SHA-256:
  `5d7f8e36bc2b814113efb7580634112a13be0423858187f463f66cf566c521a1`.
- Diff: two interpreter files, 63 insertions and 3 deletions.
- Current product focused VM/signal/reference/late-load/instrumentation panel:
  65/65 passed.
- COW product same focused panel: 65/65 passed.
- Deterministic optimized Permute, List and Base64 profiles retain identical
  dynamic instruction, call, frame-work, value-operation and branch counts in
  both VMs. RexxCPS is time-bounded and is not used for exact count equality.
- No broad CTest, sanitizer, install/package proof, production edit, commit or
  push was performed. The PoC remains disposable and rejected in its measured
  code shape.

## Accepted evidence replayed

| Bundle | Verified rows | Manifest SHA-256 |
| --- | ---: | --- |
| PERF2-01 current baseline | 1,948/1,948 | `fde9aa923fc451ea16edc0c4f09bf532faa2e024aa22fbfc72e35a965f78eacc` |
| PERF2-02 quickening PoC | 136/136 | `f2953489e4d03ee7be8211f69182adbd406fe9fa2f0ed7e08f962cc1e5f97239` |
| PERF2-02 first Release verdict | 23/23 | `405ab71b8a0ac2ccf60f94588de46d27882da89c912f177885ac6e4a16510201` |
| PERF2-05 semantic-assist panel | 2,073/2,073 | `9a564829ecd9c4a51ec752aef0c116b5e97e6bc5082574801b62200590e99426` |
| PERF2-05 R1a first Release verdict | 35/35 | `916c6f729812a7f8d51b69282e98ec9505f5f35d50a713e2ee089fa5f3a2ae95` |

The retained summary and sample files are checksummed by the package-level
`checksums.sha256`; temporary build trees and full diagnostic profiles are not
part of the durable package.
