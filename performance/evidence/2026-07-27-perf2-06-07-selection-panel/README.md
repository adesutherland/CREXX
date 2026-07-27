# PERF2-06/07 Apple selection panel

Status: first mandatory stop; Adrian selection required before any production
performance edit

Date: 2026-07-27

This package closes the bounded Apple ARM64 analysis panel for the combined
PERF2-06/07 value and VM-ownership slice. It includes the V3-R01 correctness
disposition, a fresh current-product dual-VM attribution, allocation and
lifetime evidence, and a disposition for every candidate in the combined
worklist. It does not contain a production performance optimization.

## Decision summary

- V3-R01 was reproduced in all four optimized/no-opt and `rxvm`/`rxbvm`
  cells. `DCOPY` preserved the empty destination's trusted zero codepoint
  count; `DTOS` replaced the bytes with `2.2` but changed only byte length;
  `STRLEN` therefore returned stale `0`. The correctness patch gives every
  audited in-place string writer one explicit completion contract. All four
  retained reproducer cells and the focused sibling matrix pass after the fix.
- Current optimized Richards is the dominant whole-value-copy failure:
  73,307,574 recursive copy operations and 582,076,729 copied bytes per
  profiled run in each VM. The optimized stream is 1,693.160% slower than
  no-opt in the final three-sample `rxvm` attribution control and 1,424.429% slower
  in `rxbvm`; these are diagnostic medians, not a formal verdict.
- Current optimized Permute is the secondary instance of the same compiler
  ownership problem: 10,259,602 copy operations and 74,012,810 bytes, with
  119.642%/101.916% diagnostic slowdowns in `rxvm`/`rxbvm`.
- Native samples and high-water histories independently place Richards in
  `copy_value`. The optimized Richards Apple hardware control retired about
  7.02/7.08 billion instructions in `rxvm`/`rxbvm`, versus 0.438/0.505 billion
  for no-opt.
- The smallest material next candidate is `PERF2-06-07-V1R01`: compiler-owned
  direct placement/alias-preserving mapping of a proved inlined receiver
  instead of full-object copy-in/copy-back. Richards optimized is the decisive
  target, Permute optimized is the secondary target, and Sieve, Bounce,
  Base64 and RexxCPS are guards. The current copy path remains the fallback.
- `V1R02`, `V2R01`, performance cache retention under `V3R01`, and `V5R01`
  are deferred. `C2R03` fails its current-evidence entrance gate. `V6R01`
  stays at the cross-platform architecture decision. No rejected reset,
  allocation-ledger or cleanup-only design was retried.

## Package map

| File or directory | Purpose |
| --- | --- |
| [`CORRECTNESS.md`](CORRECTNESS.md) | Exact V3-R01 reproducer, cause, invalidation contract and sibling audit |
| [`ATTRIBUTION.md`](ATTRIBUTION.md) | Current timing, operation, native, allocation, RSS, lifecycle and artifact attribution |
| [`CANDIDATE-PANEL.md`](CANDIDATE-PANEL.md) | Complete ranked candidate/owner panel and decision boundaries |
| [`PROVENANCE.md`](PROVENANCE.md) | Git, host, toolchain, build, layout, hashes and retained-evidence audit |
| [`WORKTREES.md`](WORKTREES.md) | Exact linked-worktree snapshot and preservation state |
| `freeze/` | Frozen selected source, RXAS, RXBIN and linked images |
| `raw/correctness/` | Four-cell before/after output and bounded representation traces |
| `raw/profiles/` | Maintained Level B schema-5 summaries and their exact input manifest |
| `raw/timing/` | Final-product dual-VM one-warmup/three-recorded bundles, schema-5 counts and RXSEQ |
| `raw/native/` | Apple `sample` helper/caller stacks |
| `raw/heap/` | Final-product full-mode malloc-history/high-water reports |
| `raw/rss/` | Separate `/usr/bin/time -lp` RSS, instruction and cycle controls |
| `raw/lifecycle/` | Maintained Level B compile/assemble/load-to-first-result samples |
| `raw/checksum-audits/` | Retained package checksum verification logs |
| `SHA256SUMS` | Full package inventory, generated after documentation reconciliation |

## Interpretation limits

The three recorded steady samples qualify attribution and candidate ranking;
they are not a Release verdict. Apple `/usr/bin/time -lp`, `sample` and
malloc-stack histories are diagnostic inputs. The deterministic schema-5
operation counts, frozen images and four-cell correctness matrix carry the
mechanism claim. No cross-platform or final-VM claim follows from this package.
