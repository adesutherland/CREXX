# PERF3-12B B3 isolated H1 loop-reuse PoC

Date: 2026-08-04

Status: complete; retain for the B4 route comparison, no production selection

## Outcome

The isolated RXAS PoC proves one lazy first-use joined key as a stable cache
and redirects the four later equivalent native-stem keys within the enclosing
natural loop. It deletes four `CONCAT_REG_STRING_REG` instructions and their
four exact compound-tail `C` TRACE records while retaining the first
materialization, the original one-part `STEMGET`/`STEMSET` operations and all
stem signal/write behavior.

The route removes exactly 1,960,000 hot concat dispatches with no dynamic setup
instruction. It changes the first concat destination to a new private local,
so `main` grows from 103 to 104 locals. Applied to the latest accepted X1
fixed-work total of 52,839,051, with B1 hot work neutral, the derived H1 total
is 50,879,051 under either VM (-3.709378%). This is exact instruction-count
accounting, not a new profile or wall-clock result.

The four removed concats avoid 18,340,000 bytes of joined-tail temporary
payload, excluding terminators and value-structure writes. All affected
strings fit the inline representation, so the handler/representation audit
predicts neutral heap-allocation count. B4 must confirm that inference with a
common counts product.

## Placement and proof result

Lazy first use is the only retained H1 variant. The source-159 concat remains
at its original conditional program point and becomes the cache seed. Each of
the four later uses proves the same literal and right-side `ValueId`, unchanged
cache storage, seed/candidate/stem dominance, common reducible natural loop,
private non-overlapping storage, complete candidate uses and exact TRACE
deletion.

Preheader placement is not selected. Every real candidate reports the seed as
non-signalling/speculatable but not must-execute, the right string as
loop-variant and the seed TRACE as observable. Moving the seed would therefore
change zero-iteration/conditional work and ordered trace behavior.

## Scope and provenance

- Accepted B1 base: `83de3db3c2d42f3dfcd61a1242e37d3ef196437b`.
- Isolated H1 commit: `80c78fcee628dd60fe4d572892718f4dda09a4fc`
  on `codex/perf3-12b-h1-poc`.
- Canonical source is unchanged. The replay patch changes only generated RXAS:
  it provisions local 103 by redirecting the first existing concat and its
  stem use; it adds no executable instruction.
- Product: ordinary profiling-off Release RXAS. Diagnostic `-d` assembly is
  used only for proof and scale evidence.
- Runtime authority: unchanged accepted profiling-off Release `rxvm`, `rxbvm`
  and common `library.rxbin` recorded in `artifacts.csv`.
- Host: Apple M5, Darwin 25.5.0 arm64, macOS 26.5.2, 10 logical CPUs.

## Common foundation delta

H1 and S1 both need exact string-component metadata for CONCAT/SCONCAT. That
F1 metadata independently enables one existing X01 component-placement rewrite
outside the target route. The H1 control is the B2 PoC assembler on the same H1
input: it contains F1, reports zero S1 selections and emits `380 -> 369` main
instructions. H1 emits `380 -> 365`. The four-instruction difference and the
corresponding disassembly are therefore H1-only; F1 remains excluded.

## Correctness and scale

- Debug and Release immutable graph/opcode-metadata pairs pass 2/2.
- H1 graph fixtures cover the positive natural-loop proof and changed right
  value, cache write, linked write, call-window mutation, signal-skip TRACE and
  no-loop rejection.
- The final native-stem selector passes 16/16 after explicitly building three
  fresh-worktree prerequisites omitted by the first invocation.
- Control, optimized H1 and optimizer-disabled images print PASS with zero
  stderr under both `rxvm` and `rxbvm`: six cells.
- Optimizer-disabled control and H1 images are byte-identical.
- H1 retains `.locals=104`, removes four executable instructions and four TRACE
  records, reduces the code segment by 16 bytes and the whole image by 72 bytes.
- First-epoch SSA retained bytes are identical at 83,902,504. H1 adds only
  1,448 proof-service retained bytes and gross maximum RSS differs by 851,968
  bytes in the one-sample observation.
- The host was on battery. Raw assembly observations are retained, but their
  elapsed values are not a performance verdict; B4 must run the balanced panel
  with the remote terminal absent and the host on AC.

## Evidence map

- `analysis/selection.csv`: per-site disposition and exact dynamic weights.
- `analysis/foundation-and-work.md`: dispatch, payload, allocation, placement
  and common-foundation accounting.
- `replay/`: exact generated-RXAS cache-seed patch.
- `validation/focused-correctness.txt`: focused, native-stem and dual-VM checks.
- `validation/release-structure.txt`: emitted instruction and image structure.
- `validation/assembler-scale.txt`: raw battery-host scale observations.
- `artifacts.csv`: source, tool, input, image and runtime identities.
- `checksums.sha256`: recursive evidence closure excluding itself.

## Interpretation boundary and next gate

B3 proves H1 useful and fail-closed on this slice. It does not install H1 in
production, establish a wall-clock improvement, select H1 over S1, or authorize
composition. The next item is `B4 — comparative Release panel and selection
stop`: compare S0, S1 and H1 from retained replay inputs under both VMs, then
stop for Adrian to choose a route.
