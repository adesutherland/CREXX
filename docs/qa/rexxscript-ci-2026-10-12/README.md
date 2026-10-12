# RexxScript CI repair — 12 October 2026

The array integration test was registered with `RXC_FLAGS -x`, which disables
the compiler exit implementing `REXXSCRIPT`. This broke all six product/parity
lanes in [Build CREXX 38231688994](https://github.com/adesutherland/CREXX/actions/runs/38231688994)
and the build in [CodeQL 38231688600](https://github.com/adesutherland/CREXX/actions/runs/38231688600)
at `testRexxScriptArrayExpose.crexx:22`. The same error is present in
[38228592869](https://github.com/adesutherland/CREXX/actions/runs/38228592869),
before the mainframe integration. The ordinary Release product build in the
failed workflow had already passed.

The local target build reproduced that syntax error in both optimization modes;
see [original compile failure](original-compile-failure.txt). Enabling exits
made the complete array assertions pass. The repair also adds the exit RXBIN
as a file prerequisite so changes to the bundle rebuild this consumer.
The runtime API and compatibility tests continue to disable exits.

Focused checks then exposed stale direct-exit assertions for fixed scalar
indexes: the approved array support uses an insertion cursor. Those assertions
now check the cursor-based layout. The unsupported-array fixture used `values`,
a clause keyword, and therefore tested a missing EXPOSE name instead of the
array-type diagnostic; it now uses `numbers`. Finally, generated EXPOSE and
COLLECT helper variables have explicit integer declarations to avoid duplicate
implicit-binding warnings when a procedure contains several statements.
These changes preserve the existing syntax and array/runtime contract.

## Local qualification

Source baseline: `9548714f9b5c964ee5a7b2e040562cbae0f3ebcb`, plus the three
code/test/build inputs hashed in [qualification.json](qualification.json).
The hashes were checked again after all gates finished. The manifest also
records tool/bundle hashes and SHA-256 digests of the full local logs.
Permanent result summaries normalize the source directory to `<source>`.
The existing maintained Debug and Release builds used CMake/CTest 4.3.2 on
Darwin arm64; no sanitizer configuration or overnight gate was required for
this repair.

| Gate | Result | Retained output |
| --- | --- | --- |
| Release focused runtime/exit/CLI checks | 11/11, 0.81 s | [result](release-focused.txt) |
| Debug focused runtime/exit/CLI checks | 11/11, 2.50 s | [result](debug-focused.txt) |
| Release product and optional examples build | Passed | Product build precedes smoke in the same successful command |
| Release essential/smoke QA | 186/186, 102.90 s | [summary](release-product-smoke.txt) |
| Debug optimizer correctness parity | 802/802, 136.70 s | [summary](debug-optimizer-parity.txt) |

The focused builds/checks used the following commands for each build directory:

```sh
cmake --build <build> --parallel 10 \
  --target testRexxScriptArrayExpose test_rexxscript_bin test_rexxscript_direct_bin testrexxscript
ctest --test-dir <build> --parallel 10 --output-on-failure \
  -R '^(testRexxScript|test_rexxscript|rexxscript_cli_smoke)'
cmake --build cmake-build-release --target stage-product stage-optional --parallel 10
cmake --build cmake-build-release --target qa-smoke --parallel 10
cmake --build cmake-build-debug --target qa-optimizer-parity --parallel 10
```

All final build/test logs were inspected without filtering build diagnostics;
they contain no compiler warnings or errors. The array sample verifies changed
text, automatic growth from three to four elements and the calculated fourth
value, in both compiler/assembler modes. The optimized runtime path includes
linking, so the focused checks exercise `rxc`, `rxas`, `rxlink`, and `rxvm`.
No test was removed or excluded.

## Publication contract

The explicitly requested `hotfix2` repair is ready for promotion to remote
`develop` with this receipt, developer-guide update and authoritative
[CI repair plan](../../planning/rexxscript-one-dimensional-arrays-plan-2026-10-12.md).
Expected final state: the qualified repair is published to `develop`; the
normal automatic Build CREXX and CodeQL workflows are expected to pass.
Successful push and remote-ref verification are reported live in the chat.
Hosted success is not claimed by this pre-publication receipt, and no
CI-status-only follow-up commit or extra overnight dispatch is planned.
