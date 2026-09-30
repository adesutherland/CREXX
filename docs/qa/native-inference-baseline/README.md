# STEP-04 baseline and complete normal Debug regression — 15 September 2026

Historical capture: normal Debug preparation failed at this baseline; its
full-suite result was **not established**. The later
[QA01/QA02 repair and C factory cleanup](../native-inference-qa01/README.md)
now supplies complete ordinary local coverage through a full run plus affected
rechecks. The original failure and logs below are preserved unchanged. The requested baseline captures this known qualification gap. Adrian requested the baseline
commit and complete regression status after the RXPA changes.

The earlier full 2,314-test Debug run belongs to STEP-03 and predates the latest
RXPA C object/compiler/executor changes. The current 87 focused Debug tests and
70 typed consumer runs are retained separately; they are not a full-suite pass.
`source-before.json` verifies the working implementation is identical to the
final typed acceptance capture and records the starting HEAD and all relevant
source hashes.

The selected ordinary full command is `ctest --test-dir cmake-build-debug
--parallel 30 --output-on-failure -LE performance-measurement`, after
`cmake --build cmake-build-debug --target qa-prep --parallel 6`.
Performance measurements remain excluded as in the preceding full run.
Sanitizer builds/tests remain held for STEP-06; SAN-009 is still open and
release-blocking. Windows/Linux/CUDA/Vulkan and exact-head hosted publication
qualification are not supplied by a local Debug result.


## NI-S4-QA01 — existing HTTP consumers fail during preparation

`qa-prep` exits 2 before CTest can run the full selection. Five started generated
HTTP test targets fail compilation: all four `ts_http_server` VM/optimization
variants and `ts_http_server_failures_rxbvm_noopt`. The diagnostics are
`RETURNS_VOID` / `RETVAL_MISSING` for `client.request(...)` or `client.get(...)`.
The latter reproduces independently with current Debug `rxc --no-exe-import -n
-x -i cmake-build-debug/bin`; this is not just Ninja's return code or a timeout.
See `debug-qa-prep.log` and `http-debug-reproducer.log`.

This is an ordinary compiler/import qualification failure, not a sanitizer
finding. It does not yet establish which edit caused it. Preserve the original
HTTP tests and result assertions. Next action: isolate compiler versus generated
library inputs and retain a minimal regression before any repair, then complete
QA preparation and the full ordinary Debug CTest selection. No test has been
skipped, weakened or marked expected-failure to conceal this result.

The checkpoint is intentionally not labelled full-regression-qualified or
release-ready. It retains the prior focused/typed successes and this failed broad
attempt together. The independent approved S3-D01 repair is committed separately
from the remaining RXPA/llama implementation baseline. No push or installation
into the user prefix is authorized or performed.

Independent native-worker repair commit: `82ca3216a55f366010a21333b3aaa3884cd82f48`. The remaining baseline
is the enclosing implementation/evidence commit; no behavioral production change was made
during this follow-up. Raw diagnostic logs preserve their original whitespace.

CTest inventory contains **2347 ordinary tests** and
**186 excluded performance measurements**; 0 selected tests are disabled.
`test-selection.json` retains every name. The full CTest execution did not start
because QA preparation failed. `source-after.json` verifies no behavioral production/test change during this
qualification attempt and records two trailing-whitespace cleanups made at
commit preparation. Those two raw hashes differ from the typed capture; their
non-whitespace content is identical.
