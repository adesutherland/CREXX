# RXPP STREAM Directive Implementation Plan

## Vision and intended outcomes

Add a generic `##STREAM stream-type stream-name [WITH processor]` RXPP block
directive for embedding application-specific free-form source in `.rxpp`
files. RXPP will collect the lines verbatim through `##END`, emit them as a
named cREXX array, and optionally call the named processor after the complete
array has been emitted. This provides a stable generic mechanism for Prolog,
SQL, JSON, grammar, and similar consumers without dynamically adding RXPP
keywords.

Existing hardcoded thin aliases of `##DATA` remain unchanged in syntax and
behavior: `##RELATION`, `##PROGRAM`, `##LIBRARY`, `##RULE`, `##INPUT`, and
`##SYS*` are not replaced or redefined by this work.

## Acceptance criteria

- **AC-01:** `##STREAM type name` collects each following source line verbatim
  through `##END` and emits the same array representation as `##DATA name`.
  Evidence: the focused fixture now includes a positive STREAM block; execution
  remains pending because CTest process launch is blocked by the environment.
- **AC-02:** `##STREAM type name WITH processor` emits the callback only after
  the complete array and passes the named array using the existing DATA callback
  convention. Evidence: focused callback-order assertions are present; execution
  remains pending because CTest process launch is blocked by the environment.
- **AC-03:** Existing hardcoded DATA aliases retain their current output and
  callback behavior. Evidence: existing assertions remain unchanged in
  `rxpp_data_callback`; execution remains pending for the same reason.
- **AC-04:** Malformed STREAM headers and missing terminators produce bounded,
  actionable RXPP diagnostics or the established RXPP failure behavior.
  Evidence: implementation and diagnostic catalog entries are present; focused
  negative execution coverage remains open.
- **AC-05:** RXPP documentation describes STREAM syntax, processing, callback
  association, limitations, and its distinction from existing DATA aliases and
  CFLAGS. Evidence: synchronized RXPP context, reference, and user-guide docs.
- **AC-06:** Editor tokenization recognizes `##STREAM` and its header without
  changing existing directive handling. Source recognition is updated; focused
  parser-mode execution remains open because `rxpp_sh` is not in this build.

## Implementation steps

1. **STEP-01 (AC-01/02/04):** Add `##STREAM` dispatch and a dedicated handler
   that validates `stream-type`, `stream-name`, optional `WITH processor`,
   captures through `##END`, and reuses the proven DATA array/callback emission
   semantics.
2. **STEP-02 (AC-03):** Preserve and regression-check all existing hardcoded
   DATA aliases without routing them through STREAM.
3. **STEP-03 (AC-04):** Add focused positive, callback-order, and malformed
   STREAM fixtures using the repository's CMake test conventions.
4. **STEP-04 (AC-05):** Update `RXPP_PREPROCESSOR.md`, the RXPP language
   reference, the RXPP user guide, and relevant module/reference documentation.
5. **STEP-05 (AC-06):** Extend `rxpp-sh` shallow token recognition for STREAM
   headers and add or update its focused test.
6. **STEP-06 (AC-01/02/03/04/06):** Run the smallest decisive RXPP test set,
   inspect generated output, and update this plan with retained evidence. The
   RXPP target built successfully before the final declaration-only warning
   cleanup; CTest could not start its CMake child processes in this environment.

## Scope decisions

- STREAM is additive; it does not dynamically extend the RXPP directive
  vocabulary and does not alter existing aliases.
- The initial callback receives the generated array name, matching `##DATA`.
- `stream-type` and `stream-name` are required non-empty header tokens but have no
  built-in semantic dispatch in this first implementation. Processor selection
  is explicit through `WITH processor`.
- Nested STREAM blocks and alternate terminators are out of scope.
