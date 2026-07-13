# Stage 7: Cross-Cutting Conclusions and Approval Questions

## Outcome

The repository now has a maintained, source-grounded catalogue of 2,153
language, library, BIF, class/member, native registration, plugin, exit, tool,
debugger, preprocessor, macro, example, and residual-source identities. The
catalogue is sufficient to begin Eel1 product-boundary decisions, but the Stage
6 classifications are proposals rather than approved policy.

The central conclusion is that one “core/optional/example” flag is not enough.
Eel1 needs at least four independent decisions for every component:

1. which language contract it serves;
2. whether it is bootstrap core, level core, standard, integration, optional,
   example, experimental, deprecated, or a removal candidate;
3. whether it is required, default, opt-in, developer-only, or not shipped;
4. whether its implementation is Rexx, RXAS, native, generated, hybrid, or a
   mixed package, with a separate maturity/quality record.

This avoids the current failure mode in which “built into `bin`” is read as
“supported and core”.

## 1. Level B must be a measured bootstrap closure

The present `library.rxbin` contains 131 selectors. Stage 6 provisionally puts
87 in a conservative bootstrap-core candidate, 35 in the B standard/default
layer, and nine in optional images. Even 87 is likely too broad for the stated
goal of a light, essential, very fast bootstrap language.

The final B core should be selected by this sequence:

1. define the exact Eel1 programs that must bootstrap: compiler components
   written in Rexx, RXPP, certified exits, the `crexx` driver, and any required
   class/runtime modules;
2. extract their transitive procedure/class/RXAS/native dependencies from
   linked metadata and build inputs;
3. remove convenience functions that are not in that closure;
4. set quality, size, startup, compilation, and runtime performance gates for
   the remaining closure;
5. only then begin the separate full re-engineering task.

The later performance programme should prioritise scalar text, word/parse,
arrays, binary conversions, call/link paths, required classlib structures, and
runtime-control bridges. Pure Rexx regex, JSON, HTTP, large collection
implementations, and external integrations need their own workload-specific
benchmarks; they must not distort the bootstrap baseline.

No optimisation or B-core implementation change was made in this task.

## 2. Level G is currently an overlay, not yet a complete user platform

All current typed syntax other than the level selectors is provisionally
common to B and G because the compiler does not enforce a distinct G syntax
delta. This is an honest description of the current contract, not a design
claim that every low-level B idiom should be encouraged in G programs.

The only G library is the LLM abstraction plus four providers. It demonstrates
the intended user/integration direction but does not constitute a general G
standard library. The interface should be separable from provider modules so
that credentials, service drift, and network dependencies remain opt-in.

An eventual decision to hide register views, inline RXAS, binary memory access,
or other system constructs from G is a language-design change. It requires an
approved feature matrix and compiler enforcement, not documentation alone.

## 3. Level C requires its own contract and compliance ledger

Level C is correctly treated as unique even where a B function has the same
name. Its current state has three distinct layers:

- the Classic parser/diagnostic/highlighting surface recognises broad syntax;
- the compiler has a 70-name BIF recognition table;
- the Level B-implemented dispatcher contains 19 names, while this review found
  proven compiled Level C paths only for `LENGTH` and `SUBSTR`.

Eel1 must not describe recognition as standards compliance. Each of the 70
`CBIF-*` identities needs its own signature, coercion, error, option, result,
conformance-test, and implementation status. Reuse of a B algorithm is welcome
behind that boundary, but it cannot merge the B and C contracts.

RexxScript is also separate. It is a B-authored compatibility evaluator and
CLI, not a second implementation of Level C.

## 4. Level L is a skeleton

The 12 tinyexpr exports are a useful pure-Level-L demonstration of packed
tokens and language-oriented binary facilities. They should remain an example
or developer component until Level L has an approved syntax/library purpose,
contract, and quality plan. Default installation currently communicates more
maturity than the surface supports.

## 5. RXPP is valuable; its convenience libraries need curation

RXPP itself should be treated as standard shared source tooling. Its source
maps, diagnostics, parser shell, and native `precomp` mechanics have meaningful
focused evidence. The 20 `precomp` registrations are an internal compatibility
boundary, not general user BIFs.

The four staged support files are a different quality tier:

- `maclib` has 35 macros but only a tiny tested sample; `startswith` is visibly
  broken because it accepts `suffix` and refers to undefined `prefix`;
- `macsys` has 29 macros, including inline-assembler/system helpers, but lacks
  leaf-level tests and contains suspect `execio`/`clear` definitions;
- `mathlib` has six included procedures but no focused tests, and source review
  finds incorrect use of Level B remainder in `lcm`, `isPrime`, and `modinv`;
- `syslib` is an installed empty placeholder.

The engine, the internal native API, and the shipped macro libraries must
therefore have separate delivery decisions. The proposed Eel1 position is:
RXPP default; `precomp` required internally; `rxpp-sh` developer/optional;
macro libraries developer-only until fixed and comprehensively tested;
`mathlib` and `syslib` not shipped in their current state. The unselected
machine-specific `rxppt.crexx` workbench should be archived or removed after
approval.

## 6. RXDB is an experiment, not yet a debugger product contract

RXDB is a real Level B self-hosting asset: it is compiled, assembled, linked,
C-packed, installed, and its usage smoke passes. It also exposes a step handler
and an 18-member text UI class. However, source and release notes call it a
prototype/experimental tool, and there are no automated stepping, watch,
module-load, trace-event, or UI-state tests.

It should be developer-only for Eel1. Promotion to default requires an explicit
debugger protocol, automated end-to-end sessions, error/lifecycle tests, and
measured tracing overhead. This keeps RXDB useful as a language/runtime
workload without promising production debugger quality.

