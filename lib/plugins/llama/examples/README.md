# Run and adapt the examples

[Guide index](../README.md) · [Install](../installation.md) · [Models](../models.md)

Start with [common_generation.crexx](common_generation.crexx) for a driver selected
at setup, or [common_embeddings.crexx](common_embeddings.crexx) for explicit
pooling/prefixes and packed results. Both import only `rxfnsg`, so they compile
without the optional plugin. The [common guide](../common.md) explains how a
native executable finds its optional plugin at execution time; it is not bundled
by a core-only import. The advanced examples below explicitly import `llama`,
which retains native provider bundling.

The four advanced typed programs load once, explicitly prepare a private context and process
repeated work before cleanup. They use the public C RXPA factories through
`import llama`. The programs are installed beside this page and are also in
the source checkout's `lib/plugins/llama/examples` directory.

| Program | What to follow | Success output begins |
| --- | --- | --- |
| [persistent_embeddings.crexx](persistent_embeddings.crexx) | One prepared session, twenty two-document batches, owned packed results after request close, portable f32 storage. | `PASS: persistent typed embedding example` |
| [persistent_generation.crexx](persistent_generation.crexx) | One prepared session, twenty four-prompt batches, incremental UTF-8 accumulation and per-row finishes. Prints the first batch's replies. | `PASS: persistent typed generation example` |
| [shared_embeddings.crexx](shared_embeddings.crexx) | Four real workers, twenty eight-document batches each, private results compared with isolated references, one shared weight allocation. | `PASS: four typed embedding workers` |
| [shared_generation.crexx](shared_generation.crexx) | Four real workers, twenty four-prompt batches each, ordered text/token/finish parity with each isolated reference, one shared weight allocation. | `PASS: four typed generation workers` |

The shared examples retain a parent model while workers create their own owners;
the barrier expects five owners (parent plus four workers). They return ordinary
text/bytes between workers, never native handles. They verify worker teardown
and zero remaining runtime reservations. The two generation examples use fixed
prompts with known nonempty output as controls; a general application must also
accept immediate EOS with empty output.

## macOS / Linux: build a native example

After installing and downloading the models, run in a writable directory outside
the checkout. Match `crexx_prefix` and `model_dir` to your earlier choices. The
commands use the installed examples and the installed build driver. The native
compiler must still be available when building.

```sh
crexx_prefix="$HOME/.local/crexx-llama"
model_dir="$HOME/crexx-models"
export CREXX_HOME="$crexx_prefix"
mkdir -p llama-example-work
cd llama-example-work
"$crexx_prefix/bin/crexx" --program "$PWD/embeddings" \
  "$crexx_prefix/share/crexx/llama/examples/persistent_embeddings.crexx" --jobs 1 --native
./embeddings auto "$model_dir/bge-small-en-v1.5-f16.gguf" \
  f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999
"$crexx_prefix/bin/crexx" --program "$PWD/generation" \
  "$crexx_prefix/share/crexx/llama/examples/persistent_generation.crexx" --jobs 1 --native
./generation auto "$model_dir/smollm2-360m-instruct-q8_0.gguf" \
  48ab3034d0dd401fbc721eb1df3217902fee7dab9078992d66431f09b7750201
```

Each build should report `PUBLISHED: native program`; each execution should
print its PASS marker, with no `FAIL:`, `ERROR:` or `PANIC:`. Embedding storage
shape is checked, not printed as hundreds of numbers. Generation prints model
output whose wording is not promised across CPU/GPU/backend revisions.

For the worker examples, use `shared_embeddings.crexx` / `shared_generation.crexx`
as source and a fresh output name such as `shared_embeddings` / `shared_generation`.
Run with the same three model arguments. Start with the persistent examples;
shared examples perform more work and need more private context memory.

Use `cpu` to require CPU execution or `required-gpu` to fail unless a GPU can be
used. `auto` may legitimately fall back. On this Mac the qualified GPU path is
Metal. The persistent examples print actual selected device/backend; the
shared examples verify allocation identity internally. Run each example
sequentially, without broad QA or builds competing for the machine.

## Windows: build and run — pending Windows qualification

Use the developer PowerShell from the install guide and a writable work directory.

