# NR-09 macro timing and review report

This is a rerunnable decision aid for all 60 NR-09 forms. It compares each candidate macro handler with the sum of its legacy component-opcode averages and adds the corresponding transition costs.

## Measurement boundary

- These are schema-4 instrumented-profile signals, not ordinary Release verdicts. Instruction timing includes per-instruction profiler bookkeeping, so replacing several instructions with one macro mechanically removes several bookkeeping charges.
- The profile timer minimum is 41 ns while most short-handler averages are below that value. Counts and large totals are more useful than individual-call timings.
- Component averages are global opcode averages, not the exact old call sites replaced by each macro. Call and branch transition averages are global transition averages for the same reason.
- A call-bearing macro is marked `exact-cell-required`: its call cost is population-dependent, so a global `CALL` average cannot classify it as a saving or slowdown. Use a matched expanded/fused ordinary-Release cell.
- Per-form estimates overlap shared legacy opcode populations and must not be summed into a batch performance prediction.
- `possible-slowdown` and `indicated-saving` describe the instrumented estimate only. Ordinary profiling-off Release measurements remain decisive.

## Coverage

| VM | Forms | Used | Zero-use | Indicated saving | Possible slowdown | Exact cell required | Used with <100 calls | Macro-model retired | Actual count drop | Residual | Profile instruction count |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 60 | 28 | 32 | 20 | 5 | 3 | 6 | 1525298 | 1525282 | +16 | 17746147 -> 16220865 |
| `rxbvm` | 60 | 28 | 32 | 21 | 4 | 3 | 6 | 1525298 | 1525312 | -14 | 17746177 -> 16220865 |

The exact per-component global totals/averages are in `component-timings.csv`; all calculated per-form values are in `form-timings.csv`. The reviewable source of truth is `review-ledger.tsv`.

## Dynamically observed forms

