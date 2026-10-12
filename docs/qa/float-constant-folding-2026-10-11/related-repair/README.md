# Numeric constant repair follow-up — 11 October 2026

All eight reproduced findings FSCAN-01–08 are repaired and verified locally in
the existing develop checkout at `bb0d211a8d17f0b66fdd6fcb99ad6ffd849a0283`.
Remote develop was checked and still has that revision. Changes are uncommitted
and unpublished for review. The separate Level L research checkout, retained
research toolchain and historical evidence were not changed.

The authoritative plan and acceptance status is
[float_constant_folding_worklist.md](../../../../compiler/docs/float_constant_folding_worklist.md).
The [pre-repair scan](../related-scan/README.md) retains the actual discrepancies,
sources, commands and hashes. The original finite-float repair receipt is
[here](../README.md); its evidence remains historical and intact.

## Cause and repair

| Finding | Local repair and decisive permanent control |
| --- | --- |
| FSCAN-01: integers reconstructed from display text | Preserve typed integer/Boolean payloads through rewrites and both propagation paths; emit exact integer operands. DIGITS 5 tests cover 2^53+1, a folded sum, INT64_MAX, later runtime arithmetic and unchanged scientific display. |
| FSCAN-02: float-to-decimal reconstructed from 15-digit display | Use the selected provider's `decimalFromDouble`, matching its existing 16-digit runtime conversion. DIGITS 18/5 round-trip and decimal/context controls pass. |
| FSCAN-03: fractional float-to-int accepted with an epsilon | Require finite in-range integral floats, matching checked FTOI. Near-integer and low-DIGITS propagated fractions reject; valid text/integral casts remain correct. |
| FSCAN-04: NaN collapsed into three-way equality | Represent unordered float comparisons explicitly; all six operators agree with actual runtime NaN arithmetic. Direct comparison also avoids subtracting infinities to classify ordering. |
| FSCAN-05: subnormals rejected by strict text parser | Parse source float literals with the same binary64 overflow/underflow policy as RXAS. Subnormal sum/positive and underflow-zero controls pass. Shared strict text/loose numeric-key parsing remains unchanged. |
| FSCAN-06: non-finite RXAS/RXDAS/inline transport | Approved case-insensitive `inf`, `+inf`, `-inf`, `nan` and exact `nan(<16 IEEE bits>)` operands retain FLOAT opcodes. Compiler/RXDAS and inline fields preserve bits. Decimal exceptional values use existing provider text conversion. |
| FSCAN-07: unary minus lost the zero sign | Emit and fold the same floating multiplication by -1. Negative zero/double negation retain division signs; runtime/folded NaN negation retains identical sign/payload/quieting behavior. A volatile multiplier prevents the host C optimizer substituting unary minus. |
| FSCAN-08: string consumers bypassed explicit casts | Fold/emit the declared cast first, then the consumer promotion. SAY, string assignment, chained/typed casts and checked failures pass; existing object-cast success/failure controls pass. |

Typed values are separate from contextual display. User formatting, strict and
loose string comparison, decimal context and valid folding remain enabled.
No tolerance, runtime rounding or broad optimization guard was introduced.
The decimal rewrite copies its source before pruning children that can own it.

Adrian selected the readable inf/nan family and corrected the initial `nf`
typo to `inf`. Bare NaN is the canonical positive quiet NaN; other signs,
payloads and signaling bits use the exact form. Opcode identities, RXBIN layout,
inline record arity/enum identities and finite inline spelling are unchanged.
Bare RXAS inf/nan operands are now float literals, including positions that
previously admitted a branch-target or constant-alias identifier. Functions
such as `inf()` and label definitions such as `inf:` retain their tokens;
use other names for bare branch targets/aliases. Rexx source syntax is unchanged.
The internal RXAS guide, user-facing assembler chapter and VM specification
document the notation and this consequence.

The earlier generic RXDAS register-expansion prototype is rejected and removed.
`expansion-global-proof.json` retains why: a callee stores integer 42 in a global
and returns an immediate float into that global. RET_FLOAT preserves its integer
payload; substituted RET_REG overwrote it with zero. LOAD_REG also has aliasing
semantics. The saved `rxas-bit-literal-proposal.patch` is a superseded, unapplied
0f proposal, not part of the repair.

## Permanent validation and exact evidence

