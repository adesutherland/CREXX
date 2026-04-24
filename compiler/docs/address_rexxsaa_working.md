# ADDRESS and REXXSAA Working Notes

Status: draft, direction partially approved
Last updated: 2026-04-24

## 1. Purpose

This document is the working record for the new programme around `ADDRESS`,
REXXSAA-related command environments, and the compiler/runtime refactoring that
supports them. It is intended to survive across sessions and capture:

- requirements
- current implementation facts
- candidate design direction
- open approval points
- staged plan
- progress

## 2. Scope of this programme

This work currently covers:

- the `ADDRESS` instruction and implicit command execution
- command-environment integration and future address environments
- compiler exits as the likely refactoring mechanism
- REXXSAA compatibility considerations
- variable exposure / variable-pool concerns
- redirect handling and future pipeline support

This work does not yet commit CREXX to a full legacy-compatible REXXSAA ABI as
the primary modern interface.

## 3. Current implementation review

### 3.1 Parser and AST shape today

`ADDRESS` now executes through the certified exit path and no longer has
legacy parser/AST ownership.

- explicit `ADDRESS` now enters the compiler tree as `EXIT_EXTENDED`
- clause keywords such as `INPUT`, `OUTPUT`, and `ERROR` are now consumed as
  exit tokens rather than legacy grammar productions
- `IMPLICIT_CMD` is still produced by the parser and then routed to the
  certified `ADDRESS` exit bridge
- the compiler parser wrapper in `compiler/rxcpbpar.c` promotes certified exit
  primaries and contextual exit keywords for the new path

Important consequence:

- certified exit ownership is now the active implementation path for
  `ADDRESS`
- legacy `ADDRESS` grammar productions and parser-era AST node shapes are now
  removed
- disabling exits now also disables explicit `ADDRESS` parsing, matching the
  existing `PARSE` model

### 3.2 Lowering in the compiler today

The active lowering path is now the certified/system exit bridge in
`compiler/rxcp_exit.c`, with supporting validation logic in
`compiler/rxcp_val_trans.c`.

- `ADDRESS` and `PARSE` are registered as certified exits.
- `EXIT_EXTENDED` and `IMPLICIT_CMD` nodes can be lowered by the exit bridge.
- the current `ADDRESS` lowering still targets:
  - `rc = _address(...)`
- explicit `ADDRESS` currently requires both an environment and a command
  expression
- implicit command dispatch currently hardcodes `'SYSTEM'` as the environment
- redirect operands are still normalized through the existing helper calls:
  - `_noredir()`
  - `_string2redir()`
  - `_array2redir()`
  - `_redir2string()`
  - `_redir2array()`
- warning handling for implicit command use now preserves certified-exit
  lowering rather than suppressing it.

Important consequence:

- implementation ownership has moved to the certified exit framework
- some legacy compiler knowledge about `ADDRESS` still remains in parser/AST
  structures and should be removed as a follow-on cleanup
- Stage 3 runtime work also needs a small certified-exit follow-up so
  `ADDRESS env` can set the current/default environment without executing a
  command
- the runtime backend assumptions are intentionally unchanged in this stage

### 3.3 Runtime library path today

The current runtime path is:

1. compiler lowers to `_rxsysb._address`
2. `_address` in `lib/rxfnsb/rexx/_address.rexx` builds redirect state
3. inline assembler issues `spawn`
4. VM `SPAWN_REG_REG_REG` calls `shellspawn()` in `interpreter/rxspawn.c`

Important current facts:

- `_address` currently ignores its `env` argument completely.
- So `ADDRESS shell`, `ADDRESS cmd`, `ADDRESS system`, and any other
  environment literal all currently use the same backend path.
- there is no real runtime current/default address-environment state yet
- `EXPOSE` on `ADDRESS` currently means "pass named variables as environment
  variables to the spawned command", not "grant arbitrary access to the Rexx
  variable pool".
- On POSIX, `shellspawn()` uppercases exposed variable names before `setenv()`.

### 3.4 Shell behaviour is platform-dependent

The current implementation does not uniformly "call the shell".

- On Windows, `shellspawn()` prepends `cmd /c`.
- On POSIX, `shellspawn()` tokenises the command string itself, resolves the
  executable via `PATH`, and calls `execv()` directly.

Important consequence:

- shell syntax is not consistently available across platforms
- pipes, redirects, shell expansion, and other shell semantics are not a safe
  cross-platform assumption today
- the current "environment" value is mostly nominal rather than real

### 3.5 Redirect implementation today

Current redirect support is built around opaque `REDIRECT` structures in
`interpreter/rxspawn.c`.

- output-to-string and output-to-array each create a pipe and a worker thread
- input-from-string and input-from-array each create a pipe and a worker thread
- `shellspawn()` waits for the child process and joins each redirect thread
- the spawned process sees up to three stream endpoints:
  - stdin
  - stdout
  - stderr

Assessment:

- this is workable for single-process capture/feed
- it is not yet a reusable endpoint abstraction for multi-process pipeline
  graphs
- the threading model is per-endpoint today, not shared or multiplexed

### 3.6 Compiler exits today

The current exit framework already provides useful machinery, but not enough
for this programme.

What exists:

- exit discovery and registration in `compiler/rxcp_exit.c`
- primary exit keywords
- static additional keywords
- `pre_process()` hook
- `process()` hook
- token marshalling into `rxcp.token`
- replacement, pending, reject, and error statuses

Current limitations relevant to this programme:

- built-in keywords are rejected as exit primary keywords
- additional keywords are static strings, not contextual per occurrence
- additional keywords are tracked globally, not scoped by the active exit
- `pre_process()` currently returns only a space-separated list of token
  indices to hoist
- that return format cannot express:
  - typed hoists
  - aliases
  - pools
  - contextual keyword positions
  - richer planning metadata

This limitation already shows up in exits like `parse`, which need compiler and
editor awareness of exit-local keywords.

### 3.7 Documentation gap today

The current docs are incomplete for this topic.

- `docs/cREXX Level B Language Reference.md` has empty sections for both
  `ADDRESS` and the "CREXXSAA Interface".
- `docs/books/crexx_language_reference/statements.md` only states the very
  high-level purpose of `ADDRESS`.
- `docs/books/crexx_vm_spec/Level-B-Grammar.tex` describes a simpler
  `ADDRESS` than the implemented grammar.
- `docs/books/crexx_vm_spec/Level-c-Grammar.tex` contains richer command
  connection ideas than the current Level B implementation.

Conclusion:

- a working design note is needed before code changes start

## 4. REXXSAA surface relevant to this work

The supplied `rexxsaa.h` is a useful compatibility reference. The parts most
relevant to this programme are:

