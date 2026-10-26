# STEP05: cleaned IR and RXAS before and after optimisation

These retained walkthrough outputs illustrate the earlier STEP05 construction
and transformation shapes. Their C schema predates the [current contract](../../doc/levelc-architecture.md#current-c-family-contract);
G views retain the STEP05 output. The before
side disables shared passes and the after side enables them. The
[current plan](../../PLAN.md) tracks remaining compiler work.

Use `options levelc` or `options levelg`, followed by:

```rexx
if 1 then say 'yes'
else say 'no'
```

| Language | Source | Before IR | After IR | Before RXAS | After RXAS |
|---|---|---|---|---|---|
| C | [source](branch-c.crexx) | [IR](branch-c.before.ir) | [IR](branch-c.after.ir) | [RXAS](branch-c.before.rxas) | [RXAS](branch-c.after.rxas) |
| G | [source](branch-g.crexx) | [IR](branch-g.before.ir) | [IR](branch-g.after.ir) | [RXAS](branch-g.before.rxas) | [RXAS](branch-g.after.rxas) |

The source pair must print `yes`. G links `library.rxbin`; C additionally
links `llctarget.rxbin`, `rxfnsc.rxbin`, `classlib.rxbin` and
`rxunicode-core.rxbin`. The commands differ only by the compiler's `-n` switch:

```sh
llrxc-g -n -i /path/to/bin -o before.rxas branch-g.crexx
llrxc-g    -i /path/to/bin -o after.rxas branch-g.crexx
```

After successful request status, inspect both graphs in Level B:

```rexx
sourcegraph = request.graph(); emittedgraph = request.emissiongraph()
say sourcegraph.dump()
say emittedgraph.dump()
```

Both languages prove the condition true and remove the unreachable `no` block.
G's emission graph is:

```text
entry:
  core.jump -> yes
yes:
  %1 = core.constant "yes"
  crexx.g.say %1
  core.jump -> join
join:
  %2 = core.constant 0
  core.return %2
```

The backend emits no jumps to the immediately following block. G's body therefore
contains `numsci`, `load "yes"`, `say`, `load 0`, and `ret`. The complete files also
retain the entry wrapper and source records. C retains its borrowed context,
stable owners, numeric installation and condition handlers. Its effectful truth
check remains before the proved literal branch; only the unreachable arm and
discard-safe work disappear. No contextual numeric evaluator is assumed.

Classic's C v9 checked entry dispatch remains effectful after source branch
folding. Its canonical setup/dispatch blocks preserve the context/entry slot
loads through generic transformation. The source `if 1` branch disappears;
entry-test branches remain. Permanent optimizer checks distinguish these by
the condition producer rather than a total branch count. Source records retain
logical filenames, authored ranges and derived origins for folded control.
