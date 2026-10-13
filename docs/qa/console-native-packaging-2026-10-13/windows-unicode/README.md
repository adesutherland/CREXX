# Windows helper command line and GNU runtime correction

Build 38389192522 at fcb361b6e passed the Windows product/correctness steps, then
both MSVC and MinGW failed the first fingerprint in the existing Unicode package
acceptance harness. Linux and both Mac core package jobs passed. The completed
job QA artifacts were downloaded without waiting for the whole workflow.
[MSVC failure](msvc-provider-validation.log) reports a multi-byte code-page
mapping error; [MinGW failure](mingw-provider-validation.log) has empty stderr.

The Windows owner's same-source GCC16 probe classified that empty MinGW output:
exit -1073741515 (0xC0000135, missing DLL) under system-only PATH. Adding the exact
SDK libstdc++-6.dll, libgcc_s_seh-1.dll and libwinpthread-1.dll restored the ASCII
fingerprint. The unchanged Unicode harness then failed with an illegal-byte
sequence. These are independently established command-line and runtime gaps.

## Minimal corrections

The helper's Windows entry reads GetCommandLineW/CommandLineToArgvW, holds the
native arguments with LocalFree ownership, and converts each through the existing
utf8_path boundary before executing its original logic. Narrow CRT argv is not
treated as UTF-8. Shell32 is an OS dependency. Non-Windows behavior, manifest
schema, validator, hashing, copying, fingerprinting and collision rules are
unchanged. The acceptance harness and package.hpp are byte-identical.

The MinGW target stages the selected CXX compiler's three runtime DLLs beside
the helper and installs them into bin. Resolution first asks that compiler for
its file path, then checks its adjacent bin directory; it never selects another
runtime from arbitrary PATH directories. Present runtime files contribute
LINK_DEPENDS so an SDK runtime update refreshes staging. Existing core ZIP
directory/manifests then retain their bytes. PATH isolation is not relaxed.

## Qualification and limits

The existing clean hotfix checkout was synchronized to develop ff5164868 before
editing, preserving unrelated occupied dirty worktrees. Exact inputs are in
[qualified-inputs.json](qualified-inputs.json).

| Check | Result | Evidence |
| --- | --- | --- |
| Normal Mac helper/C1/product build | Passed without compiler warnings | [helper](windows-helper-unicode-build-final.log), [build summary](mac-build-summary.txt) |
| Actual extracted Mac core, unchanged Unicode/collision acceptance and relocated native dependency consumer | 30 commands passed, 42.708 s | [summary](mac-summary.json), [acceptance](mac-provider-validation.log), [native](mac-provider-native-build.log), [relocation](mac-provider-relocated.log) |
| Installed SDK inventory/CMake/native consumer | Passed | [result](mac-sdk-result.txt), [archive](mac-sdk-archive.json) |
| Existing split-package regression suite | 13 passed | [log](windows-helper-unicode-split-tests.log) |
| Windows native rebuilt acceptance | Assigned to Windows owner; pending | Exact source commit is reported after publication |

The Windows-only API and staging branches are not claimed natively qualified by
Mac execution. The Windows task will run the unchanged package_acceptance.py
against normal rebuilt MSVC and MinGW helpers, with system-only PATH. This
bounded source repair can publish with its Mac proof and that explicit handoff;
no signed/release-ready or Windows-CI-green claim follows yet.

Full job/build artifacts remain under
`/Users/adrian/Documents/Franz-UI-Mac-Investigation-20261012/evidence/windows-helper-unicode-20261013/`.
Expected publication: two reviewable substantive fixes together on hotfix and
develop, normal automatic Build/CodeQL checks expected to pass. Actual refs and
observed native/CI results are reported live; no status-only CI commit or manual
overnight dispatch is planned. Signing/tagging remains held.

## Published-image rename guidance

AGENTS.md's existing Windows policy remains: avoid overlapping build/test
invocations, check leftovers after interruption, then retry affected publication
targets with --parallel 1. The Windows report is within one parallel build, so
that overlap rule alone does not prevent active shared-bin import readers.
classlib_native and rxfnsc currently copy linked output to private temporary
files and rename into bin; serial publication succeeding is consistent with
reader locking, but the exact owning handle has not been captured here.
No generated-image publication redesign is introduced. This rename observation
is separate from the reproduced CI helper failures.
