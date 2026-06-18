# Attribute Link Lifetime Investigation

Status: temporary design investigation. Delete this note when the approach is
accepted into permanent architecture docs or rejected.

## Question

`RexxValue` exposed a wider issue in the compiler's attribute access model.
Normal class attributes are currently linked for each access and then unlinked
through expression cleanup. Complex attributes, such as `register.0` views and
multiple typed views over the same physical `register.N` slot, are made safe by
linking to a scratch register, copying the requested view and status flags to a
local, and unlinking before ordinary expression code runs.

That is safe but expensive, and it is probably too conservative for the
RexxValue usage pattern. The VM register fields for `.string`, `.binary`,
`.int`, `.float`, `.decimal`, and the library/user status flag bands are
independent storage inside one value. RexxValue cache coherency is therefore a
RexxValue library responsibility: the class must set or clear its cache flags
when it writes a representation. The compiler should not assume that writing
one typed field invalidates another typed field.

The investigation is whether the compiler should instead analyse each callable,
assign stable local registers for attributes used by that callable, and use
direct aliases for safe cases while reserving detached copies only for cases
where compiler-generated expression code may mutate or expose the value in a
way that would break RexxValue's cache protocol.

## Current Facts To Verify

- Ordinary function, method, and factory calls marshal arguments with `swap`.
  Argument expressions are evaluated first; the emitter then swaps actuals into
  the call frame, emits `call`/`dcall`, and swaps the registers back.
- Call marshalling should not use `link`. Link-style aliasing should remain
  restricted to compiler-owned locator operations: class attributes, arrays,
  argument indexing, and deliberate low-level assembler.
- The inliner already treats raw aliasing assembler such as `link`,
  `linkattr*`, `linktoattr*`, and `unlink` as a fail-closed gate.
- Attribute reads currently use short-lived links. Non-complex attribute reads
  link the result register to the attribute and attach an `unlink` cleanup.
  Complex reads link a scratch register, copy the selected typed view plus
  status flags into the expression result, and unlink the scratch immediately.
- Attribute writes currently link the attribute, copy the RHS into it, and
  unlink. Complex writes also copy status flags back.
- The typed fields within one VM value are independent. Directly reading or
  writing the `.int` view does not inherently corrupt the `.string`,
  `.binary`, `.float`, or `.decimal` fields. The validity of those fields is
  represented by library/user flags that RexxValue code owns.
- Compiler-generated promotion/conversion instructions are the main hazard.
  Operations such as `itos`, `dtos`, `ftos`, `btos`, `stobin`, and `bintos`
  can materialize another representation in the destination register. If the
  destination is a direct alias of a complex attribute, the compiler may mutate
  cache-relevant storage without the RexxValue method deliberately updating its
  flags.
- VM `RET_REG` already copies into the caller's return register when the source
  might be an argument, object attribute, global, or linked/swapped register.
  The investigation should verify whether this fully handles return detachment
  for linked attributes, or whether the compiler still needs an upfront copy in
  some cases.
- `RETURN` currently emits `ret` after the child expression but does not have a
  general "run cleanup before returning" model. Existing code has a TODO around
  returning linked array elements. This is a direct hazard for any longer-lived
  attribute-link plan.

## Proposed Safety Principles

1. Calls use `swap`, not `link`.

   The compiler may use `link` only for storage-locator access it owns and can
   pair safely: attributes, array elements, argument indexing, and equivalent
   internal locator constructs. Ordinary call setup and return setup must not
   introduce new links.

2. Every compiler-created link has a declared lifetime.

   A link is either short-lived for one expression and unlinked by cleanup, or
   method-lived and tied to a callable prologue/exit protocol. There should be
   no third category of ad hoc links that rely on a later node happening to
   clean up correctly.

3. Method-lived normal attribute links are aliases.

   For normal attributes, the prologue can link a reserved local register to
   the receiver attribute once. Reads and writes then use that local directly.
   No copyback is needed for ordinary writes because the local aliases the
   attribute storage.

4. Complex attributes are not automatically copied.

   Direct reads and same-view writes to one field of a complex value are
   normally safe because the VM fields are independent and the library owns
   cache flag updates. A complex attribute needs a detached local only when the
   compiler will use it in an expression context that may promote, convert,
   pass, return, or otherwise expose the value outside the simple field access.

5. Detached complex locals are a fallback safety shape.

   When the checker decides direct access is unsafe, the compiler can copy the
   selected view into an ordinary local either in the prologue or around the
   unsafe expression. That local is not linked after the copy. If the local is
   written or materialized, all callable exits or expression writeback points
   must copy the selected view and any library-managed status flags back to the
   physical attribute slot.

