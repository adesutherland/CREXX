# Independent C and typed frontends

C uses `llcfrontend..request(source,...)`; G uses
`lltfrontend..request(source,...)`. The former combined `request(level,...)`
product API is retired. `tests/lltestfrontend` only routes the shared corpus;
it is not a product library. The remaining sections describe the unchanged
admitted language and diagnostics contracts.


The separate C/G frontends recognise the [admitted source profile](poc-scope.md)
and produce verified Level L IR. The compiler implementation is Level B. The
`.rxl` files are Level L specifications; their generated DFA/LR(1) tables run in
Level L. Source compilation never invokes the existing native compiler.

These services stop at source trees and IR. The separate compiler services add
RXAS selection and emission in Level B using Level L's writer; their CLIs own
file I/O. See [the backend](backend.md).

## Request and result

```rexx
options levelb
import rxfnsb
import lltfrontend
import llir

main: procedure = .int
  source = 'options levelg; total=.int; total=2+3; say total' as .binary
  request = .lltfrontend..request(source, 'cmake-build-levell-integration/bin', 'demo.crexx')
  if request.status() <> 0 then do
    say request.diagnostic()
    return 1
  end
  graph = request.graph()
  say graph.dump()
  return 0
```

Each `request(source, artifacts='', filename='<source>', budget=1000000,
nesting=512)` owns its binary source and results. The artifact directory supplies `library.rxbin` for G's
explicit `import rxfnsb`, and `rxfnsc.rxbin` when a Classic BIF is used. There is
no ambient installed-library search or old-compiler fallback. The images are
read as metadata, not imported into the frontend host or executed.

The first query performs compilation once. Subsequent queries return that
request's retained result:

| Method | Result |
|---|---|
| `status()` | Zero on success; nonzero on failure. |
| `error()` | Empty on success; stable diagnostic category and explanation on failure. |
| `offset()` | Zero-based byte offset for the diagnostic anchor. |
| `diagnostic()` | Filename, category, line/Unicode column, byte offset, source excerpt and caret. Empty on success. |
| `ast()` | Owned Level L source tree if parsing succeeded. Semantic failure may retain that tree; parse failure returns a failed tree. |
| `graph()` | Verified immutable module on success. On failure returns a failed module with nonempty `error()`, never a partly built successful graph. |

The AST and IR own their storage; borrowed node/value handles follow Level L's
owner/revision rules. Keep their owning result alive while using borrowed views.
Independent requests can succeed or fail without invalidating an earlier
request's tree or module. This establishes ordinary sequential service reuse;
concurrent reentrancy and interpreted-group activation remain later gates.

The budget bounds source bytes and semantic visits, with a token-layout bound
of sixteen times that budget. The IR builder receives the same entry limit and
64 times that limit for bytes. Generated parsing also uses Level L's maintained
work limits. Nesting is checked before recursive semantic visits. Imports have
separate expansion limits (16 MiB for the B library, 64 MiB for Classic's current
runtime package). These are resource limits, not performance promises. The
[algorithm reference](algorithms.md) describes the costs of name lookup and type propagation
and bounded failure paths.

## Source meaning and IR contracts

The [tree reference](trees-and-ir.md) defines the actual generated node fields.
Each expression precedence level uses ordered `Chain(operands+, operators*)`;
transparent single-operand chains do not introduce IR operations. Operator and
literal nodes retain their original bytes and half-open source ranges. A
nonterminal range can include trivia before the next token; leaf spelling remains
exact. IF's
`condition`, `yes` and `no` are named owned AST children. Lowering recursively
maps those children and creates separate IR blocks; they are not pre-existing
flow nodes or reserved variable names.

All compiler operation descriptors are registered by
[`lltir.crexx`](../typed/lltir.crexx). The generic Level L schema,
storage, verifier and codecs remain language-independent. The frontend adds a
request-local `crexx.calls` family for resolved G signatures. Every operation
has declared operand/result/attribute types, state effects, possible failure
and normal-control behaviour. An effect or failure name is a compiler contract,
not proof of a runtime implementation. Backend equivalence is tested in STEP04.

