# RexxScript Concept

Status: historical draft proposal for discussion. This document records design
thinking from the `EVALUATE` prototype period and may intentionally disagree
with the current implementation. The RexxScript product master documentation is
in `rexxscript/doc/`; use that directory for current user-facing and
developer-facing behavior.

## Starting Point

Peter's current `EVALUATE` guide shows a useful small interpreter:

- assignment
- `SAY` output
- single-statement `IF`
- labels and `GOTO`
- `RETURN`
- integer/string expressions
- local, case-insensitive variables
- result export through a string array

That is a good proof of direction, but labels and `GOTO` mean it has already
become a small programming language. The next step should be to name that
language, put guardrails around it, and make sure it can serve both of the
intended host contexts:

1. a safe target for a future Rexx `INTERPRET`-style instruction, especially
   for business rules and similar late-bound snippets;
2. a macro scripting language for `rxpp`, able to read preprocessor metadata
   and emit generated source lines.

The working name in this document is **RexxScript**.

## Current Prototype Harness

The current repository playground keeps Peter's evaluator mostly intact, but it
now lives in the first-class RexxScript runtime product under `rexxscript/`.
That product builds `bin/rexxscript.rxbin` and exposes the `rexxscript`
namespace. `debug` defaults to `0` for library use.
`rexxscript..rexxscript_output()` returns the captured `SAY` output from the
last evaluation, and `rexxscript..rexxscript_value(name)` reads a variable from
the last RexxScript environment.
`rexxscript..rexxscript_evaluate_exposed(script, names, values, debug)` seeds
simple string variables before evaluation.

The original `rxfnsb..evaluate(script, debug)` facade is retained as a
compatibility module in `bin/rexxscript.rxbin`; it is no longer part of the
base Level B `library.rxbin`.

A non-certified compiler exit provides an experimental instruction syntax:

```rexx
a = "1"
status = ""
REXXSCRIPT "a = a + 4; status = 'done'" EXPOSE a, status OUTPUT out RESULT raw
```

With no `EXPOSE` clause, the exit lowers to
`rexxscript_evaluate(<script>, 0)`. With `EXPOSE`, it lowers to
`rexxscript_evaluate_exposed(...)` and copies each exposed variable back from
`rexxscript_value(name)` after evaluation. `OUTPUT` receives captured `SAY`
lines as a `.string[]`; `RESULT` receives the raw evaluator result array.
For now, `EXPOSE` is a simple read/write string bridge. Tagged policies such as
`in`, `out`, or `required` remain future design space.

This is only a harness for exploring the implementation. It is not the final
RexxScript language design, and it does not add `ADDRESS`, command execution,
or host dispatch to RexxScript.

## Recommendation

Build one shared RexxScript core with two host adapters, not two separate
languages.

The core should own parsing, expression evaluation, control flow, variable pool
rules, diagnostics, and resource limits. The host adapter should provide only:

- initial variables and their read/write permissions;
- output routing for `SAY`;
- script options such as maximum steps and maximum output;
- final variable write-back.

This lets runtime `INTERPRET` and preprocessor scripting share one security
model while still behaving naturally in their own contexts. In runtime use,
`SAY` returns captured output to the caller. In `rxpp`, `SAY` appends generated
source lines to the preprocessor output stream.

The two use cases become too different only if preprocessor scripts are allowed
to read files, inspect arbitrary environment variables, invoke macros by name,
or mutate compiler/preprocessor internals directly. Those should remain host
features exposed deliberately as input variables or future allow-listed
intrinsics, not ambient RexxScript powers.

## Design Principles

RexxScript should be classic Rexx shaped, not Level B shaped and not object
oriented.

Script authors should see scalar variables, clauses, `IF`, `DO`, `SAY`, and
Rexx-like expressions. They should not see classes, methods, factories,
namespaces, imports, object casts, or typed declarations. The implementation
may be written in Level B and may use Level B structures internally, but the
script language itself should feel closer to Level C/classic Rexx.

Keywords and variable names should be case-insensitive, following classic Rexx
style. String literals should support normal single-quoted and double-quoted
forms, with quote escaping by doubling the quote character inside the literal.

Keep RexxScript deliberately smaller than Rexx:

- no `ADDRESS`;
- no host command execution;
- classic Rexx implicit command clauses are repurposed as implicit `SAY`
  output, not `ADDRESS` dispatch;
- no `CALL`;
- no `PROCEDURE`;
- no `INTERPRET` inside RexxScript;
- no imports or external programs;
- no arbitrary functions in the MVP;
- no object syntax;
- no file, network, process, shell, clock, random, or environment access unless
  a later host explicitly exposes a safe intrinsic.

The language should fail closed. Unknown syntax, unknown variables, writes to
read-only variables, excessive output, excessive steps, and unsupported
features should produce structured errors rather than best-effort behaviour.

## MVP Language Surface

The MVP should support:

- clauses separated by newline or semicolon;
- `/* ... */` comments;
- scalar assignment;
- `SAY expression`;
- implicit `SAY` expression clauses;
- `IF expression THEN instruction [ELSE instruction]`;
- `DO ... END` groups;
- counted `DO name = start TO stop [BY step] [FOR count]`;
- `DO WHILE expression`;
- `DO UNTIL expression`;
- `LEAVE`;
- `ITERATE`;
- `RETURN [expression]`;
- `NOP`;
- Rexx-like expressions over string values.

Clause syntax sketch:

```text
program     ::= clause*
clause      ::= instruction [;]
instruction ::= assignment
              | say
              | implicit_say
              | if
              | do
              | leave
              | iterate
              | return
              | nop
assignment  ::= symbol "=" expression
say         ::= "say" expression
implicit_say ::= expression
if          ::= "if" expression "then" instruction ["else" instruction]
do          ::= "do" [do_control] clause* "end"
do_control  ::= symbol "=" expression ["to" expression] ["by" expression] ["for" expression]
              | "while" expression
              | "until" expression
leave       ::= "leave"
iterate     ::= "iterate"
return      ::= "return" [expression]
nop         ::= "nop"
```

Implicit `SAY` is RexxScript's safe replacement for classic Rexx's implicit
command dispatch. If a clause is a valid expression and is not an assignment or
control instruction, it is evaluated and sent to the same output path as
`SAY expression`.

Examples:

```rexx
'Hello'
customer_name
'Processing ' || customer_name
```

Those clauses are equivalent to:

```rexx
say 'Hello'
say customer_name
say 'Processing ' || customer_name
```

Ambiguity rules must be simple:

- assignment wins over implicit `SAY`;
- reserved instruction keywords are parsed as instructions;
- malformed clauses are errors, not implicit output;
- implicit `SAY` does not execute commands and never reaches `ADDRESS`.

The MVP should not support labels, `GOTO`, or arbitrary `SIGNAL` branching.
Peter's looping example should become structured RexxScript:

```rexx
customer = 'Mary'
status = ''

do count = 1 to 5
  say 'Processing customer ' || customer || ' record ' || count
end

status = 'COMPLETE'
```

If a future phase adds classic Rexx `SIGNAL label`, it should be treated as a
separate language decision because it is essentially controlled `GOTO`. For a
sandbox, structured loops are easier to reason about, limit, test, and explain.

## Expression Rules

All user-visible variables are strings.

Arithmetic may interpret strings as numbers for the duration of an expression,
but assignment stores the result back as a string. For example:

```rexx
a = '10'
b = 20
total = a + b        /* total becomes '30' */
message = 'Total: ' || total
```

RexxScript should have its own fixed numeric rules. It should not inherit the
caller program's Level B `OPTIONS` or procedure `NUMERIC` settings, because the
same script must behave the same way in runtime and preprocessor hosts.
Numeric precision, division behavior, and result canonicalization need explicit
tests before the language is treated as stable.

The expression evaluator should not use Peter's current left-to-right
evaluation rule. It should have documented Rexx-like precedence for the
supported operator subset. At minimum:

