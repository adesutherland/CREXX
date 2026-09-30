# Beta 3 draft PR hosted fixture repair receipt

28 September 2026. Draft [PR #709](https://github.com/adesutherland/CREXX/pull/709)
published `30d72305459ca230befffcdc5208cfce83982a86`; `develop` remained
`143921e11e4d573909fcc4def28da5dceadba9d9`. This records two fixture
causes found by the ordinary Build workflow on that exact published input.
The accepted local AC-07 composite normal correctness proof remains the
[combined normal QA receipt](combined-normal-qa-2026-09-28.md); the later
fixture-only repairs do not change production code. Hosted retry of the
repaired branch, Windows checkout, and develop integration remain open.

## Windows checkout: reserved fixture basename

MSVC and MinGW failed at Git checkout before build/test with
`invalid path 'compiler/tests/fixtures/source_root_namespace_order/second/aux.crexx'`.
`AUX` is a reserved Windows device basename even with `.crexx`. Retained
runner logs: MSVC
`/var/folders/nr/7ckzqpl91kz80mcy3316h1tr0000gn/T/crexx-beta3-windows.vJmD79H09m`
(SHA-256 `09172403c4b6ae4ae039237c84ab34dc1b53550e2e60d674e1dd124c52ca43b6`),
MinGW
`/var/folders/nr/7ckzqpl91kz80mcy3316h1tr0000gn/T/crexx-beta3-mingw.m6uEIkXDvr`
(SHA-256 `f07179a5ab9c8e842fcedb76b8572f98cadcea7119d6b611462e6a3889d5722b`).

Commit `369cbeb7b587a764f48327d16a7a1329a6b02961` renames that fixture
only to `second_extra.crexx`: Git reports a 100% rename, namespace and file
contents are unchanged, and the first-root/multiple-provider regression still
selects the same providers. A scan of 49 added, renamed or untracked paths
relative to `143921e11` found no Windows reserved device basename.

| Focused command | Result and retained log |
| --- | --- |
| `ctest --test-dir cmake-build-debug -R '^source_root_namespace_order$' --parallel 1 --output-on-failure` | PASS 1/1; `/tmp/beta3-source-root-winname-debug.IAoy89`, SHA-256 `8631a2b552cd51c051973193e5a2f3b080d915e2d83af8dde652a97b7b3bbef7`. |
| `tools/asan-run.sh --phase ctest --regex '^source_root_namespace_order$' --test-jobs 1 --leaks off --no-live-tail` | Apple ASan PASS 1/1; `cmake-build-debugasan/asan-logs/20260928-153801-ctest/ctest.log`, SHA-256 `62b61be2707be72fd42780af0eae4c70bf002aae611374a2b0174cc6bdd9fde5`. Apple LSan is unsupported. |

## Parser-mode smoke: required library fixture

Linux x64, macOS ARM64 and macOS Intel each completed 175/176 smoke tests on
the published head. The sole failed test was `source_semantics`, with
`EXIT_MODULE_LOAD_ERROR: Failed to load required 'library': no VM load detail`.
The parser bridge correctly requires the library before the test sets
`disable_exits`; CTest ran the binary from `compiler/tests`, outside the build
`bin` directory containing `library.rxbin` and `rxcexits.rxbin`. The test had
declared only `test_source_semantics` as its fixture preparation target.
This is a test working-directory/dependency failure; mandatory exit loading and
the test's source-semantic assertions remain intact.

Hosted logs: macOS ARM64
`/var/folders/nr/7ckzqpl91kz80mcy3316h1tr0000gn/T/crexx-beta3-macos-arm64.2jSRBuKzaD`
(SHA-256 `5ec517bd586621f63ac8d0d3e2638795fb2782827d4027e9d48a06df0ade26d6`),
Linux x64
`/var/folders/nr/7ckzqpl91kz80mcy3316h1tr0000gn/T/crexx-beta3-linux.ktvtuFyBaa`
(SHA-256 `cedef3c7b66b3c511b593c67f71683e2da6ee770bb28efb180a5f454ff759d8a`),
macOS Intel
`/var/folders/nr/7ckzqpl91kz80mcy3316h1tr0000gn/T/crexx-beta3-macos-intel.wo02FcCGqC`
(SHA-256 `a1f06494571c244b08a1d053218deec18dca5bc91b8b181e2ecf054b7b41f34c`).
The Linux job independently passed the real RXBIN ILP32 check:
`PASS RXBIN cross-width reader/writer (gcc -m32)` at line 1459 of its log,
job `108975377026` in run `36436367998`. Linux optimizer parity also passed
job `108975376742` on the same published head. These results remain valid
across the later fixture/documentation-only changes.

The earlier local 2,283/2,286 combined run used `ENABLE_PARSER_MODE=OFF` and
**did not include** this parser-only smoke test. Both original local Debug and
ASan caches have that option OFF. A separate focused parser-enabled Debug
tree was configured with Ninja, `-DCMAKE_BUILD_TYPE=Debug`,
`-DENABLE_PARSER_MODE=ON`, `-DENABLE_LLAMA=OFF` and
`-DDSLSH_LOCAL_DIR=/Users/adrian/CLionProjects/DSL-Syntax-Highlighter`.
That read-only dependency was at
`383e5daab0ffcf6ec83e02db7a01a27709dabb2c` with only `AGENTS.md`
modified; no source/build input there was edited for this repair.

| Focused command or observation | Result and retained log |
| --- | --- |
| `cmake -S . -B cmake-build-beta3-parser-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DENABLE_PARSER_MODE=ON -DDSLSH_LOCAL_DIR=/Users/adrian/CLionProjects/DSL-Syntax-Highlighter -DENABLE_LLAMA=OFF` | PASS; `/tmp/beta3-parser-config.jCw4r4`, SHA-256 `c71af9e473aa3ddada3b264c98454c4495855ba4fcb3eb761f7eee2295d0633e`. |
| `cmake --build cmake-build-beta3-parser-debug --target test_source_semantics library compiler_exit_bin --parallel 8` | PASS; `/tmp/beta3-parser-build.XKIprt`, SHA-256 `2abaea163820b428efaa57876c519dee12b0bb032e670ef63cd63d1dff075b60`. |
| Unchanged `ctest --test-dir cmake-build-beta3-parser-debug -R '^source_semantics$' --parallel 1 --output-on-failure` | Reproduced required-library failure, 0/1; `/tmp/beta3-parser-before-ctest.u6Rn3g`, SHA-256 `4d410ee1672bc7003d240dbf56498b75850ebaf605cc72f44b2ea9484f29984e`. The identical test executable returned success when run from build `bin`. |
| After CMake fixture correction, same three-target build and focused CTest | Build PASS, `/tmp/beta3-parser-fix-build.rLVkFs`, SHA-256 `b8efaad3b4184508015ffb9262e06e816623dc9b70625125db619fe08303b67b`; test PASS 1/1, `/tmp/beta3-parser-fix-ctest.Axmyxc`, SHA-256 `93d09f39f74b2080a60bb3aefa91fb89167be51e9666ac6ebda9dad26494ee51`. |

The matching parser-enabled Apple ASan tree used the same local DSL source,
`ENABLE_PARSER_MODE=ON`, `ENABLE_LLAMA=OFF`, Debug and
`-fsanitize=address -fno-omit-frame-pointer` C/C++ compile flags with
`-fsanitize=address` executable/shared linker flags. Its configure command
was:

```sh
cmake -S . -B cmake-build-beta3-parser-asan -G Ninja -DCMAKE_BUILD_TYPE=Debug -DENABLE_PARSER_MODE=ON -DDSLSH_LOCAL_DIR=/Users/adrian/CLionProjects/DSL-Syntax-Highlighter -DENABLE_LLAMA=OFF '-DCMAKE_C_FLAGS=-fsanitize=address -fno-omit-frame-pointer' '-DCMAKE_CXX_FLAGS=-fsanitize=address -fno-omit-frame-pointer' '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address' '-DCMAKE_SHARED_LINKER_FLAGS=-fsanitize=address'
```

Configure PASS: `/tmp/beta3-parser-asan-config.lntsDt`, SHA-256
`3195a354a3b52948c59bc91e6b491e3bbb03dd8ba9074faa859bc5b02cf5508c`.
`tools/asan-run.sh --build-dir cmake-build-beta3-parser-asan --phase build
--build-target test_source_semantics --build-target library --build-target
compiler_exit_bin --build-jobs 8 --build-leaks off --leaks off --no-live-tail`
passed; build log
`cmake-build-beta3-parser-asan/asan-logs/20260928-154615-build/build.log`,
SHA-256 `f9388bc741fa9ed5f9d7e48719c6aaaa1f4026ae3293028d6ba2131d6ef5c419`.
`tools/asan-run.sh --build-dir cmake-build-beta3-parser-asan --phase ctest
--regex '^source_semantics$' --test-jobs 1 --leaks off --no-live-tail` passed
1/1 without sanitizer diagnostics; log
`cmake-build-beta3-parser-asan/asan-logs/20260928-155414-ctest/ctest.log`,
SHA-256 `c8c6ff5ff4238940b41193bcf48350e27749416c51bf14cc4cb31b595c66f524`.
Apple LSan is unsupported; the final Linux ASan/LSan and macOS ASan baseline
matrix remains a separate open AC-08 gate.

Commit `1366df3c18645d90612d9e891111e9ec8afca88d` (tree
`a91025e4d8d84d8f4d24292f24259755b5bba478`) sets `source_semantics`
working directory to
`${CMAKE_BINARY_DIR}/bin` and declares `test_source_semantics`, `library` and
`compiler_exit_bin` as prep targets. It changes no production source or test
assertions. No broad local suite was repeated. Hosted retry on the repaired
head is still required before claiming the ordinary Build checks pass.

## Adjacent parser-mode sandbox audit

A bounded read-only audit of the three direct parser-mode C tests found one
more instance of the same setup omission. `highlight_cache` enters a first
temporary sandbox holding source files only; the controller searches the
document directory, sandbox cwd and test executable directory
(`compiler/tests`), none of which contains the required modules. Its later
feature sandbox already stages both `library.rxbin` and `rxcexits.rxbin`.
`highlight_editor_diagnostics` has no isolated sandbox and passed unchanged
in the fresh parser-enabled Debug tree. Generated syntax-highlight scripts
run from a fixed `compiler/tests` cwd; the audit found no other parser-mode
fixture that changes into an isolated sandbox.

Commit `00d42cb3ac75ac4306e4b1a44028cf3efb4e4daf` (tree
`c8287f8d6fa7000c1503f26f21e0909df1ede0b7`) is the test-only repair. It
copies the already-built `library.rxbin` and
`rxcexits.rxbin` into the first cache sandbox before parsing, removes them on
success/failure cleanup, and declares `test_highlight_cache`, `library` and
`compiler_exit_bin` as prep targets. It retains every cache generation,
invalidation, import and exit assertion. No controller or product code changed.

| Focused command or observation | Result and retained log |
| --- | --- |
| `cmake --build cmake-build-beta3-parser-debug --target test_highlight_cache test_highlight_editor_diagnostics --parallel 8` | PASS; `/tmp/beta3-parser-adjacent-build.log`, SHA-256 `64d39270e9811bed318a5fc1ba6ecff0c183b0bdfd2a74ee232881352b0f2bf5`. |
| Before the cache fixture repair, `ctest --test-dir cmake-build-beta3-parser-debug -R '^(highlight_cache|highlight_editor_diagnostics)$' --parallel 1 --output-on-failure` | Cache 0/1 with the same `EXIT_MODULE_LOAD_ERROR`; editor diagnostics 1/1 PASS; `/tmp/beta3-parser-adjacent-ctest.log`, SHA-256 `816654ddf57439a141a009624ab2f3fd4e217576196542038f96d0c06a4fdeb5`. |
| After fixture repair, `cmake --build cmake-build-beta3-parser-debug --target test_highlight_cache library compiler_exit_bin --parallel 8` | PASS; `/tmp/beta3-highlight-cache-fix-build.log`, SHA-256 `a8c0c1575b485935d15633a0bd310072d7d9a21854c4dab26d7c84b1126af965`. |
| `ctest --test-dir cmake-build-beta3-parser-debug -R '^highlight_cache$' --parallel 1 --output-on-failure` | PASS 1/1; `/tmp/beta3-highlight-cache-fix-ctest.log`, SHA-256 `6bce1f2ad6004b1120b65c45df9b266d33984751cbd24ff98bf41d0cee3acb85`. |

`tools/asan-run.sh --build-dir cmake-build-beta3-parser-asan --phase build
--build-target test_highlight_cache --build-target library --build-target
compiler_exit_bin --build-jobs 8 --build-leaks off --leaks off --no-live-tail`
passed; `cmake-build-beta3-parser-asan/asan-logs/20260928-155827-build/build.log`,
SHA-256 `0be0d46086907e237e3fcfd44a13f9e366e1858fd9ad3406931ede70c61e5371`.
`tools/asan-run.sh --build-dir cmake-build-beta3-parser-asan --phase ctest
--regex '^highlight_cache$' --test-jobs 1 --leaks off --no-live-tail` passed
1/1 without sanitizer diagnostics;
`cmake-build-beta3-parser-asan/asan-logs/20260928-160057-ctest/ctest.log`,
SHA-256 `1aef0ce6459a5fd383711af38f051eb8cb1425775dcd0f3d60754c6287ca2ce4`.
The failed pre-fix run left one temporary sandbox containing only its four
source fixture files; it was inspected and removed after retaining the log.
Successful Debug/ASan runs left no cache sandbox directory.

This adjacent fixture was outside the ordinary 176-test smoke lane; it does
not change the hosted 175/176 result or the already retained 2,283 local
combined passes, which were built with parser mode OFF.
