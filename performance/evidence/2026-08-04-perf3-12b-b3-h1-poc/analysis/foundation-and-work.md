# H1 foundation and exact-work accounting

## H1-only work

The generated-RXAS replay changes `main .locals` from 103 to 104 and redirects
the first existing target concat, its exact `C` TRACE and its `STEMSET` key from
r67 to r103. No executable setup instruction is added. The first concat remains
the lazy conditional cache seed; r103 then stays live until the next `lvar`
generation.

- selected later-site weights: `280,000 + 560,000 + 560,000 + 560,000 =
  1,960,000` removed CONCAT dispatches;
- retained seed materialization: `280,000` CONCAT dispatches;
- added setup: zero dispatches;
- net dispatch reduction: `1,960,000`;
- total resulting `CONCAT_REG_STRING_REG`: `560,002`, including the retained
  target seed and independent `"1.0" || lvar` default-tail site;
- latest accepted X1 fixed-work authority: `52,839,051` instructions under
  both VMs;
- B1 metadata result: identical hot work; and
- derived H1 result: `50,879,051`, or `-3.709378%`.

No counts product was built for B3. These are exact derivations from the
retained accepted profile and source weights; B4 must confirm them.

## Payload and allocation

For `lvar` 1 through 14, the joined-tail payload length totals
`14 * 8 + (9 * 1 + 5 * 2) = 131` bytes per selected weighted site set. Seven
removed site executions per `lvar` cycle and 20,000 outer iterations give
`7 * 20,000 * 131 = 18,340,000` joined temporary payload bytes avoided,
excluding terminators and value-structure writes.

H1 adds no left-segment load and no second key materialization. All affected
strings fit the 32-byte inline representation, so the removed temporaries do
not own heap buffers. The retained seed and logical stem-key allocation are
unchanged. Heap-allocation count is consequently expected to be neutral, but
this remains a handler/representation inference for B4 to confirm.

## Lazy placement and preheader audit

All four selected reuses report:

```text
speculatable=1
must-execute=0 (not-must-execute)
right-invariant=0 (not-invariant)
trace-free=0
preheader-eligible=0
```

The seed occurs after the zero-iteration/conditional exit, the right-side
string is regenerated for each `lvar`, and its `C` TRACE is an ordered event.
Lazy first use is therefore both cheaper and semantically justified; no code is
moved to a preheader.

## Common F1 metadata effect

Exact CONCAT/SCONCAT string-component metadata is shared with B2 S1. It also
enables one unrelated existing X01 placement:

```text
dcopy r86,r89; dtos r86; stemset r8,r85,r86
    ->
dtos r89; stemset r8,r85,r89
```

The H1 control uses the B2 F1/S1 tool on the exact H1 input. S1 selects zero
sites because no stable unpunctuated left segment exists. Control main output is
`380 -> 369`; H1 is `380 -> 365`. F1 is common and excluded from all H1-only
counts above.
