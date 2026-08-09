# Compiler and code-shape findings

The source/expansion sanity ledger passed 368 checks. Both engines contain the
same 651 definitions in the same order; all 176 public handlers selected inline
have token-identical implementations and identical opcode/label mappings.

The R2 loss was not caused by changed helper sub-inlining or instruction search.
For the selected public handlers, Clang and GCC report zero changed successful
helper-inlining decision. The owner still resolves inline handlers directly.

The primary Clang defect was the pointer-rich handler facade. Making one
outlined call reachable forces hot owner locals to remain addressable even when
that call never executes. One, eight and 49 never-executed outlined sites are
all adverse but are not ordered by count. Retaining 651 unused wrapper bodies
is neutral. In threaded Clang, the facade also makes the 650-entry label map
escape, growing the observed owner frame from about 2192 to 6832 bytes and
adding a stack probe.

No governed workload took an interrupt. The semantic-invalid no-poll ceiling
improves switch dispatch but worsens threaded dispatch, so the interrupt poll
does affect compiler shape but cannot be removed. The continuation funnel with
no call is neutral.

Clang is repaired by taking a value snapshot only at one shared cold outlined
entry. GCC generates better dispatch from the original per-identity pointer-
facade lowering, so the experimental panels use compiler-specific lowering.
Applying the shared-cold form universally was rejected after GCC `rxtvm` lost
5.33% without Base64 while GCC `rxbvm` gained 13.67%.

The first cross-compiler verdict also caught an all-inline control defect:
deleting facade source that optimized away changed threaded compiler heuristics.
Restoring the exact R2 all-inline source shape returned all four controls to
within -0.280% to +0.520% of R2 without Base64 and restored the exact owner
lengths.

Finally, native Bounce samples contain `PRIVATE_R1_RELINK` and the cold handler
trampoline when private fusions are outlined. R2 attributed that dispatch to
public `UNLINK`, so its zero-outlined-call claim was incomplete. Both private
fusions are now explicit inline policy members. The selected panel is 178/590
non-reserved public-plus-private definitions, or 30.17%.
