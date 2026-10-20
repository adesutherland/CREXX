# STEP05: cleaned IR and RXAS before and after optimisation

These are actual Level B/Level L compiler outputs. The C views use the current
borrowed-context ABI; G views retain the existing STEP05 output. The before
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

Earlier execution of the source pairs with `rxas -n` and linked metadata
established `yes`. The current C view refresh adds the required header Name
read; focused OPTIONS runtime checks qualify that new work without replaying
the whole walkthrough. G links `library.rxbin`; C also links
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
retain the entry wrapper and source records. C retains its borrowed context,
stable owners, numeric installation and condition handlers. Its effectful truth
check remains before the proved literal branch; only the unreachable arm and
discard-safe work disappear. No contextual numeric evaluator is assumed.

| Complete module | G before | G after | C before | C after |
|---|---:|---:|---:|---:|
| Executable instructions | 17 | 8 | 183 | 164 |
| Calls | 1 | 1 | 20 | 18 |
| RXAS bytes | 892 | 559 | 7,878 | 7,289 |
| Body registers | 7 | 3 | 28 | 25 |
| Source records | 11 | 7 | 16 | 12 |

[Counts](shapes.json) exclude labels, metadata and external declarations;
wrappers and cold handlers are included. These are code-shape observations,
not execution-speed claims. Folded control retains derived source origins;
remaining expression/error sites retain authored ranges and logical filenames.
