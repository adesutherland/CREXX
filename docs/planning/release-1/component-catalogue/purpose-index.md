# Stage 4: Purpose-Oriented Component Index

This view reorganises the raw catalogue by what a component is for. It does
not assign product role, delivery, maturity, quality, or final contract level.
Those remain independent questions.

## Assignment rule

Every raw catalogue row has exactly one primary purpose domain. A row can also
have secondary purpose tags, but secondary tags do not duplicate its identity.
The mapping is deterministic:

- syntax, Classic BIF, macro, source-included procedure, package, exit, tool,
  demo, example, and PoC rows are
  assigned by the tables below;
- a Rexx export or public member inherits the primary purpose of its source
  selector/file;
- a native `RXPA-` registration inherits the purpose of its containing plugin
  package, except native DES registrations inherit the DES source package;
- residual implementation candidates are assigned by their named function,
  while lifecycle remains a separate later classification.

This lets the leaf files remain the exhaustive contract inventory without
copying more than two thousand rows into a second table.

## Purpose domains

| Domain | Purpose | Typical consumers |
|---|---|---|
| `P01` | language, compiler, preprocessor, debugger, and toolchain engineering | language implementers and build tooling |
| `P02` | runtime control, host environments, signals, trace, reflection, and module state | programs and host integrations |
| `P03` | text, character, word-independent search, and regular expressions | general applications and compiler/library code |
| `P04` | numeric, decimal, date/time, random, and numeric formatting | calculations and runtime numeric support |
| `P05` | binary conversion, hashing, cryptography, and identifiers | systems/data applications |
| `P06` | arrays, collections, maps, sets, stacks, stems, and vectors | general applications and compiler/library code |
| `P07` | files, console, queues, OS, process, pipe, and local system integration | hosts, tools, and applications |
| `P08` | sockets, TCP, HTTP, TLS, and network protocols | networked applications |
| `P09` | structured parsing, quoted words, masks, lexing, and data formats | language/data processing |
| `P10` | object model, classes, interfaces, factories, iteration protocols, and root value presentation | object-oriented typed programs |
| `P11` | database, GUI, XML, mainframe, and other external-system adapters | platform/application integrations |
| `P12` | Classic Rexx, Level C, and RexxScript compatibility/runtime semantics | Rexx compatibility users and tooling |
| `P13` | Level G hosted-service and LLM abstractions | Level G application users |
| `P14` | Level L language-oriented and embedded-language facilities | language/tool authors |
| `P15` | demonstrations, tutorials, examples, tests-as-samples, and compiler PoCs | learners and developers |

## Language and preprocessor capabilities

| Raw surface | Primary purpose assignment |
|---|---|
| all `SYN-CLASSIC-*` rows | `P12` Classic/Level C semantics |
| all `SYN-RXPP-*` rows | `P01` preprocessor/language tooling |
| typed class/object IDs `TYPE-OBJECT`, `CLASS`, `INTERFACE`, `IMPLEMENTS`, `FACTORY`, `METHOD`, `ATTRIBUTE`, `SELF`, `IS-TYPEOF`, `REFERENCE`, `DEREFERENCE`, `SNAPSHOT` | `P10` object model |
| typed array IDs `DYNAMIC-ARRAY`, `FIXED-ARRAY`, `MULTIDIM-ARRAY`, `ARG-ARRAY`, `ARRAY-APPEND`, `ARRAY-INSERT`, `ARRAY-REMOVE`, `ARRAY-CLEAR` | `P06` arrays/collections |
| typed runtime IDs `REGISTER-VIEW`, `PROCEDURE-EXPOSE`, `SAY`, `PULL`, `PUSH`, `QUEUE`, `DROP`, `ADDRESS`, `PARSE`, `SIGNAL`, `TRACE` | `P02` runtime/host control |
| typed binary IDs `TYPE-BINARY`, `NAMED-INTEGER-OPS`, `BINARY-AT` | `P05` binary/systems data |
| typed `CONCAT` capability | `P03` text |
| every other `SYN-TYPED-*` row | `P01` typed language/toolchain foundation |

