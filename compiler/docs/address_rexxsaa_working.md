# ADDRESS and REXXSAA Working Notes

Status: retired working record; as-built details have moved to stable docs
Last updated: 2026-04-26

Retirement note: the current implemented `crexxsaa` host API, ADDRESS callback
model, variable helper surface, and cache behaviour are now documented in
`docs/books/crexx_programming_guide/crexxsaa.md`. This file remains as the
historical design and progress record for the ADDRESS programme.

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
- explicit `ADDRESS env` can set the current/default environment without
  executing a command, where `env` is currently the unquoted identifier form
- explicit `ADDRESS env command` executes immediately in `env`
- explicit `ADDRESS "command"` executes the quoted command in the
  current/default environment; quoted second tokens are not environment names
- implicit command dispatch uses the runtime current/default environment,
  initially `SYSTEM`
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

- `_address` builds an `addressrequest` and dispatches it through the current
  `.addressenvironment` provider.
- `SYSTEM`, `COMMAND`, `CMD`, and `SHELL` use the spawn-backed shell provider.
- `PATH` uses its own provider object over the direct executable spawn
  transport.
- native `rxvml` hosts can register callback-backed providers through the same
  factory/registration path.
- `EXPOSE` on `ADDRESS` currently passes explicit string variable bindings to
  the environment and applies explicit response updates back to those exposed
  variables. This is not yet arbitrary variable-pool access.
- On POSIX, the spawn path still uppercases exposed variable names before
  `setenv()` when delegating through the built-in spawn providers.

### 3.4 Shell behaviour is environment-dependent

The current implementation has an explicit shell/direct split.

- `SYSTEM`, `COMMAND`, `CMD`, and `SHELL` call the platform command processor.
  On POSIX this is standard `sh -c`, found from the standard utility path
  before `/bin/sh` or ordinary `PATH`. On Windows this is `%COMSPEC% /D /S /C`,
  falling back to `cmd.exe`.
- `PATH` tokenises the command string itself, resolves the executable via
  process `PATH`, and calls it directly.

Important consequence:

- shell syntax is available through the system-like environments
- pipes, redirects, shell expansion, and built-ins such as `cd` are
  platform-command-processor behavior, not direct `PATH` behavior
- the current environment value now selects the shell/direct/provider route

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
  - explicit exposure declarations
  - sandbox declarations
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

### 4.1 Initial `crexxsaa` variable facade

The first implemented variable-pool bridge is deliberately narrower than a full
`RexxVariablePool()` clone.

`crexxsaa` now exposes host-side helpers for active `ADDRESS` callbacks:

- `crexxsaa_address_variable_get_alloc()`
- `crexxsaa_address_variable_set()`
- `crexxsaa_free()`

These helpers operate only while a native `ADDRESS` callback is active. They
resolve variables in this order:

1. Direct scalar `ADDRESS ... expose name` bindings.
2. Direct stem/array `ADDRESS ... expose name[]` bindings.
3. The active `ADDRESS ... sandbox pool` object.
4. The request's standard sandbox fallback.

The host therefore gets one simple name-based API, while the CREXX script
chooses whether the command should use direct exposure or sandbox storage.
Compound names such as `FILENAME.1` are mapped to exposed stems by splitting at
the first dot and using the remaining text as the stem key. Names are matched
case-insensitively at this facade layer to fit legacy REXXSAA/THE expectations;
the standard CREXX sandbox already normalises keys internally.

For compatibility with legacy command processors that treat a scalar result as
a one-item stem, an exposed scalar also has a limited compound view:

- `name.0` reads as `"1"`
- `name.1` reads the scalar value
- writes to `name.0` are accepted and ignored
- writes to any other `name.tail` update the scalar value

This rule is applied only when no exposed stem with the same base name exists.
Real `EXPOSE name[]` stems therefore keep their normal stem semantics.

This is enough for THE's existing `set_rexx_variable()` and
`get_rexx_variable()` call sites to be mapped onto CREXX without making THE know
whether a variable lives in an exposed CREXX variable, an exposed CREXX array, or
a sandbox. It is not yet a general variable-pool enumerator and it does not
provide arbitrary access to unexposed CREXX locals.

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
   - explicit string-oriented exposures
   - optional sandbox objects for command-processor state
   - editor/highlighter support
5. The internal address-environment protocol should be separated from transport.
   A future `rexxsaa.h` adapter must be possible, but it must sit on top of the
   canonical modern CREXX protocol rather than define it.
6. Compatibility-facing variable-pool operations still need support through the
   explicit sandbox contract.
7. The preferred modern authoring model should remain explicit and
   capability-based: named `EXPOSE` bindings for small exchanges, and
   caller-supplied sandbox objects for broader command-processor workspaces.
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
- explicit exposure declarations
- optional sandbox declaration
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
  contract, but express them through an explicit sandbox object rather than
  arbitrary access to the caller's local variables
- keep current/default address-environment state in the runtime/library layer
  so explicit `ADDRESS env` can update it without inventing a compiler-only
  special case

That contract should be able to:

- track and update the current/default address environment
- receive command text
- receive redirect plans, explicit exposure bindings, and an optional sandbox
- execute or delegate the command
- return RC and diagnostics
- apply allowed exports back to the Rexx side

This would support:

- native implementations
- Rexx implementations
- a future REXXSAA adapter over the canonical internal protocol
- a future JSON/socket adapter over the canonical internal protocol

### 6.4 Exposure and sandbox model

Approved compatibility and preference split:

- compatibility support must cover:
  - explicit exposure of named string variables for small modern integrations
  - a separate sandbox string-container for legacy Rexx and command processors
    that expect a variable-pool-like workspace
- the preferred command-processor direction is `sandbox`, because it gives
  variable-pool-style behaviour a clear object boundary and does not expose the
  caller's whole local scope

Notes:

- explicit `EXPOSE` is a list concept, not a single-name-only form
- explicit `EXPOSE` should keep internal and external names aligned for now;
  aliasing is deferred unless a real use case demands it
- explicit `EXPOSE` values are string-oriented; the near-term supported shapes
  are scalar `.string` variables plus explicit `.string[]` array bindings
  written as `EXPOSE name[]`
- the external environment is expected to know exposed variable names and infer
  or negotiate their role from command semantics where possible

Interpretation:

- `EXPOSE name ...` exports explicit string variable capabilities and allows
  returned updates only to those names
- `EXPOSE name[]` exports a `.string[]` array through a request-owned
  `.addressstem` binding; the array is packed from `name.1` through `name.0`
  and written back after dispatch, including provider changes to key `"0"`
- `SANDBOX pool` passes a controlled string map implementing the ADDRESS
  sandbox interface
- a sandbox key is just a string, so classic-style names such as `VALUE.3` are
  represented as key `"VALUE.3"` mapped to a string value
- sandbox access is intentionally broader than explicit `EXPOSE`, but is
  confined to the supplied object; it is not unrestricted caller namespace
  access

Recommended syntax direction:

```rexx
address cms
address "CP QUERY USERID"
address cms "CP QUERY USERID"
address editor "GET CURSOR" expose row col
address "GET CURSOR" expose row col
address cms "EXECIO * DISKR FILE A STEM VALUE." sandbox a_pool
"EXECIO * DISKR FILE A STEM VALUE." sandbox a_pool
address "EXECIO * DISKR FILE A STEM VALUE." sandbox a_pool
```

Supported syntax for review:

- `ADDRESS env` sets the current/default environment without executing a
  command. The supported selector form is an unquoted identifier, for example
  `ADDRESS cms`.
- `ADDRESS env command-expression` executes immediately in `env`; this also
  makes `env` the current/default environment.
- `ADDRESS "command"` executes the quoted command in the current/default
  environment. This is the explicit form of the common Rexx use case after
  `ADDRESS env`.
- A bare string command line such as `"command"` also executes in the
  current/default environment and does not emit the implicit-command warning.
- A bare non-string command expression such as `cmd` executes in the
  current/default environment but emits `IMPLICIT_ADDRESS` so the use is visible
  during migration.
- `INPUT`, `OUTPUT`, `ERROR`, `EXPOSE name`, `EXPOSE name[]`, and
  `SANDBOX pool` attach to a command dispatch, including current-environment
  `ADDRESS "command"` dispatches.

`SANDBOX pool` is deliberately per-command: it passes a caller-owned Rexx object
to the provider only for that dispatch. A sandbox is not retained as ADDRESS
state, because it is an ordinary Rexx variable/capability and may go out of
scope after the caller frame returns. This keeps ADDRESS environment selection
classic, while keeping command-processor variable-pool access explicit and
safe.

Standard sandbox contract:

- define a real Level B interface, working name `.addresssandbox`
- provide a standard string-map class implementing that interface
- the interface stores and returns strings; non-string callers must convert at
  the boundary
- the standard class must use case-insensitive key access, normalising names
  such as `value.3`, `VALUE.3`, and `Value.3` to the same entry for classic
  Rexx uppercase compatibility
- the intended interface surface is fetch/get, set, drop, exists, and
  iteration over keys for compatibility-style enumeration

Standard exposed-stem contract:

- define a real Level B interface, working name `.addressstem`
- provide `.standardaddressstem`, a case-insensitive string-map class with the
  same `get`, `set`, `drop`, `exists`, and `next` surface as the sandbox
- `EXPOSE name[]` is explicit syntax, not type inference; this keeps scalar
  `EXPOSE name` lowering immediate and avoids letting `ADDRESS env` identifiers
  be symbolised while an exit waits for type convergence
- the request owns exposed stem bindings during dispatch; Rexx providers use
  `request.get_binding_stem_value(index, key)` and
  `request.set_binding_stem_value(index, key, value)`, while C providers use
  `rxvml_address_binding_stem_get()` and
  `rxvml_address_binding_stem_set()`

Implemented tracer contract:

- `.addresssandbox` now exposes direct `get`, `set`, `drop`, `exists`, and
  `next` methods
- `.standardaddresssandbox` implements that direct method surface and remains a
  case-insensitive string-map
- imported providers and ADDRESS helper wrappers type sandbox arguments as
  `.addresssandbox`; the earlier generic `access(operation, name, value)` shim
  has been removed
- `addressrequest` exposes `get_sandbox_value(name)` and
  `set_sandbox_value(name, value)` for Rexx providers that need to read/write
  the caller sandbox without depending on object-return aliasing
- explicit `SANDBOX pool` requests keep the request sandbox linked to the
  selected sandbox object for the duration of dispatch

Compatibility stance:

- a future REXXSAA adapter should translate `RexxVariablePool()` and `SHVBLOCK`
  operations onto the sandbox interface
- Level C / classic Rexx assets should prefer sandbox over explicit `EXPOSE`
  when they expect dynamic stem names, `VALUE.3`-style access, or variable-pool
  enumeration

Marker-based binding:

- embedded-SQL-style markers remain deferred/experimental
- markers should not be part of the main ADDRESS operating model until explicit
  `EXPOSE` and `SANDBOX` have proved sufficient or insufficient in real use

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
   modern address contract through an explicit sandbox object.
3. Define the explicit `EXPOSE` binding model for small string-oriented
   integrations.
4. Define the `SANDBOX` object contract for command processors and
   compatibility-style variable-pool behaviour.
5. Record marker syntax as a deferred experiment rather than part of the first
   ADDRESS operating model.

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
  - `array`
  - `sandbox`
- `internal_name`
- `external_alias`
- `value_type`
- `dimensions`
- `provenance`
  - `expose`
  - `sandbox`
  - `marker` (deferred)
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
- `rxcp.addresssandbox`
- `rxcp.standardaddresssandbox`

