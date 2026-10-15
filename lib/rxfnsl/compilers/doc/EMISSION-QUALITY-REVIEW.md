# STEP04 emitted-code review

Historical inspection, 15 October 2026. Reviewed develop
`bf74c72111fbd466d1f93c421e6d5dadbbedcc2c`. **Review and acceptance update;
no compiler implementation change in that review.** The [programme plan](../PLAN.md) owns
AC16, EQ-AC01–05 and the repair sequence. Root AGENTS.md now makes emitted-code
quality a standing compiler requirement. The original STEP04 functional tests
remain useful, but did not establish good code generation or complete source
metadata fidelity.

## Evidence and comparison boundary

Compiled the existing `literal` and `pipeline` examples with `llrxc-g`,
`llrxc-c`, native `rxc` for G and B, and native `rxc-c`, using normal and `-n`
native compiler modes. The B sources differ from G only in `options levelb`.
All 16 compilations succeeded. Each result assembled with **`rxas -n`**, linked
with its runtime packages and ran under `rxbvm`, retaining source metadata.
All produced the independently expected output: literal C `01`, literal B/G
`1`; all pipelines `10`, `0`, `3`. No product rebuild, broad QA, optimisation
change or timing experiment was performed.

The [review receipt](../evidence/emission-review-20261015/README.md) retains
commands, identities, counts, execution observations and selected raw RXAS.
Existing [STEP04 examples](../examples/step04/README.md) retain source and IR.
These are static observations on tiny programs, not representative performance
results. The installed native executable version banner is an older embedded
build label; the receipt identifies actual tool bytes by SHA-256.

| Example / compiler | Executable instruction lines | `.srcstep` records | Call sites | Declared frames (`.locals`) |
|---|---:|---:|---:|---|
| `say 01`, new G | 12 | 6 | 1 | wrapper 16; body 16 |
| `say 01`, native B or G, `-n` | 5 | 2 | 0 | main 1 |
| `say 01`, native B or G, normal | 3 | 2 | 0 | main 0 |
| `say 01`, new C | 78 | 11 | 10 | wrapper 16; body 21 |
| `say 01`, native C, `-n` | 75 | 32 | 13 | main 11; body 18 |
| `say 01`, native C, normal | 83 | 46 | 11 | main 11; body 22 |
| pipeline, new G | 90 | 69 | 3 | wrapper 16; main body 57; twice 20 |
| pipeline, native B or G, `-n` | 30 | 17 | 3 | main 5; twice 2 |
| pipeline, native B or G, normal | 19 | 18 | 0 | main 4 |
| pipeline, new C | 551 | 99 | 55 | wrapper 16; main body 77; twice 29 |
| pipeline, native C, `-n` | 867 | 395 | 138 | main 11; combined body 111 |
| pipeline, native C, normal | 842 | 495 | 128 | main 11; combined body 133 |

Counts include every executable line in the emitted module, including entry
and error handling, but exclude comments, labels, imports and directives.
They are not dynamic instruction counts. Fused native instructions count as
one line. Frames are separate functions, not a summed live-register estimate.
Native C combines source routines differently; new C delegates much work to
`llctarget` and existing Classic libraries. The smaller new-C caller therefore
does **not** prove lower total cost. Native normal mode can grow output through
inlining, even when it removes calls. Native G/B also carry semantic TRACE
events that the new backend does not yet emit: the pipeline has 20 events in
native `-n`, 14 in native normal mode, and zero in the new output. Dropping
metadata is not a valid way to win a size comparison.

## Findings and ownership

### EQ-01: source records lose meaning — current defect

`llrxas.compilation.source()` emits one `.srcstep` per IR operation, uses the
same origin ID for both step and clause, hardcodes flags **17** and sets the
end column to start plus one. In `rxbin.h`, 17 means **authored + exact**.
The IR origin already exposes `kind()`, `last()` and parent provenance, but the
emitter ignores those here. Frontend-created setup also reuses ordinary source
origins instead of consistently deriving generated origins.

For example, the new G `say 01` body describes `say` with columns 1–2 and its
literal with 5–6. Native B/G identifies the statement range 1–7. The new
statement and literal receive different clause IDs, although they belong to
one authored clause. Classic pool/argument setup is repeatedly described as
exact authored text at the start of `options levelc`.

This is not a missing constant-folding pass. Repair frontend origin assignment
and backend range/provenance/clause mapping. Keep distinct anchors for genuinely
different expressions and generated code. Multiple anchors on one line are
often correct; a line number alone is not an identity. Full semantic TRACE
events remain a separate, already-open language/tooling obligation.

**Owner:** frontend + RXAS source mapper. **Closure:** EQ-AC01/02/05, including
Unicode, CRLF, multiline ranges, generated/derived code and clause grouping.

