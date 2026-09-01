# CRI-14 generic operation-contract decision

Status: **approved**; implementation active

Date: 2026-07-30

## Decision summary

CREXX does not currently have a supported external surface that completely
describes an operation's typed input, result, and structured errors.

RXBIN 007 already contains a useful internal semantic graph for types,
interfaces, members, signatures, implementations, callables, dispatch and
factory providers. That graph is sufficient to seed a static contract
exporter, but it is not itself the required public contract:

- concrete class attributes remain in raw `META_ATTR` records and are not in
  the graph;
- parameter names survive only inside descriptor text, while the structured
  parameter view exposes only type and reference/optional/vararg flags;
- locally spelled object argument types can remain separate opaque names rather
  than resolving to the canonical declared type;
- version, nullable versus omitted, result/error linkage, documentation and
  evolution policy are absent;
- RexxDoc is source-comment extraction plus a coverage ratchet and is not
  associated with or embedded in the graph;
- Level B runtime introspection reports the type and initialization state of a
  value but cannot enumerate operation or record contracts;
- the installed package contains no graph headers and exports no graph CMake
  target. The copied `librxbin.a` is therefore a private build payload, not a
  supported development API.

The smallest coherent production surface is **Option B**, a build-time
`crexx-contract` exporter that derives types from existing Level B interfaces
and emits one versioned, deterministic JSON contract artifact. It keeps the
RXBIN graph private, adds no runtime hot-path work, publishes no C ABI, changes
neither language syntax nor RXBIN 007, and gives external consumers a durable
machine-readable file.

Option B introduces a public CLI/CMake surface and the new
`crexx.operation-contract/1` JSON document format. Those are explicit design
choices. Adrian approved them on 2026-07-30 and clarified that the emitted
artifact is the long-term public boundary even if metadata acquisition evolves.
RXBIN 007 is consequently the first private adapter, not part of the public
contract identity.

## Exact minimized workload

The retained generic source is
[`CRI14-contract-probe.crexx`](CRI14-contract-probe.crexx). It deliberately
uses the strongest current source convention:

- one typed operation interface;
- typed request, result and error interfaces;
- concrete record-like classes implementing those interfaces;
- scalar, object, homogeneous array and optional argument types;
- a RexxDoc description, parameters, return and structured error statement.

The diagnostic build-tree consumer is
[`CRI14-graph-consumer.c`](CRI14-graph-consumer.c). The installed-package
consumer is
[`CRI14-external-consumer/`](CRI14-external-consumer/).

The exact compile and assembly commands were:

```sh
src=/Users/adrian/CLionProjects/CREXX/performance/evidence/2026-07-29-crexx-rag-integration-ledger/CRI14-contract-probe.crexx
rxc=/Users/adrian/CLionProjects/CREXX/cmake-build-debug/bin/rxc
rxas=/Users/adrian/CLionProjects/CREXX/cmake-build-debug/bin/rxas

"$rxc" -x -n -o /private/tmp/crexx-cri14-probe.qOo5ge/contract-noopt "$src"
"$rxas" -n -o /private/tmp/crexx-cri14-probe.qOo5ge/contract-noopt \
  /private/tmp/crexx-cri14-probe.qOo5ge/contract-noopt.rxas
"$rxc" -x -o /private/tmp/crexx-cri14-probe.qOo5ge/contract-opt "$src"
"$rxas" -o /private/tmp/crexx-cri14-probe.qOo5ge/contract-opt \
  /private/tmp/crexx-cri14-probe.qOo5ge/contract-opt.rxas
```

All four commands returned zero. The optimized and non-optimized RXAS files
are byte-identical:

```text
5962c93a603cab96c07407dd12105ad45a71d230b1e3c6aebc8cd351fe9408f7  contract-noopt.rxas
5962c93a603cab96c07407dd12105ad45a71d230b1e3c6aebc8cd351fe9408f7  contract-opt.rxas
a0f7131db713e5fb5e847b6bafe4e9767ab0905360ef57e45a55404dae48ab26  contract-noopt.rxbin
d6be3bb17c21782c112c233f818793cbbd733f91380d1ab9b9ea82a05eaa3c9c  contract-opt.rxbin
```

The graph consumer was built with no warnings:

```sh
cc -std=c99 -Wall -Wextra -Werror \
  -I /Users/adrian/CLionProjects/CREXX/binutils/include \
  -I /Users/adrian/CLionProjects/CREXX/platform \
  CRI14-graph-consumer.c \
  /Users/adrian/CLionProjects/CREXX/cmake-build-debug/bin/librxbin.a \
  -o /private/tmp/crexx-cri14-probe.qOo5ge/graph-consumer
```

