# Cross-runtime benchmark ports

These sources support NR-02 qualification. Canonical sources stay unchanged;
runtime-required adaptations use distinct files and are classified in
`performance/NR-02-WORKLIST.md`.

## RexxCPS

- `rexxcps/rexxcps_2_2.rex` is the exact official ooRexx 2.2 source captured
  from the upstream sample repository on 2026-07-15. SHA-256:
  `b86b1232a3747bacdeba64da16eb79cb0e0115bc3d91da3c2b0b08a80772c8f4`.
- `rexxcps/rexxcps_2_1n.nrx` is the exact example bundled with NetRexx
  5.10-GA. SHA-256:
  `9aa47a25f9aff0085ad8a2600fbf8785b772347b8c9b29427050ae85d93e6dbd`.
- `rexxcps/rexxcps_2_2n.nrx` is a disclosed adaptation that retains the
  bundled NetRexx timed kernel and adds the current 2.2 minimum-duration
  calibration. It is not bundled canonical 2.1n and is not canonical Classic
  2.2.
- files containing `_opaque` are diagnostic variants for opaque inputs,
  deterministic result observation, controlled perturbation and trace work.
  They do not replace or produce the canonical community score.

Opaque A and B execute the same timed clauses and branch outcomes but select
different compound-stem keys, digit/text values and false-comparison operands
from the process argument before timing. Both must observe
`variant|1|69|1.22694`. They are paired specialization/dead-work diagnostics,
not baseline/optimized modes; the exact field mapping is retained in the dated
NR-02 RexxCPS ledger.

The RexxCPS sources retain their original provenance and redistribution terms;
see `../LICENSE-REXXCPS-CPL-1.0.txt` and `../THIRD_PARTY_NOTICES.md`. The
NetRexx distribution also carries its bundled `LICENSE` (ICU license).

## Expanded portfolio ports

`netrexx/` contains the seed and expanded workload ports. The formal common
Sieve, Permute, Bounce, Richards and Base64 sources use `options nobinary
decimal`; their timed arithmetic, loop/state values and numeric arrays use
NetRexx `Rexx` semantics. Generated Java and the default HotSpot JIT are the
ordinary NetRexx implementation/runtime substrate. Base64 separately discloses
Java `byte[]` storage while retaining decimal `Rexx` arithmetic. The remaining
`options binary` expanded ports are binary/JVM diagnostics, not fair Rexx
aggregate inputs. Generated Java/classes, disassembly and pilot output belong
in dated evidence bundles rather than beside these versioned `.nrx` sources.

`oorexx/` contains object-native Bounce/List/Storage ports, the common Richards
state-machine port, a supplied `json.cls` DOM consumer, and a byte-string
Base64 port. The official portable environment wrapper is required so
`::requires 'json.cls'` resolves against the ooRexx installation.

The JSON sources intentionally exercise each implementation's available
surface: cREXX's string/path API, ooRexx's supplied DOM, and a NetRexx parser
using Java collections. Their correctness result is common, but their timed
work is not; the three JSON cells are diagnostics and are excluded from a
common score. Storage is also diagnostic for cREXX because a `StorageNode`
wrapper plus `.object[]` replaces each upstream array node. List uses a cREXX
arena to own elements because references are weak; that adaptation remains
visible in the equivalence ledger.

Base64 uses the same RFC 4648 arithmetic in all three ports. cREXX builds the
encoded output in a pre-sized `.binary`, ooRexx uses its byte-string surface,
and NetRexx uses Java `byte[]` storage with decimal `Rexx` index/byte arithmetic;
all validate length, byte equality and checksum.

## Classic seed ports

`classic/` contains portable or object-native ports for the ooRexx lane.
Regina surrogate checks are not Regina portfolio results and do not qualify an
ooRexx cell. Sieve and Permute retain the algorithm and observable work and
qualify on ooRexx 5.1.0. Mandelbrot uses an exact arithmetic byte-XOR helper
because a portable Classic source-level integer XOR spelling is not shared
with the typed source, but ooRexx decimal arithmetic also changes the common
size-500/750 checksums; the current cell is therefore `not comparable`, not a
runtime-specific accepted answer. The PERF2-08 Towers candidate replaces the
earlier numeric-node/stem diagnostic with an ordinary ooRexx object graph: one
benchmark object plus 14 disk objects, object-method link mutation and the same
8,191 recursive moves per repetition. Adrian approved it as a qualified
separate object/allocation lane on 2026-07-27. PERF2-09 formal timing uses
source SHA-256
`bb081b76306ce1d360f4e739e480e3e89ebceb31028326bc93910c8daa0267b9`.

## Lifecycle probes

`lifecycle/` contains one deliberately tiny deterministic program per runtime.
`performance/tools/run_lifecycle.crexx` measures cREXX compile, assemble and
cold load-to-first-result; ooRexx translate and cold load-to-first-result; and
NetRexx compile and JVM load-to-first-result. A load-only CLI boundary is not
available consistently, so the final phase is explicitly combined rather than
being presented as pure loader time. The NetRexx probe uses `options nobinary
decimal` and `Rexx` arithmetic; the default HotSpot JIT remains part of the
canonical runtime mode.