- parenthesized expressions;
- prefix `+` and `-`;
- `*`, `/`, `%`, `//`;
- `+`, `-`;
- `||`;
- comparisons;
- boolean `&`, `|`, and prefix `\` only if the MVP needs them.

Comparisons should be deterministic and documented. A practical MVP rule is:

- if both operands are valid RexxScript numbers, compare numerically;
- otherwise compare as strings using a fixed case-sensitive rule;
- return `'1'` for true and `'0'` for false.

Conditions in `IF`, `DO WHILE`, and `DO UNTIL` should require a logical value
of `'0'` or `'1'`. This keeps accidental strings such as `if status then ...`
from silently meaning something different from author intent.

Open decision: whether to support classic padded string comparison exactly, or
to use simpler exact string comparison for MVP and document the difference.
Open decision: the initial `NUMERIC DIGITS` equivalent and division semantics.

## Variables And Pools

RexxScript should run against an explicit variable pool.

Variable names are case-insensitive and scalar-only in the MVP. Compound
variables, stems, and arrays should be deferred until there is a specific use
case and a clear export format.

Each host-provided variable should have a policy:

| Policy | Meaning |
| --- | --- |
| `in` | readable by the script, not writable |
| `out` | writable by the script, reading before assignment is an error |
| `inout` | readable and writable |
| `local` | script-created variable, not written back unless requested for diagnostics |
| `meta` | read-only host metadata, same enforcement as `in` but reported separately |

Unknown variables should be an error when read. Assignment to an unknown name may
create a local variable, subject to a maximum variable count. A host may choose
to disallow unknown assignments and require all writable names to be declared in
the pool.

For business rules, a useful host mode is "required outputs": if a variable is
declared `out` and marked required, successful script completion must assign it.
That catches misspellings such as `sttaus = 'APPROVED'` without making the core
language abandon classic Rexx's convenient local assignment style.

This is the main guardrail for both use cases:

- runtime `INTERPRET` can expose only the business-rule variables it wants to
  share;
- `rxpp` can expose only selected metadata and selected environment variables,
  avoiding accidental secret leakage.

## ADDRESS-Like Host Interface

The current cREXX `ADDRESS` model has the right shape: an environment receives
command text plus bindings, then returns output and binding updates. RexxScript
should copy that binding discipline without becoming an `ADDRESS` environment
and without allowing host commands.

Conceptually, the host call should contain:

- source text;
- a binding list with name, value, and policy;
- a sandbox/options record;
- output capture settings.

The response should contain:

- status: `OK` or `ERROR`;
- return value, if `RETURN expression` was used;
- updated bindings;
- output lines from `SAY`;
- diagnostics with code, message, line, column, and near-source text;
- resource counters such as steps and output lines.

For a Level C/classic-feeling API, a tagged string-array or stem result is
acceptable, but it must be self-describing. Peter's current result layout can
become ambiguous because exported variables and output are only positionally
separated. A safer array shape is:

```text
result.0 = item-count
result.1 = RS_STATUS
result.2 = OK
result.3 = RS_VARS
result.4 = 2
result.5 = status
result.6 = COMPLETE
result.7 = count
result.8 = 5
result.9 = RS_OUTPUT
result.10 = 1
result.11 = Processing complete
```

The implementation can later provide Level B convenience wrappers for host
code, but the portable contract should remain simple strings and string
arrays/stems. Those wrappers must not leak into the RexxScript language.

## Future `INTERPRET` Surface

The first implementation should be a library component, not a new compiler
instruction. That lets Peter prove the engine, tests, and security model before
we commit to source syntax.

Possible future instruction shape, deliberately not final:

```rexx
interpret rexxscript rules expose customer, amount, status output messages
```

or, closer to `ADDRESS`:

```rexx
interpret rexxscript rules output messages expose customer amount status
```

The compiler could lower such an instruction to the same library/runtime helper
that the preprocessor uses. The important point is not the exact syntax yet; it
is that `EXPOSE`, output capture, sandbox options, and write-back all use the
same binding model as `ADDRESS`.

Example runtime business rule:

```rexx
/* Host exposes:
   customer_name in
   order_total   in
   status        out
   reason        out
*/

if order_total >= 1000 then do
  status = 'REVIEW'
  reason = 'Large order'
  say 'Manual review required for ' || customer_name
end
else do
  status = 'APPROVED'
  reason = ''
end
```

Host result:

```text
STATUS = OK
status = REVIEW
reason = Large order
output.1 = Manual review required for Mary
```

## Preprocessor Surface

In `rxpp`, RexxScript should be a source-generation helper, not a replacement
for the preprocessor.

Possible directive shape, deliberately not final:

```rexx
##SCRIPT REXXSCRIPT
  if build_mode = 'DEBUG' then do
    'trace all'
  end

  'call register_module(' || "'" || module_name || "'" || ')'
