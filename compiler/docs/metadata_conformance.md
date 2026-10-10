# Compiler import metadata conformance

This is the current contract-field boundary for the approved C-first compiler
split. It describes existing metadata, without changing RXBIN 007 or introducing
another serialized format. A producer may be B, G, a native RXPA provider or
another tool that emits the admitted contract. Consumers use its exported ABI
and contract fields, independent of the producer's source grammar.

The reader is [`rxcontract`](../../binutils/include/rxcontract.h), in binutils.
It returns owned type, callable, attribute and class/interface-header records.
It needs neither a compiler Context nor an AST or a B/G source parser.
[`rxcp_contract`](../rxcp_contract.h) separately adapts these records to the
canonical compiler declaration tree and runs ordinary structural/symbol
validation. Type resolution, ABI consistency and supported call semantics remain
mandatory common-backend checks.

The current signed Classic external CALL bridge can consume a G-produced
procedure with the same activation-argument ABI as a B-produced procedure.
This boundary also leaves room for later C-to-G procedure interoperability.
It does not define new Classic argument conversions, authored typed calls or
task invocation semantics; those remain language-design decisions.

## Record envelope and responsibilities

RXAS, RXBIN and RXPA adapters first identify records and their owning module.
The existing assembler, binary loader and native-registration interfaces own
their envelopes. The field reader does not parse RXAS instructions or validate
an arbitrary raw constant pool. Envelope acceptance is not proof that a
declaration's fields conform.

| Record | Required contract information | Consumer responsibility |
| --- | --- | --- |
| `META_FUNC` / RXPA procedure | Qualified identity, result, ordered arguments, provider options | Parse the signature; retain the native-provider identity and ordinary call ABI. Only actual exports become importable definitions. |
| `META_REG` / global | Qualified identity, complete type, dimensions/reference referent | Resolve the complete type; retain module-local metadata-chain ownership in shared pools. |
| `META_CLASS`, `META_INTERFACE` | Namespace and class/interface identity | Publish checked forward declarations before resolving dependent contracts. |
| `META_ATTR` | Owning class, name, complete type, register index/view | Preserve layout; common validation checks the slot/view semantics. |
| `META_MEMBER` | Owner, kind, name, result, arguments, default-method status | Preserve callable kind and interface/default-method identity. |
| `META_IMPLEMENTS` | Class and interface qualified identities | Retain and resolve relationships; provider matching uses the common contract checks. |
| `META_TASK_TARGET` | Existing task-callable identity and target | Preserve the marker. The consuming language's admitted task operations remain authoritative. |
| `META_PROVIDER`, `META_AUTOLOAD` | Existing provider/artifact identity | Preserve the existing resolution and packaged-artifact rules. |
| `META_INITIALIZER` | Ordered local `.void`, zero-argument lifecycle procedure | Use the existing assembler/linker lifecycle checks; do not turn it into an exported ordinary callable. |
| `META_INLINE` | Separately versioned optional compiler template | Use the independent inline decoder and its consistency checks. |
| Source-step/TRACE records | Existing source/step identities | Preserve their existing debugger/runtime semantics. |

Names use the existing identifier-byte spelling (letters, digits, non-ASCII
bytes, `_ ! ? $ @ #`, with dots in qualified identities), with no leading digit.
The `*` name is admitted only for a factory. Class/interface headers carry
namespace and short name separately. Existing adapters retain implements,
default-method, provider and task fields rather than translating them into
source statements. RXAS string quoting belongs to the envelope; the reader
receives decoded fields.

## Type fields

Canonical examples are `.int`, `.binary`, `.ns..item`, `.string[-2 to 3, *]`
and `reference .ns..item[]`. Primitive and user-class names share the same
lexical type grammar. The backend resolves the name and rejects unknown or
incompatible types where a resolved contract is required.

```text
type       = type-name [ dimensions ] | "reference" whitespace type
type-name  = "." identifier { ".." identifier }
dimensions = "[" dimension { "," dimension } "]"
           | { "." [ bound ] }   # historical stem spelling
dimension  = empty | bound | bound whitespace "to" whitespace bound
bound      = [ "-" ] unsigned-integer | "*"
```

Whitespace is admitted around type/dimension components. `reference` and `to`
are matched without case sensitivity. An empty dimension has the default lower
and open upper bound; a lone bound is the upper bound. The existing backend
interprets default lower bound 1 and the historical upper-bound `0` candy,
checks bounds and constructs dimensions. The reader retains the integer
spellings instead of silently coercing a bad expression to an integer.
Recursive reference parsing is bounded at 64 levels; semantic admissibility
of a reference shape still belongs to common type validation.

