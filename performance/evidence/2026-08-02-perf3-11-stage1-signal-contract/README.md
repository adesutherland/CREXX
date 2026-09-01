# PERF3-11 Stage 1 signal contract and Gate 1 decision

Date: 2026-08-02

Status: **analysis complete; stopped at Gate 1 before VM or instruction-semantic
changes**

Base: `76ec7eb65` on `codex/perf3-rxas-flow-infrastructure`

## Result

The existing opcode `RXOP_SEM_MAY_THROW` flag is not a sound source of signal
CFG edges.  It mixes VM signals with opaque calls, allocation/failure risk and
fail-closed reserved opcodes, while omitting instructions that genuinely raise
VM signals.  PERF3-11 therefore needs a separate, aligned signal-contract
inventory.  Generic effects remain useful, but only the signal inventory may
create signal continuations.

The scalable design is viable without first changing VM semantics.  Unknown
contracts remain explicit and conservative.  Five bounded semantic choices are
offered at Gate 1; the recommended selection removes avoidable irregularities
before the graph and SSA layers make them permanent.

## Inventory closure

The X-macro effect inventory contains exactly 650 opcode entries:

| Effect state | Entries |
| --- | ---: |
| Classified | 585 |
| Conservative | 6 |
| Reserved | 56 |
| Internal | 3 |

Of these, 492 carry `RXOP_SEM_MAY_THROW`: 427 classified entries and all
fail-closed conservative/reserved/internal cases where applicable.  Only 158
classified instructions are currently described as non-throwing.

A lexical audit of the 579 `START_INSTRUCTION` implementations found 236
unique blocks containing a direct `SET_SIGNAL`, plugin-signal poll or shared
OOM-signal path.  This is a lower bound because helpers and macros can signal.
Even as a lower bound it proves both directions of the mismatch:

- 212 classified `MAY_THROW` entries have no direct signal token in their VM
  body.  Some signal through helpers, but many are simply over-conservative.
- 18 source instructions with direct signal paths are classified without
  `MAY_THROW`: `INC0`, `INC1`, `INC2`, `DEC0`, `DEC1`, `DEC2`, both forms of
  `SETNUMDGTS`, `SETNUMFUZ`, `SETNUMFRM`, `SETNUMCAS`, and `SETNUMSTD`, plus
  `NUMSCI` and `NUMENG`.

The other lexical mismatches are not classified-source omissions: `INULL` is
internal, `SPAWN` is already conservative, and `name` is a macro parameter.

The existing generic inventory also identifies 12 call opcodes, 2 dynamic-call
opcodes, 19 alias creators, 13 alias releasers, 1/4/1/2 reference
create/read/write/release opcodes, 44 indirect writers and 160 opaque opcodes.
Those categories become signal dependencies or clobber facts; they must not be
silently converted into signal edges.

## Proposed first-class signal contract

Add a dedicated opcode-aligned X-macro inventory, provisionally
`rxopsignals.h`, with exactly one row per opcode.  Keep it separate from
`rxopeffects.h`: signal control is independently reviewed and the current
generic flag demonstrably has a different meaning.

Each row declares:

- classification: `NONE`, `KNOWN`, or deliberate `UNKNOWN`;
- static signal set, dynamic signal name, or unknown set;
- failure visibility: `BEFORE_WRITES`, `AFTER_WRITES`, named
  `PARTIAL_WRITES`, or `UNKNOWN`;
- components, storage mappings, numeric context and other effect versions
  visible on normal, skip and retry edges;
- payload and interrupted-address observability;
- success refinement and the complete signal-relevant dependency set; and
- whether normal completion is success-stable for identical operand and
  dependency definition identities.

Compile-time/runtime metadata validation must prove table count and opcode
alignment.  Every source opcode must be `NONE`, `KNOWN`, or explicitly
`UNKNOWN`; there is no inferred default.  Reserved, internal, conservative and
unreviewed helper-signalling instructions start `UNKNOWN`, which remains a
full optimizer barrier and emits conservative continuations.

The flow graph reads only this signal contract.  `RXOP_SEM_MAY_THROW` can later
be renamed or narrowed as generic failure/opacity metadata, but that mechanical
cleanup is not a prerequisite.