Both RXBIN inputs produced byte-identical structured graph output:

```text
88b20cea477556f583450329f05d55f69bfdc50b45e6655bcb9f43a695f8a4aa  graph-noopt.txt
88b20cea477556f583450329f05d55f69bfdc50b45e6655bcb9f43a695f8a4aa  graph-opt.txt
```

The retained raw directory is
`/private/tmp/crexx-cri14-probe.qOo5ge/`. Its relevant files are:

- `compile.log` — SHA-256
  `46902792107588a0c7ab0b12d7d2d77bf856a2d34bb2c8e66db6b9b1a4e51bbd`;
- `consumer-build.log` — empty, SHA-256
  `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`;
- `graph-noopt.txt` and `graph-opt.txt` — exact graph views above;
- `rxdas-noopt.txt` — SHA-256
  `4db8e504520afe99f23744395c7406e462683efa573e1fd8a67d4ab046720ca3`;
- `scratch-install.log` — SHA-256
  `e491a95211e1209da673b20926e602b92df527b58cba6997cfde85d6518cfd63`;
- `scratch-manifest.txt` — SHA-256
  `8c1a09f769898680a49088e5a8d33f14bef7184b95546f615ff4bf9289902188`;
- `external-configure.log` — SHA-256
  `37eb41727bc5412662e81c5d6073424f3c227af899dbfdb4c58b791c76c75d8c`;
- `fingerprints.txt` — SHA-256
  `763a9c037519a475043e06833b24eff16a484e5fd7659c64099103808c3984ab`;
- `crexx-rag-preservation-audit.txt` — SHA-256
  `fcfabf7ef54cd74f4ea845f9b636e826e904a7e9b5d32932d428338b5a1c3d81`.

This is a compile-time metadata probe, not executable application behavior;
there is no VM-specific result to compare. Both VMs load the same RXBIN graph,
and runtime execution would not fill any of the missing contract semantics.

## Observed metadata boundary

The current graph reports:

```text
types=19 members=10 callables=12 relationships=3 declarations=10
retained_bytes=7625 retained_allocations=20
serialized_facts_bytes=3661 serialized_indexes_bytes=856
```

The exact useful content is:

- four declared interfaces and three declared concrete classes;
- ten interface method declarations;
- three class-to-interface relationships;
- twelve concrete factories/methods and their dispatch links;
- scalar and array spellings;
- the optional flag on `correlation`.

The exact gaps exposed by the same source are:

1. `rxdas -p` reports nine raw `.attr` records, but all three graph class nodes
   have zero declarations. Raw attributes are private storage/register
   metadata, not a public record-field contract.
2. The graph parameter view reports types and flags but no parameter names.
   Names are recoverable only by reparsing the `rxsig1|...` descriptor.
3. `execute` returns canonical
   `cri14_contract.resultcontract`, but its request argument is the separate
   opaque `.requestcontract`. The result's `.boolean` and error return are also
   opaque rather than fully resolved graph identities. An exporter must fail on
   ambiguity and canonicalize these spellings before it can claim a portable
   contract.
4. Optional means an omitted call argument with a default. It does not mean a
   JSON field may be absent, nor that a value may be JSON `null`.
5. The source prose saying “version 1” and its `@param`, `@return`, and
   `@throws` tags do not occur in RXAS, RXBIN, the graph, or `rxdas` output.
6. Nothing links `errorcontract` to `operationcontract.execute`; naming a
   result method `problem` is only an application convention.
7. There is no field/operation stability or compatibility policy in the graph.

## Existing surfaces inventory

| Surface | What it supports | Why it is not the CRI-14 contract |
| --- | --- | --- |
| Level B types/classes/interfaces | Static arguments, returns, arrays, factories, methods, implements, checked casts | No contract marker, version, nullable/omitted distinction, error declaration, or field-export rule |
| Raw RXBIN metadata | Functions, classes, private attributes, interfaces, implementations, members and other runtime/debug records | Serialized/runtime implementation data; private attributes and register indexes are not public payload fields |
| RXBIN 007 semantic graph | Validated typed relationships and callable/member queries | Omits attributes/docs/version/errors/nullability and currently has unresolved relative type spellings |
| Level B runtime | `typeof(value)`, `initialized(value)`, value/symbol caller-frame helpers and low-level tracing metadata | Value inspection only; no supported enumeration of operation/schema contracts |
| RexxDoc | Extracts `/** ... */`, recognizes common tags in a sample renderer, and ratchets comment/tag counts | No AST association, installed generator, binary embedding, machine schema or compatibility contract |
| `rxdas -g` / `-p` | Human diagnostic graph count and RXAS-style metadata dump | No stable machine output; `-g` is only a count summary |
| Installed development package | RXPA header-only target and build helpers from CRI-07 | No `rxbin.h`, `rxgraph.h`, `rxsignature.h`, or exported graph target |