- `RexxStart()` and `RexxCallBack()` for calling Rexx and calling back into it
- `RXSYSEXIT` registration and callback families
- `RXCMD` / `RXCMDHST_PARM` for command-environment dispatch
- `RXSIO` and related host I/O hooks
- `RexxVariablePool()` with `SHVBLOCK` and `RXSHV_*` operations for
  set/fetch/drop/next semantics

Working interpretation:

- this header defines a legacy compatibility target worth considering
- it should not automatically define the primary modern CREXX API shape
- a future adapter may implement this interface over a more modern CREXX
  internal protocol
- for the current implementation wave, `rxvml` is the real external CREXX
  entry point; any `RexxStart()` / REXXSAA adapter remains later work

## 5. Approved decisions and requirements

### 5.1 Approved decisions as of 2026-04-19

The following points are now approved for this programme.

1. `ADDRESS` may become a formally approved and certified compiler exit.
   This is acceptable because exits are expected to be policed and created with
   the core team rather than as an unrestricted end-user extension surface.
2. The internal modern CREXX protocol is canonical.
3. Compatibility support remains mandatory. Modern preferred usage may still be
   narrower and safer than the full compatibility surface.
4. For this phase, redirect work is limited to abstraction and future-proofing,
   not full pipeline execution.
5. Address environments should fit a common class-shaped contract.
6. Exit documentation must be updated as part of this programme so the
   certified/system-exit model does not live only in code and private context.
7. `ADDRESS env` should be able to set the current/default address
   environment, with that state represented in the runtime/library layer
   rather than as a compiler-owned special case.
8. The first Stage 3 proof of concept should be a Rexx-written CMS/VM-style
   environment with a deliberately small and easy-to-implement command set.
   Hard-coded output is acceptable for this demo/test phase.
9. `rxvml` is the current external embedding entry point for environment
   registration and current-environment seeding. Any `RexxStart()`-style or
   `RexxRegisterSubcomExe()`-style adapter remains later work.
10. The canonical registration path should be the runtime/Rexx registration
    function itself. External embedders should prefer to call that path via
    `rxvml` rather than using a separate C-only registration model.
11. Native environments should still be represented as Rexx-visible address
    environment objects. Their methods may be implemented in C or plugin code,
    but the dispatch and registration model should remain unified.

### 5.2 Requirements emerging from iteration 1

1. Phase 1 should preserve the current VM spawn/redirect backend unless a
   change is necessary for correctness.
2. `ADDRESS` compiler handling should be moved out of the hard-coded validation
   rewrite path and into an exit-based or exit-shaped mechanism.
3. CREXX needs a notion of certified/system exits so core language keywords
   such as `ADDRESS` can be implemented by the exit framework without opening
   that capability to arbitrary user bundles.
4. The exit framework must be able to inform the compiler about more than
   hoisted names. At minimum it needs richer information for:
   - contextual keywords
   - typed exposures
   - aliases
   - pool exports
   - editor/highlighter support
5. The internal address-environment protocol should be separated from transport.
   A future `rexxsaa.h` adapter must be possible, but it must sit on top of the
   canonical modern CREXX protocol rather than define it.
6. Compatibility-facing variable-pool operations still need support.
7. The preferred modern authoring model should remain explicit and
   capability-based.
8. Redirect handling should evolve toward reusable endpoints that can later
   support pipelines built from Rexx, but the current phase only needs the
   abstraction layer and documented future potential.
9. The design should allow Rexx-implemented address environments for testing,
   emulation, or VM/CMS environment emulation.
10. The address environment protocol should use a common class shape across
    native, Rexx, local, and remote implementations.
11. Compiler-exit documentation must be revised alongside implementation to
    describe certified exits, richer planning responses, and parser/editor
    implications.
12. Runtime needs a notion of current/default address environment that
    explicit `ADDRESS env` can update without immediately requiring a command.
13. The first CMS-oriented environment should optimize for deterministic demo
    and test behaviour rather than faithful CMS implementation.
14. External registration in the current codebase should go through the same
    runtime/Rexx registration path used by Rexx code, with `rxvml` acting as
    the embedding call surface rather than defining a separate registration
    protocol.

## 6. Working design direction

This section records the current design direction. Some items below are now
approved direction; the remaining unresolved details are called out explicitly.

### 6.1 Compiler structure

Approved direction:

- keep `ADDRESS` surface syntax unchanged
- move implementation ownership into a certified first-party compiler exit or
  equivalent system-exit bridge
- keep lowering to the current `_address` / `spawn` VM path initially

This would untangle the current compiler logic without forcing a runtime
rewrite first.

### 6.2 Exit contract improvements

Approved need:

- replace the current string-only `pre_process()` contract with a structured
  planning response

Minimum information needed from that response:

- `status`
- `hoist` list
- optional type metadata for hoisted symbols
- contextual keyword information
- aliases for exported names
- pool exposure declarations
- other per-occurrence planning data

A good internal shape would be an object/class contract. If later needed for
loose coupling, the same payload can be serialized as JSON.

Stage 1 outcome required:

- propose the exact structured response shape that replaces the current
  space-separated `pre_process()` result

Documentation requirement:

- update the compiler-exit documentation to explain certified/system exits and
  the richer compiler-facing contract

### 6.3 Address environment protocol

Approved direction:

- define a standard-shaped address environment contract
- keep transport pluggable
- keep compatibility-style variable-pool operations in the modern address
  contract rather than only in an adapter layer
- keep current/default address-environment state in the runtime/library layer
  so explicit `ADDRESS env` can update it without inventing a compiler-only
  special case

That contract should be able to:

- track and update the current/default address environment
- receive command text
- receive redirect and exposure plans
- execute or delegate the command
- return RC and diagnostics
- apply allowed exports back to the Rexx side

This would support:

- native implementations
- Rexx implementations
- a future REXXSAA adapter over the canonical internal protocol
- a future JSON/socket adapter over the canonical internal protocol

### 6.4 Exposure model

Approved compatibility and preference split:

- compatibility support must cover:
  - explicit exposure forms such as `expose var`, `expose var as`, and
    `expose pool`
  - compatibility-style variable-pool operations where required
- the preferred modern source-level direction is `expose var`

Notes:

- `var` is a list concept, not a single-name-only form
- the external environment is expected to know the variable name and infer or
  negotiate its role from command semantics where possible

Interpretation:

- `var` exports explicit variable capabilities
- `var as` exports the same capability under a negotiated external name
- `pool` exports a map-like object, not unrestricted caller namespace access

Marker-based binding:

- an embedded-SQL-style marker approach is approved as an additional mechanism
  for community feedback and safety experiments
- markers would constrain access to explicitly referenced variables
- this should be treated as additive to, not automatically a replacement for,
  `expose var`
