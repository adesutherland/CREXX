# NR-09 accepted-batch QA closeout

Status: **complete with one documented platform limitation; no product
regression remains**.

## Correctness and compiler-output audit

- The final isolated Debug rebuild completed all 927 build steps.
- The focused post-fix selection passed 10/10 tests, including C2d/X2d in both
  compiler modes, dynamic/static native unwind in optimized/no-opt modes, and
  the typed final-instruction combiner.
- Full Debug CTest passed **1,864/1,864** in 249.57 seconds.
- Exactly 110 compiler goldens changed (73 no-opt and 37 optimized). A
  pre-update diff audit found only retained large-instruction substitutions
  and 56 TRACE register-retarget pairs. Every TRACE pair preserves the
  authored symbol and all non-register metadata byte-for-byte. The refreshed
  goldens are byte-identical to fresh compiler output.

The broad run found two real semantic-boundary defects and one test-harness
race before the goldens were accepted:

1. AST-side alias/cleanup fusion could select an integer copy from provenance
   even when the emitted typed operation was a string copy. That corrupted
   C2d/X2d results. The unsafe AST shortcut was removed; cleanup fusion now
   occurs only in the final typed-instruction combiner, where an actual
   `ICOPY` must be present.
2. Native signal/unwind cold restore recognized only legacy `CALL`/`DCALL`
   forms. It now recognizes the three retained fused call forms and obtains
   their count-register operand from the fixed runtime image.
3. The two legacy binary tests shared fixed scratch filenames and could race
   under parallel CTest. They now share a narrow CTest resource lock. The
   locked pair passes at Debug parallel 30 and ASan parallel 8.

The combiner unit tests now prove TRACE retargeting for single-unlink,
multi-unlink and wide immediate forms. This keeps semantic ownership at the
typed-emission boundary and prevents the broader class of provenance-versus-
emitted-opcode mistakes exposed by C2d/X2d.

## Documentation audit

The final documentation was checked against the live `rxas -i` surface rather
than the provisional batch inventory:

- **574/574** source-accepted opcode forms map to **367/367** exact mnemonic
  headings and Forms tables.
- The 25 large/fused mnemonics each have the standard purpose, forms,
  operands/semantics, signals, example and related-instruction sections.
- All **373** tagged RXAS examples assemble; **328** execute and **45** remain
  intentionally assemble-only.
- The compiler architecture records that Class 2 cleanup fusion matches the
  actual final typed mnemonic, not AST provenance.
- The VM architecture records `SWAPCALL`, `SETTPSWAPCALL`, and `SETTPCALL` in
  native cold signal-window restoration.
- The live mapping register records the final 34-form surface, 26 reserved
  withdrawals, broad QA, final Release result and pending commit state.

The audit also corrected two older selector examples to use current `rxsig1`
descriptors required by RXBIN semantic-graph validation.

## Sanitizer

The supported Apple ASan run completed the full build and exercised all 1,864
tests. It reported no sanitizer diagnostic; 1,863 tests passed and the sole
failure was the reproducible shared-scratch-file race above. After adding the
resource lock, the affected pair passes 2/2 under the requested parallel ASan
run. The initial `detect_leaks=1` attempt is retained separately: Apple's ASan
aborts because leak detection is unsupported on this platform, so LSan cannot
be claimed here.

## Release, install and old-RXBIN compatibility

- The final ordinary profiling-off Release rebuild completed all 1,130 steps.
- `cmake --install` succeeded into an isolated prefix containing 112 files.
- The installed `crexx` driver compiled and executed the installed
  `hello.crexx` example, printing `4711` and `hello CREXX world!`.
- Both installed VMs execute the exact accepted pre-batch RXBIN and library:
  `rxvm` passes at 1,218,376 CPS and `rxbvm` passes at 1,207,487 CPS.
- The accepted RXBIN/library hashes are
  `fa2856eff33d2514536e01149c76e6160f8ab9632f2896db22fc5923a69dd11b`
  and `ec8c8ae42793def24a952800164059cb52820db5f7621364e884673666240817`.
- This CMake tree defines `install/local` and `install/strip`, but no `package`
  target or CPack configuration. The isolated installed-tree proof is therefore
  the applicable packaging gate; no package artifact is claimed.

The final 78-execution Release campaign and its paired analysis are in
`../finalrun01/`.

## Evidence index

- `debug-build-safe-boundary.log`, `debug-build-safe-boundary.status`
- `safe-boundary-focused-ctest.log`
- `debug-ctest-final.log`, `debug-ctest-final.status`
- `golden-safe-preupdate.diff`, `golden-safe-preupdate-numstat.txt`
- `golden-update-commands.txt`, `golden-update.log`
- `golden-postupdate.diff`, `golden-postupdate-numstat.txt`
- `asan/20260719-205746-full/` (unsupported leak-detection attempt)
- `asan/20260719-205848-full/` (full supported ASan run)
- `asan/20260719-212042-ctest/` and `asan/20260719-212239-ctest/`
- `debug-binary-legacy-locked-pair.log`
- `release-build-final.log`, `release-build-final.status`
- `release-install.log`, `release-install.status`
- `installed-crexx-help.log`, `installed-crexx-hello.log`
- `installed-old-rxbin-compatibility.txt` and per-VM stdout/stderr logs
- `documentation-audit.log`
