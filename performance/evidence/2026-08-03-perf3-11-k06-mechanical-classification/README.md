# PERF3-11 K06 mechanical classification

Date: 2026-08-03

Status: **complete; no production-code change and no push**

## Decision

Retain the existing adjacent `COPY` plus same-pair `ACOPY` rule as a mechanical
keyhole encoding. Do not move it into the immutable CFG/SSA proof service.

The equivalence is exact local algebra:

- full `COPY` reads and writes `RXOP_COMPONENT_ALL`, which includes status
  `RXOP_COMPONENT_ATTRIBUTES`;
- `ACOPY` reads and writes only that status component;
- both opcodes have a non-signalling contract;
- `copy_value()` assigns `status.all_type_flags`, and `ACOPY` immediately
  repeats the same assignment; and
- the declarative captures require the identical source/destination pair with
  no intervening executable instruction.

The metadata test now locks this subset and signal contract. The focused
fixture adds different-target, different-source and executable-gap negatives.
Generalising full-copy subsumption to every typed copy remains a separate
future capability because payload and reference/native lifetime behavior would
need its own evidence.

## Output verdict

The production assembler is unchanged. Replaying the original fixture from
baseline commit `78bd7f6f5` with the same relative source path produces
byte-identical optimized and no-opt images and the same diagnostic signature
`d132e98f`. The expanded fixture keeps exactly that one positive decision.

Richards, Towers and RexxCPS retain their exact Release hashes and record zero
K06 accepts. No ordinary output changed, so runtime timing and broad CTest are
not warranted.

## Validation

- metadata plus optimized/no-opt fixture panel: 3/3 Debug;
- metadata plus optimized/no-opt fixture panel: 3/3 Release;
- original focused replay: 2/2 byte-identical;
- expanded fixture: one accepted pair and three preserved negatives;
- canonical exact comparison: 3/3 byte-identical, zero K06 accepts;
- Debug and Release `rxas` hashes unchanged;
- `git diff --check`: pass.

## Provenance

- baseline/source HEAD: `78bd7f6f5c6fe3d87725c52a1be452cfc136c4a6`
- branch: `codex/perf3-rxas-flow-infrastructure`
- host: Darwin 25.5.0 arm64, Apple M5, 10 logical CPUs
- toolchain: Apple Clang 21.0.0, CMake 4.3.2, Ninja 1.13.2
- Debug `rxas` SHA-256:
  `a81d9e83645efbc3316fea7a7c6f45085ce306646be58c8542bea0eb2536e456`
- Release `rxas` SHA-256:
  `a1c0e1efc6dba991794183879849ac0c5c962aedb7e8d0fc0b6910f4cb204904`

The dirty scope contains only K06 tests, current documentation and this
evidence bundle. No production optimizer, assembler, VM, ISA or ABI code was
changed.

## Next gate

K05 — migrate branch-to-conditional/dual-branch threading from queue-local
label search to immutable CFG edge rewrites and reachability.
