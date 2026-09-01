# KEYACCESS-02: KeyAccess negative-lookup scaling

Status: first Release verdict complete; awaiting review

## Problem

At 500,000 active keys, successful lookups take about 11 seconds while
missing lookups take about 198 seconds.

## Root-cause investigation

`readkey()` first checks the bounded hash-directed cache, then calls
`find_key_indexed()`. A hash miss selects one in-memory bucket and walks its
collision chain; it does not scan the on-disk index. A hash candidate causes a
disk read and complete-key comparison. The missing-key result then calls
`log_error()`, which opens, appends, and closes `keyaccess.log` for every
ordinary miss. This per-miss filesystem work is the pathological cost.

## Design selection

### Candidate A: suppress logging for expected read misses (selected)

Return `NOT_FOUND` directly from `readkey()` when the authoritative hash-index
lookup returns `KA_ERROR_NOTFOUND`. Parameter, I/O, memory, and corruption
failures continue to be logged. This preserves the public API and all lookup
semantics.

### Candidate B: retain per-miss logging or add rate-limited logging

Per-miss logging is the measured cause of the regression. Rate limiting would
add policy and state without improving the normal expected-miss contract, so
it is rejected for this fix.

## Complexity and correctness

With a fixed 1,024-bucket hash index, a miss remains expected O(1) in-memory
work (or O(chain length) under collisions) and performs no disk read when
there is no matching hash candidate. A hash candidate still requires a full
key comparison. The change removes the per-miss filesystem logging cost; it
does not treat a hash as proof of key equality and cannot create a false
`NOT_FOUND` result.

## First Release benchmark

Windows Release, serial `rxvm` runs, present and missing loops over the same
active-key count. The supplied 500,000-key baseline was 11.026439 s present
and 198.232571 s missing.

| Keys | Present | Missing | Missing / present |
| ---: | ---: | ---: | ---: |
| 5,000 | 0.048979 s | 0.016443 s | 0.34x |
| 50,000 | 0.683498 s | 0.243210 s | 0.36x |
| 500,000 | 11.174757 s | 10.640404 s | 0.95x |

The supplied pre-fix 500,000-key values were 11.026439 s present and
198.232571 s missing, a 17.97x ratio. The comparable post-fix run is 0.95x;
the missing path is slightly faster because a true hash miss performs no disk
read or value load. No pre-fix 5,000/50,000 samples were provided, so those
comparisons are intentionally not reconstructed.
