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

The standard option abbreviation rule is that the highlighted initial capital
letter is sufficient and trailing characters are ignored. For example, `R`,
`Results`, and any other word beginning with `R` select Results in classic
Rexx. cREXX should eventually follow that rule for both static `TRACE` and
dynamic `TRACE VALUE`.

Useful primary references:

- IBM z/OS REXX [`TRACE`](https://www.ibm.com/docs/en/zos/2.5.0?topic=instructions-trace)
- IBM z/OS REXX [alphabetic TRACE options](https://www.ibm.com/docs/en/zos/2.5.0?topic=trace-alphabetic-character-word-options)
- IBM z/OS REXX [prefix TRACE options](https://www.ibm.com/docs/en/zos/2.5.0?topic=trace-prefix-options)
- IBM z/OS REXX [numeric TRACE options](https://www.ibm.com/docs/en/zos/2.5.0?topic=trace-numeric-options)
- IBM z/OS REXX [TRACE output format](https://www.ibm.com/docs/en/zos/2.5.0?topic=trace-format-output)

## Accepted Names

Target standard names:

| Letter | Full word | Required meaning |
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

Current cREXX static `TRACE` accepts these names:

- Standard-ish source modes: `A`, `ALL`, `C`, `COMMAND`, `COMMANDS`, `E`,
  `ERROR`, `ERRORS`, `F`, `FAILURE`, `FAILURES`, `I`, `INTERMEDIATE`,
  `INTERMEDIATES`, `L`, `LABEL`, `LABELS`, `N`, `NORMAL`, `R`, `RESULT`,
  `RESULTS`.
- Off modes: `O`, `OFF`.
- cREXX extensions: `ASM`, `LLM`, `JSON`, `META`, `METADATA`.
- Prefix/status forms: leading `?`, signed integer settings, and
  `TRACE VALUE expr`.
- Output sinks: `TO STDOUT`, `TO STDERR`, `TO FILE expr`, and `TO expr`.

Current gaps:

- Static `TRACE` does not yet accept arbitrary trailing characters after a
  valid standard option letter.
- `!` command inhibition is not accepted.
- `S`/`Scan` is not accepted.
- `TRACE` with no option does not yet restore defaults.
- The public `TRACE()` BIF is not wired to the breakpoint-backed trace runtime.

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

Current cREXX text output is intentionally simpler and provisional:

```text
(line:column) escaped-source
```

For example, source text containing a backslash is printed visibly as
`return \\flag`. This is good for immediate metadata debugging, but it is not
yet the standard Rexx `line *-* clause` layout.

## Output By Option

| Option | Required standard records | Current cREXX status |
| --- | --- | --- |
| `A` / `All` | `*-*` for all executed clauses before execution. | Accepted; currently maps to source-clause trace only. |
| `C` / `Commands` | `*-*`/command text before each host command; `+++` return-code messages for command errors/failures. | Accepted; currently maps to source-clause trace, not host-command events. |
| `E` / `Error` | `+++` for host commands with error or failure return status after execution. | Accepted; currently maps to source-clause trace, not command result events. |
| `F` / `Failure` | `+++` for host commands with failure return status after execution. | Accepted; currently maps to source-clause trace, not command result events. |
| `I` / `Intermediates` | `*-*`, final `>>>`, and intermediate `>C>`, `>F>`, `>L>`, `>O>`, `>P>`, `>V>` records. | Accepted; currently maps to source-clause trace only. Symbol and variable values appear feasible from interrupt-frame metadata, but operation-result tracing needs compiler or VM expression instrumentation. |
| `L` / `Labels` | `*-*` for labels passed during execution. | Accepted; currently maps to source-clause trace. Needs label/pass events. |
| `N` / `Normal` | Default; `+++` for failing host commands after execution. | Accepted; currently maps to source-clause trace. Default no-option reset is not implemented. |
| `O` / `Off` | No trace output; reset `?` and `!` prefix states. | Implemented for breakpoint trace disable. Prefix reset is incomplete because `!` is not implemented. |
| `R` / `Results` | `*-*` for all clauses and `>>>` final expression results. | Accepted; currently maps to source-clause trace only. Final expression result events are not emitted. |
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
   - accept arbitrary trailing characters after valid standard option letters;
   - implement `TRACE` with no option as default reset;
   - add the public `TRACE()` BIF, returning the previous setting and applying a
     new setting when an argument is supplied.

2. Standard text output:
   - add `line *-* source` output;
   - add `+++`, `>>>`, `>.>`, and intermediate prefixes;
   - preserve the current escaped-source behavior, either as the default cREXX
     safety policy or behind a trace formatting option.

3. Event coverage:
   - add host-command before/after events in ADDRESS dispatch for `C`, `E`, `F`,
     and `N`;
   - add label-pass events for `L`;
   - add final expression result events for `R`;
   - add variable, literal, function, operation, prefix-operation, and compound
     variable events for `I`.

4. Interactive mode:
   - implement `?` as a real toggle, not only accepted syntax;
   - implement numeric skip/suppress behavior;
   - add a minimal prompt command set and stress tests for long-running traces.

5. Command inhibition:
   - implement `!` in ADDRESS dispatch so host commands are traced but not
     executed, with `RC` set to `0`.