6. Returning an aliased or complex attribute value must return a value, not an
   alias.

   Returning a value must not return a live alias to an attribute or array
   element unless the language explicitly asks for a reference. The open
   question is whether VM `RET_REG` already provides this detachment for all
   linked attribute cases. If it does, the compiler should rely on that instead
   of inserting an eager copy. If it does not, the compiler needs a return
   helper that copies the value into the real return register before `ret`.

7. Copyback is an exit operation, not a cleanup accident.

   Complex attribute copyback must be emitted before every callable exit:
   explicit `RETURN`, fallthrough return, `EXIT` where applicable, generated
   `LEAVE_WITH` return rewrites, and any later signal/control-flow exit that
   leaves the callable. A single helper should own this emission.

8. Compiler type materialization counts as mutation.

   A source read can still become a write when the compiler promotes or
   converts it. If a complex local is type-converted or materialized by
   compiler-generated expression code, the local must be considered dirty or
   detached unless the method explicitly owns the flag update.

9. Inlining should see a normal AST, not special emitted links.

   The attribute-local analysis should run after the inlining fixed point for
   normal callables. Raw aliasing assembler stays a fail-closed inline gate
   until the inliner has an operand-effect model for links.

## Proposed Attribute Access Plan

Add a callable-level analysis pass after the main validation/inlining work and
before final register assignment/emission. For every method or factory, build
an attribute access plan:

```text
AttributeAccess
  symbol
  physical register index
  view
  local register
  kind: normal_alias | complex_direct | complex_detached | direct_flag_view
  read: yes/no
  written: yes/no
  materialized: yes/no
  used_as_argument: yes/no
  returned: yes/no
  needs_copy: yes/no
  needs_copyback: yes/no
  unsafe_reason
```

Normal attributes:

- If used, reserve a callable-lifetime local register.
- Link it in the callable prologue with `link` or `linkattr1`.
- Rewrite reads and writes inside the callable to use the local directly.
- If the value is returned, ensure the caller receives a detached value. This
  can be VM `RET_REG` if verified, or a compiler copy before `ret` otherwise.

Complex attributes:

- If every use is a simple same-view read/write, prefer direct access. Direct
  means either a method-lived alias local or a short-lived link, but no
  detached copy.
- If expression lowering may promote, convert, pass, return, or otherwise
  expose the value, use a detached local unless a narrower proof says direct
  access is still safe.
- For detached locals, link a scratch register to the physical attribute in the
  prologue, copy the selected typed view to the local, then unlink the scratch.
- Mark the detached local dirty when it is assigned, passed where mutation may
  occur, or type/materialization code may alter representation fields.
- On every exit, copy dirty detached locals back to the physical attribute slot.
  Library/user cache flag updates remain explicit RexxValue code, not compiler
  magic.

Flag views:

- Keep the current direct masked read/write lowering. Flag views are not typed
  payload aliases and should not participate in complex payload copy logic.

## Complex Direct-Access Checker

This checker decides when a complex attribute needs a detached copy. It must
not assume all complex attributes require copying.

Direct access should be allowed for these cases:

- simple same-view reads;
- simple same-view writes where RexxValue code explicitly updates the relevant
  library/user flags;
- direct flag-view reads/writes through `.flags.<partition>`;
- explicit reviewed assembler in a RexxValue method when the method owns the
  flag protocol;
- same-view copy where no promotion, conversion, call, return, operator, or
  unknown assembler effect is involved.

A detached copy should be required for these cases until a narrower proof is
implemented:

- use in a general expression tree;
- participation in an operator;
- implicit or explicit compiler promotion/conversion;
- passing as an argument, unless the callee/formal proof says the value cannot
  be mutated and call marshalling cannot disturb the alias;
- return, unless VM `RET_REG` is proven to detach all relevant linked cases;
- unknown inline assembler operand effects;
- any case the checker cannot classify.

Detached copyback is required when:

- the source writes the local;
- type conversion or materialization can change representation fields;
- the local is passed by reference or to an unknown mutating operation;
- the checker cannot prove the local stayed unchanged.

## Link And Swap Coexistence Hazards

The investigation must prove these cases before changing default emission:

- A method-lived linked attribute local survives a child call because the child
  call uses `swap` only and never unlinks caller locals.
- If a linked attribute local is passed as a by-value argument, the callee still
  observes pass-by-value semantics. Large writable by-value formals must copy
  rather than mutate the caller attribute.
- If a linked attribute local is passed by `.ref`/`expose`, mutation of the
  caller attribute is intentional and remains ordered correctly.
