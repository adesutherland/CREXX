# PERF2-10/11 Intel Linux baseline and attribution

Status: **checksum-closed initial Linux baseline and attribution; no tuning
candidate selected**

This bundle retains the approved native Intel x86-64 Linux GCC/Clang
correctness, sanitizer, formal cross-runtime baseline and attribution work at
base source `d5f0827ca2708eae9d9be182c6d0d53bd6229b74`. The detached execution
tree started clean and now carries only the sanitizer correctness corrections
listed below. Its retained patch and post-correction hashes are authoritative
for all formal Linux measurements.

The control plane is
[`PERF2-10-11-INTEL-LINUX-WORKLIST.md`](../../PERF2-10-11-INTEL-LINUX-WORKLIST.md).
The retained 2026-07-27 Mac closure is a cross-platform orientation and guard,
not an absolute timing baseline for this host.

## Scope boundary

- ordinary profiling-off GCC and Clang Release products;
- full compiler-matrix correctness;
- normal Debug, supported GCC ASan/LSan and exploratory Clang UBSan;
- same-session cREXX, ooRexx, NetRexx and Regina timing/RSS/lifecycle evidence;
- schema-5 cREXX profiling and native Intel PMU attribution; and
- no `PERF2-06-D01`, LTO/PGO/layout experiment, product change, architecture
  selection or final external claim.

Raw evidence remains authoritative. The consolidated result and interpretation
boundary are in [`SCORECARD.md`](SCORECARD.md); native PMU methodology and tool
limitations are in [`native-perf/README.md`](native-perf/README.md).

## Sanitizer findings

The supported GCC ASan/LSan pass found a 10,000-byte successful-return leak in
`rxtcp.tcpreceive`. Exploratory Clang UBSan then found two Lemon C-contract
violations, null zero-length `memcpy` arguments in empty-string constant
folding, and signed left-shift overflow in bundled decNumber power scans.

The resulting source delta is correctness-only:

- transfer the TCP receive result and release the temporary buffer;
- avoid Lemon's null zero-element `qsort` and use the merge-sort comparator's
  actual function type;
- skip empty concatenation copies and handle allocation failure; and
- perform the decNumber bit scans in `uInt`;
- preserve NaN before integer exponent conversion in diagnostic `db_decimal`;
- keep Level B `x2d` within signed range while constructing negative 64-bit
  results, without changing the `ishl` instruction contract;
- write the linked-list back fence without an unaligned integer access; and
- route empty `bintos` validation through the VM's existing bounded UTF helper.

The exact eight-file patch is `provenance/source-corrections.patch`, SHA-256
`258d2a3f93fdb98ada849640291bc1eddc5af49298998ccb928349a89fa6f5fc`.
Focused normal-Debug and sanitizer reproductions pass. Final broad results:

| Product | Result | Elapsed |
| --- | ---: | ---: |
| GCC Debug | 1,925/1,925 | 771.36 s |
| Clang Debug | 1,925/1,925 | 727.77 s |
| GCC Release | 1,925/1,925 | 200.31 s |
| Clang Release | 1,925/1,925 | 195.36 s |
| Clang UBSan, `halt_on_error=1` | 1,925/1,925 | 868.47 s |
| GCC ASan/LSan, leak-enabled runner | 1,925/1,925 | 1,301.74 s |

The refreshed profiling-off Release builds are the only formal timing
authorities. GCC's incremental Release refresh took 1,669 seconds and reached
memory pressure while compiling four optimized VM variants; Clang took
478 seconds. These are build observations, not runtime results.

The optimized profiling builds are now complete and frozen. The interrupted
two-job GCC attempt caused a kernel OOM kill while one `cc1` process was at
11.3 GiB RSS. Resuming the exact configured tree serially took 1:01:49 and
peaked at 15.95 GiB RSS. The serial Clang profile build took 31:31 and peaked
at 1.15 GiB RSS. Product hashes are retained under `provenance/`; neither
profile product is a timing authority.

## Formal baseline

The final timing summaries combine each initial formal two-warmup/ten-sample
matrix with only the governed noise appends. The common-five geometric means
are:

| Compiler lane | VM | versus ooRexx | versus decimal NetRexx |
| --- | --- | ---: | ---: |
| GCC 15.2 | `rxvm` | 2.264305x | 0.703810x |
| GCC 15.2 | `rxbvm` | 2.198584x | 0.683381x |
| Clang 21.1 | `rxvm` | 2.548345x | 0.808085x |
| Clang 21.1 | `rxbvm` | 2.582703x | 0.818980x |

