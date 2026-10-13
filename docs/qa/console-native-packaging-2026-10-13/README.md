# Console/native-provider packaging correction — 13 October 2026

The official signed Beta 3 Windows core includes rxconsole.rxplugin but omits
both rxconsole.lib and rxconsole_static.lib. The independently downloaded ZIP
has SHA-256 `bb83bda9545c7506592a1d1c71d1f1110b6da65b7431255afd64722aaebab539`,
source `ae1607b8e145174422cee7f3e73fbcc37a65226c`, dirty=0;
[inventory](beta3-windows-inventory.json) retains that inspection.

A fresh current-develop build graph reproduced the cause: stage-product reached
the dynamic console target through ui_ansi but never console_static. Directory
installation copies existing artifacts, so it silently omits an unbuilt static
archive. [Baseline closure](console-package-baseline-closure.json) and
[fixed closure](console-package-fixed-closure.json) show the producer edge.

## Corrections

The G product stage explicitly selects console_static. Existing RXPA post-build
actions then publish the canonical and compatibility archive names; no provider
C source or API changed. Extracted core ZIP smoke now builds the headless console
contract with --native, relocates the executable and runs it without its producer
directory or dynamic plugin. SDK inventory requires the dynamic console provider
and canonical archive. Normal installer/core manifests already preserve existing
files and hashes; no installer filter or signing behavior needed alteration.

The Windows GTK owner also needs concrete GTK import libraries and DLL closure.
The RXPA helper now accepts LINK_FILES alongside LINK_TARGETS, preserving the
version-1 manifest. The writer scans declared .rxplugin roots as native libraries.
A standalone fixture verifies both input forms, copied hashes, module dependency
discovery and failure when its dependency is absent. The helper's old code rejects
the new concrete-file call at CMake generation; the old writer incorrectly accepts
the missing module dependency and omits a required runtime manifest entry.

These are two distinct packaging causes and will be kept in separate reviewable
commits, published together after this combined qualification. The ordinary
development-publication policy applies; no new architecture/language decision,
release tag, signing operation or broad sanitizer gate is part of this repair.

## Qualification

Base: synchronized develop `c51c65bea1c418b71b36b0bf67014338fd1370b3`, on the
existing hotfix checkout. The primary dirty hotfix2 checkout was preserved.
Before publication the clean upstream base advanced to `54b610b95` with only
optional source-only Forth examples and their empty CMake subdirectory. That
fast-forward was retained; all qualified core/helper/regression inputs remain
byte-identical, so the completed functional evidence is reused.
[Input hashes](qualified-inputs.json) identify the exact changed build/helper/
test inputs. The fresh Debug Ninja tree disabled BUILD_TESTING, parser mode,
GTK and llama, preventing broad QA artifacts from rescuing missing archives.

| Check | Result | Evidence |
| --- | --- | --- |
| Fresh C1/toolchain, product, optional example and SDK provider build | Passed; no compiler warnings/errors | [raw log identities](raw-log-identities.json) |
| Actual extracted core ZIP, both compiler modes and available VMs, relocated native hello/Classic stream/console | 26 commands passed, 26.774 s | [summary](core-summary.json), [console build](core-console-native-build.log), [relocated console](core-relocated-console-native.log) |
| Installed/extracted SDK inventory, relocated CMake exports and native consumer | Passed | [archive](sdk-archive.json) |
| Missing core console archives | Native build fails at required archive after bytecode link | [log](archive-control-missing.log), [results](archive-control-results.json) |
| Restore only matching archives; relocate native console executable | Exit 0, contract passed | [log](archive-control-restored.log), [results](archive-control-results.json) |
| Remove SDK canonical console archive | Inventory rejects exact missing archive | [results](archive-control-results.json) |
| Dynamic console opt/noopt compiler+assembler pipelines | Both passed | [commands](dynamic-console-commands.json) |
| Old helper concrete linker-file interface | Expected generation failure | [log](console-helper-baseline-config.log) |
| Old writer module runtime closure | Both regression controls fail for the reproduced cause | [log](console-writer-baseline-test.log) |
| Repaired standalone helper controls, normal Debug | 2/2, 0.14 s | [log](console-helper-debug-test.log) |
| Same controls with maintained Apple ASan flags | 2/2, 0.20 s | [runner log](console-helper-asan-test-runner.log) |
| Split-package regressions | 13 passed | [log](console-package-split-tests.log) |
| Release-pipeline/manifests/signing regressions | 15 passed | [log](console-package-pipeline-tests.log) |
| Mocked Windows snapshot signer regression | 5 passed | [log](console-package-signing-tests.log) |
| Existing Windows signing route, read-only dry run | Matched source/manager cohort identified | [log](console-package-signer-dry-run.log) |

The first local staging attempt selected only stage-product and therefore did
not create the public rxvm alias supplied by C1. A raw cmake-install tree also
lacked the release documents expected by --payload. Final proof used the
documented C1/product/optional selections and the core packager's normal staging
path, followed by the ordinary SDK packager. Those setup corrections do not
change product code. Final extracted core/SDK file hashes were reverified after
the negative controls restored their archives.

The ASan fixture libraries carry the maintained sanitizer flags but are scanned
as binary data, not executed as provider callbacks. This is scheduling/packaging
proof, not a full sanitizer or native Windows claim. Physical Windows console,
GTK controls, GTK runtime relocation and recipient EXEs remain with the Windows
task. No broad compiler rerun is justified by these build-only changes.

Complete local artifacts and unfiltered logs are retained under
`/Users/adrian/Documents/Franz-UI-Mac-Investigation-20261012/evidence/console-package-20261013/`.
The Mac ZIPs qualify the recorded base plus input hashes; they are not signed
Windows recipient distributions or a newly cut release.

## Publication and Windows continuation

The authoritative [plan](../../planning/release-1/core-console-native-packaging-2026-10-13.md)
and [Windows handoff](WINDOWS-HANDOFF.md) accompany the first substantive push.
Expected final state: both qualified commits are published to hotfix/develop,
local develop is synchronized, and normal automatic Build/CodeQL checks pass.
Actual push/ref verification is reported in the conversation without a status-only
CI follow-up commit. Native Windows app proof must precede interim-release or
signed distribution publication. The existing signer requires same-commit core,
llama plugin and an unexpired installer-manager artifact from a successful Build;
the moving snapshot's selected commit must match the app qualification cohort.