- the exact marker syntax should be proposed in Stage 1 with normal Rexx style
  and typical syntax in mind
- markers and `expose` bindings should merge into one internal list of Rexx
  variables plus external aliases rather than remain separate internal models

### 6.5 Redirect and pipeline model

Approved direction for this phase:

- do not change VM opcodes first
- first introduce an explicit redirect endpoint abstraction over the current
  `REDIRECT` implementation
- then evaluate whether runtime execution should remain per-endpoint-thread or
  move to a shared pump/evented model when true pipeline graphs are introduced
- document the future pipeline potential, but do not target executable
  multi-command pipelines in this phase

## 7. Stage 1 proposal topics

1. Propose the exact compiler-facing data shape that replaces the current
   space-separated `pre_process()` string response.
2. Define how compatibility-style variable-pool operations sit inside the
   modern address contract.
3. Propose exact marker syntax in a form that feels natural in Rexx source.
4. Define the merged internal binding model so `expose` and marker references
   feed one list of internal Rexx variables and external aliases.
5. Define conflict and duplicate-handling rules inside that merged binding
   list.

### 7.1 Certified/system exit ownership proposal

Proposal:

- split compiler exits into two classes:
  - general exits
  - certified/system exits
- only certified/system exits may claim:
  - built-in keywords
  - grammar-owned instructions such as `ADDRESS`
  - contextual keyword behaviour used by parser/highlighter integration
- general exits continue to use the current user/module discovery model for
  non-reserved primary keywords

Recommended ownership mechanism:

- the compiler should have an internal certified exit table
- each entry should define at minimum:
  - `primary_keyword`
  - `module_name`
  - `class_name`
  - `ownership_mode`
  - `feature_flags`
- `ownership_mode` should distinguish:
  - `reserved_keyword`
  - `implicit_command`
  - `hybrid`

Initial certified exit set:

- `ADDRESS` should be the first certified/system exit migrated under this model

Resolution rules:

- the certified exit table is authoritative for reserved keywords
- `RXCP_EXIT_MODULE` or other user-configured exit bundles must not be able to
  override a certified entry
- if a certified module cannot be loaded, the compiler should fail clearly
  rather than silently falling back to user exit resolution

Rationale:

- this preserves core-team control
- it keeps keyword ownership explicit
- it allows the exit framework to own core instruction behaviour without
  opening reserved syntax to arbitrary third-party bundles

Documentation impact:

- compiler-exit docs should explicitly distinguish general exits from
  certified/system exits
- the docs should state that certified/system exits are curated by the core
  team and may own reserved keywords

### 7.2 Structured planning contract proposal

Proposal:

- replace the string-only `pre_process()` contract with a compiler-owned
  planning object model
- keep `process()` and planning as separate phases
- support a compatibility fallback for old-style general exits during the
  transition

Recommended classes:

- `rxcp.exitplan`
- `rxcp.bindingplan`
- `rxcp.keywordclaim`
- `rxcp.exitresult`

Recommended method shape:

```rexx
pre_process: method = .exitplan
    arg tokens = .token[]

process: method = .exitresult
    arg tokens = .token[]
```

Recommended `exitplan` fields:

- `status`
  - `EMPTY`
  - `READY`
  - `PENDING`
  - `ERROR`
- `bindings = .bindingplan[]`
- `keywords = .keywordclaim[]`
- `error_token = .int`
- `error_message = .string`
- `notes = .string[]`

Recommended `bindingplan` fields:

- `kind`
  - `var`
  - `pool`
  - `compat_pool`
- `internal_name`
- `external_alias`
- `value_type`
- `dimensions`
- `provenance`
  - `expose`
  - `marker`
  - `compat`
- `flags`

Recommended `keywordclaim` fields:

- `token_index`
- `keyword_text`
- `keyword_role`
- `provenance`

Recommended `exitresult` fields:

- `status`
  - `REJECT`
  - `PENDING`
  - `ACCEPT`
  - `REPLACE`
  - `ERROR`
- `replacement`
- `error_token`
- `error_message`

Compatibility plan:

- during migration, the compiler may continue to accept the current
  space-separated string from old `pre_process()` methods for general exits
- certified/system exits should move directly to the structured contract
- once the model is stable, the legacy string response can be deprecated for
  new exits

Why this shape:

- the compiler needs more than hoist indices
- the bridge already deals with stateful exit objects, so a class/object return
  model fits the existing architecture better than inventing an ad hoc string
  encoding
- if loose coupling is needed later, the same logical object can be serialized
  to JSON without changing the conceptual model

### 7.3 Contextual keyword proposal

Problem:

- some exits need contextual keyword awareness for parser mode and editor
  highlighting
- static global additional-keyword registration is too blunt

Proposal:

- `keywordclaim[]` in `exitplan` should declare contextual keyword claims for
  one exit occurrence
- claims should be scoped to the current instruction, not promoted globally
- the parser/highlighter should treat claimed tokens as exit-context keywords
  for that exit instance only

Recommended behaviour:

- exact token positions are preferred over text-only global claims
- parser mode and syntax highlighting should consume the same claim data so
  editor behaviour stays aligned with compiler behaviour
- claims should be advisory to parsing/highlighting, not a general mechanism for
  rewriting the language grammar

Immediate beneficiary:

- the `parse` exit model, which needs exit-local keyword awareness

### 7.4 Common address environment contract proposal

Proposal:

- define a common class-shaped runtime contract for address environments
- represent every environment as a Rexx-visible object implementing that
  contract
- use the same logical contract for:
  - Rexx-written environments
  - native-backed environments whose methods are implemented in C/plugin code
  - remote environments
  - compatibility adapters
- keep registration unified around the runtime/Rexx registration function,
  regardless of whether the caller originates in Rexx or in an external
  `rxvml` embedder

Recommended classes:

- `rxcp.addressrequest`
- `rxcp.addressresponse`
- `rxcp.addressbinding`
- `rxcp.addressenvironment`
- `rxcp.addresspool`

Recommended `addressenvironment` method shape:

```rexx
*: factory = .addressenvironment
    arg env_name = .string

execute: method = .addressresponse
    arg request = .addressrequest
```

Provider selection shape:

```rexx
someenvironment: class implements .addressenvironment
  *: match
    arg env_name = .string
    /* return positive score to claim the environment, 0 to reject */

  *: factory
    arg env_name = .string
    /* initialise the selected provider object */
```

Interpretation:

- `addressenvironment` is both the common runtime interface and the factory
  surface for dynamic environment discovery.
- providers should expose a `match` method next to the factory; this is the
  same class/interface factory-provider mechanism documented for Level B
  classes.
- `_address` should ask `.addressenvironment(name)` for providers on demand
  and then cache the resulting object in the runtime registry.
