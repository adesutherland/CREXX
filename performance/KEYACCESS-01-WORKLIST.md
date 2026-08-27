# KEYACCESS-01: KeyAccess bulk-insert scaling

Status: first Release verdict complete; awaiting review

## Problem

Loading unique keys into one transaction currently performs a full index scan
for every `writekey()`. The supplied 50,000-key run takes about 380 seconds.

## Design selection

### Status quo

`writekey()` calls the linear `find_key()` scan for every key and discards the
lazy in-memory hash index after every successful write. The hash index is not
used by the write path, so the linear scan is the suspected O(n^2) cost.

### Candidate A: incremental existing hash index (selected)

Build the existing in-memory hash index at the first write lookup, use it for
subsequent existence checks, and add/remove entries after successful index
updates. Return the matched index-file offset for replacements. Full-key
comparison remains mandatory after hash matching. Rollback, compaction,
reopen, and reset continue to discard the index.

This preserves the file format and public API while changing unique-write
lookup from repeated linear scans to one index build plus expected O(1)
lookups.

### Candidate B: retain linear writes and add a bulk-write mode

This would avoid per-record lookup only when callers opt into a new mode, but
would add public state and a second write path without helping ordinary
transactional inserts. It is rejected for this incremental fix.

## Safety gates

- Existing replacement, delete, rollback, reopen, and traversal tests pass.
- Collision candidates compare complete keys.
- Release benchmark covers 5,000, 10,000, 20,000, and 50,000 unique keys.
- Cache population is measured separately; it is not removed speculatively.

## Root-cause evidence

The old `writekey()` path called `find_key()`, which rewound and linearly read
the index for every key. It did not rebuild the hash index after each write;
the hash index was simply discarded and was not consulted by writes. Each write
also seeks to the data-file end and appends one index record, but does not flush
the files individually. Cache population is bounded hash-directed work and was
not the source of the O(n^2) behavior.

The selected path builds the existing hash index once and maintains it after
successful new writes and deletes. Replacements use the existing record's
stored offset and do not add a duplicate active record. Rollback and compaction
still invalidate the index, preserving their existing rebuild boundary.

## First Release benchmark

Host: Windows Release build, `rxvm`, serial runs, existing
`examples/keydb_lookup_benchmark.crexx` workload, `reset()` followed by one
transaction containing unique `put()` calls. The benchmark's command-line
argument binding was corrected so the requested key counts are actually used.

| Keys | Insert time after fix | Approx. time/key |
| ---: | ---: | ---: |
| 5,000 | 0.230295 s | 46.1 us |
| 10,000 | 0.392483 s | 39.2 us |
| 20,000 | 0.744297 s | 37.2 us |
| 50,000 | 2.028873 s | 40.6 us |

The supplied pre-fix 50,000-key result was 379.952295 s, so the comparable
50,000-key run is approximately 187x faster and has linear rather than
quadratic observed scaling. No pre-fix 5,000/10,000/20,000 samples were
provided, so those cells are intentionally not reconstructed.

Focused native and wrapper correctness tests pass in both optimized and
non-optimized modes, covering new insert, replacement, delete, rollback,
reopen persistence, and firstKey()/nextKey() traversal.