| Form | Class | Family | Components | Temporary-register policy | Review bar | Calls | Full delta rxvm / rxbvm | Delta share ppm rxvm / rxbvm | Signal |
| --- | ---: | --- | ---: | --- | --- | ---: | ---: | ---: | --- |
| `SWAPN_REG_REG_REG_REG` | 1 | multi-swap | 2 | no-compiler-temporary-output | normal | 11200/11200 | -7.682716% / -17.287098% | -49.607789 / -110.351644 | indicated-saving / indicated-saving |
| `SWAPN_REG_REG_REG_REG_REG_REG` | 1 | multi-swap | 3 | no-compiler-temporary-output | normal | 1409/1409 | -66.086939% / -66.309185% | -80.525780 / -79.875693 | indicated-saving / indicated-saving |
| `SWAPN_REG_REG_REG_REG_REG_REG_REG_REG` | 1 | multi-swap | 4 | no-compiler-temporary-output | normal | 1406/1406 | -64.298387% / -57.703986% | -104.239531 / -92.482557 | indicated-saving / indicated-saving |
| `SETTPSWAP_REG_INT_REG` | 1 | call-window-preparation | 2 | no-compiler-temporary-output | normal | 2800/2800 | -53.564936% / -50.516311% | -71.907157 / -67.227955 | indicated-saving / indicated-saving |
| `LOADSETTP2_REG_INT_REG_INT` | 1 | call-window-preparation | 2 | no-compiler-temporary-output | normal | 5610/5610 | -41.551963% / -40.337528% | -89.194976 / -85.606647 | indicated-saving / indicated-saving |
| `LOADSETTPSWAP_REG_INT_REG_INT_REG` | 1 | call-window-preparation | 3 | no-compiler-temporary-output | normal | 56274/56274 | -68.700444% / -67.312536% | -2593.724091 / -2512.451195 | indicated-saving / indicated-saving |
| `SWAPSETTP_REG_REG_REG_INT` | 1 | call-window-preparation | 2 | no-compiler-temporary-output | normal | 2/2 | 1736.453527% / 1855.992025% | 1.665048 / 1.764275 | possible-slowdown / possible-slowdown |
| `SETTPSWAPSETTPSWAP_REG_INT_REG_REG_REG` | 1 | call-window-preparation | 4 | no-compiler-temporary-output | normal | 1407/1407 | -75.158954% / -75.213717% | -101.400086 / -100.596156 | indicated-saving / indicated-saving |
| `NULLN_REG_REG` | 1 | multi-null | 2 | no-compiler-temporary-output | normal | 116308/116308 | -47.626546% / -47.277424% | -2113.041680 / -2104.105443 | indicated-saving / indicated-saving |
| `NULLN_REG_REG_REG` | 1 | multi-null | 3 | no-compiler-temporary-output | normal | 31450/31450 | -63.815053% / -62.685566% | -1148.376848 / -1131.575583 | indicated-saving / indicated-saving |
| `NULLN_REG_REG_REG_REG` | 1 | multi-null | 4 | no-compiler-temporary-output | normal | 239426/239426 | -71.926033% / -71.409681% | -13138.228228 / -13084.661908 | indicated-saving / indicated-saving |
| `SWAPCALL_REG_FUNC_REG_REG_REG` | 2 | call-window-through-call | 2 | no-compiler-temporary-output | normal | 802/802 | -45.627377% / -47.917885% | -72.786050 / -76.304972 | exact-cell-required / exact-cell-required |
| `SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG` | 2 | call-window-through-call | 3 | no-compiler-temporary-output | normal | 10100/10100 | 0.693905% / -2.624869% | 15.280085 / -57.684761 | exact-cell-required / exact-cell-required |
| `SETTPCALL_REG_FUNC_REG_REG_INT` | 2 | call-window-through-call | 2 | no-compiler-temporary-output | normal | 56968/56968 | 15.026248% / 6.676333% | 1619.561606 / 719.175785 | exact-cell-required / exact-cell-required |
| `UNLINKN_REG_REG` | 2 | unlink-cleanup | 2 | no-compiler-temporary-output | normal | 44136/44136 | -46.267387% / -47.212914% | -768.556467 / -791.992397 | indicated-saving / indicated-saving |
| `ISETUNLINK_REG_REG` | 2 | unlink-cleanup | 2 | no-compiler-temporary-output | normal | 9800/9800 | -43.254944% / -44.291237% | -160.317439 / -164.808795 | indicated-saving / indicated-saving |
| `IGETUNLINK_REG_REG` | 2 | unlink-cleanup | 2 | no-compiler-temporary-output | normal | 40331/40331 | -47.418587% / -48.728026% | -723.280134 / -746.198396 | indicated-saving / indicated-saving |
| `UNLINKBR_REG_ID` | 2 | unlink-cleanup | 2 | no-compiler-temporary-output | normal | 18200/18200 | -47.851254% / -47.871942% | -325.506485 / -328.337938 | indicated-saving / indicated-saving |
| `SETLINKATTR1_REG_REG_INT_REG` | 2 | attribute-index-link | 2 | no-compiler-temporary-output | normal | 19415/19415 | -49.718259% / -49.321391% | -388.131605 / -379.646395 | indicated-saving / indicated-saving |
| `MINLINKATTR1_REG_REG_INT` | 2 | attribute-index-link | 2 | no-compiler-temporary-output | normal | 28002/28002 | -43.514184% / -46.883390% | -491.470562 / -532.267663 | indicated-saving / indicated-saving |
| `MINLINKATTR1_REG_REG_REG_INT` | 2 | attribute-index-link | 2 | no-compiler-temporary-output | normal | 195365/195365 | -42.850447% / -44.667751% | -3440.212225 / -3629.836347 | indicated-saving / indicated-saving |
| `ITOF_REG_REG` | 2 | typed-conversion-copy | 2 | no-compiler-temporary-output | normal | 1/1 | 153.998894% / 409.850584% | 0.067486 / 0.171582 | possible-slowdown / possible-slowdown |
| `ITOSCONCAT_REG_STRING_REG` | 2 | string-conversion-concat | 2 | temporary-register-passed | clear-advantage-required | 12600/12600 | -18.882102% / -17.462307% | -194.724855 / -178.661636 | indicated-saving / indicated-saving |
| `SCONCATITOS_REG_STRING_REG` | 2 | string-conversion-concat | 2 | no-compiler-temporary-output | clear-advantage-required | 5/5 | 107.024957% / -71.699852% | 0.976513 / -3.217894 | possible-slowdown / indicated-saving |
| `ISETATTR1_REG_INT_REG` | 2 | attribute-index-link | 3 | compiler-temporary-elided | normal | 19253/19253 | -65.768364% / -64.802587% | -722.301465 / -715.859740 | indicated-saving / indicated-saving |
| `UNLINKLINKATTR1_REG_REG_REG_INT` | 2 | attribute-index-link | 2 | no-compiler-temporary-output | normal | 15/15 | 102.445978% / 35.555648% | 0.584580 / 0.204859 | possible-slowdown / possible-slowdown |
| `MINSTOIATTR1_REG_REG_INT` | 2 | attribute-index-link | 5 | compiler-temporary-elided | normal | 1/1 | 399.849815% / 1350.653133% | 0.444754 / 1.563658 | possible-slowdown / possible-slowdown |
| `ILOADSETUNLINKN_REG_REG_INT_REG` | 2 | unlink-cleanup | 4 | trace-temporary-register-passed | clear-advantage-required | 15/15 | -40.382368% / -34.233827% | -0.459050 / -0.389093 | indicated-saving / indicated-saving |

