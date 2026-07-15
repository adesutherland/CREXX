# Cross-runtime benchmark ports

These sources support NR-02 qualification. Canonical sources stay unchanged;
runtime-required adaptations use distinct files and are classified in
`performance/NR-02-WORKLIST.md`.

## RexxCPS

- `rexxcps/rexxcps_2_2.rex` is the exact official ooRexx 2.2 source captured
  from the upstream sample repository on 2026-07-15. SHA-256:
  `b86b1232a3747bacdeba64da16eb79cb0e0115bc3d91da3c2b0b08a80772c8f4`.
- `rexxcps/rexxcps_2_1n.nrx` is the exact example bundled with NetRexx
  5.10-GA. SHA-256:
  `9aa47a25f9aff0085ad8a2600fbf8785b772347b8c9b29427050ae85d93e6dbd`.
- `rexxcps/rexxcps_2_2n.nrx` is a disclosed adaptation that retains the
  bundled NetRexx timed kernel and adds the current 2.2 minimum-duration
  calibration. It is not bundled canonical 2.1n and is not canonical Classic
  2.2.
- files containing `_opaque` are diagnostic variants for opaque inputs,
  deterministic result observation, controlled perturbation and trace work.
  They do not replace or produce the canonical community score.

Opaque A and B execute the same timed clauses and branch outcomes but select
different compound-stem keys, digit/text values and false-comparison operands
from the process argument before timing. Both must observe
`variant|1|69|1.22694`. They are paired specialization/dead-work diagnostics,
not baseline/optimized modes; the exact field mapping is retained in the dated
NR-02 RexxCPS ledger.

The RexxCPS sources retain their original provenance and redistribution terms;
see `../LICENSE-REXXCPS-CPL-1.0.txt` and `../THIRD_PARTY_NOTICES.md`. The
NetRexx distribution also carries its bundled `LICENSE` (ICU license).

## NetRexx seed ports

`netrexx/` ports the four AWFY seed workloads. They retain the same algorithms,
workload sizes and observable checks as the Level B sources. Primitive Java
arrays/numbers and NetRexx classes are language-required representations. The
generated Java/class evidence, JDK settings and pilot outputs belong in the
dated NR-02 evidence bundle rather than beside the versioned sources.

## Classic seed ports

`classic/` contains procedural ports for the ooRexx lane. Regina surrogate
checks are not Regina portfolio results and do not qualify an ooRexx cell.
Sieve and Permute retain the algorithm and observable work and qualify on
ooRexx 5.1.0. Mandelbrot uses an exact arithmetic byte-XOR helper because a
portable Classic source-level integer XOR spelling is not shared with the typed
source, but ooRexx decimal arithmetic also changes the common size-500/750
checksums; the current cell is therefore `not comparable`, not a runtime-
specific accepted answer. Towers represents allocated disk nodes with numeric
ids and stems; it preserves recursion/link mutations but not object dispatch or
allocator pressure, so its correct ooRexx run remains a disclosed diagnostic
and is `not comparable` for the common object/allocation score.
