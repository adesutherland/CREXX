# PERF3 fusion and quickening registry

Status: **complete — retained production mechanisms documented; adaptive
quickening closed**

Recorded: 2026-08-17

This register closes `PERF3-05-R4`. It describes the public serialized
superinstructions selected by rxc or RXAS together with the two process-private
VM fusions. It does not propose a new opcode, RXBIN change or VM candidate.

## Contract boundaries

- Public entries are ordinary RXAS/RXBIN instructions. Their opcode is visible
  to disassembly, debugging, TRACE/source metadata and semantic profiling.
- Private entries specialize only the process-owned execution image at module
  preparation. Canonical RXBIN and `segment.binary` remain unchanged.
- Static counts below are sites, not executions. Ordinary profiling-off Release
  wall clock remains the performance authority.
- RXAS may select a sequence only with the proof recorded here. When proof
  fails, the canonical sequence remains. There is no broad fail-closed removal
  of a representable optimized shape.

## Public serialized registry

| Family | Canonical input or meaning | Proof owner and eligibility | Fallback and observability | Disposition |
| --- | --- | --- | --- | --- |
| In-place concatenation | `concat d,d,s -> append d,s`; `sconcat d,d,s -> sappend d,s` | RXAS local operand identity, with its ordinary hazard rule | Expanded concatenation remains; selected opcode is public and profile-visible | retain |
| Counted loop branch | `inc c; br L -> bctp L,c` | RXAS exact executable adjacency and captured counter | Expanded increment/branch remains | retain |
| Packing | adjacent `swap`, call-window `load`/`settp`/`swap`, and adjacent `null` groups -> `swapn`, `loadsettp2`, `loadsettpswap`, `settpswap`, `swapsettp`, `swapsettpswap`, `settpswapsettpswap`, `nulln` | RXAS exact ordered operands and adjacency; packing preserves each component operation in order | Expanded operations remain; source metadata stays outside the public instruction | retain |
| Integer compare/branch | `IEQ/INE/IGT/IGTE/ILT/ILTE t,a,b; BRT/BRF L,t -> BEQ/BNE/BGT/BGE/BLT/BLE` | RXAS K04 CFG/component-SSA plan proves one block, total/context-neutral operations, unaliased result, hidden cleanup, exact incoming value and complete call window; matching TRACE result events are revalidated atomically | Any failed proof leaves compare, TRACE and branch intact; fused branch is the semantic profile identity | retain |
| NR-09 object/alias cleanup | `igetunlink`, `iloadsetunlink`, `iloadsetunlinkn`, `isetattr1`, `isetunlink`, `isetunlinkn`, `linksetattrslinkadd`, `minlinkattr1`, `setlinkattr1`, `setlinkiload`, `unlinkbr`, `unlinkn` | RXAS Class 1 backstop where adjacency is sufficient; rxc Class 2 final-stream proof where temporary identity, alias, TRACE retargeting, bounds or cleanup facts are required | Expanded public sequence remains whenever the owning proof cannot establish all effects and signals | retain |
| NR-09 call and arithmetic forms | `settpcall`, `settpswapcall`, `swapcall`, `fdivsub`, `fmulticopy` | Exact RXAS Class 1 mapping or rxc Class 2 liveness/temporary proof according to the NR-09 mapping register | Expanded calls/arithmetic remain; public instruction records any contractually visible intermediate effect | retain |
| Transactional PARSE assists | `parseplan`, `parsepos2`, `parsewords3`, `parsewords3d` | rxc owns source-level plan/result knowledge; existing instructions are used only for the exact supported contract | Existing scalar string/integer/branch lowering remains. PERF3-12D closes generated dynamic `parseExec`; this is not future quickening work | retain instructions; close programme item |
| Two-register conversion | `itof destination,source`; `stoi destination,source` | rxc/RXAS select only the proved NR-09 mapping; one-register in-place forms are not counted as fusions | One-register or explicit copy/conversion shape remains | retain |
| Legacy compare/branch opcodes | `igtbr`, `iltbr`, `fgtbr`, `fltbr` | Public authored compatibility forms. Current K04 does not select them; float comparison lacks the selected equivalent component/signal proof | Separate compare/branch remains | retain compatibility; no new selection |
| Exact copy/attribute cleanup | `copy d,s; acopy d,s -> copy d,s` | RXAS K06 exact same operands and executable adjacency | Both instructions remain for different operands or any executable gap | retain rewrite; no separate serialized identity |

The authoritative instruction contracts remain in
`docs/reference/rxas/instructions/` and the detailed proof contracts remain in
`docs/ai-context/RXAS_ASSEMBLER.md`. This table is the performance ownership and
disposition register, not a second opcode specification.

## Process-private registry

