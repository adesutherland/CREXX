# PERF3-13 Gate C checkpoint C1 census

Date: 2026-08-06

Status: **C1 complete; candidate panel frozen; stop before geometry or value-layout PoCs**

## Result first

The unchanged 240-byte value and accepted worker/slab allocator are now
measured well enough to begin controlled geometry and layout comparisons, but
no candidate is implemented or selected at this checkpoint.

The evidence rejects a single larger-slab answer. Sieve owns and repeatedly
reuses one 8,192-value attribute block whose current V0 allocation is
1,966,080 bytes. It remains an exact tracked oversized extent under every
bounded 64 KiB through 1 MiB slab proposal. Towers, conversely, makes
12,071,103 hot eight-value-block allocations; the current exact 1,920-byte
typed class is the material slab path and peaks at 455 live blocks. Typed
silos and tracked exceptional extents therefore remain separate.

The exact compiler layout models confirm a coherent value ladder:

| Rung | Bytes | Change |
|---|---:|---|
| V0 | 240 | current value, including 32-byte inline string |
| V1 | 208 | remove inline string, retain direct pointers and current widths |
| V2a | 192 | pack flags and use byte capacity-class codes for managed string/binary sidecars |
| V2b | 160 | retain `size_t` binary length; checked 32-bit managed text, decimal, UTF and object counts |
| V3 | 120 | two lazy typed sidecars for numeric/native and object-growth metadata; direct hot string, binary, references, type, active attributes and count |

V3 is a layout model, not an ABI or implementation decision. V4 stays closed
unless measured V3 survivors leave a material ceiling.

## Measurement boundary

- Planning parent: `4fe6012469bae1f539fe74ce794c7a28fba03ca2`.
- Accepted Gate B control: `f36d2c1549f9f37f3950e42bfb89f8d32f12f3ea`.
- Branch: `codex/rxvm-default-and-base64-review` in the private scratch tree.
- Host: Apple M5, Darwin arm64 25.5.0, AC attached; Adrian reserved the host
  exclusively for the run.
- Build: Apple Clang 21, CMake Release `-O3 -DNDEBUG`,
  `CREXX_VM_PROFILING=ON`, forced-inline/flatten production policy.
- Inputs: exact optimized images and `library.rxbin` retained from the Gate B
  control build.
- Mode: `--profile=counts`; instrumented elapsed time and the RexxCPS rate are
  diagnostic only and are not product timing evidence.
- Product lane: compiler-selected `rxvm -> rxbvm`. Sieve, Base64 and Towers
  were repeated under concrete `rxtvm`; their value-census sidecars are
  identical after normalizing the historical VM-mode label.
- All six product cells and three cross-engine cells passed. Every profile has
  result zero, zero counter overflow/invalid events, complete census tracking,
  and allocator failures/invalid frees/wrong-owner frees equal to zero.
- Focused allocator, value, RXPA UTF, RXVML UTF and profiler tests passed 9/9.
- A separate profiling-off Release build passed allocator and Sieve smoke
  checks and contains neither value-census nor detailed oversized-census
  strings. `sizeof(value)` remains 240 with alignment 8.

## Physical value and storage census

Unique addresses count physical register/value locations observed during the
run, not operation frequency. Frame recycler reuse means this is intentionally
different from cumulative value-slot traffic.

| Workload | Unique values | Frame | Attribute | String ever | Decimal ever | Binary ever | Object ever |
|---|---:|---:|---:|---:|---:|---:|---:|
| Sieve | 8,295 | 17 | 8,208 | 5 | 0 | 0 | 3 |
| Richards | 1,324 | 198 | 1,056 | 6 | 0 | 0 | 111 |
| Base64 | 159 | 81 | 8 | 25 | 0 | 9 | 1 |
| JSON | 1,627 | 1,469 | 88 | 30 | 0 | 10 | 10 |
| Towers | 4,343 | 465 | 3,808 | 5 | 0 | 0 | 534 |
| RexxCPS | 663 | 413 | 176 | 224 | 9 | 2 | 12 |

