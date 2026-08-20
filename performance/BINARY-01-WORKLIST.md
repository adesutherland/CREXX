# BINARY-01 native packed numeric storage

Status: semantic direction approved; production implementation remains a
separate language/runtime gate.

Approved by Adrian: 2026-08-20.

## Purpose

Provide the maximum-performance packed representation for the two native
cREXX scalar numeric types without turning arbitrary byte storage into a
second family of partially portable numeric encodings.

The fast representation is exactly one of:

- packed `rxinteger`: a contiguous sequence of the host representation used
  by the Release 1 `.int` value; or
- packed `rxfloat`: a contiguous sequence of the host representation used by
  the Release 1 `.float` value (`double` in the current runtime ABI).

It is not a file, wire, persistence, or cross-host interchange format.

## Locked semantic boundary

1. A native packed value has one element kind for its complete lifetime:
   `rxinteger` or `rxfloat`.
2. Element width, byte order, representation and alignment are exactly those
   of the current host type. There is no per-element endian conversion,
   narrowing, widening, byte-position scaling, or format selection.
3. Elements are contiguous with no application-visible padding between them.
   The storage base satisfies the native alignment of the selected type.
4. Construction or view binding validates element kind, extent, alignment,
   ownership and host compatibility once. The selected machine ceiling is one
   alias-safe native load or store per element after applicable bounds proof;
   repeated raw-format checks fail the design gate.
5. A value cannot be reinterpreted between packed integer and packed float.
   Conversion creates a new value and performs explicit element conversion.
6. The representation must not be serialized or transferred to another host
   as if it were portable. Export and import use the explicit raw/interchange
   route.

## Raw/interchange route

Existing typed binary-memory operations remain the compatibility route for
arbitrary `.binary` values. This route owns:

- zero-based byte positions;
- explicit encoded widths such as `i16`, `i32`, `i64`, `f32`, and `f64`;
- canonical or explicitly selected endianness;
- integer range checks and float width conversion;
- unaligned fields, protocol records, files, persistence, and wire formats.

Encoded `f32` is therefore not packed `rxfloat`: it converts between IEEE
binary32 storage and the VM's native binary64 `.float`. Encoded `i64` is also
not packed `rxinteger`, even though Release 1 currently defines both as signed
64-bit values, because the encoded form has a portable byte-order contract
rather than a host-native representation contract.

## Performance consumers

The first intended bulk consumer is the separately qualified native `rxstats`
provider. Future `rxvector` CPU and accelerator providers build on the same
boundary, while retaining a portable Level B oracle and explicit encoded
import/export formats.

Dependency order:

1. settle and implement native packed storage;
2. qualify packed `rxstats` algorithms and ownership;
3. design the later vector surface and optional native backends.

Scalar RCC-5 mathematics does not wait for this work. The current boxed-array
statistics procedures are compatibility evidence only and must not be promoted
as the final high-performance `rxstats` contract.

## Remaining language decisions

The representation decision above is final. Before a production edit, one
coherent language decision must still select:

1. zero- or one-based native element indexing;
2. the source spelling for native packed construction, views, indexing and
   conversion;
3. whether the owning value is a distinct packed type or a validated typed
   view over aligned binary-owned storage;
4. mutability, slicing and alias/copy rules; and
5. the migration treatment of existing `<at..type>` raw byte syntax.

None of those decisions may broaden the native fast path beyond `rxinteger`
and `rxfloat`. A request for another width, endian, layout or representation
uses the raw route or opens a later separately named design.

## Required evidence before production selection

- Exact `sizeof`, alignment and representation assertions on every supported
  compiler/platform.
- Empty, single-element, boundary, misalignment, wrong-kind, alias, mutation,
  slice, copy, lifetime, task-transfer and native-provider tests.
- Optimized/no-opt and both concrete VMs.
- A direct machine ceiling proving one native load/store in the accepted hot
  path, plus construction, view-binding, startup, RSS and artifact costs.
- Cross-platform proof that the native representation is rejected or converted
  at every persistence, process-transfer or incompatible-host boundary.
- The mandatory first profiling-off Release verdict before broad closeout.
