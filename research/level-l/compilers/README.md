# Experimental compilers using Level L

These independent compilers are research consumers of Level L. All grammar,
frontend, semantic and RXAS-emission code is Level B. Native `rxc` is the build
seed; source compilation requests never call it.

| Owner | Responsibility |
|---|---|
| `levelc/` | Classic grammar, token policy, frontend, semantic family, selector, service and CLI |
| `typed/` | Shared typed B/G parsing, binding, types, IR and RXAS selection; currently qualified for G |
| `levelg/` | G service and CLI over the typed core |
| `levelb/` | Explicit future B profile; not implemented or claimed self-hosted |
| `common/` | Checked scalar codecs/evaluation and compiler ABI interpretation |
| `levelc/runtime/` | Classic target adapter; separate from the compiler host |
| `tests/` | C/G regressions; mixed-language adapters exist only here |

Build explicitly from the repository root:

```sh
cmake -S . -B cmake-build-debug -DCREXX_BUILD_RESEARCH_COMPILERS=ON
cmake --build cmake-build-debug --target stage-product stage-research-compilers --parallel 8
```

Select one language with `-DCREXX_RESEARCH_LANGUAGES=C` or `G`; the default is
`C;G`. No B product is offered. Products are in
`cmake-build-debug/research/level-l/compilers/products/`.

Normal installation includes the SDK and Level L tools. Install research
products explicitly after installing that base:

```sh
cmake --install cmake-build-debug --prefix '/tmp/crexx review'
cmake --install cmake-build-debug --prefix '/tmp/crexx review' --component ResearchCompilers
```

The services are `llccompiler.rxbin` and `llgcompiler.rxbin`. The old combined
`llcompilers.rxbin` and `request(level,...)` API are retired. Each service exposes
`request(source, artifacts, filename, budget, nesting, bytes, registers, optimise)`;
arguments after source retain their defaults. See [backend](doc/backend.md).
Both commands preserve `-i`, `-o` and `-n`. Use installed packages with the
existing `rxas`, `rxlink` and VM for execution.

[The plan](PLAN.md#c-first-delivery-milestone) owns the active C-only delivery;
new B/G features and self-hosting are outside it. Existing G behavior remains
supported. The [target architecture](doc/architecture.md),
[requirements crosswalk](doc/levelc-requirements.md) and
[worker packages](doc/agent-work-packages.md) are approved for implementation;
open stages are not implemented capability. Shared Unicode algorithms and the
23 procedural entrypoints now belong to the optional B `rxunicode-core` component;
C adapters use that core directly, while G retains its API and two wrapper
classes. Ordinary CALL binds all 95 direct shared BIF names; complete BIF/host
semantics and wider activation/condition work remain open.

The approved C PARSE implementation admits ARG, PULL, SOURCE, LINEIN, VERSION,
VALUE and VAR, optional UPPER, comma templates, targets/drops, string/variable
patterns and absolute/relative positions. It constructs immutable binary
templates for the shared B executor, which uses the existing VM cursor for
nonempty templates. Its finite C
profile is locally qualified. Standalone ARG now reuses those templates and
executor with implicit UPPER, replacing its scalar shortcut. Standalone PULL
also uses this executor: bare/empty-first forms acquire selected queue or fallback
input once, and later comma templates receive empty text. It consumes PROCEDURE
eligibility even with no targets. Its local source/queue/EOF/private-pool profile
is delivered; configured NOTREADY, host, conditions/TRACE and platforms remain
open. The [current C contract](doc/architecture.md#current-c-family-contract)
uses the 36-export target. The plan tracks instruction status and remaining work;
complete conditions/TRACE, declared platforms and whole ARG/PULL instruction
acceptance remain open.

PUSH and QUEUE evaluate their optional expression once and insert exact Classic
text at the front or tail of the execution-selected queue. Bare forms insert
empty text. They reuse existing conversion/config methods and skip insertion
when the operand exits the program. FIFO/consumed-slot and mixed front/tail
composition retain one shared queue owner. Shared host/condition/TRACE/platform
work remains open under the [PUSH](doc/architecture.md#push-contract) and
[QUEUE](doc/architecture.md#queue-contract) contracts.

Explicit ADDRESS supports lasting environment selection, VALUE expressions,
bare current/alternate swaps and transient commands with ordered WITH resources.
For example, `address 'ENV' 'command :ITEMS[]' with output stream FILE` snapshots
FILE at that connection and passes a lexical counted-stem binding through the
existing B adapter. Commands evaluate once; operand EXIT skips construction and
dispatch. The adapter owns request snapshots, redirects, binding writeback and
RC/.RC/.RS. Implicit expression commands use the active environment and lasting
connections after once-only expression conversion. For example, `COMMAND`
dispatches its value and produces an owned `RXC-LC-IMPLICIT_ADDRESS` source
warning; a string-headed command such as `'literal :SCALAR'` suppresses that
warning and retains its lexical binding plan. CLI warnings go once to stderr;
frontend/service `warnings()` returns owned records. [The ADDRESS contract](doc/architecture.md#address-c-command-contract)
keeps configured conditions/resumption, TRACE and host/platform closure open.

Ordinary CALL accepts symbol/decoded quoted targets, source-ordered presence
packets and local-before-BIF resolution; quoted targets skip locals. Missing
routines fail43.1 when reached. Bare function RETURN45.1 and EOF44.1 retain
their separate source locations. Static signed statement providers use explicit
prebuilt metadata bindings and a void/frame routine wrapper. `--provider <artifact-stem> <qualified-callable>`
is repeatable under `-i`; `--routine <qualified-export>` produces a provider.
Selected images must be linked into the application; source providers are built
first by their owning compiler. Delayed handlers and external function
expressions retain their named open boundaries.

RETURN preserves present empty text separately from an absent result and uses
the existing full signed64 main-status boundary. Physical comma continuations
retain original source spans; malformed RETURN expressions, including dead
source, report contextual35.1. Exact traceback, conditions and broader host/
platform completion remain open under the [RETURN contract](doc/architecture.md#return-contract).

EXIT preserves owned optional scalar results on the existing program root and
propagates them through internal calls before ordinary result use or later
evaluation. Forced unwind releases pending DO/PARSE state and restores private
pools; only the root bridge narrows a status. The [EXIT contract](doc/architecture.md#exit-contract)
keeps signed-provider, delayed-handler, wider host and platform work explicit.

C compiler workers retain one private prepared language definition. Source
parsing and name admission borrow its validated DFA/LR data and CFG metadata;
comments, limits, tokens and diagnostics stay local to each request. See the
[prepared-language contract](doc/architecture.md#proposed-prepared-language-reuse).

The approved PROCEDURE increment uses one source-unit body with checked main/label
entries. Internal calls initially share the caller pool; only an executed
PROCEDURE creates a private pool and applies direct/indirect EXPOSE in source
order. Its [entry/lifetime contract](doc/architecture.md#proposed-procedure-contract)
requires explicit return/fatal restoration. This static/fatal profile is locally qualified;
complete conditions/TRACE, host/platform integration and INTERPRET remain open.

[The walkthrough](doc/pipeline-walkthrough.md) and maintained examples describe
the implemented PoC. C completion proceeds instruction by instruction, ending
with INTERPRET. Correct execution and emitted-code quality are both acceptance
requirements; native output is a comparison control, not the specification.