Recommended `addressenvironment` method shape:

```rexx
*: factory
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
- `sandbox = .addresssandbox`
- `stdin_endpoint`
- `stdout_endpoint`
- `stderr_endpoint`
- `flags`

Recommended `addressresponse` fields:

- `rc`
- `updated_bindings = .addressbinding[]`
- `diagnostics = .string[]`
- `condition_name`

Recommended `addressbinding` fields:

- `kind`
  - `var`
  - `array`
- `internal_name`
- `external_alias`
- `value`
- `flags`

For the simplified ADDRESS model, `addressbinding` is for explicit `EXPOSE`
bindings only. Sandbox access should not be represented as another binding
kind; it is a separate object on the request.

Implemented `addresssandbox` methods:

- `get(name)`
- `set(name, value)`
- `drop(name)`
- `exists(name)`
- `next(cursor)` for key iteration

These methods are the provider contract for both pure Rexx and hybrid providers.
The generic `access(operation, name, value)` tracer shim has been removed now
that the compiler can import the direct interface signatures needed by the
ADDRESS fixtures.

Case rules:

- standard sandbox lookup is case-insensitive
- keys are normalised to uppercase after trimming surrounding whitespace
- this is intentional: it keeps `VALUE.3`, `value.3`, and `Value.3` compatible
  with classic Rexx uppercase variable-pool expectations

Compatibility statement:

- compatibility-style variable-pool access should be represented through the
  sandbox contract, not as a separate hidden side channel
- a future `rexxsaa.h` adapter should translate `RexxVariablePool()` and
  related behaviour onto `addresssandbox`

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

### 7.5 Exposure and sandbox syntax proposal

Preferred explicit exposure syntax:

```rexx
address editor "GET CURSOR" expose row col
address "GET CURSOR" expose row col
address editor "LIST OPEN BUFFERS" expose buffers[]
address "LIST OPEN BUFFERS" expose buffers[]
address tool command expose status message
```

Interpretation:

- `EXPOSE` declares exact variable capabilities by name
- internal and external names are the same for now
- scalar exposure is string-oriented
- `EXPOSE name[]` declares a `.string[]` binding and is lowered through a
  request-owned `.addressstem` object rather than by passing the array directly
- providers inspect the exposed names and return updates for names they
  understand

Preferred sandbox syntax:

```rexx
address cms "EXECIO * DISKR FILE A STEM VALUE." sandbox a_pool
"EXECIO * DISKR FILE A STEM VALUE." sandbox a_pool
address "EXECIO * DISKR FILE A STEM VALUE." sandbox a_pool
```

Interpretation:

- `SANDBOX` passes a caller-supplied string-container object implementing the
  ADDRESS sandbox interface
- the provider can read and write any keys through the request sandbox contract
  (`request.get_sandbox_value(...)` / `request.set_sandbox_value(...)` in Rexx,
  or the `rxvml_address_sandbox_*` helpers in C)
- keys are strings, so `VALUE.3`, `RESULT`, `RC`, or command-processor-specific
  names are ordinary map keys
- the no-command form sets only the current/default address environment; sandbox
  objects are not retained and must be supplied on each command that needs one
- the standard sandbox class performs case-insensitive lookup and stores keys
  in uppercase form
- `ADDRESS "command" SANDBOX pool` is the explicit current-environment form;
  quoted second tokens are commands, not environment selectors

Compatibility stance:

- explicit `EXPOSE` remains useful for small, modern, safe integrations
- `SANDBOX` is the preferred model for legacy Rexx assets, command processors,
  dynamic stem names, and future REXXSAA variable-pool compatibility

Native tracer stance:

- `rxvml_address_request` now carries the sandbox object pointer
- `rxvml_address_sandbox_get()` lets a native callback read from the standard
  sandbox with the same case-insensitive key rules
- `rxvml_address_sandbox_set()` lets a native callback update the request
  sandbox; the standard sandbox/stem classes are updated through direct VM
  layout helpers, with interface method dispatch reserved as the fallback for
  non-standard implementations
- `rxvml_address_binding_stem_get()` and
  `rxvml_address_binding_stem_set()` let a native callback read and write an
  exposed `EXPOSE name[]` binding through the request-owned stem object
- native providers may still return `SANDBOX` entries in
  `rxvml_address_response.updated_bindings` when they want a batched response
  model matching explicit `EXPOSE` writeback
- native providers must not mutate the standard sandbox object layout directly;
  reads and writes go through the ADDRESS sandbox helpers or the Rexx
  `.addresssandbox` method contract

### 7.6 Marker syntax proposal

Proposal:

- marker syntax is deferred until explicit `EXPOSE` and `SANDBOX` usage has
  produced enough evidence
- if markers return later, they should be additive to explicit `EXPOSE`, not a
  replacement for sandbox

Previously considered syntax:

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

Potential parsing rule if revived:

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

### 7.7 Explicit binding and sandbox model proposal

Proposal:

- all explicit `EXPOSE` names should be normalized into one binding list before
  runtime dispatch
- the environment should see one explicit binding list of:
  - internal Rexx names
  - external names, currently the same as internal names while aliasing is
    deferred
  - binding kinds
- the environment should separately receive an optional sandbox object
- sandbox access should not be converted into individual bindings

Normalization rules:

- explicit `EXPOSE` entries should add bindings even if no command text
  references exist
- exact duplicate bindings should be coalesced
- the normalized uniqueness key should be:
  - `kind`
  - `internal_name`
  - `external_alias`
- because aliasing is deferred, `external_alias` should equal `internal_name`
  in the near-term implementation

Conflict rules:

- duplicate exposed names should be coalesced
- non-string explicit EXPOSE values should be rejected or converted only when a
  later rule explicitly defines the conversion
- same external alias mapped to different internal names remains a future
  compile-time error if aliasing is reintroduced

Result:

- the runtime environment receives one coherent explicit binding table plus one
  optional sandbox object
- explicit bindings cover small, intentional variable exchange
- sandbox covers command-processor variable-pool behaviour without exposing
  caller locals directly

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
  contract through the sandbox model
- define explicit `EXPOSE` binding syntax and sandbox syntax
- define how explicit bindings and the optional sandbox appear on
  `addressrequest`
- update compiler-exit documentation to match the certified/system-exit model

Stage 1 exit criteria:

- one approved proposal for the richer exit-planning response
- one approved proposal for explicit exposure syntax and sandbox syntax
- one approved definition of the explicit binding model
- one approved statement of how compatibility pool operations appear through
  the sandbox contract
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
"RETURN 42"
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
    rxvml_value *value_object;
    const char *flags;
} rxvml_address_binding;

typedef struct rxvml_address_request {
    const char *environment_name;
    const char *command;
    size_t binding_count;
    const rxvml_address_binding *bindings;
    rxvml_value *sandbox;
} rxvml_address_request;

typedef struct rxvml_address_response {
    int rc;
    const char *condition_name;
    const char *diagnostic;
    size_t updated_binding_count;
    const rxvml_address_binding *updated_bindings;
} rxvml_address_response;

typedef int (*rxvml_address_callback)(
    rxvml_context *ctx,
    const rxvml_address_request *request,
    rxvml_address_response *response,
    void *userdata);
```

