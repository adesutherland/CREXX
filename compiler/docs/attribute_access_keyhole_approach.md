# Attribute Access Keyhole Approach

Status: accepted implementation note for the current Level B/RexxValue slice.

The broader callable-lifetime attribute-link proposal was rejected. Attribute
access remains locally safe at the compiler emission point, and repeated
safe-access cleanup belongs in the RXAS keyhole optimiser.

## Principles

- Normal class attributes keep the existing short-lived `link`/`unlink` access
  model. The compiler may keep improving register assignment so repeated uses
  naturally reuse stable local register numbers, but it should not introduce
  method-lifetime aliases until calls, returns, inlining, and every exit path
  have a single proved protocol.
- Complex typed register views are copied by payload view only. A
  `register.0.string`, `register.0.int`, or duplicate typed view over a
  physical `register.N` slot links the physical value, emits the requested
  typed copy (`scopy`, `icopy`, `fcopy`, `dcopy`, `bcopy`, and so on), and then
  unlinks. The compiler must not use `acopy` as hidden RexxValue cache-flag
  maintenance for these views. A future explicit compiler flag-copy semantic
  would have to lower to `acopy`, but there is no current compiler use case.
- `RexxValue` owns its cache coherency. The VM value fields for string, binary,
  integer, float, decimal, object, and the public status-flag bands are
  independent storage. A library method that materializes or invalidates a
  representation must update its own library/user flags explicitly.
- Flag views are status-word views, not payload views. `.flags.vm`,
  `.flags.compiler`, and `.flags.readable` are read-only. Writable source views
  lower to `settpmask`, and the VM masks the requested write to the
  source-writable flag bands. Compiler-generated call setup may still update
  compiler-reserved flags through its own internal instructions.
- Calls use `swap` for call-frame marshalling. Compiler-created `link` remains
  reserved for storage locators such as attributes, arrays, argument indexing,
  and explicit low-level assembler.
- Returns rely on VM `RET_REG` detachment. The VM copies when the returned
  register is not a true local or is currently linked/swapped; otherwise it may
  move the expiring local. The array-element return TODO in
  `compiler/rxcp_emit_flow.c` should be kept covered by focused tests so this
  remains a proved property.

## Current Optimisation Shape

The compiler emits correct, direct sequences first. RXAS then removes local
redundancy when the queue hazard model proves it safe:

- `copy rA,rB` followed immediately by `acopy rA,rB` keeps the full `copy` and
  drops the redundant status-only copy because VM `copy_value()` already copies
  the value status word. The compiler should not emit this pair; the rule is a
  cleanup for hand-written or legacy RXAS, not hidden permission to attach
  `acopy` to typed payload copies.
- Duplicate linked reads over the same owner register may reuse the first
  detached copy:

  ```rxas
  link r1,r0
  scopy r2,r1
  unlink r1

  link r3,r0
  scopy r4,r3
  unlink r3
  ```

  can become:

  ```rxas
  link r1,r0
  scopy r2,r1
  unlink r1

  scopy r4,r2
  ```

- The same duplicate-read rule applies to `linkattr1` only when both the owner
  register and the one-based attribute slot match. Two child attributes on the
  same object are independent values and must not be merged.

The keyhole rules are intentionally conservative. Any barrier instruction or
intervening use of a register mapped by the rule blocks the rewrite. Broader
optimisation should be added as separate RXAS rules with focused noopt/opt
tests, not by making source emission depend on global callable analysis.

## Open Follow-Ups

- Add explicit register-assignment work so compiler-generated repeated
  attribute access tends to use stable locals that the keyhole optimiser can
  recognise.
- Keep the return-detachment array-element TODO under test, and remove or
  reword it once the compiler-side coverage is explicit.
- Revisit assembler-level hot paths in `RexxValue` methods after the first
  operator surface is stable. Hand-tuned methods may use direct views, but must
  leave library/user cache flags as explicit RexxValue operations.
