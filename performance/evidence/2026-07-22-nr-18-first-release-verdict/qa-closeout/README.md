# NR-18 accepted quality closeout

Status: complete on 2026-07-22.

## Provenance and scope

- Branch: local `develop`, based on accepted NR-27 commit
  `65ea6b9e292b5335565ec63d86c6a40e5f35853b`, one commit ahead of
  `origin/develop` before the NR-18 commit.
- Candidate source: the exact NR-18 worktree committed with this evidence.
  Its production scope is `assembler/rxas_flow.c`, `assembler/rxasassm.c` and
  `assembler/rxasassm.h`; the remaining changes are focused tests, RXAS
  architecture documentation and live performance records.
- Host: macOS 26.5.2 build 25F84, Darwin 25.5.0 arm64, Apple M5.
- Toolchain: Apple Clang 21.0.0, CMake 4.3.2, Ninja 1.13.2.
- Build modes: Ninja Debug and Release with `/usr/bin/cc` and
  `CREXX_VM_PROFILING=OFF`. The first-verdict instruction profiles use the
  separately identified existing profile VMs; they are not wall-clock results.
- Artifact policy: the optimized-source census retains RXAS instruction
  records. Formal common products were linked with `rxlink -s`; each accepted
  NR-27/NR-18 pair is byte-identical. No timing samples were generated for
  identical products.

## Quality gates

The complete Debug product rebuild passed all 1,129 Ninja steps:

```sh
cmake --build cmake-build-debug --parallel 10
```

The consolidated affected surface passed 63/63. It includes the six
whole-procedure/NR-18 optimized and `-n` structural checks, four optimized and
`-n` runtime checks across `rxvm` and `rxbvm`, all selected complete/malformed
jump-table and corruption checks, plus the linked-artifact fixture:

```sh
ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^(rxas_optimizer_(whole_procedure_flow|whole_procedure_flow_noopt|whole_procedure_panel|whole_procedure_panel_noopt|nr18_flow_harvest|nr18_flow_harvest_noopt)|nr18_flow_runtime_|rxasjtable|rxasjumptable)'
```

The required broad Debug gate passed 1,891/1,891 in 169.38 seconds:

```sh
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```

`git diff --check` also passes. The accepted shortest closeout does not add a
sanitizer, install/package, cross-platform or repeated timing campaign: no VM,
RXBIN, ABI, language or installed-product surface changed, the affected
semantic surface and complete Debug suite are green, and the decisive ordinary
Release evidence remains valid.
