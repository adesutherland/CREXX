# Windows compiler and RexxCPS platform comparison

This bundle is the bounded follow-up to the supported MinGW GCC Windows
scorecard. It compares Release `-O3 -DNDEBUG` GCC 15.2 and Clang 22.1.8 VM
products in one Windows session, then reviews retained RexxCPS evidence from
macOS, Linux and Windows. No profiling or production tuning was performed.

## Evidence map

| Location | Content |
| --- | --- |
| `SCORECARD.md` | Findings, qualifications and future-tuning handoff |
| `compiler-ratios.csv` | Same-session Clang/GCC ratios; higher is better |
| `cross-platform-rexxcps.csv` | Retained RexxCPS medians and runtime ratios |
| `products.csv` | VM executable hashes, file sizes and PE `.text` sizes |
| `manifests/` | Formal compiler, append and exact-class matrices |
| `timing/` | Initial 2-warmup/10-recorded compiler comparison |
| `timing-noise-append/` | Policy append for five initially flagged cells |
| `timing-final-fixed/` | Authoritative direct two-file summary |
| `netrexx-exact-class*/` | Exact macOS/Linux Java 8 class Windows control |
| `logs/` | Configure, build and capture logs |
| `provenance/` | Host, toolchain, commands and source/product identities |
| `pilots/` | One-run qualification and summary-tool diagnostic evidence |

The `timing-final/` and `netrexx-exact-class-final/` directories are independent
one-header mechanical merge checks. Their summaries are byte-identical to the
authoritative direct multi-file summaries. The original multi-file summary
attempt exposed a Windows heap failure caused by retaining both temporary input
arrays and the merged Level B array. The tool now streams input rows directly
into the merged collection; self-test and both formal two-file summaries pass.

All formal samples remain retained. No outlier was removed. Five compiler
comparison cells and the exact-class NetRexx control still exceed the runner's
min/max span threshold after the required append; medians and MAD are disclosed
in the scorecard and raw samples remain authoritative.
