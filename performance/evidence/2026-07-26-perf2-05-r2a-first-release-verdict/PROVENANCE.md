# Provenance

## Source and scope

- Branch: `develop`.
- HEAD: `537d3b3d276606767535ecb84ad2a3c80073e5dd` (`perf: generalize constant call evaluation`).
- `origin/develop`: `d1c5245d49c0bd9cc48a7d33ef16f2f4555cc986`.
- Initial relation: `+4/-0`.
- Approved slice: exact process-local recognition of canonical
  `LINKATTR1_REG_REG_INT; COPY_REG_REG; UNLINK_REG`, guarded direct
  reference-descriptor materialization, and complete canonical fallback.
- Integrated `interpreter/rxvmintp.c` SHA-256:
  `c494037881f7f206bb0a53a7fe36d85bcd02c757049e0d1771877b2d1d4ff57a`.
- New RXAS guard SHA-256:
  `5668e3880bb2e192231f556961c1d8baf340d23f528804915a8a7a0b59898079`.

The pre-existing uncommitted P05-SA1 worklist, roadmap and semantic-assist
evidence package were preserved. No public opcode table, assembler, compiler,
linker, disassembler or serialized format file changed.

## Products and inputs

Both products use CMake `Release`, `CMAKE_C_FLAGS_RELEASE=-O3 -DNDEBUG`,
`CREXX_VM_PROFILING=OFF`, Ninja 1.13.2 and Apple clang 21.0.0. The production
build is fresh and external under
`/var/folders/nr/7ckzqpl91kz80mcy3316h1tr0000gn/T/crexx-perf2-05-r2a.XXXXXX.36Q7ETgP23/release`.
The retained valid baseline is the clean detached build under
`/tmp/crexx-perf2-05-sa1.MzdvHT/baseline-release`.

| Artifact | SHA-256 | File bytes | Mach-O `__text` |
| --- | --- | ---: | ---: |
| Q0 `rxvm` | `840f29837df7797fe357eec5e81e96fbe73a5a9c547cc5797f53f9d4d8d38254` | 982,264 | 806,716 |
| R2a `rxvm` | `21f14f90c19c7160e4d14b1c3a088481a5d20af717f74232397244a084486e58` | 982,264 | 806,076 |
| Q0 `rxbvm` | `141cc0384ddf21f391d1f401fa89f57fb41cd319d2c3f380aa5428c36877e05a` | 982,440 | 803,096 |
| R2a `rxbvm` | `c4a74cb5167b72cfb97623dd9f423659b8a399186fabc312583497fafbb808a7` | 982,440 | 803,868 |

File size is unchanged. Text changes are -640 bytes (`rxvm`) and +772 bytes
(`rxbvm`), both below the 5%/4 KiB escalation threshold.

Exact retained inputs:

- optimized List RXBIN:
  `6a0b52d8da3930b2edafc849c83faffa7a8e855d96fef4e43d8d8d52784ca0c4`;
- library RXBIN:
  `0095a9073fad7aeec923705c5a9d50c17d14174fcce96128334b3fe19b34ed8a`;
- new canonical guard RXBIN:
  `84089edaf6ef722e3732f7677bd3d3ad23d3d4ebfa36e9365a10e9c1ec54a01d`.

## Correctness

- Final focused Debug dual-VM suite: 10/10 pass.
- Compiler/import/optimized/no-opt reference matrix: 49/49 pass.
- Ordinary Release guard: pass on integrated `rxvm` and `rxbvm`.
- Backward compatibility: the same new guard RXBIN passes on clean Q0 `rxvm`
  and `rxbvm`.
- Forward compatibility: the retained accepted List RXBIN/library pass on both
  integrated VMs.
- Accepted closeout: full Debug and ordinary profiling-off Release builds pass;
  broad CTest passes 1,922/1,922 in both configurations.
- Per the approved closeout path, sanitizer, install/package, cross-platform,
  expanded-portfolio and repeated-baseline work were not required or run.

## Host and capture

- Host: Darwin 25.5.0 arm64, Mac17,3 Apple M5, 10 logical CPUs, 24 GiB RAM.
- Power: AC attached; low-power mode 0.
- CMake 4.3.2; Apple clang 21.0.0; Ninja 1.13.2.
- Formal capture: 2026-07-26T12:10:49Z through 12:10:53Z, one warmup and 12
  recorded rotated pairs per VM workload.
- All 48 formal recorded executions passed; no sample was removed or replaced.

The maintained Level B matrix runner SHA-256 is
`77cfd1d1b4532b545d19f8313ac74567211834ca4fd7e2537a493c8f40bbceab`;
its self-test passed before capture. The evidence-local Level B paired reducer
and recursive package checksums bind the final retained result.
