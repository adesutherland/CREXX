# RXVECTOR-01 exact packed vector provider

Status: first ordinary-Release verdict accepted by Adrian, macOS closeout
complete on 2026-08-22, and supported Linux sanitizer qualification complete
on 2026-08-23. Final-head cross-platform Release coverage is supplied by the
GitHub Build CREXX and Sanitizer QA workflows.

Approved by Adrian: 2026-08-22.

## Scope

Implement a generic process-reentrant Level G standard/default `rxvector`
provider over the accepted BINARY-01 `.packedfloat` and `.packedint` owners.
The first provider is exact CPU arithmetic. It does not select an ANN index,
external numerical library, SQLite extension, persistent handle, or
RAG-specific policy.

The approved public contract and downstream proof boundary are retained in
`crexx-rag` at
`docs/evidence/2026-08-22-crexx-capability-sync/RXVECTOR-DESIGN.md`.

## Design selection

1. **Status quo pure Level B:** retain as correctness oracle and functional
   fallback. It is not sufficient as the sole production route because the
   retained 11,684-by-768 arithmetic result is 666,162 to 766,924 us.
2. **Selected public boundary — packed exact CPU:** direct RXPA procedures
   borrow `.packedfloat`/`.packedint`, use a row-major exact cosine scan, and
   own no state beyond the call. This is the only production implementation
   authorized by this worklist.
3. **Direct `f32le` native search:** retain only as a diagnostic machine/data-
   layout control if needed. It is not the public compute contract because it
   would make one persistence encoding a second performance representation.
4. **Prepared/ANN/external backend:** deferred. It requires a separate design
   for lifecycle, fingerprints, memory, cancellation, task ownership,
   packaging and stale-index recovery plus matched backend evidence.

The selected top-k baseline is one exact matrix scan plus a size-`k` binary
heap and a final deterministic result sort. Norms and dot products use a
block-compensated one-pass path for ordinary finite data and a scaled,
compensated replay when raw squares/products overflow or underflow. Finite
large-magnitude inputs therefore remain valid without charging every normal
embedding element for the exceptional path. Conversion uses explicit
canonical-little-endian IEEE binary32 import/export procedures.

## Public surface

```text
rxvector.decodef32le(data = .binary) = .packedfloat
rxvector.encodef32le(values = .packedfloat) = .binary
rxvector.cosine(left = .packedfloat, right = .packedfloat) = .float
rxvector.topkcosine(vectors = .packedfloat,
                    identities = .packedint,
                    dimensions = .int,
                    query_vector = .packedfloat,
                    requested = .int,
                    expose result_identities = .packedint,
                    expose result_scores = .packedfloat) = .void
```

Provider ID, namespace and artifact stem are all `rxvector`. Declarative RXBIN
metadata owns dynamic/static discovery; no Rexx declaration wrapper, manual
plugin list, initializer, sidecar manifest, or VM-hard-coded function is
authorized.

## Correctness gate

- [x] Standard conversion and cosine vectors, embedded sign/zero behavior and
      `f32le` round trip.
- [x] Empty, unequal, partial-width, non-finite, zero-norm, invalid dimension,
      count mismatch, range/overflow, uninitialized-owner and output-reset
      errors.
- [x] Deterministic score-descending, identity-ascending, row-ascending tie
      order and input immutability.
- [x] Optimized and non-optimized callers on `rxbvm` and `rxtvm`.
- [x] Declarative dynamic autoload and focused concurrent process-reentrant
      use.
- [x] Static/native package selection through an installed scratch consumer.

## Mandatory first Release verdict

After minimum focused correctness passes:

1. freeze production changes;
2. build the ordinary profiling-off Release product;
3. compare the same packed workload through pure Level B, public RXPA, and a
   direct-C ceiling, with at least one warmup and 12 paired/interleaved rounds
   for the selected decision cells;
4. report kernel time, conversion/preparation separately, exact result, both
   concrete VMs and process-inclusive elapsed; and
5. stop for Adrian's decision before broad CTest, sanitizer, install/package,
   downstream cutover, or API/documentation polish.

The representative downstream proof retains bounded pages and separately
records conversion, arithmetic/selection, total time and peak RSS. The prior
10,000-us value is an acceleration trigger, not an automatic pass/fail SLA.

## Work log

| Gate | State | Evidence / next action |
| --- | --- | --- |
| R0 design and approval | complete | Adrian approved the exact packed provider contract on 2026-08-22. |
| R1 production and focused correctness | complete | Four exact caller cells and the focused two-context dynamic-provider case pass in Debug. |
| R2 first ordinary-Release verdict | accepted | On AC with low-power mode off, all 72 recorded cells pass. Public median kernel time is 8,431/8,486 us on `rxbvm`/`rxtvm`, 15.62x/14.68x faster than the paired Level B oracle and at the direct-kernel ceiling. Adrian accepted the result on 2026-08-22. Evidence: `evidence/2026-08-22-rxvector01-first-release-verdict/`. |
| R3 closeout | complete on macOS | Scratch install proves dynamic autoload and automatic native/static selection with benchmark-only providers excluded. Public documentation and the bounded downstream cutover are complete. Normal Debug passed 2,359 tests before exposing two missing-selector harness registrations; both corrected modes pass. The installed-only downstream inventory similarly has passing evidence for all 62 tests after preserving a frozen historical benchmark and moving its maintained Level-G replacement. Supported Linux sanitizer and cross-platform proof remain release QA. |

### Pre-freeze pilot

The initial fully scaled/per-element-compensated implementation passed the
focused four-cell contract and concurrent dynamic-provider proof. A single
uncontrolled profiling-off Release diagnostic over 11,684 by 768 measured the
public path at 32,812-32,979 us, the exact shared-kernel control at
32,497-32,535 us, and the improved packed Level G oracle at 125,590-144,537 us.
This is selection evidence only, not the mandatory verdict. It shows RXPA cost
is already negligible and selects the block-compensated ordinary path plus
scaled exceptional fallback for the final pre-freeze candidate.

The frozen candidate's second uncontrolled profiling-off Release pilot measured
the public path at 8,340-8,574 us and the exact shared-kernel control at
8,405-8,615 us across `rxbvm` and `rxtvm`, with the same 1,034.2 checksum. The
Level B oracle measured 125,133-129,465 us. This selects the frozen candidate;
formal distribution evidence still requires the governed AC run.
