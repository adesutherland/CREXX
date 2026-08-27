# Current CMake Build Catalogue Summary

- Source commit: `dc44d92909e706adc575932ba9ae72f2d6f05b7d`
- Platform: `Darwin arm64`
- Configuration: `Debug`
- Status: observation-only Phase 0 export; not an executable replacement graph

## Inventory

| Item | Count |
| --- | ---: |
| targets | 1500 |
| custom targets | 1277 |
| custom commands | 1352 |
| declared outputs and byproducts | 2089 |
| cleanup operations | 438 |
| rxc import invocations | 1030 |
| tests | 2391 |
| fixtures setup | 31 |
| fixtures required | 937 |
| resource locks | 110 |
| run serial tests | 139 |
| nested build tests | 31 |
| artifact files | 2298 |

## Provisional product layers

| Layer | Targets/actions |
| --- | ---: |
| B0 | 703 |
| B1 | 324 |
| C | 325 |
| C0 | 62 |
| C1 | 416 |
| G | 102 |
| L | 10 |
| Optional | 1978 |
| Product | 2 |
| X | 207 |

## Provisional QA tiers

| Tier | Tests |
| --- | ---: |
| comprehensive | 2207 |
| essential | 12 |
| measurement | 24 |
| qualification | 3 |
| smoke | 138 |
| stress | 7 |

## Findings

| Code | Count |
| --- | ---: |
| cleanup-rewrites-own-output | 2 |
| cleanup-touches-other-output | 251 |
| custom-target-no-declared-byproducts | 3 |
| multiple-output-owners | 2 |
| test-has-no-labels | 995 |
| test-invokes-build | 31 |

## Manifest projection validation

- Schema valid: `true`
- Graph clean: `false`
- Schema errors: `0`
- Graph findings: `815`

Graph findings describe the existing CMake projection and are Phase 1 inputs;
they do not mean the Phase 0 exporter changed build behaviour.
