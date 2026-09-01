# PERF3-01 current-product decision summary

Status: **accepted by Adrian on 2026-07-31**

PERF3-01 rebuilt clean detached `3f43a0014`, restored exact current-product
Apple timing authority, captured focused current schema-5 counts and reconciled
them with retained Linux native-PMU attribution. No production source,
language rule, public interface, RXAS/RXBIN format or VM default changed.

## Current Mac scorecard

The ordinary profiling-off Release matrix completed `348/348` initial
executions. The one permitted governed append completed `50/50` executions for
`sieve-rxvm`, `permute-netrexx`, `richards-netrexx` and both Base64 VMs. All
five remain labelled noisy after `n=20`; no sample was removed and no second
append was taken.

| Workload | Status | `rxvm/ooRexx` | `rxbvm/ooRexx` | Target disposition |
| --- | --- | ---: | ---: | --- |
| Sieve | common | 7.196046x | 5.363161x | both exceed 1.50x; `rxvm` remains noisy |
| Permute | common | 8.245154x | 7.001531x | both exceed 1.50x; NetRexx comparator remains noisy |
| Bounce | common | 3.751685x | 2.867729x | both exceed 1.50x |
| Richards | common | 0.260465x | 0.256036x | needs +475.893%/+485.855% to reach 1.50x; NetRexx comparator remains noisy |
| Base64 | common | 0.773763x | 0.722197x | both cREXX cells remain noisy; needs +93.858%/+107.700% to reach 1.50x |
| Towers | separate qualified lane | 0.317918x | 0.310634x | needs +214.547%/+221.922% even for parity |
| RexxCPS | separate diagnostic | 0.964226x | 0.906008x | needs +55.565%/+65.562% to reach 1.50x |

The common-five geometric means are:

| Comparison | `rxvm` | `rxbvm` |
| --- | ---: | ---: |
| versus ooRexx | 2.139811x | 1.818954x |
| versus decimal NetRexx | 0.779920x | 0.662974x |

`rxvm` retains the 2.00x ooRexx aggregate band; `rxbvm` needs +9.953% to
reach it. Both fail the per-common-cell band on Richards and Base64. The
current run is an absolute same-session baseline, not a paired regression
verdict against PERF2; its valid conclusion is that the priority ordering has
not changed.

An earlier timing block was caught during audit with cREXX rows wired to the
retired PERF2 product. It is retained under `timing-invalid-old-product/`,
marked invalid and excluded from every current-product figure in this summary.

## Evidence boundary

- The PERF2-09 Mac closure, PERF2-08 qualification, PERF2-01 baseline and
  PERF2-06/07 accepted-closeout checksum authorities replay successfully.
- All 24 formal benchmark/lifecycle source rows and every comparator
  executable/JAR hash used here match PERF2-09 exactly.
- The five non-Base64/non-RexxCPS selected RXAS streams also match, preserving
  strong retained mechanism priors.
- Both VM executables, compiler/assembler/linker, linked library and all seven
  RXBINs differ; Base64 and RexxCPS RXAS also differ. PERF2 timing therefore
  remains historical rather than exact current-product evidence.
- Current profiling uses a separate clean Release build with
  `CREXX_VM_PROFILING=ON`; formal timing uses the clean ordinary Release build
  with profiling `OFF`.

## Current mechanisms

All ten selected schema-5 profile cells are complete. `rxvm` and `rxbvm`
execute the same deterministic selected counts, apart from immaterial
RexxCPS byte-counter differences caused by the VM representation.

| Workload | Dynamic instructions | Copies / bytes | Clear-reset-destroy / bytes | Allocations / bytes | Planning reading |
| --- | ---: | ---: | ---: | ---: | --- |
| Richards | 8,615,245 | 56,902,732 / 451,730,841 | 860,895 / 3,978,206 | 735,010 / 172,258,936 | copy/attribute ownership remains dominant |
| Towers | 7,143,456 | 26,789,582 / 206,454,410 | 31,345,134 / 41,935,002 | 17,243,733 / 5,864,230,528 | copy, teardown and allocation must remain separable |
| Base64 | 46,723,864 | 2,221,004 / 18,274,056 | 1,376 / 1,062,800 | 1,694 / 603,040 | generated work and VM/native layout dominate the residual |
| RexxCPS | 26,448,435 | 1,053,578 / 45,983,531 | 1,558,439 / 68,271,806 | 1,439,774 / 261,043,768 | 3,009,511 conversion operations keep conversion material |
| Sieve guard | 6,825,375 | 52 / 10 | 506,584 / 120,209 | 8,319 / 4,231,352 | no copy candidate; retain as zero-work/layout guard |

Retained Intel Linux native evidence supplies the machine footprint. Clang
Richards spends 77% of sampled cycles in `copy_value` and another 16.5% in
attribute trimming. Towers spends roughly 46.5-47.8% in copy, 17% in clear,
9% in reset and 5.5% in trim. Base64 leaves 92-93% in `run`, while RexxCPS is
about 46% front-end bound with decimal parse/format and string movement visible.
The exact platform-specific figures and ceilings are in
[`gap-mechanism-ledger.csv`](gap-mechanism-ledger.csv).

## Ranked PERF3 panel

1. **PERF3-02 — advance to design/PoC approval.** Full-copy, ownership and
   attribute storage remains the strongest general mechanism. Start with
   compiler proof/binding and explicit fallback; compare a runtime typed-copy
   ceiling without assuming a new RXAS operation.
2. **PERF3-05 — retain second as evidence/design.** Base64 is a current common
   deficit with little allocation volume, 46.7 million dynamic instructions,
   92-93% native `run` footprint and a strong compiler-direction control.
   Establish the semantic string/copy ceiling before any LTO/PGO, layout or
   private-stream candidate.
3. **PERF3-03 — advance as a bounded contract/ceiling review.** RexxCPS remains
   below its separate 1.50x band and current conversion counts/native symbols
   are material. Freeze grammar, locale, range, exception and rounding
   behavior before comparing implementations.
4. **PERF3-04 — keep evidence-gated and defer production work.** The retained
   concrete-access microbench remains a lead, but the selected current
   profiles record zero dynamic selector attempts and do not identify a live
   target contribution. Require a current target and hand-equivalent ceiling.

Rejected for automatic selection are benchmark-specific operations, a global
value-layout rewrite, changed conversion semantics, and preselection of LTO,
PGO or a private stream. Those may only re-enter through their recorded gates.

## Acceptance and stop

Adrian accepted the evidence boundary and ranked panel on 2026-07-31. PERF3-01
stops here and PERF3-02 may proceed through its bounded evidence/design and PoC
comparison. No PERF3-02/03/04/05 production candidate is selected or
implemented by this acceptance, and no push is authorized.
