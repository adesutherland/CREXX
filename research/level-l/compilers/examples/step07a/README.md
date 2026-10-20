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

Inspect complete emitted modules, including wrappers/cold handlers, separately
from linked helper bodies. C uses the current v9 context/entry ABI and one
source-unit body; its fresh child activation initially borrows the caller pool,
then PROCEDURE creates the private pool and binds it before ARG executes. The
views enable compiler IR optimisation. Compiler and assembler optimisation are
independent; use `rxas -n` when isolating compiler shape.

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

Each selected ARG comma template has one source acquisition and immutable
packet/executor invocation, preserving original arguments and presence. There
is no scalar ARG reader/store path. Uniform source checkpoints and conservative
register allocation remain admitted costs. C-OPT-01/02/05 own specialisation
under context, ABI and lifetime proof; these views do not establish speed or
minimal frames.