- high scores win; `0` rejects; a low-scoring unknown/failure provider may be
  used as a safe fallback to keep dispatch errors representable as
  `addressresponse` values.

Canonical registration shape:

```rexx
rc = _register_address_environment(name, env_obj)
rc = _set_address_environment(name)
```

Interpretation:

- `env_obj` should be the canonical environment object, not a parallel
  registration record
- an external `rxvml` embedder should be able to:
  - load the module defining the environment class or object
  - construct or obtain the environment object
  - call the same registration function that Rexx code calls
- a native environment should still be surfaced as a Rexx-visible object, with
  methods dispatched normally; the implementation behind those methods may be C
  or plugin code
- native declarations can now expose callable procedures plus class/interface
  contract metadata through RXPA macros, so the compiler and runtime can see
  that a C provider class implements an interface; full pure-C object
  construction still needs either a Rexx factory shim or a future RXPA helper
  that can stamp a native-created return value as a typed Rexx object
- if convenience wrappers are later added on `rxvml_context`, they should
  delegate to this canonical registration path rather than define a second
  registration model

Recommended `addressrequest` fields:

- `environment_name`
- `command`
- `bindings = .addressbinding[]`
- `stdin_endpoint`
- `stdout_endpoint`
- `stderr_endpoint`
- `compat_pool = .addresspool`
- `flags`

Recommended `addressresponse` fields:

- `rc`
- `updated_bindings = .addressbinding[]`
- `diagnostics = .string[]`
- `condition_name`

Recommended `addressbinding` fields:

- `kind`
  - `var`
  - `pool`
  - `compat_pool`
- `internal_name`
- `external_alias`
- `value`
- `flags`

Recommended `addresspool` methods:

- `fetch(name)`
- `set(name, value)`
- `drop(name)`
- `next(cursor)`
- `exists(name)`

Compatibility statement:

- compatibility-style variable-pool access should be represented through the
  common contract, not as a separate hidden side channel
- a future `rexxsaa.h` adapter should translate `RexxVariablePool()` and
  related behaviour onto `addresspool`

Phase-1 implementation guidance:

- the compiler can still lower to `_address(...)`
- `_address` should be viewed as a temporary dispatcher/backend shim
- the current spawn backend becomes one concrete environment implementation
  behind the shared request/response model

Current-address state proposal:

- the runtime should keep the current/default address environment as
  library-owned state rather than as a compiler rewrite artifact
- explicit `ADDRESS env` should update that state without immediately executing
  a command
- implicit commands should resolve through that current environment
- if no current environment has been set yet, the initial fallback should
  remain `SYSTEM`
- external embedders should be able to seed or override this state through the
  current `rxvml` entry point

### 7.5 Exposure syntax proposal

Preferred modern syntax:

```rexx
address shell cmd expose var userid orderid
address shell cmd expose var userid as "USERID"
address shell cmd expose pool session
```

Interpretation:

- `expose var` declares explicit variable capabilities
- `expose var ... as "alias"` declares explicit aliasing
- `expose pool` declares a named map-like pool capability

Compatibility stance:

- compatibility-oriented environments may still use broader variable-pool
  behaviour
- the preferred modern authoring experience should still encourage explicit
  `expose var` and `expose pool`

### 7.6 Marker syntax proposal

Proposal:

- support host-variable style markers inside literal command text
- markers are additive to `expose`, not a replacement for it

Recommended syntax:

- `:name`
  - internal Rexx variable `name`
  - external alias defaults to `name`
- `:name(alias)`
  - internal Rexx variable `name`
  - external alias `alias`
- `::`
  - escapes a literal colon

Examples:

```rexx
address sql "select * from users where userid = :userid"
address cms "listfile :fn(FILE) :ft(TYPE) :fm(MODE)"
address shell "run-tool --user :userid(USERID)"
```

Recommended parsing rule:

- markers should be recognized only inside literal command text segments that
  are available to the compiler exit during planning
- markers synthesized later through runtime-only string construction should not
  silently expand the accessible variable set

Reasoning:

- `:name` is familiar from embedded SQL and compact enough for command text
- `:name(alias)` keeps aliasing local to the marker without introducing a new
  mini-language
- keeping marker discovery tied to compiler-visible literal text preserves the
  safety goal

### 7.7 Merged binding model proposal

Proposal:

- all explicit `expose` bindings and all marker-derived bindings should be
  normalized into one binding list before runtime dispatch
- the environment should see one list of:
  - internal Rexx names
  - external aliases
  - binding kinds

Normalization rules:

- marker-only references should implicitly create `var` bindings
- explicit `expose` entries should add bindings even if no marker references
  exist
- exact duplicate bindings should be coalesced
- the normalized uniqueness key should be:
  - `kind`
  - `internal_name`
  - `external_alias`

Conflict rules:

- same external alias mapped to different internal names is a compile-time
  error
- same internal name mapped to multiple external aliases is allowed
- an explicit alias wins over an implicit same-name alias when both describe the
  same binding intent
- conflict diagnostics should name both the internal name and the external
  alias involved

Result:

- the runtime environment receives one coherent binding table
- there is no separate internal model for marker bindings versus `expose`
  bindings
- compatibility pool capabilities can live alongside explicit bindings in the
  same request object without being confused with them

### 7.8 Documentation scope proposal

Proposal:

- update the general compiler-exit docs to distinguish:
  - current general exits
  - planned certified/system exits
  - current string-based planning
  - proposed structured planning objects
- keep the working note as the canonical design record until implementation
  lands
- once code starts landing, fold approved parts of this note into:
  - compiler exit docs
  - language reference
  - `ADDRESS` reference text
  - compatibility notes for REXXSAA/CMS-style environments

## 8. Proposed staged plan

### Stage 1: architecture and contract

Status: implemented

Stage 1 deliverables:

- define certified/system exit ownership rules
- define the richer exit planning contract
- propose the exact structured `pre_process()` replacement shape
- define the common-class address environment protocol
- define exposure semantics, compatibility boundaries, and security rules
- define compatibility-style variable-pool support in the modern address
  contract
- propose marker-based binding syntax in a Rexx-appropriate style
- define the merged binding model for `expose` and marker references
- update compiler-exit documentation to match the certified/system-exit model

Stage 1 exit criteria:

- one approved proposal for the richer exit-planning response
- one approved proposal for marker syntax, or an explicit decision not to ship
  markers in the first implementation wave
- one approved definition of the merged internal binding model
- one approved statement of how compatibility pool operations appear in the
  modern address contract
- an agreed documentation update scope for compiler exits and `ADDRESS`

### Stage 2: compiler refactor

Status: implemented, including parser/AST cleanup

Completed in this stage:

- route `ADDRESS` through the new system-exit path
- mark `PARSE` as a certified exit and move its contextual keyword handling
  onto the same model