## 7. Native plugins need a narrow integration policy

The 27 `lib/plugins` directories range from required tool integration to
incomplete residue. Their default-build status is not a useful taxonomy.

The proposed policy is:

- native plugins are appropriate for OS, database, GUI, host, and external
  integration, or for a measured primitive that cannot meet its contract in
  Rexx/RXAS;
- pure algorithm/collection duplicates do not become core merely because they
  are native;
- opt-in integrations own their dependency, platform, lifecycle, sanitizer,
  fault, and conformance tests;
- demonstrators (`getpi`, `testrx`, incomplete `fpool`) and deprecated
  alternatives must not ship by default;
- the broad `system` package needs an internal tool-required subset separated
  from its wider user API;
- `mc_decimal` is the normal core decimal provider; `db_decimal` remains a
  separately tested alternative until equivalence is proven.

This reduces duplicated contracts across strings, regex, arrays, maps, stacks,
trees, file I/O, sockets, and math before performance engineering begins.

## 8. Compiler exits need bundle-level separation

Certified ADDRESS, PARSE, SIGNAL, and TRACE behavior plus token infrastructure
is core language machinery. RexxScript, developer inspection exits, optional
I/O exits, and mechanism demonstrations have different contracts. Linking all
16 into `rxcexits.rxbin` currently hides that difference.

Eel1 should either split the bundle or use an explicit manifest that records
which exits are required, default, optional, and developer-only. Demonstrator
exits should remain source examples rather than silently supported language
features.

## 9. Packaging must become manifest-driven

The current install sweeps the build `bin` directory. That couples package
contents to configuration, stale files, test-only executables, static archives,
conditional plugins, and incidental output. It also explains why experimental
RXDB, L, G providers, profiling tools, and instrumented test VMs can appear
more official than intended.

The approved catalogue should become the source for explicit Eel1 manifests:

- minimal bootstrap/developer toolchain;
- normal G user distribution;
- Level C distribution and compliance assets;
- optional integrations/providers;
- SDK/native-packaging assets;
- developer examples, profilers, RXPP shell, and RXDB.

Static archives that native packaging genuinely needs must remain deliberately
included; a manifest is not permission to “tidy” them away.

## 10. Quality and performance evidence needs release gates

The focused validation was clean: 115 library/plugin/exit/RXPP tests, 24
additional tool/runtime tests, and the RXDB smoke all passed. Those 140 selected
passes establish useful current evidence, but not coverage, conformance,
cross-platform behavior, security, or performance.

Suggested Eel1 gates by role:

| Role | Minimum evidence before promotion |
|---|---|
| B bootstrap core | deterministic functional/negative tests, cross-mode/runtime coverage where applicable, linked-size/startup/runtime baselines, RexxDoc or canonical contract docs, and no unexplained hybrid dependency |
| level core | broad functional and malformed-input tests, explicit ABI/format compatibility, cross-platform CI, and repeatable performance baselines for hot paths |
| standard | public contract/reference, focused functional/error tests, and scale tests appropriate to complexity |
| integration/optional | dependency/platform matrix, lifecycle/failure tests, ownership semantics, and opt-in packaging isolation |
| example/experimental | build/run smoke and a clear non-contract label; no default package implication |
| deprecated/removal | usage/dependency search, migration note, and explicit approval before deletion |

Native handle, persistence, process, networking, parser, and collection code
needs sanitizer/fault/concurrency testing proportional to risk. Network and I/O
latency must be separated from CPU/allocation benchmarks. VM profiling and
`rxseq` are enabling tools for this programme, not substitutes for workload and
acceptance definitions.

## 11. The catalogue should be mechanically maintained

The late discovery of primary tools, RXDB leaves, and macro libraries shows why
manual review alone is insufficient. A later implementation task should add a
read-only catalogue checker that reports, without silently rewriting policy:

- CMake selectors missing a catalogue identity;
- namespace exports or public members missing from raw leaves;
- new `ADDPROC` registrations or plugin directories;
- new Level C recognised BIF names;
- new RXPP directives, shipped `##define` macros, or `##USE` procedures;
- new tools/install targets, exits, demos, examples, or PoC roots;
- duplicate stable IDs and stale source paths;
- a changed component whose classification/evidence review has not been
  refreshed.

Classification remains a reviewed human decision. Discovery and stale-path
checks should be automated.

## Decisions requested for the later approval stage

The next review should approve, reject, or amend these points before any
re-engineering or packaging change:

1. the independent contract/role/delivery/composition/maturity axes and the
   added distinction between `bootstrap core`, `level core`, and `standard`;
2. the dependency-derived method for the final B bootstrap closure, using the
   87-selector list only as a conservative starting candidate;
3. the current B+G shared typed-syntax conclusion, and whether any low-level B
   features must be prohibited in G for Eel1;
4. the unique 70-item Level C BIF ledger and the rule that recognition,
   dispatcher implementation, executable lowering, and compliance remain
   separate statuses;
5. Level L as developer/example only for Eel1;
6. RXPP as default tooling, `precomp` as internal core, and the shipped macro
   libraries demoted until repaired/tested;
7. RXDB as developer-only experimental tooling;
8. the 27-plugin disposition table, especially `system`, native collection
   duplicates, network alternatives, and decimal-provider roles;
9. compiler-exit bundle partitioning;
10. manifest-driven delivery profiles replacing the broad `bin` install sweep;
11. the role-based quality gates and automated catalogue-drift checker.

Once those decisions are approved, the first implementation programme should
freeze and measure the final Level B bootstrap closure. It should not begin by
optimising every component currently present in `library.rxbin`.
