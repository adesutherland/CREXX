# Level L cost and lifetime model

Formal performance testing is not required for this research programme. Correct
results, bounded resources, ownership and emitted-code quality remain acceptance
requirements. This document defines how to interpret the maintained lifecycle
harness; it does not claim a production speed or memory verdict.

## Phase and ownership boundaries

`cases/lifecycle.crexx` separates grammar/table preparation, lexing, parser work,
tree projection, validation and visitor work. Counters from different engines
represent different operations; they are not interchangeable timing units.
Phase medians need not sum to an end-to-end median.

A request measurement must include release of that request's owned context.
Returning a context and timing assignment to the next result instead measures a
transition between candidates, including destruction of the previous result.
The corrected `measuredrequest` returns fixed-size metrics and releases its
context inside the request. Its measurement is named `request_with_release`.
Whole-process peak RSS includes loaded modules, preparation and allocator state;
it is not per-engine live memory or proof of a leak.

Deterministic lifecycle checks cover idempotent replay, independent overlapping
contexts, small requests after large requests, and scope exit. Resource-budget
refusal stays distinct from syntax rejection. Existing depth/state/cell bounds
are specified in `REPORT.md` and `MATH.md`; arbitrary depth is not qualified.

## Reproduction and limits

From the repository root, with current product binaries available:

```sh
CREXX_BIN_DIR="$PWD/cmake-build-debug/bin" research/level-l/run-lifecycle.sh opt
CREXX_BIN_DIR="$PWD/cmake-build-debug/bin" research/level-l/run-lifecycle.sh noopt
```

The runner owns and removes its temporary build/output directory. If a specific
performance question is later approved, choose a minimal isolated comparison,
identify the exact inputs and host load, and summarize the result in the current
plan. Delete raw samples after interpretation. Earlier measurements affected by
concurrent work or the old lifetime boundary do not support rankings. Corrected
isolated timing, attributed memory peaks and production performance remain open;
they do not block this repository reorganisation.
