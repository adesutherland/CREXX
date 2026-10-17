# Compiler ownership and pipeline

The [compiler plan](../PLAN.md) owns scope and acceptance. The repository layout
and build boundaries are described in [the compiler entry guide](../README.md).

The path for each request is source bytes → generated Level L grammar → owned
AST → language-specific semantic checks and IR → generic Level L transformations
→ language-specific RXAS selector → Level L writer/output. Assembly and linking
remain the existing `rxas` and `rxlink` tools. Source requests do not invoke the
native compiler.

`llcparse`, `llcfrontend`, `llcir` and `llcrxas` belong to Classic. `lltparse`,
`lltfrontend`, `lltir` and `lltrxas` form the typed core shared by the planned B
and current G compiler. `llccompiler` and `llgcompiler` are independent owned
services. Their separate commands each own orchestration; there is no shared
language-selecting application main. Mixed-language corpus adapters under
`tests/` are not installed or linked into either product.

The generic SDK under `lib/rxfnsl` owns grammar engines, ASTs, source coordinates,
IR schemas/builders/views, verified rewrites and the bounded writer. It does not
import a compiler. Level L itself is `langtools/`, which owns the specification
compiler (`llfront`, `llgenerate`), CLI and bootstrap. Its common helpers provide
exact RXAS quoting, raw bounded RXBIN metadata reading and atomic publication.
These helpers do not select a language or interpret compiler types.

Compiler `common/llcompilercontracts` adapts raw metadata into checked cREXX
signatures; `common/llscalar` owns cREXX scalar semantics. Family registration,
context/condition policy, binding and selectors remain with C or the typed core.
Classic currently uses typed scalar attribute types for ABI descriptions; that
is not an import of the typed frontend or its operations.

Each host closure uses Level B and Level L only. The separately emitted Classic
program calls `llctarget`, whose current runtime package depends on existing
Classic/G runtime facilities. This target dependency does not belong to the
compiler host. Proper B bootstrap, complete C/G coverage and platform replacement
qualification remain open.

The source language contract determines evaluation order, effects and failure
timing. Unknown or failing evaluations stay as operations. Generic passes use
registered verifiers/evaluators and preserve origins; they do not infer Classic
numeric or typed overflow rules. Read [algorithms](algorithms.md),
[IR representation](trees-and-ir.md), [frontends](frontends.md) and
[RXAS emission](backend.md) for the current contracts.