All rows also contain the context globals, 39 run scratch values and the API
return value. RexxCPS has one separately allocated standalone value. Reference
and native-payload storage do not occur in this workload panel, so their
correctness remains covered by focused tests rather than being misreported as
a dynamic hot/cold conclusion.

## Live, sticky and reclaimable capacity

Reuse distance is the number of intervening value-census observations between
logical deactivation and reactivation of an already attached sidecar. It is an
operation-order measure, not nanoseconds or instructions.

| Workload | Peak string bytes | Peak binary bytes | Peak attribute slots | Peak inactive sticky capacity | Reuse events |
|---|---:|---:|---:|---|---|
| Sieve | 64 | 0 | 8,208 | 8,192 attribute slots | 5,499 attribute |
| Richards | 128 | 0 | 1,056 | 136 attribute slots | 437 attribute |
| Base64 | 14,560 | 9,280 | 8 | 10,304 string bytes; 2,048 binary bytes | 4,279,994 string; 2,499 binary |
| JSON | 4,448 | 3,296 | 88 | 96 binary bytes; 72 attribute slots | 4,999 binary; 39,992 attribute |
| Towers | 64 | 0 | 3,640 | 1,248 attribute slots | 3,118,058 attribute |
| RexxCPS | 3,520 | 4,096 | 176 | 256 string bytes; 16 attribute slots | 559,996 string; 14 attribute |

Base64 reuses retained 64-byte and 2 KiB string classes 1,709,998 and
2,569,996 times respectively, and its 2 KiB binary class 2,499 times. RexxCPS
reuses the retained 64-byte string class 559,996 times. A per-reset reclaim
check or blanket release would recreate hot allocation traffic and is rejected.

JSON shows the complementary case: its 32-byte binary class is reused 4,999
times, while 9,999 final inactive episodes are released without another reuse.
Towers similarly has 12,070,854 final eight-attribute episodes and 3,118,058
reuse episodes. This supports owner-quiescent, pressure/budget-controlled
reclamation, not a universal lifetime rule.

## Allocator observation

Capacity totals below are cumulative request/capacity traffic. Peak live and
retained slab bytes are separate instantaneous/end-state dimensions.

| Workload | Allocations | Requested bytes | Capacity bytes | Peak live bytes | Retained slabs | Oversized peak | Oversized calls | Largest request |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| Sieve | 1,473 | 3,972,302 | 4,125,060 | 4,087,172 | 1,179,648 | 3,494,388 | 18 | 1,966,080 |
| Richards | 1,916 | 2,253,140 | 2,417,484 | 2,379,148 | 1,441,792 | 1,451,148 | 16 | 359,184 |
| Base64 | 3,984 | 4,488,646 | 4,649,540 | 2,052,676 | 1,179,648 | 1,397,236 | 15 | 359,184 |
| JSON | 11,533 | 2,600,830 | 2,770,004 | 2,412,052 | 1,179,648 | 1,756,068 | 29 | 359,184 |
| Towers | 48,285,887 | 25,496,174,757 | 25,496,417,716 | 3,170,996 | 1,376,256 | 1,397,236 | 15 | 359,184 |
| RexxCPS | 691,566 | 138,549,593 | 138,719,068 | 2,198,476 | 1,310,720 | 1,455,948 | 17 | 359,184 |

Except for Sieve's value block, the oversized histogram is dominated by a
small number of context-lifetime 32-512 KiB requests. There is no observed hot
repeated buffer in the provisional S3a 128-256 KiB standard-class band.

## Frozen C1 candidate panel

### Geometry

- **S0 control:** 64 KiB slab, 16 KiB byte maximum, current typed classes and
  exact tracked oversized extents.
- **S1:** 64 KiB slab, 8 KiB byte maximum. This tests removing the current
  three-slot 16 KiB class while leaving qualifying typed classes independent.
- **S1b:** 128 KiB slab, 16 KiB byte maximum. This is the smallest power-of-two
  bridge that retains the threshold and gives the largest byte class seven
  actual slots after the header.
