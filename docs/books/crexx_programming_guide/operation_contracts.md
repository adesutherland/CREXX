# Operation Contracts

`crexx-contract` exports a compiled Level B operation as a deterministic,
machine-readable JSON contract. It is a build-time integration tool: it does
not add runtime reflection, change RXBIN, publish a C ABI, or prescribe an RPC
transport.

The public boundary is the emitted `crexx.operation-contract/1` document. The
current exporter obtains type facts from CREXX's private RXBIN semantic graph,
but consumers must not depend on that graph or its numeric IDs. A future
annotation or reflection facility may replace the metadata adapter while the
contract document and consumers remain unchanged.

## Source Convention

Declare the operation and its payloads as Level B interfaces. The operation is
one abstract interface method. Each abstract, zero-argument method on a payload
interface is one read-only field:

```rexx
options levelb
namespace example_contract expose operationcontract requestcontract resultcontract errorcontract

operationcontract: interface
  execute: method = .resultcontract
    arg request = .requestcontract, correlation = ""

requestcontract: interface
  query: method = .string
  tags: method = .string[]

resultcontract: interface
  ok: method = .boolean
  problem: method = .errorcontract

errorcontract: interface
  code: method = .string
  message: method = .string
```

Payload factories, concrete classes, default/final methods, and payload methods
with arguments are rejected. Concrete classes may implement the interfaces at
runtime, but their private attributes are never exported.

## Command Line

Compile and assemble the source, then identify the operation and its
supplemental contract facts explicitly:

```console
rxc -x -o example_contract example_contract.crexx
rxas -o example_contract example_contract.rxas
crexx-contract \
  --rxbin example_contract.rxbin \
  --operation example_contract.operationcontract.execute \
  --contract-version 1.0.0 \
  --nullable example_contract.resultcontract.problem \
  --optional-field example_contract.errorcontract.details \
  --error example_contract.errorcontract \
  --output example_contract.rxcontract.json
```

`--nullable TYPE.FIELD`, `--optional-field TYPE.FIELD`, and `--error TYPE` are
repeatable. Every named field must be reachable from the operation or an error
type; a misspelling fails the command. Optional operation arguments come from
their compiled Level B default declarations. Omission and JSON `null` are
independent: fields and required inputs are present and non-null unless stated
otherwise. Format 1 does not make operation inputs or the result nullable.

Use `--previous old.rxcontract.json` to enforce the evolution rules before the
new output is written. Both versions must use `MAJOR.MINOR.PATCH`.

## Installed CMake Helper

An installed CREXX package provides `CREXX::crexx-contract` and
`crexx_add_operation_contract()`:

```cmake
find_package(CREXX CONFIG REQUIRED)

crexx_add_operation_contract(
        TARGET example_operation_contract
        RXBIN "${CMAKE_CURRENT_BINARY_DIR}/example_contract.rxbin"
        OPERATION example_contract.operationcontract.execute
        CONTRACT_VERSION 1.0.0
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/example_contract.rxcontract.json"
        PREVIOUS "${CMAKE_CURRENT_SOURCE_DIR}/released.rxcontract.json"
        NULLABLE example_contract.resultcontract.problem
        OPTIONAL_FIELDS example_contract.errorcontract.details
        ERROR_TYPES example_contract.errorcontract)
```

`TARGET`, `RXBIN`, `OPERATION`, `CONTRACT_VERSION`, and `OUTPUT` are required.
`PREVIOUS` is optional. The helper creates the named build target and sets
`<TARGET>_OUTPUT` in the caller's scope. The RXBIN may itself be the output of a
custom command; the generated contract then follows that dependency without a
source-tree fallback.

## Format 1

Every UTF-8 document has these top-level members, in deterministic order:

| Member | Meaning |
| --- | --- |
| `format` | Always `crexx.operation-contract` |
| `formatVersion` | Always `1` for this grammar |
| `contractVersion` | Caller-owned semantic version for the operation |
| `operation` | Fully qualified interface and method identity |
| `input` | Ordered operation parameters with `required`, `nullable`, and `schema` |
| `result` | Result schema |
| `errors` | Sorted structured error schemas |
| `types` | Sorted closed set of referenced interface-record definitions |

Record fields are sorted by their UTF-8 names. Operation inputs retain call
order. Optimized and non-optimized builds of the same declarations produce the
same document.

The format-1 type mapping is:

| Level B type | JSON contract schema |
| --- | --- |
| `.string` | `{"kind":"string"}` |
| `.boolean` | `{"kind":"boolean"}` |
| `.int` | `integer` with signed 64-bit minimum and maximum |
| `.float` | finite `number`; NaN and infinities are outside the contract |
| `.decimal` | `string` with `encoding: "crexx-decimal"` to retain exact spelling |
| `.binary` | `string` with `contentEncoding: "base64"` |
| `.T[]` | ordered homogeneous `array` with the recursively mapped item schema |
| payload interface | `record` referring to its fully qualified type name |
| `.void` | `void`, for an operation result only |

Only one-dimensional dynamic arrays are accepted. Fixed, bounded, or
multidimensional arrays; references/exposed parameters; varargs; `.object`;
`.unknown`; concrete payload classes; and ambiguous relative type names fail
closed. Interface inheritance is flattened into the derived record and
conflicting inherited field definitions are rejected.

## Evolution Rules

`formatVersion` changes only when the JSON document grammar changes.
`contractVersion` describes one operation:

- removing or renaming an input, field, result, or error; changing a type;
  making optional data required; or making nullable data non-nullable requires
  a major-version increase;
- adding an optional input or field, adding an error type, or loosening
  required/nullability constraints requires a minor-version increase;
- documentation-only clarification may use a patch increase;
- deterministic regeneration of an unchanged contract may retain the same
  version, but versions may never move backwards.

The document describes typed input, result, and structured error payloads. It
does not define HTTP status codes, an invocation envelope, ownership of a
transport buffer, or runtime error dispatch; those remain responsibilities of
the selected host adapter.
