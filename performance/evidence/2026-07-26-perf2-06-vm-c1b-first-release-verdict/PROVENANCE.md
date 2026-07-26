# Provenance

## Source and products

- Branch/worktree baseline: `develop` at
  `e7090198e45002a6a73b654f6d98b9eb91d2e5cb`.
- Candidate source: isolated detached worktree at the same commit plus only
  `interpreter/rxvmintp.c` and `interpreter/rxvmintp.h` changes.
- Candidate patch SHA-256: `5d3ae9c6dd67555ad039de8ffe785c4c04231a79f8413157fa61d82d8a19aae4`.
- Candidate C SHA-256: `a6f6af6d30334f0d8664d3909d9d25fe5081bfa172f47f33e7577ee4726e46e0`.
- Candidate header SHA-256: `34e38cfc7453205b7a8ebf0e5ea7d394a9b15512e7c2ad53a4ec69704f61a231`.
- Configuration: CMake `Release`, `-O3 -DNDEBUG`,
  `CREXX_VM_PROFILING=OFF`, Ninja 1.13.2, Apple clang 21.0.0.

| Artifact | SHA-256 |
| --- | --- |
| baseline `rxvm` | `24f18bc5aad49299fbf2b0bfae714c82d83a4be93252b8cf6f2fdfef3b77f31a` |
| candidate `rxvm` | `95f6914442c43bfd553c31846876897a30079862bcd4fc981607ab2da0f0f02f` |
| baseline `rxbvm` | `b4560b04d91aa8ad7f99c6c3cbd74507978522e733860dafd1e1d76137960113` |
| candidate `rxbvm` | `71129131c9908e526164b18c1319ebba3a716428c8d24f613562d0b21da4574b` |

Exact retained inputs:

- List optimized RXBIN: `6a0b52d8da3930b2edafc849c83faffa7a8e855d96fef4e43d8d8d52784ca0c4`;
- Permute optimized RXBIN: `bd7bc9c4d4b09a5a582e7666e4a5d5ef3b48eb35c9135cd6d2dfd07f1a00e6ae`;
- Sieve optimized RXBIN: `75d264a9acb4f17aab6c85e1a2991904580950ec6cd4da1c235d1010319e4f7f`;
- library RXBIN: `a9eee54dfeacd0631271841f69481c0bffcd1288f3c0dcc0ce7f60ef1d8f260f`.

No public RXAS, RXBIN, ABI, compiler, assembler, linker or language file
changed.

## Host and capture

- Darwin 25.5.0 arm64, Mac17,3 Apple M5, 10 logical CPUs, 24 GiB RAM.
- AC attached at both captures; no recorded thermal, performance or CPU-power
  warning.
- Pre-capture load: `3.25 4.31 4.15`; post-capture load:
  `3.38 4.19 4.12`.
- Capture completed by `2026-07-26T17:51:23Z`.
- One warmup and 12 balanced recorded rounds per cell; 144 passing recorded
  executions and no exclusions.

The maintained Level B matrix runner SHA-256 is
`77cfd1d1b4532b545d19f8313ac74567211834ca4fd7e2537a493c8f40bbceab`.
The evidence-local Level B paired reducer SHA-256 is
`d23f4bd1057bba3404ce85a9a581abf2b513c09a9606a2e4795325f3a156f7b8`.

## Accepted closeout identity

- Full Debug product rebuild and CTest: 1,924/1,924 passed.
- Full ordinary Release product rebuild and CTest: 1,924/1,924 passed.
- Closeout Release cache: `CMAKE_BUILD_TYPE=Release`,
  `CREXX_VM_PROFILING=OFF`.
- Closeout main-tree `rxvm` SHA-256:
  `3f7cfbb9973f7df79df7d9dbbca5c35ac27aa81e777c1862aca5b7cd9ddd10e9`.
- Closeout main-tree `rxbvm` SHA-256:
  `7339b8cfbb0505264177648130773f9508cebb95c109e138f9d449c3cb8e3372`.

The closeout products confirm the committed tree but are not substituted for
the isolated timing products above; build path and identity differ. The exact
implementation commit is recorded in the follow-up ledger as
`TO_BE_RECORDED`.
