# RXPP variadic forwarding plan

## Vision and intended outcome

RXPP currently uses bare `...` for a repeating macro body.  Add a separate,
named forwarding form, `...args`, for macro facades that must accept and pass
through an arbitrary number of arguments exactly once.  The form must work in
both C-style `##define`/`##define cmd` macros and scripted `##MACRO` definitions,
so command facades such as GLOBALV do not need an arbitrary fixed argument
limit.  Existing bare-`...` macros retain their current meaning and behavior.

## Acceptance criteria

- **AC-01** — Existing bare-`...` repeat-body macros continue to expand with
  their current output; the existing macro regression tests pass.
- **AC-02** — A C-style function or command macro declared with terminal
  `...args` accepts zero, one, or many actual arguments and forwards them once
  in source order.
- **AC-03** — A scripted `##MACRO` declared with terminal `...args` receives
  the complete actual argument sequence once, without silently truncating
  extra arguments.
- **AC-04** — A GLOBALV-style facade can forward arbitrary operands without a
  fixed ten-parameter wrapper and produces the intended single command.
- **AC-05** — Fixed-arity macros retain their existing argument-count
  diagnostics, while forwarding macros are not reported as fixed-arity
  mismatches.
- **AC-06** — A malformed forwarding marker, including a non-terminal marker,
  is rejected clearly by RXPP rather than being treated as an ordinary name.
- **AC-07** — The RXPP macro documentation and a maintained demonstration or
  regression fixture describe the new form and its distinction from bare `...`.
- **AC-08** — Focused RXPP tests pass, and generated source remains valid for
  the normal RXPP-to-cREXX pipeline.

## Implementation steps

- **STEP-01** — Inspect the existing C-style and scripted macro parsers and
  identify the shared quote-aware argument representation. (AC-01, AC-05)
- **STEP-02** — Extend macro-header parsing to recognize only a terminal named
  forwarding marker such as `...args`, while preserving bare `...`. (AC-02,
  AC-05, AC-06)
- **STEP-03** — Implement one-shot forwarding for C-style and command macros,
  preserving argument order and source spelling where it is needed by a
  facade. (AC-02, AC-04)
- **STEP-04** — Implement the corresponding scripted-macro binding for the
  complete argument sequence, including zero arguments. (AC-03, AC-04)
- **STEP-05** — Add focused regression coverage and update the GLOBALV demo to
  exercise the forwarding form. (AC-01, AC-02, AC-03, AC-04, AC-08)
- **STEP-06** — Document syntax, semantics, quoting expectations, and the
  distinction from the existing repeat-body form. (AC-07)
- **STEP-07** — Run focused RXPP tests and inspect generated CREXX/RXAS for
  the forwarding and legacy cases. Record results here. (AC-08)

## Status

STEP-01 is complete: the existing C-style repeat path and scripted-macro
argument binding were inspected, and both use quote-aware argument state that
can support one-shot forwarding.

STEP-02 through STEP-06 are implemented in the working tree: terminal named
forwarding markers are parsed and validated, C-style and scripted expansion
paths forward one argument sequence, documentation and the GLOBALV demo are
updated, and `rxpp_variadic_forwarding` covers ordinary, quoted, fixed-prefix,
and malformed-marker cases.

STEP-07 is verified for the focused path: the optimized `rxpp` target built
successfully, the `rxpp_variadic_forwarding` CMake script passed directly,
and the maintained GLOBALV demonstration was preprocessed without a panic.
The generated demonstration source contains one `ADDRESS GLOBALV` command per
call, and its generated source compiled through `rxc` and `rxas`. The runtime
GLOBALV invocation itself was not used as a pass criterion because this check
did not initialize/load the external GLOBALV runtime environment.

During verification, three implementation defects were repaired: a multi-
character `strip` delimiter, reversed `changestr` arguments, and loss of
commas inside quoted forwarded arguments during scripted-macro normalization.
The focused test fixture also had a call spelling typo (`EMITPREFIX` instead
of the declared `EMITPREFIXED`), which was corrected. AC-01 through AC-08 are
now covered by the focused implementation and checks; the broad full build
remains outside this focused verification.

## Follow-up: quote-aware demo argument iteration

The maintained forwarding fixture now uses `qwords(args)` when iterating a
forwarded argument string. Ordinary `words()` treats a quoted value containing
spaces as multiple words; that is independent of comma preservation by the
native macro argument splitter. The fixture therefore exercises the same
quote-aware contract used by the variadic facade itself.

## Follow-up: preserve quotes in scripted forwarding

The scripted forwarding call is assembled as a single-quoted RexxScript
assignment. RXPP must therefore escape only embedded single quotes; doubling
double quotes changes them into literal escaped quotes before `qwords()` and
`qword()` receive the value. The focused fixture also doubles quotes in its
display copy before placing the forwarded text inside a generated source
string; the original `args` value remains quote-aware for iteration.

Focused verification after rebuilding `rxpp`:

```text
cmake --build cmake-build-release --target rxpp --parallel 1
cmake -D... -P preprocessor/tests/run_rxpp_variadic_forwarding.cmake
```

Both commands passed. The generated source compiled through `rxc`, and the
quoted arguments `"beta gamma"` and `"beta,gamma"` remained single tokens.