### EQ-02: repeated mapping and location updates — current emitter defect

The new pipeline has 69 source records for G and 99 for C. In the ordered
source-record stream, respectively 21 and 44 records repeat the immediately
preceding record exactly. These are candidates for coalescing, not a claim that
every occurrence can be deleted: labels, joins and separate function entries
must retain correct mappings. The literal C example has eleven records but
only three distinct record texts.

`.srcstep` is an address-stamped metadata record, not an executable TRACE-print
instruction. The assembler's `rxasmestp()` adds a metadata entry to the constant
pool. Its size and debugger meaning matter even when it adds no executable
opcode. Classic also emits an actual `load location,"file:line:column"` before
each such record: **99 location loads** in the pipeline, with only 30 distinct
location strings. Repeated loads inside an unchanged straight-line region do
real redundant work.

Coalesce source mapping by meaningful source transition and control-flow
boundary; update the diagnostic register where required for observable failures.
Do not globally deduplicate records or cache location state across control-flow
joins without proof. Shared data-flow optimisation may improve this later, but
emitting unchanged state for every IR operation is a direct backend issue now.

**Owner:** RXAS emitter. **Closure:** EQ-AC02/05 with branches, loops, skipped
errors, calls and nested failures.

### EQ-03: physical input paths escape into output — current output defect

The CLI passes its input filename unchanged into `llcompiler.request`; the
backend repeats it in `.srcstep` and Classic location strings. Supplying an
absolute input path embeds the full temporary/checkout path. The fresh native
B/G/C controls use the source basename for these examples. The stored new
pipeline examples instead show `../step01/pipeline-*.crexx`, because that was
the filename supplied during their generation.

Separate physical file access from logical source identity. The proposed
default is a basename for a single-file command, with deliberate stable source
mapping for projects and service callers. A resolver must disambiguate equal
basenames rather than silently confusing files. The exact API/mapping surface
needs review before implementation; no new option is claimed to exist.

**Owner:** command/service boundary + source registry/emitter. **Closure:**
EQ-AC01/05, identical logical inputs under two roots and collision tests.

### EQ-04: call-register groups have an ABI purpose

The repeated groups are not all redundant initialisation. For a three-argument
Classic pool write, selection emits the equivalent of:

```rxas
mkref r69,r19
copy r70,r18
load r71,"TOTAL"
load r68,3
settp r69,768
settp r70,768
settp r71,768
call r66,llctarget.poolwrite(),r68
null r69
null r70
null r71
```

Here `r68` holds the count and the next registers form a private argument
window. 768 is `REGTP_VAL | REGTP_NOTSYM` (256 + 512), the compiler call flags.
The pool is borrowed by reference; the value is copied. The `null` operations
release the window contents after return. Removing a group on appearance alone
can change ownership, argument presence or values. Native code uses some fused
call forms; the new selector emits the pieces separately.

A different group of three occurs inside `llctarget.binaryop/unaryop`: digits,
fuzz and form are extracted and installed in the helper's VM frame. Returning
restores the caller's settings. That protects Classic numeric semantics, but
repeating generic dispatch, numeric extraction and helper context construction
for every operation is a substantive lowering-cost concern. Ordinary copy
propagation alone will not remove the whole abstraction boundary.

**Owner:** target adapter + shared backend. **Disposition:** preserve the current
ABI while reviewing direct/specialised selection and activation/numeric facts
under EQ-STEP03 and STEP05. Prove errors, mutable NUMERIC, recursion, borrowing
and lifetime behaviour before any reduction. Include helper-body costs in the
comparison. Neither the old compiler nor the new adapter is an optimality oracle.

### EQ-05: unused defaults and frame scaffolding — baseline quality defects

The frontend's `Say` arm constructs a default empty value before testing whether
an explicit operand exists, then overwrites its local handle. The unused IR
operation remains. For G this becomes an unused `load ..., ""`; for C it becomes
a pointless `llctarget.literal("")` call. Classic assignment has the same eager-default
construction pattern. Construct the default only for the source form that needs
it. A later dead-operation pass should not be necessary to hide this mistake.

The backend starts `maxargs` at eight and sizes it from every IR operation's
inputs, rather than actual selected calls. A call-free literal G body therefore
declares sixteen locals, although its highest referenced local is r4. Its entry
also branches to the immediately following block. The fixed wrapper/frame floor
and unconditional entry jump are conservative implementation conveniences,
not language requirements. Right-sizing actual scratch/call needs and choosing
fall-through do not require a sophisticated register allocator. Keep call/result
separation and frame-budget checks when correcting them.

**Owner:** frontend + target selection/frame layout. **Closure:** EQ-AC03/05,
explicit/empty SAY, assignments, call-free bodies, real maximum-arity calls and
recursive/error paths. Stable one-location-per-value allocation remains the
approved initial strategy; broader reuse belongs to EQ-07.

