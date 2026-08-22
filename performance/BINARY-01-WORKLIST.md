# BINARY-01 native packed numeric storage

Status: Release 1 Level B surface implemented and first Release verdict
accepted; initial explicit Level G owner classes and their approved getter
rework are implemented and performance-qualified by the accepted rework
Release verdict; automatic packed ordinary numeric arrays remain a
post-release Level G roadmap item.

Approved by Adrian: 2026-08-20.
Explicit-buffer selection approved by Adrian: 2026-08-21.
Exact Release 1 Level B surface approved by Adrian: 2026-08-21.
Initial Level G owner-class surface approved by Adrian: 2026-08-21.
Level G transient-borrow, imported-accessor and binary-reference design
approved by Adrian: 2026-08-21.

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

## Selected Release 1 Level B contract

1. `.binary` remains the sole owning value and the sole native-provider
   payload type. There is no packed base type, compiler packed flag, descriptor
   or new RXBIN file metadata.
2. `<packed..int>(index) value` and `<packed..float>(index) value` read one
   native item; the same expressions are writable assignment targets.
3. Item indexes are zero-based. Index `2` denotes the third item. `<blen>` and
   `binresize` continue to count bytes, so callers explicitly allocate
   `count * <sizeof..int>` or `count * <sizeof..float>` bytes.
4. Element width, byte order, representation and alignment are exactly those
   of the current host type. There is no per-element endian conversion,
   narrowing, widening, byte-position scaling, or format selection.
5. Every ordinary runtime-owned `.binary` base is aligned sufficiently for
   both `rxinteger` and `rxfloat`. Elements are contiguous with no
   application-visible padding. Access uses alias-safe native loads/stores.
6. The access spelling supplies the interpretation: the same bytes may be
   inspected as native integers or native floats. No hidden kind tag exists.
   Numeric conversion remains an explicit element-by-element operation.
7. A negative index, index multiplication overflow, or complete item extending
   beyond the current logical byte length raises `OUT_OF_RANGE`. Stores do not
   resize the value and signal before changing it.
8. Only `int` and `float` are accepted after `packed..`. String, object,
   decimal, fixed-width encoded forms and every other suffix are compile-time
   errors and use the existing raw/interchange or ordinary value route.
9. The representation must not be serialized or transferred to another host
   as if it were portable. Export and import use the explicit raw/interchange
   route.

Example:

```crexx
values = .binary
call binresize values, count * <sizeof..float>
<packed..float>(0) values = 100.0
first = <packed..float>(0) values
third = <packed..float>(2) values
```

RXAS owns four explicit item operations: `pgeti`, `pseti`, `pgetf` and
`psetf`. Their index operand counts native items, not bytes. They operate on
the existing binary register component and therefore require no RXBIN format
revision.

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

## Design selection

The Release 1 production selection starts from these considered designs:

1. **Status quo: encoded binary access.** Continue using `<at..i64>` and
   `<at..f64>` at byte offsets. Rejected for native numerical kernels because
   it retains endian assembly/conversion semantics and obscures item indexing.
2. **Selected: explicit `.binary` plus native item access.** Add the four
   `packed` intrinsics and RXAS operations above. This keeps ownership,
   lifetime, copying, task transfer and RXPA payload handling on the existing
   `.binary` path while exposing the exact host representation to Level B.
3. **Distinct packed types or validated view objects.** Rejected for Release 1:
   they add type/descriptor ownership, conversion and RXBIN/ABI questions
   without improving the steady-state scalar ceiling.
4. **Automatic native layout for ordinary numeric arrays.** Deferred to
   post-release Level G because indexed reference identity, dynamic growth and
   boxed/generic array compatibility are language-wide concerns.

The machine ceiling is one bounds check plus one alias-safe native scalar load
or store. A compiler may later remove a proved redundant check through the
normal effects/signal framework; the Release 1 instruction itself remains
strict and general.

## Level boundary

This explicit-buffer surface is guaranteed at Level B and therefore also
usable by Level G libraries. It is the bootstrapping-compatible mechanism for
native numerical storage. Level G after Release 1 may make ordinary `.int[]`
and `.float[]` arrays use the same packed runtime representation automatically;
that later convenience must not create a second storage implementation.

## Selected initial Level G owner classes

Release 1 Level G provides explicit `.packedfloat` and `.packedint` owner
classes over the same Level B storage and instructions. They improve ordinary
application ergonomics without adding another representation or changing
ordinary array semantics:

```rexx
import rxfnsg

floats = .packedfloat(3)
call floats.set(2, 100.0)
third = floats.get(2)

integers = .packedint.fromBinary(native_bytes)
call integers.fill(0)
```

Both classes expose the same zero-based contract:

- `*(size)` allocates and zero-fills `size` host-native items;
- `fromBinary(data)` owns a copy and rejects a partial final item;
- `size()`, `get(index)`, `set(index, value)`, `resize(size)`, and
  `fill(value)` operate in item units; and
