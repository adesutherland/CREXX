# S4-D01 approved text correction — 14 September 2026

Adrian approved the [bounded host-service change](../../planning/native-inference-text-boundary-proposal.md).
The provisional implementation preserves the complete UTF-8 byte length through
RXPA. Request admission copies bounded owned text; selectors/paths reject NUL.
The unversioned initializer declaration is byte-identical to baseline HEAD
`c2cf28a4f5c66430b4c2cc4d49b00ae720675ab8` (see the retained layout check).

The immutable service table is passed into each plugin session through a
size-checked V2 manifest tail. Both loaders copy only the available old prefix
and complete optional callback. Existing factories remain unchanged; modern
hosts prefer the host-aware factory without fallback. rxllama rejects old or
missing services instead of reverting to lengthless text. No borrowed VM value
or input pointer is retained after a native call.

Evidence:

- `debug-focused.log`: 8/8 normal Debug host, old-manifest, session, rollback,
  legacy/reentrant and worker-transition controls pass. The host fixture was
  subsequently corrected to use `value_init` and matching pointer types; its
  final rebuild/rerun is in `debug-old-host.log`.
- `debug-old-host.log`: the real rxllama adapter rejects pre-V2, old V2 and
  missing-service hosts; final host-service CTest passes. Dynamic fixtures cover
  a genuinely short historical manifest and the modern factory. Static replay
  includes a poisoned unavailable tail, missing/short/wrong-version services,
  borrowed pointer identity, full ASCII/NUL/Unicode bytes, empty text and cleanup.
- `debug-text.log`: the [original ordinary failure](../native-inference-step04/text-boundary-red.log)
  now passes through rxc/rxas/rxlink and both rxbvm/rxtvm. No expected-fail rule.
- CPU/Metal logs: direct-library vector parity, exact byte/token count for
  `cat`, U+0000, ` dog`, NUL role/selector rejection, full-length byte-limit
  rejection, 512/513-token controls and lifecycle cleanup. Both VMs also pass
  100 singles/20 batches and caller-mutation/result-ownership controls.
- `candidate-identity.json`: exact source/build/test identities at production
  freeze. Later reference-control allocation alignment is qualified by both
  ordinary Release numeric oracle runs in the Release bundle.

The first [ordinary Release verdict](../../../performance/evidence/2026-09-14-ni-s4-first-release/README.md)
shows CPU overhead within variation, with a retained GPU tripwire and substantial
variation. STEP-04 remains paused for Adrian's verdict; this does not close the
whole embedding step. No sanitizer build/test ran. SAN-009, S3-D01 and new
host/request sanitizer/platform proof remain assigned to STEP-06 native-inference
release QA, owned by Codex under Adrian's direction. Work remains uncommitted.