## Alternatives

### Option 0 — retain the current contract

External consumers continue to receive handwritten application-specific schema
or parse diagnostic output/private RXBIN structures themselves.

- Correctness: cannot state or validate nullability, error linkage, version or
  evolution; private attribute leakage is easy.
- Compatibility/ABI: no new CREXX commitment, but every consumer invents its
  own unstable one.
- Time/memory: no CREXX implementation cost or runtime overhead.
- Maintenance/platform: duplicated consumer logic and source-tree coupling;
  the installed-package probe already fails.

This is viable only as a deliberate decision that CREXX will not provide the
requested capability. It does not meet the functional intent of CRI-14 and is
not recommended.

### Option A — publish the RXBIN graph C SDK

Install `rxbin.h`, `rxgraph.h`, `rxsignature.h`, the required transitive
headers/library, and an imported `CREXX::RXBIN` target. Document the current
query API as the public external surface.

- Correctness: gives consumers the useful graph but still lacks public record
  fields, version, nullable/omitted semantics, error linkage and evolution.
  Each consumer must add conventions and repair/resolve type spellings.
- Compatibility/ABI: immediately turns the C structs, ownership rules,
  functions and static-library behavior into a public SDK commitment. Dynamic
  packaging later would add a binary ABI obligation.
- Time/memory: linear graph traversal; no VM hot-path change. The probe graph
  is 7,625 retained bytes plus 4,517 serialized graph bytes.
- Maintenance/platform: every consumer needs C integration and must track graph
  evolution. Other languages need bindings.

This can be a future low-level SDK, but it is not by itself a complete generic
operation contract and is not recommended as the CRI-14 surface.

### Option B — static versioned contract artifact (recommended)

Add an installed `crexx-contract` build tool and
`crexx_add_operation_contract()` CMake helper. The tool reads a compiled RXBIN
through the private graph library and emits one deterministic UTF-8 JSON file
with suffix `.rxcontract.json`.

The public invocation is:

```sh
crexx-contract \
  --rxbin contract.rxbin \
  --operation cri14_contract.operationcontract.execute \
  --contract-version 1.0.0 \
  --nullable cri14_contract.resultcontract.problem \
  --error cri14_contract.errorcontract \
  --output contract.rxcontract.json
```

`--nullable TYPE.FIELD`, `--optional-field TYPE.FIELD`, and `--error TYPE` are
repeatable. Optional operation parameters come from the compiled signature;
nullability is always explicit and separate. The command fails rather than
guessing when a type is ambiguous, a field path is absent, a payload interface
contains a non-field member, or an unsupported type shape is encountered.

The emitted top-level document is the new public format
`crexx.operation-contract/1` with these required members:

```text
format                 "crexx.operation-contract"
formatVersion          1
contractVersion        caller-supplied version
operation              fully qualified owner and member
input                  named operation parameters and required/nullable state
result                 typed result schema
errors                 zero or more typed structured error schemas
types                  closed set of referenced record/interface definitions
```

Type and record rules for format 1 are:

- operation and payload object types are Level B interfaces;
- each abstract, zero-argument payload-interface method is one read-only field;
  default/final methods, factories and methods with arguments are rejected on a
  payload interface;
- field identity is its method name; concrete implementation classes and their
  private attributes are never exported;
- `.string` maps to JSON string, `.boolean` to JSON boolean, and `.int` to JSON
  integer with the signed 64-bit CREXX limits retained in the schema;
- `.float` maps to a finite JSON number; non-finite values are outside format 1;
- `.decimal` maps to a JSON string carrying the exact decimal spelling so the
  external boundary cannot lose precision;
- `.binary` maps to a base64-encoded JSON string;
- one-dimensional dynamic arrays map to ordered homogeneous JSON arrays and
  recursively apply the item rule;
- fixed/multidimensional/bounded arrays, references, exposed arguments,
  varargs, `.object`, and `.unknown` fail closed in format 1 rather than being
  widened silently;
- fields and required operation parameters are present and non-null by default;
  omission and JSON null are independent flags;
- an error is a separately declared typed payload. The contract document does
  not prescribe transport, status codes or an invocation envelope; a host
  adapter chooses those without changing the CREXX operation schema.

Version and evolution rules are:

- `formatVersion` versions the CREXX document grammar;
- `contractVersion` versions one operation and is required;
- field and parameter names are stable identities;
- removing/renaming a member, changing its type, making optional input
  required, making nullable data non-nullable, or changing result/error meaning
  requires a contract major-version change;