The string pointers supplied in `rxvml_address_request` are callback-duration
views over VM values. A host that needs to retain them after the callback should
copy them. `updated_bindings` pointers supplied on `rxvml_address_response`
only need to remain valid until the callback returns; `rxvml` copies them into
the Rexx `addressresponse` object immediately.

For `kind == "stem"` bindings, `value_object` points at the request-owned
`.addressstem` object for the callback duration. Native providers should use
`rxvml_address_binding_stem_get()` / `rxvml_address_binding_stem_set()` rather
than retaining or directly dereferencing that object.

Stage 3.3 exit criteria:

- a C-only dummy host can register `EDITOR`, run Rexx, and observe
  `ADDRESS EDITOR` callbacks
- explicit and implicit ADDRESS dispatch both reach the native callback
- exposed binding names and values are visible to the callback; the binding
  shape still carries an alias/name field for future compatibility, but current
  explicit `EXPOSE` keeps it equal to the internal name
- callback-provided updated bindings can be written back to explicitly exposed
  Rexx variables
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
*: factory
  arg env_name = .string
```

- concrete providers implement class-side `*: match` and `*: factory`
  members taking the same `env_name` argument
- factory declarations no longer spell a return type in source; the interface
  factory returns `.addressenvironment`, while each provider factory returns its
  concrete provider class and is checked for assignability
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
- `CMS` has been moved out of the shipped library and is now a pure Rexx test
  provider in `compiler/tests/rexx_src/address_cms_provider.rexx`, implementing
  `.addressenvironment`
- `address_cms_provider.rexx` imports the ADDRESS helper classes/procedures and
  implements the imported `_rxsysb.addressenvironment` interface directly
- the CMS test provider is dynamically selected by `.addressenvironment("CMS")`
  when its fixture module is explicitly loaded beside the linked library
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
- class factories use bare `return` to return the constructed object; the
  compiler rewrites this through its internal factory object rather than
  requiring a source-level `this`
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
- the RXPA `ADDFACTORY` argument is metadata-level only; Rexx source factories
  omit return types and the compiler derives them from the owning interface or
  class
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
- full compatibility variable-pool operations and `RexxVariablePool()`
  compatibility
- sandbox interface implementation, standard sandbox class, and compatibility
  pool enumeration
- non-string EXPOSE writeback
- redirect endpoint abstraction and host-side stream pumping
- transport-specific remote adapters

### Stage 3.4: explicit EXPOSE writeback

Status: implemented in debug build; focused verification green on 2026-04-24

Primary goal:

- make `ADDRESS ... EXPOSE name` useful for host/editor experiments by letting
  environments return explicit binding updates that `_address` applies back to
  the exposed Rexx variables

Implemented design:

- `addressresponse` now carries `updated_bindings = .addressbinding[]`
- `_address` applies only response updates whose `var` binding name or alias
  matches a variable explicitly supplied through the `EXPOSE` tail
- writeback is string-only for this tracer bullet, matching the current
  compiler lowering of `arg expose ... = .string`
- Rexx providers call `response.add_updated_binding(...)`
- native callbacks set `response->updated_binding_count` and
  `response->updated_bindings`; `rxvml` copies those callback-duration views
  into the Rexx response object before returning from the native trampoline
- no general variable-pool access is granted yet

Verification coverage:

- `address_cms_provider` proves pure Rexx provider writeback
- `address_callback_host` proves native callback writeback
- existing ADDRESS compiler-exit and library ADDRESS tests still pass

### Stage 3.5: ADDRESS sandbox interface

Primary goal:

- introduce a controlled string-container object for command processors and
  legacy Rexx compatibility without exposing arbitrary caller variables

Planned syntax:

```rexx
address cms "EXECIO * DISKR FILE A STEM VALUE." sandbox a_pool
address cms
"EXECIO * DISKR FILE A STEM VALUE." sandbox a_pool
address cms "QUERY BUFFERS" expose buffers[]
```

Implemented tracer deliverables:

- defined the `.addresssandbox` interface
- provided `.standardaddresssandbox`, a case-insensitive string-map
  implementation
- extended `addressrequest` with a sandbox object
- added request-level sandbox read/write methods for Rexx providers so writes do
  not depend on mutating an object value returned by `get_sandbox()`
- extended the ADDRESS certified exit to parse `SANDBOX pool`
- rejected commandless `ADDRESS env SANDBOX pool`; use `ADDRESS env` to select
  the default environment and pass `SANDBOX pool` on each command that needs it
- kept explicit `EXPOSE` as a separate small-binding model
- added Rexx CMS-provider tests showing sandbox read/write and response-update
  application through the sandbox interface
- strengthened the CMS provider test with an `rxvml` host assertion so the Rexx
  fixture's returned error count is checked directly
- added native callback-host coverage showing a C callback reading the supplied
  standard sandbox through `rxvml_address_sandbox_get()`, writing through
  `rxvml_address_sandbox_set()`, and writing through `SANDBOX` response updates
- added `.addressstem` / `.standardaddressstem` and `EXPOSE name[]` lowering for
  explicit string-array bindings; Rexx and C providers now mutate the
  request-owned stem and the caller array is resized/written back after dispatch
- added C stem helpers:
  `rxvml_address_binding_stem_get()` and
  `rxvml_address_binding_stem_set()`

Non-goals for this stage:

- arbitrary caller variable-pool access
- full REXXSAA adapter surface
- typed object exchange through `ADDRESS`
- arbitrary native mutation of non-standard object internals

Known follow-ups recorded for the next compiler/runtime cleanup:

- replace the current RXAS-metadata-to-temporary-source import path with a richer
  metadata importer, or extend metadata so original optional/default expressions
  are not approximated during stub reconstruction
- keep `expose` and optional/default semantics independent in import handling;
  RXAS metadata uses `?` for optional arguments and `expose` for reference
  arguments, and neither marker implies the other
- add focused object/reference regression tests around interface-typed helper
  returns and temporary variables. ADDRESS now avoids relying on those aliases
  by using request-level sandbox read/write helpers and live request links, but
  the wider object-aliasing contract still needs direct compiler/runtime
  coverage before more abstractions depend on it
- harden class/interface metadata import so repeated imported stubs do not
  leak duplicate synthetic `main` procedures or duplicate class/interface
  declarations into validation/linking

### Stage 3.6: redirect endpoint abstraction

Primary goal:

- introduce redirect endpoint objects over current `REDIRECT` once environment
  dispatch is real and testable

Deliverables:

- wrap the existing redirect machinery in endpoint-shaped runtime objects
- prepare for future pipeline graphs without changing VM opcodes first
- keep executable multi-command pipelines out of scope for this stage

### Stage 4: compatibility and transport adapters

Stage 4 now starts JSON-first rather than REXXSAA-first.

Rationale:

- JSON is a reusable foundation for web-service APIs, LLM provider calls,
  request/response debugging, and a future loose-coupled ADDRESS transport.
- REXXSAA remains important, but it should be an adapter over the modern
  `rxvml`/ADDRESS/sandbox contract rather than the design driver for the core
  protocol.

Stage 4.1 JSON foundation deliverables:

- provide a pure Level B `rxjson` library module with string-oriented JSON
  validation, quoting, path extraction, member enumeration, and object/array
  construction
- support LLM-shaped request/response tests without binding CREXX to a specific
  model provider
- keep the API path simple and Rexx-friendly: dot-separated object keys plus
  one-based array indexes such as `choices.1.message.content`

Stage 4.2 transport / web-service direction:

- layer HTTP/socket work on top of JSON once the JSON body and response
  extraction primitives are stable
- use provider-specific LLM APIs as practical examples, but keep the lower
  library generic

Stage 4.3 `crexxsaa` compatibility layer direction:

- build the initial `crexxsaa.h` / C wrapper layer as the compatibility surface
  over the modern runtime contract
- use real integration needs to decide how much of the historical REXXSAA shape
  belongs in `crexxsaa`, rather than treating the old ABI as an all-or-nothing
  implementation target
- treat THE as the first large non-mainframe proving ground for that layer, not
  as a one-off THE-specific facade
- map subcommand registration to ADDRESS environments
- map `RexxVariablePool()` / `SHVBLOCK` operations to sandbox/stem helpers
- put CREXX program compilation/loading orchestration behind the `RexxStart()`-
  shaped entry point, including source/instore compile, assemble/link/cache, and
  `rxvml` load/run as needed
- keep source compilation exact: host integrations should provide valid CREXX
  source for the language level they need; `crexxsaa` should not inject
  `OPTIONS`, an `ADDRESS` prelude, or other host-specific clauses
- manage compiled-script cache state in `crexxsaa`, using a host namespace,
  source-path bucket, source/content hash, compiler/library fingerprint, and
  explicit invalidate/refresh hooks
- decide which compatibility surfaces ship and at what support level

Coherence target:

- new integrations should prefer the modern `rxvml` / ADDRESS / sandbox
  contracts directly
- legacy integrations should be able to enter through `crexxsaa` where useful
- the two paths should converge on the same internal request/response,
  environment, sandbox, and stem contracts so compatibility does not become a
  second runtime model
- full arbitrary variable-pool emulation remains a deliberate decision point;
  the first compatibility slice should implement only the request-pool behaviour
  justified by real host integrations

### Stage 4.3.1: temporary `crexxsaa` user and cache documentation

This section is deliberately written as user-facing and implementor-facing
documentation inside the working note. It should be moved into the permanent
`crexxsaa` documentation once the integration surface settles.

Current scope:

- `crexxsaa` is the initial CREXX compatibility API for hosts that want a
  REXXSAA-shaped entry point without requiring the full historical REXXSAA
  surface
- the first supported host model is command-environment execution:
  - host C code creates a `crexxsaa_context`
  - registers one or more ADDRESS callback environments
  - configures the CREXX compiler/assembler paths
  - runs either an existing `.rxbin` or source that `crexxsaa` compiles and
    caches
- variable-pool / `SHVBLOCK` emulation is not part of this first slice
- source is compiled exactly as supplied; `crexxsaa` does not add
  `OPTIONS LEVELB`, `ADDRESS`, host preludes, or compatibility boilerplate

Writing source for a host integration:

- write normal CREXX source at the language level required by the current
  compiler
- if the host expects Level B, put `options levelb` in the script
- if commands should be routed to the host environment, put the relevant
  `address <environment>` clause in the script
- for THE-hosted scripts today, that means the profile or macro should start
  with the equivalent of:

```rexx
options levelb
address the
'emsg CREXX_PROFILE_HOSTED'
```

The important point is ownership: the script declares its language level and
host command environment. `crexxsaa` only compiles, caches, loads, and runs it.

Minimal host-side flow:

```c
crexxsaa_context *ctx = NULL;

