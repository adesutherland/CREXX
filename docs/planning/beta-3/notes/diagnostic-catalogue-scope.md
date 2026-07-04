# Diagnostic Catalogue Completeness Scope

Status: working note for beta 3 diagnostic localisation follow-up.

The current `rxc_diagnostic_catalogs` CTest is a compiler-scope guard. It:

- compiles and runs `tools/check_diagnostic_catalogs.crexx` with the built
  `rxc`, `rxas`, and `rxvm`;
- verifies base catalogue coverage for diagnostic codes emitted from compiler
  sources and compiler exits;
- verifies key and placeholder parity for complete catalogues;
- treats `en_US` as an override catalogue whose entries must exist in `en_GB`.

`rxpp_diagnostic_catalogs` now applies the same catalogue parity rules to the
first-class preprocessor source. This is still not a complete toolchain
diagnostic audit.

## Known Out Of Scope Producers

Future localisation work must still extend catalogue coverage beyond `rxc` and
RXPP:

- Debugger diagnostics and command errors.
- VM/runtime diagnostics that are surfaced to users.
- Signals and condition names that originate in the Rexx runtime library and
  are later reported by the VM or driver.

The initial VM/library scope should stay narrow: cover user-facing unhandled
signal and condition names first. Do not sweep every internal VM status string
into the message catalogue until it is routed through the common diagnostic
emitter as a structured code with parameters.

## Extension Shape

Keep the catalogue parity rules common, but split producer-specific discovery
when source formats differ:

- `rxc_diagnostic_catalogs`: compiler C, parser grammar, and compiler exits.
- `rxpp_diagnostic_catalogs`: preprocessor Rexx sources that emit structured
  `RXPP_*` diagnostics through `rxpp_diag`.
- `rxdb_diagnostic_catalogs`: debugger command and runtime diagnostics.
- `rxvm_signal_catalogs`: VM-visible unhandled signals and runtime condition
  names.

These can be separate CTests and, if the scanner rules diverge, separate thin
Level B scripts that share or copy the small catalogue parity routines. The
important boundary is that each test should describe the producer it covers and
avoid false coverage claims for sources it does not scan.

If a producer constructs message codes dynamically, add an explicit manifest for
that producer rather than relying on fragile string scanning. The checker should
then validate both the manifest codes and any statically discoverable codes
against the same locale catalogues.

## Acceptance Rule

A catalogue completeness test should fail when:

- a structured user-facing diagnostic code can be emitted but is missing from
  the base catalogue;
- a complete locale is missing a base key;
- a complete locale contains a key not present in the base catalogue;
- a translation changes the placeholder set for a key;
- an override catalogue key does not exist in the base catalogue.

It should not fail merely because a source file contains ordinary prose, debug
strings, trace labels, environment names, or internal status text that is not
emitted through the structured diagnostic path.
