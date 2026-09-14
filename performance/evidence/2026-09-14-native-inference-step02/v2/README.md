# S2-D01 version 2 correctness evidence

Adrian approved this numerical policy on 2026-09-14, preserving batching and
GPU performance and excluding upstream determinism work. Authority remains the
[STEP-02 record](../../../../docs/planning/native-inference-step-02.md).
Version 1 results remain unchanged in the parent bundle.

`identity.json` retains exact commands, model/source/binary/reference hashes,
build scope and validation results. `control-v1-to-v2.patch`, reversed against
the identified version 2 source, reconstructs the original direct workload;
its SHA-256 must match the parent identity. The original upstream libraries
and pinned BGE F16 model remain unchanged. No provider implementation exists.

The Release CPU and Metal controls both pass 100 repeated single requests,
20 eight-row batches and four private contexts sharing one model. Additional
four-row controls distinguish batch-layout variation from repeatability.
JSON records all vectors and comparison limits/counts/worst values; logs retain
the ordinary PASS result and backend execution. Every same-layout repeat and
private-context comparison has zero observed coordinate difference.

| Comparison | Maximum coordinate difference | Minimum cosine | Result |
| --- | ---: | ---: | --- |
| CPU single versus eight-row batch | 0.000765192 | 0.999989958 | Pass v2 |
| Metal single versus eight-row batch | 0.000399479 | 0.999998319 | Pass v2 |
| CPU versus Metal, four-row batch | 0.000727215 | 0.999989540 | Pass v2 |
| CPU versus Metal, eight-row batch | 0.000725016 | 0.999989430 | Pass v2 |

The cheap CTests pass 2/2 in Release and 2/2 in Debug. The Debug build covers
the QA executable; linked upstream libraries remain Release. The tripwire
accepts the positive numerical controls and rejects deliberate coordinate and
distributed-direction drift, nonfinite values, zero norm, wrong dimensions and
empty output. Expected rejection diagnostics in `tripwire.log` belong to this
negative-input test; they do not turn real-model failures into passing tests.

`retrieval-illustration.json` retains an exact-search calculation over the
existing control's one query (row 1) and seven document rows. Its algorithm is
specified in the file: dot products of normalized 384-dimensional vectors,
descending score, stable row-ID tie-break. For all 36 combinations of six
query and six document execution configurations (CPU/Metal, single/four/eight
rows), the complete ranking is unchanged. The two cat passages, rows 4 and 0,
remain first and second. The maximum score change is 0.000733417 against a CPU
batch-eight index queried with the CPU single-input vector. This includes
querying fixed stored vectors from different execution configurations.

This illustration is too small to establish corpus recall, near-tie behavior
or approximate-index stability. Corpus retrieval qualification belongs to the
consuming RAG application. A numerical cosine is not a retrieval-accuracy
percentage. These are correctness checks, not a new timing baseline or
sanitizer/platform qualification; all remaining STEP-02 work stays open.