## First timing-review candidates

These forms have at least 100 observations and a `possible-slowdown` signal in one or both VMs. They are investigation priorities, not removal verdicts.

| Form | Calls | Full delta rxvm / rxbvm | Delta share ppm rxvm / rxbvm | Temporary-register policy |
| --- | ---: | ---: | ---: | --- |

## Zero-use forms in this profile

`NUMCTX_INT_INT_INT_INT_INT`, `SWAPSETTPSWAP_REG_REG_REG_INT_REG`, `SETTPSWAPSETTP_REG_INT_REG_REG`, `ILOADCOPY_REG_REG_INT`, `ILOADN_REG_INT_REG_INT`, `ILOADN_REG_REG_INT`

`ILOADSETUNLINK_REG_INT`, `ISETUNLINKN_REG_REG_REG`, `ILOADSETUNLINKN_REG_INT_REG`, `SETLINKATTR1_REG_REG_INT_INT`, `SETLINKATTR1_REG_REG_INT_REG_INT`, `STOI_REG_REG`

`FDIVSUB_REG_REG_REG_FLOAT`, `FMULTICOPY_REG_FLOAT_REG_REG`, `FSUBILOAD_REG_REG_FLOAT_REG_INT`, `ILOADGETATTRS_REG_INT_REG_REG_INT`, `IGETATTR1_REG_REG_INT`, `MINIGETATTR1_REG_REG_INT`

`ISETATTR1_REG_INT_INT`, `RELINKATTR1_REG_REG_INT`, `UNLINKRELINKATTR1_REG_REG_REG_INT`, `LINKSETATTRS_REG_REG_INT_INT`, `LINKSETATTRSADD_REG_REG_INT_INT_REG_REG_INT`, `LINKSETATTRSLINKADD_REG_REG_INT_INT_REG_REG_INT`

`SETATTRSADD_REG_INT_REG_REG_INT`, `LINKILOAD_REG_REG_REG_REG_INT`, `SETLINKILOAD_REG_REG_INT_REG_REG_INT`, `LINKATTR1ADD_REG_REG_REG_INT`, `ISETATTR1_REG_REG_INT`, `STOIATTR1_REG_REG_INT`

`ILOADSETUNLINK_REG_REG_INT`, `LINKILOADSETUNLINK_REG_REG_REG_REG_INT`

Zero use means this canonical profile cannot decide the form. A targeted ordinary-Release family/isolation test is required before keeping or removing it on performance grounds.

