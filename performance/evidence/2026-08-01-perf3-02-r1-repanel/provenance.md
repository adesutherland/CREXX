# Provenance and replay

## Identity

- Repository: `/Users/adrian/CLionProjects/CREXX`.
- Branch/HEAD: `develop` at
  `e38e514bf611ae3873513368c44742e2ae7332d1`.
- Upstream: `origin/develop` at
  `21fdcf529d0e51ea264bf0c92ccfbdc06dea8200`.
- Build: CMake 4.3.2, Ninja 1.13.2, Apple Clang 21.0.0, `Release`, `-O3
  -DNDEBUG`, arm64, profiling off.
- Locked P1 `rxas`: SHA-256
  `09ea8e49c54dfc839833a1fa40dfa5809467aa53c730dc050ae0a2b95f73669e`.
- Locked P1 tracked diff: SHA-256
  `b05623eef33dfcd30fb330f264a2e7b01df7cba94ac0f565d7c916f5bc3efbc6`.
- Compiler option source diff: [`compiler-option-source.diff`](compiler-option-source.diff),
  SHA-256
  `7bf1f32e61b1fb40bbdae6d1f12cd9fb36ca2825c48270a51fabee289252fdbf`.

The dirty worktree contains the approved P1 and C1b work, this option replay,
its tests/docs/evidence and five protected pre-existing lifecycle RXBINs. No
public compiler option, metadata schema, RXAS, RXBIN or ABI change was used.

## Variant build

All eight `rxc` products came from the same source and common Release archive.
The ordinary `rxcp_inline.c` compile command from `compile_commands.json` was
replayed with only
`-DRXCP_PERF3_RECEIVER_OPTIONS=<0|1|2|3|4|7|8|14>` added. The resulting one
object replaced `rxcp_inline.c.o` in an isolated copy of `librxclib.a`, and the
ordinary Ninja `rxc` link command was replayed with that isolated archive.

This method is self-checking: mask 2 produced SHA-256
`0323b70207c22486896d6883a80ca4388f6a4c164d03737bcefc3a3a8ed6be52`,
exactly matching the ordinary Release build and the accepted C1b correctness
product. Exact product hashes and option-image hashes are in `variants.csv`.

Each variant compiled optimized and no-opt Richards/Towers plus optimized
Permute/Bounce/Sieve. The locked P1 assembler assembled every RXAS. All no-opt
Richards images are identical at
`d18ff4e622d02748a892b8dcb812d62ceb0b1a0dae612cb56f16ac33308b5eac`;
all no-opt Towers images are identical at
`50f5f4e01da31481fcd8b1eb75122885a447033def9ef9eff2b4b09991995c76`.
Permute, Bounce and Sieve are also variant-identical.

## Timing

The existing Level B cross-runtime matrix runner used `manifest-v1.txt` with
one warmup and 12 recorded serial rotated rounds. It wrote every raw sample and
output under `run-v1/`. Candidate elapsed effects are computed per workload,
VM and recorded round as `(candidate / same-round C0 - 1) * 100`. Quartiles use
linear interpolation and the mean intervals use the two-sided 95% Student-t
critical value for 11 degrees of freedom. No sample was removed.

The retained 2026-07-31 clean-host candidate effects remain historical timing
authority. Fresh C0 cells are same-session drift and pair controls only.
