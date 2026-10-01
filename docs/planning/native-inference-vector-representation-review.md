# S4-D02 — Embedding representation and sidecar review

Read-only review requested by Adrian on 14 September 2026. No output-format,
rxvector, SQLite or crexx-rag change is approved or implemented here. S4-D01's
text-length correction is unrelated to this representation choice. The active
[glue probes](native-inference-glue-probes.md) measure the existing representation.

## Numbered intended outcomes

1. **D02-OUT-01:** Establish which layers require float64 and which store float32,
   using current source rather than attributing a noisy timing difference to an
   assumed conversion cost.
2. **D02-OUT-02:** Identify a bounded, portable storage interface if it avoids
   unnecessary widening/narrowing, without silently changing existing packedfloat
   callers, numeric tripwires or application ownership.
3. **D02-OUT-03:** Distinguish the generic reusable vector facilities from the
   application's rebuildable sidecar and SQLite authority.

## Current source trace

The inspected crexx-rag checkout is `/Users/adrian/CLionProjects/crexx-rag`, HEAD
`dd96144bb3689ad4da4a9a43c8df914555402877`, with unrelated dirty product/QA work.
It was read only. No live library, SQLite database, provider or model was invoked.
The owning vector/provider files below were not dirty in that inspection.

| Layer | Actual representation / operation | Source |
| --- | --- | --- |
| llama.cpp embedding output | `const float *`, native float32 | Pinned `llama_get_embeddings_seq`; current bridge normalization loop |
| rxllama request | L2 calculation uses double accumulation; stores normalized `std::vector<double>` | `lib/plugins/llama/bridge.cpp`, `process_embedding` |
| RXPA publication | One `SETNATIVEPAYLOAD` bulk binary copy; no per-coordinate RXPA call | `lib/plugins/llama/rxllama.c`, `RXLLAMA_EMBEDDINGS` |
| `.packedfloat` | Owned contiguous native VM float values, backed by `.binary`; currently eight-byte doubles | `lib/rxfnsg/rexx/packednumeric.crexx:52` |
| crexx-rag provider record | `f32le-v1`, payload length exactly dimension × 4 | `crexx/providers/provider_contract.crexx:282`; `industrial_provider.crexx:599` uses `node_f32_array` |
| SQLite | `embeddings.vector` BLOB; provider payload bound directly | `crexx/application/ragschema.crexx:175`; `ragembedding.crexx:546` |
| Current `.rxvec` sidecar | JSON ANN manifest, float32-LE centroid bytes encoded as hex, cluster membership/IDs/digests; full embeddings stay in SQLite | `crexx/application/ragembedding.crexx:274`–276, `storage: sqlite-embedding-blobs` |
| ANN retrieval | Decode query/centroids and paged SQLite float32 vectors into `.packedfloat`, then `rxvector.topkcosine` | `crexx/application/ragretrieval.crexx:370`, 419–425 |
| Generic conversion boundary | `decodef32le` widens each float32 into double; `encodef32le` narrows finite doubles and writes explicit little endian | `lib/plugins/vector/rxvector.c:386`, 433 |

The sidecar check `centroid_count * dimension * 8` counts **hex characters**, not
eight-byte floats: each four-byte coordinate becomes eight ASCII hex characters.
The sidecar is not currently a compact file containing every document vector.
Its JSON, cluster identities, generation/profile/checksum and recovery rules are
application policy; SQLite is authoritative and the sidecar is rebuildable.

## Assessment and options

Conversion is required by the **existing packedfloat/rxvector interface**, not by
llama.cpp or SQLite. A BGE row occupies 3,072 bytes as double and 1,536 as float32;
eight rows occupy 24,576 versus 12,288 bytes. Widening a raw float32 does not add
model information. Double accumulation for L2 normalization can remain useful
without retaining or transmitting an entire double result array.

1. **Keep the approved packedfloat result.** It directly serves existing
   cREXX/rxvector computation; a storage consumer calls `encodef32le`. This retains
   the current contract but entails a float32 → double → float32 storage route.
2. **Add a separately named normalized f32le binary output** while retaining the
   packedfloat convenience surface. A storage-oriented consumer could bind the
   exact portable bytes to SQLite without first allocating a double matrix.
   Merely narrowing today's stored double result in a second method would not
   eliminate the original widening; the request's internal result policy must be
   considered and checked against existing normalization/numeric semantics.
3. **Extend generic rxvector search to consume float32 buffers directly.** This
   could remove search-time widening too, but is a distinct generic-vector
   design/performance change. It is not necessary to add in-process inference or
   to store f32le vectors. Do not silently bundle it into the llama plugin.

Recommendation: retain the current interface for the authorized diagnostic and
accept any measured cost required by that interface, as Adrian directed. A
separate portable f32le output is a sensible storage-facing addition to review;
there is no inference requirement to insist on doubles throughout the pipeline.
Do not copy the application's current sidecar wholesale into rxllama. If a generic
binary index/sidecar is selected later, keep its storage/search ownership in the
generic vector layer and leave corpus identity/publication policy in crexx-rag.
Neither a sidecar format change nor a float32 search rewrite is authorized here.

## Numbered checkable criteria for any later selected change

1. **D02-AC-01:** Explicit output type, element width, byte order, dimension,
   ordering, ownership and normalization are documented; binary is not silently
   treated as `.packedfloat`.
2. **D02-AC-02:** Existing packedfloat calls and result lifetime remain valid;
   f32le output matches the selected normalize-then-round contract and accepted
   numeric tripwires, including nonfinite and dimension failures.
3. **D02-AC-03:** Direct SQLite BLOB binding/round-trip is checked without model
   calls; representation/model identity compatibility remains explicit.
4. **D02-AC-04:** Measure only changed glue/allocation/copy costs. Existing index
   reuse is not declared safe merely from matching dimensions; sidecar authority,
   generation and profile rules remain the application's responsibility.

## Numbered possible next steps — not implementation authority

1. **D02-01:** Complete the read-only trace and record the current formats. Done.
2. **D02-02:** Finish NI-S4-P01 phase attribution; do not claim conversion explains
   the GPU slowdown without measurements. Adrian conditionally accepts the cost
   of conversion that the retained packedfloat interface requires.
3. **D02-03:** If selected, present the exact additional f32le API and result
   storage/compatibility choice for approval before an output contract change.
4. **D02-04:** Keep any float32-native search or generic sidecar proposal separate,
   with its own bounded acceptance and performance evidence.