The Level G and Level L `OPTIONS` selectors remain in `P01`: they select a
front-end surface, while their library components are in `P13` and `P14`.

All 70 `CBIF-*` rows are in `P12`. The eight typed intrinsic/reserved/doc-only
rows are assigned as follows: `BIF-TYPED-ARG` to `P01`; `ADDRESS`, `CONDITION`,
`QUEUED`, `SOURCELINE`, `STORAGE`, and reserved `TRACE` to `P02`; and
`JUSTIFY` to `P03`.

RXPP support leaves inherit the purpose of their containing support file:

| Support surface | Primary purpose assignment |
|---|---|
| `MACRO-maclib-*` | by function: text helpers to `P03`, numeric helpers to `P04`, stem/map/control helpers to `P06`, file helpers to `P07`, `cparse` to `P09`, and debug/comment helpers to `P01` |
| `MACRO-macsys-*` | by function: text predicates/quoting to `P03`, numeric/logical macros to `P04`, `argv`/`execio`/`clear` to `P07`, and inline-assembler helpers to `P01` |
| `RXPPPROC-mathlib-*` | `P04` numeric |
| `LIB-RXPP-syslib` | `P15` placeholder/sample residue because it contains no callable surface |

## Level B library selectors

All exports and public members sourced from a selector inherit the selector's
domain.

| Domain | `lib/rxfnsb/rexx` selectors |
|---|---|
| `P02` runtime/control | `_address`, `_rxsystem`, `getenv`, `linesize`, `loadmodule`, `raise`, `signal`, `symbol`, `trace`, `value`, `version` |
| `P03` text | `abbrev`, `center`, `centre`, `changestr`, `compare`, `copies`, `countstr`, `delstr`, `filter`, `insert`, `length`, `lower`, `overlay`, `pos`, `lastpos`, `left`, `right`, `reverse`, `space`, `strip`, `substr`, `substro`, `translate`, `upper`, `verify`, `regex`, `sequence`, `find`, `index`, `wordrep` |
| `P04` numeric/date/time | `_datei`, `_dateo`, `_ftrunc`, `_itrunc`, `_jdn`, `abs`, `date`, `format`, `max`, `min`, `numeric`, `random`, `reradix`, `sign`, `time`, `trunc`, `fmtmask`, `compilemask`, `runmask` |
| `P05` binary/hash | `b2x`, `b2d`, `binary`, `c2x`, `c2d`, `d2b`, `d2c`, `d2x`, `fnv`, `x2b`, `x2c`, `x2d`, `xrange` |
| `P06` arrays/stems | `arrayfind`, `splice`, `arrayinsert`, `arraydelete`, `arrayappend`, `arrayprepend`, `objectarrayinsert`, `objectarraydelete`, `objectarrayappend`, `objectarrayprepend`, `objectarraydrop`, `objectarraymove`, `arraypop`, `arrayshift`, `arrayget`, `arrayset`, `arraycontains`, `arrayindexof`, `arrayreverse`, `arrayjoin`, `arraysort`, `arraycopy`, `arraydrop`, `arrayhi`, `arraymove`, `stem` |
| `P07` files/output | `fileio`, `fsayfmt` |
| `P08` network | `rxsocket`, `rxhttp` |
| `P09` parsing/data | `delword`, `qpos`, `qsplit`, `qsplitsafe`, `qextractall`, `qextractpair`, `qstripcomment`, `qremoveall`, `word`, `words`, `wordindex`, `subword`, `wordlength`, `wordpos`, `qword`, `qwordlength`, `qwords`, `qwordindex`, `qwordpos`, `qsubword`, `parsecompile`, `parsestring`, `parse`, `datatype`, `parseExec`, `rxjson` |

This table contains all 129 active selectors exactly once. The former
`arraydump` and `arrayformat` selectors are catalogued as function examples.

