# S1 foundation and exact-work accounting

## S1-only work

The same-shaped PoC control and candidate both add one loop-local register and
one `LOAD` per `lvar` generation. The control loads the unrelated literal
`"Failed1"`, so the S1 proof cannot fire; the candidate loads `"Key Bee"`.
Both are assembled by the same PoC tool and therefore share the CONCAT
component-metadata foundation.

- selected target-site weights: `280,000 + 560,000 + 560,000 + 560,000 =
  1,960,000` removed CONCAT dispatches;
- stable-left provisioning: `280,000` added LOAD dispatches;
- net dispatch reduction: `1,680,000`;
- retained target CONCAT: `280,000` at source 159;
- total resulting `CONCAT_REG_STRING_REG`: `560,002`, including the independent
  `"1.0" || lvar` default-tail site;
- latest accepted X1 fixed-work authority: `52,839,051` instructions under
  both VMs;
- B1 metadata result: identical hot work; and
- derived S1 result: `51,159,051`, or `-3.179467%`.

No counts product was built for B2. The last two figures are a derivation from
the retained accepted profile and exact source weights; B4 must confirm them.

## Payload and allocation

For `lvar` 1 through 14, the joined-tail payload length totals
`14 * 8 + (9 * 1 + 5 * 2) = 131` bytes per selected weighted site set. Seven
selected site executions per `lvar` cycle and 20,000 outer iterations give
`7 * 20,000 * 131 = 18,340,000` joined temporary payload bytes avoided. The
new seven-byte left literal is loaded 280,000 times, or 1,960,000 bytes. Net
temporary payload writing avoided is therefore 16,380,000 bytes, excluding
terminators and value-structure writes.

All affected strings fit the 32-byte inline representation. The removed
CONCAT temporaries and the setup LOAD therefore allocate no heap buffer.
`STEMGET2` streams its segments. `STEMSET2` materializes a canonical joined key
only for a new insertion, matching the retained logical key allocation of the
one-part set. Heap-allocation count is consequently expected to be neutral,
but this is a handler/representation inference. The retained accepted profile's
280,023 `string_buffers` is whole-workload data without site attribution; B4
must confirm allocation neutrality in a counts build.

## Common F1 metadata effect

The PoC adds exact string component reads/writes for CONCAT/SCONCAT. On the
unmodified accepted optimized RXAS, where no stable-left register exists and
S1 selects zero sites, that metadata enables one extra existing X01 component
placement:

```text
dcopy r86,r89; dtos r86; stemset r8,r85,r86
    ->
dtos r89; stemset r8,r85,r89
```

The B1 tool emits SHA-256 `cca6bbcb92a640e4a06b93999aa270c4813d60fd9afa3ee94dd26cf0274b7ffd`
at 68,521 bytes. The PoC tool emits
`5c2417812bec33c13436429fa3e10f456d3bc55565c519d58e1d9618f188cf8e`
at 68,513 bytes, deleting one `DCOPY` and matching TRACE metadata with no S1
selection. This F1 result is an independent common-foundation observation and
is excluded from every S1-only count above.
