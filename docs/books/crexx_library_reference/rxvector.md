# Packed vectors and binary indexes with `rxvector`

`rxvector` is the Level G standard/default C native provider for exact vector
computation. Its original packed-vector procedures remain process-reentrant:
they borrow host-native payloads read-only for each call and retain no pointer
or mutable state. The additional `.vectorindex` interface owns immutable
float32 data and uses session-affine native object calls. Neither route needs
USearch, BLAS or a C++ vector runtime.

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
initializer, or VM-hard-coded function is required. The following immutable
owner supports prepared float32 matrices and portable binary persistence.
ANN indexes, BLAS and external vector databases remain separate capabilities.


## Immutable float32 index

Import `rxfnsg` and `rxvector`. This is exact full-scan cosine search with a
bounded result heap; storage stays float32 and accumulation uses double. It
adds no model, database, identity or application policy. The original packed
APIs and their wider numerical contract above are unchanged.

| Surface | Contract |
| --- | --- |
| `.vectorindex(data, dimensions, labels, metadata)` | Own a row-major `f32le` binary matrix. Labels are a `.string[]`, one per row; metadata is an opaque string. |
| `rxvector..decodeindex(data)` | Decode the provider's binary format or signal an error. |
| `rxvector..openindex(data, index, error)` | Checked decode: return 0 and clear error on success; return -1 with error on failure. Clear any previous output owner first. |
| `index.encode()` | Return an independent `.binary` encoding. |
| `index.rows()`, `index.dimensions()` | Matrix shape. |
| `index.metadata()`, `index.label(row)` | Opaque text, preserving UTF-8 and embedded NUL. Label rows are zero-based. |
| `index.search(query, count, keys, scores)` | Query is a `f32le` binary vector. Exposed outputs are initialized `.packedint` and `.packedfloat` owners. Return zero-based row indexes and cosine scores. |
| `index.close()` | Release this object's reference; repeat close succeeds. |

```rexx
labels = .string[]
labels[1] = "first"
labels[2] = "second"
index = .vectorindex("0000803F00000000000000000000803F"x as .binary, 2, labels, "example")
keys = .packedint(0)
scores = .packedfloat(0)
call index.search("0000803F00000000"x as .binary, 1, keys, scores)
say index.label(keys.get(0)) scores.get(0)
encoded = index.encode()
copy = index
call index.close()
say copy.rows()  /* still 2 */
call copy.close()
```

Matrices must have positive rows and dimensions, complete rows, finite values,
and each squared row norm within `[FLT_MIN, FLT_MAX]`. Queries have matching
dimensions and the same finite/norm limits. These bounds preserve the previous
RXVIDX/1 owner contract and reject zero/extreme vectors; use the packed double
API when its wider compensated numerical contract is required. `count` is
positive and clamps to the number of rows. Exact ties order by row ascending.
Failed search clears both result owners before signalling `INVALID_ARGUMENTS`;
allocation or publication failure signals `FAILURE`. Invalid import,
construction, out-of-range label access or use after close signals
`INVALID_ARGUMENTS`; `openindex` instead returns its checked failure.

Assignment shares immutable native storage through reference counting. Closing
one copy does not invalidate another; the last reference releases storage.
There are no retained VM values, borrowed caller buffers or host-service pointers
inside the owner. Session hooks scope current host services to each native call;
finalization needs no live host. The original four stateless procedures retain
their process-reentrant capability declarations.

### Provider-owned RXVIDX/1 format

All lengths/counts are unsigned little-endian 64-bit integers. The encoding is:

1. Eight-byte magic `52 58 56 49 44 58 01 00` (`RXVIDX`, version 1, zero).
2. Dimensions, row count, metadata byte length, then opaque metadata bytes.
3. For each row: label byte length, then opaque label bytes.
4. The complete row-major matrix as little-endian IEEE-754 binary32 values.

No trailing bytes are permitted. Import checks bounds and overflow before
allocation/access and validates the numeric matrix before publishing an owner.
The owner retains a canonical binary buffer plus label offsets, so encode is
byte-preserving and search does not expand the matrix into doubles. This format
is compatible with the earlier incubated provider; external checksums,
publication, freshness and recovery remain the consumer's responsibilities.

The local implementation/qualification boundary and remaining platform work are
recorded in [RXVECTOR-02](../../planning/rxvector-binary-owner-20260919.md).