- `binary()` returns a weak `reference .binary` to the owner value's binary
  component. Source `local = dereference reference_value` is a scoped live
  alias and does not copy the bytes; `snapshot reference_value` is the explicit
  owned-copy operation.

The object's bytes are stored directly in `register.0.binary`. A native
provider that declares the corresponding class argument can therefore bind
the object payload directly; a Rexx-level `binary()` conversion or reference
deref is not part of the provider call path. The returned reference is mutable
and does not retain the owner. The packed object must outlive every use of the
reference, and native code must not retain the current raw byte pointer across
a call or resize. Callers that need an independent ordinary binary value use
`snapshot object.binary()`.

These classes do not add bracket indexing, references to individual items, a
packed object/string form, or automatic array conversion. Custom `x[index]`
indexing and automatic packed storage for ordinary `.int[]`/`.float[]` arrays
remain post-release language work.

## Approved Level G accessor implementation design

The initial wrapper pilot exposed a general language-lowering issue rather
than an acceptable class-library cost. An imported `get()`/`set()` body with a
binary class attribute did not carry consumable inline metadata, and a packed
read through the complex `register.0.binary` view detached the entire binary
before loading one item. The diagnostic two-million-item float pilot observed
36,333,823 us for wrapper reads versus 11,937 us for direct Level B reads. It
is diagnostic evidence, not the formal verdict, but it rejects full-buffer
materialisation as the wrapper implementation.

The approved alternatives and selection are:

1. **Status quo value materialisation.** Keep the method call and full binary
   snapshot before each packed read. Rejected because its cost grows with the
   complete container for an operation whose semantic result is one scalar.
2. **RXAS-only linked-read cleanup.** Recognise the emitted
   `link`/`bcopy`/`unlink`/packed-operation sequence after source emission.
   Retained as a possible authored-RXAS backstop, but rejected as the sole
   implementation because it does not make the imported method body available
   to the caller inliner and lacks the source-level component-view contract.
3. **Selected: compiler-owned transient component borrow plus exact imported
   accessor inlining.** A class attribute used only as the storage base of a
   binary-memory operation is borrowed for that operation when its exact
   component effects make the access safe. `register.0.binary` lowers directly
   to the receiver. A child attribute may use a scoped `linkattr`/`unlink`, but
   no detached binary copy. The inliner transports the exact non-array binary
   register-0 view and recognises structural packed getter/setter bodies in its
   exact-accessor lane. Ordinary value reads and returns retain copy semantics;
   a borrow never escapes implicitly.
4. **Packed-class-specific native or opcode bypass.** Rejected. The public
   Level G class remains ordinary cREXX over the existing four Level B packed
   operations, and the proof is expressed in terms of storage view, consumer
   effects and lifetime rather than class names.

The hot-path ceiling remains one checked alias-safe scalar `memcpy` inside
`pgeti`/`pgetf` or `pseti`/`psetf`. Optimized direct-receiver access may retain
the receiver-initialization guard and source/TRACE metadata, but it must not
copy the complete binary, create an escaping implicit alias, or execute a
method call at an exact imported direct-receiver site.

After minimum focused source/RXAS/RXDAS/binary-import and dual-VM correctness
checks, implementation freezes for the mandatory ordinary profiling-off
Release wrapper-versus-direct verdict. Broad Debug, sanitizer, install and
cross-platform closeout remain at the consolidated end of RCC-5.

### Focused implementation checkpoint (2026-08-21)

The approved implementation is frozen for its first Release verdict:

- ordinary no-opt packed methods emit `pget*`/`pset*` directly against receiver
  argument `a1`; optimized source and binary imports emit those instructions
  against the caller's receiver register;
- the optimized imported hot path contains no accessor call and no binary
  `bcopy`, `link`, or `unlink`; a getter retains only its scalar result copy and
  a setter writes the receiver directly;
- the exact `register.0.binary` layout and packed accessor body survive RXAS,
  RXBIN, RXDAS and reassembly before binary-only import;
- an explicit reference to a physical class component now captures the
  underlying owner/component locator rather than a detached temporary. This is
  covered independently by a `register.0.int` reference regression as well as
  `.packedfloat.binary()` and `.packedint.binary()` live-borrow tests; and
- the focused Debug checks pass in no-opt/opt modes and on both concrete VMs,
  including live mutation through `dereference`, detached `snapshot`, bounds,
  resize/copy, direct class storage and imported packed accessors.

The profiling-off Release wrapper/direct comparison remains pending and is the
next mandatory stop. These focused results are not the consolidated RCC-5
Debug/sanitizer/install/cross-platform closeout.

### First Level G Release verdict (2026-08-21; decision pending)

The retained 20-warmup/240-sample balanced Release block is in
`performance/evidence/2026-08-21-binary01-packed-class-first-release-verdict/`.
All executions passed on both concrete VMs with no thermal/performance warning
and no excluded sample.

