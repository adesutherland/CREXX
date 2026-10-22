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
semantics and wider activation/platform work remain open.

Labels after THEN/ELSE (including WHEN THEN) prefix one required instruction,
retaining nearest-ELSE ownership. Local CALL/function entries can reach an eligible IF-arm
label without evaluating its enclosing condition. The first source label wins:
DO/SELECT labels are trace-only and block unquoted invocation with 16.3; quoted
invocation keeps its local bypass. Labels do not rearm executed PROCEDURE
eligibility. The [label/entry contract](doc/architecture.md#label-arm-prerequisite)
was introduced with C17 without changing runtime interfaces.

**SIGNAL is complete for the agreed instruction contract.** It accepts direct
symbols, decoded quoted names and once-evaluated
VALUE expressions, including an implied VALUE before a parenthesized or unary
expression. Targets use the selected case-insensitive Unicode profile and
strict text equality: `1.20` and `1.2` remain distinct labels. Transfer sets
SIGL/.SIGL, closes active loop control and reaches the selected source
label in the current invocation, retaining its pool and numeric settings.
Known static DO/SELECT targets refuse with 16.2; VALUE resolves eligibility when
reached. Seven-condition ON/OFF uses shared activation policy and condition
recording, same-frame handlers and typed payload/origin transport. Actual
NOVALUE reads, LOSTDIGITS checks and shared BIF/PARSE/ADDRESS producers use
the [whole SIGNAL contract](doc/architecture.md#signal-complete-contract).
Real host HALT now uses Group 2 ingress; broader host/platform acceptance and INTERPRET
retain their explicit [conformance qualifications](doc/levelc-conformance.md).

**TRACE is complete for the agreed practical scope.** Bare/static/VALUE forms
and TRACE() share activation state; existing B services own option parsing,
command inhibition/reporting and output. The generated observer reads live
scalar values from VM metadata and restores enclosing handler/controller state
on normal and abrupt completion. The [contract](doc/architecture.md#trace-complete-contract)
and [conformance profile](doc/levelc-conformance.md#practical-trace-target) record
the approved interactive/count/SCAN/event-coverage limitations and minor
presentation differences. Focused component qualification does not close
formal whole-compiler QA or the remaining C requirements.

The [Group 1 conditions and observations contract](doc/architecture.md#conditions-observations-complete-contract)
adds CALL ON/OFF for ERROR, FAILURE, HALT and NOTREADY, authored clause
completion, CONDITION state and PARSE/ARG/PULL field/drop observations. Existing
B services own policy replacement, delayed delivery, suppression and buffered
HALT. One finite source dispatcher preserves captured operands/control and
handler RETURN/EXIT cleanup; idle completion allocates no result packet, and
source without CALL ON declarations omits completion guards. The native practical
scope remains the ceiling. Group 2 adds execution-local real HALT ingress;
arbitrary raw-provider resumption and wider host/platform proof remain open. Cross-function internal IR
calls from a function with immutable CALL ON declarations are refused because
handler ordinals belong to that source function; ordinary source uses one shared
body, and existing multi-function IR without CALL declarations remains admitted. Component
qualification and local delivery are tracked by CCOND, not full-C acceptance.

The begin-only [CLAUSE1 profile](doc/architecture.md#clause-clock-contract)
locally qualifies lazy DATE/TIME samples in the current activation, including
ordered DO captures, repeated WHILE and first/repeated UNTIL. Clock-free owners
emit no clock reset or clock-only context transport. Practical TRACE retains
source metadata without activating clocks. CCOND adds completion/checkpoints
while preserving lazy samples across handler execution; wider clock host/platform
behavior remains open.

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
is delivered; CCOND adds admitted NOTREADY delivery and actual target/drop TRACE
records through the shared executor. Wider host/platform proof remains open.
The [current C contract](doc/architecture.md#current-c-family-contract)
uses the 36-export target. The plan tracks instruction status and remaining work;
declared platforms and whole ARG/PULL instruction
acceptance remain open.

PUSH and QUEUE evaluate their optional expression once and insert exact Classic
text at the front or tail of the execution-selected queue. Bare forms insert
empty text. They reuse existing conversion/config methods and skip insertion
when the operand exits the program. FIFO/consumed-slot and mixed front/tail
composition retain one shared queue owner. CCOND supplies admitted clause and
condition observations; wider host/platform work remains open under the
[PUSH](doc/architecture.md#push-contract) and
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
frontend/service `warnings()` returns owned records. CCOND integrates admitted
ERROR/FAILURE/NOTREADY completion and TRACE; [the ADDRESS contract](doc/architecture.md#address-c-command-contract)
retains wider host/NUL transport and platform closure.

NUMERIC DIGITS/FUZZ retain their optional expressions and defaults 9/0. FORM
accepts SCIENTIFIC, ENGINEERING, VALUE expression and bare scientific reset;
VALUE may be omitted before a parenthesized or unary expression. Each operand
evaluates once through the shared activation setter, then installs validated
settings in the executing frame. Internal/private calls inherit settings and
restore the caller frame. CCOND preserves the existing configured
SYNTAX/NOVALUE/LOSTDIGITS and practical TRACE paths. The [NUMERIC contract](doc/architecture.md#numeric-c-contract)
retains wider numeric/expression and host/platform scope.

CALL and function expressions resolve internal labels, BIFs, then explicitly
selected external bindings. Quoted targets skip internal labels and still admit
correctly spelled BIFs. A configured EXTERNALROUTINE hook can handle a reached
missing name; otherwise it fails43.1. Bare function RETURN45.1 and EOF44.1 keep
their separate source locations.

| CLI option | Binding / output |
|---|---|
| `--provider artifact-stem qualified-callable` | Explicit RXBIN Classic packet or ordinary typed scalar export under `-i`. |
| `--native artifact-stem qualified-callable` | Explicit RXPA declaration/identity from the owning native loader; ordinary typed CALL dispatch. |
| `--executable alias path` | Direct argv process binding; normal exit status is the result, stdout/stderr remain inherited/configured streams. |
| `--routine qualified-export` | Signed Classic activation-packet provider. |
| `--host qualified-export` | Ordinary typed host request/result export using the [shared host facade](../../../lib/rxfnsc/RexxClassicHost.md). |

Bindings are repeatable and share one external alias namespace using the
existing callable-identifier grammar, including qualified names. Quoting changes
lookup precedence; an executable path is separate from its alias. Selected
bytecode providers must be linked into the application; native provider
requirements retain their actual manifest identities. Ordinary scalar calls
convert RexxValues at runtime, check arity/presence at compile time and leave
defaults to the callee. Exposed scalar arguments use discarded temporary cells;
true references, aggregates, objects, methods/factories and tasks require
explicit adapters. B/G globals remain inaccessible to Classic callers. Delayed
handlers reuse the same resolved routes and zero-argument admission.

The optional compiler-only `rxcontract` provider projects through binutils'
existing metadata owner. Static C applications do not contain this decoder or a
source compiler. Process bindings use exactly ADDRESS's existing child and
byte-channel services, with no argv flattening or implicit shell. No automatic
PATH/source lookup is added; Classic source loading remains with final INTERPRET.
Formal compiler/platform/delivery qualification remains open in PLAN.

RETURN preserves present empty text separately from an absent result and uses
the existing full signed64 main-status boundary. Physical comma continuations
retain original source spans; malformed RETURN expressions, including dead
source, report contextual35.1. CCOND captures completion before handler delivery
and preserves handler resumption/cleanup. Exact traceback and broader host/
platform completion remain open under the [RETURN contract](doc/architecture.md#return-contract).

EXIT preserves owned optional scalar results on the existing program root and
propagates them through internal calls before ordinary result use or later
evaluation. Forced unwind releases pending DO/PARSE state and restores private
pools; only the root bridge narrows a status. CCOND integrates delayed-handler
EXIT with its correct program root and optional result. The [EXIT contract](doc/architecture.md#exit-contract)
retains wider signed-provider lifecycle, host and platform work.

C compiler workers retain one private prepared language definition. Source
parsing and name admission borrow its validated DFA/LR data and CFG metadata;
comments, limits, tokens and diagnostics stay local to each request. See the
[prepared-language contract](doc/architecture.md#proposed-prepared-language-reuse).

The approved PROCEDURE increment uses one source-unit body with checked main/label
entries. Internal calls initially share the caller pool; only an executed
PROCEDURE creates a private pool and applies direct/indirect EXPOSE in source
order. Its [entry/lifetime contract](doc/architecture.md#proposed-procedure-contract)
requires explicit return/fatal restoration. This static/fatal profile is locally qualified;
CCOND adds admitted handler/TRACE cleanup. Dynamic eligibility, host/platform
integration and INTERPRET remain open.

[The walkthrough](doc/pipeline-walkthrough.md) and maintained examples describe
the implemented PoC. C completion proceeds instruction by instruction, ending
with INTERPRET. Correct execution and emitted-code quality are both acceptance
requirements; native output is a comparison control, not the specification.
