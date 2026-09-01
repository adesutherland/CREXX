# PERF3-05 candidate ledger

| ID | Form | Correct | Decisive evidence | Disposition |
| --- | --- | --- | --- | --- |
| L0 | Exact C1abc+A1 ordinary Release | yes, 10/10 workload/VM cells | Fresh and drift binaries are byte-identical | retain production control |
| L1-A1 | Base64 codepoint/position control | yes, exact checksum/round trip | 2.691x/2.959x faster than current | retain semantic ceiling only |
| L1-A2 | Base64 codepoint/arithmetic control | yes, exact checksum/round trip | 10.691x/10.354x faster than current | retain semantic ceiling only |
| L2-LTO | Apple ThinLTO through both core object libraries and final links | yes, 240/240 timed executions | Sieve `rxvm` -4.443%; Base64 `rxbvm` -11.351%; opposite VM directions | reject default; replayable control |
| L2-PGO-0 | Release flags supplied only through `CMAKE_C_FLAGS_RELEASE` | not timed | Interpreter CMake reset means the VM cores were not instrumented | reject ineffective setup; retained negative |
| L2-PGO-M | Five-workload merged profile | yes, training 10/10 and timing 240/240 | Sieve -40.399%/-33.734%; Base64 -27.740%/-48.840%, despite Richards +16.357%/+13.450% | reject over-specialized layout |
| L2-PGO-V | Separate per-VM profiles | yes, pilot complete | Reproduces the material PGO skew rather than curing it | reject |
| L3-NF | Remove `RX_FLATTEN` from `run()` in detached PoC | yes, timing 240/240; RSS 20/20 cells; lifecycle pass | Build -18.4%, build RSS -9.8%, but `rxbvm` Sieve -3.639% and cross-cell reversals | reject runtime; retain build/layout evidence |
| L4-E | Eager compact decoded image | not opened | Current execution image is already eager/private/bound; no additional direct-load ceiling proved | defer |
| L4-L | Lazy per-procedure image | not opened | Would add first-hit/lifetime/invalidation work without a demonstrated steady-state ceiling | defer |
| L4-S | Narrow hot-operand sidecar | not opened | No remaining operand-access mechanism survives the L2/L3 panel | defer |

No rejected option is deleted: the exact flags, build logs, profiles, patch,
binaries or hashes and timing inputs required to replay each meaningful row are
retained in this evidence directory or the disposable build paths named by its
logs.

