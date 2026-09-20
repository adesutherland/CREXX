# RXAS on the historical CMS ELF profile

20 September 2026. This is an upstream portability dependency of Mainframe
Lab TP-AC-05, starting at `4468c7d75925cc158a568392f4abe97a59156df6`.

## Intended outcome

Run the actual RXAS assembler on historical CMS, cross-built on macOS. Preserve
the current language, 64-bit integer and RXBIN contracts, optimizer and legacy
CMS port. The modern compiler profile has its own ASCII/newlib service layer;
its inherited Linux predefines must not select Linux platform services.
Mainframe Lab owns the compiler, runtime, guest and application qualification.
This worktree owns only necessary cREXX portability changes. A host build or
successful object probe does not establish CMS execution or memory fit.

## Acceptance criteria

- [ ] **CMSRX-AC-01:** explicit `__MAINFRAME_LAB_VM370_4381__` selection takes
  precedence over inherited OS aliases; legacy `__CMS__` and normal host paths
  remain distinct. Compile all 28 RXAS translation units through the profile.
- [ ] **CMSRX-AC-02:** file existence uses the profile's standard I/O contract;
  unsupported directory enumeration reports `ENOSYS`. Host tests cover file
  lookup, missing files, no executable path and unsupported directory behavior.
- [ ] **CMSRX-AC-03:** ordinary host RXAS still builds and produces unchanged
  optimized/unoptimized reference bytecode. Retain exact source identities and
  relevant normal tests before considering integration into develop.
- [ ] **CMSRX-AC-04:** actual CMS application execution, expected output and
  errors, cleanup and measured memory pass in Mainframe Lab. Until then this
  remains an unintegrated port candidate, not a qualified CMS release.

## Implementation steps

1. **CMSRX-STEP-01** (AC-01): add explicit platform selection within the existing
   platform interface; keep generated files and downstream source rewrites out.
2. **CMSRX-STEP-02** (AC-02/03): add focused platform tests and rebuild host RXAS;
   qualify unchanged reference assembly with and without optimization.
3. **CMSRX-STEP-03** (AC-01/04): export the upstream candidate for the target
   build; record link/service dependencies and run the real guest application.
   Runtime work proceeds under Mainframe Lab's numbered TP05 plan.

The CMS profile has no process ID, executable pathname, POSIX terminal or
directory iterator service. Those remain unavailable. File names passed to
standard I/O are mapped by the CMS runtime, not by RXAS serialization code.
