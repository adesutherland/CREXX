# A complete C/G compilation, with failures

These retained walkthrough outputs illustrate the earlier STEP06 construction
and error paths. Their C schema predates the [current contract](../../doc/architecture.md#current-c-family-contract);
G views retain the STEP06 output. The
[current plan](../../PLAN.md) tracks instruction status and remaining work. [Coverage](../../doc/poc-coverage.md) maps the whole
agreed PoC, rather than treating this one example as complete language coverage.

Start each file with `options levelc` or `options levelg`, then:

```rexx
say 01
if 1 then say 'ok'
else say 1 / 0
```

C prints `01` and `ok`. G prints `1` and `ok`. Both are correct: the source
spelling is the same, but C preserves literal bytes while G uses an integer
value and formats it for SAY.

| Language | Source | Actual AST | Frontend IR | Optimised IR | Baseline RXAS | Optimised RXAS |
|---|---|---|---|---|---|---|
| C | [source](walk-c.crexx) | [tree](walk-c.ast) | [before](walk-c.before.ir) | [after](walk-c.after.ir) | [before](walk-c.before.rxas) | [after](walk-c.after.rxas) |
| G | [source](walk-g.crexx) | [tree](walk-g.ast) | [before](walk-g.before.ir) | [after](walk-g.after.ir) | [before](walk-g.before.rxas) | [after](walk-g.after.rxas) |

The C AST retains COptions and its ordinary `levelc` Name expression. Current
C IR uses required context/entry parameters, separate generated binding and
dispatch blocks, and one source-unit body. Checked dispatch remains after the
literal source IF is folded; it has context/condition effects and is not a
pure integer branch fact.

The AST listing is a parents-first view: `kind|start:finish|source text`, with
zero-based byte spans and multiline source text left intact. Both contain
`Number|19:21|01`, an `IfElse` and its two SAY arms. The condition and arms are
owned AST children. The frontend maps those children to IR blocks/operations;
IR successors are references, not extra source children. The division remains
in the frontend IR even though the condition is a literal true value.

The shared optimiser asks the registered language service for the condition's
exact Boolean meaning. It then selects the true successor and removes the
unreachable division block. It does not evaluate `1 / 0` at compile time.
G's resulting IR is:

```text
entry.1:
  %1 = core.constant 1 : crexx.g.text
  crexx.g.say %1
  core.jump -> if.yes.2
if.yes.2:
  %2 = core.constant ok : crexx.g.text
  crexx.g.say %2
  core.jump -> if.join.4
if.join.4:
  %3 = core.constant 0 : crexx.g.int64
  core.return %3
```

This excerpt omits only origin IDs and the function heading; the linked file
retains both. The Level B backend selects RXAS through Level L's `llir.writer`.
It omits the two jumps because their targets immediately follow in emission
order. G's body executes `numsci`, loads/prints `"1"` and `"ok"`, then returns
zero. The complete output retains its entry wrapper and meaningful source
records. C retains Classic activation, pool, numeric, argument and condition
work; inspecting the caller alone does not measure work inside those helpers.

Read the full RXAS, including wrapper and cold handlers, alongside shared helper
bodies. Stable root/context owners, numeric setup and result packets remain
necessary costs. Structural checks require the unreachable source division to
disappear without dropping required dispatch or truthful source records.

## Run with installed tools

Use a complete install produced by `stage-product` followed by `cmake --install`.
Set `tools` to its `bin` directory and run from a directory where the target
runtime packages can be found (the installed `bin` directory is sufficient):

```sh
tools=/path/to/install/bin
examples=/path/to/CREXX/research/level-l/compilers/examples/step06
work=/tmp/crexx-poc-walk
mkdir -p "$work"
cd "$tools"
"$tools/llrxc-g" -i "$tools" -o "$work/walk-g.rxas" "$examples/walk-g.crexx"
"$tools/rxas" -n "$work/walk-g.rxas"
"$tools/rxlink" -o "$work/walk-g-linked" "$work/walk-g.rxbin" "$tools/library.rxbin"
"$tools/rxbvm" "$work/walk-g-linked.rxbin"

"$tools/llrxc-c" -i "$tools" -o "$work/walk-c.rxas" "$examples/walk-c.crexx"
"$tools/rxas" -n "$work/walk-c.rxas"
"$tools/rxlink" -o "$work/walk-c-linked" "$work/walk-c.rxbin" \
  "$tools/library.rxbin" "$tools/llctarget.rxbin" "$tools/rxfnsc.rxbin" \
  "$tools/classlib.rxbin" "$tools/rxunicode-core.rxbin"
"$tools/rxbvm" "$work/walk-c-linked.rxbin"
```

Add `-n` to either `llrxc-*` command for baseline IR; keep `rxas -n` to isolate
the shared compiler passes. `rxtvm` runs the same linked output where available.
Do not pass linker `-s` if you want the retained source locations.

## Error variants

Change `if 1` to `if 0`. Both modes still compile, print the first line, and
then fail at `else say 1 / 0`, line 4, column 10:

- G: `DIVISION_BY_ZERO`, [actual diagnostic](reached-g.txt).
- C: `RXC-LC-42.3`, with authored context and original Classic helper fault,
  [actual diagnostic](reached-c.txt).

The runtime diagnostic also names the assembled module's identity; that is
separate from the logical source filename in `.srcstep` metadata. The refreshed
C diagnostic omits its temporary host-directory prefix.

Remove `then` instead. Both report `LL_COMPILER_SYNTAX` at the unexpected ELSE
on line 4, column 1, with THEN in the expected-token set:
[Classic](syntax-c.txt), [G](syntax-g.txt). The parser can continue interpreting
the preceding tokens as an expression until this detection point. Existing
output remains byte-for-byte unchanged, and no staging directory is left.

## Regenerate and inspect

From the repository root, enable the research build and prepare the small Level B
view consumer. Supply an installed base plus ResearchCompilers `bin` directory
as `crexx_runtime`, so the Classic adapter and target libraries are together:

```sh
cmake -S . -B cmake-build-debug -DCREXX_BUILD_RESEARCH_COMPILERS=ON
cmake --build cmake-build-debug --target llcompiler_backend-poc_opt
crexx_runtime=/path/to/installed/crexx/bin
mkdir -p /tmp/crexx-poc-views
cmake-build-debug/bin/rxbvm \
  cmake-build-debug/research/level-l/compilers/backend/opt/backend-poc-linked.rxbin \
  -a g research/level-l/compilers/examples/step06/walk-g.crexx \
  "$crexx_runtime" /tmp/crexx-poc-views/walk-g
```

Repeat with `c`, `walk-c.crexx`, and a `walk-c` output prefix. The consumer uses
test-only adapters over the independent C and typed/G parser and compiler
services; `graph()` and `emissiongraph()` expose the two IR views, and
`output()` supplies Level L writer output. It writes all five views and checks
the accepted example's invariants. It is a test/example consumer, not a new
compiler command or a general-purpose dump option. Delete `/tmp/crexx-poc-views`
after inspecting the views.

`ctest -R '^llcompiler_poc_'` runs the narrow permanent checks after preparing
their declared producers. Each test covers the walkthrough, its two failure
variants and the paired nested-control/NUMERIC cases, with source trees and
actual emitted execution. The compiler source is never rewritten as native B
source or handed off to the old compiler.