- keep the existing `_address` / `spawn` runtime backend
- add focused tests for:
  - explicit `ADDRESS`
  - implicit address
  - redirects
  - `EXPOSE`
  - parser/highlighter keyword handling
  - direct Rexx-side exit harness coverage for `pre_process()` and `process()`
- remove legacy `ADDRESS` parsing from `compiler/rxcpbgmr.y`
- remove obsolete `ADDRESS` AST artifacts and dedicated parser-era ownership
  assumptions
- remove dead keyword/parser support left behind by the migration once the new
  path was trusted

### Stage 2.5: exit protocol unification

Status: complete

Deliverables:

- make `describe()` / `pre_process()` / `process()` mandatory for all exits
- remove legacy compatibility from the compiler bridge
- move exit-owned imports from compiler metadata into exit-declared descriptor
  or plan data
- make multiline replacement text first-class
- declare helper procedures in `pre_process()` so file-tail helpers can be
  appended before symbol harvesting
- update bundled exits, tests, and docs together

### Stage 3: runtime abstraction

Status: Stage 3.1 complete; Stage 3.2 complete with interface-based concrete
environment classes

Stage 2 and Stage 2.5 are complete. The next implementation work starts here.

Overall goals:

- make environment handling real instead of ignored
- introduce a true runtime environment registry and current-environment state
- keep `_address(...)` as the dispatcher shim during the transition
- prepare redirect/runtime structures for future pipelines

### Stage 3.1: Rexx-written environment proof of concept

Status: complete for approved POC scope on 2026-04-19

Implemented in this slice:

- `_address(...)` now dispatches through runtime request/response objects
- the runtime now owns a named environment registry plus current/default
  environment state
- explicit `ADDRESS env` now updates that current/default environment without
  requiring a command
- implicit command dispatch now resolves through the current/default
  environment, with `SYSTEM` as the fallback
- `SYSTEM`, `CMD`, and `SHELL` currently share the existing spawn-backed
  implementation
- a Rexx-written `CMS` proof-of-concept environment is registered with a
  deterministic demo command set:
  - `CP QUERY USERID`
  - `CP SET MSG ON`
  - `CP SET MSG OFF`
  - `LISTFILE`
  - `TYPE`
- focused runtime/compiler coverage now exercises:
  - explicit `ADDRESS env`
  - implicit command dispatch after environment changes
  - the CMS proof-of-concept commands
  - existing redirect behaviour

Known limitation of the current POC:

- the current implementation proves registration/current-environment state and
  dispatch, but it still uses one `addressenvironment` class with internal kind
  switching rather than separate concrete environment classes/objects for
  `SYSTEM`, `CMS`, and later environments
- that was acceptable for the proof of concept but does not fully exercise the
  intended environment class shape as a first-class object model
Primary goal:

- prove the address-environment contract with a Rexx-written environment before
  adding external registration surfaces or native-environment generalization

Deliverables:

- turn `_address` into a dispatcher/backend shim over the approved common
  request/response contract
- add runtime registration and lookup for named address environments
- add a runtime-owned current/default address-environment variable
- make explicit `ADDRESS env` update that current/default environment
- make implicit command dispatch resolve through the current/default
  environment, with `SYSTEM` as the initial fallback
- register the existing spawn backend as the concrete `SYSTEM`
  implementation so current behaviour remains available
- implement one Rexx-written `CMS` proof-of-concept environment for demo/test
  use

Approved scope for the `CMS` proof of concept:

- use a deliberately small, easy-to-implement command set
- command behaviour may be hard-coded and deterministic
- the goal is environment registration, dispatch, and syntax/runtime proof,
  not faithful CMS emulation

Initial demo command set target:

- `CP QUERY USERID`
- `CP SET MSG ON`
- `CP SET MSG OFF`
- `LISTFILE`
- `TYPE`

Non-goals for Stage 3.1:

- full CMS or VM command semantics
- full compatibility variable-pool support
- redirect-endpoint refactoring
- native/non-Rexx environment registration
- any `RexxStart()` or REXXSAA adapter work

### Stage 3.2: canonical registration and bridge startup path

Status: complete on 2026-04-23

Implemented in this slice:

- keep `_register_address_environment(name, env_obj)` as the canonical runtime
  registration operation
- add thin `rxvml` helper wrappers that delegate to
  `_register_address_environment(...)` and `_set_address_environment(...)`
- add focused bridge/embedding coverage around registration and
  current-environment seeding
- close the environment set for this stage at:
  - `SYSTEM`
  - `COMMAND`
  - `PATH`
  - `CMS`
  - plus long-term synonyms `CMD` and `SHELL`
- represent the common callable contract as a real Level B
  `addressenvironment` interface
- register separate concrete Rexx environment objects behind that contract:
  - `.systemaddressenvironment`
  - `.pathaddressenvironment`
  - `.cmsaddressenvironment`
- keep `SYSTEM`, `COMMAND`, `CMD`, and `SHELL` on the existing spawn-backed
  behaviour via the registered `systemaddressenvironment` object
- keep `PATH` on the same current spawn-backed transport for now; on POSIX this
  already resolves the executable through process `PATH`, but it now has its
  own concrete `pathaddressenvironment` class/object
- keep `CMS` as the deterministic Rexx-written test/demo environment, now
  implemented as its own concrete `cmsaddressenvironment` class/object
- make bridge/native construction follow the same source-visible class surface
  by obtaining environment objects through class factories rather than
  procedure-only helpers

What this stage intentionally still does not do:

- it does not yet provide the fully general native-backed environment object
  model originally sketched for later Stage 3 work

Conclusion for Stage 3.2:

- the runtime now uses the intended source-level object model for this stage:
  one shared `addressenvironment` contract with multiple concrete
  implementations registered under environment names and aliases
- this closes the Stage 3.2 follow-on that was previously deferred behind the
  Level B callable-contract / interface work
- see `compiler/docs/classes.md` for the enduring compiler-side reference for
  the callable-contract machinery that now enables this model

Rationale:

- `rxvml` is the real embedding surface in the current codebase
- using the same registration path for Rexx and external callers avoids
  bifurcating the model
- this closes the practical runtime-registration/startup goal for Stage 3.2
  using the real Level B interface dispatch model rather than an ad hoc
  pseudo-interface mechanism inside `ADDRESS` itself

### Stage 3.3: modern `rxvml` native address callback contract

Status: implemented in debug build; verification green on 2026-04-23

Primary goal:

- let a C host embed CREXX through `rxvml`, register a native callback-backed
  address environment, start Rexx code, and receive calls from Rexx
  `ADDRESS` dispatch without introducing a REXXSAA compatibility layer first

Design intent:

