# cREXX TRACE Requirements And Status

This note records the intended TRACE compatibility target, the current cREXX
implementation status, and the likely enhancement path. It covers both the
`TRACE` instruction and the companion `TRACE()` built-in function.

## Compatibility Baseline

Classic Rexx TRACE has three option families:

- Alphabetic word options: `All`, `Commands`, `Error`, `Failure`,
  `Intermediates`, `Labels`, `Normal`, `Off`, `Results`, and `Scan`.
- Prefix options: `?` for interactive debug and `!` for host-command
  inhibition.
- Numeric options: positive numbers skip interactive pauses; negative numbers
  temporarily suppress trace output and pauses for matching clauses.

cREXX follows a CMS-style minimum-abbreviation rule for implemented TRACE
options: the supplied option must be a left-prefix of the canonical option word
and must be at least the documented minimum length. Classic Rexx options use a
one-letter minimum (`R`, `RE`, `RES`, `RESULT`, and `RESULTS` all select
Results). Non-prefix spellings such as `RAS` are rejected instead of being
treated as Results.

Useful primary references:

- IBM z/OS REXX [`TRACE`](https://www.ibm.com/docs/en/zos/2.5.0?topic=instructions-trace)
- IBM z/OS REXX [alphabetic TRACE options](https://www.ibm.com/docs/en/zos/2.5.0?topic=trace-alphabetic-character-word-options)
- IBM z/OS REXX [prefix TRACE options](https://www.ibm.com/docs/en/zos/2.5.0?topic=trace-prefix-options)
- IBM z/OS REXX [numeric TRACE options](https://www.ibm.com/docs/en/zos/2.5.0?topic=trace-numeric-options)
- IBM z/OS REXX [TRACE output format](https://www.ibm.com/docs/en/zos/2.5.0?topic=trace-format-output)

## Accepted Names

Target standard names:

| Minimum | Full word | Required meaning |
| --- | --- | --- |
| `A` | `All` | Trace all clauses before execution. |
| `C` | `Commands` | Trace host commands before execution and trace command return codes when they error or fail. |
| `E` | `Error` | Trace host commands after execution when they return an error or failure, with the return code. |
| `F` | `Failure` | Trace host commands after execution when they fail, with the return code. |
| `I` | `Intermediates` | Trace all clauses and expression intermediate values. |
| `L` | `Labels` | Trace labels passed during execution. |
| `N` | `Normal` | Default: trace failing host commands after execution. |
| `O` | `Off` | Disable trace and reset prefix options. |
| `R` | `Results` | Trace all clauses and final expression results. |
| `S` | `Scan` | Syntax-scan remaining clauses without executing them. |
| `AS` | `ASM` | cREXX extension: VM/RXAS instruction trace. |
| `LL` | `LLM` | cREXX extension: JSON-lines-style trace records for tooling. |
| `ENV` | `ENV` | cREXX extension: re-read `CREXX_TRACE` / `CREXX_TRACE_TO` and apply that trace setting. |

Current cREXX `TRACE` accepts these names for static options, and
`TRACE VALUE expr` normalizes dynamic option values with the same rules:

- Implemented classic modes: any valid left-prefix of `ALL`, `COMMAND`,
  `COMMANDS`, `ERROR`, `ERRORS`, `FAILURE`, `FAILURES`, `INTERMEDIATE`,
  `INTERMEDIATES`, `LABEL`, `LABELS`, `NORMAL`, `OFF`, `RESULT`, or
  `RESULTS`, with a one-character minimum.
- cREXX extensions: `AS`/`ASM` and `LL`/`LLM`; `JSON`, `META`, and `METADATA`
  are exact aliases for `LLM`; exact `ENV` reads trace settings from the
  environment at that program point.
- Legacy cREXX source trace spelling: exact `REXX`. It is not abbreviated,
  because `R` and `RE...` belong to `RESULTS`.
- Prefix/status forms: leading `?`, signed integer settings, and
  `TRACE VALUE expr`.
- Output sinks: `TO STDOUT`, `TO STDERR`, `TO FILE expr`, and `TO expr`.
- Runtime environment override: `TRACE ENV` reads `CREXX_TRACE` and
  `CREXX_TRACE_TO` each time it executes. `CREXX_TRACE` uses the same mode
  normalization rules as `TRACE VALUE`; `CREXX_TRACE_TO` uses the same sink
  rules as `TO`. An empty `CREXX_TRACE` turns tracing off. An invalid
  `CREXX_TRACE` turns tracing off and emits a `+++` trace message.

Current gaps:

- `!` command inhibition is not accepted.
- `S`/`Scan` is not accepted.
- `TRACE` with no option does not yet restore defaults.
- Environment-variable tracing still needs an explicit compiled `TRACE ENV`
  marker; there is no compiler/runtime bootstrap path for tracing a program
  that has no TRACE instruction at all.
- The public `TRACE()` BIF is not wired to the breakpoint-backed trace runtime.
- The ADDRESS condition taxonomy is still coarse. `TRACE N` and `TRACE E`
  currently report any non-zero `RC` or condition; `TRACE F` reports negative
  `RC` or a `FAILURE` condition.

## Standard Text Output Format

Standard text TRACE output should be line-oriented:

```text
line-number indentation prefix content
```

The standard prefix tags are:

| Prefix | Required meaning |
| --- | --- |
| `*-*` | Source text of a single clause. |
| `+++` | Trace message, command return code, interactive prompt, syntax traceback, or similar processor message. |
| `>>>` | Result of an expression, parse assignment, or subroutine return value. |
| `>=>` | Value assigned to a variable by an assignment clause. Observed in Regina-compatible output for `TRACE R`; cREXX should treat this as the assignment-result form unless the final compatibility target says otherwise. |
| `>.>` | Value assigned to a PARSE placeholder period. |
| `>C>` | Compound-variable name after substitution, before value fetch. Required for `TRACE I`; useful for stem/compound debugging. |
| `>F>` | Function call result. Intermediates only. |
| `>L>` | Literal, uninitialized variable, or constant symbol. Intermediates only. |
| `>O>` | Binary operation result. Intermediates only. |
| `>P>` | Prefix operation result. Intermediates only. |
| `>V>` | Variable contents. Required for `TRACE I`; also appears in Regina-compatible `TRACE R` for variable substitutions that contribute to an expression or command. |

The standard format encloses result values in double quotes so leading and
trailing blanks are visible. Control characters may be made display-safe.

Current cREXX text output uses the standard prefix vocabulary for implemented
events:

```text
     5 *-* escaped-source
       >=>   "escaped-assignment-result"
       +++   RC=-3 ENVIRONMENT escaped-command
```

The source line number is right-aligned in a six-character field. cREXX does
not yet add nesting indentation. Source, command, and result text is escaped so
backslashes, quotes, newlines, and other control characters stay visible and a
single trace event stays on a single output line. For example, source text
containing a backslash is printed visibly as `return \\flag`.

## Output By Option

| Option | Required standard records | Current cREXX status |
| --- | --- | --- |
| `A` / `All` | `*-*` for all executed clauses before execution. | Implemented for source clauses with standard `*-*` text output. |
| `C` / `Commands` | `*-*`/command text before each host command; `+++` return-code messages for command errors/failures. | Implemented for ADDRESS dispatch. Command-before records use `       *-* command`; non-zero `RC` or condition also emits `+++`. |
| `E` / `Error` | `+++` for host commands with error or failure return status after execution. | Implemented for ADDRESS dispatch as non-zero `RC` or any condition. |
| `F` / `Failure` | `+++` for host commands with failure return status after execution. | Implemented for ADDRESS dispatch as negative `RC` or `FAILURE` condition. |
| `I` / `Intermediates` | `*-*` plus intermediate `>C>`, `>F>`, `>L>`, `>O>`, `>P>`, `>V>`, and related final-result records. | Accepted. Uses `.srcstep` source records and `.traceevent` semantic hints. Initial compiler coverage includes assignments, variable reads, literals, and simple operation/prefix/function-result events where a register or constant still exists. Full expression and compound coverage is not complete. |
| `L` / `Labels` | `*-*` for labels passed during execution. | Accepted, but no label-pass events are emitted yet. |
| `N` / `Normal` | Default; `+++` for failing host commands after execution. It should not trace every statement. | Implemented for ADDRESS dispatch and intentionally quiet for ordinary statements. Default no-option reset is not implemented. |
| `O` / `Off` | No trace output; reset `?` and `!` prefix states. | Implemented for breakpoint trace disable. Prefix reset is incomplete because `!` is not implemented. |
| `R` / `Results` | `*-*` for all clauses, `>V>` for variable substitutions used by expressions or commands, and `>=>`/`>>>` final results as appropriate. | Implemented with source clauses plus visible `.traceevent` values. Assignment results use `>=>`; variable reads use `>V>`. `.tracecontroller` owns structured metadata lookup and pending value state; the generated signal helper performs only the frame-local `metalinkpreg` read for registers named by trace-event metadata. Full final-expression, stem/compound, and subroutine-return result coverage is not complete. |
| `S` / `Scan` | Syntax-scan remaining clauses without executing them. | Not implemented or accepted. Requires compiler/runtime scan semantics separate from normal execution. |
| `ASM` | cREXX extension: VM/RXAS instruction trace with source when available. | Implemented. |
| `LLM` | cREXX extension: structured trace records for tooling. | Implemented. |

## Regina Compatibility Observations

Local Regina probes are useful as a classic Rexx compatibility reference. For
this program:

```rexx
trace results
s. = "default"
s.1 = "one"
i = 1
x = s.i
say x s.2
```

Regina `TRACE R` emits assignment results with `>=>`, variable substitution
values with `>V>`, and both the compound tail value and the resolved compound
value for `x = s.i`. Regina `TRACE I` emits the same variable values, plus
`>C>` records for resolved compound names such as `S.1`, and `>O>` records for
operation results such as a concatenated `SAY` expression.

That behavior means `TRACE R` is not limited to assignment values, and
`TRACE I` is not just `TRACE R` plus one final expression record. Stems and
compound variables require the compiler/runtime to know the evaluated tail, the
resolved compound name, and the fetched or assigned value at the point those
events happen.

Ordering matters. Regina prints the `*-*` source line before the values caused
by that source clause; the value records appear after the producing operation
has made the value available and before the next source clause is displayed.
cREXX models that by attaching trace-event metadata at the VM address where the
value is available, then letting the generated signal helper read only the
named frame-local register before continuing.

## Beta Health Warnings

The current implementation is suitable for beta use, but not yet a faithful
classic TRACE implementation:

- `.meta_reg` is scope/register metadata. It is emitted when a register comes
  into scope, not when a variable changes. It is appropriate for lookup-style
  tools such as `VALUE()`, including search-backwards behavior, but it is not a
  reliable trace-result event stream.
- Assignment-result capture no longer parses the left hand side from source
  text. It uses `.traceevent` records emitted where the compiler still has a
  register or constant value to name. Optimised-away and folded values may be
  absent rather than recreated solely for TRACE.
- Desugaring passes that erase source-level intent before emission should attach
  an `AST_SEMANTIC_CONTEXT` sidecar to the lowered node. The sidecar is a
  generic rxc-to-emitter/exits note, not TRACE-specific execution syntax. TRACE
  currently consumes it for stem/property/index sugar so lowered `get`/`set`
  member calls can still produce source-level compound, read, and assignment
  events while suppressing compiler-created receiver/key-building operations.
  Optimiser folding preserves this internal marker for synthetic property-tail
  keys instead of reintroducing literal TRACE noise.
- `.srcstep` metadata is self-contained exact instruction/source-step metadata:
  pooled file name, whole source line, active range, step id, clause id, and
  provenance flags. It is valuable for debugger stepping and `TRACE LLM`, while
  classic text TRACE can group by authored clause to suppress unhelpful
  generated fragments.
- Compound/indexed TRACE is still partial. The compiler can emit value events
  for many register-backed reads and writes, but final resolved compound-name
  `>C>` coverage needs additional codegen metadata at the point the runtime
  tail value is available.
- The default filtering of trace-runtime procedures is currently heuristic and
  substring-based. It works for the initial exit/runtime split, but it is brittle
  and should be replaced with explicit trace roles plus stack-frame/history
  rules.
- Classic text formatting now belongs to the generated TRACE exit handler. The
  shared `rxfnsb.trace` layer still owns structured metadata lookup, controller
  state, output plumbing, and frame-read coordination helpers.

The recommended beta stance is to describe `TRACE R` and `TRACE I` result
records as partial, and to use `TRACE LLM` primarily for inspecting actual
`.srcstep`/instruction metadata rather than as proof of full classic TRACE
compatibility.

## cREXX LLM Output Format

`TRACE LLM` emits one JSON-lines-style object per trace event. Current fields:

```json
{
  "event": "trace",
  "format": "llm",
  "mode": "REXX",
  "module": "1",
  "address": "27",
  "procedure": "main",
  "line": "6",
  "column": "3",
  "source": "return \\\\flag",
  "asm": "..."
}
```

All values are currently encoded as strings. Source text is escaped so
backslashes and control characters remain visible and one trace event stays on
one output line. This format is intended for:

- validating `.srcstep` metadata emitted by `rxc`;
- feeding debugger tools and LLM-assisted test triage;
- avoiding ambiguity when the VM interrupt lands on partial source-line
metadata rather than a whole authored line.

## Interactive Debug Feasibility

Interactive TRACE looks feasible because the breakpoint handler already runs
with the interrupted frame available. The same frame-safe discipline documented
for `rxdb` watch values applies: symbol/register inspection must happen inside
the interrupt handler while the child frame is still current.

The main risk is not feasibility; it is controlling event volume and blocking
behavior. A long-running program in `TRACE ?I` could pause or allocate on a very
large number of interrupts. Recommended requirements before enabling it:

- Maintain explicit trace state: active option, interactive toggle, command
  inhibition toggle, pause-skip count, and temporary suppress count.
- Pause only on events that the active trace option would actually print.
- Implement numeric options before or with interactive mode, so users can skip
  pauses or suppress trace for hot paths.
- Provide an immediate `TRACE O`/quit path from the prompt.
- Avoid prompting when input is not interactive unless explicitly forced.
- Keep `TRACE LLM TO FILE` noninteractive and streaming-friendly.

## Enhancement Roadmap

1. Parser and option state compatibility:
   - accept `!`;
   - accept `S`;
   - implement `TRACE` with no option as default reset;
   - add a compiler/runtime bootstrap path for `CREXX_TRACE` when a program has
     no explicit `TRACE ENV` statement;
   - add the public `TRACE()` BIF, returning the previous setting and applying a
     new setting when an argument is supplied.

2. Standard text output:
   - add nesting indentation;
   - use `>=>` for assignment-result records;
   - add `>.>` and intermediate prefixes;
   - broaden result output beyond the current simple-assignment subset.

3. Event coverage:
   - add label-pass events for `L`;
   - add full final expression result events for `R`;
   - add variable substitution events for `R`;
   - add variable, literal, function, operation, prefix-operation, assignment,
     and compound-variable events for `I`.

4. Compiler-provided trace event hints:
   - broaden the existing `.traceevent` metadata coverage separate from
     `.meta_reg`, including nesting depth, generated-fragment role, final
     expression results, subroutine returns, and resolved compound names where
     applicable;
   - define source ids as metadata identities, not addresses: `source_step_id`
     identifies one source-step anchor in a module, while `clause_id` groups all
     source steps and trace events for one logical authored clause; `0` means no
     anchor, and linked-image consumers should qualify ids by module;
   - have the generated signal helper read only registers named by trace-event
     hints, keeping frame-sensitive `metalinkpreg` use local to the interrupted
     frame;
   - treat optimized-away or folded values as absent events; the compiler emits
     trace-event hints for values that still exist at an address, rather than
     recreating values solely for TRACE;
   - keep `.srcstep` exact-address metadata available for debuggers and
     `TRACE LLM`, but do not make classic text TRACE infer semantic results
     from source text.

5. Structured driver and formatter split:
   - move semantic event collection into a generic trace-event driver;
   - move classic text formatting, Regina/IBM compatibility decisions, and
     cREXX extension formatting into a TRACE-specific formatter;
   - replace substring runtime-procedure filtering with explicit module/procedure
     trace roles and stack-frame/history rules.

6. Interactive mode:
   - implement `?` as a real toggle, not only accepted syntax;
   - implement numeric skip/suppress behavior;
   - add a minimal prompt command set and stress tests for long-running traces.

7. Command inhibition:
   - implement `!` in ADDRESS dispatch so host commands are traced but not
     executed, with `RC` set to `0`.