##END
```

In the preprocessor host:

- explicit `SAY` and implicit `SAY` expression clauses append generated source
  lines;
- metadata such as source filename, current line, selected target, module name,
  and configured flags arrives as read-only `meta` variables;
- environment variables are not ambient; `rxpp` may expose an allow-listed set;
- diagnostics should point back to both the script block location and the
  generated line where possible.

Generated output must enter the `rxpp` pipeline at a defined stage. By default,
RexxScript output should be treated as already-generated Rexx source and should
not be reprocessed as `##` directives. Otherwise a sandboxed script could emit
`##INCLUDE`, `##USE`, or a macro directive and escape the intended capability
model through the preprocessor itself. A future opt-in mode may allow directive
generation, but it should be a separate host decision.

Source generation has one extra risk: output can be syntactically unsafe even
when the script itself is sandboxed. A future pure intrinsic such as
`quote(value)` or a dedicated `SAY QUOTED` form may be needed before
RexxScript is used for nontrivial generated Rexx source. That is not arbitrary
host function exposure; it is a built-in escaping primitive. Until such a
primitive exists, preprocessor RexxScript should emit fixed source text or
values already sanitized by the host.

## Security Guardrails

The MVP engine should enforce:

- maximum source bytes;
- maximum clauses;
- maximum parse depth;
- maximum loop nesting;
- maximum execution steps;
- maximum loop iterations per loop and per script;
- maximum output lines and output bytes;
- maximum variables;
- maximum variable name length;
- maximum string length;
- maximum diagnostic text length.

The MVP engine should reject:

- `ADDRESS`;
- host command execution;
- `CALL`;
- `PROCEDURE`;
- external functions;
- `INTERPRET`;
- labels and `GOTO`;
- arbitrary `SIGNAL` branch;
- file/network/process access;
- direct environment access;
- binary data;
- object syntax;
- imports;
- dynamic variable name lookup;
- preprocessor directive re-entry from generated output.

Errors should not reveal host variables that were not exposed. Diagnostics
should name the offending RexxScript source span and error code, but not dump
the full variable pool.

## Implementation Shape

The implementation should be a simple interpreter written in Level B if
practical. RXAS should be considered only for a measured hot loop after the
semantics and tests are stable.

Suggested stages:

1. tokenizer with line/column tracking;
2. parser into a small internal instruction/expression representation;
3. validation pass that rejects unsupported features and enforces static limits;
4. optional lowering of structured loops to internal jump offsets;
5. execution loop with step counting and output limits;
6. variable-pool import/export;
7. structured diagnostics;
8. adapter for runtime use;
9. adapter for `rxpp`.

This is intentionally not "compile RexxScript through `rxc` and run it in
`rxvm`". The whole point is to avoid giving untrusted snippets the full Rexx
runtime, import system, command model, and function surface.

Early acceptance tests should include:

- `2 + 3 * 4` and `(2 + 3) * 4` to lock precedence;
- an implicit `SAY` expression appends output through the same path as explicit
  `SAY`;
- reading an unknown variable fails;
- writing a read-only variable fails;
- a required output variable not assigned fails;
- `GOTO`, `CALL`, and `ADDRESS` fail as executable RexxScript instructions;
- step limits stop accidental infinite loops;
- output limits stop runaway `SAY`;
- preprocessor output containing `##INCLUDE` is emitted or rejected according to
  the adapter mode, but is not silently reprocessed.

## Roadmap

MVP:

- assignment;
- `SAY`;
- implicit `SAY`;
- `IF`/`THEN`/`ELSE`;
- `DO`/`END` grouping;
- counted and conditional `DO`;
- `LEAVE`, `ITERATE`, `RETURN`, `NOP`;
- scalar string variables;
- numeric/string expressions;
- explicit variable pool with policies;
- captured output;
- structured error result;
- resource limits;
- runtime and preprocessor host adapters.

Near future:

- `SELECT`/`WHEN`/`OTHERWISE`;
- pure string intrinsics needed for business rules and safe source generation;
- cached parsed scripts;
- better source-map support for preprocessor output;
- optional strict declaration mode requiring all writable variables to be in
  the host pool;
