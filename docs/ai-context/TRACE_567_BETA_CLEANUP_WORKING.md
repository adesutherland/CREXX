# TRACE/#567 Beta Cleanup Working Notes

This is a durable working note for the TRACE/debug metadata cleanup so context
compaction does not lose the agreed design. It records the current reviewed
state, the approved direction, the compact-code adjustment, and the staged
implementation plan.

## Reviewed Baseline

Current binary and metadata state:

- `binutils/include/rxbin.h` currently uses `BIN_VERSION "003"`.
- Current source metadata is split across sticky `META_FILE` and fragment
  `META_SRC`.
- `meta_src_constant` carries `line`, `column`, and a pooled source fragment.
- `meta_file_constant` carries a pooled file name and remains sticky until the
  next `.srcfile`.
- `.meta_reg` records scope/register placement, not value changes. It must not
  be treated as an assignment or expression event stream.
- `META_INLINE` currently transports compiler inline payload version `I4`.

Current assembler/disassembler/linker/runtime state:

- `rxas` accepts `.srcfile="file"` and `.src line:column="fragment"`.
- `rxdas` round-trips `.srcfile` and `.src`.
- `rxlink` rewrites metadata pool references and strips only `META_SRC` and
  `META_FILE` for source stripping. It strips `META_INLINE` by default in
  linked images unless inline metadata is preserved.
- `rxvm` panic/source lookup walks the metadata chain to the target address,
  remembering sticky `META_FILE` and closest `META_SRC`.
- `metaloaddata` exposes `.meta_src` as `[line, column, source]` and
  `.meta_file` as `[file]`.

Current compiler/source state:

- `compiler/rxcp_emit_core.c` emits `.srcfile` when the file changes and emits
  `.src` fragments for source anchors.
- Source spans can be partial fragments, including generated loop pieces and
  bracket/index expressions whose span is not the whole authored line.
- `compiler/rxcp_source_tree.*` preserves source nodes with file, token,
  source start/end, line, and column.
- `compiler/rxcp_inline.c` exports `I4` source entries as
  `;u,<source-id>,<file-id|-1>,<line>,<column>,<SourceProvenance>,<hex-source-text>`.
  The source text is the active fragment, not necessarily the whole line.
- Inline import preserves callee file/provenance, so the new source-step
  emitter must retain original callee file and authored line for inlined code.

Current TRACE/debug runtime state:

- `lib/rxfnsb/rexx/trace.crexx` currently owns too much formatting and also
  guesses assignment results from `.src` text plus `.meta_reg`/`.meta_const`.
- `compiler/exits/trace/Trace.crexx` generates the signal handler and can
  safely read frame-local pending registers with `metalinkpreg`.
- `debugger/rxdb.crexx` and `debugger/rxdb_gui.crexx` use shared trace context
  data and should make UI-level stepping/rendering decisions.

## Approved Direction

The cleanup is allowed to break beta metadata compatibility. The binary format
and inline metadata format may be bumped. Old `.srcfile`/`.src` may be kept as
assembler/runtime fallback during transition, but new compiler output should
move to the new contract.

Important correction from the user: compact the trace structures and strings
where feasible, and do not store final output prefixes such as `>=>` in binary
metadata. Binary TRACE events carry semantic codes. TRACE classic text, LLM
records, and RXDB UI rendering map those codes to presentation strings later.

## Binary Version

Stage 1 should bump:

```c
#define BIN_VERSION "004"
```

No released compatibility is required for the old beta metadata format.

## Source-Step Metadata

Add a self-contained source-step metadata record. It supersedes sticky
`META_FILE` plus fragment `META_SRC` for compiler-emitted source anchors.

Proposed enum addition:

```c
enum const_pool_type {
    STRING_CONST, BINARY_CONST, DECIMAL_CONST, FLOAT_CONST, PROC_CONST,
    EXPOSE_REG_CONST, EXPOSE_PROC_CONST,
    META_SRC, META_FILE, META_FUNC, META_REG, META_CONST, META_CLEAR,
    META_CLASS, META_ATTR, META_INTERFACE, META_IMPLEMENTS, META_MEMBER,
    META_INLINE,
    META_SOURCE_STEP,
    META_TRACE_EVENT
};
```

`META_TRACE_EVENT` may be defined with the version bump even if its full use is
stage 2. If that causes too much churn, add only `META_SOURCE_STEP` in stage 1
and add `META_TRACE_EVENT` in stage 2 before committing that slice.

Proposed compact source-step struct:

```c
typedef struct meta_source_step_constant {
    meta_entry base;
    size_t file;        /* STRING_CONST offset */
    size_t source_line; /* STRING_CONST offset, whole authored/generated line */
    uint32_t step_id;
    uint32_t clause_id;
    uint32_t line;                /* 1-based */
    uint32_t active_start_column; /* 1-based inclusive */
    uint32_t active_end_column;   /* 1-based exclusive */
    uint32_t flags;
} meta_source_step_constant;
```

Proposed source-step flags:

```c
#define RXBIN_SOURCE_AUTHORED   0x00000001u
#define RXBIN_SOURCE_GENERATED  0x00000002u
#define RXBIN_SOURCE_SYNTHETIC  0x00000004u
#define RXBIN_SOURCE_INLINED    0x00000008u
#define RXBIN_SOURCE_EXACT      0x00000010u
#define RXBIN_SOURCE_INHERITED  0x00000020u
#define RXBIN_SOURCE_COMPOSITE  0x00000040u
```

`step_id` and `clause_id` are local numeric ids. Stage 1 may assign both from
the same monotonic id if a richer clause-grouping map would delay the source
metadata migration. The field is present so later debug/TRACE policy can step
per address, active range, authored line, or clause.

## RXAS Source-Step Spelling

Use a compact numeric spelling, not a string flag vocabulary:

```text
.srcstep <step-id> <clause-id> <flags> "<file>" <line> <start-col> <end-col> "<whole-source-line>"
```

Example:

```text
                .srcstep 17 17 17 "example.crexx" 12 1 15 "return x + y"
```

In that example, flags `17` means `AUTHORED | EXACT`.

Keep `.srcfile` and `.src` readable for now so handwritten or existing RXAS can
still assemble during the transition, but `rxc` should emit `.srcstep` for new
source anchors.

## Source-Step Link, Strip, Disassembly, VM Behavior

Assembler:

- Add lexer/parser support for `.srcstep`.
- Intern `<file>` and `<whole-source-line>` as string constants.
- Create `META_SOURCE_STEP` at the current instruction address.

Disassembler:

- Round-trip `META_SOURCE_STEP` as `.srcstep` with decimal ids/flags and quoted
  pooled strings.
- Keep old `.srcfile`/`.src` output for old metadata while fallback support
  exists.

Linker:

- Treat `META_SOURCE_STEP` as metadata.
- Rewrite `file` and `source_line` offsets through the linked shared constant
  pool.
- Include `META_SOURCE_STEP` in source stripping along with old `META_SRC` and
  `META_FILE`.
- Preserve trace-event metadata under source stripping. If a future strip mode
  is needed for semantic TRACE metadata, add it explicitly rather than folding
  it into source stripping.

Runtime:

- `rxvm` source lookup should prefer closest `META_SOURCE_STEP` at or before
  the address.
- Runtime panic reporting should show `file:line:active_start` and the whole
  source line from the source-step record.
- Keep old `META_FILE`/`META_SRC` fallback while old RXAS remains accepted.

`metaloaddata` shape for source steps:

```text
.meta_source_step [
  0 step_id,
  1 clause_id,
  2 flags,
  3 file,
  4 line,
  5 active_start_column,
  6 active_end_column,
  7 source_line
]
```

## Inline Source Payload Update

