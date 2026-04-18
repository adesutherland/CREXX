# ADDRESS and REXXSAA Working Notes

Status: draft, direction partially approved
Last updated: 2026-04-17

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

## 5. Approved decisions and requirements

### 5.1 Approved decisions as of 2026-04-17

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

That contract should be able to:

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
- use the same logical contract for:
  - native environments
  - Rexx environments
  - remote environments
  - compatibility adapters

Recommended classes:

- `rxcp.addressrequest`
- `rxcp.addressresponse`
- `rxcp.addressbinding`
- `rxcp.addressenvironment`
- `rxcp.addresspool`

Recommended `addressenvironment` method shape:

```rexx
execute: method = .addressresponse
    arg request = .addressrequest
```

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

Status: next active phase

Stage 2 and Stage 2.5 are complete. The next implementation work starts here.

- introduce redirect endpoint objects over current `REDIRECT`
- make environment handling real instead of ignored
- prepare runtime structures for future pipelines

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

## 10. Evidence and code anchors

- `compiler/rxcpbgmr.y`
- `compiler/rxcp_val_trans.c`
- `compiler/rxcp_val_orch.c`
- `compiler/rxcp_exit.c`
- `compiler/rxcpbpar.c`
- `compiler/exits/parse/Parse.rexx`
- `compiler/exits/execio/Execio.rexx`
- `lib/rxfnsb/rexx/_address.rexx`
- `interpreter/rxspawn.c`
- `interpreter/rxvmintp.c`
- `rexxsaa.h`
- `docs/books/crexx_language_reference/statements.md`
- `docs/books/crexx_vm_spec/Level-B-Grammar.tex`
- `docs/books/crexx_vm_spec/Level-c-Grammar.tex`
