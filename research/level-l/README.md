# Level L research

Level L itself lives in [`langtools/`](../../langtools/README.md). Its reusable
SDK lives in [`lib/rxfnsl/`](../../langtools/doc/user-guide.md). This directory
owns experimental algorithms and the [new compiler programme](compilers/README.md),
not a second copy of the production libraries.

[PLAN.md](PLAN.md) owns the accepted programme and current repository refactor.
The [compiler plan](compilers/PLAN.md) and [Classic instruction ledger](compilers/doc/LEVELC-INSTRUCTION-REVIEW.md)
own C-first delivery. Both research compilers are Level B consumers of Level L;
proper B/self-hosting and INTERPRET remain later work.

The broader research compares lexer/parser algorithms and shared structures.
Read [SURFACE.md](SURFACE.md) for authored rules, [SEMANTICS.md](SEMANTICS.md)
for meaning, [MATH.md](MATH.md) for proofs and limits, [AUDIT.md](AUDIT.md) for
coverage, and [SOURCE-AUDIT.md](SOURCE-AUDIT.md) for external source provenance.
Pika and Squirrel remain experimental; the installed generator does not silently
select them. Research conclusions are conditional on admitted profiles and
corpora, not claims of universal parser complexity or production readiness.

`src/` contains experimental implementations only. Runners read canonical SDK
sources and the built Level L frontend; there are no source aliases. `cases/`
contains maintained consumers and independent oracle code; `cases/fixtures/`
contains their named inputs. Authored specs, grammars and licensed source data
remain where their consumers use them.

Run a focused experiment against an existing native build, for example:

```sh
CREXX_BIN_DIR="$PWD/cmake-build-debug/bin" research/level-l/run-toolkit.sh opt toolkit
```

Runners own temporary build/output directories and delete them on exit. Oracle
scripts compare maintained fixtures by default; an explicit `--update` is for
reviewed fixture changes. Do not store logs, raw samples, receipts or per-run
hash bundles in this tree. Current conclusions belong in the owning plan or
contract; full programme and platform gates stay open until verified.
