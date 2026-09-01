# PERF3-02-C1B correctness-only PoC

Date: 2026-08-01

Status: **success — mandatory stop before timing**

## Verdict

The bounded C1b-R1 detached receiver-guard algorithm passes its approved
correctness gate. It removes the two intended Richards receiver copies, reduces
the total static `copy` count by exactly two, preserves the canonical result in
both VMs with optimization on and off, and passes an independent
storage-identity and exit-state check. No timing was performed and no
production candidate is selected.

## Frozen products

| Product | Role | SHA-256 |
| --- | --- | --- |
| retained `rxc-c0` | ordinary profiling-off Release control | `caec40f5d7d6304e29e2a678c5604a5d78e42172bc1ee0bf82b52a130f5f99d1` |
| retained `rxas-p1` | approved P1 storage-flow assembler | `09ea8e49c54dfc839833a1fa40dfa5809467aa53c730dc050ae0a2b95f73669e` |
| final C1b-R1 `rxc` | ordinary profiling-off Release candidate | `0323b70207c22486896d6883a80ca4388f6a4c164d03737bcefc3a3a8ed6be52` |
| final `rxas` | unchanged P1 assembler | `09ea8e49c54dfc839833a1fa40dfa5809467aa53c730dc050ae0a2b95f73669e` |

Repository identity is `develop` at
`e38e514bf611ae3873513368c44742e2ae7332d1`; product-code parent is
`3f43a0014be10c930a12b8a636297b60f294c0a6`.

## Proof boundary

The compiler opens direct receiver binding only for an exact method template:

1. Boolean scalar result with multiple explicit returns and no fallthrough;
2. one or more top-level `IF` guards followed by a final Boolean return;
3. each guard contains exactly one indexed scalar Boolean class-attribute read;
4. each taken branch immediately returns scalar Boolean `0` or `1`;
5. scalar by-value formals only; and
6. no calls, writes, non-guard receiver reads, loops, references, assembler,
   nested-return blocks, callee signal constructs or same-caller-frame signal
   continuations.

Each accepted predicate is evaluated into generated scalar storage before its
`IF`. The cloned assignment/`IF`/`LEAVE_WITH` sequence is independently
revalidated before commit. Exported/imported `I6` templates repeat the same
shape proof; there is no metadata-schema, RXAS/RXBIN, object-layout or runtime
ABI change.

## Correctness matrix

[`correctness-matrix.csv`](correctness-matrix.csv) records all required cells.
Every cell exits zero and reports:

```text
benchmark=awfy_richards repetitions=1 queue_packets=23246 holds=9297
PASS: AWFY Richards
```

## Deterministic image proof

The retained C0 Richards RXAS is SHA-256
`ec61aa8cb044312675502c0ce2a46d4f131e0640bae6188d5db007bfb8731673`;
the final C1b-R1 RXAS is
`7b26e91dd94f513e40d181ee1f4e7222d0067093cf7e9acd8500fb257deeaee4`.
The image grows from 4,925 to 4,947 lines because six explicit scalar guard
snapshots and their cleanup are now visible, while static `copy` instructions
fall from 71 to 69. The source-anchored receiver copies at
`Scheduler.runTask` line 218 and `Scheduler.schedule` line 300 are absent. The
independent procedure analysis below attributes exactly one fewer full-copy
event to each of those targets. Inserted scalar temporaries also renumber some
emitted registers, so raw operand-text identity is not used as the proof.

Towers remains byte-identical at
`e78bc406672af9c3fc7f9434ad9a4790d9fd12fe0df2852d05696fd2fffcadba`.
The class-method control remains byte-identical at
`b055e5349c052c1f5702a82ea14b9b609246e182802d5d0309df9dba58def5c9`.
Exact counts are in [`image-delta.csv`](image-delta.csv).

The retained C0 runtime counts therefore remain a deterministic ceiling, not a
timing result: 172,394 dynamic full receiver copies, 25,341,738 recursive copy
operations and 201,354,752 logical bytes are removed if C1b-R1 is selected.

## Independent P1 storage proof

The P1 assembler's diagnostic storage-identity analysis was run against C0 and
C1b-R1 Richards RXAS. [`storage-identity.csv`](storage-identity.csv) records
the procedure summaries. In both target procedures:

- exact link/unlink balance is unchanged;
- unknown join state falls to zero;
- exactly one full-copy event disappears; and
- the remaining exact/base and tainted full-copy counts agree.

Ordinary and diagnostic assembly of each same-stem input produce byte-identical
RXBIN images, so the verifier itself does not alter the executable proof.

## Focused validation

The final ordinary profiling-off Release product passed:

- 9/9 focused inliner, import and receiver tests, including local/imported
  detached-guard positives and fail-closed call, write, non-guard read, loop,
  reference-formal, nested-return and same-frame-signal cases;
- 4/4 Richards benchmark/fixture correctness tests; and
- optimized and no-opt manual Richards runs in both `rxvm` and `rxbvm`.

The gate stops here. The next authorized action requires Adrian's separate
approval: run the smallest decisive ordinary profiling-off Release timing
comparison against retained C0 evidence, then report the first Release verdict.

[`checksums.sha256`](checksums.sha256) closes the four evidence files in this
directory.
