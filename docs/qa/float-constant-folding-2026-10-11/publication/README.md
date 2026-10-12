# Full CTest qualification and develop publication — 11 October 2026

Adrian authorized full CTest, then commit/publication when green. The research
agent owns pulling/merging the published develop changes and qualifying them
after publication. This task did not edit or stop the research checkout.

The full normal Debug catalogue has **3,329 tests**. Preparation used
`cmake --build cmake-build-debug --target qa-prep-all --parallel 10` and passed
with **zero warnings**. An assembler edit was restored during preparation;
its exact validated contents were restored and staged, then preparation was
repeated against the stable source. Adrian confirmed no other edits were
intended. Source hashes stayed frozen through the full sweep.

The complete command was:

```sh
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure \
  --output-junit docs/qa/float-constant-folding-2026-10-11/publication/full-ctest.xml
```

The full sweep took **2,271.82 seconds**: **3,324 passed**, five failed, zero
skipped/disabled tests and no timeout or sanitizer finding. All failures were
inspected rather than excluded:

- Three AST goldens had 11 synthetic register-index constants still annotated
  `.unknown`; typed literal parsing now correctly annotates them `.int`.
  Every difference was verified to be that annotation alone.
- `select_dispatch_general_opt` now loads/formats seven positive numeric
  literals before strict string comparison, respecting consumer formatting.
  The seven comparisons retain their logic, and runtime controls pass.
- `int64_literal_overflow_fail` exposed an unintended error-category change.
  Strict parsing must retain OVERFLOW_UNDERFLOW for a lexer-validated integer
  literal whose parse reports ERANGE. The correction changes only this
  already-failed error path; invalid text/fraction casts remain BAD_CONVERSION,
  and the INT64_MIN exception remains intact. The final golden also removes an
  obsolete duplicate assignment diagnostic caused by the former same-type
  display reparse: two invalid literals now produce two precise errors.

After the correction, **all five failures passed** in a 0.72-second replay.
The focused normal check covered numeric matrices, int64, comparisons, casts,
contexts, register views and metadata; all unaffected controls passed. Its one
remaining failure was the obsolete duplicate diagnostic expectation, resolved
by the final replay. Expanded maintained Apple ASan passed **9/9** in
**39.52 seconds**, with no findings. Existing Apple SDK deprecation warnings in
unchanged functions are retained; normal builds are warning-free. Apple LSan
is unsupported, so this is not Linux leak/cross-platform qualification.

**Final full-catalogue outcome: 3,329/3,329 green**, using the retained 3,324
unchanged passes and the five repaired/replayed outcomes. This is explicitly a
full sweep plus focused replay, not a claim that the initial sweep passed.
Successful compiler paths are unchanged by the diagnostic-only correction;
repeating the entire 38-minute sweep would add no decisive proof. No new test
exclusion, suppression, timeout weakening or broad optimization guard was used.

`full-inputs.json` records the frozen sweep inputs. `final-inputs.json` records
the final source/golden/tool changes and their qualified closure. The original
JUnit and full logs retain the five failures; `final-failure-replay.xml` and
`final-catalogue-outcomes.json` map every test to its final passing evidence.
Build, focused, replay and sanitizer logs are retained alongside them.

Delivery is the compatible numeric transport, unordered comparison, float
negation and explicit-cast commits published together to **develop**. This
receipt and the complete code/tests/docs/evidence ship in that first push.
Normal automatic Release product/smoke, optimizer-parity and CodeQL workflows
are expected to succeed; hosted results are not claimed verified here, and no
manual overnight/deep workflow is dispatched or used as an additional gate.
Publication/ref verification is reported in the conversation, without a
CI-status-only follow-up commit.

RS-F01 is resolved and RS-AC04's compiler blocker is addressed. The research
agent will qualify the adopted repair after publication; wider Level L research
completion is not claimed. Earlier finite-repair, scan and local qualification
receipts remain historical evidence.