- Packed integer remains decisively faster than encoded `i64`: 1.744x-2.279x
  across read/write and both VMs, winning all 48 pairs.
- Inlined `.packedfloat`/`.packedint` setters are 1.448x-1.970x faster than the
  direct source comparator and win all 48 wrapper/direct write pairs.
- Inlined getters no longer copy the container, but remain slower than direct
  packed reads: paired ratios are 0.403x-0.512x for float and 0.738x-0.848x for
  integer. Wrapper read throughput is therefore 15.217%-59.683% lower.

Recommendation: retain the accepted packed primitives and current no-copy
storage/reference model, but rework exact getter result-register coalescing and
proved receiver-initialization guard elimination/hoisting before qualifying the
Level G wrappers for performance. Production implementation remains frozen at
the mandatory verdict stop pending Adrian's accept/rework decision.

### Approved exact-getter rework (2026-08-21)

Adrian accepted the rework recommendation and authorized the required Release
measurements on a clear host. The implementation remains deliberately bounded
to the generic inliner mechanics exposed by the packed classes:

1. A single final scalar packed load in an inlined block expression may donate
   its temporary register to the block result. This removes the otherwise
   redundant `icopy`/`fcopy`; it is allowed only when there is one direct final
   `LEAVE_WITH`, the value is the packed load itself, and no crossed runtime
   cleanup can invalidate that register.
2. A direct final `LEAVE_WITH` falls through to its immediately following
   block-end label after performing the same child, crossed-scope and signal
   cleanup. Only the redundant unconditional branch is omitted; early and
   nested exits retain it.
3. The method-entry `assertinitialized` remains the default and remains
   mandatory for an unknown or possibly uninitialized receiver. It may be
   omitted only when a conservative caller-side dominance/single-write proof
   establishes that the exact receiver local was produced by a successful
   factory expression before the call. The existing uninitialized-receiver
   signal tests remain the negative control.
4. Correctness is rechecked after each independently measurable change. The
   final accepted implementation receives a new balanced profiling-off Release
   wrapper/direct verdict; broad Debug, sanitizer, install and cross-platform
   closure remain at the consolidated RCC-5 gate.

### Rework Release verdict (2026-08-21; accepted 2026-08-22)

The retained verdict is in
`performance/evidence/2026-08-21-binary01-packed-class-rework-release-verdict/`.
It repeats the original 20-warmup/240-sample balanced Release block on the
same program and workload. All 260 executions passed, no sample was excluded,
and the host reported no thermal or performance warning before or after.

- Exact getter reads now emit one `pgetf`/`pgeti` feeding the consumer: there
  is no accessor call, binary copy/borrow, scalar handoff copy, block-end branch
  or receiver guard on the proved initialized hot receiver.
- Unknown/uninitialized receivers retain `assertinitialized`; the focused
  source and binary-import test raises and catches `OBJECT_NOT_INITIALIZED` as
  the negative control.
- Wrapper/direct paired read ratios are 1.007x and 1.033x on `rxbvm`, and
  1.015x and 1.004x on `rxtvm`, for float and integer respectively. Every read
  wrapper wins eight of 12 pairs.
- Wrapper writes remain 1.652x-2.169x faster and win every pair. Direct packed
  integer remains 1.779x-2.233x faster than encoded `i64`.

Adrian accepted the generic result-coalescing and conservative
receiver-initialization proof as the BINARY-01 baseline on 2026-08-22. The
initial Level G packed owners are performance-qualified. Consolidated RCC-5
broad QA remains separate.

## Post-release Level G numeric-array roadmap

After Release 1, review making ordinary `.int[]` and `.float[]` arrays use the
native packed representation automatically. That later gate must settle
numeric-array initialization, indexed `arg expose` and reference identity,
dynamic growth, generic RXVML/RXPA array compatibility, task transfer and the
migration of code that currently depends on boxed numeric elements. It should
reuse the Release 1 packed descriptor, item RXAS operations and native span ABI
rather than create a second runtime implementation.

## Required evidence and exit gate

- Exact `sizeof`, alignment and representation assertions on every supported
  compiler/platform.
- Empty, single-element, boundary, negative/overflow, wrong-suffix, alias,
  mutation, resize/copy, lifetime, task-transfer and native-provider tests.
- Optimized/no-opt and both concrete VMs.
- A direct machine ceiling proving one native load/store in the accepted hot
  path, plus startup, RSS and artifact costs.
- A focused Release comparison of `.packedfloat`/`.packedint` `get` and `set`
  against direct Level B packed access, plus packed integer against encoded
  `i64`, before the Level G wrappers are treated as performance-qualified.
- Cross-platform proof that the native representation is rejected or converted
  at every persistence, process-transfer or incompatible-host boundary.
- The mandatory first profiling-off Release verdict before broad closeout.