## Handler-policy and continuation contract

Handler policy is mutable frame state with its own SSA identity:

- the root frame owns the table; child frames inherit the pointer;
- the first child mutation copies the table; child changes disappear on return
  or unwind;
- `sigpush`/`sigpop` mutate a frame-local saved-entry stack;
- handler installation validates the signal name and obtains a private table
  before changing policy;
- `SIGCALLA` can select skip, retry or fail after its call frame returns;
- branch/call/return handlers can unwind frames and restore compiler call-window
  mappings; and
- native/asynchronous entry uses a conservative synthetic root rather than
  dense edges from every instruction.

For a faulting instruction, normal, skip and retry are distinct edge states.
Skip resumes after the instruction with only failure-visible writes.  Retry
re-enters the recorded faulting address with failure-visible state.  Faults
publish the faulting address; breakpoint/native asynchronous entry publishes
the next/resume address.  Signal code, name, payload, module and address are
observable.

TRACE/source metadata is also observable, but it is not an executable anchor.
The current VM batches all visible trace events at the reached executable
address.  The graph must preserve that ordered metadata batch; it must not keep
or manufacture `cnop` merely to give each past event a separate instruction.

## Executable oracle

[`current-signal-phase.rxas`](current-signal-phase.rxas) covers:

- before-write failure: `DCOPY`, `STOI`, `SETNUMDGTS`;
- after-write failure: `FTOI`, literal `SETNUMFUZ`;
- named partial failure: truncated-UTF-8 `FREADCDPT` changes its string
  component while retaining the prior integer component;
- current non-signalling `FTOS` and mc-decimal absent `DTOS` behaviour;
- skip and retry continuations; and
- optimized and no-opt images on both `rxvm` and `rxbvm`.

All four combinations passed.  [`action-fail.rxas`](action-fail.rxas) proves
the remaining action-aware result: all four combinations selected the default
panic at bytecode address 3 and returned signal code 30.

The Stage 0 focused suite already passed 61/61 and supplies the wider executable
contract:

| Contract | Existing oracle |
| --- | --- |
| ignore, branch, call, call/branch, return, halt | `tests_signals.rxas`, `tests_signal_handler_modes.rxas` |
| action-aware skip and dynamic names | `tests_signal_actions.rxas` |
| branch-bound signal value/payload | `tests_signal_branch_value.rxas` |
| inherited policy and child isolation | `tests_signal_frame_scope.rxas` |
| push/pop restoration | `tests_signal_pushpop.rxas` |
| nested pending signal order | `tests_signal_nested_pending.rxas` |
| call-window restoration on unwind | `tests_signal_call_unwind.rxas` |
| payload and source metadata | `tests_signal_call_attrs.rxas`, panic/source tests |
| native/asynchronous entry | `tests/native_signals/intr_test.rxas` |
| breakpoint next-address/trace batch | breakpoint address and trace-event tests |
| pre-write alias failure under skip | `storage_identity_runtime.rxas` |

The two new evidence fixtures are retained outside the ordinary CTest product
suite because Gate 1 may intentionally change some of the recorded current
semantics.  Selected semantics will receive permanent focused tests with the
metadata implementation.

## Conversion-family decision table

`MAY_THROW` below means current coarse metadata, not proven VM signal control.

