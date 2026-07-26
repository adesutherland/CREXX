# Verdict

Status: **R2a accepted; closeout complete**

R2a passes its first ordinary Release gate. The selected work-100 List cell is
clearly favorable in both VM modes, all correctness and exact-input
compatibility checks pass, canonical RXBIN/public RXAS remain unchanged, and
artifact growth is below the programme escalation threshold.

Adrian accepted R2a on 2026-07-26. The required completion path is now green:

1. the full Debug build and broad Debug CTest pass 1,922/1,922;
2. the ordinary profiling-off Release build and broad Release CTest pass
   1,922/1,922;
3. R2b stays deferred because no evidence isolates canonical `copy_value` as
   the residual cost; and
4. R1a stays separate and was not begun during R2a closeout.

The approved closeout path did not require sanitizer, install/package,
cross-platform, expanded-portfolio or repeated-baseline work, so none was
added. No R2b PoC, R1a implementation or push was performed.