- richer comparison compatibility tests against Level C/classic Rexx.

Later, only with explicit design approval:

- allow-listed host functions;
- compound variables or stems;
- arrays;
- `PARSE VAR` subset;
- host-provided read-only libraries of constants;
- internal RXAS acceleration;
- classic `SIGNAL label` compatibility, if the sandbox costs are accepted.

Out of scope for the foreseeable future:

- object orientation;
- arbitrary procedures;
- arbitrary function packages;
- `ADDRESS`;
- shell commands;
- file I/O;
- network I/O;
- recursive script loading;
- full Rexx compatibility.

## Review Of Peter's Current Prototype

Good foundations:

- small entry point;
- multiple statements in one string;
- assignments and expression evaluation;
- `SAY` captured as output rather than host command execution;
- a natural place to add implicit `SAY` as the safe command-clause replacement;
- local case-insensitive variables;
- result export;
- explicit statement subset.

Changes needed before committing as a direction:

- replace labels/`GOTO` examples with structured `DO` loops;
- define implicit `SAY` expression clauses and add tests for ambiguity rules;
- document operator precedence and add tests for it;
- add `ELSE`;
- add execution and output limits;
- add line/column diagnostics;
- distinguish input variables, output variables, locals, and metadata;
- make result format tagged/self-describing;
- decide how unknown variable reads and writes behave;
- decide whether preprocessor emission needs a built-in quote/escape primitive;
- define where generated preprocessor output enters the `rxpp` pipeline;
- keep the name away from "Mini INTERPRET" if it is now a small language.

## Reread From Different Perspectives

Security perspective:

The strongest part of the design is that there is no ambient authority. The
weakest parts would be loops, generated source text, and future function
exposure. The document now requires resource limits, no host access, no
external functions in MVP, explicit variable policies, and a note that source
quoting is a separate safety problem.

Preprocessor perspective:

The preprocessor needs generated lines, metadata, and readable diagnostics more
than it needs full language power. The design keeps explicit and implicit
`SAY` as the output path, makes metadata read-only, and avoids direct
environment access. The open issue is safe quoting for generated Rexx source.
The reread also adds a hard rule that generated output must not silently
re-enter `##` directive processing.

Runtime `INTERPRET` perspective:

Business-rule callers need controlled input and write-back. The design mirrors
`ADDRESS` binding discipline but removes command execution. It supports output
capture, including implicit `SAY`, and gives callers a stable way to collect
changed variables without letting the script see the whole caller scope.

Developer perspective:

The syntax stays familiar: assignment, `IF`, `DO`, explicit or implicit `SAY`,
strings, numbers, and semicolon-separated snippets. The design deliberately
avoids Level B types and object syntax because the target author is closer to a
classic Rexx user or rules author than a cREXX systems programmer.

Language-engineering perspective:

The risky prototype choices are arbitrary branches and left-to-right
expressions. The design moves control flow to structured loops and requires
documented precedence. It also separates source syntax from internal execution:
the interpreter may lower loops to jump offsets internally without exposing
`GOTO` as language syntax.

Implementation perspective:

The component can start as a Level B interpreter over string arrays and simple
records. There is no need for RXAS until profiling proves the execution loop is
material. The most valuable early tests are parser diagnostics, expression
precedence, variable policy enforcement, output limits, and loop limits.

## Open Decisions

1. Confirm the name `RexxScript`.
2. Decide whether unknown assignments create locals by default or require prior
   host declaration.
3. Decide whether string comparison should be exact in MVP or classic padded.
4. Decide numeric precision, division behavior, and canonical numeric string
   formatting.
5. Confirm that implicit `SAY` is expression-only in MVP, and decide whether a
   later preprocessor template mode should support raw literal output clauses.
6. Decide whether a tiny set of pure built-in intrinsics belongs in MVP.
7. Decide whether `SELECT` is MVP or near-future.
8. Decide whether classic `SIGNAL label` should ever be allowed, knowing it has
   the same control-flow risk as `GOTO`.
9. Decide whether generated preprocessor output is always post-directive source
   in MVP.
10. Decide the first public API shape: tagged array/stem only, or tagged
   array/stem plus a host-side Level B convenience wrapper.