- **S2:** 256 KiB slab, 32 KiB byte maximum. This is the largest geometry to
  advance to a short screen.
- **S3/S3a rejected at C1:** 1 MiB slabs with 128/256 KiB byte classes still do
  not contain Sieve's 1.97 MiB typed extent, have no observed hot reusable
  demand in their added band, and would multiply the many small-class slab
  commitments. Reopen only if S2 demonstrates a benefit attributable to span
  size that survives its memory guard.
- Keep typed value blocks exact. Do not add a `value128+` slab class from this
  census; the Sieve block remains a reusable owned oversized extent.
- Compare reserve limits in bytes, not slab count, for every geometry.

### Value shape

- V1 screens both 16-byte and 32-byte first string-sidecar policies; both use
  the existing power-of-two byte silos and no inline string.
- V2a uses one-byte power-of-two capacity codes for ordinary managed string and
  binary buffers. Zero means unattached; an escape code retains explicit
  oversized/foreign handling through allocator/native metadata.
- V2b keeps binary actual length and allocator arithmetic at `size_t`; managed
  text, decimal, UTF-cache and object counts become checked `uint32_t`.
- V3 keeps string/binary pointers, lengths/classes, reference identity/payload,
  object type, active attribute pointer and count in the 120-byte hot header.
  Numeric/native and object-growth metadata use two independent lazy typed
  sidecars with one lifecycle protocol. No universal sidecar is required.

On the current 64 KiB payload, exact eight-value blocks progress from 1,920,
1,664, 1,536, 1,280 to 960 bytes across V0/V1/V2a/V2b/V3, increasing slots per
slab from 34 to 39, 42, 51 and 68. That is the principal Towers density
hypothesis; it is not yet a measured speed claim.

### Reclamation

- R0 remains the control: logical reset never reclaims.
- R1 is an explicit owner-quiescent trim operation.
- R2 observes a global pressure state only at allocator slow paths or explicit
  VM/worker quiescent safe points. Its first screen reclaims inactive owned
  attachments at least 128 KiB, scans at most 64 values and returns at most
  1 MiB per pass while allowing one larger object for progress. High/low
  hysteresis prevents repeated pressure toggling.
- R2 does not touch the hot <=2 KiB string/binary classes demonstrated by
  Base64/RexxCPS and never places a pressure branch in ordinary value reset.
- The selected value shape may change sidecar enumeration; therefore
  reclamation is compared after the provisional shape/geometry results, as
  planned.

## Interpretation limits

- This is counts-only diagnostic evidence. It selects experiments, not a
  product performance winner.
- Unique-address maxima and cumulative allocator traffic are not RSS.
- Geometry memory statements above are projections from exact slab sizes and
  observed class use until the PoCs provide ordinary profiling-off Release
  RSS/throughput evidence.
- The layout sizes are compiler-confirmed C models of the frozen field sets.
  ABI compatibility, native/plugin migration and implementation complexity
  remain Gate C/D decisions.
- Spawn was deliberately not used, under Adrian's accepted transitional rule.

## Evidence map

- `profiles/`: raw main profiler CSV, value-census sidecar, benchmark output,
  allocator telemetry and process-resource output for every cell.
- `value-layout.txt`: current offsets plus exact V1/V2a/V2b/V3 model sizes.
- `cross-engine-identity.txt`: deterministic dispatch-engine comparison.
- `pre-state.txt`, `post-state.txt`: host, power, source/build and binary state.
  The final post-state executable hashes are authoritative for the retained
  rows; the initial pre-state predates the schema-only final profiler relink.
- `logs/focused-tests.log`: final 9/9 diagnostic correctness panel.
- `logs/ordinary-release-proof.log`: profiling-off allocator/Sieve proof and
  absence of census strings.
- `COMMANDS.md`: exact command shapes and validation boundary.
- `value-census-summary.csv`, `allocator-summary.csv`: compact derived views.
- `checksums.sha256`: recursive identity closure excluding itself.
