# NR-21 accepted QA closeout

Adrian accepted the first production Release verdict and approved QA,
documentation audit, closeout and a local commit on 2026-07-20.

## Commands and results

```text
cmake --build cmake-build-debug --parallel 10
```

Result: complete Debug product build passed.

```text
ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^(nr06_codegen_contract|nr21_fixed_call_contract|repro_duplicate_call_argument_run_(noopt|opt)|nr06_call_window_scalar_run_(noopt|opt)|inline_test_byvalue_arg_reuse_run_(noopt|opt)|test_signal_block_(noopt|opt|bin_build)|rxlink_format_check|rxlink_rxdas_smoke|rxdas_roundtrip_(basic|compact_format)|compact_format_check|rxas_optimizer_metadata)$'
```

Result: 17/17 passed.

```text
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```

Initial result: 1,775/1,871 passed. The 96 failures were compiler golden
comparisons whose generated RXAS contained the accepted fixed-call lowering;
runtime counterparts remained green.

The 96 files were refreshed through the documented
`crexx_test_driver --update-gold` path. A zero-context mechanical diff audit
found 500 added and 933 removed RXAS lines:

- additions: `call`, `call1...call4`, preserved standalone `settp`, and four
  `setlinkattr1` instructions unfused after count-load removal;
- removals: count loads/calls, call-window `swap`/fused forms, the four
  corresponding `setlinkiload` combinations, and one repeated-scalar snapshot;
- absent: `.locals`, metadata, source/TRACE, or unrelated opcode drift.

Final result after the audited refresh: **1,871/1,871 passed** in 156.32
seconds at `--parallel 30`.

## Documentation audit

- `compiler/docs/emitter_architecture.md`: selection/fallback rules,
  standalone status setup, repeated-status fallback, unchanged callee
  `a1...aN` view, and RXBIN feature requirement.
- `docs/ai-context/RXAS_ASSEMBLER.md`: fixed direct forms and writer behavior.
- `docs/ai-context/RXBIN_007_SEMANTIC_GRAPH.md`: feature-bit schema and reader
  enforcement.
- `docs/ai-context/RXVM_INTERPRETER.md`: pointer capture, ordinary callee
  bindings, native exclusion and unwind boundary.
- `docs/books/crexx_language_reference/procedures_and_arguments.md`: audited,
  unchanged because the user-visible pass-by-value, `.ref`, optional and
  argument semantics are unchanged.

NR-12 read-only-argument work remains deferred until flow analysis. Inline
formal/result simplification remains separate. In accordance with the
approved shortest closeout path, this QA did not add sanitizer,
install/package, cross-platform, alternate-layout, extra timing, or follow-on
prototype work.
