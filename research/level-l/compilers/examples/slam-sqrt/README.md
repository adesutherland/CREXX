# SLAM square-root comparison

This opt-in experiment compares three Classic Rexx adaptations of published
ooRexx routines: [`FDSqrt` before](https://github.com/animatedads/alchemy/blob/main/packages/oorexx_slam_v0.1/tests/fixtures/fdsqrt_fixed14.cls)
and [after](https://github.com/animatedads/alchemy/blob/main/packages/fd_door_micro_motion_v0.2-dev8/src/FDDoorMicroMotion.cls)
the 14-step defect repair, and Vision's separate
[`rx_vision_sqrt`](https://github.com/animatedads/alchemy/blob/main/packages/oorexx_vision/src/Vision.cls).
It uses the 29 inputs in SLAM's
[`test_fdsqrt.rex`](https://github.com/animatedads/alchemy/blob/main/packages/oorexx_slam_v0.1/tests/test_fdsqrt.rex).
The original door routine was written by ChatGPT; SLAM's architecture is Tom
Dyer's and its code is Claude's (Opus 5.5). The adaptations replace ooRexx
routine/argument syntax with Classic labels, `PROCEDURE` and `PARSE ARG`; the
positive-input calculations are otherwise preserved. Vision's ooRexx negative
input exception is excluded by the driver's explicit nonnegative domain.

From the repository root, with the normal Debug Classic C research products
already prepared:

```sh
python3 research/level-l/compilers/examples/slam-sqrt/compare.py --build cmake-build-debug --check
```

`compare.py` compiles its Level B analyser from the prepared frontend imports
and links it with the existing frontend and Level L modules. It compiles each
Classic routine with the same `NUMERIC DIGITS 9` source wrapper through
`llrxc-c`, `rxas`, `rxlink` and `rxbvm`. All generated files live in a temporary
directory and are removed after the run. The analyser asks the Classic frontend
for an AST and checked IR, then `llirgraph` for CFG cycle facts. The report
links each routine's DO form to its source byte span. The wrapper's input loop
is excluded from the AST list; the IR totals include it.

The Python checker turns decimal input and output strings into exact fractions.
For a nonnegative candidate `r` and input `x`, it checks:

- Relative tolerance: `(r - r·10⁻⁸)² ≤ x ≤ (r + r·10⁻⁸)²`, with a zero lower bound.
- Nearest nine significant digits: `r` lies on the nine-digit decimal grid and
  `x` lies between the squared midpoints to its adjacent grid values. The
  predecessor has a smaller spacing at a power-of-ten boundary.

The inequalities are checked exactly, including failed cases. Midpoint ties
are accepted as within half a unit; the tool does not decide a tie-breaking
convention. Output agreement is listed separately from either certificate.
`--check` asserts the published 29-value contrast, including the three values
where the repaired and Vision routines agree but the nine-digit certificate
fails. A custom nonnegative finite decimal set can be supplied with `--values`
without the published-contrast assertion.

These certificates establish only the stated input/output claims. AST loop
forms and CFG cycles establish structural facts. No all-input termination,
all-input numerical correctness, compiler correctness, or ooRexx equivalence
outside the selected positive-input body is claimed. If `--check` fails, the
printed per-input evidence is the starting point for examining cREXX versus
ooRexx arithmetic or an adaptation error; the expected values are not silently
changed.
