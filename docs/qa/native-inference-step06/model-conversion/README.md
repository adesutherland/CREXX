# S6-D01 reproducible candidate evidence

Candidate production and QA disposition complete; model adoption/distribution
not pursued following Adrian's instruction to keep this a QA activity. Authority:
[numbered vision, acceptance and plan](../../../planning/native-inference-model-provenance-proposal.md).
The main provider and approved model pins are unchanged.

1. **Immutable inputs:** [inputs.json](inputs.json), [BGE tree](bge-tree.json) and
   [Smol tree](smol-tree.json). Downloaded files match repository Git-object or
   LFS SHA-256 identities; source/config/tokenizer/readme hashes are retained.
   Official pinned source cards declare MIT for BGE and Apache-2.0 for Smol.
2. **Locked conversion:** [wheel lock](requirements-macos-arm64.lock),
   [resolver report](install-report.json), [conversion.json](conversion.json).
   The converter comes from the verified STEP-01 source archive; execution uses
   its in-tree GGUF library. The wheel lock is specific to macOS arm64 / CPython
   3.12. The conversion process requests offline Hugging Face/Transformers use;
   this environment setting is not an enforced-network-denial proof.
3. **Repeatability:** repeat 1 and repeat 2 have identical complete GGUF hashes
   for both models. Logs: [BGE 1](conversion-1-bge.log),
   [BGE 2](conversion-2-bge.log), [Smol 1](conversion-1-smol.log),
   [Smol 2](conversion-2-smol.log).
4. **Compatibility:** [five passing controls](candidate-compatibility.json),
   [GGUF comparison](gguf-comparison.json), [scratch bridge identity](scratch-provider-identity.json).
   Existing embedding CPU, Metal and cross-device controls pass unchanged;
   existing generation CPU and Metal token/text/finish, bounds and cancellation
   controls pass unchanged. No inference/model tuning or performance panel.
5. **Retained candidates:** [local locations, sizes and hashes](candidate-locations.json).
   These are local files, not public downloads. No publication occurred.

The current provider rejects candidate hashes before inference by design. That
initial [readiness failure](compatibility.json) is retained; the candidate-only
scratch build substitutes exactly the two allowed model hashes and retains all
other source/engine/guard behavior. It is not a changed main provider, a test
exclusion or a general bypass for arbitrary models. The scratch build initially
used incorrect unprefixed ggml linker filenames; the retained corrected CMake
uses the packaged `libcrexx-ggml*` filenames. No product repair was involved.

The full tensor comparison finds no changed Smol tensor bytes, and BGE changes
only `position_embd.weight` from F16 to F32 under the upstream converter. BGE's
tokenizer representation/metadata also changes. Compatibility here means the
unchanged documented native contracts and tripwires pass; it does not certify
stored-vector interchangeability, model quality or every hardware/backend cell.

The reproducible scripts are [download-conversion-inputs.py](../download-conversion-inputs.py),
[convert-candidates.py](../convert-candidates.py) and
[compare-candidate-gguf.py](../compare-candidate-gguf.py). They write only to
explicit scratch directories. The approved product pins/downloads remain unchanged.
