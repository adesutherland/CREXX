# Packed vectors with `rxvector`

`rxvector` is the process-reentrant Level G standard/default native provider
for exact vector computation over `.packedfloat` and `.packedint` owners. It
borrows host-native payloads read-only for each call and retains no pointer,
matrix, index, or mutable provider state.

| Procedure | Result |
|---|---|
| `decodef32le(data = .binary)` | Owning `.packedfloat`, widened from canonical little-endian IEEE binary32. |
| `encodef32le(values = .packedfloat)` | Owning canonical little-endian IEEE binary32 `.binary`. |
| `cosine(left = .packedfloat, right = .packedfloat)` | Exact native-float cosine similarity. |
| `topkcosine(vectors, identities, dimensions, query_vector, requested, result_identities, result_scores)` | Deterministic bounded top-k into exposed packed result owners. |

```rexx
options levelg floats_binary
import rxfnsg
import rxvector

vectors = .packedfloat(4)
call vectors.set(0, 1.0)
call vectors.set(3, 1.0)

identities = .packedint(2)
call identities.set(0, 41)
call identities.set(1, 42)

query_vector = rxvector..decodef32le("0000803F00000000"x as .binary)
result_identities = .packedint(0)
result_scores = .packedfloat(0)
call rxvector..topkcosine(vectors, identities, 2, query_vector, 1, result_identities, result_scores)
say result_identities.get(0) result_scores.get(0)
```

## Matrix and ordering contract

`topkcosine` treats `vectors` as a zero-based row-major matrix with
`dimensions` floats per row. The inferred row count must equal
`identities.size()`, and `query_vector.size()` must equal `dimensions`.
`requested` is between zero and the row count inclusive.

Results order by score descending, then identity ascending, then source row
ascending when both preceding values are exactly equal. The identities are
caller values, not row indexes. A request of zero succeeds with empty outputs.
Both output owners must be initialized; a failed call resets both outputs to
empty before signalling.

## Numerical and error contract

Cosine accumulation uses a block-compensated ordinary path and a scaled,
compensated fallback when raw products or squared norms overflow or underflow.
Scores are clamped only to the mathematical `[-1, 1]` bound after finite
calculation. This is exact full-scan CPU search, not approximate nearest
neighbor search.

Inputs must be finite. Cosine operands are nonempty, equal length, and
non-zero norm. Top-k requires at least one complete row, positive dimensions,
matching shapes, and non-zero norms. Invalid shapes and values signal
`INVALID_ARGUMENTS`; invalid requested counts signal `OUT_OF_RANGE`;
uninitialized owners signal `OBJECT_NOT_INITIALIZED`; unrepresentable finite
calculations or float32 narrowing signal `OVERFLOW_UNDERFLOW`; allocation or
result-publication failures signal `FAILURE`.

`decodef32le` accepts empty input but rejects partial items and non-finite
float32 values. `encodef32le` accepts an empty initialized owner but rejects
non-finite values, float32 overflow, and nonzero values that underflow to zero.

## Persistence and deployment

`.packedfloat` and `.packedint` are host-native computation owners, not file or
wire formats. Persist portable vectors as explicit `f32le` bytes with the
codec, element count, dimensional meaning, and application model/profile
metadata held by the owning schema. Convert in bounded pages when a dataset is
larger than the intended working-memory envelope.

RXBIN provider metadata selects `rxvector.rxplugin` automatically for ordinary
execution and `rxvector.a` or the platform-equivalent archive for
`crexx -native`. No Rexx declaration wrapper, explicit plugin list,
initializer, or VM-hard-coded function is required. Prepared matrices,
persistent handles, ANN indexes, BLAS and external vector databases require a
separate lifecycle and compatibility design.
