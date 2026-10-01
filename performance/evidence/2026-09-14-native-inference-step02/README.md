# Native inference STEP-02 evidence, 2026-09-14

Authority: [approved plan](../../../docs/planning/native-inference-backlog.md)
and [STEP-02 status/thresholds](../../../docs/planning/native-inference-step-02.md).
The [completion record](completion/README.md) closes S2-01 through S2-05 and
retains final controls, Debug/ASan evidence, bounded generation prefill,
CPU/Metal absolute measurements and memory probes. Original v1/v2 records below
remain historical evidence with their original dispositions.
This is a focused native-library control, not a representative language
portfolio result, a RexxCPS comparison, or a CREXX provider performance verdict.

`identity.json` records the original version 1 source/binary hashes, exact upstream identity, compiler,
SDK, build scope and the existing Release cREXX capture toolchain. The
[versioned workload and commands](../../../tests/native-inference/README.md)
reproduce the controls. No product code or provider implementation was changed.
The approved [S2-D01 version 2 rerun](v2/README.md) has its own identities and
retains a reverse patch to reconstruct the version 1 workload. Do not compare
the original control binary hash with the rebuilt version 2 executable.

- `controls/`: structured model/token/vector outputs and the independent
  upstream CLI reproduction. `result: FAIL` is a failure even when the workload
  completed and retained diagnostic values. Earlier short precision controls
  are explicitly diagnostic; they are not performance baselines.
- `logs/`: source build, control build, CPU/Metal runs, failed scratch generator,
  failed provider compilation and successful packed-vector control. The first
  Metal initialization log also retains shader compilation and real offload
  evidence. Subsequent setup may benefit from OS/compiler caches.
- `capture/`: unchanged Level B runner manifests, every raw serial sample and
  CSV, plus host power/load/thermal snapshots. Warm upstream compute counters
  appear in each `CONTROL_JSON` record; complete-process elapsed time includes
  setup, verification, serialization, cancellation and teardown.
- `checksums.sha256`: hashes of retained evidence files, excluding itself.

SmolLM2 CPU and Metal controls exercise 100 repeated requests, 20 bounded
batches and four private contexts sharing one model. CPU and Metal generated
text need not be identical; each backend must satisfy its own serial/repeat
controls and token/finish limits. BGE tests retain the original failed numeric
checks. The separate version 2 CPU/Metal runs pass the approved S2-D01 tripwire;
version 1 failures are not relabelled. No BGE performance pass is claimed.

Both serial captures passed all twelve correctness checks but showed substantial
elapsed-time variation: CPU 23.16–39.88 seconds and Metal 18.79–27.20 seconds for
recorded complete-control processes. Host load also changed. Keep these samples as diagnostic evidence;
do not use them as a qualified comparative baseline or remove slower samples.
An active VM was observed later; it was not interrupted. No speedup or optimal
hardware-selection claim follows from these captures.

All initial-delivery ACs remain open. These records do not establish installed
packaging, cREXX worker ownership, corpus retrieval quality, CUDA/Vulkan, native
sanitizer closure, or product qualification. The completion record supplies the later Step 2
performance/cancellation controls without closing those product gates.
