# Choose and manage local models

[Guide index](README.md) · [Install](installation.md) · [Examples](examples/README.md)

The common generation example takes a local GGUF filename; its lowercase SHA-256
is optional. The advanced reproducibility examples also take an expected hash.
Download once and reuse the file for repeated runs. No Hugging Face client, Python, model
server or account token is needed for these public files. The provider does not
download models on startup. Allow about 454 MB for both files, plus build/runtime
space; the memory required while running is larger than the file sizes.

## Which models can I use?

Use the general `generation` or `embedding` profile for a compatible GGUF.
Supply its real path. The bridge calculates SHA-256 for model identity when
omitted, or verifies a supplied expected SHA-256 before loading tensors.
Automatic identification reads the file synchronously during opening, before
looking up shared weights; tensor loading remains asynchronous. No model-name
or hash registry must be edited. Different
quantizations are allowed when supported by the pinned engine and geometry
checks. Provision models as immutable files while owners are active.

With the [common client](common.md), selecting `.llm.open(config)` defaults to
`generation`; `.embedding.open(config)` defaults to `embedding`. With the advanced
typed API, pass that profile to `runtime.model(path,sha256,profile,config)`;
an empty `sha256` selects automatic identity.
The old profile names below preserve exact hashes and preparation for repeatable
reference checks. Do not use a preset name for a different model or override its
preprocessing; choose a general profile instead.

### General generation

The GGUF needs a text tokenizer and a chat template supported by the pinned
engine's finite `llama_chat_apply_template` set. You may explicitly set
`chat_template` to a supported template or `raw`. Raw sends the literal prompt
and disallows a system prompt; applications own any role markers in that text.
Unsupported templates fail with an actionable error rather than silently using
Smol's ChatML. For example, the locally tested Gemma 4 E4B artifact works with
explicit raw role markers; its newer template is not automatically supported
by this pinned engine. This does not qualify every Gemma 4 variant.

### General embeddings

Set `pooling` (`cls`, `mean`, `last`), `normalization` (`l2`, `none`),
`query_prefix` and `document_prefix` explicitly, even when prefixes are empty.
Follow the model's intended preprocessing. The loader cannot infer retrieval
quality from its architecture. Output dimensions are read from the model and
carried into packed results. The complete embedding-space specification belongs
beside stored vectors; changing it requires compatible re-embedding/indexing.

### Compatibility limits

The provider admits bounded dense decoder-style execution, including the pinned
engine's BERT embedding path. Expert/MoE, recurrent, hybrid, diffusion and
encoder-decoder layouts are rejected. Token-only GGUF fixtures cannot serve
text requests. Context must be a multiple of 256, at most 8192 per sequence and
no larger than the model's training context; up to eight rows are supported.
Large models need an explicitly adequate RAM/VRAM budget. Model-dependent
reservations are conservative admission estimates, not an allocator or RSS cap.
Unsupported metadata, dimensions, options and limits fail without truncation.

KV head counts and feed-forward widths may be scalar integers or integer arrays
with exactly one value per layer, matching the pinned engine's metadata forms.
Every element is checked against the existing positive geometry bounds before
allocation. Reservation uses the largest value for every layer; it may
overestimate heterogeneous models. Key/value widths and query-head geometry
retain their scalar restrictions. This does not relax RAM/VRAM admission.

The 19 September [per-layer repair](../../../docs/planning/native-inference-layer-geometry-20260919.md)
admits the local Gemma 4 12B Q4_0 artifact: three short native Metal requests
completed at 512 context tokens. At 4096 tokens the unchanged 20 GiB budget
still refuses admission. Raw output includes model control markers; parsing,
template handling and useful answer quality require separate qualification.

Acceptance of a file is distinct from qualification of that artifact,
preprocessing, backend and device. Consult [qualification](qualification.md)
for evidence and remaining platform limits. No automatic download, model-quality
claim, or RAG index migration accompanies general loading.