- the modern `rxvml` contract is canonical for this stage
- REXXSAA source-compatible wrappers remain a longer-term adapter over this
  modern contract
- Rexx-written and native-backed environments still share one logical
  `addressenvironment` object model
- the C callback API should be simple enough for host applications and LLM
  debugging clients to exercise directly
- the first implementation should optimize for observability and testability,
  not full compatibility coverage

Deliverables:

- added a public `rxvml` callback registration surface:
  - `rxvml_address_register_callback_environment(ctx, name, callback, userdata)`
  - `rxvml_address_callback(ctx, request, response, userdata)`
- defined small C request/response structs for the first callback contract:
  - environment name
  - command string
  - binding count
  - binding kind, internal name, external alias, value, and flags
  - return code
  - optional condition/diagnostic text
- implemented the native callback path as a Rexx-visible
  `nativeaddressenvironment` proxy that
  implements `.addressenvironment`, rather than as a parallel environment
  registry
- kept `_register_address_environment(name, env_obj)` as the canonical runtime
  registration path; the new `rxvml` helper should delegate into that path
  after constructing or obtaining the proxy object
- preserved current/default environment semantics:
  - explicit `ADDRESS env` can still seed or switch the current environment
  - implicit commands still dispatch to the current environment
- added a deliberately simple dummy C host/client test that:
  - creates an `rxvml_context`
  - loads the standard library and a tiny Rexx fixture
  - registers an `EDITOR` environment backed by a C callback
  - runs Rexx code that calls `ADDRESS EDITOR`
  - verifies explicit `ADDRESS`, implicit command dispatch after `ADDRESS
    EDITOR`, binding/expose capture, and RC propagation
- keep the dummy client boring and debugger-friendly:
  - record call count
  - record the last command
  - record exposed bindings
  - return deterministic RC values for known commands
  - fail loudly with useful diagnostics when callback flow is not reached

Implemented dummy Rexx fixture shape:

```rexx
address editor "OPEN demo.txt" expose buffer
address editor
"CURSOR 7 9"
address "" "RETURN 42"
```

Implemented dummy callback behaviour:

- `OPEN ...` records exposed bindings and returns `0`
- `CURSOR` proves implicit command dispatch after `ADDRESS EDITOR`
- `RETURN 42` returns `42` to prove callback RC propagation

Implemented first C API shape:

```c
typedef struct rxvml_address_binding {
    const char *kind;
    const char *internal_name;
    const char *external_alias;
    const char *value;
    const char *flags;
} rxvml_address_binding;

typedef struct rxvml_address_request {
    const char *environment_name;
    const char *command;
    size_t binding_count;
    const rxvml_address_binding *bindings;
} rxvml_address_request;

typedef struct rxvml_address_response {
    int rc;
    const char *condition_name;
    const char *diagnostic;
} rxvml_address_response;

typedef int (*rxvml_address_callback)(
    rxvml_context *ctx,
    const rxvml_address_request *request,
    rxvml_address_response *response,
    void *userdata);
```

The string pointers supplied in `rxvml_address_request` are callback-duration
views over VM values. A host that needs to retain them after the callback should
copy them.

Stage 3.3 exit criteria:

- a C-only dummy host can register `EDITOR`, run Rexx, and observe
  `ADDRESS EDITOR` callbacks
- explicit and implicit ADDRESS dispatch both reach the native callback
- exposed binding names/aliases/values are visible to the callback
- callback RC propagates back to Rexx
- existing Rexx-written environments (`SYSTEM`, `PATH`, `CMS`) still pass
  their current tests
- the public `rxvml` API remains independent of REXXSAA naming and ABI details

Stage 3.3 factory/provider refinement:

Status: working proposal implemented on 2026-04-24

Reason for the refinement:

- the first Stage 3.3 tracer bullet proved callback dispatch, but the
  registration path still manually constructed a native proxy object by handle
  and the CMS fake environment still lived inside the core `_address.rexx`
  module
- the intended design is cleaner if all environments are providers for the
  `.addressenvironment(name)` interface factory
- this factory-provider shape is also a natural prerequisite for later stages:
  `ADDRESS` dispatch can discover pure Rexx, hybrid Rexx/C, and eventually
  pure C providers through one contract

Implemented refinement:

- `.addressenvironment` now declares the default factory:

```rexx
*: factory = .addressenvironment
  arg env_name = .string
```

- concrete providers implement class-side `*: match` and `*: factory`
  members taking the same `env_name` argument
- `_new_address_environment(name)` creates an environment object through the
  interface factory
- `_ensure_address_environment(name)` materialises and caches a provider on
  demand
- `_set_address_environment(name)` and request dispatch now call the same
  dynamic ensure path rather than relying on a fixed startup registry
- `SYSTEM`, `COMMAND`, `CMD`, and `SHELL` are selected by
  `systemaddressenvironment`
- `PATH` is selected by `pathaddressenvironment`
- a low-score `unknownaddressenvironment` gives unknown names a normal
  `addressresponse` failure object instead of requiring factory exceptions for
  normal dispatch errors
- `CMS` has been moved out of `_address.rexx` into
  `lib/rxfnsb/rexx/cmsaddress.rexx`, a pure Rexx provider implementing
  `.addressenvironment`
- `cmsaddress.rexx` imports the ADDRESS helper classes/procedures and implements
  the imported `_rxsysb.addressenvironment` interface directly
- `cmsaddress.rexx` is dynamically selected by `.addressenvironment("CMS")`
- `nativeaddressenvironment` is now a hybrid provider:
  - Rexx declares the provider class, `match`, and factory
  - `rxvml.c` provides `_native_address_match`,
    `_native_address_handle`, and `_native_address_execute`
  - host callback registrations are keyed by normalised environment name
  - native match scores higher than built-in providers so an embedder can
    intentionally override a shipped name
  - ordinary `rxvm` execution does not call `rxvml`-only native hooks; `rxvml`
    enables the native provider before constructing native callback
    environments
- `rxvml_address_register_callback_environment(...)` now creates the provider
  through the interface factory and registers the resulting object through the
  canonical `_register_address_environment(...)` path
- `rxvml_address_create_environment(ctx, name, &obj)` has been added as a
  small C helper for clients/tests that want to obtain any dynamic provider
  through the canonical factory

Current provider routes:

- pure Rexx: define a Rexx class implementing `.addressenvironment` with
  `*: match`, `*: factory`, and `execute`
- separate provider modules can implement imported interface factory contracts;
  the compiler signature checker now treats implicit class-factory returns as
  the enclosing class and accepts concrete class returns assignable to the
  imported interface return type
- hybrid Rexx/C: define the class/factory in Rexx and delegate selected
  methods to native functions declared through RXAS metadata