### G values, bindings and control

G has int64, UTF-8 text, binary float64, exact-spelling decimal and Boolean
carrier types. Constants use checked codecs. Integer overflow at literal
admission is diagnosed; an arithmetic operation such as `1 / 0` remains in IR,
so an untaken branch does not become an unconditional compiler error. Decimal
literal conversion preserves source digits instead of first rounding to float.
Unary minus retains a separate operation where observable, including signed
float zero; the minimum int64 literal is admitted without first overflowing its
positive magnitude.

G bindings are scoped per routine. Explicit declarations and inferred bindings
receive their language-defined default at entry. Type propagation reaches a
bounded fixed point before lowering. Parameters admit required scalars, scalar literal defaults and primitive
`?name=type` defaults. `?name` observes original formal presence. Arbitrary
default expressions, varargs, arrays and references remain outside this slice.
STEP07A adds declaration-only `namespace name expose routine` providers and
explicit qualified calls; procedure pool EXPOSE remains outside this slice.
Numeric promotion and explicit `as` conversions select typed compiler operations.
Loose text comparison follows G's numeric-aware/padded comparison contract;
strict `==` compares converted text and therefore retains G's literal
canonicalisation. It is not redefined as a host-language byte comparison.

`crexx.g.boolean` stores the existing integer Boolean carrier. For example,
Boolean arithmetic can leave a carrier other than 0/1. `crexx.g.truth` explicitly
converts it to the canonical core Boolean required by `core.branch`. This keeps
the core branch type's invariant while preserving G's arithmetic behaviour.

`&` and `|` create RHS/skip/join blocks; the skipped path retains the converted
left carrier, and the evaluated path stores the right carrier. `&&` evaluates
both sides. Hidden typed slots carry values across joins. Ordinary binary
operands and call arguments evaluate left to right; a left value is saved before
lowering a right expression that may create blocks or cause effects.

| Operation family | Contract |
|---|---|
| `crexx.g.int64`, `.float64`, `.decimal`, `.boolean` arithmetic | Selected typed arithmetic/unary meaning and checked possible failures. The earlier direct-IR demo retains its separate checked `iadd` operation. |
| `crexx.g.convert.<from>.<to>` | Explicit language conversion; conversion failures remain observable. |
| Typed `eq/ne/lt/le/gt/ge/strict`, `boolean.not/xor`, `truth` | G comparison/logical semantics and explicit core-Boolean conversion. |
| `crexx.g.text.concat`, `.say` | Text concatenation; output in source order, with output/conversion failure contracts. |
| `crexx.calls.local.<name>`, `.rxfnsb.length` | Checked signatures; local calls conservatively carry state/failure effects. No inlining or execution occurs in this stage. |

### Classic values and activation

Classic literals retain exact spelling as `crexx.c.value`; a name is a
case-insensitive dynamic-pool key. An unset read remains a pool read and can
produce the name according to Classic's default policy. Empty assignment stores
an empty value. It is not converted into a G typed local or declaration default.

Classic quoted literals immediately followed by a single `X`/`B` symbol use
the existing ordinal syntax, case-insensitively. Token layout preserves the
whole spelling in one `String` node. Lowering checks digit/group boundaries
and uses the Level B `x2c`/`b2x` helpers to create a constant: every decoded byte
maps to U+0000–U+00FF. `'C3A9'x` therefore contains two characters, while `'é'`
contains one. The first group may be short; leading/trailing blanks and invalid
digits/grouping fail with a located `LL_COMPILER_SYNTAX` diagnostic. A space,
comment or longer identifier after the quote retains ordinary concatenation.
G's admitted literal syntax is unchanged. Decoding adds no target operation or
runtime helper call. See the [SAY instruction review](LEVELC-INSTRUCTION-REVIEW.md).

