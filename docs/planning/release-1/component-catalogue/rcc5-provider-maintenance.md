# RCC-5 provider-maintenance ledger

Status: current for the approved RCC-5A through RCC-5E worktree on 2026-08-21.

This is the authoritative post-Stage-7 overlay for the mathematics and
historical-provider split. The Stage 2 raw files remain the review snapshot
from which the programme began; their old `rxmath`, `system`, private `id._*`,
and `mylib1` rows are not current product contracts.

## Current provider components

| Catalogue ID | Provider/artifact ID | Public namespace | Public leaves | Direct product consumers | Delivery and retained qualification |
|---|---|---|---:|---|---|
| `PKG-RXPA-float` | `rxfloat` | `rxfloat`; direct scalar compatibility names in `rxmath` | 37 canonical + 37 compatibility | ordinary user modules; Level G math tests | process-reentrant dynamic/static provider; optimized/no-opt, both VMs, concurrency, install/native package, contract and accepted performance evidence |
| `PKG-RXPA-stats` | `rxstats` | `rxstats` | 5 | ordinary user modules; RCC-5F oracle | process-reentrant dynamic/static provider; transitional boxed arrays, error/offset coverage, concurrency, install/native package and accepted performance evidence |
| `PKG-RXPA-hash` | `rx_hash` | `rxhash` | 5 | `crexx-rag` SHA-256 consumer and ordinary user modules | process-reentrant dynamic/static provider; binary-safe vectors, both VMs, concurrency and install/native package |
| `PKG-RXPA-id` | `rxid` | `rxid` | 6 | `classlib_native` `Id` adapter and ordinary user modules | bundled optional Level G process-reentrant provider, callable from B when installed; canonical public names replace private `id._*` declarations |
| `PKG-RXPA-fs` | `rxfs` | `rxfs` | 11 | `crexx` driver, `classlib_native` `Os` adapter, ordinary user modules | process-reentrant dynamic/static provider; filesystem contract, concurrency and install/native package |
| `PKG-RXPA-platform` | `rxplatform` | `rxplatform` | 5 | `classlib_native` `Os` adapter and ordinary user modules | bundled optional Level G process-reentrant provider, callable from B when installed; target `_platform` explicitly publishes provider ID `rxplatform` |

`rxint` adds five Level-G standard Rexx exports (`gcd`, `lcm`, `isqrt`,
`powmod`, `factorial`). `rxdecimal` adds seven (`sqrt`, `exp`, `ln`, `sin`,
`cos`, `pi`, `euler`) plus a private module initializer and private helpers.
These are Rexx library leaves rather than RXPA registrations.

## Exact RCC-5 catalogue delta

Relative to the post-RCC-4 catalogue overlay:

- native RXPA public leaves change from 423 to 473: remove 16 historical
  mixed-`rxmath`, 32 broad-`system`, and seven private-`id` leaves; add 74
  `rxfloat` canonical/compatibility leaves, five `rxstats`, six canonical
  `rxid`, eleven `rxfs`, five `rxplatform`, and four additional `rx_hash`
  leaves;
- the one duplicated `console.setcolor` registration site remains, so there
  are 474 registration sites for those 473 public native identities;
- plugin package IDs change from 28 to 30: add `float`, `stats`, `fs`, and
  `platform`, and retire `rxmath` and `system`;
- namespace-exposed Rexx leaves gain the twelve `rxint`/`rxdecimal` exports
  and lose the two unbuilt `mylib1` example procedures; and
- no new repository-wide aggregate is asserted here because other product
  programmes have also added source/library leaves since the Stage 2 census.
  A future catalogue checker must compute that wider number from current
  source instead of combining asynchronous manual deltas.

## Retired pre-release surfaces

The following are removals, not compatibility aliases:

- broad provider/package `system` and all `system.*` registrations;
- the mixed native `rxmath` provider's statistics, hash/checksum, UUID, and
  `inlinec` entries (only scalar names directly registered by `rxfloat`
  remain);
- private `id._uuid`, `_uuidt`, `_uuidv7`, `_ulid`, `_nanoid`, `_snowflake`,
  and `_base58` declarations; and
- dormant developer/UI/legacy entries for parsing, pipes, process-global
  storage, clipboard, beep, and the historical RXBIN module scanner.

The `crexx` driver now imports `rxfs`; its linked RXBIN declares that provider
and native packaging selects the archive through the normal RCC-1/RCC-2
metadata route. `classlib_native` declares only `rxid`, `rxfs`, and
`rxplatform` according to its actual adapters. None of these procedures is
hard-coded into `rxc`, `rxlink`, or either VM.

## Remaining boundary

`BINARY-01` and RCC-5F replace the boxed `rxstats` arrays with aligned packed
host-native `rxfloat`/`rxinteger` storage and record the later `rxvector`
dependency. This ledger does not bless the boxed representation as a stable
high-performance contract.