| Instruction | Current VM signal/write contract | Current metadata | Gate 1 treatment |
| --- | --- | --- | --- |
| `BTOI` | no signal; integer normalized in place | `MAY_THROW` | metadata correction to `NONE` |
| `BTOD` | plugin call then signal poll; plugin-result state visible | `MAY_THROW` | known plugin-partial contract |
| `BTOF` | no signal | `MAY_THROW` | metadata correction to `NONE` |
| `BTOS` | no signal | `MAY_THROW` | metadata correction to `NONE` |
| `ITOS` | no signal; integer + numeric context derive string | non-throwing | retain `NONE`, success-stable |
| `FTOS` | no signal; float + numeric context derive string | `MAY_THROW` | metadata correction to `NONE`, success-stable |
| `ITOF` | no signal | non-throwing | retain `NONE`, success-stable |
| `FTOI` | writes integer, then signals `CONVERSION_ERROR` if inexact | `MAY_THROW` | known `AFTER_WRITES`, success-stable after normal completion |
| `FTOB` | no signal | `MAY_THROW` | metadata correction to `NONE` |
| `ITOB` | no signal | `MAY_THROW` | metadata correction to `NONE` |
| `STOB` | no signal | `MAY_THROW` | metadata correction to `NONE` |
| `STOF` | parse failure leaves prior float unchanged | `MAY_THROW` | known `BEFORE_WRITES`; normal completion success-stable |
| `STOI` | parse failure leaves prior integer unchanged | `MAY_THROW` | known `BEFORE_WRITES`; normal completion success-stable |
| `STOD` | plugin call then poll; plugin-result state visible | `MAY_THROW` | known plugin-partial contract |
| `DTOS` | writes/formats string, then polls plugin | `MAY_THROW` | Gate 1 choice S3 |
| `DTOI` | plugin conversion then poll; result may be visible | `MAY_THROW` | named plugin-partial contract |
| `DTOB` | writes boolean, then polls plugin | `MAY_THROW` | known `AFTER_WRITES` |
| `ITOD` | plugin call then poll; plugin-result state visible | `MAY_THROW` | named plugin-partial contract |
| `FTOD` | plugin call then poll; plugin-result state visible | `MAY_THROW` | named plugin-partial contract |
| `DTOF` | plugin conversion then poll; result may be visible | `MAY_THROW` | named plugin-partial contract |

The generic repeated-operation proof is not limited to `xTOS`.  A normal first
execution can prove a dominated identical second execution non-signalling only
when metadata declares success stability and every source component, numeric
context and named effect identity is unchanged.  This applies to `STOI`,
`STOF`, `FTOI`, decimal operations and future instructions as their contracts
are proved.  A skipped or failed first execution supplies no success fact.

## Gate 1 semantic choices

The recommended package is **S1 through S5**.

| Choice | Proposed contract | Classification and compatibility |
| --- | --- | --- |
| **S1** | Correct signal metadata to actual VM behaviour: make the total scalar/boolean conversions above `NONE`; add known signal contracts for checked `INC`/`DEC` and all numeric setters/`NUMSCI`/`NUMENG`; retain deliberate unknowns elsewhere. | Metadata correction; no VM behaviour change. It can change later optimized output only when a proof consumes it. **Recommend accept.** |
| **S2** | Make `DCOPY` total like the other typed copies: an absent decimal source makes the destination decimal component absent, retains reusable backing storage, preserves unrelated components, and does not signal. | Compatible totalisation for normal code; skip handlers that deliberately observe `INVALID_ARGUMENTS` change. **Recommend accept.** |
| **S3** | Make `DTOS` non-signalling. Both decimal plugins format an absent decimal as `nan`; formatting clears plugin diagnostic state before and after materialization. Same decimal + numeric-context identities give a success-stable string derivation. | Public signal-contract change. It also fixes the current plugin disagreement: mc-decimal returns `nan` for absence while db-decimal dereferences the absent payload. **Recommend accept**, matching the requested TOS-family contract. |
| **S4** | Route all checked in-place `INC`/`DEC` forms through a temporary and commit only on success, giving `BEFORE_WRITES` overflow semantics on every compiler/platform. | Cross-platform semantic normalization. Clang/GCC overflow builtins may currently store a wrapped value while the fallback helper preserves the old value. Skip/retry observers can therefore change. **Recommend accept.** |
| **S5** | Add the missing early dispatch to invalid literal `SETNUMFUZ`; all invalid numeric-context setters then preserve the prior context and signal before writes. | Observable correction of an inconsistent current literal form, which today installs and synchronizes negative fuzz after queuing its signal. **Recommend accept.** |

If any choice is declined, the infrastructure remains correct by recording the
current contract and preserving the associated edge state.  The cost is more
irregular proof logic and fewer non-signalling or repetition proofs.

## Gate 1 stop

No VM implementation, plugin implementation or opcode signal metadata was
changed in this stage.  Gate 1 approval is required before applying S1-S5 and
adding the aligned metadata inventory.  Stage 2 graph construction follows
only after the selected executable contract is locked.