These are separate same-lane cREXX/reference scorecards, not a paired
GCC-versus-Clang experiment. Comparator medians drift materially between the
two sessions, so the apparent compiler delta is orientation only. Richards
remains the largest common deficit: cREXX is 0.342-0.351x ooRexx and
0.171-0.173x NetRexx across the four compiler/VM lanes. Base64 ranges from
0.710-0.876x ooRexx. Towers remains a separate qualified object/allocation row
at 0.462-0.478x ooRexx. RexxCPS remains a separately disclosed 2.2d rate row at
1.141-1.175x canonical ooRexx.

Native cREXX peak RSS is approximately 14.3-14.8 MB across the selected
workloads. JVM RSS rows remain labelled noisy after the governed appends.
Lifecycle phases remain separate from steady-state timing. The exact raw
samples, output strings, medians, MAD/noise flags and appends are retained
under `timing/`, `rss/` and `lifecycle/`.

## Attribution result

GCC schema-5 profiling covers all 11 qualified noopt/opt workload pairs in both
VMs. The bounded Clang control covers noopt/opt Sieve, Permute, Bounce,
Richards, Base64, Towers and RexxCPS in both VMs. All 72 cells are complete,
exact-hash, zero-overflow profiles with all required domains and RXSEQ
N=2/3/4. The RexxCPS counts-only Clang append uses its versioned bounded smoke
form and remains linked to the formal canonical-default result.

Native attribution uses only the ordinary profiling-off Release products:

- 98 GCC `perf stat` files cover seven workloads, both VMs and seven isolated
  event passes;
- 56 Clang files cover the same cells with four compiler-direction passes;
- all 154 files are 100% scheduled, correctness-passing and contain no
  unsupported or uncounted event; and
- 16 focused cycle profiles cover Richards, Base64, Towers and RexxCPS under
  both VMs and compilers with no lost samples.

RexxCPS native commands use the canonical default. Because it may
self-calibrate independently in each event pass, its absolute counters are
normalized to 10 million clauses using the retained `REXXCPS-EFFECTIVE`
record. Raw counts and the normalization table remain alongside the summary.

The retained evidence selects three pre-production investigation families:

| Workload | Schema-5 mechanism | Native mechanism |
| --- | --- | --- |
| Richards | 56.9M copy operations, 451.7 MB copied | `copy_value` is 55-57% of GCC and 77% of Clang samples; Clang also spends 16.5% in `maybe_trim_attribute_storage` |
| Towers | 26.8M copies, 31.3M clear/reset/destroy operations, 5.86 GB allocation requests | `copy_value`, `clear_value_contents` and storage reset dominate; GCC is 12-14% front-end bound with about 9% indirect-branch misses |
| Base64 | 46.7M VM instructions; `SCOPY_REG_REG` ranks third | 92-96% remains in `run`; GCC is about 51% back-end bound and individual libc conversion/string helpers are small |
| RexxCPS | optimized GCC `rxvm` records 6.11M VM instructions, 673,511 conversions and 330,174 allocation requests in the bounded smoke | canonical GCC `rxvm` is 42.0% front-end bound; `run` is 35-44% across the four profiles, followed by decimal parse/format/arithmetic and string movement |

Clang executables are much smaller than GCC (`rxvm` 1,197,872 versus
3,514,896 bytes; `rxbvm` 1,181,488 versus 3,448,240 bytes), but compiler
direction reverses by workload. No compiler, layout or value representation
has been selected.

## Sufficiency verdict

The Linux x86-64 host has now supplied the required correctness, sanitizer,
formal baseline, ELF identity, schema-5 and Intel PMU evidence. Expanding this
campaign is less efficient than returning to the faster macOS host. The next
step is outside-tree mechanism ceilings and PoC ranking for copy/value,
attribute-storage teardown/trim and Base64 dispatch/string shapes. Any
production edit must be batched and receive the mandatory first ordinary
Release verdict on macOS before this Linux host is used again for selected
GCC/Clang PMU, sanitizer or final platform validation.

This is not Gate E completion. Supported Linux ARM64, same-machine Windows, a
selected candidate and the final default-VM/cross-platform decision remain
open.

## Directory map

| Directory | Content |
| --- | --- |
| `provenance/` | Git, host, power, kernel, compiler and comparator freeze |
| `manifests/` | Exact GCC and Clang formal matrix manifests |
| `logs/` | Configure, build, CTest and sanitizer logs |
| `timing/` | Formal two-warmup/ten-sample serial timing matrices |
| `rss/` | Independent zero-warmup/three-sample peak-RSS matrices |
| `lifecycle/` | Compile/assemble/translate/load-to-first-result evidence |
| `artifacts/` | Product, workload and generated-runtime size/hash inventory |
| `profiles/` | Schema-5 count diagnostic profiles and mechanism summaries |
| `native-perf/` | Ordinary-Release PMU counters, samples, symbol reports and hot-offset disassembly |
| `pilots/` | Qualified and failed calibration evidence, including profiler portability limits |
| `checksums.sha256` | Recursive bundle checksum closure |