## Exact reference presets

| Use / profile | Local filename | Bytes | Expected SHA-256 |
| --- | --- | ---: | --- |
| Embeddings / `bge-small-en-v1.5` | `bge-small-en-v1.5-f16.gguf` | 67,308,128 | `f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999` |
| Generation / `smollm2-360m-instruct` | `smollm2-360m-instruct-q8_0.gguf` | 386,404,992 | `48ab3034d0dd401fbc721eb1df3217902fee7dab9078992d66431f09b7750201` |

Use the revision-pinned URLs in the commands below. They identify specific
artifacts, not a moving `main` branch or a similarly named quantization. The BGE
repository alias may redirect to `ggml-org/models-moved`; its content hash is
the identity check. Keep a copy of this table/URLs beside your model files.

## macOS / Linux

This Bash/sh recipe requires `curl` and either `sha256sum` or `shasum`. It checks
an existing file without downloading it again. A download is published under
its final filename only after hash verification; on mismatch it stops and
leaves the `.part` file for diagnosis.

```sh
set -eu
model_dir="$HOME/crexx-models"
mkdir -p "$model_dir"
model_sha256() {
  if command -v sha256sum >/dev/null 2>&1; then
    sha256sum "$1" | cut -d ' ' -f 1
  else
    shasum -a 256 "$1" | cut -d ' ' -f 1
  fi
}
fetch_model() {
  model_file="$model_dir/$1"
  expected_sha="$2"
  model_url="$3"
  if [ ! -f "$model_file" ]; then
    curl --fail --location --retry 3 --output "$model_file.part" "$model_url"
    [ "$(model_sha256 "$model_file.part")" = "$expected_sha" ] || {
      echo "SHA-256 mismatch: $model_file.part" >&2; return 1;
    }
    mv "$model_file.part" "$model_file"
  fi
  [ "$(model_sha256 "$model_file")" = "$expected_sha" ] || {
    echo "SHA-256 mismatch: $model_file" >&2; return 1;
  }
  echo "Verified: $model_file"
}
fetch_model bge-small-en-v1.5-f16.gguf \
  f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999 \
  'https://huggingface.co/ggml-org/models/resolve/499bc8821c6b12b4e53c5bffcb21ec206f212d81/bert-bge-small/ggml-model-f16.gguf'
fetch_model smollm2-360m-instruct-q8_0.gguf \
  48ab3034d0dd401fbc721eb1df3217902fee7dab9078992d66431f09b7750201 \
  'https://huggingface.co/HuggingFaceTB/SmolLM2-360M-Instruct-GGUF/resolve/593b5a2e04c8f3e4ee880263f93e0bd2901ad47f/smollm2-360m-instruct-q8_0.gguf'
```

Run as a script if you do not want `set -eu` to affect your interactive shell.
The directory is a suggestion; any readable local path works, including spaces
when quoted. For an offline machine, download/verify elsewhere, copy the GGUFs
and this identity record, then verify again at the destination.

## Windows PowerShell

This recipe uses Windows' `curl.exe` explicitly, avoiding the PowerShell `curl`
alias, and `Get-FileHash`. It is ready for Windows qualification; the local
macOS checks do not establish PowerShell execution or Windows inference.