- pure C: source-declarable at the contract level through RXPA metadata
  macros. This means a native module can publish "there is a class/interface
  with these factories and methods" so imports and type checks work. It does
  not yet mean native C can allocate a fully typed Rexx object instance with
  class identity and method dispatch. Providers that need full object creation
  still need either a Rexx factory/class shim or a future RXPA construction
  helper.

Implemented pure-C declaration slice:

- RXPA/native declaration macros now publish class/interface metadata in the
  same logical shape as compiler-emitted RXAS metadata:
  - `ADDCLASS(class_name)`
  - `ADDINTERFACE(interface_name)`
  - `ADDIMPLEMENTS(class_name, interface_name)`
  - `ADDFACTORY(class_name, factory_name, return_type, args)`
  - `ADDMETHOD(class_name, method_name, return_type, args)`
- the declaration path works for dynamically loaded `.rxplugin` modules and
  statically linked `DECL_ONLY` compiler declaration libraries
- the VM records native plugin class/interface metadata in the native module
  constant pool, so runtime factory/provider discovery can inspect the same
  contract metadata for native modules

Recommended remaining pure-C follow-on:

- add an RXPA object-construction API for native factories that need to return
  typed class instances without a Rexx shim. This helper should cover the
  currently missing operation: creating or obtaining a Rexx object that really
  has the declared class/interface type, rather than just filling a primitive
  RXPA return slot.
- native-backed object payloads should use the VM binary payload slot plus
  shared native payload operations when the payload owns C resources. The
  provider must supply a finalizer and, for unique resources, a copy hook that
  retains/clones/duplicates the resource. If no copy hook is supplied the VM
  byte-copies the payload, so bit-copy safety becomes the provider's
  responsibility.
- consider ADDRESS-specific convenience macros once the lower-level contract
  remains stable
- until native object construction exists, a hybrid Rexx provider class backed
  by C functions remains the cleanest complete ADDRESS provider pattern

Stage 3.2 prerequisite status:

- the required Level B callable-contract / interface mechanism is now in place
  and applied to `ADDRESS`

Explicitly deferred from Stage 3.3:

- full REXXSAA subcommand registration compatibility
- REXXSAA `RexxStart()` / `RexxRegisterSubcomExe()` style source-compatible
  wrappers
- full variable-pool writeback and `RexxVariablePool()` compatibility
- redirect endpoint abstraction and host-side stream pumping
- transport-specific remote adapters

### Stage 3.4: redirect endpoint abstraction

Primary goal:

- introduce redirect endpoint objects over current `REDIRECT` once environment
  dispatch is real and testable

Deliverables:

- wrap the existing redirect machinery in endpoint-shaped runtime objects
- prepare for future pipeline graphs without changing VM opcodes first
- keep executable multi-command pipelines out of scope for this stage

### Stage 4: compatibility and transport adapters

- prototype an optional REXXSAA adapter
- prototype an optional loose-coupled transport, likely JSON/socket or similar
- decide which adapters ship and at what support level

## 9. Progress

- 2026-04-17: iteration 1 working note created
- 2026-04-17: current compiler/runtime review captured
- 2026-04-17: first pass of requirements, design direction, and staged plan
  recorded
- 2026-04-17: core direction approved:
  - `ADDRESS` may become a certified exit
  - the internal modern protocol is canonical
  - the preferred exposure direction is explicit `expose var`
  - compatibility support remains mandatory
  - redirect work is abstraction-only in this phase
  - address environments should follow a common class shape
- 2026-04-17: further direction approved:
  - compatibility-style variable-pool support belongs in the modern address
    contract
  - marker-based binding should be prototyped
  - marker syntax should be proposed during Stage 1
  - `expose` and marker bindings should merge into one internal list of Rexx
    variables and external aliases
  - Stage 1 is now in progress
- 2026-04-17: Stage 1 proposals documented:
  - certified/system exit ownership model
  - structured planning contract
  - contextual keyword claim model
  - common address request/response/pool contract
  - preferred `expose` forms and marker syntax proposal
  - merged binding normalization and conflict rules
- 2026-04-17: Stage 2 implementation landed:
  - `ADDRESS` and `PARSE` now run through the certified exit path
  - structured `exitplan` planning is active for binding hoists and contextual
    keyword claims
  - implicit command dispatch now falls back to the certified `ADDRESS` exit
  - redirect handling defers until operand typing is known, so array redirects
    lower correctly
  - focused direct Rexx-side exit harness tests now cover `pre_process()` and
    `process()` for `ADDRESS` and `PARSE`
- 2026-04-17: post-Stage-2 stabilisation completed:
  - `EXECIO` moved to typed binding-hoist planning for `STEM`
  - build dependencies for exit-compiled Rexx artifacts were corrected
  - legacy string exit responses are no longer misidentified as
    `rxcp.exitplan` objects, which removed the dummy-exit
    `PANIC: (SIGNAL OUT_OF_RANGE)` noise
  - full verification is green again:
    - `cmake --build cmake-build-debug`
    - `ctest --test-dir cmake-build-debug --output-on-failure`
    - result: `697/697` tests passed
- 2026-04-17: next implementation task agreed:
  - remove legacy `ADDRESS` parsing and AST artifacts now that the certified
  exit path is working
  - then do the associated grammar/parser/highlighter cleanup for long-term
  maintainability
- 2026-04-17: post-Stage-2 parser cleanup completed:
  - explicit `ADDRESS` now parses only through certified exit promotion
  - legacy `ADDRESS` and redirect AST node types were removed
  - `rxc -x` now rejects explicit certified exits such as `ADDRESS` and
    `PARSE` with `#CERTIFIED_EXIT_DISABLED`
  - inline instruction sites such as `if ... then address ...` now use the
    same certified-exit promotion path instead of falling through to
    `IMPLICIT_CMD`
- 2026-04-18: enabled `PARSE` follow-up fixes completed:
  - certified exits can now declare required runtime imports, and `PARSE`
    auto-imports `rxfnsb` instead of depending on a handwritten source import
  - the `PARSE` replacement now quotes generated runtime string arguments,
    which fixes templates containing double-quoted literals such as `","`
  - inline certified exits under `IF` / `WHEN` / `OTHERWISE` now wrap branch
    bodies structurally before hoisting bindings, while preserving the parent
    lexical scope so later constant-folding and control-flow rewrites do not
    drop or localize the rewritten statement
- 2026-04-18: Stage 2.5 exit protocol unification completed:
  - all bundled exits now implement the V2 `describe()` / `pre_process()` /
    `process()` contract
  - the compiler bridge no longer accepts legacy string-plan or getter-based
    result shapes
  - exit-owned imports now come from descriptor or plan data instead of
    compiler hardcoding
  - replacement text is now multiline-capable through `replacement_lines`
  - helper procedures are declared structurally during `pre_process()`
  - bridge token marshalling now preserves semantic token categories for
    certified-exit payloads instead of flattening every `EXIT_TOKEN` to one
    raw kind
  - full verification is green again:
    - `cmake --build cmake-build-debug`
    - `ctest --test-dir cmake-build-debug --output-on-failure`
    - result: `704/704` tests passed