## Forms with a raised review bar

| Form | Reason | Calls rxvm/rxbvm | Profile signal | Review status |
| --- | --- | ---: | --- | --- |
| `ILOADCOPY_REG_REG_INT` | producer-consumer; coherence-review; temporary-register-passed | 0/0 | inactive / inactive | coherence=pending; implementation=pending-hot-path-audit; decision=undecided |
| `FDIVSUB_REG_REG_REG_FLOAT` | producer-consumer; coherence-review; temporary-register-passed | 0/0 | inactive / inactive | coherence=pending; implementation=pending-hot-path-audit; decision=undecided |
| `FMULTICOPY_REG_FLOAT_REG_REG` | independent-bundle; clear-advantage-required; no-compiler-temporary-output | 0/0 | inactive / inactive | coherence=pending; implementation=pending-hot-path-audit; decision=undecided |
| `FSUBILOAD_REG_REG_FLOAT_REG_INT` | independent-bundle; clear-advantage-required; no-compiler-temporary-output | 0/0 | inactive / inactive | coherence=pending; implementation=pending-hot-path-audit; decision=undecided |
| `ITOSCONCAT_REG_STRING_REG` | producer-consumer; clear-advantage-required; temporary-register-passed | 12600/12600 | indicated-saving / indicated-saving | coherence=pending; implementation=pending-hot-path-audit; decision=undecided |
| `SCONCATITOS_REG_STRING_REG` | independent-bundle; clear-advantage-required; no-compiler-temporary-output | 5/5 | possible-slowdown / indicated-saving | coherence=pending; implementation=pending-hot-path-audit; decision=undecided |
| `ILOADGETATTRS_REG_INT_REG_REG_INT` | independent-bundle; clear-advantage-required; no-compiler-temporary-output | 0/0 | inactive / inactive | coherence=pending; implementation=pending-hot-path-audit; decision=undecided |
| `LINKSETATTRSADD_REG_REG_INT_INT_REG_REG_INT` | link-capacity-and-index; coherence-review; no-compiler-temporary-output | 0/0 | inactive / inactive | coherence=pending; implementation=pending-hot-path-audit; decision=undecided |
| `SETATTRSADD_REG_INT_REG_REG_INT` | capacity-and-index; coherence-review; no-compiler-temporary-output | 0/0 | inactive / inactive | coherence=pending; implementation=pending-hot-path-audit; decision=undecided |
| `LINKILOAD_REG_REG_REG_REG_INT` | independent-bundle; clear-advantage-required; no-compiler-temporary-output | 0/0 | inactive / inactive | coherence=pending; implementation=pending-hot-path-audit; decision=undecided |
| `SETLINKILOAD_REG_REG_INT_REG_REG_INT` | independent-bundle; clear-advantage-required; no-compiler-temporary-output | 0/0 | inactive / inactive | coherence=pending; implementation=pending-hot-path-audit; decision=undecided |
| `ILOADSETUNLINK_REG_REG_INT` | constant-store-and-cleanup; clear-advantage-required; trace-temporary-register-passed | 0/0 | inactive / inactive | coherence=pending; implementation=pending-hot-path-audit; decision=undecided |
| `ILOADSETUNLINKN_REG_REG_INT_REG` | constant-store-and-cleanup; clear-advantage-required; trace-temporary-register-passed | 15/15 | indicated-saving / indicated-saving | coherence=pending; implementation=pending-hot-path-audit; decision=undecided |
| `LINKILOADSETUNLINK_REG_REG_REG_REG_INT` | direct-attribute-write; clear-advantage-required; trace-temporary-register-passed | 0/0 | inactive / inactive | coherence=pending; implementation=pending-hot-path-audit; decision=undecided |

## Per-form decision rule

For each form, review in this order: (1) ordinary-Release saving and dynamic relevance, (2) whether the opcode is a coherent semantic unit, (3) whether a temporary register is passed merely to retain a component effect, and (4) whether the handler is the fastest correct implementation. Record the result in the manifest review columns and rerun this report.
