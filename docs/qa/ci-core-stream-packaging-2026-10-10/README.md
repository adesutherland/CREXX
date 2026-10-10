# GitHub pipeline repair, 10 October 2026

The [authoritative plan](../../planning/release-1/ci-core-stream-packaging-2026-10-10.md)
owns this repair. All three failed workflows checked `develop` revision
`10e8184e39ad28e62178055473249925f9db82a7`; the scheduled runs' top-level SHA
`77ba820c3` belongs to the default-branch workflow, not the tested checkout.

## Diagnosis and changes

| Failed gate | Retained cause and repair |
| --- | --- |
| [Build CREXX 37987446535](https://github.com/adesutherland/CREXX/actions/runs/37987446535) | All four optional-plugin native consumers lack the core's `rxcstream.a`/`.lib`. `stage-c-rexx-tools` now selects `cstream_static`; extracted core ZIP smoke exercises a relocated native Classic encoded-stream consumer. |
| [Deep Build QA 38018313879](https://github.com/adesutherland/CREXX/actions/runs/38018313879) | Five native ADDRESS smokes hit the same archive omission. Every platform lacks the lifecycle executable because directory QA finalization preceded its registration. Both Windows Unicode drivers decode UTF-8 source as CP1252. Finalization now follows all tests, and fixture reads explicitly select UTF-8. |
| [Sanitizer QA 38022249457](https://github.com/adesutherland/CREXX/actions/runs/38022249457) | Four Linux integer-limit modes panic in ARG after a validated canonical integer is rounded through binary float. Fifty-four consumers across 24 Classic sources now use checked `as .int` conversion, including conversion before stream position subtraction. SAN-QA-018 owns supported full sanitizer closure. |

The complete failed Linux artifact contains no AddressSanitizer or
LeakSanitizer diagnostic. Its four functional conversion failures are retained
in [linux-integer-boundary-failure.log](linux-integer-boundary-failure.log).
Remaining cases after the stopped gate are unexecuted, not successful.

## Focused package and harness proof

[qualified-inputs.json](qualified-inputs.json) retains the first build/package
phase, the downloaded ZIP's hash and missing-file inventory, generated stage
selection, output-rebuild command, and local log hashes. The product build was
forced to rebuild only the stream target's three generated archive outputs
through `stage-product`; all three delivered archive hashes agree.

The strengthened permanent core smoke fails against the exact downloaded ZIP
at `classic-native-build`, after all previous core smoke commands pass. With
only the canonical/compatibility stream archives restored, a newly created and
extracted ZIP passes every command; all original manifest files remain
byte-identical. [failed-core-smoke.json](failed-core-smoke.json) and
[repaired-core-smoke.json](repaired-core-smoke.json) retain those controls
(7.99 and 8.48 seconds).

[classic-integer-qualified-inputs.json](classic-integer-qualified-inputs.json)
anchors the final numeric/test/build inputs. Debug and Release products pass.
Both generated ARG procedures use `stoi`, replacing `stof`/`fadd`/`ftoi`.
Complete RexxDoc blocks and tags remain byte-identical in all 24 modified
Classic sources. Both product graphs select the static stream archive and
both comprehensive-prep graphs select the lifecycle executable.

[debug-bif-panel.log](debug-bif-panel.log) retains **169/169** relevant normal
Debug checks (294.94 seconds), after building their exact selected producer
targets. This covers the direct BIF library, shared B/G aliases, Classic
validation/reference audits, all four integer-boundary modes, lifecycle and
native ADDRESS smoke. No unrelated broad correctness sweep was repeated.
Six final stream position casts were added after that panel. Its unchanged BIF
results remain valid; the input receipt identifies the exact predecessor/final
stream hashes. The final six-test panel passes in normal Debug (19.05 seconds)
and maintained Apple ASan (30.65 seconds), retained in
[debug-focused.log](debug-focused.log) and [asan-focused.log](asan-focused.log).
Both instrumented build phases pass. Apple LeakSanitizer is unsupported, so
these Apple build/test phases explicitly use leak checks off.

A CP1252-default `Path.read_text` control reproduces the original Windows
`UnicodeDecodeError`. The final driver passes the complete 156-command Unicode
contract under that same default. The final replay additionally checks a
zero-length read and omitted-text output at exact INT64_MAX positions
(31.68 seconds); fixture reads explicitly use UTF-8. Source generation and VM
output remain UTF-8. The three new position assertions add no allocation or
new scenario/program to the measured serialized aggregate.

[final-core-smoke.json](final-core-smoke.json) covers another newly extracted
ZIP containing the final repaired Classic runtime. Both optimization modes,
public/alternate VMs, original relocated Level B native consumer and relocated
Classic native consumer pass. The Classic consumer decodes `€é` and writes
exact Windows-1252 bytes `80 E9`, checked by the Python harness.

[package-regressions.log](package-regressions.log) retains 47 split-package,
release-workflow and Windows-package checks: 46 pass and the native Windows
Authenticode control is skipped on macOS. Changed Python syntax and diff
whitespace checks pass. No compiler/VM core or language-design change is made.

## Delivery and platform assurance

Expected delivery is the compatible build/harness and numeric repairs published
together to `origin/develop`, with this receipt and plan in the first push.
Normal automatic Release product/smoke, optimizer-parity and CodeQL checks are
expected to pass. Their actual result belongs to GitHub run records; no
status-only commit is planned after success.

SAN-QA-018 remains under qualification until the permanent focused normal/
maintained-sanitizer checks and complete supported Linux ASan/LSan and Apple
ASan gates pass. Adrian explicitly authorized bounded publication on
2026-10-10 and assigned remaining SAN-QA-018 platform proof to **Codex at the
next Release 1 release-QA gate**. No exclusion, suppression,
supported-platform leak-off wrapper or sanitizer-clean/release-ready claim is
part of this repair. Deep QA can exercise the restored producers and Windows
fixture handling in its normal overnight run.