- Call marshalling must not allocate or swap through registers reserved for
  method-lived attribute links unless the swap/restore proof explicitly allows
  it.
- `RETURN attr`, `RETURN array[i]`, and `RETURN complex_attr` return detached
  values, not live aliases.
- Early returns and generated inlined `LEAVE_WITH` rewrites run required
  complex copybacks.
- Raw aliasing assembler inside the callable disables this optimization unless
  the instruction effect is known and modelled.

## Register Assignment TODO

This work needs a register-assignment review, not just an emitter rewrite.

`rxcp_emit_reg.c` currently assigns node temporaries and short-lived additional
registers. Method-lived attribute locals need a reserved-register class whose
lifetime is the whole callable. The register allocator must:

- reserve those locals before assigning expression temporaries;
- prevent call-frame additional registers from clobbering reserved locals;
- account for inlined bodies that increase pressure;
- allow later liveness work to reuse attribute locals only when the callable
  plan proves non-overlap;
- expose enough debug output to explain why a callable's register count grew.

This should be tracked as an explicit compiler TODO because it also relates to
the existing examples where inlining creates excessive register pressure.

## Paper Exercises

The useful comparison is not just "linked attribute versus copied attribute".
For RexxValue there are at least three shapes:

- expression-scoped complex copy: safest today, but repeats link/copy/unlink
  work and is likely too expensive in hot methods;
- method-lived detached local: removes repeated links, but still pays prologue
  copy and exit copyback, even when the method only checks flags or writes one
  view deliberately;
- direct complex access: best for simple field and flag operations, provided
  compiler-generated conversion does not target the physical complex attribute.

### Flag Check Of Two Values

Typical operator flow:

```text
left_flags = left.flags.library
right_flags = right.flags.library
decide promotion from the masks
```

The compiler should use direct flag views here. No payload view is copied, and
no cache invalidation happens. This is already close to the intended model:
masked flag reads are ordinary integer operations over the status field.

Performance conclusion: detached complex locals do not help this case. The win
is keeping flag access direct and avoiding any payload copy around the checks.

### Simple Representation Setter

Typical setter flow:

```text
self.value.int = new_int
self.value.flags.library = (old_flags & keep_mask) | new_int_valid_mask
```

Because register views are independent, writing `.int` does not inherently
corrupt `.string`, `.binary`, `.float`, or `.decimal`. RexxValue is responsible
for publishing the intended cache state by updating its flags. The compiler
should not insert a detached copy for this same-view write.

Performance conclusion: direct complex access is the right default for explicit
RexxValue setter methods. A detached local adds work and can obscure the flag
protocol that maintainers need to see.

### Fast Materializer Hit

Typical `asString()` hit:

```text
if flags say string is current:
  return self.value.string
```

There is no conversion and no reason to copy the whole complex value before the
read. The only open return question is whether `RET_REG` detaches a linked
attribute source in every relevant case. If it does, the method can return the
direct view and rely on the VM. If it does not, the compiler needs a return
helper that copies the returned view immediately before `ret`.

Performance conclusion: direct read is the desired body shape. The
investigation must add a focused return-alias test before deciding whether an
upfront return copy is needed.

### Materializer Miss

Typical `asString()` miss:

```text
if int is current:
  temp = itos(self.value.int)
  self.value.string = temp
  self.value.flags.library = updated_string_mask
  return self.value.string
```

The conversion instruction is the hazard, not the source read. If the compiler
emits `itos` with the physical complex attribute as the destination, it may
materialize `.string` without the RexxValue method deliberately updating flags.
The method should instead convert into an explicit temporary, then assign the
string view and flags as visible RexxValue operations.

Performance conclusion: direct read of the source representation is fine. The
compiler must keep conversion destinations away from the physical complex
attribute unless the method deliberately requested that write.

### Binary Text Detection

Typical binary-to-text validation flow:

```text
if binary is current:
  temp_string = bintos(self.value.binary) or validation helper
  if temp_string is valid text:
    self.value.string = temp_string
    self.value.flags.library = updated_binary_and_text_masks
```

The text/binary distinction is a RexxValue semantic decision. A binary payload
may later be recognized as valid text and marked as such. The compiler should
not infer validity or invalidate one view when another view is written.

Performance conclusion: direct binary reads are safe; conversion or validation
results should land in temporaries first, then be committed to the complex
attribute by explicit RexxValue code.

### Add Operator

Typical `add(left, right, result)` flow:

```text
left_flags = left.flags.library
right_flags = right.flags.library
choose int, decimal, float, or string/numeric promotion
promote into temps only where needed
perform the add on temps or already-current direct views
result.value.<view> = operator result
result.value.flags.library = valid mask for the result view
```