crexxsaa_create(location, library_rxbin, &ctx);
crexxsaa_set_compiler(ctx, rxc_path, rxas_path, import_dir);
crexxsaa_register_address_environment(ctx, "THE", the_callback, userdata);
crexxsaa_set_address_environment(ctx, "THE");
crexxsaa_run_source(ctx, profile_path, "THE", 0, argc, argv, &program_rc);
crexxsaa_destroy(ctx);
```

The host namespace argument, `"THE"` in this example, is part of the cache
identity. Two different hosts may compile the same source path without sharing a
cache bucket accidentally.

User cache tool:

- the CREXX build/install now includes a `crexxsaa` maintenance binary in the
  normal `bin` directory
- it is a troubleshooting tool for the compiled-script cache; it is not the
  script runner
- common commands:

```sh
crexxsaa --location
crexxsaa --list
crexxsaa --clear
crexxsaa --cache-dir /tmp/crexxsaa-cache --list
crexxsaa --cache-dir /tmp/crexxsaa-cache --clear --list
```

Default invocation with no arguments prints the cache location and lists cache
entries. `--location` alone prints only the resolved cache directory, which is
useful in scripts.

Example list output:

```text
cache: /Users/adrian/Library/Caches/crexx/crexxsaa
source: /path/to/profile.the
  bucket: 0379ad70148bf7ca
  rxbin: /Users/adrian/Library/Caches/crexx/crexxsaa/v1/0379ad70148bf7ca/b0ec3f1ffcc51e4c.rxbin
  rxbin_size: 1216
  source_hash: 9100aa6921615883
  config_hash: 5af0757a39c4eba1
