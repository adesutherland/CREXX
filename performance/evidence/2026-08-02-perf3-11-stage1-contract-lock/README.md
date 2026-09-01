# PERF3-11 Stage 1 selected signal-contract lock

Status: **complete — Gate 1 selected S1-S5 and retired `MAY_THROW`**

Base: `656a37803a28` on `codex/perf3-rxas-flow-infrastructure`

## Locked result

Adrian approved S1-S5 and required `RXOP_SEM_MAY_THROW` to be corrected or
retired coherently with the new metadata. The implementation retires the flag
completely. Existing non-signal semantic flag values remain unchanged, with
the retired bit left unused. Signal capability and failure state come only
from the opcode-aligned `RxOpSignalContract` inventory.

The two one-to-one inventories each contain 650 rows. The signal inventory is
deliberately fail-closed:

| Signal state | Rows |
| --- | ---: |
| Proven non-signalling | 163 |
| Known signal contract | 58 |
| Deliberate unknown | 429 |

Known contracts record the signal-name source, before/after/partial write
phase, failure-visible register/component/context writes, dependencies,
continuation classes and observation properties. Unknown rows retain every
conservative continuation.

## Selected semantic changes

- `DCOPY` is total for decimal absence, preserves unrelated components and
  reusable backing storage, and does not signal.
- Both decimal plugins make `DTOS` total and non-signalling; absence formats as
  `nan` and stale/plugin formatting diagnostics do not escape the conversion.
- Generic and fixed-register checked `INC`/`DEC` commit through a temporary,
  preserving the prior integer on overflow on every compiler/platform.
- Invalid literal `SETNUMFUZ` dispatches its signal before changing or
  synchronizing numeric context.
- Actual non-signalling scalar conversions and directly proved failure phases
  are represented in the signal sidecar; unreviewed helpers remain unknown.

## Verification

- Focused signal/storage/conversion/plugin/metadata CTest: **68/68 pass**.
- Permanent semantic matrix: `rxvm` and `rxbvm`, optimized and no-opt:
  **4/4 pass**.
- `test_rxop_metadata`: 650 aligned effect rows and 650 aligned signal rows;
  effect-state closure is 585 classified, 6 conservative, 56 reserved and 3
  internal.
- Maintained Level B sequence-ledger tool compiles, assembles and runs against
  the retained NR-09 input: schema 2, 650 effect rows, 650 signal rows, 11,332
  aggregate rows and 76 selected identities.
- Ordinary profiling-off Release `rxc`, `rxas`, `rxlink`, `rxvm` and `rxbvm`
  targets build successfully.
- Richards, Towers and RexxCPS generated RXBIN hashes exactly match Gate 0;
  this metadata/semantic lock introduces no ordinary optimized image drift.

The stale storage-identity test expectations for false `LINK`/`SWAP`/`UNLINK`
signal edges were updated to the exact dedicated-contract edge counts. No
optimizer consumer was added, so the first Release runtime-verdict gate has not
yet been triggered. Stage 2 may proceed to immutable graph construction.