## Class, G, C, L, vector, and RexxScript sources

| Domain | Source selectors/files and inherited leaves |
|---|---|
| `P10` object protocols | `StringIterable`, `StringIterator`, `ObjectIterable`, `ObjectPrintable`, `ObjectIterator`, `ObjectComparator`, `RexxComparator`, and `Rexx` in classlib |
| `P06` collections | all classlib `*ArrayList*`, `*HashMap*`, `*HashSet*`, `*LinkedList`, `*Stack`, `*TreeMap*`, `*TreeSet*`, and their iterators, including `StringOldTreeMap`; `lib/veclib/veclib.crexx` |
| `P09` search/lexing | classlib `Qfind` and `Scanlex` |
| `P05` identifiers | native-backed classlib `Id` |
| `P11` persistent adapters | native-backed classlib `KeyDB` |
| `P07` operating-system adapter | native-backed classlib `Os`; `mylib1` procedures embedded in the system-plugin examples |
| `P13` Level G hosted services | all `lib/rxfnsg/rexx/llm.crexx` types and members |
| `P12` Classic/RexxScript runtime | every `lib/rxfnsc` export/member and every `rexxscript` export/member |
| `P14` embedded-language support | every `lib/rxfnsl/rexx/tinyexpr.crexx` export |

The classlib row rules account for all 33 pure-Rexx selectors and all three
native-backed selectors.

## Native and hybrid packages

All public RXPA registrations beneath a package inherit the package domain.

| Domain | Packages |
|---|---|
| `P03` text/regex | `PKG-RXPA-strings`, `PKG-RXPA-regex` |
| `P04` numeric | `PKG-RXPA-getpi`, `PKG-RXPA-float`, `PKG-RXPA-stats`, `PKG-RXVM-mc-decimal`, `PKG-RXVM-db-decimal`; Rexx `rxint` and `rxdecimal` exports |
| `P05` crypto/identifier | `PKG-RXPA-cipher`, `PKG-RXPA-hash`, `PKG-RXPA-id`, `PKG-RXFNSB-NATIVE-DES` |
| `P06` collections | `PKG-RXPA-arrays`, `PKG-RXPA-llist`, `PKG-RXPA-map`, `PKG-RXPA-matrix`, `PKG-RXPA-stack`, `PKG-RXPA-treemap` |
| `P07` local system | `PKG-RXPA-console`, `PKG-RXPA-fileio`, `PKG-RXPA-fpool`, `PKG-RXPA-pipe`, `PKG-RXPA-process`, `PKG-RXPA-fs`, `PKG-RXPA-platform` |
| `P08` network | `PKG-RXPA-rxtcp`, `PKG-RXPA-socket` |
| `P11` external adapters | `PKG-RXPA-gui`, `PKG-RXPA-keyaccess`, `PKG-RXPA-odbc`, `PKG-RXPA-pick`, `PKG-RXPA-recv390`, `PKG-RXPA-rxml` |
| `P15` sample residue | `PKG-RXPA-testrx` |
| `P01` preprocessor infrastructure | `PKG-RXPA-precomp` |

The matrix assigns all 28 `lib/plugins` RXPA directories, the separate internal
RXPP `precomp` package, both VM plugin directories, and the native DES source
package.

## Compiler exits and tools

