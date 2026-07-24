# PERF2-01 verification record

Status: complete. Product timing authority is the profiling-off Release build;
diagnostic verification uses the separately attributable profiling-on build.

Final results:

- Formal product: 31/31 benchmark tests passed from the clean detached exact
  commit; formal rxvm/rxbvm/library hashes match `../01-artifacts/`.
- Diagnostic Release: 17/17 profile, call-census, RXSEQ and documentation tests
  passed. Diagnostic Debug: `rxvmprofileaccounting` passed.
- Focused sanitizer: the profiling-on ASan build passed and
  `rxvmprofileaccounting` passed with `detect_leaks=0`. The prior leak-on
  invocation is retained as a platform limitation because Apple ASan reports
  that leak detection is unsupported; it did not enter the test body.
- Counts mode: two deterministic Sieve profiles are byte-identical at SHA-256
  `95a2c3dc70b4b95098afa384892b8e723ef96682ca9a326dfb2c1fa5c7916374`;
  counts timing fields are zero by contract.
- Compile out: the OFF cache, symbols, strings, compile definitions and CLI
  rejection all pass in `profiling-off-compile-out.log`.
- Profile closure: rxvm/rxbvm counts have 22 entries each; selected timing has
  10 each; all four internal checksum manifests independently verify. Domain
  audit reports zero overflow, unavailable, degraded or overflowed rows.
- Level B tools: timing matrix, evidence bundle, schema-4/5 profile summarizer,
  artifact summarizer, gap ledger and inventory self-tests all pass.
- Native correctness: 17 accepted stdout captures pass. The checksum-less
  Mandelbrot attempt is rejected and replaced. Four accepted heap stdout
  captures pass; the Bounce lite high-water failure is rejected and replaced.
- `git diff --check` passed, and the final recursive bundle checksum file was
  generated and reread by the cREXX inventory tool.

The two prerequisite-diagnostic logs are intentionally retained. The first
17-test run could not execute `rxseqfileformat` and could not reach the expected
module-mismatch assertion because the focused build lacked `test_rxseqfile`
and `tests_basic.rxbin`. The first correction used a non-existent target name;
the final exact targets `test_rxseqfile` and `run_tests_basic` built the needed
artifacts, after which the unchanged test selection passed 17/17. These are
control-plane invocation corrections, not product or test failures.

The cREXX gap-ledger self-test also guards the summary median column. During
draft dossier construction that new tool was found reading Q1 for displayed
throughputs and RexxCPS. Common ratios/geomeans produced by the existing runner
were already median-based. The tool was repaired before bundle closure and all
draft Q1-derived RexxCPS values were replaced.