```powershell
$ErrorActionPreference = 'Stop'
$modelDir = Join-Path $env:LOCALAPPDATA 'crexx-models'
New-Item -ItemType Directory -Force -Path $modelDir | Out-Null
function Get-CrexxModel($name, $expected, $url) {
    $file = Join-Path $modelDir $name
    if (-not (Test-Path -LiteralPath $file)) {
        & curl.exe --fail --location --retry 3 --output "$file.part" $url
        if ($LASTEXITCODE -ne 0) { throw "Download failed: $name" }
        $actual = (Get-FileHash -LiteralPath "$file.part" -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($actual -ne $expected) { throw "SHA-256 mismatch: $file.part" }
        Move-Item -LiteralPath "$file.part" -Destination $file
    }
    $actual = (Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($actual -ne $expected) { throw "SHA-256 mismatch: $file" }
    Write-Output "Verified: $file"
}
Get-CrexxModel 'bge-small-en-v1.5-f16.gguf' `
  'f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999' `
  'https://huggingface.co/ggml-org/models/resolve/499bc8821c6b12b4e53c5bffcb21ec206f212d81/bert-bge-small/ggml-model-f16.gguf'
Get-CrexxModel 'smollm2-360m-instruct-q8_0.gguf' `
  '48ab3034d0dd401fbc721eb1df3217902fee7dab9078992d66431f09b7750201' `
  'https://huggingface.co/HuggingFaceTB/SmolLM2-360M-Instruct-GGUF/resolve/593b5a2e04c8f3e4ee880263f93e0bd2901ad47f/smollm2-360m-instruct-q8_0.gguf'
```

## Model behavior and identity

BGE is the small English embedding example: 384 dimensions, CLS pooling and
L2 normalization. Use the `document` role for stored text and `query` for search
text. The provider adds its pinned search instruction for queries; do not add
it a second time. Its 512-token limit includes the instruction and special
tokens. Oversize text is rejected, so applications own chunking.

SmolLM2 is a lightweight generation demonstrator. The provider applies the
pinned system/user chat template. An empty system string selects the template's
default instruction. Its current defaults are 512 tokens per sequence and 32
output tokens; the rendered prompt plus output must fit. The GGUF's 8,192-token
model maximum does not change those application defaults. Greedy sampling is
the supported policy. Model answers are not correctness assertions; applications
evaluate their own usefulness. See the [operating reference](reference.md).

Store embedding identity with your index: model hash, profile, query/document
preparation, normalization, dimensions, runtime and backend settings. Small
CPU/GPU/batch numeric differences within retained tripwires do not by themselves
define a new embedding space, but near ties can change ranking. This work does
not prove corpus retrieval quality. Changing model/preparation/quantization
requires application compatibility evaluation and may require rebuilding an
index. The examples do not migrate the companion RAG product from Nomic.

## Licences and conversion provenance

The pinned [BGE source card](https://huggingface.co/BAAI/bge-small-en-v1.5/blob/5c38ec7c405ec4b44b94cc5a9bb96e735b38267a/README.md)
reports MIT; the pinned [SmolLM2 GGUF publisher card](https://huggingface.co/HuggingFaceTB/SmolLM2-360M-Instruct-GGUF/blob/593b5a2e04c8f3e4ee880263f93e0bd2901ad47f/README.md)
reports Apache-2.0. Retain the applicable licence/notices when redistributing
weights. Runtime notices cover llama.cpp/GGML, not automatically the model data.

The exact distributed GGUF bytes are verified. Their precise conversion-tool
revision and original checkpoint ancestry are still unresolved in the STEP-01
record. A current source checkpoint is not proof of that ancestry. Parent AC-10
remains open until provenance is obtained or a recorded conversion is approved,
re-pinned and qualified. Do not substitute another file silently to close it.
The repository's `docs/planning/native-inference-step-01-lock.json` is the full
historical pin record; its then-pending inference fields are not current status.

## Scratch models

Pinned upstream `test-llama-archs` can generate tiny random-weight GGUFs for
native plumbing tests. The current provider profiles still require BGE/Smol
semantics; do not pass a scratch llama under the Smol profile. At this upstream
pin BERT fixtures are skipped because vocabulary support is unfinished, so BGE
remains the small real embedding control. Stories260K is also a native test
fixture, not a qualified product profile or a redistributed example dependency.
Scratch controls do not establish meaningful embeddings, model answers or GPU
qualification. The repository's `tests/native-inference/README.md` records the
explicit scratch tooling separately from these user examples.