### EQ-06: `generated Classic entry` is diagnostic plumbing

This is a string loaded into the function's diagnostic-location register before
four fatal-signal handlers are installed. It is not an operation mandated by
Classic Rexx. The local handler exists because a failure in the adapter/runtime
can be too far from the authored caller for the VM's short panic backtrace;
it attaches the saved location and re-raises the same signal.

Retain correct error attribution. Use meaningful entry provenance for the
fallback, and reduce repeated location writes under EQ-02. Longer-term source
and activation contracts should support errors directly without making every
expression maintain a presentation string. That design belongs to the target
review; this review does not authorise a VM change. The string alone is not
evidence that all Classic lowering is wrong.

**Owner:** backend diagnostics / target contract. **Closure:** EQ-AC02/04/05.

### EQ-07: real remaining IR/target optimisation

The new G pipeline keeps `2 + 3`, repeated slot loads/stores and copies, the local
`twice` call, and separate conversion temporaries. Each IR result gets its own
register. Native normal mode folds constants and `length('cat')`, inlines the
small function, uses immediate operands and needs a much smaller frame.

Shared constant evaluation, propagation, branch simplification and safe dead
operation elimination are explicitly STEP05. Liveness/register reuse and richer
instruction selection need their own checked target contracts. Classic also
constructs pool/argument/numeric machinery even in a program that only says a
literal; removing unused activation components needs a reviewed use/effect and
failure contract. Generic passes must respect Classic exact text, unknown calls,
numeric snapshots and potential conditions, and G's typed/short-circuit rules.

This is where purposeful IR designs should pay off. Keep constants, state uses,
effects and origins visible to shared analyses; avoid burying all meaning in
opaque generic helper calls before the passes can use it. Both C and G must
show improvements. These obligations remain open, with EQ-STEP03/04 and
LLCOMP-AC06/13/16 as their closure gates.

## Recommended next increment

Repair EQ-01/02/03/05 and the EQ-06 fallback in a focused STEP04 follow-through,
with a short review of logical filename/clause mapping first. Then review the
Classic target contract and proceed with the already-planned shared passes.
Preserve the Level B-only implementation and Level L emitter boundary.

The subsequent [condition-provenance design](CONDITION-PROVENANCE-REVIEW.md)
narrows the immediate diagnostic proposal following Adrian's direction to get
the essentials right without a wholesale redesign. It keeps the current handler
and necessary authored-location field, corrects names/ranges/provenance and the
loss of original fault on re-raise, and removes redundant updates. Proper clause
grouping and a possible later adapter-boundary design remain open; the interim
uses unknown clause IDs rather than pretending origin IDs are clause identities.
This does not remove the other findings or their existing acceptance gates.

Add structural assertions for the actual defects, alongside execution and error
tests; do not freeze incidental register numbering or demand that unoptimised
code match the native optimiser. Current passing functional evidence stays
valid for its stated inputs. Source metadata portions of F4-AC03/05 are reopened;
F4-AC07 is open. No code-quality gate is closed by this report.

## Mechanism references

- [Frontend construction](../driver/llfrontend.crexx): `at`, `statement` / `Say`.
- [RXAS backend](../rxas/llrxas.crexx): `invoke`, `procedurebody`, `source`.
- [Classic target adapter](../rxas/llctarget.crexx): `binaryop`, `unaryop`, `failure`.
- [IR origins](../../rexx/llir.crexx): `origin.kind/first/last/parents`.
- [RXAS source and TRACE contract](../../../../docs/ai-context/RXAS_ASSEMBLER.md).
- [Source flags and record layout](../../../../binutils/include/rxbin.h),
  [call flags](../../../../binutils/include/rxflags.h), and
  [assembler metadata construction](../../../../assembler/rxasassm.c).

## Essential correction after this review

The approved CP correction implements logical source names, full single-line
ranges, truthful generated/synthetic flags, explicit unknown clause IDs,
straight-path source/context deduplication and origin-preserving fatal relays.
The current worked examples include those changes; the table and raw review
evidence above remain the original `bf74c7211` baseline. See the
[bounded design](CONDITION-PROVENANCE-REVIEW.md) and
[correction receipt](../evidence/provenance-20261015/README.md) for its proof and
before/after counts.

This closes the bounded CP correction, not all review findings. Real clause
associations and complete TRACE events remain open. Unused operand/default
construction, excess frame/call windows, entry jumps, numeric/helper costs and
shared IR optimisations retain their existing plan gates. The Classic location
field remains necessary with the current function-level handler. Preserving the
original signal uses a small extra cold dispatch sequence; static caller line
counts are not a timing or total-runtime-cost claim.
