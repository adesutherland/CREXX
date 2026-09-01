# Native Intel PMU evidence

This directory contains serial `perf stat` and cycle-sampling evidence from the
exact ordinary profiling-off GCC and Clang Release products. No command in this
capture configured, regenerated or built a product.

## Product freeze

`provenance/products.sha256` identifies both VM binaries, the shared runtime
library and the two exact formal manifests. Every cell also has
`products.sha256` and shell-escaped `command.txt`.

The capture host reported:

- Intel Core i5-1135G7;
- Linux 7.0.0-27-generic;
- kernel PMU name `icelake`;
- perf 7.0.12;
- `kernel.perf_event_paranoid=-1`;
- `kernel.kptr_restrict=1`; and
- `kernel.nmi_watchdog=1`.

`provenance/pre-native-perf.txt` records the preflight, including the absence
of active build/test processes.

## Counter method

The primary GCC matrix covers Sieve, Permute, Bounce, Richards, Base64, Towers
and the separately governed RexxCPS community lane in both VM modes. Each
formal command ran once per isolated event pass:

```text
core:
  {cycles,instructions,branches,branch-misses}
topdown:
  {slots,topdown-retiring,topdown-bad-spec,topdown-fe-bound,topdown-be-bound}
retired-l1i:
  cpu/event=0xc6,umask=0x1,frontend=0x12/
retired-itlb:
  cpu/event=0xc6,umask=0x1,frontend=0x14/
front-walk:
  {icache_64b.iftag_miss,itlb_misses.walk_completed}
indirect:
  {br_inst_retired.indirect,br_misp_retired.indirect}
host:
  {task-clock,context-switches,cpu-migrations,page-faults}
```

The command template was:

```bash
perf stat --no-big-num -x ';' -e "$EVENTS" -o "$CELL/$GROUP.stat" \
  -- "${ARGV[@]}"
```

`ARGV` came directly from the versioned formal manifest. The expected
correctness string was required after every pass. A cell received `COMPLETE`
only after all passes succeeded. An interrupted run therefore resumes without
repeating completed cells.

The two `frontend_retired.*` names exposed by `perf list` cannot be
co-scheduled on this host: one reported not counted and the other not
supported. Their documented raw encodings count correctly when isolated, so
the formal GCC capture uses one raw pass for each. All 98 GCC stat files are
100% scheduled and contain no unsupported/uncounted event.

The bounded Clang direction control uses the same 14 cells with core, top-down,
front-walk and indirect passes. All 56 files are 100% scheduled and
correctness-passing. Host and constrained retired-event passes were not
duplicated because GCC is the primary PMU authority.

RexxCPS canonical-default runs can self-calibrate independently in each event
pass. Raw files and `REXXCPS-EFFECTIVE` output remain authoritative.
`rexxcps-counter-normalization.csv` records each pass's effective count, and
`stat-summary.csv` scales RexxCPS absolute counters to 10 million executed
clauses. Within-pass IPC, miss rates and top-down percentages need no scaling.
This avoids either comparing unequal work or replacing the canonical command
with a fixed diagnostic argument.

## Sample method

Richards, Base64, Towers and RexxCPS were sampled once under both VMs and
compilers:

```bash
perf record -e cycles:u -F 199 -o "$CELL/perf.data" -- "${ARGV[@]}"
perf report --stdio -i "$CELL/perf.data" --no-children -g none \
  --percent-limit 0.1 --sort comm,dso,symbol
```

Fifteen captures are compact flat cycle profiles. GCC Richards `rxvm` retains
one deeper `--call-graph dwarf,8192` capture: 5,026 samples in 40.389 MiB.
All 16 captures passed their benchmark result and report zero lost samples.
The canonical RexxCPS profiles contain 242 to 590 samples; percentages are
diagnostic attribution, not formal timing or precise cross-compiler deltas.

Perf 7.0.12 on this host has three retained analysis limitations:

1. DWARF `perf report` spends minutes unwinding without completing.
2. LBR recording succeeds, but both `perf report` and `perf script` time out
   while decoding even a 19 KiB pilot.
3. `perf annotate` stalls for flat and DWARF inputs.

The valid DWARF data is retained for possible offline unwinding. Flat reports
use `-g none`, which processes it in about 0.11 seconds. Focused instruction
attribution uses:

```bash
perf report --stdio -i "$CELL/perf.data" --no-children -g none \
  -S "$SYMBOL" --percent-limit 0 --field-separator ';' \
  --fields overhead,sample,symbol,symoff --sort symbol,symoff
objdump -d -M intel --disassemble="$SYMBOL" "$EXACT_BINARY"
```

This yields `hot-offsets.csv`, `hot-instructions.csv` and retained exact-binary
disassemblies without rerunning a workload.

## Files

| Path | Meaning |
| --- | --- |
| `stat/gcc/` | 14 primary cells, seven event passes each |
| `stat/clang/` | 14 bounded compiler-control cells, four passes each |
| `stat-summary.csv` | counts and ratios for all 28 cells; RexxCPS totals normalized per 10 million clauses |
| `rexxcps-counter-normalization.csv` | retained effective-count and scale provenance for every RexxCPS event pass |
| `compiler-comparison.csv` | unmatched-session compiler-direction control; not a verdict |
| `record/gcc/`, `record/clang/` | 16 focus `perf.data` files and symbol reports |
| `hot-symbols.csv` | top ten available symbols from every focus profile |
| `hot-offsets.csv` | top sampled offsets in selected symbols |
| `hot-instructions.csv` | hot offsets mapped to exact disassembly |
| `annotations/` | full offset reports and symbol disassemblies |
| `provenance/` | host preflight and product identities |

Raw stat files and `perf.data` are authoritative. Derived CSVs do not convert
this diagnostic capture into formal timing or a compiler selection.