```

Cache location rules:

- `CREXXSAA_CACHE_DIR` overrides the platform default
- a host may also call `crexxsaa_set_cache_dir()`
- for THE specifically, `THE_CREXX_CACHE_DIR` is accepted by THE and forwarded
  to `crexxsaa`
- if both `CREXXSAA_CACHE_DIR` and a host-provided cache directory are present,
  `CREXXSAA_CACHE_DIR` wins
- platform defaults:
  - macOS: `$HOME/Library/Caches/crexx/crexxsaa`
  - Windows: `%LOCALAPPDATA%\crexx\crexxsaa`, falling back under
    `%USERPROFILE%\AppData\Local`
  - other Unix-like systems: `$XDG_CACHE_HOME/crexx/crexxsaa`, falling back to
    `$HOME/.cache/crexx/crexxsaa`

Runtime cache controls:

- `CREXXSAA_CACHE_DISABLE=1`
  - compile source through temporary files only
  - do not read or write the persistent cache
- `CREXXSAA_CACHE_REFRESH=1`
  - ignore a valid cache hit
  - compile again and replace the manifest for that source bucket
- `CREXXSAA_CACHE_TRACE=1`
  - write cache decisions to `stderr`
  - currently emits events such as `miss`, `hit`, `stale`, `refresh`, and
    `disabled`

Compiler path controls:

- hosts normally call `crexxsaa_set_compiler(ctx, rxc, rxas, import_dir)`
- the environment variables `CREXXSAA_RXC`, `CREXXSAA_RXAS`, and
  `CREXXSAA_IMPORT_DIR` override the configured compiler values
- THE also has host-level variables:
  - `THE_CREXX_RXC`
  - `THE_CREXX_RXAS`
  - `THE_CREXX_IMPORT_DIR`
  - `THE_CREXX_LOCATION`
  - `THE_CREXX_LIBRARY_RXBIN`

Technical cache layout:

- the cache schema is versioned; current entries live under `v1`
- each source path has a source bucket:

```text
<cache-root>/v1/<source-key>/
  manifest
  <object-hash>.rxbin