| Identity | Canonical RXBIN shape | Preparation proof | Runtime guard and fallback | Identity/observability | Accepted evidence | Disposition |
| --- | --- | --- | --- | --- | --- | --- |
| `PRIVATE_R1_RELINK` | `unlink destination; linkref destination,source_reference` | exact opcode/arity/adjacency; same destination; distinct source | fast relink only for a valid reference target; otherwise resume canonical `linkref`; debug or pending breakpoint uses canonical execution | private dispatch identity for placement; semantic profiles attribute the first canonical `unlink` | accepted List work-100: -1.151991% `rxvm`, -3.022743% `rxbvm` paired median | retain immutable load-time fusion |
| `PRIVATE_R2_COPYATTR1` | `linkattr1 temporary,object,immediate; copy destination,temporary; unlink temporary` | exact opcode/arity/adjacency and all three registers distinct | fused descriptor path only for the supported reference payload; range error, generic value, debug or pending breakpoint executes canonically | private dispatch identity for placement; semantic profiles attribute the first canonical `linkattr1` | accepted List work-100: -2.731% `rxvm`, -1.745% `rxbvm` paired median | retain immutable load-time fusion |

Both concrete VMs prepare the same two identities. `rxtvm` stores a private
label and `rxbvm` stores a numeric private opcode above the public range. There
is no adaptive rewrite, subsequent dequickening or mutation of canonical code.

## Portfolio-v2 static census

The 2026-08-17 census disassembles the final optimized RXBIN for all 17
portfolio-v2 cREXX images. This matters: counting compiler RXAS alone would
miss fusions selected later by RXAS.

| Measure | Count |
| --- | ---: |
| Public serialized sites | 1,248 |
| Observed public identities | 34 |
| Exact private R1 eligible sites | 18 |
| Exact private R2 eligible sites | 16 |
| Total public plus private-eligible sites | 1,282 |

The private sites are concentrated in List (14), Towers (9), NBody (7), Bounce
(2) and Richards (2). The complete per-identity and per-workload records are in
the retained evidence. Zero sites are an honest portfolio observation for
`fmulticopy`, the four legacy `*gtbr`/`*ltbr` forms, `iloadsetunlink`,
`parseplan`, `sappend`, `settpswap` and `settpswapsettpswap`; it is not a claim
that their public contracts are unused outside this portfolio.

## Dynamic evidence and profiling contract

The current semantic profiler deliberately maps a private execution back to
the first canonical public opcode. That keeps public instruction profiles
comparable but means they are not an exact dynamic census of private identity.
The separate effective-placement hook reports `inline`, `outline` or `mixed`;
it does not add a 651-handler identity table.

Retained native sampling proves hot `PRIVATE_R1_RELINK` execution in Bounce and
corrected the earlier R2 claim that no outlined handler ran. The later R5a
profile records 887,867,426 Bounce instructions, of which 424,204
(0.047778%) are outlined and 424,200 are `CALL1_REG_FUNC_REG`; it proves no
hidden outlined hot path in that build, but it is not an exact R1/R2 execution
count. No private dynamic number is inferred from those data.

## Gap report and final decisions

1. **RXAS cannot prove the two runtime payload facts.** R1 depends on the
   loaded value being a valid reference target. R2 depends on the loaded
   one-based attribute having the supported reference descriptor and preserves
   canonical range/error behavior. Static promotion would need a new public
   instruction contract and RXBIN compatibility decision; the current 34 exact
   sites do not justify that architecture change.
2. **There is no uncovered private miss.** Every exact R1/R2 site in the final
   optimized images is eligible for the existing immutable preparation. The
   census therefore supplies no residual exact shape that adaptive quickening
   could recover.
3. **Adaptive state remains rejected.** The measured Q7 substrate requested
   56,264/62,536 bytes, tied the direct reference option on Bounce, was neutral
   on Richards and retained lifecycle gaps. Lazy specialization learned no
   durable fact. Reopening it would add state, transition and invalidation cost
   without a current miss population.
4. **Dequickening is unnecessary for the retained mechanism.** Debug and
   breakpoint guards select canonical execution at the site, while canonical
   RXBIN never changes. The prepared mapping is immutable for the module load.
5. **Profiling remains deliberately two-level.** Public semantic attribution
   stays canonical; placement diagnostics retain private awareness. A separate
   exact private dynamic schema is deferred until a concrete decision cannot be
   made from static eligibility, native attribution and placement evidence.

Result: retain all accepted static/public mechanisms and both exact private
fusions; close adaptive quickening, dequickening, selector caches and new public
fusion opcodes as PERF3 activities.

## Evidence

- [`2026-08-17-perf3-closeout-fusion-registry`](evidence/2026-08-17-perf3-closeout-fusion-registry/)
- [`PERF2-05 R2a verdict`](evidence/2026-07-26-perf2-05-r2a-first-release-verdict/)
- [`PERF2-05 R1a verdict`](evidence/2026-07-26-perf2-05-r1a-first-release-verdict/)
- [`PERF3-05 R3 analysis`](evidence/2026-08-09-perf3-05-r3-handler-codegen-analysis/)
- [`PERF3-05 R5a placement profile`](evidence/2026-08-10-perf3-05-r5a-handler-placement-profiling/)
