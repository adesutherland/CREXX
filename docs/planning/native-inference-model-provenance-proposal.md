# S6-D01 — Reproducible demonstration-model conversion

Status: QA conversion/compatibility exercise complete, 15 September 2026.
Adrian's latest direction is to do what is necessary for QA confidence and avoid
overcomplication. Agent disposition: retain these results as QA evidence, keep
the existing approved pins/downloads, and do not pursue new model distribution.
No approved model has been replaced. Original-artifact conversion ancestry is
still unproven; this experiment does not manufacture that historical fact.
Parent authority: [OUT-05, AC-05/10 and S6-07](native-inference-backlog.md).

## Vision and outcomes

1. **MP-OUT-01:** a user can trace each distributed demonstration GGUF to exact
   checkpoint/tokenizer files and an executable, pinned conversion recipe.
2. **MP-OUT-02:** retain the approved lightweight BGE-small-en-v1.5 F16 and
   SmolLM2-360M-Instruct Q8_0 families, preparation and in-process CPU/GPU surface.
3. **MP-OUT-03:** change artifact identity only with explicit review; preserve
   historical evidence and identify which model-dependent checks need refreshing.

## Evidence and choice

Both current GGUFs are byte-pinned, readable and operational. The STEP-01 lock
contains their inspected metadata and source comparison revisions. The pinned
[BGE converter card](https://huggingface.co/CompendiumLabs/bge-small-en-v1.5-gguf/blob/d32f8c040ea3b516330eeb75b72bcc2d3a780ab7/README.md)
and [Smol publisher card](https://huggingface.co/HuggingFaceTB/SmolLM2-360M-Instruct-GGUF/blob/593b5a2e04c8f3e4ee880263f93e0bd2901ad47f/README.md),
re-read in STEP-06, still do not identify an exact converter and original
checkpoint revision. Smol names the GGUF-my-repo conversion service; that does
not identify the service revision or its dependencies. The GGUF metadata does
not close the gap. A repository name or a later comparison checkpoint is not
proof of ancestry.

Recommended: produce candidate GGUFs ourselves from the immutable official
comparison checkpoints already recorded in STEP-01, using the already pinned
llama.cpp converter. Retain candidates alongside the approved models and report
their identities and focused compatibility results before adopting new pins.
This involves development-only conversion dependencies; ordinary plugin users
still download ready-made GGUFs and need no Python or conversion tools.

Alternative: keep the existing artifacts and explicitly revise AC-10 to accept
unresolved conversion ancestry for these demonstrators. That would be a change
to the accepted outcome and needs Adrian's decision; it is not an inferred waiver.
Obtaining conclusive original conversion records would avoid either change, but
the currently available records do not supply them.

## Candidate inputs and recipe

| Input | Exact candidate source |
| --- | --- |
| BGE checkpoint/tokenizer | `BAAI/bge-small-en-v1.5` at `5c38ec7c405ec4b44b94cc5a9bb96e735b38267a` |
| Smol checkpoint/tokenizer | `HuggingFaceTB/SmolLM2-360M-Instruct` at `a10cc1512eabd3dde888204e902eca88bddb4951` |
| Converter and GGUF library | llama.cpp `5266f24da75dc449bd56cbed7addb9c8e4a6a73e`, the existing engine pin |

After approval, fetch only the checkpoint/config/tokenizer/licence inputs needed
from those revisions into fresh directories, retaining every downloaded file's
SHA-256. Resolve the pinned converter's declared dependencies in an isolated
environment and retain the interpreter identity, exact dependency lock and
package hashes. The inspected converter supports these output choices:

```sh
python llama.cpp/convert_hf_to_gguf.py checkpoints/bge \
  --outtype f16 --outfile candidates/bge-small-en-v1.5-f16.gguf
python llama.cpp/convert_hf_to_gguf.py checkpoints/smol \
  --outtype q8_0 --outfile candidates/smollm2-360m-instruct-q8_0.gguf
```

These are proposed commands, not completed conversion evidence. Record all
resolved metadata, tokenizer/chat-template fields, tensor types and final hashes.
Repeat conversion from the same locked inputs to verify reproducibility; if
outputs differ, diagnose the difference before proposing adoption. Do not change
pooling, prefixes, normalization, context limits or sampling to make a test pass.

## Numbered checkable acceptance criteria

1. [x] **MP-AC-01:** every source/config/tokenizer/licence file and converter/
   dependency input has an immutable identity and retained hash; no ancestry claim
   is based on inference. Covers MP-OUT-01.
2. [x] **MP-AC-02:** two conversions with identical locked inputs yield identical
   GGUF hashes, or an explicit reproducibility defect remains open. Covers MP-OUT-01.
3. [x] **MP-AC-03:** candidates preserve the documented 384-dimensional BGE CLS/L2
   preparation and bounded Smol generation contracts. Existing same-backend direct
   parity and CPU/Metal embedding tripwires pass without relaxed thresholds;
   missing device checks remain named. No model-quality or performance-tuning
   study is introduced. Covers MP-OUT-02.
4. [x] **MP-AC-04 — QA disposition (revised after Adrian's direction):** record
   old/candidate hashes, conversion recipe, licences, compatibility and limits;
   retain current pins and the existing performance verdicts under their original
   identities. No adoption/distribution project follows from this QA exercise.
   Covers MP-OUT-03; replaces the proposed adoption-review criterion.

## Numbered execution plan

1. [x] **MP-01:** Adrian decides between candidate conversion and the explicit
   AC-10 revision. Approved response: “Produce reproducible candidates
   (recommended)”. No current artifact/pin changes; adoption follows MP-03.
2. [x] **MP-02:** if conversion is selected, freeze/download the named source
   inputs, lock the isolated conversion environment and produce two identical
   candidate artifacts. Serves MP-AC-01/02; depends on MP-01.
3. [x] **MP-03:** run the existing bounded contract/numeric controls on candidates,
   retain identities/results and propose adoption. Serves MP-AC-03/04; depends
   on MP-02. Do not overwrite historical or approved artifact files.
4. [x] **MP-04 — Record QA disposition:** retain results without repinning or
   changing download instructions; continue the main STEP-06 checks. Serves
   MP-AC-04; depends on MP-03. This replaces the proposed adoption task following
   Adrian's direction: “This is a QA activity - just do the necessary to gain
   confidence in teh QA - do not oiver complicate”.

## Candidate results for adoption review

The [retained conversion/compatibility record](../qa/native-inference-step06/model-conversion/README.md)
includes all input hashes, the 28-package wheel lock, exact converter commands,
two matching outputs per model, GGUF inspection and the five passing controls.

| Artifact | Current approved SHA-256 | Candidate SHA-256 / bytes |
| --- | --- | --- |
| BGE F16 | `f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999` | `cdff157dc108aee08f38024b8f046f1774f6a4a29fb3a170564232ceeb74ac17` / 67,582,784 |
| Smol Q8_0 | `48ab3034d0dd401fbc721eb1df3217902fee7dab9078992d66431f09b7750201` | `afb8552a500c22958467716b7991000dcc941d1f108a516c721b4cc40d41bad9` / 386,405,280 |

Smol retains identical tensor bytes and tokenizer metadata; its differences are
descriptive/architecture metadata. BGE retains 196 identical tensors; the pinned
converter emits its positional-embedding tensor as F32 rather than F16 and
updates tokenizer representation/metadata. These are the upstream converter's
outputs, with no local conversion or inference tuning. BGE remains 384-dimensional
with the existing CLS/L2/profile preparation and passes the unchanged numeric
tripwires. This evidence does not claim old and new stored BGE vectors are
interchangeable: indexes should retain and enforce the full model/profile identity.

The main provider intentionally enforces current model hashes. The candidate
test therefore used a separately built scratch bridge whose only source changes
are the two exact hash substitutions, with a distinct candidate build ID. It
used the unchanged embedding/generation controls and the same pinned CPU/Metal
engine/backend binaries. The original pin-rejection result is retained as an
expected guard result. The main source, model cache files and installed provider
were not repinned.

The proposed adoption/release-assets work is not pursued following Adrian's QA
direction. Generated GGUFs remain local QA artifacts; existing download recipes,
approved pins and historical STEP-04/05 performance verdicts stay unchanged.
No timing panel, upload or new distribution channel is introduced. The exercise
supports confidence in the model families, pinned converter and existing native
contracts; it does not prove the unknown historical ancestry of the current
distributed GGUFs or close the remaining platform/sanitizer/package gates.