```

- `source-key` is an FNV-1a 64-bit hash rendered as 16 hex characters
- the source-key hash includes:
  - host namespace
  - canonical source path when available, otherwise the supplied source path
- `object-hash` is also an FNV-1a 64-bit hash rendered as 16 hex characters
- the object hash includes:
  - source content hash
  - compiler/library configuration hash

Manifest fields:

```text
version=1
source_path=/absolute/or/supplied/source/path
source_hash=<16-hex-content-hash>
source_size=<bytes>
source_mtime=<mtime>
config_hash=<16-hex-config-hash>
rxbin=<absolute/cache/path/to/object.rxbin>
```

The content hash, not only the timestamp, decides whether source changed. Size
and mtime are recorded for diagnostics and human inspection.

The configuration hash includes:

- cache schema and `CREXXSAA_ABI_VERSION`
- configured or environment-overridden `rxc` path
- configured or environment-overridden `rxas` path
- configured or environment-overridden import directory
- loaded `library.rxbin` path
- file size and mtime for the compiler, assembler, import directory, and
  library path where the platform can stat them

This means normal source edits, CREXX rebuilds, compiler path changes, and
library rebuilds cause a recompile without manual cache clearing.

Compile and update sequence:

- compute source hash and configuration hash
- read the source bucket manifest if it exists
- on a valid hit, load the cached `.rxbin`
- on miss, stale, or refresh:
  - run `rxc -i <import-dir> -o <temp-base> <source>`
  - run `rxas -o <temp-base> <temp-base>.rxas`
  - rename the temporary `.rxbin` into the source bucket
  - write a new manifest through a temporary file and rename it into place
  - remove the previous cached `.rxbin` for the bucket when it has been
    superseded

Invalidation:

- C API:
  - `crexxsaa_invalidate_source(ctx, source_path, namespace)`
  - `crexxsaa_invalidate_all(ctx)`
  - `crexxsaa_clear_cache(cache_dir_override)`
- CLI:
  - `crexxsaa --clear`
  - `crexxsaa --cache-dir DIR --clear`
- the cache is disposable; deleting it should affect performance only, not
  program semantics
- the current implementation uses atomic file replacement for compiled objects
  and manifests, but it is not yet a full cross-process locking protocol; for
  troubleshooting, prefer clearing the cache while the host application is idle

## 9. Progress

- 2026-04-17: iteration 1 working note created
- 2026-04-17: current compiler/runtime review captured
- 2026-04-17: first pass of requirements, design direction, and staged plan
  recorded
- 2026-04-17: core direction approved:
  - `ADDRESS` may become a certified exit
  - the internal modern protocol is canonical
  - the initial exposure direction was explicit `EXPOSE`; later sandbox work
    refined this into `EXPOSE` for small bindings and `SANDBOX` for
    command-processor state
  - compatibility support remains mandatory
  - redirect work is abstraction-only in this phase
  - address environments should follow a common class shape
- 2026-04-17: further direction approved:
  - compatibility-style variable-pool support belongs in the modern address
    contract
  - marker-based binding was considered for prototype feedback, but has since
    been deferred behind explicit `EXPOSE` and `SANDBOX`
  - compatibility-style variable pools should now be expressed through an
    explicit sandbox object
  - Stage 1 is now in progress
- 2026-04-17: Stage 1 proposals documented:
  - certified/system exit ownership model
  - structured planning contract
  - contextual keyword claim model
  - common address request/response/environment contract
  - preferred explicit exposure and sandbox syntax
  - explicit binding normalization and sandbox boundary rules
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
    - Stage 3.4 explicit EXPOSE writeback
    - Stage 3.5 ADDRESS sandbox interface
    - Stage 3.6 redirect endpoint abstraction
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
    `compiler/tests/rexx_src/address_callback_host.crexx` provide the dummy
    host/fixture for debugger-friendly validation
  - focused ADDRESS/interface/bridge verification passed `82/82`
  - full debug CTest passed `852/852`
- 2026-04-24: Stage 3.3 factory/provider refinement implemented:
  - `.addressenvironment` now owns the dynamic factory contract and providers
    advertise themselves with class-side `*: match`/`*: factory`
  - `_address` creates providers on demand through `.addressenvironment(name)`
    and caches them in the runtime registry
  - CMS moved from the core `_address.rexx` module to a pure Rexx provider;
    it now lives as the compiler-test fixture
    `compiler/tests/rexx_src/address_cms_provider.rexx`, not in the shipped
    linked library
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
- 2026-04-24: Stage 3.4 explicit EXPOSE writeback implemented:
  - `addressresponse` now carries updated binding objects
  - `_address` applies returned `var` updates only to variables explicitly
    supplied through `ADDRESS ... EXPOSE`
  - `rxvml_address_response` now includes callback-duration updated binding
    views, copied into the Rexx response object by the native trampoline
  - CMS/Rexx and native callback fixtures both prove exposed `buffer`
    writeback
  - focused ADDRESS writeback coverage passed `11/11`
- 2026-04-24: sandbox direction accepted for the next ADDRESS design slice:
  - `SANDBOX` is now the preferred model for command processors, legacy Rexx
    assets, dynamic stems, and future REXXSAA variable-pool compatibility
  - explicit `EXPOSE` remains as the small string-binding model for modern
    integrations
  - marker/alias ideas are deferred until the simpler operating model has been
    tested with real providers
- 2026-04-24: Stage 3.5 sandbox tracer implemented:
  - ADDRESS now accepts per-command `SANDBOX pool` clauses
  - `addressrequest` carries a sandbox object through Rexx and native
    dispatch
  - `.standardaddresssandbox` provides case-insensitive string-map access for
    classic Rexx-style keys such as `VALUE.3`
  - Rexx CMS-provider tests prove method-based sandbox read/write and
    response-applied sandbox updates
  - native callback tests prove a C caller can read the supplied standard
    sandbox through `rxvml_address_sandbox_get()`
  - compiler/import issues discovered by this tracer are recorded as the next
    cleanup slice rather than expanded in this stage
- 2026-04-24: Stage 3.5 sandbox cleanup implemented:
  - `.addresssandbox` now uses the direct `get/set/drop/exists/next` method
    contract; the temporary generic `access(...)` shim has been removed
  - `_address_with_sandbox` now types sandbox values as `.addresssandbox`
  - retained default sandbox state was removed; sandbox objects are caller-owned
    capabilities and must be passed on each command that needs one
  - Rexx providers now use `addressrequest.get_sandbox_value()` and
    `addressrequest.set_sandbox_value()` for stable sandbox mutation
  - RXAS import stub reconstruction now preserves `expose` while converting
    metadata optional markers into parseable temporary declarations
  - native callback tests now prove both C reads and C-originated sandbox writes;
    writes are covered both through `rxvml_address_sandbox_set()` and through
    `SANDBOX` response updates applied by Rexx
  - VM `rxvml` nested calls now preserve the outer external-call trampoline, so
    callback-originated Rexx calls no longer corrupt the callback return path;
    standard ADDRESS sandbox/stem writes now avoid repeated method dispatch by
    using direct VM-layout helpers
  - remaining object/reference mutation edge cases are recorded as focused
    compiler/runtime follow-up tests, not as ADDRESS API restrictions
- 2026-04-25: Stage 3.5 exposed-array/stem cleanup implemented:
  - `EXPOSE name[]` is now the explicit syntax for exposing `.string[]` values
    to ADDRESS providers
  - `.addressstem` and `.standardaddressstem` mirror the sandbox string-map
    method surface and use case-insensitive keys
  - generated ADDRESS lowering packs caller arrays into request-owned stem
    bindings, dispatches, then resizes and writes the caller array back from
    the request binding
  - Rexx providers use `request.get_binding_stem_value()` and
    `request.set_binding_stem_value()`; native callbacks use
    `rxvml_address_binding_stem_get()` and
    `rxvml_address_binding_stem_set()`
  - standard sandbox/stem C writes now use direct VM-layout helpers, with method
    dispatch left as the fallback for non-standard implementations
  - ADDRESS examples now prefer implicit current-environment command lines after
    `ADDRESS env`, rather than the ugly `ADDRESS "" "cmd"` form
  - explicit `ADDRESS "command"` now dispatches in the current/default
    environment, matching the usual Rexx distinction between `ADDRESS cmd`
    and `ADDRESS "cmd"`
  - focused ADDRESS coverage passed `24/24`; full debug-tree coverage passed
    `866/866` on 2026-04-25
- 2026-04-25: Stage 4 JSON-first direction accepted and initial pure Rexx
  `rxjson` Level B library module implemented:
  - `rxjson` exposes `jsonvalid`, `jsontype`, `jsonget`, `jsoncount`,
    `jsonmembers`, `jsonquote`, `jsonunquote`, `jsonarray`, and `jsonobject`
  - the API is deliberately string-oriented for request bodies, response-field
    extraction, and future JSON/socket ADDRESS transport experiments
  - path lookup uses case-sensitive object keys and one-based array indexes,
    e.g. `choices.1.message.content`
  - the library contract is documented in `lib/rxfnsb/rexx/rxjson.md`
  - focused `rxjson` no-opt and linked-opt coverage passed `2/2`; full
    debug-tree coverage passed `868/868`
- 2026-04-25: `crexxsaa` integration stance clarified:
  - the compatibility layer should be an initial CREXX compatibility API whose
    roadmap is guided by real integration needs
  - THE is expected to be the first major non-mainframe legacy-style integration
    and a practical proving ground, not the sole design target
  - `RexxStart()`-shaped entry should hide CREXX source/instore compilation,
    caching, loading, environment seeding, and execution orchestration
  - legacy entry points and modern `rxvml`/ADDRESS/sandbox usage should converge
    on one coherent internal runtime model
- 2026-04-25: initial THE tracer integration added, later superseded by the
  embedded profile driver:
  - THE discovers a CREXX `rxvm` runner from sibling build/install locations,
    exposes a batch-valid `CREXX` command, and can run a supplied `.rxbin`
  - an optional `USE_CREXX_AS_REXX` build switch routes THE's existing `REXX`
    command spelling to that tracer while the fuller `crexxsaa` entry point is
    still being developed
  - the batch smoke path compiles a tiny CREXX program to `.rxbin`, starts THE
    in batch mode, and verifies `rxvm` output flows back through THE
- 2026-04-25: first embedded THE profile driver implemented:
  - `crexxsaa.h` / `libcrexxsaa` now wrap the `rxvml` ADDRESS callback path as
    the initial compatibility API surface
  - THE can build with `USE_CREXX`, register `ADDRESS THE`, compile a profile
    source with `rxc`/`rxas`, and run the resulting `.rxbin` in-process
  - callback execution routes CREXX ADDRESS commands to THE's existing
    `command_line()` dispatcher, preserving the long-standing THE command
    integration point
  - the temporary THE `CREXX` command and `USE_CREXX_AS_REXX` tracer switch have
    been removed in favour of the embedded profile path
  - variable-pool / `SHVBLOCK` compatibility remains deliberately deferred;
    this first slice is command-environment execution only
- 2026-04-25: `crexxsaa` compiled-script cache added:
  - source execution moved behind `crexxsaa_run_source()`, with THE reduced to
    compiler/cache configuration plus `ADDRESS THE` registration
  - cache roots default to the platform user cache area, with
    `CREXXSAA_CACHE_DIR` and host-provided cache directories available for test
    and integration control
  - cache entries are keyed by host namespace, canonical source path, source
    content hash, and compiler/library fingerprint; edits and CREXX build-output
    changes recompile automatically
  - `CREXXSAA_CACHE_DISABLE`, `CREXXSAA_CACHE_REFRESH`,
    `crexxsaa_invalidate_source()`, and `crexxsaa_invalidate_all()` provide
    bypass and invalidation controls
  - source is compiled as supplied; THE's profile test is now explicit Level B
    source with its own `OPTIONS LEVELB` and `ADDRESS THE`
- 2026-04-25: `crexxsaa` cache maintenance tool added:
  - the `crexxsaa` binary in the CREXX build/install `bin` directory can print
    the resolved cache location, list cached entries, and clear the cache
  - `--cache-dir` supports troubleshooting or test-specific cache roots without
    changing the platform default

## 10. Evidence and code anchors

- `compiler/rxcpbgmr.y`
- `compiler/rxcp_val_trans.c`
- `compiler/rxcp_val_orch.c`
- `compiler/rxcp_exit.c`
- `compiler/rxcpbpar.c`
- `compiler/exits/address/Address.crexx`
- `compiler/exits/parse/Parse.crexx`
- `compiler/exits/execio/Execio.crexx`
- `lib/rxfnsb/rexx/_address.rexx`
- `lib/rxfnsb/rexx/rxjson.rexx`
- `lib/rxfnsb/rexx/rxjson.md`
- `lib/rxfnsb/tests_functional/ts_rxjson.rexx`
- `compiler/tests/rexx_src/address_cms_provider.rexx`
- `compiler/tests/rexx_src/address_cms_host.crexx`
- `interpreter/crexxsaa.h`
- `interpreter/crexxsaa.c`
- `interpreter/crexxsaa_tool.c`
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
