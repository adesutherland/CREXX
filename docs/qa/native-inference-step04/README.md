# STEP-04 initial embedding evidence — 14 September 2026

[STEP-04](../../planning/native-inference-step-04.md) owns the current scope.
STEP-03 closure and its named STEP-06 sanitizer handoff are approved. This is
uncommitted STEP-04 work on baseline `c2cf28a4f5c66430b4c2cc4d49b00ae720675ab8`.
No sanitizer build/test or performance measurement was run in this slice.

- `pre-implementation-identity.json`: retained STEP-03 identity; all 27 source
  hashes matched before STEP-04 code edits.
- `embedding-before-implementation.log`: ordinary embedding acceptance fails
  with missing request operations before implementation (compiler exit 2).
- `debug-build.log`, `debug-oracle-build.log`: normal Debug builds pass.
- `debug-cpu.log`, `debug-metal.log`, corresponding `*-toolchain.log`: rxc, rxas,
  rxlink and both rxbvm/rxtvm pass 100 repeated single requests, 20 eight-row
  batches, packed result shape/norm, invalid input, cancellation, active-request
  admission, session reuse and teardown. Each VM uses one loaded BGE model.
- `debug-oracle-cpu.log`, `debug-oracle-metal.log`: fixed direct-library numeric
  comparison passes for batching/order, query prefix, empty/Unicode inputs,
  512/513-token limits, same/different batch layout, private sessions and cleanup.
  These are integration correctness controls, not llama.cpp/model benchmarks.
- `text-boundary-build.log`, `text-boundary-red.log`: permanent cREXX reproducer
  builds and fails at runtime (exit 1). RXPA loses the true string length;
  an embedded U+0000 silently shortens a configuration value. The same seam is
  unsuitable for the approved complete embedding-text contract. No expected-fail
  registration or weakened assertion hides this failure.
- `candidate-identity.json`: exact current provider/test/build-artifact hashes.

[S4-D01](../../planning/native-inference-text-boundary-proposal.md) proposes a
negotiated length-aware RXPA text service. No host/plugin-contract edit was made
without approval. STEP-04 is provisional: this defect, further request boundary
and concurrent-worker coverage, typed facade, native/install acceptance and the
first ordinary Release integration-overhead verdict remain open. Broad inherited
Debug tests were not repeated. STEP-06 owns new and outstanding sanitizer proof.

Reproduce the four-tool controls with `cmake -DBUILD=ABS_BUILD -DSOURCE=ABS_SOURCE
-DOUTPUT=SCRATCH -DMODE=cpu -DMODEL=BGE_PATH -DHASH=PINNED_SHA256 -P
tests/native-inference/step04_toolchain.cmake`; repeat with `required-gpu` on the
qualified local Metal host. The exact commands are retained in toolchain logs.
The explicit `rxllama_embedding_bridge` target runs as `MODE BGE_PATH SHA256`.
The text-boundary source builds with the ordinary `crexx --program ...` driver,
then its rxbin executes with the matching `rxvm`. No tests download models.