- 2026-04-18: final Stage-2/2.5 stabilisation completed:
  - incomplete `PARSE` input such as bare `parse` or `parse into` now reports
    normal user-facing protocol diagnostics instead of throwing bridge/runtime
    exceptions
  - exit-authored diagnostics are now reported as user diagnostics, while real
    bridge failures remain internal bridge diagnostics
  - editor/highlighter coverage now includes incomplete `PARSE` forms
  - Stage 2 and Stage 2.5 are complete; Stage 3 runtime abstraction is the
    next active phase
- 2026-04-18: Stage 3 roadmap refined and approved:
  - `ADDRESS env` should set the current/default address environment through
    runtime/library-owned state
  - the first proof of concept should be a Rexx-written `CMS` environment with
    a tiny deterministic command set and hard-coded output where useful
  - `SYSTEM` should remain available as the spawn-backed default environment
    during the transition
  - `rxvml` is the current external embedding entry point for environment
    registration and current-environment seeding
  - Stage 3 is now split into:
    - Stage 3.1 Rexx environment proof of concept
    - Stage 3.2 `rxvml` startup registration
    - Stage 3.3 generalized Rexx/non-Rexx environment model
    - Stage 3.4 redirect endpoint abstraction
- 2026-04-18: Stage 3.1 prototype implemented:
  - `_address(...)` now dispatches through runtime request/response objects
  - runtime now owns the current/default address environment and named
    environment registry
  - explicit `ADDRESS env` now updates the current/default environment
  - implicit commands now resolve through the current/default environment
  - `SYSTEM` remains the spawn-backed default backend
  - a Rexx-written `CMS` environment is registered with hard-coded demo
    commands for `CP QUERY USERID`, `CP SET MSG`, `LISTFILE`, and `TYPE`
  - focused verification is green for `test_address*` and `ts_address*`
- 2026-04-19: Stage 3.1 reviewed and marked complete for the approved proof of
  concept scope:
  - the current POC is sufficient to close Stage 3.1, but it leaves one known
    follow-on: `SYSTEM` and `CMS` still sit behind a kind-switched
    implementation instead of separate concrete environment classes/objects
  - the roadmap is now adjusted so Stage 3.2 closes that object-shape gap while
    also making the runtime/Rexx registration function the canonical
    registration path for both Rexx code and external `rxvml` embedders
  - Stage 3.3 is now framed as unified native-backed environment objects rather
    than a separate Rexx vs non-Rexx registration model
- 2026-04-23: Stage 3.2 completed on top of the shipped callable-contract /
  interface model:
  - `addressenvironment` is now a real interface rather than a kind-switched
    concrete class
  - `SYSTEM`, `PATH`, and `CMS` now register separate concrete environment
    objects implementing that shared contract
  - bridge coverage now constructs the environment object through the exposed
    class factory before registering it through the canonical runtime path
- 2026-04-23: Stage 3.3 direction approved and refined:
  - the next implementation slice targets a modern `rxvml` native address
    callback contract, not a REXXSAA compatibility layer
  - the first proof/debug target is a simple C dummy host that registers an
    `EDITOR` address environment and verifies Rexx `ADDRESS` calls back into C
  - REXXSAA source-compatible wrappers remain a longer-term adapter over the
    modern `rxvml` contract
- 2026-04-23: Stage 3.3 implemented and verified:
  - `rxvml.h` now exposes the native ADDRESS callback request/response
    contract and `rxvml_address_register_callback_environment()`
  - `rxvml.c` registers a static native `_rxsysb._native_address_execute`
    trampoline, owns callback handles per `rxvml_context`, and delegates
    registration through the canonical Rexx `_register_address_environment`
    path via `.nativeaddressenvironment`
  - `compiler/tests/src/test_address_callback_host.c` and
    `compiler/tests/rexx_src/address_callback_host.rexx` provide the dummy
    host/fixture for debugger-friendly validation
  - focused ADDRESS/interface/bridge verification passed `82/82`
  - full debug CTest passed `852/852`
- 2026-04-24: Stage 3.3 factory/provider refinement implemented:
  - `.addressenvironment` now owns the dynamic factory contract and providers
    advertise themselves with class-side `*: match`/`*: factory`
  - `_address` creates providers on demand through `.addressenvironment(name)`
    and caches them in the runtime registry
  - CMS moved from the core `_address.rexx` module to the pure Rexx provider
    `lib/rxfnsb/rexx/cmsaddress.rexx`
  - native callback environments now use Rexx-visible provider matching backed
    by `_native_address_match`, `_native_address_handle`, and
    `_native_address_execute`
  - `rxvml_address_create_environment()` exposes the canonical factory path to
    C callers
  - compiler factory feedback fixed imported-interface provider signature
    checking for implicit class-factory returns
  - focused ADDRESS/interface/bridge tests passed `85/85`
  - full debug CTest passed `856/856`
- 2026-04-24: RXPA class/interface declaration slice implemented:
  - `crexxpa.h` now exposes native declaration macros for classes,
    interfaces, implementations, factories, methods, and default methods
  - dynamic plugin import and static `DECL_ONLY` compiler import both consume
    the new metadata
  - native plugin loading now records class/interface metadata in the VM module
    constant pool for runtime factory/provider discovery
  - focused RXPA/ADDRESS/interface coverage passed `89/89`
  - full debug CTest passed `860/860`

## 10. Evidence and code anchors

- `compiler/rxcpbgmr.y`
- `compiler/rxcp_val_trans.c`
- `compiler/rxcp_val_orch.c`
- `compiler/rxcp_exit.c`
- `compiler/rxcpbpar.c`
- `compiler/exits/address/Address.rexx`
- `compiler/exits/parse/Parse.rexx`
- `compiler/exits/execio/Execio.rexx`
- `lib/rxfnsb/rexx/_address.rexx`
- `lib/rxfnsb/rexx/cmsaddress.rexx`
- `lib/rxfnsb/rxas/_rxvml_address_native.rxas`
- `interpreter/rxvml.h`
- `interpreter/rxvml.c`
- `interpreter/rxspawn.c`
- `interpreter/rxvmintp.c`
- `compiler/tests/src/test_bridge.c`
- `rexxsaa.h`
- `docs/books/crexx_language_reference/statements.md`
- `docs/books/crexx_vm_spec/Level-B-Grammar.tex`
- `docs/books/crexx_vm_spec/Level-c-Grammar.tex`
