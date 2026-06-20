# Level C Classic Compliance Reference

Status: extracted implementation reference for Level C syntax, evaluation,
execution, configuration, and diagnostics
Last updated: 2026-06-20

Source: publicly available Classic REXX language specification, with BIF details
separated into
`compiler/docs/levelc_classic_bifs.md`.

This is an implementation guide for cREXX Level C. It is intentionally not a
verbatim copy of the language-specification source. The goal is to capture the
rules that compiler, lowering, runtime, and library code must respect while
Level C is implemented in Level B.

## Fixed Direction

Level C will be implemented in Level B.

The Level C front end should parse and validate Classic REXX source, then lower
execution to Level B helpers and runtime libraries. The shared runtime layer is
`lib/rxfnsc`:

- `RexxValue` owns Classic scalar string/numeric/binary materialization.
- `RexxStem` owns stem-tail storage and compound-variable values.
- `RexxVariablePool` owns variable lookup, dropped state, exposure, stems, and
  host/API pool behavior.

RexxScript should reuse the same value and variable-pool foundations where its
sandboxed language surface overlaps Classic scalar behavior. RexxScript remains
smaller by design: no `ADDRESS`, host command dispatch, external calls,
ambient stream I/O, unrestricted variable-pool access, or nested `INTERPRET`.

## Compliance Boundary

A conforming Level C implementation must document configuration-dependent
choices. The important configurable surfaces are:

- accepted source character set and extra source character classes;
- uppercase mapping, character comparison, substring, length, and range order;
- stream names, default streams, persistent stream behavior, binary/text stream
  treatment, and positioning support;
- command environment behavior and return-code mapping;
- external routine lookup and variable-pool adapters;
- time, random, queue, trace, initialization, termination, and halt hooks;
- implementation limits.

Minimum limit targets from the public specification source:

| Limit area | Minimum support target |
| --- | --- |
| `NUMERIC DIGITS` | at least `999` |
| exponent digits | enough for at least the largest nine-digit non-exponent number |
| string length | at least the same magnitude as the largest supported non-exponent number |
| literal length | at least `250` characters |
| symbol/name length | at least `250` characters |

Level C may support higher limits. Runtime failures above documented limits
should report the matching message identity rather than silently truncating or
wrapping.

## Configuration And Host Interfaces

The language processor is described as a processor plus a configuration. In
cREXX, keep the split explicit:

- compiler/runtime logic owns syntax, evaluation, variable-pool semantics, and
  condition state;
- a Level C configuration adapter owns host services such as commands, streams,
  external routines, queues, time, random, and default I/O.

### Start And Completion

`API_Start` conceptually starts a program with:

| Field | Level C meaning |
| --- | --- |
| `How` | `COMMAND`, `FUNCTION`, or `SUBROUTINE` invocation mode |
| `Source` | source identifier used in diagnostics and `PARSE SOURCE` |
| `Environment` | initial active command environment and stream settings |
| `Arguments` | argument values plus omitted-argument flags |
| `Streams` | default input, output, and error stream settings |
| `Traps` | caller-provided overrides for config hooks |

Completion response classes:

| Indicator | Meaning |
| --- | --- |
| `N` | completed without a returned expression |
| `R` | completed with a returned value |
| `E` | completed with error/condition information |
| `X` | system resources exhausted, maps to syntax condition `5.1` where implicit |
| `S` | unable to continue, maps to syntax condition `48.1` where implicit |

The exact representation need not expose these names, but the Level B runtime
entry point needs equivalent state.

### Source And Character Services

Source scanning depends on configuration services:

- `Config_SourceChar` yields source characters, end-of-line events, and
  end-of-source events; invalid source encoding maps to message `22.1`.
- `Config_OtherBlankCharacters` supplies blank-equivalent characters beyond
  the space character.
- `Config_Upper` supplies the uppercase mapping for symbols and options.
- `Config_Compare`, `Config_Substr`, `Config_Length`, and `Config_Xrange`
  define character comparison, indexing, length, and range behavior.
- `Config_B2C` and `Config_C2B` convert between binary digits and encoded
  characters for binary/hex strings and conversion BIFs.

Level B `.string` currently assumes valid UTF-8. Do not weaken that invariant
accidentally. Level C byte-text behavior should be a deliberate policy in
`RexxValue`, where binary bytes and valid text flags are already distinct.

