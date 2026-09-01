# PERF3-11 K02/K03 linked-read proof migration

Status: **K02/K03 complete; output-neutral canonical verdict**

K02/K03 replace twelve syntax-expanded duplicate linked-read rules with one
immutable storage/component/path proof and atomic rewrite. Direct `LINK` reads
name the source `StorageId`; immediate one-based `LINKATTR1` reads name an
interned path keyed by owner storage, exact attribute-count `ValueId`,
reference-effect generation and slot. All six legacy copy families are
covered: `COPY`, `BCOPY`, `ICOPY`, `SCOPY`, `FCOPY` and `DCOPY`.

The candidate link/copy/unlink triple is removed only after the proof has
validated both triples, the complete typed-copy component mask, detached-value
equivalence, cursor state and (for `LINKATTR1`) an exact in-range slot. Owner
aliases can therefore prove by storage identity. Different owners/slots,
changed count/source/detached value/cursor, aliased calls, divergent phis and
unproved signal paths reject. The former twelve keyhole rules are deleted.

## Cross-consumer regressions caught before closeout

The first canonical comparison exposed an integration regression: newly
indexed writes were represented as opaque observations, so K04 treated them
as reads and disabled all fourteen accepted RexxCPS compare/branch fusions.
The first broad Debug gate exposed the same boundary in M05/M06: explicit pure
writes with no `ValueId` were mistaken for unknown reads and disabled four
retained optimizer expectations. The proof service now distinguishes
observations, read/write uses and pure explicit/opaque/cursor writes once for
all consumers. Unknown writes remain conservative component-proof barriers
without claiming to read an overwritten `ValueId`. The combined 28-test proof
panel guards K02/K03, K04, M04, M05 and M06 in both Debug and Release; the
corrected broad Debug gate passes 2,010/2,010.

## Verdict

The original string/binary and different-slot focused images remain
byte-identical to frozen K04. The relevant metadata and TRACE cases are
stronger safe acceptances: their events continue to read the unchanged first
detached value while the redundant second executable read is removed.

The final ordinary profiling-off Release assembler accepts no K02/K03
candidate in canonical Richards, Towers or RexxCPS. All three images are
byte-identical to frozen K04, so no end-to-end runtime timing is warranted.
This is an output-neutral migration/consolidation verdict with additional
focused capability.

## Environment and boundary

- source HEAD: `e1bc5fda1d41a0a95360e9a89c37b0ee8bee0430`;
- branch: `codex/perf3-rxas-flow-infrastructure`;
- worktree: dirty by the accepted uncommitted PERF3-11 K04 and K02/K03
  production, test, documentation and evidence scope; no push;
- host: Darwin 25.5.0 arm64, Apple M5, 10 logical CPUs;
- toolchain: Apple Clang 21.0.0, CMake 4.3.2, Ninja 1.13.2;
- builds: ordinary profiling-off Debug and Release; and
- Release `rxas` SHA-256:
  `11ea6e9b223aaf531397696ee316e0c6973bd38c8b4b5c78fbfcd5922dcf3b62`.

The retained claims are exact correctness, structural and byte-comparison
observations. They make no runtime-performance claim because representative
ordinary output does not change.
