# STEP07A scalar call and module examples

These C/G sources exercise the [implemented call contract](../../doc/STEP07A-CONTRACTS.md).
The B files remain independent native-seed controls, not a new B frontend.

| Files | Expected behaviour |
|---|---|
| `scale-c`, `scale-g`, `scale-b` | Print `14`, then `21`; G/B use a declared default, C uses ARG presence and an explicit assignment. |
| `presence-c`, `presence-g`, `presence-b` | Omitted is false; explicit empty and nonempty are true. Assigning the formal does not change presence. C ARG uppercases the supplied `given` value to `GIVEN`. |
| `scalelib` and `import-g` | Separately compile the exported G routine and its caller; print `14`, then `21`. |
| `missing-g` | Refuse the missing required first argument at line 2, column 10. |

The `.expected` files contain the complete expected output. The [implemented contract and qualification boundary](../../doc/STEP07A-CONTRACTS.md#acceptance-and-next-implementation-checks)
distinguish the native controls from new compiler runs. `scale-c.ir`, `scale-g.ir` and matching selected
IR/RXAS files show the actual service output. The G fallback load is guarded by
presence; Classic retains its explicit ARG/IF semantics and runtime helpers.

To compile the provider and consumer with installed commands:

```sh
# BIN is the installed cREXX bin directory; WORK is a writable directory.
llrxc-g -i "$BIN" -o "$WORK/scalelib.rxas" scalelib.crexx
rxas -n "$WORK/scalelib.rxas"
llrxc-g -i "$WORK" -o "$WORK/import-g.rxas" import-g.crexx
rxas -n "$WORK/import-g.rxas"
rxlink -o "$WORK/demo" "$WORK/import-g.rxbin" "$WORK/scalelib.rxbin" "$BIN/library.rxbin"
rxbvm "$WORK/demo.rxbin"
```

Run from this example directory, or supply full source paths. The output is
`14`, then `21`. Change the provider's `factor = 2` to `factor = 4`, rebuild only
the provider, and relink the unchanged consumer: the output becomes `28`, `21`.
The current `-i` is one explicit directory; a source using additional runtime
imports must have their images available there too. No source import, hidden
compiler invocation or library initialiser is implied.

## Emission review

Counts cover the complete emitted modules, including wrappers and cold handlers;
helper/library bodies are outside these text counts. New output uses compiler IR
optimisation on; the native design controls used `rxc -n` / `rxc-c -n`. All were
previously assembled with `rxas -n` for semantic attribution. The current C
views include the ordinary header Name read; focused OPTIONS runtime checks
qualify that work. The current C ARG refresh uses shared source/template
execution and the existing nine-name PARSE fatal bridge. The refreshed C view
was assembled with `rxas -n`, linked explicitly and executed for `14`, `21`;
other retained languages keep their earlier proof. Counts do not establish speed.

| Module | Instructions | Calls (all call variants) | Frame registers | Source records | TRACE events |
|---|---:|---:|---|---:|---:|
| new-c | 935 | 88 | 16, 42, 49 | 42 | 0 |
| new-g | 47 | 3 | 2, 16, 8 | 18 | 0 |
| native-b | 21 | 3 | 4, 3 | 7 | 11 |
| native-g | 21 | 3 | 4, 3 | 7 | 11 |
| native-c | 643 | 113 | 11, 79 | 242 | 16 |

The new G caller passes absence directly and the callee guards its literal load;
no caller default or empty placeholder is built. Required-only signatures avoid
this selection. Per-value registers and residual private-slot stores/copies are
the retained allocator/slot-cleanup limitation from STEP05/06, owned by the later
liveness and packing work. They are visible in the example, not a claim that
this is optimal code. Native B/G use specialised call forms and smaller frames.
Classic's current output retains stable activation/pool/bundle owners, a shared
root configuration, receiver/reference windows, executing-frame numeric setup,
argument/result packets and fatal error relay. Temporary receiver links and
argument windows require explicit cleanup. These obligations and conservative
root setup must be distinguished from later specialisation; they must not
be removed without an ABI/lifetime proof. Further Classic specialisation remains
open. Source records retain distinct expression origins; they are not executed
TRACE events. Full TRACE fidelity still has its separate gate.

The earlier C view had four fatal names per procedure; moving this maintained
view to the existing nine-name profile contributes 70 of its 241 additional
instructions. The remaining 171 instructions and seven extra calls per ARG
belong to conservative shared acquisition/checkpoint/executor machinery,
including validation/UPPER/cursor work inside helpers. There
is one acquisition/immutable packet/executor per selected comma template and no
scalar ARG reader/store path. Uniform source checkpoints and conservative
registers remain admitted costs in the [current plan](../../PLAN.md#current-action-arg).
C-OPT-01/02/05 own later specialization under the existing context, ABI and
lifetime proof requirements; no minimal-frame or performance claim follows.
