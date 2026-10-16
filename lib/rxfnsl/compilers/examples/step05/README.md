# STEP05: cleaned IR and RXAS before and after optimisation

These actual outputs were generated on 15 October 2026 from the Level B-only
STEP05 implementation over develop `afcde7f6c2b2dcce6a8a6e89705457aafcac8344`.
The before side includes the construction/emitter cleanup and disables the
shared passes. The after side enables them. This supersedes the original
proposal-only examples; the [review](../../doc/STEP05-REVIEW.md) retains the
historical design sketches.

Use `options levelc` or `options levelg`, followed by:

```rexx
if 1 then say 'yes'
else say 'no'
```

| Language | Source | Before IR | After IR | Before RXAS | After RXAS |
|---|---|---|---|---|---|
| C | [source](branch-c.crexx) | [IR](branch-c.before.ir) | [IR](branch-c.after.ir) | [RXAS](branch-c.before.rxas) | [RXAS](branch-c.after.rxas) |
| G | [source](branch-g.crexx) | [IR](branch-g.before.ir) | [IR](branch-g.after.ir) | [RXAS](branch-g.before.rxas) | [RXAS](branch-g.after.rxas) |

All four outputs assembled with `rxas -n`, linked with source metadata and
executed successfully, printing `yes`. G links `library.rxbin`; C also links
`llctarget.rxbin` and `rxfnsc.rxbin`. The C runtime's normal class library remains
available. The commands differ only by the compiler's `-n` switch:

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
retain the entry wrapper and source records. C retains its activation, pool,
argument/numeric packets and condition handler. Only the exact literal truth
and the unreachable arm disappear; no Classic numeric semantics are guessed.

| Complete module | G before | G after | C before | C after |
|---|---:|---:|---:|---:|
| Executable instructions | 17 | 8 | 111 | 83 |
| Calls | 1 | 1 | 13 | 9 |
| RXAS bytes | 892 | 559 | 4,337 | 3,474 |
| Body registers | 7 | 3 | 17 | 14 |
| Source records | 11 | 7 | 13 | 9 |

[Counts](shapes.json) exclude labels, metadata and external declarations;
wrappers and cold handlers are included. These are code-shape observations,
not execution-speed claims. Folded control retains derived source origins;
remaining expression/error sites retain authored ranges and logical filenames.
