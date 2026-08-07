# PERF3-13 Gate C2 allocator-geometry first Release verdict

Date: 2026-08-06

Status: mandatory first Release verdict complete; stopped for Adrian

## Validity correction

The first attempted capture was invalidated before retention. Its selector
added a hot cutoff branch to S0, so S0 was semantically equivalent but not a
machine-level replay of the accepted allocator control. None of its numbers
has decision authority.

The corrected selector changes the compile-time byte-class table instead. An
exact `otool -tvV` comparison proves all 22 emitted `rxvm_memory_*` text
symbols in the corrected S0 binary are identical to the accepted ordinary C1
S0 control, including allocation, resize, release, depot, stats and teardown.
Whole-binary hashes differ because the builds embed different build identities;
`s0-disassembly-identity.txt` records the bounded text-symbol proof.

## Verdict

The valid unchanged-240-byte V0/R0 screen is correctness-clean but does not
select a new geometry. All three alternatives are within about 0.6% of S0 on
the short stable-six timing geomean, while their memory behaviour differs:

| Candidate | Slab / byte cutoff | Stable-six median geomean | Peak RSS range vs S0 | Allocator-retained bytes vs S0 |
|---|---:|---:|---:|---:|
| S1 | 64 / 8 KiB | -0.353% | 0.000%..+1.512% | -196,608 on every workload |
| S1b | 128 / 16 KiB | +0.445% | -1.631%..-2.488% | +589,824..+917,504 |
| S2 | 256 / 32 KiB | +0.562% | -5.820%..+5.247% | +2,424,832..+3,145,728 |

S1 is the allocator-memory-lean candidate. Removing the 16 KiB standard byte
class turns those observed requests into exact extents and reduces internal
fragmentation, peak live capacity and retained slabs. Its largest short timing
loss is Permute at -1.067%; no timing or RSS guard fires.

S1b has the best balanced compromise in this screen: +0.445% stable-six,
Towers +1.075%, and lower measured process RSS on every row. It nevertheless
retains about 576-896 KiB more allocator slabs than S0 because larger mostly
empty slabs do not all become resident. Process RSS cannot substitute for
allocator-retained accounting, especially when that local cost will multiply
by worker count.

S2 improves Towers by +1.689% and RexxCPS by +1.289%, but raises Sieve median
peak RSS by 966,656 bytes (+5.247%), breaching the positive 3% workload guard.
It also retains about 2.31-3.00 MiB more allocator slabs per current logical
worker. S2 must not advance unchanged.

Base64 reverses direction from the invalid scratch capture and remains noisy;
it is recorded but does not select allocator policy. The recommended next step
is to reject S2 and run the formal 12-round S0/S1/S1b product-lane panel with
separate RSS and allocator scorecards. This requires Adrian's direction; no
formal panel, `rxtvm` diagnostic, V1 layout or reclamation candidate has
started.

## Candidate implementation boundary

- Source control: `0d1fe884782ff369960b1c67c38127407ce54588`, branch
  `codex/rxvm-default-and-base64-review`, with the exact provisional dirty
  scope recorded in `post-state.txt`.
- S0 remains the default. A closed CMake selector admits only S0, S1, S1b and
  S2; ordinary builds remain profiling-off.
- Generic byte cutoffs change independently of exact typed value/reference
  silos. S1 has ten byte classes through 8 KiB, S0/S1b have eleven through
  16 KiB, and S2 adds only the twelfth 32 KiB class.
- Central reserve ceilings are normalized to 2 MiB globally and 128 KiB per
  class in whole slabs, retaining at least one slab for an active class.
- `value` remains exactly 240 bytes, automatic reclamation remains off and no
  ABI, worker/thread, process, channel, HTTP, RXAS/RXBIN or spawn path changed.
- The selector and candidates remain provisional and revertable pending the
  verdict decision.

## Correctness and sampling

- Host: Apple M5 MacBook Air, Darwin arm64; ten logical CPUs; machine reserved
  exclusively by Adrian.
- Toolchain: Apple Clang 21.0.0, Ninja, ordinary Release `-O3 -DNDEBUG`,
  `CREXX_VM_PROFILING=OFF`.
- Power before timing: AC attached, 82% battery, load averages
  1.14/1.20/1.23. Exact pre/post state is retained.
- Product lane: every `rxvm` symlink resolves to Apple Clang's `rxbvm`.
- Images: exact accepted Gate B optimized RXBIN images and `library.rxbin` are
  reused for every cell. C2 performs no source/TRACE stripping or image
  rebuild, so image and metadata form are identical across geometries.
- Focused correctness: allocator geometry/ownership and value lifecycle tests
  pass 2/2 for each geometry, 8/8 total.
- Timing: Sieve, Permute, Bounce, Richards, Base64, Towers and canonical
  RexxCPS; one warmup plus four serial pairwise/position-balanced recorded
  rounds across four geometries. All 140 processes pass; no sample is removed.
- RSS: the same workloads and geometries, four balanced recorded rounds. All
  112 processes pass; no sample is removed.
- Telemetry: one untimed teardown-count run per workload/geometry; all 28 pass
  with zero allocation failures, invalid frees and wrong-owner frees.

Four rounds are a rejection screen, not formal candidate-selection evidence.
Base64 remains a noisy non-selector and is excluded from the stable-six
geomean.

## Evidence map

- `geometry-manifest.txt`: exact candidate commands and workload grouping.
- `s0-disassembly-identity.txt`: accepted-control identity proof for all 22
  emitted allocator text symbols.
- `timing/`: raw samples/output, absolute summaries and explicit S0 ratios.
- `rss/`: raw peak-RSS samples/output, summaries and guard deltas.
- `telemetry/`: raw stdout/stderr, allocator summary and S0 deltas.
- `logs/focused-tests.log`: final 8/8 focused proof.
- `logs/gatec-c2-v2-*-build.log`: ordinary Release build logs.
- `build-configs.txt`: selector, profiling mode, product links, hashes and
  artifact sizes.
- `pre-state.txt`, `post-state.txt`: host, power, toolchain and source state.
- `COMMANDS.md`: exact replay commands and interpretation boundary.
- `checksums.sha256`: recursive evidence identity, excluding itself.

All timing and memory figures are observations from this short Apple/Clang
screen. They are not a cross-platform claim or permission to industrialise a
candidate.