`numeric_constant_contract` executes **476 tool commands / 200 VM runs** in
each complete Debug/ASan run. It covers B/G, optimized/unoptimized compiler and
assembler, rxtvm/rxbvm, linked images, ordinary and isolated source/RXAS/RXBIN
imports, success/rejection and later runtime use. Fractional constant casts
reject at compile time with BAD_CONVERSION; unoptimized checked casts reject
at runtime with CONVERSION_ERROR. Both reject instead of producing an integer.

An independent provider fixture carries an exact negative NaN payload through
RXAS/RXBIN imports. The optimized consumer must actually inline and emit its
bits. RXDAS tests independently inject pool values through the owning RXBIN
reader/writer: both infinities, canonical/noncanonical/signed/signaling NaNs,
the original 0.30000000000000004 value, negative zero and a subnormal.
Binary storage verifies every bit. LOAD_FLOAT must preserve integer 23 in the
same register, and RET_FLOAT must preserve global integer 42. Both assembler
modes and VMs pass after disassembly/reassembly/linking. Readable case/sign
spellings, function/label tokens, decimal special promotion and malformed
NaN-bit rejection are also checked.

The original `float_constant_folding_contract` remains enabled and passes:
**156 commands / 64 VM runs**, including actual CLI-fed runtime addition versus
the folded counterpart, the residual, equal float and decimal controls,
display/string/context behavior, exact emission and isolated imports.

- Final isolated new aggregate: Debug **1.34 seconds**, maintained Apple ASan
  **25 seconds**, measured before registration. CTest uses RUN_SERIAL and a
  generous 300-second hang backstop.
- Final normal correctness selection: **58/58**, including object-cast
  success/failure controls. Numeric, comparison, decimal, constant propagation,
  casts/constructors, binary/register views, contexts, inline mutation and
  metadata/disassembly controls all pass. The earlier 48-test sweep is
  superseded because decimal special promotion was subsequently completed.
- Registered maintained Apple ASan: **3/3**, **33.53 seconds** in the earlier registered run, no sanitizer
  findings. These are the two aggregates and independent metadata contract.
- Core/prerequisite normal builds: **zero warnings**. Earlier ASan recompilation
  emitted existing Apple SDK sprintf deprecation warnings in unchanged
  rxdamain.c/rxvmvars.h functions; its full log is retained. Recompiling the final negation change reports the same two header warnings;
  they do not originate in the repair. No blanket suppression was added. Apple LSan is unsupported; this
  receipt does not claim Linux leak or cross-platform qualification.

`final/manifest.json` retains base/host/configuration, code/test/build-input
SHA-256 hashes, tool/runtime hashes, counts and evidence hashes. Full command
logs retain argv, exit status, stdout and stderr for every aggregate command.
Build, CTest, measurement, import-selection and representative emitted/dumped
RXAS logs are in `final/`. Failed development fixtures were corrected before
the final proof; they were fixture operand/context errors, not product failures.
No broad/overnight/hosted run was dispatched. No unchanged passing broad gate
was repeated. Narrow repeats followed restoration of the assembler source edit
and a final NaN-negation repair; their exact logs are retained.

No reproduced defect remains open in the eight-finding scope. Other decimal
providers and host platforms are not qualified by this local run. RS-F01's
original compiler discrepancy is resolved, and the compiler blocker under
RS-AC04 is addressed. The research owner must adopt/requalify the repair;
the wider Level L worklist is not claimed complete.


## Final source and negation reconciliation

The final source audit caught the assembler decimal-normalization edit restored
to its original contents after earlier testing. The specific edit was restored,
and the affected normal/ASan tool build and contracts were repeated. The source
is retained with the edit, and final hashes are refreshed after verification.
No separate research checkout was edited or stopped.

An additional exact-bit probe at the FSCAN-07 boundary exposed `-(0.0 / 0.0)`:
noopt matched actual runtime negation (1), optimization disagreed (0). C unary
minus used in folding toggled the NaN sign, whereas emitted arithmetic
propagated/quieted the NaN. Folding now executes the same multiplication as
emission. `nan-negation-before.json`, source and RXAS/RXBIN fixtures preserve
that discrepancy; the numeric aggregate permanently compares the runtime and
folded result via binary storage. Host Clang `-O3` IR retains the volatile read
and multiplication, avoiding a native optimizer's unary-minus substitution.
The final normal aggregate takes 1.54 seconds; command/VM counts are unchanged.
The corresponding final maintained ASan run is retained in
`final/asan-negation-final-ctest.log` and qualifies the completed repair.