```powershell
$crexxPrefix = Join-Path $env:LOCALAPPDATA 'crexx-llama'
$modelDir = Join-Path $env:LOCALAPPDATA 'crexx-models'
$env:CREXX_HOME = $crexxPrefix
New-Item -ItemType Directory -Force -Path 'llama-example-work' | Out-Null
Set-Location 'llama-example-work'
& "$crexxPrefix/bin/crexx.exe" --program "$PWD/embeddings" `
  "$crexxPrefix/share/crexx/llama/examples/persistent_embeddings.crexx" --jobs 1 --native
if ($LASTEXITCODE -ne 0) { throw 'Embedding build failed' }
& './embeddings.exe' auto "$modelDir/bge-small-en-v1.5-f16.gguf" `
  'f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999'
if ($LASTEXITCODE -ne 0) { throw 'Embedding example failed' }
& "$crexxPrefix/bin/crexx.exe" --program "$PWD/generation" `
  "$crexxPrefix/share/crexx/llama/examples/persistent_generation.crexx" --jobs 1 --native
if ($LASTEXITCODE -ne 0) { throw 'Generation build failed' }
& './generation.exe' auto "$modelDir/smollm2-360m-instruct-q8_0.gguf" `
  '48ab3034d0dd401fbc721eb1df3217902fee7dab9078992d66431f09b7750201'
if ($LASTEXITCODE -ne 0) { throw 'Generation example failed' }
```

Check the PASS markers as well as exit codes. Use the same source-name/output-name
substitutions for shared workers. An installed-VM consumer can run on a machine
without a native C compiler; compilation/linking of its cREXX bytecode can be
done on the development machine. Keep its provider install available.

## The explicit four-tool installed-VM route

This Unix recipe makes compiler, assembler, linker and VM steps visible and
does not invoke a native C compiler. It also builds the complete image needed
by workers. Define `crexx_prefix` and `model_dir` as above; use a writable work
directory. Substitute either persistent or shared source; use the corresponding
model/hash for generation.

```sh
"$crexx_prefix/bin/rxc" --no-exe-import -i "$crexx_prefix/bin" \
  -o embeddings_vm "$crexx_prefix/share/crexx/llama/examples/persistent_embeddings.crexx"
"$crexx_prefix/bin/rxas" -o embeddings_vm embeddings_vm
"$crexx_prefix/bin/rxlink" -o embeddings_linked embeddings_vm \
  "$crexx_prefix/bin/library" "$crexx_prefix/bin/classlib" "$crexx_prefix/bin/rxfnsg"
"$crexx_prefix/bin/rxvm" embeddings_linked -a auto \
  "$model_dir/bge-small-en-v1.5-f16.gguf" \
  f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999
```

On Windows use `& "$crexxPrefix/bin/rxc.exe"` and the equivalent `.exe` names,
PowerShell continuation syntax, and the same arguments. Use the product `rxvm`
for ordinary execution; maintainers also qualify each concrete VM that is built.

## Adapting the programs

Replace the fixed text/prompt arrays with your input source; keep configuration,
runtime, model and session creation outside the batch loop. Close each request
after capturing the owned output. Choose a bounded queue and number of workers
that fit total private context memory. Local operation and worker-barrier waits
have a wide ten-minute hang backstop. Shared-worker scopes have no whole-workload
deadline: they join the finite batch workload with `collectall(pool, -1)`.
These are functional examples, not provider performance guarantees. The QA
harness runs model-bearing programs one at a time with a thirty-minute process
watchdog; a timeout is a hang/backstop finding to diagnose. Check the
[status matrix](../qualification.md) before
claiming a new platform works.

For embedding queries use role `query`; do not duplicate the provider's query
prefix. Keep the model/preparation identity with SQLite blobs or sidecars. For
generation consume each chunk once, retain token deltas separately from text
length, and handle every finish reason. To cancel, call `request.cancel()`
between process calls, capture remaining output/diagnostics and close the
request. A work-token budget is not a GPU preemption deadline.

The shared allocation controls are functional demonstrations, not speed or
model-quality benchmarks. Indicative memory and glue evidence is summarized
in [qualification](../qualification.md); model selection, retrieval quality,
conversation policy and application scheduling remain application concerns.