The inputs should not be copied merely because their fields are complex. The
method first reads flags, then only promotes the representations required for
the selected operation. Promotion destinations should be ordinary temporaries.
The result write is an explicit same-view write to the result RexxValue followed
by explicit flag publication.

Performance conclusion: this is the main reason to avoid blanket complex
copies. Copying both operands before flag checks wastes work. The useful copies
are the promotion temporaries that the operation actually needs.

### Argument Passing And Child Calls

Passing a direct complex attribute as a general by-value argument is less clear
than reading or writing one view locally. Until callee/formal effects are
modelled, argument expressions should detach unless the compiler can prove the
call cannot mutate or retain the alias. Deliberate `.ref` or expose-style
aliasing must remain explicit in the source and visible in the plan.

Performance conclusion: be conservative at call boundaries. Avoiding payload
copies inside RexxValue hot methods matters, but accidental alias escape would
be a semantic bug.

### Returning A Complex View

There are two possible designs:

- rely on VM `RET_REG` if focused tests prove that returning a linked
  attribute, array element, or complex view always copies into the caller's
  return register;
- insert an upfront compiler copy for return expressions that are aliases.

The VM route is likely cheaper and appears to match the current interpreter
shape, but it must be proven with tests. The compiler-copy route is safer as a
fallback, but it may undo much of the direct-access win for simple materializer
hits.

Performance conclusion: return handling is a separate decision from the
complex-copy checker. The first prototype should measure both if `RET_REG`
semantics are not already conclusive.

### Initial Rule From The Exercises

The starting rule should be:

- use direct access for simple same-view field reads/writes and flag views;
- use explicit temporaries inside RexxValue methods for promotions and
  conversions;
- detach for general expression use, argument passing, unknown assembler
  effects, and returns until the return tests prove VM detachment is enough;
- keep all cache-validity changes in RexxValue code, not compiler side effects.

## Exploration Plan

1. Characterize current emission.

   Capture RXAS for `class_attr_register_views`, `testRexxValue`, and one or
   two high-register inlining examples. Add small tests for returning an
   attribute, returning an array element, returning a `register.0` view, and
   linked attribute use across child calls if gaps exist. These tests should
   determine whether VM `RET_REG` already detaches linked return sources.

2. Add analysis-only instrumentation.

   Build the `AttributeAccess` plan and dump it under a debug flag, without
   changing emitted code. Validate that the plan identifies normal attributes,
   complex attributes, flag views, reads, writes, returns, and call arguments.

3. Prototype direct complex access behind a guard.

   Start with the RexxValue-style use cases where the checker can classify
   simple same-view reads/writes and flag views. Confirm that compiler
   promotion/conversion instructions use explicit temporaries and do not target
   physical complex attributes by accident.

4. Prototype normal attribute prologue links behind a guard.

   Rewrite only ordinary scalar attributes first. Keep complex attributes on
   the existing short-lived-safe path. Prove noopt/opt behaviour with focused
   tests.

5. Centralize callable exit emission.

   Add a helper that can emit return-value detachment and pending copybacks
   before every callable exit. Use it initially with no behavior change, then
   turn on complex copyback.

6. Prototype complex detached locals only for unsafe cases.

   Use detached locals for general expression use, promotion/conversion
   hazards, argument passing without a callee proof, unknown assembler effects,
   and returns if `RET_REG` is not sufficient. Copy the view in the prologue or
   expression setup, use local registers in the body, and copy back only when
   dirty exits require it.

7. Add the conservative direct-access checker.

   Keep the initial rule simple: direct only when the use is trivially safe,
   otherwise detach. Measure RXAS and runtime changes before broadening the
   proof.

8. Revisit inlining.

   Confirm the analysis runs after local inlining and that imported inline
   payloads still reject raw aliasing assembler. Add tests where an inlined
   method body reads caller attributes before and after the rewrite.

9. Decide.

   Accept the approach only if it improves RexxValue/operator RXAS materially
   without weakening return/call semantics. Otherwise delete this note and keep
   the current expression-scoped linking model plus narrower keyhole cleanup.

## QA Gate For Any Prototype

- Focused compiler tests for attribute views, register flag views, RexxValue,
  array/argument locator returns, and inlining.
- Noopt and opt runtime tests for child-call hazards.
- RXAS diff inspection showing fewer repeated links/copies in RexxValue hot
  methods.
- Full `ctest --test-dir cmake-build-debug --parallel $(nproc)
  --output-on-failure` before any checkpoint commit.
