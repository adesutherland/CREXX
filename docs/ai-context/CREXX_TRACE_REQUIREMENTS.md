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

Current cREXX `TRACE` accepts these names for static options, and
`TRACE VALUE expr` normalizes dynamic option values with the same rules:

- Implemented classic modes: any valid left-prefix of `ALL`, `COMMAND`,
  `COMMANDS`, `ERROR`, `ERRORS`, `FAILURE`, `FAILURES`, `INTERMEDIATE`,
  `INTERMEDIATES`, `LABEL`, `LABELS`, `NORMAL`, `OFF`, `RESULT`, or
  `RESULTS`, with a one-character minimum.
- cREXX extensions: `AS`/`ASM` and `LL`/`LLM`; `JSON`, `META`, and `METADATA`
  are exact aliases for `LLM`.
- Legacy cREXX source trace spelling: exact `REXX`. It is not abbreviated,
  because `R` and `RE...` belong to `RESULTS`.
- Prefix/status forms: leading `?`, signed integer settings, and
  `TRACE VALUE expr`.
- Output sinks: `TO STDOUT`, `TO STDERR`, `TO FILE expr`, and `TO expr`.
- Runtime environment override: `CREXX_TRACE` supplies the initial trace mode
  and `CREXX_TRACE_TO` supplies the initial output sink. The override is applied
  once, during the first generated TRACE setup, and uses the same mode and sink
  normalization rules as the instruction form.

Current gaps:

- `!` command inhibition is not accepted.
- `S`/`Scan` is not accepted.
- `TRACE` with no option does not yet restore defaults.
- Environment-variable tracing still needs at least one compiled `TRACE`
  statement so the certified exit imports `rxfnsb.trace` and emits the
  breakpoint helper.
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
| `>.>` | Value assigned to a PARSE placeholder period. |
| `>C>` | Compound-variable name after substitution, before value fetch. Intermediates only. |
| `>F>` | Function call result. Intermediates only. |
| `>L>` | Literal, uninitialized variable, or constant symbol. Intermediates only. |
| `>O>` | Binary operation result. Intermediates only. |
| `>P>` | Prefix operation result. Intermediates only. |
| `>V>` | Variable contents. Intermediates only. |

The standard format encloses result values in double quotes so leading and
trailing blanks are visible. Control characters may be made display-safe.

Current cREXX text output uses the standard prefix vocabulary for implemented
events:

```text
     5 *-* escaped-source
       >>>   "escaped-result"
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
| `I` / `Intermediates` | `*-*`, final `>>>`, and intermediate `>C>`, `>F>`, `>L>`, `>O>`, `>P>`, `>V>` records. | Accepted. Currently emits source records and the same simple-assignment `>>>` subset as `R`; intermediate records still need compiler or VM expression instrumentation. |
| `L` / `Labels` | `*-*` for labels passed during execution. | Accepted, but no label-pass events are emitted yet. |
| `N` / `Normal` | Default; `+++` for failing host commands after execution. It should not trace every statement. | Implemented for ADDRESS dispatch and intentionally quiet for ordinary statements. Default no-option reset is not implemented. |
| `O` / `Off` | No trace output; reset `?` and `!` prefix states. | Implemented for breakpoint trace disable. Prefix reset is incomplete because `!` is not implemented. |
| `R` / `Results` | `*-*` for all clauses and `>>>` final expression results. | Implemented for source clauses plus `>>>` for simple assignment targets where `.meta_reg`/`.meta_const` identifies the left-hand variable. `.tracecontroller` owns the metadata lookup and pending value state; the generated signal helper only performs the frame-local `metalinkpreg` read that must happen before returning to the shared runtime handler. Full expression and subexpression result coverage is not complete. |
| `S` / `Scan` | Syntax-scan remaining clauses without executing them. | Not implemented or accepted. Requires compiler/runtime scan semantics separate from normal execution. |
| `ASM` | cREXX extension: VM/RXAS instruction trace with source when available. | Implemented. |
| `LLM` | cREXX extension: structured trace records for tooling. | Implemented. |

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

- validating `.src` metadata emitted by `rxc`;
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
     no explicit `TRACE` statement;
   - add the public `TRACE()` BIF, returning the previous setting and applying a
     new setting when an argument is supplied.

2. Standard text output:
   - add nesting indentation;
   - add `>.>` and intermediate prefixes;
   - broaden `>>>` beyond the current simple-assignment subset.

3. Event coverage:
   - add label-pass events for `L`;
   - add full final expression result events for `R`;
   - add variable, literal, function, operation, prefix-operation, and compound
     variable events for `I`.

4. Interactive mode:
   - implement `?` as a real toggle, not only accepted syntax;
   - implement numeric skip/suppress behavior;
   - add a minimal prompt command set and stress tests for long-running traces.

5. Command inhibition:
   - implement `!` in ADDRESS dispatch so host commands are traced but not
     executed, with `RC` set to `0`.
