# Level C Token Adapter Trace PoC

Status: throwaway reference PoC

This PoC explores the Level C token adapter proposed in
`compiler/docs/levelc_working_architecture.md`.

It is deliberately not a production scanner or parser. It uses a small C
scanner and a shallow adapter to make the key decisions visible:

- keyword-spelled assignment left-hand sides are demoted to `TK_VAR_SYMBOL`;
- labels are recognized by lookahead and followed by synthetic `TK_EOC`;
- `THEN` after `IF`/`WHEN` is promoted before expression fallback can happen;
- `THEN`, `OTHERWISE`, and labels demonstrate synthetic end-of-clause tokens;
- `DO`, `PARSE`, and `ADDRESS` subkeywords are promoted only in their local
  contexts;
- keyword spellings in ordinary expression positions stay identifiers;
- inferred concatenation is inserted only before non-keyword operands.

Run:

```sh
compiler/pocs/levelc_token_adapter_trace/run.sh
```

The output prints each fixture in two phases:

1. `scanner`: raw tokens with uppercased spelling and `blank_before`.
2. `adapter`: adapted token, role, and the rule/note that produced it.

The fixture matrix mirrors the working architecture document:

- `if = then`
- `say = if`
- `say then`
- `if a then say b`
- `if then then say b`
- `if a then if b then say c else say d`
- `select; when a then say b; otherwise say c; end`
- `do i = 1 to 10 by 2 while ready; say i; end i`
- `parse value a b with x . y`
- `address value env with output stem out.`
- `label: say label`

Observed result on 2026-05-10:

- The harness builds and runs with `cc -std=c11 -Wall -Wextra -Werror`.
- Keyword-spelled assignment left-hand sides are correctly emitted as
  identifiers in `if = then` and `say = if`.
- `say then` emits `SAY` as a clause-leading keyword and `THEN` as an
  identifier, proving that spelling alone is not enough.
- `if a then say b` emits synthetic `TK_EOC` before and after hard `TK_THEN`.
- `if then then say b` emits the first `THEN` as the condition terminator and
  the second `THEN` as an unexpected structural keyword, preserving the ANSI
  rule found by PoC 1.
- `if a then if b then say c else say d` uses a shallow `pending_else` hint to
  promote `ELSE` and associate it with the nearest pending `IF`. The final
  grammar/walker still needs to own full dangling-`ELSE` validation.
- `SELECT`, `DO`, `PARSE VALUE ... WITH`, `ADDRESS ... WITH`, labels, parse
  targets, parse `.` placeholders, and optional `END` names all receive
  distinct trace roles.
- Inferred blank concatenation is visible in `PARSE VALUE a b WITH ...`, but is
  not inserted before `WITH` once it is promoted to a keyword.

Conclusion:

- A Level C adapter layer is viable and valuable for DSLSH because it can
  assign token roles before Lemon builds the AST.
- The adapter should be shallow, but it needs a small amount of control context:
  assignment/label lookahead, active `IF`/`WHEN` conditions, pending `ELSE`,
  `SELECT` body state, `DO` specification state, `PARSE` mode, `ADDRESS` mode,
  and expression operand state for inferred concatenation.
- The production adapter should remain coupled to Lemon grammar tests; this
  PoC proves the direction, not the full Classic REXX grammar.
