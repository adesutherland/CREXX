# Unicode grapheme first-Release verdict

Status: accepted by Adrian's 2026-08-30 direction to proceed with full
qualification.

## Verdict

Retain the selected one-pass streaming implementation for direct
`graphemeCount` and bounded `graphemeSubstr`. Building a complete boundary
index inside every direct call did not improve either operation on the
decisive large-buffer lane:

| VM | Operation | Streaming median | Per-call index median | Index delta |
| --- | --- | ---: | ---: | ---: |
| `rxbvm` | count | 47,614.5 us | 48,472.5 us | +1.802% |
| `rxbvm` | substring | 47,787.0 us | 48,839.0 us | +2.201% |
| `rxtvm` | count | 50,303.0 us | 51,788.5 us | +2.953% |
| `rxtvm` | substring | 50,564.0 us | 51,668.0 us | +2.183% |

The explicit immutable `.graphemes` view still builds one packed boundary
index because its purpose is to amortize repeated indexed access, position,
reverse, and iteration. This verdict rejects only hidden per-call full indexing
for direct operations.

## Scope

- Ordinary profiling-off Release product on an AC-powered macOS ARM64 host
  with low-power mode disabled.
- One 49,152-byte input containing 7,168 extended grapheme clusters.
- Twenty calls per observation, with twelve balanced/interleaved recorded
  observations for every VM, operation, and implementation variant.
- Every count and substring checksum matched.

The consistent 1.8-3.0% point differences are sufficient for this bounded
architecture choice. They are not a general performance baseline or a claim
that differences below 3% are independently repeatable. `raw.txt` retains all
96 observations, `summary.txt` retains the medians and ranges, and `host.txt`
retains pre/post host state.

The later source-level `SIGNAL` cleanup changes only cold error paths and
lowers back to the same RXAS signal instruction. It does not change the
scanner, table access, boundary state, or benchmarked operations.
