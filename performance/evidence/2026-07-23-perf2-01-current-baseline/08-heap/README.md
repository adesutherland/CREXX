# Targeted allocation and lifetime lane

Apple `heap`, `vmmap` and `malloc_history` captures were made against the
uninstrumented profiling-off Release rxvm with `MallocStackLogging` for Bounce,
Richards, Storage and Towers. Towers was added because the portfolio count
profile identified it as the new allocation outlier. Captures are scaled
diagnostics; formal peak RSS remains in `../04-lifecycle-rss/`.

| Workload | Heap high water | Cumulative heap/VM allocation | Alloc/free operations | Dominant evidence |
| --- | ---: | ---: | ---: | --- |
| Bounce | 9 MB | 14.9 MB | 9,411 / 6,060 | retained high water is chiefly loader material; value-copy allocations remain visible by count |
| Richards | 9.2 MB | 35.6 MB | 47,497 / 42,788 | copy-value stacks; low retained lifetime relative to cumulative traffic |
| Storage | 122.8 MB | 374.8 MB | 688,429 / 475,568 | 117 MB and 219,840 recursive operations under `copy_value` at high water |
| Towers | 9 MB | 260 MB | 481,235 / 473,841 | high churn with copy/clear/reset and allocator stacks, low retained lifetime |

The initial Bounce `-lite` high-water invocation failed with exit 249 because
that mode does not retain the required record. It is retained as rejected
diagnostic evidence. `bounce-full.*` is the successful replacement; no failed
output is treated as evidence.