- adding an optional input/field or an additional structured error is minor;
- documentation-only clarification is patch;
- the exporter compares against an optional prior artifact and fails a build
  when the declared version does not permit the observed change.

Compile-time/runtime ownership is explicit:

- generation and compatibility checking are build/package-time operations;
- the RXBIN graph remains internal and its numeric IDs never enter the output;
- external consumers own/read an ordinary JSON file and receive no CREXX heap
  pointers;
- there is no VM reflection API, runtime allocation, payload copy, dispatch,
  ABI or hot-loop change;
- runtime adapters may consume the artifact but are outside format 1.

Implementation consequences:

- first correct canonical relative type resolution in the internal graph and
  retain positive/ambiguous negative tests; this changes graph content, not the
  RXBIN 007 layout;
- add the exporter as generic compiler/binutils tooling, not application or RAG
  native logic;
- retain the generic Level B source probe plus C unit/installed-consumer tests;
- test deterministic output from optimized/non-optimized input, malformed and
  ambiguous contracts, every supported scalar/array/record/error boundary,
  compatibility comparisons, Unicode names/text and clean scratch install;
- document the source convention, CLI, CMake helper, mapping and evolution
  rules.

Correctness is stronger than Options 0/A because the tool validates one closed
typed graph and all supplemental semantics. Compatibility is a versioned JSON
file and CLI, not a C ABI. Time and memory are build-only and linear in the
reachable contract; the probe demonstrates that the existing graph seed is
small. The implementation is portable anywhere the existing RXBIN tools and C
runtime build. This is the recommendation.

### Option C — first-class source annotations and runtime reflection

Design new source annotations for operations, field nullability, errors and
versions; associate RexxDoc with AST declarations; serialize the data in RXBIN;
and expose it through runtime reflection and an external SDK/exporter.

- Correctness: richest single-source model and supports dynamic discovery.
- Compatibility/ABI/format: new language/annotation semantics, a serialized
  graph extension or RXBIN version, reflection behavior and likely a public API.
- Time/memory: metadata grows in every annotated binary and runtime loaders must
  retain/query it; VM startup/memory and strip policy require measurement.
- Maintenance/platform: highest compiler, assembler, linker, VM, debugger,
  tooling and documentation burden.

This may be the long-term reflection architecture, but the current use case
does not justify making it the Release 1 dependency. Option B's emitted file can
remain stable if annotations later replace its CLI metadata inputs.

## Recommendation and exact decision

Approve **Option B exactly as specified**:

- existing Level B interfaces remain the typed source model;
- add build-time `crexx-contract` plus a CMake helper;
- add the public deterministic `crexx.operation-contract/1` JSON artifact;
- use explicit CLI metadata only for version, nullable/optional fields and
  structured error types;
- keep RXBIN 007, language syntax, runtime reflection and public C ABI
  unchanged;
- fail closed outside the format-1 type slice.

The smallest decision Adrian must make is:

> Approve or reject CRI-14 Option B's new CLI/CMake surface and
> `crexx.operation-contract/1` JSON document contract.

No other language, ABI, runtime-reflection or serialized-RXBIN choice is bundled
into that decision.

## Paste-ready continuation prompt

```text
I approve CRI-14 Option B exactly as specified in
performance/evidence/2026-07-29-crexx-rag-integration-ledger/CRI14-CONTRACT-SURFACE-DECISION.md.
Resume the CREXX integration ledger at CRI-14. Implement the build-time
crexx-contract tool, CMake helper, deterministic crexx.operation-contract/1
artifact, internal canonical-type fix, focused generic and installed-consumer
tests, and documentation without adding language syntax, runtime reflection, a
public C ABI, or changing RXBIN 007. Validate the approved format-1 type and
evolution rules, then complete the remaining scratch-installed downstream
proof and final ledger closeout. Keep crexx-rag read-only and stop on any new
design, ABI, serialized-format, or performance decision.
```

## Preservation audit at the stop

`/Users/adrian/CLionProjects/crexx-rag` remains on `main` at
`97cd87e91344d6ac1773a054bd38df23eb128ed2`. Its preserved hashes still match
the prior audits exactly:

```text
tracked diff  97443028cfa8e86624fc5fda9ea5fb33d02f4d7413e5a8a848d92ec365288ef9
index diff    30cea35503c6dc073f3007218b9458f2bc0c28b2c7661327b9144036d5a7c61d
porcelain-v2  02a6e4216ff47be3ce7252be2ccb39157bc45fda316c254f7198648109df14f1
```

No hosted call, credential, normal-prefix install, read-only-checkout write,
stage, commit, stash, reset, clean, push or publication was performed.
