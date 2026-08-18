# Stage 4 structured-control correctness review

Date: 2026-08-18

Status: **accepted correctness repair; full Debug qualification green**

## Finding

The final cREXX Havlak review exposed an optimized-only wrong result: an
inlined early `RETURN` crossed a counted-loop register cleanup without
executing it. The wider structured-control review then found the same missing
cleanup class on labelled `LEAVE`/`ITERATE`, runtime `SIGNAL` branches and
nested signal-handler unwinding. A separate allocation overlap allowed a
block-expression result to reuse a linked loop register that its newly emitted
cleanup then restored.

These are compiler/runtime correctness defects, not benchmark exceptions.
They were repaired at their supported optimized shapes. No inlining form was
disabled to hide the problem.

## Repair boundary

- Structured exits reproduce cleanup for every crossed counted loop and
  cleanup-owning lexical scope before branching.
- A block-expression result is reserved before a crossed cleanup only when
  that cleanup can mutate or release storage. Metadata-only scalar scopes do
  not acquire an unnecessary extra register.
- Runtime signal-handler entry restores descendant register links, reference
  lifetimes and metadata before executing the handler.
- Signal branch entries retain their handler-stack boundary, and a branch
  unwinds nested registrations installed after that boundary.
- All names installed by one signal block share one stack boundary.

## Regression coverage

Focused optimized/unoptimized tests cover:

- inlined early return across a linked loop bound;
- labelled `LEAVE` and `ITERATE` across an inner counted loop;
- runtime signal across linked loop and scope state;
- nested signal-handler unwinding;
- block-expression result/register separation;
- the existing nested-scope and Classic `VALUE` surfaces.

The complete signal-labelled Debug set passed 68/68. After one broad run
identified three intentional optimized golden changes, the register proof was
narrowed to runtime-affecting cleanup and those goldens were updated to require
the new metadata clear. The final ordinary Debug run passed 2,262/2,262:

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```

## First profiling-off Release verdict

The ordinary Release build had `CMAKE_BUILD_TYPE=Release` and
`CREXX_VM_PROFILING=OFF`. Focused correctness passed 16/16 before the final
proof narrowing; the final Release qualification is part of the Stage 4 freeze.
Both concrete VMs returned the required Havlak result of 1,605 loops. The
confirmation timings remained in the already accepted shape:

| VM | optimized | no-opt |
| --- | ---: | ---: |
| `rxtvm` | 3.58 s | 3.13 s |
| `rxbvm` | 3.61 s | 3.13 s |

The optimized/no-opt inversion is recorded but deferred; it is not a
correctness exception and was not used to reject the repair.

## Related future optimisation

RXAS signal policy is a fact about what happens when a signal is raised, not
proof that an instruction cannot signal or partially affect state. Roadmap
item `PERF3-11-R1` therefore permits only profile-led, bounded signal-policy
region proof. It explicitly rejects blanket compiler `SIGIGNORE` emission.
