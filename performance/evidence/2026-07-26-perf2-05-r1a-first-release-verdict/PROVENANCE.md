# Provenance

## Source and scope

- Branch: `develop`.
- HEAD baseline: `e9ecd880364f4ccc704e1596360128f52f2d52b7`
  (`perf: materialize reference descriptors privately`).
- `origin/develop`: `d1c5245d49c0bd9cc48a7d33ef16f2f4555cc986`.
- Initial relation: `+5/-0`; initial worktree clean.
- Approved slice: exact process-local recognition of adjacent
  `UNLINK_REG; LINKREF_REG_REG` with the same destination and a distinct source,
  preserving unlink-first state and complete canonical fallback.
- Integrated `interpreter/rxvmintp.c` SHA-256:
  `775f09f76f8fd6b4fab5fd4b7545f89a2b30a9132514d90c9c8d1f33eec731cc`.
- Integrated `interpreter/CMakeLists.txt` SHA-256:
  `48b0255df28152bfc6960a3421341b17ae1c1fa51fe432de2c55f8a39ebd2d58`.
- New RXAS guard source SHA-256:
  `3183812a662e076c9acd2be221ab18e741e2d02d2b06027ae4ebff8ba8f8ffcf`.

No public opcode table, assembler grammar, compiler lowering, linker,
disassembler, serialized format or ABI file changed.

## Products and exact inputs

Both products use CMake `Release`, `CMAKE_C_FLAGS_RELEASE=-O3 -DNDEBUG`,
`CREXX_VM_PROFILING=OFF`, Ninja 1.13.2 and Apple clang 21.0.0. The preserved
accepted R2a product is under `/tmp/crexx-perf2-05-r1a.xbBPgs/baseline/`; the
fresh corrected R1a build is under
`/tmp/crexx-perf2-05-r1a.xbBPgs/candidate-release-v2/`.

| Artifact | SHA-256 | File bytes | Mach-O `__text` |
| --- | --- | ---: | ---: |
| R2a `rxvm` | `663de379bd1239e446a42e5e2d269319bf9ae1eb98c487d058c2f8cfc5d9bb9b` | 982,264 | 806,076 |
| R1a `rxvm` | `512acce470a7acb4257fd76ceb346a2c14b0aac7ae7103be866d40ace8a108fd` | 998,776 | 810,700 |
| R2a `rxbvm` | `63f9b9fbec1ada324b580e950acde3eada0442861f516984e59c1f21d71f5eb4` | 982,440 | 803,868 |
| R1a `rxbvm` | `bd0e22869583641e9541716fa62cc93eaf670a38c7c0964406bc1941c527e583` | 982,440 | 804,704 |

Exact retained inputs:

- optimized List RXBIN:
  `6a0b52d8da3930b2edafc849c83faffa7a8e855d96fef4e43d8d8d52784ca0c4`;
- library RXBIN:
  `0095a9073fad7aeec923705c5a9d50c17d14174fcce96128334b3fe19b34ed8a`;
- new canonical guard RXBIN:
  `05bdadd7d046b408736c32ab6fbd7f493e02da7f75f0f21f20ee986d2834b46a`.

## Host and capture

- Host: Darwin 25.5.0 arm64, macOS 26.5.2, Mac17,3 Apple M5, 10 logical CPUs,
  24 GiB RAM.
- Power: AC attached; low-power, thermal and performance warnings absent at
  both host captures.
- Formal capture window: 2026-07-26T13:48:05Z through 13:49:06Z.
- Three sequential captures each used one warmup and 12 balanced recorded
  pairs, giving 36 pairs and 144 passing recorded executions in total.
- No sample was removed, replaced or reclassified.

The maintained Level B matrix runner SHA-256 is
`77cfd1d1b4532b545d19f8313ac74567211834ca4fd7e2537a493c8f40bbceab`;
its self-test passed immediately before capture. The evidence-local Level B
paired reducer SHA-256 is
`85ac21559a0a518482caa7d2ded3ea878d0b6ea481c26b42530b9b805fb2350f`.

The pre-correction candidate and its timing data were excluded completely from
this package. They remain recoverably isolated at
`/tmp/crexx-perf2-05-r1a.xbBPgs/superseded-direct-signal-verdict/`.

## Accepted closeout validation

After Adrian accepted the first Release verdict on 2026-07-26:

- the complete Debug product rebuilt without warning/error diagnostics;
- focused Debug validation passed 12/12 plus 49/49;
- broad Debug CTest passed 1,924/1,924;
- the complete ordinary profiling-off Release product rebuilt without
  warning/error diagnostics;
- focused Release validation passed 12/12 plus 49/49; and
- broad Release CTest passed 1,924/1,924.

The closeout did not change the measured implementation or rerun the accepted
timing baseline. No sanitizer, install/package, cross-platform or expanded
portfolio claim is made for this bounded slice.
