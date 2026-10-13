# RXPP Optional Output Formatting Plan

## Vision and intended outcomes

Allow RXPP users to opt into the existing indentation-only CREXX formatter
after macro expansion and immediately before the final `.crexx` output is
written. The default RXPP output and build graph must remain unchanged. When
requested with `##CFLAG format`, RXPP formats the final output buffer; when
requested with `##CFLAG 4buf`, it displays that post-format buffer for
inspection. `n4buf` is enabled by default so the additional buffer is silent
unless explicitly selected.

The formatter must preserve source-map control lines and must not alter text
inside statements, strings, continuation lines, or inline compound clauses
beyond the behavior already provided by `reformat.crexx`.

## Acceptance criteria

- **AC-01:** Without `format`, RXPP output is byte-for-byte governed by the
  existing output path. Evidence: existing RXPP smoke test.
- **AC-02:** With `##CFLAG format`, RXPP applies indentation formatting after
  pass 3 and before writing the final `.crexx` file. Evidence: standalone
  formatting fixture and output inspection.
- **AC-03:** With `##CFLAG 4buf`, RXPP displays the post-format buffer; without
  it, no fourth-buffer dump is printed. Evidence: focused fixture runs.
- **AC-04:** `n4buf` is present in RXPP's default CFLAG state, while explicit
  `4buf` enables the report. Evidence: initialization inspection and fixture.
- **AC-05:** Source-map lines remain valid and are not indented as CREXX code by
  the formatter. Evidence: formatted output with source maps and smoke test.
- **AC-06:** The CMake files are unchanged. Evidence: scoped status inspection.
- **AC-07:** User and reference documentation describe `format` and `4buf`.
  Evidence: updated RXPP guides.

## Implementation steps

- **STEP-01:** Add the standalone formatter procedures to `rxpp.crexx`, with
  source-map preservation and no change to the default path. Serves AC-01,
  AC-02, and AC-05.
- **STEP-02:** Add `format` and `4buf` handling around final output and add
  `n4buf` to the default CFLAG state. Serves AC-02 through AC-04.
- **STEP-03:** Add a standalone `testx_format.rxpp` fixture without CMake
  registration. Serves AC-02, AC-03, and AC-05.
- **STEP-04:** Update the RXPP reference and User's Guide. Serves AC-07.
- **STEP-05:** Build RXPP and run focused formatting plus smoke checks. Verify
  CMake files are unchanged. Serves all criteria.

## Status and evidence

- **AC-01:** `rxpp_smoke` passes; the default path remains unchanged.
- **AC-02:** `testx_format.rxpp` produces indented output with `format`.
- **AC-03:** The same fixture prints `Source Buffer after Formatting` with
  `4buf`; `n4buf` remains the default.
- **AC-04:** `rxppinit` now includes `n4buf` in the default CFLAG state.
- **AC-05:** A source-map-enabled formatting run preserved `@` control lines
  at column 1.
- **AC-06:** No CMake files were changed.
- **AC-07:** Updated the reference, general User's Guide, and macro User's
  Guide.
- The focused RXPP build passed with the two existing unrelated Level B scope
  warnings in `rxpp.crexx`.
