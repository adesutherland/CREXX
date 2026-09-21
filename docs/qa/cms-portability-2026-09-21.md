# CMS portability integration review — 21 September 2026

This is host qualification for an opt-in cREXX integration candidate, not a
CMS application or beta-release qualification. The authoritative scope and
acceptance criteria are in
[the integration plan](../planning/cms-portability-qa-20260921.md).

## Source and integration decision

Fetched `origin/develop` remains
`5de72ae57df4e9fa4939d5a3f9412aa3b0569108`, also the candidate's original base.
All eleven commits through `e68681cd1dd0203088f8a4ad73d9d9a80700084d` are
unapplied upstream. Preserve that history on
`temp/cms-portability-qa-20260921`; no rebase or duplicate cherry-picks were
necessary. The repair/test commit is `edc4f3378e3bb4c867f7943d2c3ca6d52d855b37`.
The original `temp/cms-release-poc` worktree and its evidence are
unchanged. The unrelated dirty main checkout was not used for implementation.

Mainframe Lab started at `6b1795d71884e0fa22544fdd66381928f180294c`.
The later fetch found HEAD/main at `7d737174eb8300085273df58c0ce17a057485c32`.
The intervening VM/CE uplink/production-glue integration changes no maintained
GCC/checker source, requested CMS document or cREXX recovery patch; its combined
integration report and updated instructions/profile guidance were inspected.
The eleven ordered recovery patches passed their retained SHA-256 checks.
Existing guest and compiler-profile evidence remains dated evidence; this
review does not extend those results.

The primary target route is maintained GCC 16.2, cross-built and run on macOS.
Clean, justified legacy compatibility is welcome; uncertain changes require
owner approval. The intended C baseline is C99, while retained target recipes
use GNU99. Neither the host build nor those recipes establish full strict-C99
or C90 qualification.

## Eleven-topic review

| Topic / original commit | Disposition |
| --- | --- |
| Explicit CMS ELF platform / `e05066b01` | Explicit profile takes precedence over inherited Linux aliases. Standard I/O lookup and unavailable service contracts stay in the platform layer; legacy `__CMS__` remains distinct. |
| C99 utilities / `b9c8e901f` | Portable string copies and `__func__` remove unnecessary extension dependence without changing desktop behavior. No claim of whole-project strict-C99 conformance. |
| Sequential source input / `5bbf84049` | Reads non-seekable streams through EOF with overflow/allocation/read-error handling and two zero sentinels. Seekable input retains rewind behavior. Added empty-pipe coverage. |
| Environment/process separation / `1e4810066` | Environment lookup has its own source file; normal VM targets retain both it and process creation. The constrained leaf can omit process services without losing environment lookup. |
| Single-thread VM / `bd96ddd56` | Opt-in leaf owns a consistent configuration; worker objects still track execution/memory ownership. Nested state and signals remain explicit. NTHREADED alone does not remove OS workers. No fake pthread API or new dispatch design. |
| Bounded allocator / `78107fc44` | Portable alignment retains the allocation base for release; small pools fall back to ordinary extents above their limit. Desktop defaults and the 64-byte slab header contract remain intact. |
| Allocator settings / `3a0e5c635` | Compile-time power-of-two, header/slot and pooled-range checks; alternative size execution and invalid configuration controls pass. Settings must agree across VM translation units. |
| Embedded RXC / `e9f003cfd` | Public embedding library has no VM CLI main. The compiler source closure retains exits and Level B/C; generated inputs and RXAS archive come from the pinned normal build. Parity includes exits enabled. |
| Optional directory interface / `b7afdf985` | Runtime owns CMS snapshots/naming; cREXX reuses existing prefix/type filters. Default ENOSYS remains explicit. Independent iterators, no matches and open errors covered. |
| CMS 20 selection / `20e3de226` | Distinct CMS20 selector shares the deliberately selected ELF platform services without selecting Linux services or conflating historical and 31-bit execution qualification. |
| Text hooks and encoding CLI / `e68681cd1` | Conversion surrounds text I/O, before lexing and after emission. UTF-8 buffers and binary RXBIN remain unchanged. Review exposed ignored input-close errors; fixed with permanent failure injection. |

The repair rejects failed closes while reading main source, import headers,
imported procedures and RXAS input. It also moves the compiler's debug success
message after output/close/report checks. The original tools returned success
when injected input close callbacks failed; retained reproduction is
`input-close-original.json` in the local evidence directory.

CMS selection tests previously shared temporary names. They now use separate
working directories, and the owning CMake directory declares QA tiers and
preparation dependencies. New test-only front ends exercise the real compiler
and assembler through an injected stream provider. No provider implementation
or test encoding enters product binaries.

## Qualification

Host: macOS arm64, Apple Clang 21.0.0, CMake 4.3.2, Ninja 1.13.2.
Ordinary top-level builds keep default parser integration and disable optional
llama. The sibling parser input is
`383e5daab0ffcf6ec83e02db7a01a27709dabb2c`; its only local edit is AGENTS.md.

