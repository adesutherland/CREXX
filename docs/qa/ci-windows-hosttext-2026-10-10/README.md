# Windows host-text repair, 10 October 2026

Full [Deep Build QA 38086282692](https://github.com/adesutherland/CREXX/actions/runs/38086282692)
on `a69745746117ab01df47ae24d47793f397962488` passes Linux, both Macs,
books, isolated stress and the product graph/bytecode comparisons. MinGW and
MSVC each fail exactly `classic_frontend_parity` and
`classic_unicode_contract_opt`. Both Windows product builds already pass in
[Build CREXX 38086265138](https://github.com/adesutherland/CREXX/actions/runs/38086265138);
the prior SDK/AST namespace collision is repaired.

The typed broker receives an ACP-encoded `É` path but converts its arguments
as UTF-8, so it cannot launch the owning `rxc`. The Unicode contract loses
the `变量.rexx` filename to `??.rexx` in CRT argv. The permanent end-to-end
tests remain unchanged. The retained CP1252 control reproduces both byte
boundaries. A second required correction quotes the broker's wide arguments:
[`_wspawnv` joins them with spaces](https://learn.microsoft.com/en-us/cpp/c-runtime-library/spawn-wspawn-functions?view=msvc-170),
so spaced roots otherwise become multiple arguments.

Core CLI executables now embed the
[standard UTF-8 process manifest](https://learn.microsoft.com/en-us/windows/apps/design/globalizing/use-utf8-code-page).
MSVC merges it into the executable manifest; MinGW compiles the same XML into
resource 1. It keeps narrow argv and host filenames in internal UTF-8 on
Windows 10 version 1903 and later. Earlier Windows Unicode-path behavior and
embedding hosts are not qualified by this standalone-tool property. External
stream codecs, byte ordinals, Unicode source text, authored diagnostics and
metadata/inline identities retain their existing contracts.

The broker quotes each UTF-16 argument using the
[CRT parsing rules](https://learn.microsoft.com/en-us/cpp/c-language/parsing-c-command-line-arguments?view=msvc-170),
preserving empty strings, spaces/tabs, quotes, backslash runs and trailing
backslashes without a shell. `compiler_windows_argv` has nine permanent
vectors over that production helper. Debug passes 1/1 in 0.38 seconds, then
maintained Apple ASan passes the same command 1/1 in 0.40 seconds. Apple LSan
is unavailable; hosted Linux build/test leak detection remains enabled.
Both Debug compiler entry points build without warnings. All 15 existing and
extended matrix/package workflow controls pass.

The first ASan target attempt found the stale Make tree did not yet know the
new target. Reconfiguring that existing tree and repeating the exact helper
build/test succeeds; this is a preparation error, not a sanitizer finding.
All attempts are retained. No sanitizer suppression or test exclusion is added.

The original and repaired broker produce byte-identical Unix preprocessing
with the actual Debug compile arguments. The Windows manifest CMake block is
inactive on Unix. All existing full/single-platform matrix selections produce
the same rows and workload flags. The added `windows` manual selection runs
both Windows toolchains concurrently, leaving books/stress/product-graph
replays to the retained complete gate. Reuse the successful Unix Deep lanes
and currently running full Linux/macOS sanitizer jobs; the new helper's focused
Debug/ASan proof covers the added portable test. Do not rerun unchanged broad
work solely because the Windows repair has another commit ID.

`inputs-and-results.json` records source/build/test identities, original failed
log/artifact hashes and local evidence. The authoritative acceptance criteria
remain in [the pipeline plan](../../planning/release-1/ci-core-stream-packaging-2026-10-10.md).
Expected first-push delivery is synchronized `origin/develop`, green automatic
Build/CodeQL, and the Windows-only Deep replay dispatched. Actual repaired
Windows proof, full sanitizer completion and SAN-011/012/SAN-QA-018 disposition
remain open until their required evidence is verified. GitHub run records and
the ignored evidence `active-runs.json` own observed status; no routine
follow-up CI-status commit is planned.
