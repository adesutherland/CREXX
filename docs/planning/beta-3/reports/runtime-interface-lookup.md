# Runtime Interface Lookup And Late Loading

Date: 2026-07-11

## Scope

This beta 3 foundation slice closes the current Release 1 core work for
interface method/factory lookup and late-loaded provider rebinding. It also
adds an observational benchmark so performance claims are tied to a repeatable
workload rather than the registry shape alone.

## Runtime Design

The VM rebuilds interface registries when linking or late module loading marks
them dirty. A rebuild now:

- sorts method rows by concrete class FQN and callable descriptor;
- sorts factory rows by interface FQN, factory member, class FQN, and
  descriptor;
- parses and retains each provider's callable signature once.

An exact method call performs a binary search and returns the resolved
`proc_runtime *` without allocating or parsing a descriptor. The existing
direct `class.member` fallback remains available and performs signature
validation only after an indexed miss.

Factory selection parses the requested descriptor once, binary-searches the
first row for its interface/member bucket, and considers only providers in that
bucket. Match scoring and deterministic class-name tie breaking are unchanged.

## Explicit Late Loading

The regression fixture separates an interface contract, provider, and host
into three modules. The host receives the exact provider filename and calls
`loadmodule()` before exercising:

- the default interface factory;
- a named interface factory;
- a concrete method;
- a default interface method.

The fixture runs under `rxvm`, `rxbvm`, and the `crexx` driver. For the driver,
the contract is supplied as an exact `-l` filename. A `-l` argument containing
a directory component is now used exactly; only a bare packaged name is
resolved below `CREXX_HOME/bin`. This preserves compatibility while allowing a
host to avoid broad library-path searches.

## Measurement

`tests/performance/runtime_interface_lookup_compare.crexx` creates 24 unrelated
interface/factory groups, then performs 50,000 method calls and 5,000 factory
calls against the last group. The table below records five-run medians from the
Darwin ARM64 Release build. Optimized timings use the normal linked-runtime
test path.

| Mode and operation | Before slice (us) | Indexed/cached (us) | Change |
| --- | ---: | ---: | ---: |
| No-opt method | 9,060 | 2,713 | 70.1% faster |
| No-opt factory | 84,642 | 77,669 | 8.2% faster |
| Optimized method | 9,135 | 2,579 | 71.8% faster |
| Optimized factory | 6,992 | 5,691 | 18.6% faster |

Method dispatch benefits directly from avoiding both the linear registry scan
and descriptor parsing. Factory lookup now scales with the matching provider
bucket, but construction, calls, and match execution dominate the no-opt
workload at 24 groups. The benchmark therefore demonstrates bounded selection
work; it does not claim that object construction itself is inexpensive.

These results are component evidence only. The central beta 3 performance
baseline remains tracked by GitHub issue #623.

## Coverage

Focused coverage includes exact dispatch, default methods, named/default
factories, match scoring, deterministic provider ties, both VM variants, the
driver's explicit library path, and late registry rebuilding. Full Debug CTest
is the final integration gate for this slice.