### Commands, Routines, Queues, Streams

Host command dispatch is a configuration service, not ordinary function
dispatch:

- `Config_Command(Environment, Command)` runs a command in the selected
  environment and reports normal, error, or failure completion. Level C sets
  `RC`, `.RC`, and `.RS` from the result and raises `ERROR`/`FAILURE` when
  enabled.
- `Config_ExternalRoutine` invokes external functions and subroutines. It must
  preserve omitted-argument flags, active environment, stream settings, traps,
  and variable-pool API access while the call is active.
- `Config_Push`, `Config_Queue`, `Config_Pull`, and `Config_Queued` back the
  external data queue used by `PUSH`, `QUEUE`, `PULL`, `PARSE PULL`, and
  `QUEUED`.
- stream hooks back `CHARIN`, `CHAROUT`, `CHARS`, `LINEIN`, `LINEOUT`,
  `LINES`, `QUALIFY`, `STREAM`, default I/O, and `ADDRESS WITH` redirection.

The stream adapter must cover character input/output, stream positioning,
stream commands, stream state, stream qualification, unique temporary stream
names, stream queries, close, and count/availability queries.

### Traps And API Variable Pools

Every trap corresponds to a configuration hook with the same suffix, for
example `Trap_Command` before `Config_Command`. A trap returning a null string
falls through to the configuration function; a non-null trap result replaces
the configuration result.

Variable-pool API entry points are only valid while the runtime has explicitly
enabled API access around external calls or host hooks. Required behavior:

- `API_Set`, `API_Value`, and `API_Drop` accept ordinary symbols and perform
  compound-tail substitution.
- `API_SetDirect`, `API_ValueDirect`, and `API_DropDirect` operate on direct
  symbols without compound-tail substitution.
- `API_ValueOther` reads processor state such as arguments and source facts.
- `API_Next` and `API_NextVariable` iterate visible pool names and values.

This maps naturally to `RexxVariablePool`, but it requires more than a string
map: bindings need dropped/not-dropped, exposed/not-exposed, and
implicit/not-implicit state.

## Lexical And Parser Rules

Classic REXX has contextual keywords. There are no general reserved words. A
word such as `IF`, `THEN`, `ADDRESS`, or `TRACE` can be a variable name when
the surrounding syntax proves a variable role.

### Scanner Events And Actions

The scanner must recognize these source events:

- end of line and end of source from the source adapter;
- radix suffixes `X`/`x` and `B`/`b` after string literals only when not
  followed by a general letter, digit, or period;
- comma continuation when comma is followed by blanks/comments through end of
  line and not immediately by end of source;
- exponent signs `+` or `-` only in numeric constants after `E`/`e` with
  digits following.

Token actions:

- variable symbols are uppercased by configuration and later become either
  keywords or `VAR_SYMBOL`;
- constant symbols are uppercased and become `NUMBER` if numeric, otherwise
  `CONST_SYMBOL`;
- quoted strings collapse doubled quote delimiters and enforce literal-length
  limits;
- binary and hexadecimal strings validate grouping, pad to whole bytes on the
  left, convert through configuration encoding, and emit `STRING`;
- alternative negator characters normalize to backslash in operators;
- blanks are remembered because they can infer concatenation.

Nested `/* ... */` comments are part of the lexical grammar. Unterminated
comments and quotes report the `6.*` message family.

### Lexical And Top-Syntax Interaction

The Level C parser glue is where the synchronized lexical/top-syntax behavior
belongs:

