# DECIMAL-01 RXAS numeric-context opening evidence

Status: **Gate 0 opening evidence; current defects reproduced; no fix made**

Date: 2026-08-05

## Scope and provenance

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch/source commit: `develop`,
  `4813e98d1dca1ac77d5899dd6c5787e4b83f4772`
- Upstream before DECIMAL-01 edits: `origin/develop` at the same commit
- Host: Darwin 25.5.0 ARM64, Apple M5, 10 logical CPUs
- Product: current Debug binaries; correctness execution only
- VMs: `rxvm` and `rxbvm`
- Providers: static/default `mc_decimal`, explicit dynamic `mc_decimal`, and
  explicit dynamic `db_decimal`
- Source diagnostic:
  `tests/performance/decimal/rxas_numeric_context_observable.rxas`
- Unrelated concurrent worktree file `performance/PERF3-13-WORKLIST.md` was not
  read, changed or included in this activity.

No elapsed time in this bundle is performance evidence. The host was not
reserved, and no performance cell was run.

## Existing focused baseline

The relevant binaries were rebuilt. The current focused selection passed 9/9
both before and after extending `interpreter/tests/tests_decimal.rxas` with
tests 59-66:

- compiler combined-context contract in optimized and no-opt modes;
- default, explicit `mc_decimal` and explicit `db_decimal` RXAS decimal tests
  under both VMs; and
- the `mc_decimal` and `db_decimal` provider suites.

The added direct coverage passes for:

- all ten immediate/register individual setter forms and five getters;
- register-source preservation;
- `NUMSCI` and `NUMENG` state readback and precision effect;
- combined/five-setter exponent-formatting equivalence; and
- invalid combined digits/case/standard signalling before partial mutation.

This establishes that the instructions store/read the requested state and that
the combined instructions operate like the expanded path for the tested
current provider behaviour. It does not prove that every provider consumes
every field correctly.

## Observable semantic diagnostic

The separately named diagnostic asserts the documented effects of `FUZZ`,
form, case and standard and checks nested frame restoration. Exact results:

| VM | Provider | Exit | Findings |
| --- | --- | ---: | --- |
| `rxvm` | default/static `mc_decimal` | 1 | `FUZZ 1` comparison failed |
| `rxbvm` | default/static `mc_decimal` | 1 | same |
| `rxvm` | explicit `mc_decimal` | 1 | same |
| `rxbvm` | explicit `mc_decimal` | 1 | same |
| `rxvm` | explicit `db_decimal` | 3 | `FUZZ`, engineering/lower formatting and Common half-even rounding failed |
| `rxbvm` | explicit `db_decimal` | 3 | same |

Passing parts of the same diagnostic were `FUZZ 0`, `mc_decimal`
scientific/engineering and case formatting, `mc_decimal` Common/Classic tie
rounding, Classic rounding under `db_decimal`, and child/caller context
isolation/restoration.

## Gate 0 disposition

1. **Individual and combined RXAS state installation:** preliminary
   `conformant-current` for the covered forms.
2. **`NUMERIC FUZZ` decimal comparison:**
   `confirmed-current-defect`. `SETNUMFUZ` stores and reports the value, but
   neither provider applies it to decimal comparison.
3. **`db_decimal` form/case consumption:** `confirmed-current-defect` unless
   Adrian chooses to formalize the provider as an explicitly limited
   diagnostic control. `NUMENG 6,1,1` formatted `1e20d` as uppercase
   scientific `1E+20`, not lowercase engineering `100e+18`.
4. **`db_decimal` Common rounding:** `confirmed-current-defect` unless
   formalized as a limitation. At five digits the exact halfway case rounded
   to `1.2345`, while Common half-even requires `1.2344`.
5. **VM parity:** current failures and passes are identical under `rxvm` and
   `rxbvm`; there is no evidence here of a VM-dispatch-specific defect.

No repair, semantic reinterpretation or baseline change is authorized. Gate 0
continues only with the remaining correctness inventory and must stop for
Adrian's defect/rebaseline decision before Gate 1.

## Artifact hashes

| Artifact | SHA-256 |
| --- | --- |
| expanded `interpreter/tests/tests_decimal.rxas` | `c719b696a924194884de38791a3c6cf1cef37551fcb4a54c16a3e26ed457f8a9` |
| diagnostic RXAS source | `1e4ed3f179570d1e2728a3688c095fc05030e9b95b3146759044e8aacbc7c256` |
| `rxas` | `ef2a42e6a1b357d03383b2ac79e73f827b7b65e7fe21b449b4a78896b8b1b87d` |
| `rxvm` | `d96e5481e298497c2100406fcc5340dd94f3a9e00a66e29f241906cfc40132db` |
| `rxbvm` | `479a2dc91e9a2a58f25dced3700ecc1ac7e7ceb73b15927a801fa9841c5f8a5d` |
| `library.rxbin` | `9a6d5ee6ce7fb77e0fa25cd9958668c91f0b4bff460ec673ba1ef47d1db2bb6c` |
| explicit `mc_decimal` plugin | `874a40743f494b3784b2f3b4770fda4e7b2963771b867af8c3a4fa59102d4d7c` |
| explicit `db_decimal` plugin | `09c772847392bb714b9051a51159b2027552d1ca861acd7e5488daef063f477b` |

Commands and exact stdout are retained beside this record.