| Domain | Components |
|---|---|
| `P01` compiler/tooling | `EXIT-dump`, `EXIT-pprint`, `EXIT-query`, `INFRA-EXIT-TOKEN`; `TOOL-rxc`, `TOOL-rxas`, `TOOL-rxlink`, `TOOL-rxdas`, `TOOL-rxcpack`, `TOOL-crexx`, `TOOL-rxpp`, `TOOL-rxpp-sh`, `TOOL-rxdb`, `TOOL-rxseq`, `TOOL-rxvm-instrumented`, `TOOL-rxbvm-instrumented`; `INFRA-RXPA`, `INFRA-RXPASHIM`, `INFRA-VM-PROFILING`; `LIB-RXPP-maclib`, `LIB-RXPP-macsys`, `LIB-RXPP-mathlib` |
| `P02` language/runtime control | `EXIT-address`, `EXIT-parse`, `EXIT-signal`, `EXIT-trace`; `TOOL-rxvm`, `TOOL-rxbvm`, `TOOL-rxvme`, `TOOL-rxbvme`, `LIB-rxvml`, `LIB-rxbvml`, `LIB-crexxsaa`, `TOOL-crexxsaa` |
| `P03` text | `EXIT-substr` |
| `P04` numeric | `EXIT-add` |
| `P06` collections | `EXIT-sort`, `EXIT-sortx` |
| `P07` I/O | `EXIT-execio`, `EXIT-fsay`, `EXIT-msay` |
| `P12` Rexx compatibility | `EXIT-rexxscript`, `TOOL-rexxscript` |
| `P15` exit demonstration | `EXIT-dummy` |

All 16 exits and all primary toolchain/runtime/host/profiling entries are
assigned exactly once. An exit can be language-critical or a demonstrator
within the same linked image; that product-role distinction is made in Stage 6.

## Residual and alternate sources

| Domain | Raw IDs/source family |
|---|---|
| `P02` runtime bridge | `SRC-RXFNSB-RXAS-rxvml-address-native-rxas` |
| `P03` text | residual RXAS `copies`, `lastpos`, `left`, `length`, `pos`, `reverse`, `right`, `substr`, and all residual word routines |
| `P04` numeric/time | `LIB-rxmath-source`, every `SRC-RXMATH-*` row, `SRC-RXFNSB-RXAS-elapsed-rxas`, and `dectest.rxas` |
| `P15` test residue | `SRC-RXFNSB-RXAS-testbifs-rxas` |
| `P01` preprocessor workbench residue | `SRC-RXPP-rxppt` |

Residual status is not itself a purpose; these rows remain grouped with the
capability they were intended to provide.

## Demonstrations, examples, and PoCs

Every `DEMO-*`, `EXAMPLE-*`, and `POC-*` raw row has primary domain `P15`.
Their secondary tags preserve what they demonstrate:

| Secondary purpose | Components |
|---|---|
| host/ADDRESS | two CMS demos, LLM ADDRESS demo/environment, native KV demo, installed rxsqlite provider/Level G ADDRESS consumer demo |
| hosted LLMs | Anthropic, Gemini, Ollama, and OpenAI generation demos |
| Rexx compatibility | RexxScript demo |
| numeric/algorithms | Monte Carlo, factorial, Fibonacci, Gaussian square, GCD, LCM, nth-root, sieve, and sort examples |
| text/data | anagram, Caesar, strings, stems/RXPP, and cross-reference examples |
| object/system basics | bank, expose variables, hello, date, time, and system examples |
| compiler architecture | Level C Lemon fallback and token-adapter-trace PoCs |

## Purpose-level duplication map

This view makes the principal overlaps visible without deciding which
implementation wins:

| Purpose | Coexisting implementations |
|---|---|
| arrays/collections | typed `rxfnsb` routines, pure-Rexx classlib collections, RXPA arrays/list/map/stack/tree/matrix plugins, and `veclib` |
| text/regex | typed Rexx functions, pure-Rexx regex, native strings/regex plugins, and residual RXAS functions |
| numeric | typed Rexx functions, native `rxmath`, old unbuilt `rxmath`, two decimal VM plugins, and tiny `getpi` |
| files/OS/process | typed file helpers, native system/fileio/pipe/process/console plugins, and native-backed `Os` |
| networking | VM-backed `rxsocket` and Rexx `rxhttp`, plus native `rxtcp` and deprecated OpenSSL socket plugin |
| identifiers/keys | native `id` and `keyaccess` plus classlib adapters |
| Rexx execution | Level C front end/runtime support and the distinct RexxScript evaluator/runtime |

These overlaps are the input to quality assessment and provisional product
classification, not evidence that the components are interchangeable.
