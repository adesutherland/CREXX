# Level L Bootstrap Frontend And cREXX Lexer Proof

This retained experiment implements the first executable Level L lexer-maker
slice entirely outside the production compiler.

The implemented path is:

```text
authored .levell
    -> hand-written Level B scanner and deterministic parser
    -> Level B Token objects and automatic structural AST
    -> validated IDs, references, actions, and lexer invariants
    -> deterministic re2c adapter input
    -> retained re2c 4.5.1 cREXX syntax adapter
    -> generated Level B scanner
    -> rxc -> rxas -> rxlink -> rxtvm and rxbvm
```

The authored language is Level L. re2c is used only to construct and emit the
DFA in this proof; its syntax and embedded-action surface are not exposed to a
Level L author.

## Implemented surface

`levell_bootstrap.crexx` provides Level B `levelltoken`, `levellastnode`, and
`levellresult` classes plus the bootstrap parser and adapter emitter. The
frontend implements the documented first lexer subset:

- `options levell`, `namespace`, Rexx `/* ... */` comments, and static dotted
  declaration names;
- `token`, `diagnostic`, `lexer input`, `charset`, `pattern`, and `rule`
  declarations;
- Unicode scalar literals, inclusive ranges, charset union and subtraction,
  `scalar`, grouping, concatenation, alternation, and repetition;
- `emit`, `skip`, and `error` actions plus `eof`, `malformed`, and `otherwise`
  rule subjects;
- deterministic token-ID assignment, reference/action validation, special-rule
  validation, non-empty ordinary-rule checks, source byte spans, token dumps,
  and structural AST dumps; and
- a first-error panic with no recovery or speculative parser backtracking.

`specs/tinyexpr.levell` is the small differential proof.
`specs/gennorm2.levell` is the real Unicode-language input: it describes the
lexer for ICU's retained `gennorm2` normalization file notation, including
version, hexadecimal, range, canonical-combining-class, mapping, comment, and
line-boundary tokens.

## Reproduce

From the repository root:

```sh
experiments/unicode/level-l-bootstrap/run.sh
```

The default product build is `cmake-build-unicode-debug`. Optional environment
variables are:

- `CREXX_BUILD_DIR`: product build directory;
- `CREXX_LEVEL_L_BOOTSTRAP_BUILD_DIR`: untracked experiment output directory;
- `CREXX_BUILD_JOBS`: parallel build count, default `10`; and
- `RE2C_BIN`: alternate re2c executable, which must report version `040501`.

The run regenerates the Token dump, AST dump, re2c adapter input, and final
cREXX for both specifications. It requires byte identity with the committed
files under `generated/`, compiles and links both generated scanners, and runs
all focused checks on `rxtvm` and `rxbvm`.

The TinyExpr scanner agrees with maintained `rxfnsl` token kinds, names, and
byte spans. The Unicode scanner accepts all 2,500 lines of the retained ICU
78.3 `nfc.txt` and checks exact counts for the 163 ranges, 403 CCC records, 961
two-way mappings, 1,120 one-way mappings, version declaration, line endings,
and EOF. A non-ASCII comment fixture checks UTF-8 matching and byte offsets.

## Deliberate boundaries

- This is an experimental Level B tool, not new syntax in production `rxc` and
  not a released Level L compiler.
- The bootstrap source scanner currently accepts ASCII Level L source. Unicode
  input symbols are fully expressible with `U+` literals; a non-ASCII authored
  literal panics explicitly rather than being misread.
- Generated scanners currently accept `.string`, whose Level B ingress contract
  already guarantees valid UTF-8. The authored `malformed` rule is validated
  but unreachable and omitted by this adapter. A future `.binary` input path
  must add an explicit UTF-8 decoder before claiming malformed-byte handling.
- The PoC emitter walks the validated syntax tree directly and uses re2c as its
  automaton backend. A separately reusable neutral IR and native Level B/Level L
  automaton builder remain later work.
- Tokens use a portable 12-byte proof layout with `<at..u16>`/`<at..u32>`.
  Packed host-native records are not selected by this experiment.
- This lexer recognizes the Unicode rule-file language; it does not yet parse
  its records, compile normalization tables, normalize text, or implement NFC.
- Level L parser-maker syntax, self-lexing/self-hosting, recovery, token
  payloads, streaming, and physical packed AST storage remain later work.
