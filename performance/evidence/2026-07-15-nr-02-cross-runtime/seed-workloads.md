# Seed workload qualification ledger

Status: current CREXX / ooRexx / NetRexx seed cells captured and dispositioned

All retained pilots are serial and process-startup-inclusive. They are NR-02
qualification samples, not formal NR-10 baselines or aggregate inputs. Each
`pilot/manifest.json` records exact argv, cwd, lifecycle, warmups and runs;
`samples.csv` and `raw/` retain elapsed time and every output stream.

## Source and license ledger

| Workload | CREXX source | NetRexx source | Classic/ooRexx-targeted source | License |
| --- | --- | --- | --- | --- |
| Sieve | `tests/benchmarks/awfy_sieve.crexx` | `tests/benchmarks/cross-runtime/netrexx/awfy_sieve.nrx` | `tests/benchmarks/cross-runtime/classic/awfy_sieve.rex` | AWFY/SOM MIT |
| Permute | `tests/benchmarks/awfy_permute.crexx` | `tests/benchmarks/cross-runtime/netrexx/awfy_permute.nrx` | `tests/benchmarks/cross-runtime/classic/awfy_permute.rex` | AWFY/SOM MIT |
| Mandelbrot | `tests/benchmarks/awfy_mandelbrot.crexx` | `tests/benchmarks/cross-runtime/netrexx/awfy_mandelbrot.nrx` | `tests/benchmarks/cross-runtime/classic/awfy_mandelbrot.rex` | AWFY/Benchmarks Game Revised BSD |
| Towers | `tests/benchmarks/awfy_towers.crexx` | `tests/benchmarks/cross-runtime/netrexx/awfy_towers.nrx` | `tests/benchmarks/cross-runtime/classic/awfy_towers.rex` | AWFY/SOM MIT |

The CREXX sources are the versioned seed. Other sources are direct ports from
those algorithms and their documented upstream lineage, not unrelated programs
selected to produce matching answers. Notices and full texts remain in
`tests/benchmarks/THIRD_PARTY_NOTICES.md`, `LICENSE-SOM-MIT.txt` and
`LICENSE-AWFY.md`.

## Correctness, equivalence and optimizer resistance

| Workload | Contract and perturbations | NetRexx generated-form finding | Disposition |
| --- | --- | --- | --- |
| Sieve | 5,000 slots; identical marking bounds; 669 primes; repetitions 1/2 pass on all three runtimes | primitive `int[5001]`; argv controls repetitions; result controls failure and output | CREXX, ooRexx and NetRexx `equivalent port`; Classic stems are the runtime's array representation |
| Permute | new state per repetition; `permute(6)`; paired swaps; 8,660 calls; repetitions 1/2 pass on all three runtimes | generated object construction, recursive method calls and six-element primitive array retained | CREXX, ooRexx and NetRexx `equivalent port`; Classic procedural representation is disclosed |
| Mandelbrot | CREXX/NetRexx sizes 1/500/750 produce 128/191/50; ooRexx produces 128/255/128 and fails the common contract at 500/750 | runtime size and result path retained; NetRexx has no Java `^` source spelling, so exact XOR is an eight-step arithmetic helper and final partial-byte shift is a loop | CREXX `equivalent port`; NetRexx `disclosed adaptation` pending aggregate review; ooRexx `not comparable` because decimal arithmetic changes boundary results |
| Towers | disk construction through size 0; 13-disk recursive move graph; 8,191 moves; repetitions 1/2 pass on all three runtimes | generated disk/benchmark objects, `new TowersDisk`, three pile roots, recursion and result path retained | CREXX/NetRexx `equivalent port`; ooRexx procedural diagnostic is `not comparable` for object/allocation scoring because numeric nodes/stems remove object allocator/dispatch cost |

For every NetRexx workload, generated Java, all emitted class files and
`javap -c -p` output are retained under the workload's `netrexx/` directory.
The compiler emitted class-file version 52.0 (Java 8) on JDK 26.0.1. Generated
Java demonstrates that opaque process arguments feed the workload and that
the deterministic result remains externally observed; the JIT cannot remove
the entire workload without changing the pass/fail/output path.

For every ooRexx source, `rexxc` produced an executable translated image under
the workload's `oorexx/translated/` directory and the image was executed with
an opaque argument. ooRexx does not expose a human-readable generated source
equivalent to NetRexx's Java output; the encoded image and the RexxCPS
interpretive trace remain distinct evidence modes.

## Retained pilot modes

| Workload | Formal seed argument used for pilot | CREXX recorded process range | ooRexx recorded process range | NetRexx recorded process range | Sampling |
| --- | ---: | ---: | ---: | ---: | --- |
| Sieve | 50 repetitions | 30.877–31.881 ms | 79.831–83.167 ms | 38.019–41.171 ms | 1 warmup + 3 recorded each |
| Permute | 50 repetitions | 118.665–131.372 ms | 193.434–283.832 ms | 34.577–35.857 ms | 1 warmup + 3 recorded each |
| Mandelbrot | size 500 | 278.577–481.684 ms | no score; common checksum failed after 22.255 s | 68.608–71.039 ms | passing runtimes: 1 warmup + 3; ooRexx negative: one serial run |
| Towers | 10 repetitions | 1.162–1.567 s | 221.435–243.227 ms diagnostic only | 42.993–43.841 ms | 1 warmup + 3 recorded each |

The CREXX Mandelbrot maximum, ooRexx Permute spread and other noisy observations
are retained rather than removed. No ratio or ranking is approved from these
small startup-inclusive pilots. NR-10 must repeat a controlled same-host matrix
and separate steady-state from startup. ooRexx Mandelbrot and Towers are not
eligible for a common aggregate under their current dispositions.

## ooRexx qualification and prior surrogate boundary

The official ooRexx 5.1.0 runtime passed Sieve and Permute at repetitions 1 and
2 and retained pilot sizes, and passed Towers as a disclosed procedural
diagnostic. The previously recorded Regina executions remain syntax/correctness
surrogates only; Regina has no non-RexxCPS portfolio role.

Mandelbrot passed size 1 under ooRexx but failed the common checksum at sizes
500 and 750. The two serial negative captures retain exact elapsed time and raw
output. Changing the reference result to accept ooRexx decimal arithmetic would
silently change the correctness contract, so the cell is `not comparable`.

## Negative and bounded findings

- NetRexx compiler scripts returned shell status zero for a failed translation
  during Mandelbrot development; automation must also inspect compiler output
  or required class artifacts instead of trusting the wrapper exit code alone.
- Portable Classic/NetRexx integer XOR cannot be spelled as Java `^` in
  NetRexx source. The exact arithmetic helper is visible and deliberately not
  hidden as equivalent-cost work.
- Classic Towers can preserve the algorithm with node ids and stems but not
  object allocation/dispatch pressure. A matching result alone is insufficient
  for common-score inclusion, so the current ooRexx cell is `not comparable`.
- ooRexx decimal arithmetic changes Mandelbrot boundary decisions at the common
  sizes. Size 500 returned 255 instead of 191 and size 750 returned 128 instead
  of 50; both failures are retained and excluded rather than normalized away.
- The pilot timings include JVM/VM process startup and class/module load. They
  do not answer steady-state throughput.