- inject `VALUE` before `+`, `-`, `\`, or `(` when the current top-level
  context allows an omitted `VALUE`, except immediately after `PARSE`;
- if `=` can be assignment, pass the preceding symbol as `VAR_SYMBOL`;
- if an operand followed by `:` can be a label, pass the operand as `LABEL`;
- promote `WHILE` and `UNTIL` inside `DO` condition expressions;
- promote `TO`, `BY`, and `FOR` inside repetitive `DO` counts;
- promote `WITH` in `PARSE VALUE` and `ADDRESS` contexts;
- promote `THEN` while parsing the expression immediately after `IF` or
  `WHEN`;
- otherwise promote a symbol only when the top syntax accepts that keyword in
  the current context.

Concatenation may be inferred:

- if blanks were recorded before the next operand or left parenthesis, infer
  blank concatenation;
- otherwise infer `||`;
- do not infer an operator for a function call, where `(` follows an operand in
  a function-call context.

Recognized `THEN`, `ELSE`, and `OTHERWISE` synthesize semicolon boundaries.
`THEN` also synthesizes a preceding semicolon. Labels synthesize a following
semicolon.

### Symbols And Function Names

Reserved period-start constant symbols are limited to:

```text
.MN .RESULT .RC .RS .SIGL
```

Any other period-start non-number constant reports `50.1`.

The leftmost component of a function name must not end in a period; report
`51.1` when it does.

## Syntax Surface

Program structure:

- source is a sequence of clauses;
- a null clause may contain labels;
- a clause ends at an explicit semicolon or an inferred semicolon;
- an instruction is a group instruction, a single instruction, assignment, or
  implicit command expression.

Group instructions:

- `DO ... END [name]`
- `IF expression THEN instruction [ELSE instruction]`
- `SELECT ... WHEN expression THEN instruction ... [OTHERWISE ...] END`

Single-instruction families:

| Family | Required coverage |
| --- | --- |
| Environment | `ADDRESS [env-or-value [command]] [WITH ...]` |
| Arguments/parsing | `ARG`, `PULL`, `PARSE` |
| Calls/control transfer | `CALL`, `EXIT`, `INTERPRET`, `ITERATE`, `LEAVE`, `RETURN`, `SIGNAL` |
| Variables | assignment, `DROP`, `PROCEDURE [EXPOSE ...]` |
| Numeric/options/trace | `NUMERIC`, `OPTIONS`, `TRACE` |
| Queue/output/no-op | `PUSH`, `QUEUE`, `SAY`, `NOP` |
| Commands | expression-only clauses dispatched through active `ADDRESS` environment |

### Expressions

Expression precedence, high to low:

1. terms: symbol, string, function call, parenthesized expression;
2. prefix `+`, `-`, `\`;
3. power `**`;
4. multiplication, division, integer division, remainder: `*`, `/`, `%`, `//`;
5. addition and subtraction: `+`, `-`;
6. concatenation: blank and `||`;
7. comparisons, both normal and strict;
8. logical AND: `&`;
9. logical OR and exclusive OR: `|`, `&&`.

The public specification grammar makes power left-associative. Preserve the
Classic behavior even if Level B has configurable power associativity.

Comparisons include normal `=`, `\=`, `<>`, `><`, `>`, `<`, `>=`, `<=`, `\>`,
and `\<`, plus strict `==`, `\==`, `>>`, `<<`, `>>=`, `<<=`, `\>>`, and
`\<<`.

### Parse Templates

`PARSE` must support these source forms:

- `ARG`, `PULL`, `SOURCE`, `LINEIN`, `VERSION`;
- `VALUE [expression] WITH template`;
- `VAR symbol`;
- optional `UPPER`.

Templates support:

- variable targets and `.` placeholders;
- literal string patterns;
- parenthesized variable patterns;
- absolute positions;
- `=` positions;
- relative `+` and `-` positions;
- comma-separated template lists.

`ARG` is equivalent to `PARSE UPPER ARG`. `PULL` is equivalent to
`PARSE UPPER PULL`.

### Static Validation

Validation after parse must enforce syntax-adjacent rules that grammar alone
does not fully express:

- `END name` must match the controlled `DO` variable; `END name` on a
  non-controlled `DO` is invalid.
- `LEAVE` and `ITERATE` must occur within a repetitive `DO`; named forms must
  match an active controlled loop, with nesting correction recorded for
  runtime control flow.
- labels inside active groups can be trace-only and cannot be branch targets or
  internal routine entry points.
- `ELSE` associates with the nearest preceding compatible `IF`.
- clause line numbers are based on source lines before the first token in the
  clause.
- message choice rules must pick the more specific syntax message where the
  grammar source allows alternatives.

## Evaluation Rules

### Variable Pools

Each pool binding has:

- name qualification: non-tailed or tailed;
- dropped or not-dropped;
- exposed or not-exposed;
- implicit or not-implicit;
- an associated value when not dropped.

Non-tailed names include scalar symbols and stems ending in a single period.
Tailed names are compound variable names derived from a stem and tail.

Pool operations:

- `Var_Empty` initializes every possible name as dropped, implicit, and
  not-exposed.
- `Var_Set` stores a value and marks the binding not-dropped and not-implicit.
  Setting a stem also affects existing tailed names below that stem and marks
  them implicit.
- `Var_Value` returns a value or dropped indication. Dropped variables normally
  raise `NOVALUE` when evaluated.
- `Var_Drop` restores dropped state. Dropping a stem drops tailed names below
  the stem.
- `Var_Expose` marks names as exposed to the parent pool.
- `Var_Reset` resets API iteration state.

Exposure is recursive through parent pools. This is the main reason Level C
must not use ordinary Level B locals as the semantic variable store.

### Symbols And Compound Variables

`NUMBER` and non-reserved `CONST_SYMBOL` evaluate to their token spelling.
`VAR_SYMBOL` can be taken as a constant in contexts such as labels, routine
names, and environment names.

Variable evaluation:

- simple symbols and stems use non-tailed pool lookup;
- compound variables derive their tail by evaluating tail components;
- evaluating tail components for compound-name derivation does not raise
  `NOVALUE`;
- after derivation, lookup of the final tailed name can raise `NOVALUE`;
- reserved symbols read from pool `0`.

`SIGL` and `.SIGL` are assigned the line number of the clause that caused a
label search when a matching label is found.

### Numeric And Logical Semantics

Numeric checks use Classic `DATATYPE` behavior:

- numeric strings may include surrounding blanks and an optional sign;
- whole-number checks require numeric form with no non-zero fractional part;
- exponent range failures raise `41.7`.

Arithmetic operands are validated per operator. Numeric overflow, underflow,
divide-by-zero, integer-division notation failures, and lost digits must route
through the message/condition model.

Logical values are exact strings `0` or `1`. `IF`, `WHEN`, `WHILE`, `UNTIL`,
logical operators, and prefix logical not report `34.*` when given any other
value.

Normal comparison compares numerically if both operands are numeric; otherwise
it strips leading/trailing blanks, pads shorter strings with blanks, and uses
configuration comparison. Strict comparison is character-by-character with no
numeric conversion and no blank stripping.

### Function And CALL Evaluation

Arguments preserve omitted positions. The call frame needs both argument value
and argument-exists state.

Resolution order for a routine/function name:

1. matching non-trace-only internal label;
2. built-in function name;
3. external routine adapter.

Internal function invocation must return a value from `RETURN expression`.
Returning without an expression from a function raises `45.1`.

`CALL` discards function syntax and invokes a subroutine. If the routine returns
a value, assign `RESULT` and `.RESULT`; if not, drop them.

## Execution Rules

### Program And Clause Lifecycle

Program initialization sets invocation mode, source, initial environment,
arguments, default numeric settings, configuration constants, message catalog,
condition enablement, trace state, and variable pools.

Clause termination is not just a sequence point. It is where the runtime checks
for `HALT`, handles delayed `CALL ON` conditions, and performs trace/pause
activity.

### Core Instructions

`ADDRESS`:

- bare `ADDRESS` swaps active and alternate environments;
- `ADDRESS env` sets the active environment;
- `ADDRESS env command` or `ADDRESS VALUE expr command` issues one transient
  command;
- `WITH INPUT|OUTPUT|ERROR` supports `NORMAL`, `STREAM name`, and `STEM name`,
  with output/error `APPEND` or `REPLACE`;
- command result updates `RC`, `.RC`, and `.RS`, then raises `ERROR` or
  `FAILURE` if enabled.

`CALL`:

- invokes internal, built-in, or external routines;
- `CALL ON/OFF ERROR|FAILURE|HALT|NOTREADY [NAME target]` controls delayed
  condition handlers.

`DO`:

- simple `DO` groups only sequence instructions;
- repetitive `DO` records loop state for `LEAVE`/`ITERATE`;
- repeat and `FOR` counts must be non-negative whole numbers;
- control variable start, `TO`, and `BY` values must be numeric;
- `WHILE` and `UNTIL` expressions must be exact logical values;
- updating the control variable uses variable-pool semantics and can raise
  `NOVALUE`.

`DROP`:

- direct variable-list words drop the named variable;
- parenthesized entries evaluate to an uppercase word list, and each word must
  be valid variable syntax.

`EXIT`:

- optional expression becomes the program result;
- finalization hook runs before completion;
- falling off the end of the program is equivalent to `EXIT` without an
  expression.

`INTERPRET`:

- evaluates an expression to source text;
- checks `HALT` before recognition;
- parses the value as source text with an implicit final line/end sequence;
- syntax recognition failures raise `SYNTAX`;
- labels in interpreted source raise `47.1`;
- otherwise executes the interpreted instruction list in the current runtime
  context.

`NUMERIC`:

- `DIGITS` defaults to `9`, must be whole, positive, greater than `FUZZ`, and
  within the configured digit limit;
- `FORM` defaults to `SCIENTIFIC`; value forms accept `S` or `E`;
- `FUZZ` defaults to `0`, must be whole, non-negative, and less than `DIGITS`.

`OPTIONS` passes blank-delimited words to the processor. Unknown words are
ignored unless cREXX deliberately documents stricter Level C options.

`PARSE`:

- constructs the parse source from arguments, line input, queue/default input,
  source description, expression value, variable value, or version string;
- applies optional uppercasing before assignment;
- assigns through the variable pool;
- `.` targets discard data.

`PROCEDURE`:

- valid only as the first instruction after internal routine initialization;
- creates a new pool;
- `EXPOSE` aliases listed variables or parenthesized uppercase word-list
  entries to the previous pool.

`PUSH` and `QUEUE` place optional expression values on opposite ends of the
external data queue. Missing expression means the null string.

`RETURN`:

- optional expression becomes the invocation result;
- function invocations require an expression;
- procedure pools are popped before returning to the caller;
- returning from the outermost level performs program finalization.

`SAY` writes the optional expression value, or the null string, to default
output.

`SELECT` evaluates `WHEN` expressions in order. Each must be exact `0` or `1`.
If none are true and there is no `OTHERWISE`, raise `7.3`.

`SIGNAL`:

- `SIGNAL ON/OFF condition [NAME target]` controls condition traps;
- branch form searches labels, rejects missing/trace-only labels, clears loop
  state, and leaves interactive pause state when relevant.

`TRACE` supports option letters `A C E F I L N O R`, leading `?` toggles,
numeric skip/inhibit values, source tracing, result/intermediate tracing,
command tracing, and interactive trace behavior.

## Conditions And Messages

Conditions include at least `SYNTAX`, `HALT`, `ERROR`, `FAILURE`, `NOTREADY`,
`NOVALUE`, and `LOSTDIGITS`.

Raising a `SYNTAX` condition:

- renders the message from the catalog;
- stores the message number in `.MN` in pool `0`;
- outputs source/line prefix and trace/traceback when not trapped;
- terminates if disabled.

Raising non-`SYNTAX` conditions:

- ignored when disabled, except `HALT`;
- `SIGNAL ON` transfers immediately and disables the condition;
- `CALL ON` records a delayed event handled at a clause boundary;
- delayed events are not generally stacked, except one extra `HALT` can be
  held while a first `HALT` is being handled.

The `CONDITION` BIF reads condition name, description, extra data, and
instruction type from this state.

Message identities and normalized catalog text live in
`compiler/docs/levelc_standard_error_messages.md`. BIF-specific message usage
is summarized in `compiler/docs/levelc_classic_bifs.md`.

## Implementation Guidance

1. Keep Level C authored source and lowered Level B work trees separate. DSLSH
   and diagnostics must report against authored Level C source.
2. Treat the Level C scanner/parser adapter as a required semantic component,
   not an incidental convenience. Keyword roles are contextual.
3. Lower all Classic variable access through `RexxVariablePool` helpers until a
   later optimizer proves a narrower representation is valid.
4. Route scalar operators through `RexxValue` helpers that accept an explicit
   numeric context. Avoid one-off numeric parsing in instruction or BIF code.
5. Put host services behind a Level C configuration adapter. Do not call
   ordinary Level B file, process, stream, or environment helpers directly from
   lowered Classic semantics.
6. Keep RexxScript sandbox policy separate. Shared value/pool code is good;
   sharing `ADDRESS`, external calls, unrestricted streams, or ambient host
   access is not.
7. Prefer focused compatibility fixtures for each family: lexical edge cases,
   contextual keyword roles, expression precedence, parse templates, variable
   pools, conditions, `ADDRESS WITH`, and BIF argument validation.