Each source unit receives required borrowed-context and integer-entry
parameters. The caller owns a fresh activation and separate child bundle that
initially borrows its current pool; the root owns the UTF8 configuration.
Arguments remain ordered presence packets. Internal calls evaluate actuals
before constructing child owners and inherit the parent's numeric state.
Only executed PROCEDURE replaces the live pool; see the
[current entry/lifetime contract](architecture.md#proposed-procedure-contract). NUMERIC validates the live activation, then the executing C frame
installs its validated settings; child return restores the parent frame.

| Operation | Contract |
|---|---|
| `load`, `store` | Resolve named reads/writes through the borrowed bundle's current pool. |
| `add/sub/mul/div`, comparisons, `positive/negative/not` | Exact Classic values and explicit numeric context. Arithmetic/conversion/logical errors occur at the operation's runtime point. |
| `concat/blank/abut`, `and/or/xor` | Source-specific concatenation; both logical operands evaluate in source order. |
| `truth` | Validate the Classic logical value through the live context; invalid values can fail. Exact literal `0`/`1` also proves a derived core Boolean for control selection, while retaining runtime truth effects. |
| `numeric.digits/fuzz/form`, `digits` | Validate/update the borrowed activation or inspect the executing frame, including defaults/reset and caller inheritance. |
| `arguments.empty/append` | Ordered original argument values and a separate presence Boolean; omitted and supplied-empty arguments differ. |
| `parse.source/execute` | PARSE, standalone ARG and PULL share acquisition and immutable LCP1 execution. ARG implies UPPER only for captured source; empty ARG templates skip acquisition/execution and preserve ordinal gaps. PULL implies UPPER, acquires once even when bare/empty-first, and supplies empty text to later comma templates. Live names/presence/original arguments retain the shared runtime contract. |
| `arguments.count/exists` | ARG count and `ARG(n,'E')`/presence option validation. |
| `call` | Resolved local entry, argument packet, borrowed context and function/subroutine attribute; fresh child activation/bundle initially share the current pool. |
| `bif`, `call.missing` | Exact named shared BIF binding or reached missing43.1 after actual evaluation; preserve raw presence. |
| `return.present/empty/absent`, `requirevalue`, `setresult` | Present value, authored bare RETURN45.1 in a function, or generated EOF absence; expression EOF fails44.1 with target identity, and CALL sets or drops RESULT/.RESULT. |
| `length`, `say`, `exit` | Actual Classic BIF contract, observable output and process exit. |

These names carry the `crexx.c.` prefix in IR. No contextual Classic constant
evaluator is registered. Empty `NUMERIC FORM` selects SCIENTIFIC. Classic
labels and PROCEDURE remain separate AST nodes; semantic inventory pairs the
admitted entry with its statement interval. Label-only shared-pool routines and
fall-through into a procedure entry receive profile diagnostics.

Both languages lower IF, loops, LEAVE, ITERATE and RETURN to the common
block/edge structure. Classic's [approved static DO forms](architecture.md#approved-do-contract)
use ordered header captures and function-local state keys, checked by the
compiler-owned `llcdoprotocol` before source-graph publication and again after
transformation/loaded-graph admission. G retains its existing DO WHILE profile.
EXIT has a diverging compiler operation; a structural
return closes its block but is unreachable under that contract. General flow
services must refuse to claim complete coverage when their accepted termination
profile excludes such a diverging extension. Neither graph shape nor an effect
label alone proves arbitrary abnormal control flow.

## Imported contracts

[`llcompilercontracts.crexx`](../common/llcompilercontracts.crexx) is a Level B
RXBIN007 reader. It validates the image envelope, directory ranges/alignment,
reserved fields, expansion bounds, constant/metadata record framing, ordered IDs,
and the selected callable/export/type/member/autoload records. Its bounded LZSS
decoder expands only the constant and metadata sections. Unused code sections
and unneeded metadata payloads stay opaque; this is not full object-file or
production metadata conformance.

G LENGTH must resolve to `rxfnsb.length`, returning `.int` with one required
`.string`. The Classic LENGTH/DIGITS/ARG routes check their actual exported
runtime signatures, including the B-hosted BIF context and ARG activation
argument type. Required/optional flags are checked, including the required Classic context
and optional ARG activation parameter. A request caches imported catalogs and
selected signatures. G scalar exports are read from an explicit module RXBIN
and matched per position; missing/private/ambiguous exports, duplicate formal
names, malformed literal-default metadata and unsupported types fail with a
source location. Defaults remain in the provider; imported metadata supplies
optionality, not a value to insert in the caller.
Runtime packages are metadata inputs; they are separate from the compiler's
B/L host imports. Full Classic packaging still has its existing G build
prerequisite, which this frontend does not import.

## Errors and recovery boundary

Syntax failures include the original unexpected token and the generated
expected-token set, translated to source words and punctuation. These grammars
currently stop at the first syntax error: they declare no repair policy. Source
preparation still distinguishes invalid UTF-8 and unclosed comments. The
source-to-IR service also refuses any recovered tree if a future parser policy
produces one.

| Category | Meaning |
|---|---|
| `LL_COMPILER_SYNTAX` | Missing/unexpected token, unterminated quote or unmatched source byte. |
| `LL_UTF8`, `LL_COMMENT` | Source encoding or nested-comment error from Level L. |
| `LL_COMPILER_PROFILE` | Recognised construct outside the admitted subset, or unsupported language/options. |
| `LL_COMPILER_TYPE`, `BIND`, `CALL`, `RETURN`, `FLOW`, `IMPORT` | Prefixed by `LL_COMPILER_`: type/binding/call/return/control/import explanation at the source anchor. |
| `LL_COMPILER_LIMIT`, `LL_COMPILER_RECOVERED` | Resource refusal or repaired-source refusal; no successful IR. |
| `LL_IR_*`, `LL_COMPILER_INVALID` | Checked construction/contract failure or trapped internal operation failure; no successful IR. |

Recognised unadmitted instruction names such as INTERPRET receive PROFILE.
Arbitrary token sequences outside the grammar may still receive SYNTAX; this is
not a complete parser for the full languages. Diagnostics are compiler-service
categories, not newly assigned Classic runtime error numbers.

## Inspecting a source file

From the repository root, after the normal product build:

```sh
cmake-build-levell-integration/bin/rxbvm \
  cmake-build-levell-integration/research/level-l/compilers/frontends/opt/frontend-probe-linked.rxbin \
  -a g research/level-l/compilers/examples/step01/pipeline-g.crexx \
  cmake-build-levell-integration/bin
```

Use `parse-probe-linked.rxbin -a g <file>` for the owned source tree and byte
ranges. The [paired walkthrough](pipeline-walkthrough.md) links all six generated
tree/IR outputs and explains their semantic differences. These inspection
consumers exercise the same service as the regression tests; they are not the
future installed `llrxc-c`/`llrxc-g` commands.

## Preparing focused tests

CTest executes prebuilt images; it does not compile them. The normal product
build includes the compiler test images. A QA-only build instead uses the
registered preparation dependencies, collected after this directory has
declared all its tests. The optimizer-parity preparation target builds both
the earlier IR demo/contracts and the frontend tests in opt/no-opt modes.

For an already configured Debug build:

```sh
cmake --build cmake-build-levell-integration --target qa-prep-optimizer-parity --parallel 8
ctest --test-dir cmake-build-levell-integration --output-on-failure --parallel 4 \
  -R '^llcompiler_(ir_|frontend_)'
```

This runs eight IR demo/contract tests and 24 frontend tests across both VMs.
The frontend groups cover syntax/messages, grammar, mappings, calls, imported
contracts and request lifecycle. The ordinary hosted optimizer-parity job runs
these alongside the repository's other parity tests.