Bump inline payload source anchors from `I4` to `I5`.

Current `I4` source record:

```text
;u,<source-id>,<file-id|-1>,<line>,<column>,<SourceProvenance>,<hex-source-text>
```

Proposed `I5` source record:

```text
;u,<source-id>,<file-id|-1>,<line>,<active-start-column>,<active-end-column>,<SourceProvenance>,<hex-whole-source-line>
```

Notes:

- Keep internal `SourceNode` line numbering consistent with the compiler's
  existing zero-based source tree. Convert to one-based only when emitting
  `.srcstep`.
- Store active start/end columns relative to the whole line. Prefer one-based
  inclusive/exclusive columns to match `.srcstep`; document and convert at the
  import/export boundary if an existing internal helper is zero-based.
- Imported inline nodes must point `source_start`/`source_end` into the owned
  whole-line buffer so the normal emitter can compute the correct active range.
- Inlined source-step records should use the original callee file and source
  line where the imported source anchor proves that origin.

## Trace-Event Metadata

Trace events are separate from source steps. They are semantic hints emitted at
the address where the value is actually available. They must not be inferred
from source text.

Binary metadata should store compact codes, not output strings. Classic
prefixes are a formatter concern:

| Semantic kind | Code | Classic formatter may render |
| --- | --- | --- |
| Source clause | `S` | `*-*` |
| Variable value | `V` | `>V>` |
| Assignment value | `A` | `>=>` |
| Compound resolved name | `C` | `>C>` |
| Literal/constant | `L` | `>L>` |
| Binary operation | `O` | `>O>` |
| Prefix operation | `P` | `>P>` |
| Function result | `F` | `>F>` |
| Final result | `R` | `>>>` |
| Trace message | `M` | `+++` |

Proposed compact trace-event struct:

```c
#define RXBIN_TRACE_REF_NONE ((size_t)-1)

typedef struct meta_trace_event_constant {
    meta_entry base;
    uint8_t kind;         /* ASCII code such as 'A', 'V', 'C' */
    uint8_t value_source; /* 'N' none, 'R' register, 'K' constant */
    uint8_t value_type;   /* compact cREXX value type code */
    uint8_t register_type;/* register bank/type code, or 0 */
    uint32_t mode_mask;   /* TRACE modes where this event is visible */
    uint32_t flags;
    size_t value_ref;     /* register number or constant offset */
    size_t symbol;        /* STRING_CONST offset or RXBIN_TRACE_REF_NONE */
    size_t resolved_name; /* STRING_CONST offset or RXBIN_TRACE_REF_NONE */
    uint32_t source_step_id;
    uint32_t clause_id;
} meta_trace_event_constant;
```

Proposed value-source codes:

```text
N = no value
R = frame-local register value; signal handler may use metalinkpreg
K = constant-pool value
```

Mode mask should be numeric bit flags, not strings:

```text
TRACE_MODE_A = 0x00000001
TRACE_MODE_R = 0x00000002
TRACE_MODE_I = 0x00000004
TRACE_MODE_C = 0x00000008
TRACE_MODE_E = 0x00000010
TRACE_MODE_F = 0x00000020
TRACE_MODE_L = 0x00000040
TRACE_MODE_ASM = 0x00000080
TRACE_MODE_LLM = 0x00000100
```

The exact bit names can live in `rxbin.h` so assembler, VM, trace runtime, and
docs share one contract.

## RXAS Trace-Event Spelling

Use one-character semantic codes in RXAS and store numeric codes in binary.
Avoid spelling final output prefixes such as `>=>`.

Preferred stage-2 spelling:

```text
.traceevent "<kind>" <mode-mask> "<value-source>" "<value-type>" "<register-type>" <value-ref> <source-step-id> <clause-id> <flags> "<symbol>" "<resolved-name>"
```

Examples:

```text
                .traceevent "A" 6 "R" "S" "s" 3 17 17 0 "a" ""
                .traceevent "V" 6 "R" "S" "s" 4 18 18 0 "a" ""
                .traceevent "C" 4 "K" "S" "" 91 19 19 0 "s" "s.1"
```

The assembler converts one-character strings to `uint8_t` codes. The
disassembler should round-trip the same compact spelling. If the parser churn
for one-character strings is not worth it, an all-integer spelling is an
acceptable fallback, but the binary contract must stay code-based.

`metaloaddata` shape for trace events:

```text
.meta_trace_event [
  0 kind_code,
  1 mode_mask,
  2 value_source_code,
  3 value_type_code,
  4 register_type_code,
  5 value_ref,
  6 source_step_id,
  7 clause_id,
  8 flags,
  9 symbol,
  10 resolved_name
]
```

Absent strings should surface to REXX as empty strings while binary metadata
uses `RXBIN_TRACE_REF_NONE`.

## Trace-Event Emission Rules

Emit trace events only where values are actually available:

- Scalar assignment and reassignment: kind `A`, visible in Results and
  Intermediates, value source register or constant.
- Variable reads in expressions, `SAY`, and `IF`: kind `V`, visible according
  to target compatibility behavior for Results/Intermediates.
- Compound/indexed name resolution: kind `C` where the resolved compound name
  is known.
- Literals/constants: kind `L` where feasible.
- Binary operation results: kind `O` where a result register is available.
- Prefix operation results: kind `P` where a result register is available.
- Function results: kind `F` where a result register is available.

The generated signal handler may use frame-local `metalinkpreg`, but only for
registers explicitly named by `META_TRACE_EVENT`. It must not scan `.meta_reg`
as a value-change log.

## Runtime/Exit Split

Shared `lib/rxfnsb/rexx/trace.crexx` should provide structured helpers only:

- decode source-step metadata;
- decode trace-event metadata;
- filter by mode, module, procedure, and trace-runtime frames;
- coordinate frame-local reads for explicitly named event registers;
- maintain breakpoints/controller state.

Formatting belongs outside the shared helper:

- Classic TRACE text and LLM rendering belong in
  `compiler/exits/trace/Trace.crexx` and its generated handler layer.
- RXDB formatting belongs in `debugger/rxdb.crexx` and
  `debugger/rxdb_gui.crexx`.
- Remove or quarantine the old source-text LHS guessing path once trace-event
  assignment metadata exists.

## Implementation Stages

The work should be divided into five commit-sized implementation steps. Each
step should run focused checks first, then the required full build and full
`ctest`, then commit if clean.

Step 1, source-step metadata plumbing:

1. Add `META_SOURCE_STEP`, compact struct, flags, and binary version bump.
2. Add `.srcstep` assembler support.
3. Add `rxdas` round-trip output.
4. Add `rxlink` rewrite and source-strip handling.
5. Add `rxvm` source lookup and `metaloaddata` exposure.
6. Keep old `.srcfile`/`.src` as fallback during the transition.

Focused checks:

- Assemble/disassemble a hand-written `.srcstep`.
- Link with source preserved and source stripped.
- Verify `metaloaddata` exposes `.meta_source_step`.

Step 2, compiler source-step emission and inline provenance:

1. Update `rxc` source emitter to emit whole-line self-contained source steps.
2. Stop new compiler output from depending on sticky `.srcfile`.
3. Update inline payload source anchors from `I4` to `I5`.
4. Ensure imported/inlined source steps use original callee file and line.
5. Add focused tests for simple clauses, counted loops, bracket/indexed
   expressions, linked preserved/stripped source, and inlined provenance.

Focused checks:

- `rxc` output contains `.srcstep` whole lines.
- Counted loops do not expose misleading standalone generated fragments as
  ordinary authored source clauses.
- Inlined calls retain callee/original source provenance.

Step 3, compact trace-event metadata plumbing:

1. Add `META_TRACE_EVENT`, compact struct, codes, mode mask, RXAS spelling,
   assembler/disassembler/linker/runtime support.
2. Add `metaloaddata` exposure for `.meta_trace_event`.
3. Preserve trace-event metadata under source stripping.
4. Keep semantic codes/masks in binary metadata; do not store presentation
   prefixes such as `>=>`.

Focused checks:

- Assemble/disassemble hand-written `.traceevent` records.
- Link source-preserved and source-stripped images and verify trace events
  survive both.
- Verify `metaloaddata` exposes compact event fields.

Step 4, compiler trace-event emission and TRACE regressions:

1. Emit initial semantic events for scalar assignments, reassignments,
   variable reads, compound resolution, literals, binary operations, and prefix
   operations where values are available.
2. Make emitted events point at registers/constants that are genuinely
   available at that address.
3. Add regression tests comparing expected TRACE output for scalar assignment,
   reassignment, `SAY`, `IF`, counted loops, and array/stem-style access.

Focused checks:

- `TRACE N` remains quiet for ordinary statements.
- `TRACE R` scalar assignment and reassignment use semantic events.
- `say a`, `if a > 0 then say a`, and `s[i]` cases no longer depend on
  source-text LHS guessing.

Step 5, runtime/exit/RXDB split and docs:

1. Refactor `rxfnsb.trace` into structured metadata/controller helpers.
2. Move classic TRACE text and LLM formatting into the TRACE exit handler.
3. Remove/quarantine source-text LHS guessing.
4. Update RXDB to use source-step and trace-event metadata directly.
5. Update `docs/ai-context/CREXX_TRACE_REQUIREMENTS.md`.
6. Update `docs/books/crexx_language_reference/statements.md`.
7. Update `docs/ai-context/CREXX_LIBS.md`.
8. Update `docs/ai-context/RXAS_ASSEMBLER.md`.
9. Update `docs/ai-context/RXLINK_LINKER.md`.
10. Update `docs/ai-context/RXVM_INTERPRETER.md`.
11. Update inliner/source provenance docs if needed.
12. Remove beta warnings fixed by these stages and keep TODOs for remaining
   TRACE modes.

Focused checks:

- Classic TRACE text formatting still matches expected output.
- RXDB source display uses source-step whole lines and active ranges.
- `return \flag` escaping remains stable.

## Regression Cases To Preserve

- `return \flag` trace source escaping.
- `TRACE N` remains quiet for ordinary statements.
- `TRACE R` scalar assignment: `a = 10`.
- `TRACE R` scalar reassignment: `a = a + 1`.
- Counted loop: `do i = 1 to 2; a = a + 1; end`.
- Counted loop trace must not print misleading standalone `to 2` or bare `i`
  as source clauses unless a debug mode explicitly asks for generated steps.
- `say a` should show variable reads in Results/Intermediates according to the
  chosen compatibility target.
- `if a > 0 then say a`.
- Indexed/stem-style access: `s[i] = "one"` and `x = s[i]`.
- Inlined function source provenance uses the callee/original source where
  appropriate.
- Linked image with source preserved.
- Linked image with source stripped.

## Testing And Commit Discipline

- Use temp logs for verbose build, `ctest`, debug, and trace output.
- Run focused tests after each slice.
- Run full `cmake --build cmake-build-debug`.
- Run full `ctest --test-dir cmake-build-debug --output-on-failure`.
- Commit after each implementation stage.
- Never stage `re2c/bootstrap/src/parse/parser.cc` or
  `re2c/bootstrap/src/parse/parser.h` unless explicitly requested.

## Open Safety Notes

- The source-step migration is approved in general, including the binary
  version bump.
- The compact-code adjustment is approved: trace-event binary metadata stores
  semantic codes and masks, not presentation prefixes.
- Pause again only if a new binary/RXAS syntax choice, output compatibility
  decision, or architecture tradeoff appears that is not covered by this note.
