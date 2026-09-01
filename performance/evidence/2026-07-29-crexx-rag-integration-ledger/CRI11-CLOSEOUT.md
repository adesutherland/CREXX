# CRI-11 closeout

Status: **documented/package-closed**

Date: 2026-07-30

## Reproducer and diagnosis

The harmless external fixture and minimized Level B importer are retained in
`/tmp/crexx-cri11-repro.LPn6vO/`:

| Artifact | SHA-256 |
| --- | --- |
| `argv_echo.c` | `1772651e4fe00c5ee349f22a45c373bf515300612494530bb6c65a5f04b73d95` |
| corrected `cri11_address_argv.crexx` | `19930cfae08c7651d1f273f2eaa4da10f9033e72b248fbcf415d1440429c2376` |
| `argv_echo` executable | `ee690ba71d3a8a6e469b6619baa7d4ac0a03da35231fa40c8db1b85a0eb3cb40` |

The first minimized source named its capture variable `output`. The compiler
correctly diagnosed `#ADDRESS OUTPUT clause repeated`, because `output` is the
clause keyword in that position. That failed investigation log is retained as
`/tmp/crexx-cri11-repro.LPn6vO/commands-and-results.log`, SHA-256
`451e748363ef4e55a6162af7a4d2889cb71e409493064c908c3440a8c9e832a7`.
Renaming the variable to `captured` produced the intended reproducer.

The supported argv-preserving path already exists:

```rexx
address CREXX "run :argv[]" output captured error errors
```

`CREXX run` consumes the exposed `.string[]` as an argument vector and launches
the executable directly. It does not serialize the array into a shell command.
By contrast, `ADDRESS COMMAND`, `ADDRESS SYSTEM`, and `ADDRESS CMD` deliberately
accept environment command strings and retain their shell/platform parsing
contract. No new ADDRESS syntax or runtime surface is required.

The corrected reproducer was compiled optimized and non-optimized and executed
with both `rxvm` and `rxbvm`. All 4/4 executions passed while preserving:

- a whitespace-bearing argument;
- an empty argument;
- embedded double and single quotes;
- Unicode text `Gràdh 中 😀`; and
- shell metacharacters `; && | $(touch NEVER) * ? [x] {y}` as inert data.

The raw passing log is
`/tmp/crexx-cri11-repro.LPn6vO/commands-and-results-pass.log`, SHA-256
`488eec8b3a81a765ef4062419128238f21d55bea6c1646563345480429b884791`.

## Maintained closure

The maintained external fixture is
`lib/rxfnsb/tests_functional/address_argv_fixture.c`. Its Windows entry point
converts the native UTF-16 argument vector to UTF-8 so the same public contract
can be tested without weakening Windows Unicode coverage. The maintained Level
B test is `lib/rxfnsb/tests_functional/ts_address_argv.crexx`. The public
statements reference now distinguishes argv-preserving `CREXX run :argv[]` from
the command-string environments.

| Maintained artifact | SHA-256 |
| --- | --- |
| `address_argv_fixture.c` | `c5f6e1fcb3991c78780a160a694fe1898c69e506e2651e93c00cee4fadb4e391` |
| `ts_address_argv.crexx` | `04fe2b40000e991ca745190a451731615df3501f481e654a3985d1b499c95c10` |
| functional CMake registration | `ffad44390f530de579dbb094be72afd814483e527c70a57d675f9104b45d06ef` |
| `statements.md` | `f7956c965f30b00509f636b4fa91bb1426f2178e02e5e48df19073e12b6c3c0c` |

Focused commands:

```sh
cmake --build /tmp/crexx-integration-ledger.mZ5jyp/candidate-debug \
  --target ts_address_argv_noopt_artifact \
           ts_address_argv_opt_artifact \
           address_argv_fixture --parallel 10
ctest --test-dir /tmp/crexx-integration-ledger.mZ5jyp/candidate-debug \
  -R '^ts_address_argv' --parallel 10 --output-on-failure
```

The exact inventory is five tests: the linked-runtime fixture plus optimized
and non-optimized execution on each VM. All 5/5 pass, zero failed or skipped.
Raw logs and hashes:

- `/tmp/cri11-focused-build.log`:
  `87af13bde557a8a1cbc686926ef342f78f34bb06ffa5678134175a32de0b4939`;
- `/tmp/cri11-focused-list.log`:
  `ad23f9043ed2259a1a6938a8233641a03f480dfcda17dcc6c644f31a9b7becbd`;
- `/tmp/cri11-focused-ctest.log`:
  `5c7f5062f0f528493ec2c36012e0110129698b2241dff4e8a4beabe9191f3a83`.

`git diff --check` passes. The final programme-wide full Debug suite will
include this matrix.

## Compatibility and remaining boundary

No production code, syntax, native ABI, RXAS/RXBIN, serialized format, command
environment behavior, or ownership contract changed. Existing command-string
users remain source-compatible. The documented direct-execution route is the
safe choice when exact argument boundaries matter; consumers that explicitly
select a command-string environment continue to own its quoting and injection
risks.

Redirect-array reuse is independent and remains CRI-12.