| Check | Result |
| --- | --- |
| Ordinary Debug and Release `rxc rxas rxlink rxvm rxbvm` builds | Pass |
| Normal Debug essential/smoke/comprehensive correctness selection | 2,350/2,350 pass, 0 failures; 993.40 seconds |
| Constrained Debug and MinSizeRel `single_vm_state`, `single_vm_embedding`, `single_vm_parity` | 3/3 each; 14 optimized/unoptimized reference comparisons, 33 caught unavailable operations, unhandled signal 12, process-worker and dynamic-loader rejection |
| `check_single_rxc`, Debug / MinSizeRel / ASan+UBSan | Pass in all three: six assembly/bytecode/execution comparisons with compiler exits enabled, plus invalid UTF-8 literal rejection |
| Allocator configurations | Four valid compile controls and six expected rejections pass; Debug/sanitizers use 4 KiB/2 KiB, MinSizeRel executes with 8 KiB/4 KiB |
| CMS platform, CMS20 platform, directory interface | Pass in normal Debug and maintained ASan |
| Text/import/output/RXAS boundary | 69 controls pass in normal Debug and maintained ASan; default UTF-8, explicit UTF8, adapter UTF8 and transformed text have identical opt/noopt products and execution |
| Constrained ASan+UBSan lifecycle, embedding and VM parity | 3/3 pass, with halt-on-UB and no sanitizer findings |

The text test includes non-seekable short reads, source-buffer growth, imported
headers and procedures, UTF-8 accents, encoding-option failures, read/conversion
failures, immediate/buffered output failures and input/output close failures.
It compares RXC assembly, RXAS bytecode, RXLink images and VM output; binary
files must never enter the hook. TEST-XOR is deliberately a mock codec, not an
IBM1047 or CMS implementation. The registered nested test is RUN_SERIAL with
a 300-second hang-protection timeout; measured Debug/ASan commands took
1.82/5.21 seconds before registration.

Sanitizer commands use `tools/asan-run.sh`. The normal maintained tree uses
ASan; the constrained leaf uses ASan+UBSan. Its prebuilt normal RXAS archive
and reference tools remain outside that leaf's instrumentation boundary.
Apple LeakSanitizer is unavailable, so leak detection is disabled there and
no leak or cross-platform sanitizer pass is claimed. No actual sanitizer
finding required a new SAN worklist entry.

## Retained evidence and remaining work

The normal correctness command, after preparing its prerequisites, is:

```sh
cmake --build "$QA/debug" --target qa-prep-comprehensive --parallel 10
ctest --test-dir "$QA/debug" --parallel 30 --output-on-failure \
  --label-regex '^(essential|smoke|comprehensive)$'
```

`QA` is the external evidence/build directory below. The source manifest
`qualified-code-inputs.json` records every tracked or newly added regular non-document
input before the final suite. Build metadata retains the original candidate's
SHA plus a dirty marker; content hashes, rather than that display label, bind
the tested repairs to the eventual commits. Documentation-only closeout does
not invalidate this evidence.

Raw commands, return codes, build/test logs, source fingerprints and binaries
are outside Git at
`/Users/adrian/CLionProjects/crexx-cms-qa-evidence-20260921/`. Failed harness
setup attempts and the original ignored-close reproducer remain beside the
accepted logs. Generated files and build directories are not part of commits.
The earlier evidence at `CREXX-cms-release-poc/artifacts/cms-release-poc/` is
preserved, including the initial missing-test-driver failure and passing retry.

No CMS guest was run for this review. The historical RXC `-X`/RXAS/RXVM
demonstration and the small 31-bit runtime/text fixtures prove different things.
Actual 31-bit application workloads, compiler-exit memory fit, native text and
imports, fresh 24/31-bit package reconstruction, release packaging and
CMS-hosted GCC remain Mainframe Lab work. Full hosted/deep/overnight matrices
were not dispatched for this ordinary integration candidate. Publication and
any release still require their applicable authority and gates.

Mainframe Lab should pin the final integration revision and regenerate recovery
patches from Git, retaining the original candidate series and evidence. Rebuild
and qualify target applications against that pin before extending guest claims.

The final input comparison matches all 33,252 recorded regular code/test/build
files to the repair commit. Three pre-existing dangling historical-evidence
symlinks, excluded from the byte manifest, are unchanged Git blobs. The local
`accepted-logs.json`, `qualification-environment.json` and
`qualified-input-check.json` bind commands, log hashes, build metadata and this
comparison. No broad test repeat is needed for the documentation commit.

A refreshed ordered recovery series, base/candidate identities and SHA-256
manifest are provided under the external evidence directory's `recovery/`.
Its patches are generated from Git and checked by reconstructing the complete
candidate tree in an isolated temporary index. Keep the original eleven-patch
Lab series until its consumer pins are deliberately updated.

## Publication follow-up: MSVC preprocessing repair

After publication was authorized, the first automatic Windows build exposed
C5101/C2143 in the two random-seeding handlers: preprocessor directives inside
function-like macro arguments are nonportable. The clock condition now selects
a small statement macro outside `RXVM_HANDLER` arguments, with the same
time-based desktop seed and constrained `NOT_IMPLEMENTED` signal. The helper
is undefined after the two handlers. Dispatch architecture is unchanged.

Clang rejects the original form with `-Werror=embedded-directive` and accepts
both repaired clock configurations. Rebuilt desktop threaded/switch VMs pass
the random regression; optimized/unoptimized CTests pass 2/2. Constrained Debug
and ASan/UBSan state/embedding/parity pass 3/3 each, and compiler-exit parity
passes all six comparisons in both configurations. No sanitizer finding arose.
The retained broad suite predates this bounded repair; these focused checks
and the final automatic hosted build qualify the changed preprocessing. Hosted
results are pending at commit time and are retained with the external
publication receipt. VM/CE agents can fetch cREXX develop directly; this task
does not modify or publish Mainframe Lab.
