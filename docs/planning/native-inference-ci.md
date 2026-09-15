# Native inference release pipeline and branch qualification

Status: implementation authorized by Adrian, 15 September 2026. This is the
STEP-06 pipeline work package under [the authoritative plan](native-inference-backlog.md),
preserving OUT-01–05 and AC-01–14. It does not replace real-device qualification
with a fixture pass. Candidate branch: `temp/llama-release-qa` on `origin`.

## Vision and outcomes

1. **CI-OUT-01:** a release recipient can use `import llama` from the supplied
   cREXX binaries after provisioning a supported model. The release includes
   the provider, pinned engine, CPU/backend libraries, redistributable runtime
   dependencies, manifests, notices, guides and examples. No recipient C/C++
   build, separate llama.cpp installation or inference server is required.
2. **CI-OUT-02:** routine builds perform small, useful checks of the actual
   payload. A generated random-weight GGUF exercises a few engine computations;
   the public provider is checked for load, discovery, native factories, model
   rejection and cleanup. Public profile/hash checks are unchanged. Retained
   real BGE/Smol tests cover their successful full provider inference paths.
3. **CI-OUT-03:** the remote candidate can be rebuilt, inspected and cancelled
   independently of `develop`. Artifacts/logs identify the exact commit. Wider
   core regression and supported sanitizer workflows qualify that candidate;
   promotion is considered only after the required workflows are green.

## Numbered acceptance criteria

1. [x] **CI-AC-01:** the candidate contains the accepted implementation and
   current remote RXPP changes, with both parents retained. Neither remote
   `develop` nor a GitHub release is modified during qualification.
2. [ ] **CI-AC-02:** the ordinary four release archives contain a complete
   llama provider: CPU/Vulkan on Linux and Windows; CPU/Metal on both Mac
   architectures. The archive includes documentation, examples and notices.
3. [ ] **CI-AC-03:** additional complete Linux and Windows CUDA archives include
   CPU fallback and their redistributable dependencies. Windows CUDA uses a
   compatible MSVC build; existing MinGW delivery is retained separately.
   GPU build/package success does not assert real-device execution.
4. [x] **CI-AC-04:** smoke uses a generated fixture smaller than 5 MiB, with
   generator/source/seed/toolchain/hash provenance, no large model download and
   no test bypass in the public model loader. Checks cover bounded generation
   and finite embedding extraction in the packaged engine, plus public provider
   behavior through installed VM and relocated native consumers.
5. [ ] **CI-AC-05:** smoke fails for a missing/altered required package file,
   verifies fixture identity and required backends, and runs outside the build
   tree using the staged runtime. Test helpers and fixture weights are not
   accidentally shipped as user runtime dependencies.
6. [ ] **CI-AC-06:** branch-scoped cancellation and rerun controls work; failures
   preserve logs. Deadline-sensitive work is serial within each host, using
   wide hang guards. New smoke is measured alone in normal Debug and maintained
   ASan before CTest registration. No model-quality/performance workload enters
   the routine candidate pipeline.
7. [ ] **CI-AC-07:** the candidate release builds/smokes and wider core Deep
   Build/Sanitizer QA gates have terminal results for the exact candidate SHA.
   Linux ASan/LSan keeps leak detection enabled. Retained unchanged local tests
   are reused; new RXPP inputs are covered by hosted qualification.
8. [ ] **CI-AC-08:** human/agent guides, parent acceptance status and artifact
   evidence describe exactly what is shipped and tested. Remaining real-GPU,
   model/provenance and release gates remain visible; green fixture checks do
   not silently close them.

## Numbered implementation steps

1. [x] **CI-01 — Freeze the candidate:** checkpoint accepted STEP-05/06/07 work
   (`12647a91aa7e91478b23961a68f7ff9fb492e1e1`), then merge the two remote RXPP
   commits through `b5b827489` (`c1de1670812a33a8121ab7f28b99b4e50ca5b208`).
   Serves CI-AC-01/07; local `develop` remains at its prior head.
2. [x] **CI-02 — Implement bounded smoke:** reuse pinned upstream generated
   fixture evidence, add package/engine and public-provider controls, measure
   the exact aggregate in normal/ASan isolation, then set scheduling properties.
   Serves CI-AC-04–06; depends on CI-01.
3. [ ] **CI-03 — Build the complete delivery:** enable provider/package targets,
   add CPU/GPU build prerequisites and complete CUDA variants, stage the same
   payload for branch artifacts and eventual release, and smoke it before upload.
   Serves CI-AC-02/03/05; depends on CI-02. Preserve signing/installer safeguards.
4. [ ] **CI-04 — Run remote qualification:** push only the candidate branch;
   exercise selected fast lanes first, repair failures with focused controls,
   then run required wider workflows on the resulting exact SHA. Retain logs
   and cancel superseded candidate runs. Serves CI-AC-01/06/07.
5. [ ] **CI-05 — Reconcile and report:** record terminal run/artifact identities,
   update this checklist and parent criteria, and make the green candidate
   reviewable before promotion. Serves CI-AC-08; depends on CI-01–04. Creating
   the candidate is not permission to publish an unqualified snapshot.

## Design boundaries

- The pinned upstream `test-llama-archs.cpp` generates a 4,763,872-byte dense
  llama fixture already exercised in STEP-02. BERT fixtures are not supported
  at this pin. Random-token engine embeddings are smoke plumbing, not BGE
  pooling/tokenizer or retrieval-quality qualification.
- Keep the actual public provider unchanged: its successful real-model paths
  remain covered by the retained local model campaign and named device gates.
  A fixture is deliberately rejected as an unqualified BGE/Smol artifact.
- Build and package GPU backends without requiring a GPU on every hosted
  runner. CPU fallback must execute on those hosts. Actual GPU computation is
  tested only when a real device is available and is reported separately.
- CI CUDA tooling may be larger than the smoke data; use pinned build inputs
  and caching. No real embedding or generation model is downloaded by default.
- `crexx --native` itself still needs a C toolchain when an application author
  creates a native executable. A release recipient running ordinary cREXX
  programs, or a recipient of a prepared native application, does not need one.
- Later idea from Adrian (15 September): a separate Cognitive example package
  could combine useful programs, model-download guidance and diagnostics. The
  random-weight fixture is developer QA data, has no useful model output, and
  is not included in the current user release. This idea does not expand the
  present delivery scope.

References: [pinned fixture generator](https://github.com/ggml-org/llama.cpp/blob/5266f24da75dc449bd56cbed7addb9c8e4a6a73e/tests/test-llama-archs.cpp),
[GitHub branch concurrency](https://docs.github.com/en/actions/how-tos/write-workflows/choose-when-workflows-run/control-workflow-concurrency).

## Local checkpoint before the first remote run

The aggregate passed isolated Debug/Apple-ASan measurement (44.5/48.3 seconds),
then its registered serial CTest passed (28.20/45.57 seconds). Workflow validation
and 14 publication/signing/matrix controls pass.
[Retained evidence](../qa/native-inference-ci/README.md) records scope and inputs.
CI-03/04 remain open until the candidate packages and wider hosted jobs finish.
