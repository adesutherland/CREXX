# re2c Level L Emitter Experiment

This experiment tests whether re2c 4.5.1 can emit useful cREXX Level L scanner
code without a re2c source fork. It is an output-adapter proof, not the Level L
authored syntax and not a production lexer generator.

The experiment succeeds for the retained TinyExpr lexer:

- `crexx.syntax` overlays re2c's bundled Python `loop-switch` syntax with cREXX
  assignments, `if` blocks, `do forever`, `iterate`, and selector-form
  `select`;
- `tinyexpr.re` retains the re2c input and the surrounding Level L module;
- `generated/tinyexpr.crexx` is the byte-reproducible generated source;
- `test_tinyexpr.crexx` compares every generated token kind, byte span, numeric
  value, and name with the hand-written `rxfnsl` TinyExpr proof, including an
  invalid-input prefix; and
- `run.sh` compiles with `rxc`, assembles with `rxas`, links a standalone image
  with `rxlink`, and runs the differential test on both `rxtvm` and `rxbvm`.

Run from the repository root:

```sh
experiments/unicode/level-l-emitter/run.sh
```

The script resolves its own location, so an absolute path to `run.sh` also
works from another directory.

The default clean/debug product build is `cmake-build-unicode-debug`. The
script configures it when absent and builds the necessary umbrella targets in
dependency order. The first build is substantial; later runs reuse it.

Optional environment variables are:

- `CREXX_BUILD_DIR`: product build directory;
- `CREXX_LEVEL_L_EMITTER_BUILD_DIR`: untracked experiment output directory;
- `CREXX_BUILD_JOBS`: parallel build count, default `10`; and
- `RE2C_BIN`: alternate re2c executable, which must report version `040501`.

## What the experiment proves

The generated source uses a single re2c `loop-switch` automaton expressed as:

```rexx
state = 0
do forever
  select state
    when 0 then do
      /* generated transition tests */
    end
    when 1 then do
      /* generated action */
    end
  end
end
```

The ordinary compiler output lowers the state selector to `.jtable` plus
`jumpi`. Source-byte reads lower to `bgetu8`, and the shared 16-byte token
record uses `bsetu16`, `bsetu32`, and `bseti64`. No compiler, assembler,
linker, or VM change was required for this scanner.

The generated scanner agrees with the hand-written TinyExpr proof for empty,
whitespace-only, identifier, numeric, punctuation, expression, and invalid
input fixtures on both VM implementations. Fresh generation must match the
committed `.crexx` source byte for byte.

## Boundaries and lessons

The result supports a re2c syntax-file adapter as a useful bootstrap and
differential backend. It does not make re2c the Level L language:

- the retained `.re` file still contains re2c patterns and raw cREXX action
  bodies;
- the overlay inherits unmodified generic templates from re2c's Python syntax,
  so a production dependency would need a complete frozen syntax file;
- vendored re2c 4.5.1 resolves a user syntax file relative to the current
  directory even when given an absolute pathname, so `run.sh` deliberately
  changes to the specification directory and supplies `crexx.syntax` as a
  relative path;
- the proof uses a zero sentinel and a whole-buffer ASCII scanner; embedded NUL,
  native UTF-8 decoding, malformed UTF-8, streaming, tags, captures, start
  conditions, and trailing context are not proved;
- nested character-range tests are deliberately used so grouped switch cases
  do not require a new cREXX `select` surface;
- the token layout matches the existing portable TinyExpr record only; packed
  host-native and hybrid layouts remain a later measured decision; and
- rule parsing, neutral IR, automaton construction, and generated layout remain
  separate Level L components even if re2c is retained as an oracle or optional
  emitter.

The key architectural result is that the current cREXX control-flow and binary
surface can execute a nontrivial generated DFA efficiently. The next generator
work should consume the documented Level L syntax and lower it to a neutral IR;
it should not expand this re2c overlay into the authored language.
