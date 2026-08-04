# PERF3-11 K01 storage-permutation closeout

Date: 2026-08-03

Status: **complete; local K01 changes are uncommitted and not pushed**

## Decision

K01 replaces both bounded raw-register cancelling-`SWAP` keyholes with one
immutable sparse storage-permutation and observation proof. The selected proof
is the sole deletion authority: it proves physical-register dominance and
restored `StorageId` bindings, then rejects any relevant intervening read,
write, cursor access, metadata/TRACE observation, call window, opaque effect,
asynchronous handler use or non-linear control split.

The proof also accepts an exact self-cancelling four-operand `SWAPN`. Supporting
SSA transfer now composes overlapping `SWAPN` pairs in operand order; direct
unit coverage proves both the three-register overlap and identity forms.
General permutation rewriting remains future work.

## First ordinary Release verdict

The six inherited implicit-use/barrier cases and the retained same/reversed
orientations are byte-identical to committed baseline `45e027685`. Canonical
Richards, Towers and RexxCPS each record zero K01 accepts and byte-identical
images. This is therefore an output-neutral infrastructure migration; runtime
timing is not warranted and no runtime performance claim is made.

The stronger panel proves nine safe optimized/no-opt shapes, including a pair
beyond the old queue bound, unrelated metadata/TRACE, inherited signal policy,
same-storage identity and disjoint exact permutation. It retains fail-closed
controls for relevant observations/writes, opaque effects, real call windows,
unrestored mappings, locally installed or asynchronous handlers and control
splits.

## Validation summary

- strict GNU90 syntax: pass, with only the pre-existing unused `graph`
  parameter warning in `rxas_flow_attach_storage`;
- flow-graph contract: pass in Debug and Release;
- K01 focused optimized/no-opt panel: 9/9 in Debug and Release;
- shared proof-consumer regression panel: 37/37 in Debug and Release;
- canonical exact comparison: 3/3 byte-identical with zero K01 accepts;
- complete Debug build and CTest: 2,012/2,012 in 368.43 seconds;
- `git diff --check`: pass.

Exact commands and hashes are retained alongside this note in `COMMANDS.md`,
`focused-hashes.csv`, `canonical-hashes.csv` and `validation.csv`.

## Provenance

- branch: `codex/perf3-rxas-flow-infrastructure`
- source HEAD/baseline commit: `45e027685a1ac250c3b6cfd61963ba4e35fc764f`
- baseline Debug `rxas` SHA-256:
  `9be497a401660244b3e68b2b042ade1efe068ef28da0e7b2257784a40f6580a4`
- baseline Release `rxas` SHA-256:
  `11ea6e9b223aaf531397696ee316e0c6973bd38c8b4b5c78fbfcd5922dcf3b62`
- K01 Debug `rxas` SHA-256:
  `a81d9e83645efbc3316fea7a7c6f45085ce306646be58c8542bea0eb2536e456`
- K01 Release `rxas` SHA-256:
  `a1c0e1efc6dba991794183879849ac0c5c962aedb7e8d0fc0b6910f4cb204904`
- host: Darwin 25.5.0 arm64, Apple M5, 10 logical CPUs
- toolchain: Apple Clang 21.0.0, CMake 4.3.2, Ninja 1.13.2

The evidence was captured from the dirty K01 code/test/documentation scope
above the committed baseline. No push was performed.

## Next gate

K06 — classify or replace the adjacent `COPY` plus same-pair `ACOPY`
subsumption rule using the generic component-write proof where warranted;
then proceed to K05 branch threading.