Reject missing names, unclosed dimensions, non-integer bounds, executable
expressions, trailing tokens, `.void` arrays and references to `.void`.
The exact-length type entry also rejects embedded NUL. Successful parsing
requires consuming the complete field.

## Callable and argument fields

The current callable descriptor version remains `rxsig1|name|return|arguments`.
Its existing signature reader is reused by the contract reader. There is no
new wire descriptor for B, C or G.

```text
arguments = empty | argument { "," argument }
argument  = [ "expose" whitespace ] [ "?" ] name "=" type
          | [ "expose" whitespace ] "..." "=" type
```

The comma separator ignores commas inside bracketed dimensions. `expose`
retains a by-reference formal; `?` retains an optional formal; `...` retains
the final vararg formal. A vararg cannot be optional or followed by another
formal. Argument types cannot be `.void`. Empty and whitespace-only argument
fields both mean zero arguments. The current emitter spells `expose` in
lowercase; this reader uses that existing signature convention.

Factory body contracts carry a `.void` result; the ordinary callable registry
also describes the constructed object result for call-site typing. Keep those
two identities distinct when rebuilding a factory template.

Older authored RXAS signatures such as POS/LASTPOS carry `start=1` rather
than a solved optional type. The reader admits complete integer, floating
and quoted-string literals in that historical default position, retains
the literal and marks the formal optional. It infers the declaration type;
the provider owns default execution. This is not an expression evaluator.
The legacy signature splitter does not admit commas inside quoted defaults;
generated metadata uses solved optional types, and executable/default trees
belong to the separately validated inline payload.

Reject empty components, missing `=`, missing/invalid type fields, statement
separators outside quoted literals, incomplete literals, trailing expression
text and invalid vararg ordering. No rejected contract may become a silently
different signature. Import adapters retain the existing
`SYNTAX_ERROR_IN_IMPORT_DECL` and `RXPA_IMPORT_SIGNATURE_INVALID` diagnostics
and native-provider failure behavior.

## Ownership, provenance, versions and failure

Reader outputs own all names, type trees, bounds, argument arrays and retained
literal text. Callers may release or modify their input after a successful
read. A failed read leaves an initialized output; matching free functions are
safe after success or failure. Free old contents before reusing an output.
The error record contains static field/reason text and a field byte offset
when available. There is no global reader state.

Declaration fields without authored positions have unknown coordinates.
The AST adapter must not inherit an unrelated consumer or inline-body position
for those declarations. For example, a shadowing warning reports an imported
global identity instead of the line number of formerly manufactured B source.
Explicit source identities carried by inline/debug records retain their own
decoder and ownership rules.

RXBIN/envelope versions continue through their existing loaders. Unknown
callable descriptor versions are rejected by the descriptor reader. Current
imported inline readers admit `I4`, `I5`, `I6` and `I7`; an unsupported,
malformed or inconsistent optional template cannot provide optimization proof.
An ordinary callable may still be used through its valid independent contract.
Supported inline shapes must retain their optimization; dropping them is a
regression requiring repair, not a way to make the split conform.

Source dependencies are compiled by their owning frontend, then imported
through this same metadata boundary. Source syntax, grammar recovery and
source-extension policy remain the owning frontend's responsibilities.
The C-only desktop product uses the sibling `rxc` tool for typed source and
editor services, with explicit argv and private temporary RXAS files.
The broker preserves the parent's import-root order, RXAS opt-in and disabled
exit policy; source contract adapters retain the provider's own location.
Windows standalone core tools embed the UTF-8 process-codepage manifest
(Windows 10 version 1903 and later), so narrow CRT argv and host paths retain
Unicode. The broker converts those UTF-8 arguments to UTF-16 and quotes each
argument for the Windows CRT before `_wspawnv`; spaces and trailing backslashes
remain part of their original argument. This process property does not select
an external stream codec. An embedding host retains responsibility for its
own process encoding; earlier Windows Unicode-path behavior is not qualified.

## Executable conformance evidence

`metadata_contract_conformance` links only binutils and checks accepted type
and callable fields, ownership/freeing, malformed spans/signatures, literal
defaults, mappings, class/interface identities and descriptor-version rejection.
`compiler_contract_builder` checks the separate AST adapter and common
validation. The standalone Classic parity workload compiles B and G signed
providers and executes C callers in opt/no-opt and linked modes through both
compiler entry points. Existing source/binary globals, class/interface,
native-provider and imported-inline suites retain the broader ABI checks.

The C-phase acceptance and actual qualification status remain in
[`docs/ROADMAP.md`](../../docs/ROADMAP.md#proposed-c-first-extraction-phase--2026-10-10).
