# Level C Lemon Fallback PoC

Status: throwaway reference PoC

This folder tests whether Lemon `%fallback` is enough to implement Classic
REXX contextual keywords.

The PoC intentionally keeps the grammar tiny:

- assignment: `ID = expr`
- `SAY expr`
- `IF expr THEN statement`
- expression: one identifier

It builds two parser variants:

- `fallback_no_then.y`: `%fallback ID IF SAY DO.`
- `fallback_with_then.y`: `%fallback ID IF SAY DO THEN.`

Run:

```sh
compiler/pocs/levelc_lemon_fallback/run.sh
```

Expected findings:

1. A keyword can fall back to `ID` when it is the current lookahead and the
   keyword form has no parse action. Example: `SAY IF ;`.
2. Fallback cannot recover when a keyword was already shifted as a keyword and
   a later token proves that choice wrong. Examples: `IF = A ;` and
   `SAY = A ;`.
3. Putting `THEN` in the fallback set accepts `IF THEN THEN SAY A ;`, treating
   the first `THEN` as the condition variable. That is not aligned with the
   ANSI rule that `THEN` is a keyword wherever a variable symbol would be part
   of the expression immediately after `IF` or `WHEN`.

Conclusion:

- Lemon `%fallback` is useful as a limited helper.
- It cannot be the main Classic REXX keyword strategy.
- Level C still needs a contextual token adapter in parser glue.
- `THEN` should not be in a broad global fallback set. If candidate keyword
  tokens are introduced later, the adapter must never emit a fallback-eligible
  `THEN` while parsing the condition after `IF` or `WHEN`.

Observed result on 2026-05-10:

- Both parser variants built with the repository Lemon binary.
- Both variants accepted `SAY IF ;`, `IF A THEN SAY IF ;`, and `SAY DO ;`.
- Both variants rejected clean acceptance of `IF = A ;` and `SAY = A ;`,
  demonstrating that fallback does not replace assignment-lookahead demotion.
- Only the `THEN` fallback variant accepted `IF THEN THEN SAY A ;`.
  This is the key negative result: broad `THEN` fallback would accept a shape
  the ANSI interaction rule says should be terminated by keyword `THEN`, not
  treated as a variable in the condition expression.
